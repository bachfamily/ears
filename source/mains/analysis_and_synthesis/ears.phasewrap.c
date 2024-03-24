/**
	@file
	ears.phasewrap.c
 
	@name
	ears.phasewrap~
 
	@realname
	ears.phasewrap~
 
	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	Get principal phases
 
	@description
    Wraps sample values between -pi and pi.
 
	@discussion
 
	@category
	ears spectral
 
	@keywords
	buffer, phasewrap, wrap
 
	@seealso
	ears.phaseunwrap~, ears.fromdiffs~
	
	@owner
	Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "foundation/llllobj.h"
#include "foundation/llll_commons_ext.h"
#include "math/bach_math_utilities.h"
#include "ears.object.h"



typedef struct _buf_phasewrap {
    t_earsbufobj       e_ob;
} t_buf_phasewrap;



// Prototypes
t_buf_phasewrap*         buf_phasewrap_new(t_symbol *s, short argc, t_atom *argv);
void			buf_phasewrap_free(t_buf_phasewrap *x);
void			buf_phasewrap_bang(t_buf_phasewrap *x);
void			buf_phasewrap_anything(t_buf_phasewrap *x, t_symbol *msg, long ac, t_atom *av);

void buf_phasewrap_assist(t_buf_phasewrap *x, void *b, long m, long a, char *s);
void buf_phasewrap_inletinfo(t_buf_phasewrap *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(phasewrap)

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
    
    CLASS_NEW_CHECK_SIZE(c, "ears.phasewrap~",
                         (method)buf_phasewrap_new,
                         (method)buf_phasewrap_free,
                         sizeof(t_buf_phasewrap),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    // @method list/llll @digest Process buffers
    // @description A list or llll in the first inlet is supposed to contain buffer names and will
    // trigger the buffer processing and output the processed buffer names (depending on the <m>alloc</m> attribute).
    EARSBUFOBJ_DECLARE_COMMON_METHODS_HANDLETHREAD(phasewrap)

    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_blocking_attr(c);
    earsbufobj_class_add_alloc_attr(c);
    earsbufobj_class_add_polyout_attr(c);

    earsbufobj_class_add_angleunit_attr(c);

    
    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
}

void buf_phasewrap_assist(t_buf_phasewrap *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        if (a == 0)
            sprintf(s, "symbol/list/llll: Incoming Buffer Names"); // @in 0 @type symbol/list/llll @digest Incoming buffer names
    } else {
        sprintf(s, "symbol/list: Output Buffer Names"); // @out 0 @type symbol/list @digest Output buffer names
    }
}

void buf_phasewrap_inletinfo(t_buf_phasewrap *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_phasewrap *buf_phasewrap_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_phasewrap *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_phasewrap*)object_alloc_debug(s_tag_class);
    if (x) {
        earsbufobj_init((t_earsbufobj *)x, EARSBUFOBJ_FLAG_SUPPORTS_COPY_NAMES);
        
        // @arg 0 @name outnames @optional 1 @type symbol
        // @digest Output buffer names
        // @description @copy EARS_DOC_OUTNAME_ATTR

        t_llll *args = llll_parse(true_ac, argv);
        t_llll *names = earsbufobj_extract_names_from_args((t_earsbufobj *)x, args);
        
        attr_args_process(x, argc, argv);
        
        earsbufobj_setup((t_earsbufobj *)x, "E", "E", names);

        llll_free(args);
        llll_free(names);
    }
    return x;
}


void buf_phasewrap_free(t_buf_phasewrap *x)
{
    earsbufobj_free((t_earsbufobj *)x);
}



void buf_phasewrap_bang(t_buf_phasewrap *x)
{
    long num_buffers = earsbufobj_get_instore_size((t_earsbufobj *)x, 0);
    
    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_buffers, true);
    
    earsbufobj_mutex_lock((t_earsbufobj *)x);
    earsbufobj_init_progress((t_earsbufobj *)x, num_buffers);
    
    for (long count = 0; count < num_buffers; count++) {
        t_buffer_obj *in = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, count);
        t_buffer_obj *out = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, count);

        ears_buffer_phasewrap((t_object *)x, in, out, (e_ears_angleunit)x->e_ob.l_angleunit);

        if (earsbufobj_iter_progress((t_earsbufobj *)x, count, num_buffers)) break;
    }
    earsbufobj_mutex_unlock((t_earsbufobj *)x);
    
    earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
}


void buf_phasewrap_anything(t_buf_phasewrap *x, t_symbol *msg, long ac, t_atom *av)
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
            
            buf_phasewrap_bang(x);
        }
    }
    llll_free(parsed);
}


