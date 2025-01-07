/**
	@file
	ears.ptrack.c
 
	@name
	ears.ptrack~
 
	@realname
    ears.ptrack~
 
	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	Partial tracking
 
	@description
	Performs partial tracking by connecting spectral peaks.
 
	@discussion
    If an ordinary (temporal) buffer is input, a Short-Time Fourier Transform is computed,
    its peaks are found, and finally partial tracking is performed on them.
    If a spectral buffer is input (with as many channels as bins), then the peaks
    are directly computed from the spectral representation and partial tracking is
    performed on them.
    If an llll is input, it is considered as a set of peaks to be tracked.

	@category
	ears spectral
 
	@keywords
	buffer, partial tracking, partials, sinusoid, spectrum
 
	@seealso
	ears.peaks~, ears.pshow~, ears.stft~, ears.cqt~, ears.trans~, ears.model.ptrack.synthesis~,
	
	@owner
	Daniele Ghisi
 */


#include "ext.h"
#include "ext_obex.h"
#include "foundation/llllobj.h"
#include "foundation/llll_commons_ext.h"
#include "math/bach_math_utilities.h"
#include "ears.object.h"
#include "ears.spectral.h"

typedef struct _buf_ptrack {
    t_earsbufobj       e_ob;

    long        e_interpolate;
    long        e_maxPeaks;
    double      e_minPeakDistance;
    t_symbol    *e_orderBy;
    double      e_threshold;
    
    char        e_account_for_masking; // currently unimplemented
    
    t_llll      *e_fromPeaks;
    
    // partial tracking options
    double      e_freq_speed_threshold; // connected to max glissando slope
    double      e_graceperiod_before_dying;
    double      e_min_partial_length;
    double      e_min_partial_avg_amp;
    char     e_thresh_timeunit;
    char     e_thresh_frequnit;
    char      e_thresh_ampunit;
} t_buf_ptrack;



// Prototypes
t_buf_ptrack*    buf_ptrack_new(t_symbol *s, short argc, t_atom *argv);
void           buf_ptrack_free(t_buf_ptrack *x);
void           buf_ptrack_bang(t_buf_ptrack *x);
void           buf_ptrack_anything(t_buf_ptrack *x, t_symbol *msg, long ac, t_atom *av);

void buf_ptrack_assist(t_buf_ptrack *x, void *b, long m, long a, char *s);
void buf_ptrack_inletinfo(t_buf_ptrack *x, void *b, long a, char *t);


// Globals and Statics
static t_class    *s_ptrack_class = NULL;

EARSBUFOBJ_ADD_IO_METHODS(ptrack)

/**********************************************************************/
// Class Definition and Life Cycle

t_max_err buf_ptrack_setattr_threshtimeunit(t_buf_ptrack *x, void *attr, long argc, t_atom *argv)
{
    if (argc && argv) {
        if (atom_gettype(argv) == A_LONG)
            x->e_thresh_timeunit = atom_getlong(argv);
        else if (atom_gettype(argv) == A_SYM) {
            t_symbol *s = atom_getsym(argv);
            e_ears_timeunit new_timeunit = ears_timeunit_from_symbol(s);
            if (new_timeunit != EARS_TIMEUNIT_UNKNOWN)
                x->e_thresh_timeunit = new_timeunit;
            else
                object_error((t_object *)x, "Unknown threshold time unit!");
        }
    }
    return MAX_ERR_NONE;
}

t_max_err buf_ptrack_setattr_threshampunit(t_buf_ptrack *x, void *attr, long argc, t_atom *argv)
{
    if (argc && argv) {
        if (atom_gettype(argv) == A_LONG)
            x->e_thresh_ampunit = atom_getlong(argv);
        else if (atom_gettype(argv) == A_SYM) {
            t_symbol *s = atom_getsym(argv);
            e_ears_ampunit new_ampunit = ears_ampunit_from_symbol(s);
            if (new_ampunit != EARS_AMPUNIT_UNKNOWN)
                x->e_thresh_ampunit = new_ampunit;
            else
                object_error((t_object *)x, "Unknown threshold amplitude unit!");
        }
    }
    return MAX_ERR_NONE;
}


