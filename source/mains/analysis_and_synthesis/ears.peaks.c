/**
	@file
	ears.peaks.c
 
	@name
	ears.peaks~
 
	@realname
	ears.peaks~
 
	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	Find spectral or temporal peaks
 
	@description
	Finds the spectral of temporal peaks of an incoming buffer.

	@discussion
    If an ordinary (temporal) buffer is input, by default a Short-Time Fourier Transform is computed
    and its peaks are found. If one wants temporal peaks instead, one should set the <m>spectral</m> attribute to zero.
    If a spectral buffer is input (with as many channels as bins), then the peaks
    are directly computed from the spectral representation.
 
	@category
	ears spectral
 
	@keywords
	buffer, peaks, spectrum
 
	@seealso
	ears.stft~, ears.cqt~, ears.trans~, ears.ptrack~, ears.pshow~
	
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

typedef struct _buf_peaks {
    t_earsbufobj       e_ob;

    long        e_interpolate;
    long        e_maxPeaks;
    double      e_minPeakDistance;
    t_symbol    *e_orderBy;
    double      e_threshold;
    char        e_thresh_ampunit;
    
    long        run_spectral_analysis;
    long        downmix;
    
    char        e_account_for_masking; // currently unused
} t_buf_peaks;



// Prototypes
t_buf_peaks*     buf_peaks_new(t_symbol *s, short argc, t_atom *argv);
void			buf_peaks_free(t_buf_peaks *x);
void			buf_peaks_bang(t_buf_peaks *x);
void			buf_peaks_anything(t_buf_peaks *x, t_symbol *msg, long ac, t_atom *av);

void buf_peaks_assist(t_buf_peaks *x, void *b, long m, long a, char *s);
void buf_peaks_inletinfo(t_buf_peaks *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_peaks_class = NULL;

EARSBUFOBJ_ADD_IO_METHODS(peaks)

/**********************************************************************/
// Class Definition and Life Cycle

t_max_err buf_peaks_setattr_threshampunit(t_buf_peaks *x, void *attr, long argc, t_atom *argv)
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


