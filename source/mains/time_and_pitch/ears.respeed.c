/**
	@file
	ears.respeed.c
 
	@name
	ears.respeed~
 
	@realname
	ears.respeed~
 
	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	Vary buffer playback speed
 
	@description
	Transposes and time-stretches the buffer content by resampling it.
 
	@discussion
 
	@category
	ears time and pitch
 
	@keywords
	buffer, respeed, property, change, vinyl
 
	@seealso
	ears.rubberband~, ears.soundtouch~
	
	@owner
	Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "foundation/llllobj.h"
#include "foundation/llll_commons_ext.h"
#include "math/bach_math_utilities.h"
#include "ears.object.h"



typedef struct _buf_respeed {
    t_earsbufobj        e_ob;
    
    t_llll             *e_pitchshift_env;
    t_llll             *e_timestretch_env;
    
    double              e_derivative_sampling_rate; // sampling rate in case a derivative needs to be computed
} t_buf_respeed;




// Prototypes
t_buf_respeed*         buf_respeed_new(t_symbol *s, short argc, t_atom *argv);
void			buf_respeed_free(t_buf_respeed *x);
void			buf_respeed_bang(t_buf_respeed *x);
void			buf_respeed_anything(t_buf_respeed *x, t_symbol *msg, long ac, t_atom *av);

void buf_respeed_assist(t_buf_respeed *x, void *b, long m, long a, char *s);
void buf_respeed_inletinfo(t_buf_respeed *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(respeed)

/**********************************************************************/
// Class Definition and Life Cycle


void C74_EXPORT ext_main(void* moduleRef)
{
    common_symbols_init();
    llllobj_common_symbols_init();
    
    if (llllobj_check_version(bach_get_current_llll_version()) || llllobj_test()) {
        ears_error_bachcheck();
        return;
    }
    
    t_class *c;
    
    CLASS_NEW_CHECK_SIZE(c, "ears.respeed~",
                         (method)buf_respeed_new,
                         (method)buf_respeed_free,
                         sizeof(t_buf_respeed),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    // @method list/llll @digest Process buffers
    // @description A list or llll with buffer names will trigger the buffer processing and output the processed
    // buffer names (depending on the <m>alloc</m> attribute).
    EARSBUFOBJ_DECLARE_COMMON_METHODS_HANDLETHREAD(respeed)
    
    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_nativeout_attr(c);
    earsbufobj_class_add_blocking_attr(c);
    earsbufobj_class_add_alloc_attr(c);
    earsbufobj_class_add_slopemapping_attr(c);
    earsbufobj_class_add_antimeunit_attr(c);
    earsbufobj_class_add_timeunit_attr(c);
    earsbufobj_class_add_envtimeunit_attr(c);
    earsbufobj_class_add_pitchunit_attr(c);

    earsbufobj_class_add_resamplingfiltersize_attr(c);
    earsbufobj_class_add_resamplingmode_attr(c);
    earsbufobj_class_add_polyout_attr(c);
    
    CLASS_ATTR_BASIC(c, "resamplingfiltersize", 0);
    CLASS_ATTR_BASIC(c, "resamplingmode", 0);
    
    
    CLASS_ATTR_DOUBLE(c, "derivativesr", 0, t_buf_respeed, e_derivative_sampling_rate);
    CLASS_ATTR_STYLE_LABEL(c, "derivativesr",0,"text","Derivative Sampling Rate");
    // @description Sets a sampling rate for the computation of derivatives.
    // Only useful when envelopes with absolute time points are input
    

    earsbufobj_class_add_fileusage_method(c);
    
    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
}

void buf_respeed_assist(t_buf_respeed *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        if (a == 0)
            sprintf(s, "symbol/list/llll: Incoming Buffer Names"); // @in 0 @type symbol/list/llll @digest Incoming buffer names
        else if (a == 1)
            sprintf(s, "float/list/llll: Resampling Factors or Envelopes"); // @in 1 @type float/list/llll @digest Resampling factors or envelopes
        else
            sprintf(s, "float/list/llll: Transposition Intervals or Envelopes"); // @in 2 @type float/list/llll @digest Transposition intervals or envelopes
    } else {
        sprintf(s, "symbol/list: Processed Buffer Names"); // @out 0 @type symbol/list @digest respeedd buffer names
    }
}