t_max_err buf_ptrack_setattr_threshfrequnit(t_buf_ptrack *x, void *attr, long argc, t_atom *argv)
{
    if (argc && argv) {
        if (atom_gettype(argv) == A_LONG)
            x->e_thresh_frequnit = atom_getlong(argv);
        else if (atom_gettype(argv) == A_SYM) {
            e_ears_frequnit new_frequnit = ears_frequnit_from_symbol(atom_getsym(argv));
            if (new_frequnit != EARS_FREQUNIT_UNKNOWN)
                x->e_thresh_frequnit = new_frequnit;
            else
                object_error((t_object *)x, "Unknown threshold frequency unit!");
        }
    }
    return MAX_ERR_NONE;
}



void C74_EXPORT ext_main(void* moduleRef)
{
    common_symbols_init();
    llllobj_common_symbols_init();
    
    if (llllobj_check_version(bach_get_current_llll_version()) || llllobj_test()) {
        ears_error_bachcheck();
        return;
    }
    
    t_class *c;
    
    CLASS_NEW_CHECK_SIZE(c, "ears.ptrack~",
                         (method) buf_ptrack_new,
                         (method) buf_ptrack_free,
                         sizeof(t_buf_ptrack),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    // @method list/llll @digest Process buffers
    // @description A list or llll with buffer names will trigger the buffer processing and output the processed
    // buffer names (depending on the <m>alloc</m> attribute).
    // If the input buffer is a spectral buffer, then magnitudes and phases are expected to be input from the
    // two inlets. Otherwise, only the temporal buffer is expected to be input from the left inlet, and
    // a Short-Time Fourier Transform will be performed.
    EARSBUFOBJ_DECLARE_COMMON_METHODS_HANDLETHREAD(ptrack)
    
    llllobj_class_add_out_attr(c, LLLL_OBJ_VANILLA);

    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_blocking_attr(c);
    earsbufobj_class_add_alloc_attr(c);
    earsbufobj_class_add_frequnit_attr(c);
    earsbufobj_class_add_timeunit_attr(c);
    earsbufobj_class_add_ampunit_attr(c);
    earsbufobj_class_add_angleunit_attr(c);

    earsbufobj_class_add_framesize_attr(c);
    earsbufobj_class_add_hopsize_attr(c);
    earsbufobj_class_add_numframes_attr(c);
    earsbufobj_class_add_overlap_attr(c);
    earsbufobj_class_add_wintype_attr(c);
    earsbufobj_class_add_winstartfromzero_attr(c);
    earsbufobj_class_add_fftnormalization_attr(c);

    
    CLASS_ATTR_LONG(c, "interpolate", 0, t_buf_ptrack, e_interpolate);
    CLASS_ATTR_STYLE_LABEL(c,"interpolate",0,"onoff","Interpolate");
    CLASS_ATTR_CATEGORY(c, "interpolate", 0, "Peak Detection");
    CLASS_ATTR_BASIC(c, "interpolate", 0);
    // @description Toggles the ability to perform cubic interpolation to find 'true' peaks.
    
    
    CLASS_ATTR_LONG(c, "maxpeaks", 0, t_buf_ptrack, e_maxPeaks);
    CLASS_ATTR_STYLE_LABEL(c,"maxpeaks",0,"text","Maximum Number of Peaks");
    CLASS_ATTR_CATEGORY(c, "maxpeaks", 0, "Peak Detection");
    CLASS_ATTR_BASIC(c, "maxpeaks", 0);
    // @description Sets the maximum number of peaks, 0 means: all.

    CLASS_ATTR_DOUBLE(c, "minpeakdistance", 0, t_buf_ptrack, e_minPeakDistance);
    CLASS_ATTR_STYLE_LABEL(c,"minpeakdistance",0,"text","Minimum Distance Between Peaks");
    CLASS_ATTR_CATEGORY(c, "minpeakdistance", 0, "Peak Detection");
    CLASS_ATTR_BASIC(c, "minpeakdistance", 0);
    // @description Sets the minimum distance between the peaks

    CLASS_ATTR_DOUBLE(c, "peakampthresh", 0, t_buf_ptrack, e_threshold);
    CLASS_ATTR_STYLE_LABEL(c,"peakampthresh",0,"text","Peak Threshold");
    CLASS_ATTR_CATEGORY(c, "peakampthresh", 0, "Peak Detection");
    CLASS_ATTR_BASIC(c, "peakampthresh", 0);
    // @description Sets a threshold, so that peaks with amplitude below this threshold are discarded.
    

    CLASS_ATTR_CHAR(c, "threshtimeunit", 0, t_buf_ptrack, e_thresh_timeunit);
    CLASS_ATTR_STYLE_LABEL(c,"threshtimeunit",0,"enumindex","Threshold Time Values Unit");
    CLASS_ATTR_ENUMINDEX(c,"threshtimeunit", 0, "Milliseconds Samples Relative");
    CLASS_ATTR_ACCESSORS(c, "threshtimeunit", NULL,buf_ptrack_setattr_threshtimeunit);
    CLASS_ATTR_BASIC(c, "threshtimeunit", 0);
    CLASS_ATTR_CATEGORY(c, "threshtimeunit", 0, "Units");
    // @description Sets the unit for thresholds: Milliseconds (default), Samples, Relative (0. to 1. as a percentage of the buffer length).

    CLASS_ATTR_CHAR(c, "threshfrequnit", 0, t_buf_ptrack, e_thresh_frequnit);
    CLASS_ATTR_STYLE_LABEL(c,"threshfrequnit",0,"enumindex","Threshold Frequency Values Unit");
    CLASS_ATTR_ENUMINDEX(c,"threshfrequnit", 0, "Hertz BPM Cents MIDI");
    CLASS_ATTR_ACCESSORS(c, "threshfrequnit", NULL,buf_ptrack_setattr_threshfrequnit);
    CLASS_ATTR_BASIC(c, "threshfrequnit", 0);
    CLASS_ATTR_CATEGORY(c, "threshfrequnit", 0, "Units");
    // @description Sets the unit for threshold frequency values: Hertz (default), BPM, Cents, MIDI numbers (semitones)

    CLASS_ATTR_CHAR(c, "threshampunit", 0, t_buf_ptrack, e_thresh_ampunit);
    CLASS_ATTR_STYLE_LABEL(c,"threshampunit",0,"enumindex","Threshold Amplitude Values Unit");
    CLASS_ATTR_ENUMINDEX(c,"threshampunit", 0, "Linear Decibel");
    CLASS_ATTR_ACCESSORS(c, "threshampunit", NULL,buf_ptrack_setattr_threshampunit);
    CLASS_ATTR_BASIC(c, "threshampunit", 0);
    CLASS_ATTR_CATEGORY(c, "threshampunit", 0, "Units");
    // @description Sets the unit for amplitudes: Linear (default) or Decibel.

    
    
    CLASS_ATTR_DOUBLE(c, "freqratethresh", 0, t_buf_ptrack, e_freq_speed_threshold);
    CLASS_ATTR_STYLE_LABEL(c,"freqratethresh",0,"text","Frequency Change Threshold");
    CLASS_ATTR_CATEGORY(c, "freqratethresh", 0, "Partial Tracking");
    CLASS_ATTR_BASIC(c, "freqratethresh", 0);
    // @description Sets a threshold for the rate of frequency change (i.e. max glissando slope), in <m>threshfrequnit</m> per seconds.
    // For instance, if <m>threshfrequnit</m> is cents, this value is the maximum cents variation per second

    CLASS_ATTR_DOUBLE(c, "maxgap", 0, t_buf_ptrack, e_graceperiod_before_dying);
    CLASS_ATTR_STYLE_LABEL(c,"maxgap",0,"text","Maximum Gap Between Peaks In A Partial");
    CLASS_ATTR_CATEGORY(c, "maxgap", 0, "Partial Tracking");
    CLASS_ATTR_BASIC(c, "maxgap", 0);
    // @description Sets a maximum allowed temporal gap between peaks in a partial, in <m>threshtimeunit</m>.

    CLASS_ATTR_DOUBLE(c, "minpartiallength", 0, t_buf_ptrack, e_min_partial_length);
    CLASS_ATTR_STYLE_LABEL(c,"minpartiallength",0,"text","Minimum Partial Length");
    CLASS_ATTR_CATEGORY(c, "minpartiallength", 0, "Partial Tracking");
    CLASS_ATTR_BASIC(c, "minpartiallength", 0);
    // @description Sets a minimum partial length, in <m>threshtimeunit</m>.

    CLASS_ATTR_DOUBLE(c, "minpartialavgamp", 0, t_buf_ptrack, e_min_partial_avg_amp);
    CLASS_ATTR_STYLE_LABEL(c,"minpartialavgamp",0,"text","Minimum Partial Average Amplitude");
    CLASS_ATTR_CATEGORY(c, "minpartialavgamp", 0, "Partial Tracking");
    CLASS_ATTR_BASIC(c, "minpartialavgamp", 0);
    // @description Sets a minimum partial average amplitude, in <m>threshampunit</m>.
    
    
    class_register(CLASS_BOX, c);
    s_ptrack_class = c;
}

void buf_ptrack_assist(t_buf_ptrack *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        if (a == 0)
            // @in 0 @type symbol @digest Source temporal buffer or spectrogram magnitudes
            // @description If you want to compute the peaks of a temporal buffer, send it in the first inlet; if you
            // want to compute the peaks of a spectral representation, send the magnitudes here (and optional phases in the second inlet)
            sprintf(s, "symbol/llll: Source Buffer Or Spectrogram Magnitudes");
        else
            // @in 1 @type symbol @digest Optional buffer containing spectrogram phases
            // @description Source spectrogram phases
            sprintf(s, "symbol/llll: Optional Phase Spectrogram Buffer");
    } else {
        char *type = NULL;
        llllobj_get_llll_outlet_type_as_string((t_object *) x, LLLL_OBJ_VANILLA, a, &type);
        if (a == 0)
            // @out 0 @type llll @digest Detected partials
            // @description Outputs partials organized as follows: one llll per partial;
            // each partial contains a sublist for each peak, in the form: <b>[<m>onset</m> <m>frequency</m> <m>amplitude</m> <m>phase</m>]</b>,
            // where <m>phase</m> is only present if a temporal buffer or a phase spectrogram has been provided.
            sprintf(s, "llll (%s): Partials", type);
        else
            // @out 1 @type llll @digest Detected partials in <o>bach.roll</o> format
            // @description Partials embedded the gathered syntax of a <o>bach.roll</o> proportionally notate score.
            sprintf(s, "llll (%s): Partials In bach.roll Format", type);
    }
}

