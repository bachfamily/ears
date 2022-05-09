/**
	@file
	ears.envelope.c
 
	@name
	ears.envelope~
 
	@realname
	ears.envelope~
 
	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
    Fill a buffer with an envelope or a number
 
	@description
	Generates a buffer from a number or a breakpoint function
 
	@discussion
 
	@category
	ears basic
 
	@keywords
	buffer, envelope, bpf, breakpoint, function, generate, produce
 
	@seealso
	ears.gain~, ears.window~
	
	@owner
	Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "llllobj.h"
#include "llll_commons_ext.h"
#include "bach_math_utilities.h"
#include "ears.object.h"



typedef struct _buf_envelope {
    t_earsbufobj       e_ob;
    
    t_llll             *envelope;
    t_atom             duration;
} t_buf_envelope;



// Prototypes
t_buf_envelope*         buf_envelope_new(t_symbol *s, short argc, t_atom *argv);
void			buf_envelope_free(t_buf_envelope *x);
void			buf_envelope_bang(t_buf_envelope *x);
void			buf_envelope_anything(t_buf_envelope *x, t_symbol *msg, long ac, t_atom *av);

void buf_envelope_assist(t_buf_envelope *x, void *b, long m, long a, char *s);
void buf_envelope_inletinfo(t_buf_envelope *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(envelope)

/**********************************************************************/
// Class Definition and Life Cycle

int C74_EXPORT main(void)
{
    common_symbols_init();
    llllobj_common_symbols_init();
    
    if (llllobj_check_version(bach_get_current_llll_version()) || llllobj_test()) {
        ears_error_bachcheck();
        return 1;
    }
    
    t_class *c;
    
    CLASS_NEW_CHECK_SIZE(c, "ears.envelope~",
                         (method)buf_envelope_new,
                         (method)buf_envelope_free,
                         sizeof(t_buf_envelope),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    // @method llll @digest Envelope
    // @description An incoming llll is expected to contain the envelope that should be reproduced in the buffer.
    EARSBUFOBJ_DECLARE_COMMON_METHODS_HANDLETHREAD(envelope)
    
    CLASS_ATTR_ATOM(c, "duration", 0, t_buf_envelope, duration);
    CLASS_ATTR_STYLE_LABEL(c,"duration",0,"text","Duration");
    CLASS_ATTR_BASIC(c, "duration", 0);
    // @description When the <m>envtimeunit</m> attribute is set to relative mode, this attribute sets the
    // absolute duration of the buffer, either in milliseconds or in samples, depending in turns on the <m>timeunit</m> attribute.
    
    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_blocking_attr(c);
    earsbufobj_class_add_timeunit_attr(c);
    earsbufobj_class_add_envtimeunit_attr(c);
    earsbufobj_class_add_naming_attr(c);
    earsbufobj_class_add_slopemapping_attr(c);

    
    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
    return 0;
}

void buf_envelope_assist(t_buf_envelope *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        sprintf(s, "llll: Envelope"); // @in 0 @type llll @digest Envelope
    } else {
        sprintf(s, "symbol/list: Output Buffer Names"); // @out 0 @type symbol/list @digest Output buffer names
    }
}

void buf_envelope_inletinfo(t_buf_envelope *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_envelope *buf_envelope_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_envelope *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_envelope*)object_alloc_debug(s_tag_class);
    if (x) {
        x->envelope = llll_from_text_buf("1.", false);
        atom_setlong(&x->duration, 1000);
        
        // @arg 0 @name outnames @optional 1 @type symbol
        // @digest Output buffer names
        // @description @copy EARS_DOC_OUTNAME_ATTR

        earsbufobj_init((t_earsbufobj *)x, 0);

        t_llll *args = llll_parse(true_ac, argv);
        t_llll *names = earsbufobj_extract_names_from_args((t_earsbufobj *)x, args);
        
        attr_args_process(x, argc, argv);

        // @arg 1 @name duration @optional 1 @type number
        // @digest Duration
        // @description Duration of the envelope (in the unit given by the <m>timeunit</m> attribute).

        if (args && args->l_head) {
            if (hatom_gettype(&args->l_head->l_hatom) == H_LONG)
                atom_setlong(&x->duration, hatom_getlong(&args->l_head->l_hatom));
            else
                atom_setfloat(&x->duration, hatom_getdouble(&args->l_head->l_hatom));
        }

        earsbufobj_setup((t_earsbufobj *)x, "44", "E", names);

        llll_free(args);
        llll_free(names);
    }
    return x;
}