void buf_respeed_inletinfo(t_buf_respeed *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_respeed *buf_respeed_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_respeed *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_respeed*)object_alloc_debug(s_tag_class);
    if (x) {
        earsbufobj_init((t_earsbufobj *)x, EARSBUFOBJ_FLAG_SUPPORTS_COPY_NAMES);

        // @arg 0 @name outnames @optional 1 @type symbol
        // @digest Output buffer names
        // @description @copy EARS_DOC_OUTNAME_ATTR


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
        
        x->e_pitchshift_env = llll_from_text_buf("0."); // in cents by default

        x->e_derivative_sampling_rate = EARS_DEFAULT_DERIVATIVE_SAMPLING_RATE;

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


void buf_respeed_free(t_buf_respeed *x)
{
    llll_free(x->e_timestretch_env);
    llll_free(x->e_pitchshift_env);
    earsbufobj_free((t_earsbufobj *)x);
}



void buf_respeed_bang(t_buf_respeed *x)
{
    long num_buffers = earsbufobj_get_instore_size((t_earsbufobj *)x, 0);
    e_ears_resamplingmode mode = x->e_ob.l_resamplingmode;
    
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
        double sr_in = ears_buffer_get_sr((t_object *)x, in);
        
        long window_width_samples = earsbufobj_time_to_samps((t_earsbufobj *)x, x->e_ob.l_resamplingfilterwidth, in, EARSBUFOBJ_CONVERSION_FLAG_ISANALYSIS);
        
        if (in != out)
            ears_buffer_clone((t_object *)x, in, out);
        
        
        if (hatom_gettype(&ts_el->l_hatom) != H_LLLL && hatom_gettype(&ps_el->l_hatom) != H_LLLL) {
            // simple case: just numbers
            double ts = hatom_getdouble(&ts_el->l_hatom);
            double ps = hatom_getdouble(&ps_el->l_hatom);
            
            double factor = 1;
            factor /= ears_cents_to_ratio(earsbufobj_pitch_to_cents((t_earsbufobj *)x, ps));
            factor *= earsbufobj_time_to_durationratio((t_earsbufobj *)x, ts, in);
            
            if (factor < 0) {
                ears_buffer_rev_inplace((t_object *)x, out);
                factor *= -1;
            }
            
            if (factor == 0) {
                object_error((t_object *)x, "Resampling factor cannot be zero.");
            } else if (factor != 1) {
                ears_buffer_resample((t_object *)x, out, factor, window_width_samples, mode);
            }
            
        } else if (hatom_gettype(&ts_el->l_hatom) == H_LLLL && hatom_gettype(&ps_el->l_hatom) == H_LLLL) {
            object_error((t_object *)x, "The object does not support defining two separate envelopes for time and pitch. Only use either one of them.");
        } else {

            t_llll *env;
            t_double_func remap_fn = NULL;
            if (hatom_gettype(&ps_el->l_hatom) == H_LLLL) {
                switch (x->e_ob.l_pitchunit) {
                    case EARS_PITCHUNIT_CENTS:
                    case EARS_PITCHUNIT_MIDI:
                    case EARS_PITCHUNIT_HERTZ:
                        env = earsbufobj_pitch_llllelem_to_cents_and_samples((t_earsbufobj *)x, ps_el, in);
                        remap_fn = ears_cents_to_invratio;
                        break;
                    
                    case EARS_PITCHUNIT_FREQRATIO:
                    default:
                        env = earsbufobj_llllelem_to_env_samples((t_earsbufobj *)x, ps_el, in);
                        remap_fn = ears_reciprocal;
                        break;
                }
            } else {
                env = earsbufobj_time_llllelem_to_relative_and_samples((t_earsbufobj *)x, ts_el, in, x->e_derivative_sampling_rate);
                remap_fn = NULL;
            }
            
            // check if envelope crosses zero or is constantly negative (and hence needs reverse)
            t_ears_envelope_iterator eei = ears_envelope_iterator_create(env, 1., remap_fn, earsbufobj_get_slope_mapping((t_earsbufobj *)x));
            double min_y = ears_envelope_iterator_get_min_y(&eei);
            double max_y = ears_envelope_iterator_get_max_y(&eei);
            
            if (mode != EARS_RESAMPLINGMODE_SINC) {
                object_warn((t_object *)x, "Only sinc interpolation is supported with envelopes. Defaulting to sinc.");
            }
            
            // (incidentally, the remapping function is assumed to be increasing monotonically)
            
            if (min_y == 0 || max_y == 0) {
                object_error((t_object *)x, "Resampling envelopes cannot touch zero.");
            } else if (min_y * max_y < 0) {
                object_error((t_object *)x, "Resampling envelopes cannot cross zero.");
            } else if (min_y < 0 && max_y < 0) {
                ears_buffer_rev_inplace((t_object *)x, out);
                for (t_llllelem *el = env->l_head; el; el = el->l_next) {
                    t_llll *ll = hatom_getllll(&el->l_hatom);
                    if (ll && ll->l_size >= 2 && is_hatom_number(&ll->l_head->l_next->l_hatom))
                        hatom_setdouble(&ll->l_head->l_next->l_hatom, hatom_getdouble(&ll->l_head->l_next->l_hatom) * -1);
                }
                ears_buffer_resample_envelope((t_object *)x, out, env, window_width_samples, remap_fn, earsbufobj_get_slope_mapping((t_earsbufobj *)x));
            } else {
                ears_buffer_resample_envelope((t_object *)x, out, env, window_width_samples, remap_fn, earsbufobj_get_slope_mapping((t_earsbufobj *)x));
            }
            
            llll_free(env);
        }

        if (earsbufobj_iter_progress((t_earsbufobj *)x, count, num_buffers)) break;
    }
    earsbufobj_mutex_unlock((t_earsbufobj *)x);
    
    earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
}


void buf_respeed_anything(t_buf_respeed *x, t_symbol *msg, long ac, t_atom *av)
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
            
            buf_respeed_bang(x);
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


