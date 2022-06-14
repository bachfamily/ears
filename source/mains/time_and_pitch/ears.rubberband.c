/**
	@file
	ears.rubberband.c
 
	@name
	ears.rubberband~
 
	@realname
	ears.rubberband~
 
	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	Rubber Band pitch and time processing
 
	@description
    Implements the Rubber Band library algorithms for pitch shifting and time stretching.
 
	@discussion
    The original code for the Rubber Band library is by Breakfast Quay (https://breakfastquay.com/rubberband/)
 
	@category
	ears time and pitch
 
	@keywords
	buffer, pitch, stretch, shift, pitchshift, timestretch, rubberband, rubber, band
 
	@seealso
	ears.soundtouch~, ears.paulstretch~, ears.resample~
	
	@owner
	Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "foundation/llllobj.h"
#include "foundation/llll_commons_ext.h"
#include "bach_math_utilities.h"
#include "ears.object.h"
#include "ears.rubberband_commons.h"


// Rubberband options via define
#define HAVE_VDSP
#define NO_THREADING
// #define USE_PTHREADS

typedef struct _buf_rubberband {
    t_earsbufobj       e_ob;
    
    long               e_transients;
    long               e_detector;
    long               e_stretchmode;
    long               e_phase;
    long               e_fft_size;
    long               e_smoothing;
    long               e_formant;
    long               e_pitchmode;
    long               e_blocksize;
    
    t_llll             *e_pitchshift_env;
    t_llll             *e_timestretch_env;
} t_buf_rubberband;



// Prototypes
t_buf_rubberband*         buf_rubberband_new(t_symbol *s, short argc, t_atom *argv);
void			buf_rubberband_free(t_buf_rubberband *x);
void			buf_rubberband_bang(t_buf_rubberband *x);
void			buf_rubberband_anything(t_buf_rubberband *x, t_symbol *msg, long ac, t_atom *av);

void buf_rubberband_assist(t_buf_rubberband *x, void *b, long m, long a, char *s);
void buf_rubberband_inletinfo(t_buf_rubberband *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(rubberband)

/**********************************************************************/
// Class Definition and Life Cycle