void buf_ptrack_inletinfo(t_buf_ptrack *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_ptrack *buf_ptrack_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_ptrack *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_ptrack*)object_alloc_debug(s_ptrack_class);
    if (x) {
        x->e_interpolate = 1;
        x->e_maxPeaks = 0; // 0 = all
        x->e_minPeakDistance = 0.;
        x->e_orderBy = gensym("amplitude");
        x->e_threshold = -100000;
        
        x->e_thresh_frequnit = EARS_FREQUNIT_CENTS;
        x->e_thresh_timeunit = EARS_TIMEUNIT_MS;
        x->e_thresh_ampunit = EARS_AMPUNIT_DECIBEL;

        x->e_freq_speed_threshold = 400; // 400 cents per second: it makes about 20 cents per ordinary frame, with standard analysis parameters
        x->e_graceperiod_before_dying = 25; // ms
        x->e_min_partial_length = 100; // ms
        x->e_min_partial_avg_amp = -90; // dB

        earsbufobj_init((t_earsbufobj *)x, EARSBUFOBJ_FLAG_NONE); // EARSBUFOBJ_FLAG_SUPPORTS_COPY_NAMES);
        
        // @arg 0 @name outname @optional 1 @type symbol
        // @digest Output buffer name
        // @description @copy EARS_DOC_OUTNAME_ATTR
        
        t_llll *args = llll_parse(true_ac, argv);
        t_llll *names = earsbufobj_extract_names_from_args((t_earsbufobj *)x, args);

        attr_args_process(x, argc, argv);

        earsbufobj_setup((t_earsbufobj *)x, "ee", "44", names);

        llll_free(args);
        llll_free(names);
    }
    return x;
}


