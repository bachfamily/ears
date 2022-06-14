/**
	@file
	ears.overdrive.c
 
	@name
	ears.overdrive~
 
	@realname
	ears.overdrive~
 
	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	Buffer soft clipping
 
	@description
	Perform a soft clip distorsion on the incoming buffers
 
	@discussion
 
	@category
	ears distorsion
 
	@keywords
	buffer, overdrive, distort, hard
 
	@seealso
	ears.clip~, overdrive
	
	@owner
	Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "foundation/llllobj.h"
#include "foundation/llll_commons_ext.h"
#include "bach_math_utilities.h"
#include "ears.object.h"



typedef struct _buf_overdrive {
    t_earsbufobj       e_ob;
    
    t_llll             *drive;
} t_buf_overdrive;



// Prototypes
t_buf_overdrive*         buf_overdrive_new(t_symbol *s, short argc, t_atom *argv);
void			buf_overdrive_free(t_buf_overdrive *x);
void			buf_overdrive_bang(t_buf_overdrive *x);
void			buf_overdrive_anything(t_buf_overdrive *x, t_symbol *msg, long ac, t_atom *av);

void buf_overdrive_assist(t_buf_overdrive *x, void *b, long m, long a, char *s);
void buf_overdrive_inletinfo(t_buf_overdrive *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(overdrive)

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
    
    CLASS_NEW_CHECK_SIZE(c, "ears.overdrive~",
                         (method)buf_overdrive_new,
                         (method)buf_overdrive_free,
                         sizeof(t_buf_overdrive),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    // @method list/llll @digest Function depends on inlet
    // @description A list or llll in the first inlet is supposed to contain buffer names and will
    // trigger the buffer processing and output the processed buffer names (depending on the <m>naming</m> attribute). <br />
    // A number or an llll in the second inlet is expected to contain a overdrive threshold (depending on the <m>ampunit</m>) or
    // an envelope (also see <m>envampunit</m>).
    EARSBUFOBJ_DECLARE_COMMON_METHODS_HANDLETHREAD(overdrive)

    // @method number @digest Set overdrive
    // @description A number in the second inlet sets the overdrive parameter (depending on the <m>ampunit</m>).

    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_blocking_attr(c);
    earsbufobj_class_add_envtimeunit_attr(c);
    earsbufobj_class_add_naming_attr(c);
    earsbufobj_class_add_slopemapping_attr(c);

    earsbufobj_class_add_polyout_attr(c);

    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
    return 0;
}

void buf_overdrive_assist(t_buf_overdrive *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        if (a == 0)
            sprintf(s, "symbol/list/llll: Incoming Buffer Names"); // @in 0 @type symbol/list/llll @digest Incoming buffer names
        else
            sprintf(s, "number/llll: Drive Factor or Envelope"); // @in 1 @type number/llll @digest Drive factor or envelope
    } else {
        sprintf(s, "symbol/list: Output Buffer Names"); // @out 0 @type symbol/list @digest Output buffer names
    }
}

void buf_overdrive_inletinfo(t_buf_overdrive *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_overdrive *buf_overdrive_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_overdrive *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_overdrive*)object_alloc_debug(s_tag_class);
    if (x) {
        x->drive = llll_from_text_buf("1.", false);
        
        earsbufobj_init((t_earsbufobj *)x,  EARSBUFOBJ_FLAG_SUPPORTS_COPY_NAMES);
        
        // @arg 0 @name outnames @optional 1 @type symbol
        // @digest Output buffer names
        // @description @copy EARS_DOC_OUTNAME_ATTR

        // @arg 1 @name drive @optional 1 @type number
        // @digest Drive factor
        // @description Sets the drive factor.

        t_llll *args = llll_parse(true_ac, argv);
        t_llll *names = earsbufobj_extract_names_from_args((t_earsbufobj *)x, args);
        
        if (args && args->l_head) {
            llll_clear(x->drive);
            llll_appendhatom_clone(x->drive, &args->l_head->l_hatom);
        }
        
        attr_args_process(x, argc, argv);
        
        earsbufobj_setup((t_earsbufobj *)x, "E4", "E", names);

        llll_free(args);
        llll_free(names);
    }
    return x;
}


void buf_overdrive_free(t_buf_overdrive *x)
{
    llll_free(x->drive);
    earsbufobj_free((t_earsbufobj *)x);
}



void buf_overdrive_bang(t_buf_overdrive *x)
{
    long num_buffers = earsbufobj_get_instore_size((t_earsbufobj *)x, 0);
    
    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_buffers, true);
    
    earsbufobj_mutex_lock((t_earsbufobj *)x);
    earsbufobj_init_progress((t_earsbufobj *)x, num_buffers);
    
    t_llllelem *el = x->drive->l_head;
    for (long count = 0; count < num_buffers; count++, el = el && el->l_next ? el->l_next : el) {
        t_buffer_obj *in = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, count);
        t_buffer_obj *out = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, count);

        t_llll *env = earsbufobj_llllelem_to_env_samples((t_earsbufobj *)x, el, in);
        
        if (env->l_size == 0) {
            object_error((t_object *)x, "No drive factor defined.");
            if (in != out)
                ears_buffer_clone((t_object *)x, in, out);
        } else if (env->l_depth == 1 && env->l_head) {
            // drive is a single number
            ears_buffer_overdrive((t_object *)x, in, out, hatom_getdouble(&env->l_head->l_hatom));
        } else {
            // drive is an envelope in llll form
            ears_buffer_overdrive_envelope((t_object *)x, in, out, env, earsbufobj_get_slope_mapping((t_earsbufobj *)x));
        }
        
        llll_free(env);

        if (earsbufobj_iter_progress((t_earsbufobj *)x, count, num_buffers)) break;
    }
    earsbufobj_mutex_unlock((t_earsbufobj *)x);
    
    earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
}


void buf_overdrive_anything(t_buf_overdrive *x, t_symbol *msg, long ac, t_atom *av)
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
            
            buf_overdrive_bang(x);
            
        } else if (inlet == 1) {
            earsbufobj_mutex_lock((t_earsbufobj *)x);
            llll_free(x->drive);
            x->drive = llll_clone(parsed);
            earsbufobj_mutex_unlock((t_earsbufobj *)x);
        }
    }
    llll_free(parsed);
}