void C74_EXPORT ext_main(void* moduleRef)
{
    common_symbols_init();
    llllobj_common_symbols_init();
    
    if (llllobj_check_version(bach_get_current_llll_version()) || llllobj_test()) {
        ears_error_bachcheck();
        return 1;
    }
    
    t_class *c;
    
    CLASS_NEW_CHECK_SIZE(c, "ears.rubberband~",
                         (method)buf_rubberband_new,
                         (method)buf_rubberband_free,
                         sizeof(t_buf_rubberband),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    // @method list/llll @digest Process buffers
    // @description A list or llll with buffer names will trigger the buffer processing and output the processed
    // buffer names (depending on the <m>naming</m> attribute).
    EARSBUFOBJ_DECLARE_COMMON_METHODS_HANDLETHREAD(rubberband)

    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_blocking_attr(c);
    earsbufobj_class_add_naming_attr(c);
    earsbufobj_class_add_timeunit_attr(c);
    earsbufobj_class_add_antimeunit_attr(c);
    earsbufobj_class_add_envtimeunit_attr(c);
    earsbufobj_class_add_pitchunit_attr(c);
    earsbufobj_class_add_slopemapping_attr(c);

    earsbufobj_class_add_polyout_attr(c);

    CLASS_ATTR_LONG(c, "transients", 0, t_buf_rubberband, e_transients);
    CLASS_ATTR_STYLE_LABEL(c,"transients",0,"enumindex","Transient Type");
    CLASS_ATTR_ENUMINDEX(c,"transients", 0, "Crisp Mixed Smooth");
    CLASS_ATTR_BASIC(c, "transients", 0);
    CLASS_ATTR_DEFAULT(c, "transients", 0, "2");
    // @description Sets the transient mode, to control the component frequency
    // phase-reset mechanism that
    // may be used at transient points to provide clarity and realism to percussion
    // and other significant transient sounds. Options are: <br />
    // - <b>Crisp</b> (0): Reset component phases at the
    //   peak of each transient (the start of a significant note or
    //   percussive event).  This, the default setting, usually
    //   results in a clear-sounding output; but it is not always
    //   consistent, and may cause interruptions in stable sounds
    //   present at the same time as transient events.  The
    //   <m>detector</m> attribute can be used to tune this to some
    //   extent. <br />
    // - <b>Mixed</b> (1): Reset component phases at the
    //   peak of each transient, outside a frequency range typical of
    //   musical fundamental frequencies.  The results may be more
    //   regular for mixed stable and percussive notes than
    //   "Crisp", but with a "phasier" sound.  The
    //   balance may sound very good for certain types of music and
    //   fairly bad for others. <br />
    // - <b>Smooth</b> (2, default): Do not reset component phases
    //   at any point.  The results will be smoother and more regular
    //   but may be less clear than with either of the other
    //   transients flags.

    
    CLASS_ATTR_LONG(c, "detector", 0, t_buf_rubberband, e_detector);
    CLASS_ATTR_STYLE_LABEL(c,"detector",0,"enumindex","Transient Detection Algorithm");
    CLASS_ATTR_ENUMINDEX(c,"detector", 0, "Compound Percussive Soft");
    CLASS_ATTR_DEFAULT(c, "detector", 0, "0");
    // @description Controls the type of
    // transient detector used.  These options may be changed
    // after construction when running in real-time mode, but not when
    // running in offline mode. Options are: <br />
    // - <b>Compound</b> (0, default): Use a general-purpose
    //   transient detector which is likely to be good for most
    //   situations. <br />
    // - <b>Percussive</b> (1): Detect percussive
    //   transients.  Note that this was the default and only option
    //   in Rubber Band versions prior to 1.5. <br />
    // - <b>Soft</b> (2): Use an onset detector with less
    //   of a bias toward percussive transients.  This may give better
    //   results with certain material (e.g. relatively monophonic
    //   piano music).
    
    CLASS_ATTR_LONG(c, "stretchmode", 0, t_buf_rubberband, e_stretchmode);
    CLASS_ATTR_STYLE_LABEL(c,"stretchmode",0,"enumindex","Stretch Mode");
    CLASS_ATTR_ENUMINDEX(c,"stretchmode", 0, "Elastic Precise");
    CLASS_ATTR_DEFAULT(c, "stretchmode", 0, "0");
    // @description Controls the profile used for
    // variable timestretching.  Rubber Band always adjusts the
    // stretch profile to minimise stretching of busy broadband
    // transient sounds, but the degree to which it does so is
    // adjustable.  Options are: <br />
    // - <b>Elastic</b> (0, default): The audio will be
    //   stretched at a variable rate, aimed at preserving the quality
    //   of transient sounds as much as possible.  The timings of low
    //   activity regions between transients may be less exact than
    //   when the precise flag is set. <br />
    // - <b>Precise</b> (1): Although still using a variable
    //   stretch rate, the audio will be stretched so as to maintain
    //   as close as possible to a linear stretch ratio throughout.
    //   Timing may be better than when using \c OptionStretchElastic, at
    //   slight cost to the sound quality of transients.

    CLASS_ATTR_LONG(c, "phase", 0, t_buf_rubberband, e_phase);
    CLASS_ATTR_STYLE_LABEL(c,"phase",0,"enumindex","Phase Adjustment");
    CLASS_ATTR_ENUMINDEX(c,"phase", 0, "Elastic Precise");
    CLASS_ATTR_DEFAULT(c, "phase", 0, "0");
    // @description Controls the adjustment of
    // component frequency phases from one analysis window to the next
    // during non-transient segments. Options are: <br />
    // - <b>Laminar</b> (0, default): Adjust phases when stretching in
    //   such a way as to try to retain the continuity of phase
    //   relationships between adjacent frequency bins whose phases
    //   are behaving in similar ways.  This, the default setting,
    //   should give good results in most situations. <br />
    // - <b>Independent</b> (1): Adjust the phase in each
    //  frequency bin independently from its neighbours.  This
    //   usually results in a slightly softer, phasier sound.

    CLASS_ATTR_LONG(c, "fftwin", 0, t_buf_rubberband, e_fft_size);
    CLASS_ATTR_STYLE_LABEL(c,"fftwin",0,"enumindex","FFT Windows Size");
    CLASS_ATTR_ENUMINDEX(c,"fftwin", 0, "Short Standard Long");
    CLASS_ATTR_DEFAULT(c, "fftwin", 0, "0");
    // @description Controls the window size for
    // FFT processing. The window size actually used will depend on
    // many factors, but it can be influenced.  Options are: <br />
    // - <b>Short</b> (0): Use a shorter window.  This may
    //   result in crisper sound for audio that depends strongly on
    //   its timing qualities. <br />
    // - <b>Standard</b> (1, default): Use the default window size.
    //   The actual size will vary depending on other parameters.
    //   This option is expected to produce better results than the
    //   other window options in most situations.
    // - <b>Long</b> (2): Use a longer window.  This is
    //   likely to result in a smoother sound at the expense of
    //   clarity and timing.

    
    CLASS_ATTR_LONG(c, "smoothing", 0, t_buf_rubberband, e_smoothing);
    CLASS_ATTR_STYLE_LABEL(c,"smoothing",0,"onoff","Smoothing");
    CLASS_ATTR_DEFAULT(c, "smoothing", 0, "0");
    // @description Controls the use of
    // window-presum FFT and time-domain smoothing. By default it is off.
    // If it is on, the algorithm Use time-domain smoothing.  This
    // will result in a softer sound with some audible artifacts
    // around sharp transients, but it may be appropriate for longer
    // stretches of some instruments and can mix well with
    // short <m>fftwin</m>
    
    
    CLASS_ATTR_LONG(c, "formants", 0, t_buf_rubberband, e_formant);
    CLASS_ATTR_STYLE_LABEL(c,"formants",0,"onoff","Preserve Formants");
    CLASS_ATTR_DEFAULT(c, "formants", 0, "0");
    // @description Controls the handling of formant shape
    // (spectral envelope) when pitch-shifting.
    // If off (default), it applies no special formant
    // processing.  The spectral envelope will be pitch shifted as normal.
    // If on, it preserves the spectral
    // envelope of the unshifted signal.  This permits shifting the
    // note frequency without so substantially affecting the
    //  perceived pitch profile of the voice or instrument.

    
    CLASS_ATTR_LONG(c, "pitchmode", 0, t_buf_rubberband, e_pitchmode);
    CLASS_ATTR_STYLE_LABEL(c,"pitchmode",0,"enumindex","Pitch Shift Mode");
    CLASS_ATTR_ENUMINDEX(c,"pitchmode", 0, "High Speed High Quality High Consistency");
    CLASS_ATTR_DEFAULT(c, "pitchmode", 0, "2");
    // @description Control the method used for
    // pitch shifting.  Options are: <br />
    // - <b>High Speed</b> (0): Use a method with a CPU cost
    //   that is relatively moderate and predictable.  This may
    //   sound less clear than OptionPitchHighQuality, especially
    //   for large pitch shifts. <br />
    // - <b>High Quality</b> (1): Use the highest quality
    //   method for pitch shifting.  This method has a CPU cost
    //   approximately proportional to the required frequency shift.
    // - <b>High Consistency</b> (2, default): Use the method that gives
    //   greatest consistency when used to create small variations in
    //   pitch around the 1.0-ratio level.  Unlike the previous two
    //   options, this avoids discontinuities when moving across the
    //   1.0 pitch scale in real-time; it also consumes more CPU than
    //   the others in the case where the pitch scale is exactly 1.0.
    
    
    CLASS_ATTR_LONG(c, "blocksize", 0, t_buf_rubberband, e_blocksize);
    CLASS_ATTR_STYLE_LABEL(c,"blocksize",0,"text","Block Size");
    CLASS_ATTR_DEFAULT(c, "blocksize", 0, "1024");
    // @description Sets the block size for granular processes such as envelopes.
    // The unit depends on the <m>antimeunit</m> attribute.

    
    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
    return 0;
}

void buf_rubberband_assist(t_buf_rubberband *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        if (a == 0)
            sprintf(s, "symbol/list/llll: Incoming Buffer Names"); // @in 0 @type symbol/list/llll @digest Incoming buffer names
        else if (a == 1)
            // @in 1 @type float/list/llll @digest Stretch factor or envelope
            // @description Sets the stretch factor, either as a single number or as an llll
            // containing an envelope in the form <b>[[<m>x</m> <m>factor</m> <m>slope</m>] [<m>x</m> <m>factor</m> <m>slope</m>]...]</b>.
            // where <m>x</m> values' range depends on the <m>envtimeunit</m> attribute.
            sprintf(s, "float/llll: Stretch Factor or Envelope");
        else if (a == 2)
            // @in 2 @type float/list/llll @digest Pitch shift amount or envelope
            // @description Sets the pitch shift (unit defined via the <m>pitchunit</m> attribute), either as a single number or as an llll
            // containing an envelope in the form <b>[[<m>x</m> <m>shift</m> <m>slope</m>] [<m>x</m> <m>shift</m> <m>slope</m>]...]</b>,
            // where <m>x</m> values' range depends on the <m>envtimeunit</m> attribute.
            sprintf(s, "float/llll: Pitch Shift Amount or Envelope");
    } else {
        sprintf(s, "symbol/list: Output Buffer Names"); // @out 0 @type symbol/list @digest Output buffer names
    }
}