void C74_EXPORT ext_main(void* moduleRef)
{
    common_symbols_init();
    llllobj_common_symbols_init();
    
    if (llllobj_check_version(bach_get_current_llll_version()) || llllobj_test()) {
        ears_error_bachcheck();
        return;
    }
    
    t_class *c;
    
    CLASS_NEW_CHECK_SIZE(c, "ears.peaks~",
                         (method)buf_peaks_new,
                         (method)buf_peaks_free,
                         sizeof(t_buf_peaks),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    // @method list/llll @digest Process buffers
    // @description A list or llll with buffer names will trigger the buffer processing and output the processed
    // buffer names (depending on the <m>alloc</m> attribute).
    // If the input buffer is a spectral buffer, then magnitudes and phases are expected to be input from the
    // two inlets. Otherwise, only the temporal buffer is expected to be input from the left inlet, and
    // a Short-Time Fourier Transform will be performed.
    EARSBUFOBJ_DECLARE_COMMON_METHODS_HANDLETHREAD(peaks)
    
    llllobj_class_add_out_attr(c, LLLL_OBJ_VANILLA);

    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_blocking_attr(c);
    earsbufobj_class_add_alloc_attr(c);
    earsbufobj_class_add_timeunit_attr(c);
    earsbufobj_class_add_ampunit_attr(c);
    earsbufobj_class_add_angleunit_attr(c);

    earsbufobj_class_add_fftnormalization_attr(c);
    earsbufobj_class_add_framesize_attr(c);
    earsbufobj_class_add_hopsize_attr(c);
    earsbufobj_class_add_numframes_attr(c);
    earsbufobj_class_add_overlap_attr(c);
    earsbufobj_class_add_wintype_attr(c);
    earsbufobj_class_add_winstartfromzero_attr(c);

    
    CLASS_ATTR_LONG(c, "interpolate", 0, t_buf_peaks, e_interpolate);
    CLASS_ATTR_STYLE_LABEL(c,"interpolate",0,"onoff","Interpolate");
    CLASS_ATTR_CATEGORY(c, "interpolate", 0, "Peak Detection");
    CLASS_ATTR_BASIC(c, "interpolate", 0);
    // @description Toggles the ability to perform cubic interpolation to find 'true' peaks.
    
    
    CLASS_ATTR_CHAR(c, "threshampunit", 0, t_buf_peaks, e_thresh_ampunit);
    CLASS_ATTR_STYLE_LABEL(c,"threshampunit",0,"enumindex","Threshold Amplitude Values Unit");
    CLASS_ATTR_ENUMINDEX(c,"threshampunit", 0, "Linear Decibel");
    CLASS_ATTR_ACCESSORS(c, "threshampunit", NULL, buf_peaks_setattr_threshampunit);
    CLASS_ATTR_BASIC(c, "threshampunit", 0);
    CLASS_ATTR_CATEGORY(c, "threshampunit", 0, "Units");
    // @description Sets the unit for amplitudes: Linear (default) or Decibel.

    
    
    CLASS_ATTR_LONG(c, "downmix",    0,    t_buf_peaks, downmix);
    CLASS_ATTR_STYLE_LABEL(c, "downmix", 0, "onoff", "Downmix to Mono");
    CLASS_ATTR_BASIC(c, "downmix", 0);
    // @description Toggles the ability to downmix all the channels into one. If this flag is not set, then
    // one buffer per channel is output.

    CLASS_ATTR_LONG(c, "spectral",    0,    t_buf_peaks, run_spectral_analysis);
    CLASS_ATTR_STYLE_LABEL(c, "spectral", 0, "onoff", "Run Spectral Analysis If Needed");
    CLASS_ATTR_BASIC(c, "spectral", 0);
    // @description Toggles the ability to run spectral analysis on non-spectral buffers.
    // By default it is on, but if you want to analyze peaks of an ordinary temporal buffer, turn this off.
    
    
    CLASS_ATTR_LONG(c, "maxpeaks", 0, t_buf_peaks, e_maxPeaks);
    CLASS_ATTR_STYLE_LABEL(c,"maxpeaks",0,"text","Maximum Number of Peaks");
    CLASS_ATTR_CATEGORY(c, "maxpeaks", 0, "Peak Detection");
    CLASS_ATTR_BASIC(c, "maxpeaks", 0);
    // @description Sets the maximum number of peaks, 0 means: all.

    CLASS_ATTR_DOUBLE(c, "minpeakdistance", 0, t_buf_peaks, e_minPeakDistance);
    CLASS_ATTR_STYLE_LABEL(c,"minpeakdistance",0,"text","Minimum Distance Between Peaks");
    CLASS_ATTR_CATEGORY(c, "minpeakdistance", 0, "Peak Detection");
    CLASS_ATTR_BASIC(c, "minpeakdistance", 0);
    // @description Sets the minimum distance between the peaks

    CLASS_ATTR_SYM(c, "orderby", 0, t_buf_peaks, e_orderBy);
    CLASS_ATTR_STYLE_LABEL(c,"orderby",0,"enum","Order By");
    CLASS_ATTR_ENUM(c,"orderby", 0, "position amplitude");
    CLASS_ATTR_CATEGORY(c, "orderby", 0, "Peak Detection");
    CLASS_ATTR_BASIC(c, "orderby", 0);
    // @description Sets the ordering type (either "position" or "amplitude").

    CLASS_ATTR_DOUBLE(c, "peakampthresh", 0, t_buf_peaks, e_threshold);
    CLASS_ATTR_STYLE_LABEL(c,"peakampthresh",0,"text","Peak Threshold");
    CLASS_ATTR_CATEGORY(c, "peakampthresh", 0, "Peak Detection");
    CLASS_ATTR_BASIC(c, "peakampthresh", 0);
    // @description Sets a threshold, so that peaks with amplitude below this threshold are discarded.
    

    class_register(CLASS_BOX, c);
    s_peaks_class = c;
}

void buf_peaks_assist(t_buf_peaks *x, void *b, long m, long a, char *s)
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
        // @out 0 @type llll @digest Detected peaks
        // @description Outputs peaks organized as follows: one llll per channel,
        // then one llll per frame with the onset as first element, then one llll per peak,
        // each peak being in the form: <b>[<m>position</m> <m>amplitude</m> <m>phase</m>]</b>,
        // where <m>phase</m> is only present if a temporal buffer or a phase spectrogram has been provided.
        sprintf(s, "llll (%s): Peaks", type);
    }
}

void buf_peaks_inletinfo(t_buf_peaks *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_peaks *buf_peaks_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_peaks *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_peaks*)object_alloc_debug(s_peaks_class);
    if (x) {
        x->e_interpolate = 1;
        x->e_maxPeaks = 0; // 0 = all
        x->e_minPeakDistance = 0.;
        x->e_orderBy = gensym("amplitude");
        x->e_threshold = -100000;
        x->run_spectral_analysis = true;
        x->e_thresh_ampunit = EARS_AMPUNIT_DECIBEL;

        earsbufobj_init((t_earsbufobj *)x, EARSBUFOBJ_FLAG_NONE); // EARSBUFOBJ_FLAG_SUPPORTS_COPY_NAMES);

        // @arg 0 @name outname @optional 1 @type symbol
        // @digest Output buffer name
        // @description @copy EARS_DOC_OUTNAME_ATTR
        
        t_llll *args = llll_parse(true_ac, argv);
        t_llll *names = earsbufobj_extract_names_from_args((t_earsbufobj *)x, args);

        attr_args_process(x, argc, argv);

        earsbufobj_setup((t_earsbufobj *)x, "EE", "4", names);

        llll_free(args);
        llll_free(names);
    }
    return x;
}


void buf_peaks_free(t_buf_peaks *x)
{
    earsbufobj_free((t_earsbufobj *)x);
}