void buf_ptrack_free(t_buf_ptrack *x)
{
    if (x->e_fromPeaks)
        llll_free(x->e_fromPeaks);

    earsbufobj_free((t_earsbufobj *)x);
}



void buf_ptrack_bang(t_buf_ptrack *x)
{
    long num_buffers = earsbufobj_get_instore_size((t_earsbufobj *)x, 0);
    
    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_buffers, true);

    t_llll *peaks = NULL;
    if (x->e_fromPeaks) {
        peaks = llll_clone(x->e_fromPeaks);
    } else if (num_buffers > 0) { // must be just 1 buffer
        t_buffer_obj *in1 = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, 0);
        t_buffer_obj *in2 = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 1, 0);
        t_ears_err err = EARS_ERR_NONE;

        peaks = llll_get();
        if (ears_buffer_is_spectral((t_object *)x, in1)) {
        
            t_llll *these_peaks = ears_specbuffer_peaks((t_object *)x, in1, in2, x->e_interpolate, x->e_maxPeaks > 0 ? x->e_maxPeaks : INT_MAX, x->e_minPeakDistance, x->e_orderBy, x->e_threshold, (e_ears_timeunit)x->e_ob.l_timeunit, (e_ears_ampunit)x->e_ob.l_ampunit, (e_ears_angleunit)x->e_ob.l_angleunit, (e_ears_ampunit)x->e_thresh_ampunit, &err);
       
            llll_appendllll(peaks, these_peaks);
            
        } else {
            t_buffer_obj *mags = ears_buffer_make(NULL);
            t_buffer_obj *phases = ears_buffer_make(NULL);

            long numchannels = ears_buffer_get_numchannels((t_object *)x, in1);
            peaks = llll_get();
            for (long c = 0; c < numchannels; c++) {
                ears_buffer_stft((t_object *)x, in1, NULL, c,
                                 mags, phases,
                                 earsbufobj_time_to_samps((t_earsbufobj *)x, x->e_ob.a_framesize, in1, EARSBUFOBJ_CONVERSION_FLAG_ISANALYSIS),
                                 earsbufobj_time_to_samps((t_earsbufobj *)x, x->e_ob.a_hopsize, in1, EARSBUFOBJ_CONVERSION_FLAG_ISANALYSIS),
                                 x->e_ob.a_wintype ? x->e_ob.a_wintype->s_name : "rect",
                                 false, true, false, (e_ears_angleunit)x->e_ob.l_angleunit, x->e_ob.a_winstartfromzero, (e_ears_fft_normalization)x->e_ob.a_fftnormalization);

                t_llll *these_peaks = ears_specbuffer_peaks((t_object *)x, mags, phases, x->e_interpolate, x->e_maxPeaks > 0 ? x->e_maxPeaks : INT_MAX, x->e_minPeakDistance, x->e_orderBy, x->e_threshold, (e_ears_timeunit)x->e_ob.l_timeunit, (e_ears_ampunit)x->e_ob.l_ampunit, (e_ears_angleunit)x->e_ob.l_angleunit, (e_ears_ampunit)x->e_thresh_ampunit,  &err);

                llll_appendllll(peaks, these_peaks);
            }

            ears_buffer_free(mags);
            ears_buffer_free(phases);
        }
    }
    
    if (peaks) {
    
        t_llll *ptrack = ears_ptrack((t_object *)x, peaks, x->e_freq_speed_threshold, x->e_graceperiod_before_dying, x->e_min_partial_length, x->e_min_partial_avg_amp,
                                     (e_ears_timeunit)x->e_ob.l_timeunit, (e_ears_frequnit)x->e_ob.l_frequnit, (e_ears_ampunit)x->e_ob.l_ampunit, // input units for the peeks
                                     (e_ears_timeunit)x->e_thresh_timeunit, (e_ears_frequnit)x->e_thresh_frequnit, (e_ears_ampunit)x->e_thresh_ampunit, // threshold units
                                     earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, 0));
        
        t_llll *roll_gs = ears_ptrack_to_roll((t_object *)x, ptrack, (e_ears_timeunit)x->e_ob.l_timeunit, (e_ears_frequnit)x->e_ob.l_frequnit, (e_ears_ampunit)x->e_ob.l_ampunit);

        earsbufobj_outlet_llll((t_earsbufobj *)x, 1, roll_gs);
        earsbufobj_outlet_llll((t_earsbufobj *)x, 0, ptrack);
        llll_free(roll_gs);
        llll_free(ptrack);
        llll_free(peaks);
    }
}