void buf_rubberband_inletinfo(t_buf_rubberband *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_rubberband *buf_rubberband_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_rubberband *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_rubberband*)object_alloc_debug(s_tag_class);
    if (x) {
        // @arg 0 @name outnames @optional 1 @type symbol
        // @digest Output buffer names
        // @description @copy EARS_DOC_OUTNAME_ATTR

        x->e_transients = 2;
        x->e_phase = 0;
        x->e_fft_size = 1;
        x->e_stretchmode = 0;
        x->e_detector = 0;
        x->e_smoothing = 0;
        x->e_formant = 0;
        x->e_pitchmode = 2;
        x->e_blocksize = 1024;
        
        earsbufobj_init((t_earsbufobj *)x,  EARSBUFOBJ_FLAG_SUPPORTS_COPY_NAMES);
        
        // @arg 1 @name stretch_factor @type float/list/llll
        // @digest Stretch factor or envelope
        // @description Sets the stretch factor, either as a single number or as an llll
        // containing an envelope in the form <b>[[<m>x</m> <m>factor</m> <m>slope</m>] [<m>x</m> <m>factor</m> <m>slope</m>]...]</b>.
        // where <m>x</m> values' range depends on the <m>envtimeunit</m> attribute.
        
        x->e_timestretch_env = llll_from_text_buf("1.");

        // @arg 2 @name pitch_shift_amount @type float/list/llll
        // @digest Pitch shift amount or envelope
        // @description Sets the pitch shift (unit defined via the <m>pitchunit</m> attribute), either as a single number or as an llll
        // containing an envelope in the form <b>[[<m>x</m> <m>shift</m> <m>slope</m>] [<m>x</m> <m>shift</m> <m>slope</m>]...]</b>,
        // where <m>x</m> values' range depends on the <m>envtimeunit</m> attribute.
        
        x->e_pitchshift_env = llll_from_text_buf("1.");

        x->e_ob.l_timeunit = EARS_TIMEUNIT_DURATION_RATIO;

        t_llll *args = llll_parse(true_ac, argv);
        t_llll *names = earsbufobj_extract_names_from_args((t_earsbufobj *)x, args);
       
        if (args && args->l_head) {
            llll_clear(x->e_timestretch_env);
            llll_appendhatom_clone(x->e_timestretch_env, &args->l_head->l_hatom);
            if (args->l_head->l_next) {
                llll_clear(x->e_pitchshift_env);
                llll_appendhatom_clone(x->e_pitchshift_env, &args->l_head->l_next->l_hatom);
            }
        }

        attr_args_process(x, argc, argv);

        earsbufobj_setup((t_earsbufobj *)x, "E44", "E", names);

        llll_free(args);
        llll_free(names);
        
    }
    return x;
}


