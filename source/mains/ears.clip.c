/**
	@file
	ears.clip.c
 
	@name
	ears.clip~
 
	@realname
	ears.clip~
 
	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	Buffer hard clipping
 
	@description
	Perform a hard clip distorsion on the incoming buffers
 
	@discussion
 
	@category
	ears distorsion
 
	@keywords
	buffer, clip, distort, hard
 
	@seealso
	ears.overdrive~, clip
	
	@owner
	Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "llllobj.h"
#include "llll_commons_ext.h"
#include "bach_math_utilities.h"
#include "ears.object.h"



typedef struct _buf_clip {
    t_earsbufobj       e_ob;
    
    t_llll             *threshold;
} t_buf_clip;



// Prototypes
t_buf_clip*         buf_clip_new(t_symbol *s, short argc, t_atom *argv);
void			buf_clip_free(t_buf_clip *x);
void			buf_clip_bang(t_buf_clip *x);
void			buf_clip_anything(t_buf_clip *x, t_symbol *msg, long ac, t_atom *av);

void buf_clip_assist(t_buf_clip *x, void *b, long m, long a, char *s);
void buf_clip_inletinfo(t_buf_clip *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(clip)

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
    
    CLASS_NEW_CHECK_SIZE(c, "ears.clip~",
                         (method)buf_clip_new,
                         (method)buf_clip_free,
                         sizeof(t_buf_clip),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    // @method list/llll @digest Function depends on inlet
    // @description A list or llll in the first inlet is supposed to contain buffer names and will
    // trigger the buffer processing and output the processed buffer names (depending on the <m>naming</m> attribute). <br />
    // A number or an llll in the second inlet is expected to contain a clip threshold (depending on the <m>ampunit</m>) or
    // an envelope (also see <m>envampunit</m>).
    EARSBUFOBJ_DECLARE_COMMON_METHODS_HANDLETHREAD(clip)

    // @method number @digest Set clip
    // @description A number in the second inlet sets the clip parameter (depending on the <m>ampunit</m>).

    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_ampunit_attr(c);
    earsbufobj_class_add_envampunit_attr(c);
    earsbufobj_class_add_envtimeunit_attr(c);
    earsbufobj_class_add_naming_attr(c);
    earsbufobj_class_add_slopemapping_attr(c);

    
    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
    return 0;
}

void buf_clip_assist(t_buf_clip *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        if (a == 0)
            sprintf(s, "symbol/list/llll: Incoming Buffer Names"); // @in 0 @type symbol/list/llll @digest Incoming buffer names
        else
            sprintf(s, "number/llll: Clip threshold"); // @in 1 @type number/llll @digest Clipping threshold or envelope
    } else {
        sprintf(s, "symbol/list: Output Buffer Names"); // @out 0 @type symbol/list @digest Output buffer names
    }
}

void buf_clip_inletinfo(t_buf_clip *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_clip *buf_clip_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_clip *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_clip*)object_alloc_debug(s_tag_class);
    if (x) {
        x->threshold = llll_from_text_buf("1.", false);
        
        earsbufobj_init((t_earsbufobj *)x, EARSBUFOBJ_FLAG_SUPPORTS_COPY_NAMES);
        
        // @arg 0 @name outnames @optional 1 @type symbol
        // @digest Output buffer names
        // @description @copy EARS_DOC_OUTNAME_ATTR

        // @arg 1 @name threshold @optional 1 @type number
        // @digest Clipping threshold
        // @description Sets the clip amount in the unit defined by the <m>ampunit</m> attribute or by the following argument.

        // @arg 2 @name amp_unit @optional 1 @type symbol
        // @digest Amplitude unit
        // @description Sets the amplitude unit (see <m>ampunit</m> attribute).

        t_llll *args = llll_parse(true_ac, argv);
        t_llll *names = earsbufobj_extract_names_from_args((t_earsbufobj *)x, args);
        
        if (args && args->l_head) {
            llll_clear(x->threshold);
            llll_appendhatom_clone(x->threshold, &args->l_head->l_hatom);
            if (args->l_head->l_next && hatom_gettype(&args->l_head->l_next->l_hatom) == H_SYM) {
                t_atom av;
                atom_setsym(&av, hatom_getsym(&args->l_head->l_next->l_hatom));
                object_attr_setvalueof(x, gensym("ampunit"), 1, &av);
            }
        }
        

        attr_args_process(x, argc, argv);
        
        earsbufobj_setup((t_earsbufobj *)x, "E4", "E", names);

        llll_free(args);
        llll_free(names);
    }
    return x;
}


void buf_clip_free(t_buf_clip *x)
{
    llll_free(x->threshold);
    earsbufobj_free((t_earsbufobj *)x);
}



void buf_clip_bang(t_buf_clip *x)
{
    long num_buffers = ((t_earsbufobj *)x)->l_instore[0].num_stored_bufs;
    
    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_buffers, true);
    
    earsbufobj_mutex_lock((t_earsbufobj *)x);
    t_llllelem *el = x->threshold->l_head;
    for (long count = 0; count < num_buffers; count++, el = el && el->l_next ? el->l_next : el) {
        t_buffer_obj *in = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, count);
        t_buffer_obj *out = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, count);

        t_llll *env = earsbufobj_llllelem_to_env_samples((t_earsbufobj *)x, el, in);
        
        if (env->l_size == 0) {
            object_error((t_object *)x, "No threshold defined.");
            if (in != out)
                ears_buffer_clone((t_object *)x, in, out);
        } else if (env->l_depth == 1 && env->l_head) {
            // clip is a single number
            ears_buffer_clip((t_object *)x, in, out, hatom_getdouble(&env->l_head->l_hatom), x->e_ob.l_ampunit == EARS_AMPUNIT_DECIBEL);
        } else {
            // clip is an envelope in llll form
            ears_buffer_clip_envelope((t_object *)x, in, out, env, x->e_ob.l_envampunit == EARS_AMPUNIT_DECIBEL, earsbufobj_get_slope_mapping((t_earsbufobj *)x));
        }
        
        llll_free(env);
    }
    earsbufobj_mutex_unlock((t_earsbufobj *)x);
    
    earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
}


void buf_clip_anything(t_buf_clip *x, t_symbol *msg, long ac, t_atom *av)
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
            
            buf_clip_bang(x);
            
        } else if (inlet == 1) {
            earsbufobj_mutex_lock((t_earsbufobj *)x);
            llll_free(x->threshold);
            x->threshold = llll_clone(parsed);
            earsbufobj_mutex_unlock((t_earsbufobj *)x);
        }
    }
    llll_free(parsed);
}


