/**
	@file
	ears.granulate.c
 
	@name
	ears.granulate~
 
	@realname
	ears.granulate~
 
	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	Off-line buffer granulation
 
	@description
	Performs granular synthesis starting from a buffer waveform
 
	@discussion
 
	@category
	ears granulation
 
	@keywords
	buffer, granulate, granular synthesis, grain
 
	@seealso
	ears.psola~
	
	@owner
	Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "foundation/llllobj.h"
#include "foundation/llll_commons_ext.h"
#include "math/bach_math_utilities.h"
#include "ears.object.h"



typedef struct _buf_granulate {
    t_earsbufobj       e_ob;
    
    t_llll             *output_duration;
    t_llll             *grain_interval;
    t_llll             *grain_size;
    t_llll             *grain_onset;
    t_llll             *grain_interval_jit;
    t_llll             *grain_size_jit;
    t_llll             *grain_onset_jit;
    t_llll             *change_pitch;
    t_llll             *change_rate;
} t_buf_granulate;



// Prototypes
t_buf_granulate*         buf_granulate_new(t_symbol *s, short argc, t_atom *argv);
void			buf_granulate_free(t_buf_granulate *x);
void			buf_granulate_bang(t_buf_granulate *x);
void			buf_granulate_anything(t_buf_granulate *x, t_symbol *msg, long ac, t_atom *av);

void buf_granulate_assist(t_buf_granulate *x, void *b, long m, long a, char *s);
void buf_granulate_inletinfo(t_buf_granulate *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(granulate)

DEFINE_LLLL_ATTR_DEFAULT_GETTER(t_buf_granulate, grain_interval_jit, buf_granulate_getattr_intervaljit);
DEFINE_LLLL_ATTR_DEFAULT_SETTER(t_buf_granulate, grain_interval_jit, buf_granulate_setattr_intervaljit);
DEFINE_LLLL_ATTR_DEFAULT_GETTER(t_buf_granulate, grain_onset_jit, buf_granulate_getattr_onsetjit);
DEFINE_LLLL_ATTR_DEFAULT_SETTER(t_buf_granulate, grain_onset_jit, buf_granulate_setattr_onsetjit);
DEFINE_LLLL_ATTR_DEFAULT_GETTER(t_buf_granulate, grain_size_jit, buf_granulate_getattr_sizejit);
DEFINE_LLLL_ATTR_DEFAULT_SETTER(t_buf_granulate, grain_size_jit, buf_granulate_setattr_sizejit);

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
    
    CLASS_NEW_CHECK_SIZE(c, "ears.granulate~",
                         (method)buf_granulate_new,
                         (method)buf_granulate_free,
                         sizeof(t_buf_granulate),
                         (method)NULL,
                         A_GIMME,
                         0L);

    // @method list/llll @digest Function depends on inlet
    // @description A list or llll in the first inlet is supposed to contain buffer names and will
    // trigger the buffer processing and output the processed buffer names (depending on the <m>alloc</m> attribute). <br />
    // An llll in the second inlet is expected to contain an onset envelope for starting grain positions
    // (depending on the <m>timeunit</m> and <m>envtimeunit</m>).
    EARSBUFOBJ_DECLARE_COMMON_METHODS_HANDLETHREAD(granulate)

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
    earsbufobj_class_add_wintype_attr(c, "Settings");

    CLASS_ATTR_LLLL(c, "intervaljit", 0, t_buf_granulate, grain_interval_jit, buf_granulate_getattr_intervaljit, buf_granulate_setattr_intervaljit);
    CLASS_ATTR_STYLE_LABEL(c,"intervaljit",0,"text","Grain Interval Jitter");
    // @description Sets the amount of grain interval jitter (in the <m>timeunit</m>).

    CLASS_ATTR_LLLL(c, "onsetjit", 0, t_buf_granulate, grain_onset_jit, buf_granulate_getattr_onsetjit, buf_granulate_setattr_onsetjit);
    CLASS_ATTR_STYLE_LABEL(c,"onsetjit",0,"text","Grain Onset Jitter");
    // @description Sets the amount of jitter on the initial position of the extracted grains with respect to the
    // original buffer (in the <m>timeunit</m>).

    CLASS_ATTR_LLLL(c, "sizejit", 0, t_buf_granulate, grain_size_jit, buf_granulate_getattr_sizejit, buf_granulate_setattr_sizejit);
    CLASS_ATTR_STYLE_LABEL(c,"sizejit",0,"text","Grain Size Jitter");
    // @description Sets the amount of jitter on the grain length (in the <m>timeunit</m>).


    
    earsbufobj_class_add_resamplingpolicy_attr(c);
    earsbufobj_class_add_resamplingfiltersize_attr(c);
    earsbufobj_class_add_resamplingmode_attr(c);
    
    earsbufobj_class_add_fileusage_method(c);

 class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
}

void buf_granulate_assist(t_buf_granulate *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        switch (a) {
            case 0:
                sprintf(s, "symbol/list/llll: Incoming Buffer Names"); 
                // @in 0 @type symbol/list/llll @digest Incoming buffer names
                break;
                
            case 1:
                sprintf(s, "number/llll: Output Duration %s", ears_timeunit_to_abbrev((e_ears_timeunit)x->e_ob.l_timeunit));
                // @in 1 @type number/llll @digest Duration of output buffer
                break;

            case 2:
                sprintf(s, "number/llll: Grain Interval %s", ears_timeunit_to_abbrev((e_ears_timeunit)x->e_ob.l_timeunit));
                // @in 2 @type number/llll @digest Temporal interval between grains
                break;

            case 3:
                sprintf(s, "number/llll: Grain Size %s", ears_timeunit_to_abbrev((e_ears_timeunit)x->e_ob.l_timeunit));
                // @in 3 @type number/llll @digest Grain size
                break;

            case 4:
                sprintf(s, "number/llll: Window Onset %s", ears_timeunit_to_abbrev((e_ears_timeunit)x->e_ob.l_timeunit));
                // @in 4 @type number/llll @digest Grain starting onset
                break;

            default:
                break;
        }
    } else {
        sprintf(s, "symbol/list: Output Buffer Names"); // @out 0 @type symbol/list @digest Output buffer names
    }
}

void buf_granulate_inletinfo(t_buf_granulate *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_granulate *buf_granulate_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_granulate *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_granulate*)object_alloc_debug(s_tag_class);
    if (x) {
        x->output_duration = llll_from_text_buf("10000");
        x->grain_interval = llll_from_text_buf("100");
        x->grain_size = llll_from_text_buf("400");
        x->grain_onset = llll_from_text_buf("0");
        x->grain_interval_jit = llll_from_text_buf("0");
        x->grain_size_jit = llll_from_text_buf("0");
        x->grain_onset_jit = llll_from_text_buf("0");
        x->change_rate = llll_from_text_buf("1");
        x->change_pitch = llll_from_text_buf("0");

        
        earsbufobj_init((t_earsbufobj *)x, EARSBUFOBJ_FLAG_SUPPORTS_COPY_NAMES);
        
        // @arg 0 @name outnames @optional 1 @type symbol
        // @digest Output buffer names
        // @description @copy EARS_DOC_OUTNAME_ATTR

        t_llll *args = llll_parse(true_ac, argv);
        t_llll *names = earsbufobj_extract_names_from_args((t_earsbufobj *)x, args);
        
        t_llllelem *arg_el = (args && args->l_head) ? args->l_head : NULL;
        if (arg_el) {
            
            // @arg 1 @name output_duration @optional 1 @type number/llll
            // @digest Output duration
            // @description Sets the output duration (based on the <m>timeunit</m>).
            
            llll_free(x->output_duration);
            x->output_duration = llll_get();
            if (hatom_gettype(&args->l_head->l_hatom) == H_LLLL)
                x->output_duration = llll_clone(hatom_getllll(&args->l_head->l_hatom));
            else {
                x->output_duration = llll_get();
                llll_appendhatom_clone(x->output_duration, &args->l_head->l_hatom);
            }
            arg_el = arg_el->l_next;
            
            if (arg_el) {
                
                // @arg 2 @name grain_interval @optional 1 @type number/llll
                // @digest Grain Interval
                // @description Sets the grain interval, either as a single number or as an envelope
                // (depending on the <m>timeunit</m> and <m>envtimeunit</m>).

                llll_free(x->grain_interval);
                x->grain_interval = llll_get();
                if (hatom_gettype(&arg_el->l_hatom) == H_LLLL)
                    x->grain_interval = llll_clone(hatom_getllll(&arg_el->l_hatom));
                else {
                    x->grain_interval = llll_get();
                    llll_appendhatom_clone(x->grain_interval, &arg_el->l_hatom);
                }
                arg_el = arg_el->l_next;
                
                if (arg_el) {
                    
                    // @arg 3 @name grain_duration @optional 1 @type number/llll
                    // @digest Grain Duration
                    // @description Sets the grain duration either as a single number or as an envelope
                    // (depending on the <m>timeunit</m> and <m>envtimeunit</m>).
                    
                    llll_free(x->grain_size);
                    x->grain_size = llll_get();
                    if (hatom_gettype(&arg_el->l_hatom) == H_LLLL)
                        x->grain_size = llll_clone(hatom_getllll(&arg_el->l_hatom));
                    else {
                        x->grain_size = llll_get();
                        llll_appendhatom_clone(x->grain_size, &arg_el->l_hatom);
                    }
                    arg_el = arg_el->l_next;
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


void buf_granulate_free(t_buf_granulate *x)
{
    llll_free(x->change_pitch);
    llll_free(x->change_rate);
    llll_free(x->grain_interval);
    llll_free(x->grain_size);
    llll_free(x->grain_onset);
    llll_free(x->grain_size_jit);
    llll_free(x->grain_onset_jit);
    llll_free(x->grain_interval_jit);
    llll_free(x->output_duration);
    
    earsbufobj_free((t_earsbufobj *)x);
}



void buf_granulate_bang(t_buf_granulate *x)
{
    long num_buffers = earsbufobj_get_instore_size((t_earsbufobj *)x, 0);
    
    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_buffers, true);
    
    earsbufobj_mutex_lock((t_earsbufobj *)x);
    earsbufobj_init_progress((t_earsbufobj *)x, num_buffers);

    t_llllelem *output_duration_el = x->output_duration->l_head;
    t_llllelem *grain_interval_el = x->grain_interval->l_head;
    t_llllelem *grain_interval_jit_el = x->grain_interval_jit->l_head;
    t_llllelem *grain_size_el = x->grain_size->l_head;
    t_llllelem *grain_onset_el = x->grain_onset->l_head;
    t_llllelem *grain_onset_jit_el = x->grain_onset_jit->l_head;
    t_llllelem *change_pitch_el = x->change_pitch->l_head;
    for (long count = 0; count < num_buffers; count++,
         output_duration_el = output_duration_el && output_duration_el->l_next ? output_duration_el->l_next : output_duration_el,
         grain_interval_el = grain_interval_el && grain_interval_el->l_next ? grain_interval_el->l_next : grain_interval_el,
         grain_interval_jit_el = grain_interval_jit_el && grain_interval_jit_el->l_next ? grain_interval_jit_el->l_next : grain_interval_jit_el,
         grain_size_el = grain_size_el && grain_size_el->l_next ? grain_size_el->l_next : grain_size_el,
         grain_onset_el = grain_onset_el && grain_onset_el->l_next ? grain_onset_el->l_next : grain_onset_el,
         grain_onset_jit_el = grain_onset_jit_el && grain_onset_jit_el->l_next ? grain_onset_jit_el->l_next : grain_onset_jit_el,
         change_pitch_el = change_pitch_el && change_pitch_el->l_next ? change_pitch_el->l_next : change_pitch_el
         ) {
        t_buffer_obj *in = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, count);
        t_buffer_obj *out = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, count);
        
        double sr = ears_buffer_get_sr((t_object *)x, in);

        double duration_samps = earsbufobj_time_to_samps((t_earsbufobj *)x, hatom_getdouble(&output_duration_el->l_hatom), in);
        ears_buffer_set_size_samps((t_object *)x, out, (long)round(duration_samps));

        t_llll *grain_size = earsbufobj_time_llllelem_to_samples_and_samples((t_earsbufobj *)x, grain_size_el, out);
        t_llll *grain_interval = earsbufobj_time_llllelem_to_samples_and_samples((t_earsbufobj *)x, grain_interval_el, out);
        t_llll *grain_interval_jit = earsbufobj_time_llllelem_to_samples_and_samples((t_earsbufobj *)x, grain_interval_jit_el, out);
        t_llll *grain_onset = earsbufobj_time_llllelem_to_samples_and_samples((t_earsbufobj *)x, grain_onset_el, out);
        t_llll *grain_onset_jit = earsbufobj_time_llllelem_to_samples_and_samples((t_earsbufobj *)x, grain_onset_jit_el, out);
        t_llll *change_pitch = earsbufobj_pitch_llllelem_to_cents_and_samples((t_earsbufobj *)x, change_pitch_el, out);

        
        ears_buffer_granulate((t_object *)x, in, out, duration_samps, grain_size, grain_interval, grain_interval_jit,
                              grain_onset, grain_onset_jit, x->e_ob.a_wintype, (e_slope_mapping)x->e_ob.l_slopemapping);
        
        
        llll_free(grain_size);
        llll_free(grain_interval);
        llll_free(grain_interval_jit);
        llll_free(grain_onset);
        llll_free(grain_onset_jit);
        llll_free(change_pitch);

        if (earsbufobj_iter_progress((t_earsbufobj *)x, count, num_buffers)) break;
    }
    earsbufobj_mutex_unlock((t_earsbufobj *)x);
    
    earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
}


void buf_granulate_anything(t_buf_granulate *x, t_symbol *msg, long ac, t_atom *av)
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
            
            buf_granulate_bang(x);
            
        } else if (inlet == 1) {
            earsbufobj_mutex_lock((t_earsbufobj *)x);
            llll_free(x->output_duration);
            x->output_duration = llll_clone(parsed);
            earsbufobj_mutex_unlock((t_earsbufobj *)x);
        } else if (inlet == 2) {
            earsbufobj_mutex_lock((t_earsbufobj *)x);
            llll_free(x->grain_interval);
            x->grain_interval = llll_clone(parsed);
            earsbufobj_mutex_unlock((t_earsbufobj *)x);
        } else if (inlet == 3) {
            earsbufobj_mutex_lock((t_earsbufobj *)x);
            llll_free(x->grain_size);
            x->grain_size = llll_clone(parsed);
            earsbufobj_mutex_unlock((t_earsbufobj *)x);
        } else if (inlet == 4) {
            earsbufobj_mutex_lock((t_earsbufobj *)x);
            llll_free(x->grain_onset);
            x->grain_onset = llll_clone(parsed);
            earsbufobj_mutex_unlock((t_earsbufobj *)x);

        }
    }
    llll_free(parsed);
}