void buf_rubberband_free(t_buf_rubberband *x)
{
    llll_free(x->e_timestretch_env);
    llll_free(x->e_pitchshift_env);
    earsbufobj_free((t_earsbufobj *)x);
}

RubberBand::RubberBandStretcher::Options buf_rubberband_get_options(t_buf_rubberband *x)
{
    RubberBand::RubberBandStretcher::Options options = 0; 

    switch (x->e_pitchmode) {
        case 0:
            options |= RubberBand::RubberBandStretcher::OptionPitchHighSpeed;
            break;

        case 1:
            options |= RubberBand::RubberBandStretcher::OptionPitchHighQuality;
            break;

        case 2:
            options |= RubberBand::RubberBandStretcher::OptionPitchHighConsistency;
            break;

        default:
            options |= RubberBand::RubberBandStretcher::OptionPitchHighSpeed;
            break;
    }

    
    switch (x->e_stretchmode) {
        case 0:
            options |= RubberBand::RubberBandStretcher::OptionStretchElastic;
            break;
            
        default:
            options |= RubberBand::RubberBandStretcher::OptionStretchPrecise;
            break;
    }

    switch (x->e_transients) {
        case 0:
            options |= RubberBand::RubberBandStretcher::OptionTransientsCrisp;
            break;
            
        case 1:
            options |= RubberBand::RubberBandStretcher::OptionTransientsMixed;
            break;

        default:
            options |= RubberBand::RubberBandStretcher::OptionTransientsSmooth;
            break;
    }

    switch (x->e_detector) {
        case 0:
            options |= RubberBand::RubberBandStretcher::OptionDetectorCompound;
            break;
            
        case 1:
            options |= RubberBand::RubberBandStretcher::OptionDetectorPercussive;
            break;
            
        default:
            options |= RubberBand::RubberBandStretcher::OptionDetectorSoft;
            break;
    }

    switch (x->e_phase) {
        case 0:
            options |= RubberBand::RubberBandStretcher::OptionPhaseLaminar;
            break;
            
        default:
            options |= RubberBand::RubberBandStretcher::OptionPhaseIndependent;
            break;
    }
    
    if (x->e_formant)
        options |= RubberBand::RubberBandStretcher::OptionFormantPreserved;
    else
        options |= RubberBand::RubberBandStretcher::OptionFormantShifted;

    options |= RubberBand::RubberBandStretcher::OptionThreadingNever;

    switch (x->e_fft_size) {
        case 0:
            options |= RubberBand::RubberBandStretcher::OptionWindowShort;
            break;
        case 1:
            options |= RubberBand::RubberBandStretcher::OptionWindowStandard;
            break;

        default:
            options |= RubberBand::RubberBandStretcher::OptionWindowLong;
            break;
    }
    
    return options;
}