void buf_peaks_bang(t_buf_peaks *x)
{
    long num_buffers = earsbufobj_get_instore_size((t_earsbufobj *)x, 0);
    
    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_buffers, true);

    if (num_buffers > 0) { // can be more than 1 buffer in case of spectral inputs (every buffer corresponding to a channel)
        t_buffer_obj *in1 = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, 0);
        t_ears_err err = EARS_ERR_NONE;
        t_llll *out = llll_get();
        
        if (!x->run_spectral_analysis && !ears_buffer_is_spectral((t_object *)x, in1)) {
            t_buffer_obj *temp = ears_buffer_make(NULL);
            for (long c = 0; c < num_buffers; c++) {
                t_buffer_obj *inA = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, c);
                ears_buffer_transpose((t_object *)x, inA, temp);
                
                t_llll *ch_peaks = ears_specbuffer_peaks((t_object *)x, temp, NULL, x->e_interpolate, x->e_maxPeaks > 0 ? x->e_maxPeaks : INT_MAX, x->e_minPeakDistance, x->e_orderBy, x->e_threshold, (e_ears_timeunit)x->e_ob.l_timeunit, (e_ears_ampunit)x->e_ob.l_ampunit, (e_ears_angleunit)x->e_ob.l_angleunit, (e_ears_ampunit)x->e_thresh_ampunit, &err);
                
                llll_appendllll(out, ch_peaks);
            }
            ears_buffer_free(temp);
        } else if (ears_buffer_is_spectral((t_object *)x, in1)) {
            for (long c = 0; c < num_buffers; c++) {
                t_buffer_obj *inA = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, c);
                t_buffer_obj *inB = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 1, c);
                
                t_llll *ch_peaks = ears_specbuffer_peaks((t_object *)x, inA, inB, x->e_interpolate, x->e_maxPeaks > 0 ? x->e_maxPeaks : INT_MAX, x->e_minPeakDistance, x->e_orderBy, x->e_threshold, (e_ears_timeunit)x->e_ob.l_timeunit, (e_ears_ampunit)x->e_ob.l_ampunit, (e_ears_angleunit)x->e_ob.l_angleunit, (e_ears_ampunit)x->e_thresh_ampunit, &err);
                
                llll_appendllll(out, ch_peaks);
            }
        } else { // must run spectral analysis
            t_buffer_obj *mags = ears_buffer_make(NULL);
            t_buffer_obj *phases = ears_buffer_make(NULL);
            
            long numchannels = ears_buffer_get_numchannels((t_object *)x, in1);
            for (long c = x->downmix ? -1 : 0; c < (x->downmix ? 0 : numchannels); c++) {
                double framesize_samps = earsbufobj_time_to_samps((t_earsbufobj *)x, x->e_ob.a_framesize, in1, EARSBUFOBJ_CONVERSION_FLAG_ISANALYSIS);
                double hopsize_samps = earsbufobj_time_to_samps((t_earsbufobj *)x, x->e_ob.a_hopsize, in1, EARSBUFOBJ_CONVERSION_FLAG_ISANALYSIS);
                
                ears_buffer_stft((t_object *)x, in1, NULL, c /* -1 means downmixing */,
                                 mags, phases, framesize_samps, hopsize_samps,
                                 x->e_ob.a_wintype ? x->e_ob.a_wintype->s_name : "rect",
                                 false, true, false, (e_ears_angleunit)x->e_ob.l_angleunit, x->e_ob.a_winstartfromzero,
                                 (e_ears_fft_normalization)x->e_ob.a_fftnormalization);
                
                t_llll *ch_peaks = ears_specbuffer_peaks((t_object *)x, mags, phases, x->e_interpolate, x->e_maxPeaks > 0 ? x->e_maxPeaks : INT_MAX, x->e_minPeakDistance, x->e_orderBy, x->e_threshold, (e_ears_timeunit)x->e_ob.l_timeunit, (e_ears_ampunit)x->e_ob.l_ampunit, (e_ears_angleunit)x->e_ob.l_angleunit, (e_ears_ampunit)x->e_thresh_ampunit, &err);
                
                llll_appendllll(out, ch_peaks);
            }
            
            ears_buffer_free(mags);
            ears_buffer_free(phases);
        }
        
        if (x->e_account_for_masking) {
            // TO DO
            //
            //            e^{-3.5\cdot \frac{0.24}{0.021\cdot f_1+19}\cdot (f_2-f_1)}-e^{-5.75\cdot \frac{0.24}{0.021\cdot f_1+19}\cdot (f_2-f_1)}
        }
        
        earsbufobj_outlet_llll((t_earsbufobj *)x, 0, out);
        llll_free(out);
    }
}



void buf_peaks_anything(t_buf_peaks *x, t_symbol *msg, long ac, t_atom *av)
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
                
                buf_peaks_bang(x);
            } else {
                long num_bufs = llll_get_num_symbols_root(parsed);
                earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 1, num_bufs, true);
                earsbufobj_store_buffer_list((t_earsbufobj *)x, parsed, 1);
            }
        }
    }
    llll_free(parsed);
}


