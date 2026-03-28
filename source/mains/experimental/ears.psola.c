/**
	@file
	ears.psola.c
 
	@name
	ears.psola~
 
	@realname
	ears.psola~
 
	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	Pitch-synchronous overlap add
 
	@description
	Performs PSOLA synthesis starting from a buffer waveform
 
	@discussion
 
	@category
	ears distorsion
 
	@keywords
	buffer, psola, pitch, overlap, add
 
	@seealso
	ears.roll.synthesis~
	
	@owner
	Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "foundation/llllobj.h"
#include "foundation/llll_commons_ext.h"
#include "math/bach_math_utilities.h"
#include "ears.object.h"



typedef struct _buf_psola {
    t_earsbufobj       e_ob;
    
    double             input_offset;
    double             duration;
    t_llll             *pitch;
    double             grain_duration_factor;
    double             stride_duration_factor;
    
    
    char    compensate_pitch_for_stride;
    double  highpass_cutoff;
    long    oversampling;
    
} t_buf_psola;



// Prototypes
t_buf_psola*         buf_psola_new(t_symbol *s, short argc, t_atom *argv);
void			buf_psola_free(t_buf_psola *x);
void			buf_psola_bang(t_buf_psola *x);
void			buf_psola_anything(t_buf_psola *x, t_symbol *msg, long ac, t_atom *av);

void buf_psola_assist(t_buf_psola *x, void *b, long m, long a, char *s);
void buf_psola_inletinfo(t_buf_psola *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(psola)

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
    
    CLASS_NEW_CHECK_SIZE(c, "ears.psola~",
                         (method)buf_psola_new,
                         (method)buf_psola_free,
                         sizeof(t_buf_psola),
                         (method)NULL,
                         A_GIMME,
                         0L);

    // @method list/llll @digest Function depends on inlet
    // @description A list or llll in the first inlet is supposed to contain buffer names and will
    // trigger the buffer processing and output the processed buffer names (depending on the <m>alloc</m> attribute). <br />
    // An llll in the second inlet is expected to contain a pitch envelope (depending on the <m>pitchunit</m> and <m>envtimeunit</m>).
    EARSBUFOBJ_DECLARE_COMMON_METHODS_HANDLETHREAD(psola)

    // @method number @digest Function depends on inlet
    // @description A number in the second inlet is expected to to contain a pitch value (depending on the <m>pitchunit</m>). <br />
    // A number in the third inlet is expected to contain the duration (depending on the <m>timeunit</m>). <br />
    // A number in the fourth inlet is expected to contain the grain duration (as a ratio w.r.t. the fundamental period defined
    // by the pitch). <br />
    // A number in the fifth inlet is expected to contain the stride amount (as a ratio w.r.t. the fundamental period defined
    // by the pitch). <br />


    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_nativeout_attr(c);
    earsbufobj_class_add_blocking_attr(c);
    earsbufobj_class_add_timeunit_attr(c);
    earsbufobj_class_add_envtimeunit_attr(c);
    earsbufobj_class_add_alloc_attr(c);
    earsbufobj_class_add_slopemapping_attr(c);
    earsbufobj_class_add_polyout_attr(c);
    earsbufobj_class_add_pitchunit_attr(c);

    earsbufobj_class_add_resamplingpolicy_attr(c);
    earsbufobj_class_add_resamplingfiltersize_attr(c);
    earsbufobj_class_add_resamplingmode_attr(c);

    CLASS_ATTR_LONG(c, "oversampling", 0, t_buf_psola, oversampling);
    CLASS_ATTR_STYLE_LABEL(c,"oversampling",0,"text","Oversampling");
    CLASS_ATTR_CATEGORY(c, "oversampling", 0, "Resampling");
    CLASS_ATTR_BASIC(c, "oversampling", 0);
    // @description Sets the oversampling factor for PSOLA processing.
    

    CLASS_ATTR_CHAR(c, "compensate", 0, t_buf_psola, compensate_pitch_for_stride);
    CLASS_ATTR_STYLE_LABEL(c,"compensate",0,"onoff","Compensate Pitch for Stride");
    CLASS_ATTR_BASIC(c, "compensate", 0);
    // @description Toggles pitch compensation due to stride shift.
    

    CLASS_ATTR_DOUBLE(c, "highpassfreq", 0, t_buf_psola, highpass_cutoff);
    CLASS_ATTR_STYLE_LABEL(c,"highpassfreq",0,"text","Highpass Cutoff Frequency");
    // @description Sets the highpass cutoff frequency (0 = none). Defaults to 10.
    
    
    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
}

void buf_psola_assist(t_buf_psola *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        switch (a) {
            case 0:
                sprintf(s, "symbol/list/llll: Incoming Buffer Names"); 
                // @in 0 @type symbol/list/llll @digest Incoming buffer names
                break;
                
            case 1:
                sprintf(s, "number/llll: Pitch %s", ears_pitchunit_to_abbrev((e_ears_pitchunit)x->e_ob.l_pitchunit));
                // @in 1 @type number @digest Target pitch or pitch envelope
                break;

            case 2:
                sprintf(s, "number: Duration %s", ears_timeunit_to_abbrev((e_ears_timeunit)x->e_ob.l_timeunit));
                // @in 2 @type number @digest Total duration
                break;

            case 3:
                sprintf(s, "number: Grain Duration Factor");
                // @in 3 @type number @digest Grain duration factor (1 corresponding to fundamental period)
                break;

            case 4:
                sprintf(s, "number: Stride Amount");
                // @in 4 @type number @digest Stride amount as fraction of the period
                break;

            default:
                break;
        }
    } else {
        sprintf(s, "symbol/list: Output Buffer Names"); // @out 0 @type symbol/list @digest Output buffer names
    }
}

void buf_psola_inletinfo(t_buf_psola *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_psola *buf_psola_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_psola *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_psola*)object_alloc_debug(s_tag_class);
    if (x) {
        x->pitch = llll_from_text_buf("6000");
        x->duration = 1000;
        x->grain_duration_factor = 2.;
        x->stride_duration_factor = 0.;
        x->oversampling = 1;
        x->compensate_pitch_for_stride = true;
        x->highpass_cutoff = 10.;
        x->input_offset = 0;
        
        earsbufobj_init((t_earsbufobj *)x, EARSBUFOBJ_FLAG_SUPPORTS_COPY_NAMES);
        
        // @arg 0 @name outnames @optional 1 @type symbol
        // @digest Output buffer names
        // @description @copy EARS_DOC_OUTNAME_ATTR

        t_llll *args = llll_parse(true_ac, argv);
        t_llll *names = earsbufobj_extract_names_from_args((t_earsbufobj *)x, args);
        
        t_llllelem *arg_el = (args && args->l_head) ? args->l_head : NULL;
        if (arg_el) {
            
            // @arg 1 @name pitch @optional 1 @type number/llll
            // @digest Pitch or pitch envelope
            // @description Sets the pitch or pitch envelope (depending on the <m>pitchunit</m> and <m>envtimeunit</m>).
            
            llll_free(x->pitch);
            x->pitch = llll_get();
            if (hatom_gettype(&args->l_head->l_hatom) == H_LLLL)
                x->pitch = llll_clone(hatom_getllll(&args->l_head->l_hatom));
            else {
                x->pitch = llll_get();
                llll_appendhatom_clone(x->pitch, &args->l_head->l_hatom);
            }
            arg_el = arg_el->l_next;
            
            if (arg_el) {
                
                // @arg 2 @name duration @optional 1 @type number
                // @digest Duration
                // @description Sets the duration (depending on the <m>timeunit</m>).
                
                x->duration = hatom_getdouble(&arg_el->l_hatom);
                arg_el = arg_el->l_next;
                
                if (arg_el) {
                    
                    // @arg 3 @name relgrainduration @optional 1 @type number
                    // @digest Relative grain duration
                    // @description Sets the relative grain duration (as a ratio w.r.t. the fundamental period).
                    
                    x->grain_duration_factor = hatom_getdouble(&arg_el->l_hatom);
                    arg_el = arg_el->l_next;
                    
                    if (arg_el) {
                        
                        // @arg 4 @name relstrideamount @optional 1 @type number
                        // @digest Relative stride amount
                        // @description Sets the relative stride amount (as a ratio w.r.t. the fundamental period).

                        x->stride_duration_factor = hatom_getdouble(&arg_el->l_hatom);
                        arg_el = arg_el->l_next;
                    }
                }
            }
        }
            
        
        attr_args_process(x, argc, argv);
        
        earsbufobj_setup((t_earsbufobj *)x, "E4444", "E", names);

        llll_free(args);
        llll_free(names);
    }
    return x;
}


void buf_psola_free(t_buf_psola *x)
{
    llll_free(x->pitch);
    earsbufobj_free((t_earsbufobj *)x);
}



void buf_psola_bang(t_buf_psola *x)
{
    long num_buffers = earsbufobj_get_instore_size((t_earsbufobj *)x, 0);
    
    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_buffers, true);
    
    earsbufobj_mutex_lock((t_earsbufobj *)x);
    earsbufobj_init_progress((t_earsbufobj *)x, num_buffers);
    
    long oversampling = x->oversampling;
    
    t_llllelem *el = x->pitch->l_head;
    for (long count = 0; count < num_buffers; count++, el = el && el->l_next ? el->l_next : el) {
        t_buffer_obj *in = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, count);
        t_buffer_obj *out = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, count);
        double sr = ears_buffer_get_sr((t_object *)x, in);

        t_buffer_obj *in_wk;
        if (oversampling > 1) { // upsampling
            in_wk = ears_buffer_make(NULL); 
            ears_buffer_clone((t_object *)x, in, in_wk);
            ears_buffer_resample((t_object *)x, in_wk, oversampling, x->e_ob.l_resamplingfilterwidth);
            ears_buffer_set_sr((t_object *)x, in_wk, sr * oversampling);
        } else {
            in_wk = in;
        }
        
        long input_offset_samps = (long)round(earsbufobj_time_to_samps((t_earsbufobj *)x, x->input_offset, in_wk));
        double duration_samps = earsbufobj_time_to_samps((t_earsbufobj *)x, x->duration, in_wk); // may have been oversampled, so everything is ok
        
        t_llll *pitch_env = llll_get();
        llll_appendhatom_clone(pitch_env, &el->l_hatom);
        llll_flatten(pitch_env, 1, 0);
        ears_llll_to_env_samples(pitch_env, duration_samps, sr * oversampling, x->e_ob.l_envtimeunit);
        
        if (pitch_env->l_size == 0) {
            object_error((t_object *)x, "No pitch defined.");
            if (in != out)
                ears_buffer_clone((t_object *)x, in, out);
        } else if (pitch_env->l_depth == 1 && pitch_env->l_head) {
            // pitch is a single number
            ears_buffer_psola_envelope((t_object *)x, in_wk, out, pitch_env, duration_samps, x->grain_duration_factor, x->stride_duration_factor, earsbufobj_get_slope_mapping((t_earsbufobj *)x), x->compensate_pitch_for_stride, x->highpass_cutoff, input_offset_samps);
        } else {
            // psola is an envelope in llll form
            ears_buffer_psola_envelope((t_object *)x, in_wk, out, pitch_env, duration_samps, x->grain_duration_factor, x->stride_duration_factor, earsbufobj_get_slope_mapping((t_earsbufobj *)x), x->compensate_pitch_for_stride, x->highpass_cutoff, input_offset_samps);
        }
        
        if (oversampling > 1) { // downsampling
            ears_buffer_resample((t_object *)x, out, 1./oversampling, x->e_ob.l_resamplingfilterwidth);
            ears_buffer_set_sr((t_object *)x, out, sr);
            ears_buffer_free(in_wk);
        }
        
        llll_free(pitch_env);

        if (earsbufobj_iter_progress((t_earsbufobj *)x, count, num_buffers)) break;
    }
    earsbufobj_mutex_unlock((t_earsbufobj *)x);
    
    earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
}


void buf_psola_anything(t_buf_psola *x, t_symbol *msg, long ac, t_atom *av)
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
            
            buf_psola_bang(x);
            
        } else if (inlet == 1) {
            earsbufobj_mutex_lock((t_earsbufobj *)x);
            llll_free(x->pitch);
            x->pitch = llll_clone(parsed);
            earsbufobj_mutex_unlock((t_earsbufobj *)x);
        } else if (inlet == 2) {
            if (parsed->l_head) {
                earsbufobj_mutex_lock((t_earsbufobj *)x);
                x->duration = hatom_getdouble(&parsed->l_head->l_hatom);
                earsbufobj_mutex_unlock((t_earsbufobj *)x);
            }
        } else if (inlet == 3) {
            if (parsed->l_head) {
                earsbufobj_mutex_lock((t_earsbufobj *)x);
                x->grain_duration_factor = hatom_getdouble(&parsed->l_head->l_hatom);
                earsbufobj_mutex_unlock((t_earsbufobj *)x);
            }
        } else if (inlet == 4) {
            if (parsed->l_head) {
                earsbufobj_mutex_lock((t_earsbufobj *)x);
                x->stride_duration_factor = hatom_getdouble(&parsed->l_head->l_hatom);
                earsbufobj_mutex_unlock((t_earsbufobj *)x);
            }
        }
    }
    llll_free(parsed);
}