void buf_envelope_free(t_buf_envelope *x)
{
    llll_free(x->envelope);
    earsbufobj_free((t_earsbufobj *)x);
}



void buf_envelope_bang(t_buf_envelope *x)
{
    long num_buffers = x->envelope->l_size;
    
    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_OUT, 0, num_buffers, true);
    
    earsbufobj_mutex_lock((t_earsbufobj *)x);
    earsbufobj_init_progress((t_earsbufobj *)x, num_buffers);

    t_llllelem *el = x->envelope->l_head;
    for (long count = 0; count < num_buffers; count++, el = el && el->l_next ? el->l_next : el) {
        t_buffer_obj *out = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, count);
        long duration_samps = 0;
        if (x->e_ob.l_envtimeunit == EARS_TIMEUNIT_DURATION_RATIO || x->e_ob.l_envtimeunit == EARS_TIMEUNIT_NUM_INTERVALS || x->e_ob.l_envtimeunit == EARS_TIMEUNIT_NUM_ONSETS) {
            duration_samps = earsbufobj_atom_to_samps((t_earsbufobj *)x, &x->duration, out);
        } else {
            t_atom temp;
            ears_envelope_get_max_x(el, &temp);
            switch (x->e_ob.l_envtimeunit) {
                case EARS_TIMEUNIT_SAMPS:
                    duration_samps = atom_getlong(&temp);
                    break;
                case EARS_TIMEUNIT_MS:
                    duration_samps = ears_ms_to_samps(atom_getfloat(&temp), ears_buffer_get_sr((t_object *)x, out));
                    break;
                case EARS_TIMEUNIT_SECONDS:
                    duration_samps = ears_ms_to_samps(atom_getfloat(&temp)*1000., ears_buffer_get_sr((t_object *)x, out));
                    break;
                default:
                    break;
            }
        }
        
        ears_buffer_set_size_and_numchannels((t_object *)x, out, duration_samps, 1);
        ears_buffer_fill_inplace((t_object *)x, out, 1.);

        t_llll *env = earsbufobj_llllelem_to_env_samples((t_earsbufobj *)x, el, out);
        
        
        if (env->l_depth == 1 && env->l_head) {
            // envelope is a single number
            ears_buffer_gain((t_object *)x, out, out, hatom_getdouble(&env->l_head->l_hatom), x->e_ob.l_ampunit == EARS_AMPUNIT_DECIBEL);
        } else {
            // envelope is an envelope in llll form
            ears_buffer_gain_envelope((t_object *)x, out, out, env, x->e_ob.l_envampunit == EARS_AMPUNIT_DECIBEL, earsbufobj_get_slope_mapping((t_earsbufobj *)x));
        }
        
        llll_free(env);

        if (earsbufobj_iter_progress((t_earsbufobj *)x, count, num_buffers)) break;
    }
    earsbufobj_mutex_unlock((t_earsbufobj *)x);
    
    earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
}


void buf_envelope_anything(t_buf_envelope *x, t_symbol *msg, long ac, t_atom *av)
{
    long inlet = earsbufobj_proxy_getinlet((t_earsbufobj *) x);
    t_llll *parsed = earsbufobj_parse_gimme((t_earsbufobj *) x, LLLL_OBJ_VANILLA, msg, ac, av);
    if (!parsed) return;
    
    if (parsed && parsed->l_head) {
        if (inlet == 0) {
        earsbufobj_mutex_lock((t_earsbufobj *)x);
        llll_free(x->envelope);
        x->envelope = llll_clone(parsed);
        earsbufobj_mutex_unlock((t_earsbufobj *)x);
        
        buf_envelope_bang(x);
        } else {
            earsbufobj_mutex_lock((t_earsbufobj *)x);
            if (hatom_gettype(&parsed->l_head->l_hatom) == H_LONG) {
                atom_setlong(&x->duration, hatom_getlong(&parsed->l_head->l_hatom));
            } else {
                atom_setfloat(&x->duration, hatom_getdouble(&parsed->l_head->l_hatom));
            }
            earsbufobj_mutex_unlock((t_earsbufobj *)x);
        }
    }
    llll_free(parsed);
}