void buf_ptrack_anything(t_buf_ptrack *x, t_symbol *msg, long ac, t_atom *av)
{
    long inlet = earsbufobj_proxy_getinlet((t_earsbufobj *) x);

    t_llll *parsed = earsbufobj_parse_gimme((t_earsbufobj *) x, LLLL_OBJ_VANILLA, msg, ac, av);
    if (!parsed) return;
    
    if (parsed && parsed->l_head) {
        if (hatom_gettype(&parsed->l_head->l_hatom) == H_SYM) {
            
            if (inlet == 0) {
                long num_bufs = llll_get_num_symbols_root(parsed);
                earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_bufs, true);
                earsbufobj_store_buffer_list((t_earsbufobj *)x, parsed, 0);
                if (x->e_fromPeaks)
                    llll_free(x->e_fromPeaks);
                x->e_fromPeaks = NULL;
                
               buf_ptrack_bang(x);
            } else {
                long num_bufs = llll_get_num_symbols_root(parsed);
                earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 1, num_bufs, true);
                earsbufobj_store_buffer_list((t_earsbufobj *)x, parsed, 1);
            }
        } else {
            if (inlet == 0) { // from peaks
                x->e_fromPeaks = llll_clone(parsed);
                buf_ptrack_bang(x);
            }
        }
    }
    llll_free(parsed);
}