void buf_rubberband_bang(t_buf_rubberband *x)
{
    long num_buffers = earsbufobj_get_instore_size((t_earsbufobj *)x, 0);
    
    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_buffers, true);
    
    earsbufobj_mutex_lock((t_earsbufobj *)x);
    earsbufobj_init_progress((t_earsbufobj *)x, num_buffers);

    t_llllelem *ts_el = x->e_timestretch_env->l_head;
    t_llllelem *ps_el = x->e_pitchshift_env->l_head;

    for (long count = 0; count < num_buffers; count++,
         ts_el = ts_el && ts_el->l_next ? ts_el->l_next : ts_el, ps_el = ps_el && ps_el->l_next ? ps_el->l_next : ps_el) {
        t_buffer_obj *in = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, count);
        t_buffer_obj *out = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, count);

        t_llll *ts_env = earsbufobj_time_llllelem_to_relative_and_samples((t_earsbufobj *)x, ts_el, in);
        t_llll *ps_env = earsbufobj_pitch_llllelem_to_cents_and_samples((t_earsbufobj *)x, ps_el, in);

        if (ts_env->l_size == 0) {
            object_error((t_object *)x, "No time stretch defined.");
            if (in != out)
                ears_buffer_clone((t_object *)x, in, out);
        } else if (ps_env->l_size == 0) {
            object_error((t_object *)x, "No pitch shift defined.");
            if (in != out)
                ears_buffer_clone((t_object *)x, in, out);
        } else {
            ears_buffer_rubberband((t_object *)x, in, out, ts_env, ps_env, buf_rubberband_get_options(x), earsbufobj_time_to_samps((t_earsbufobj *)x, x->e_blocksize, in, EARSBUFOBJ_CONVERSION_FLAG_ISANALYSIS), earsbufobj_get_slope_mapping((t_earsbufobj *)x), x->e_ob.l_timeunit != EARS_TIMEUNIT_DURATION_RATIO);
        }
        
        llll_free(ts_env);
        llll_free(ps_env);
        
        if (earsbufobj_iter_progress((t_earsbufobj *)x, count, num_buffers)) break;
    }
    earsbufobj_mutex_unlock((t_earsbufobj *)x);
    
    earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
}


void buf_rubberband_anything(t_buf_rubberband *x, t_symbol *msg, long ac, t_atom *av)
{
    long inlet = earsbufobj_proxy_getinlet((t_earsbufobj *) x);

    t_llll *parsed = earsbufobj_parse_gimme((t_earsbufobj *) x, LLLL_OBJ_VANILLA, msg, ac, av);
    if (!parsed) return;
    
    if (parsed && parsed->l_head) {
        if (inlet == 0) {
            long num_bufs = llll_get_num_symbols_root(parsed);

            earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_bufs, true);
            earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_OUT, 0, num_bufs, true);
            
            earsbufobj_store_buffer_list((t_earsbufobj *)x, parsed, 0);
            
            buf_rubberband_bang(x);
            
        } else if (inlet == 1) {
            earsbufobj_mutex_lock((t_earsbufobj *)x);
            llll_free(x->e_timestretch_env);
            x->e_timestretch_env = llll_clone(parsed);
            earsbufobj_mutex_unlock((t_earsbufobj *)x);

        } else if (inlet == 2) {
            earsbufobj_mutex_lock((t_earsbufobj *)x);
            llll_free(x->e_pitchshift_env);
            x->e_pitchshift_env = llll_clone(parsed);
            earsbufobj_mutex_unlock((t_earsbufobj *)x);
        }
    }
    llll_free(parsed);
}


