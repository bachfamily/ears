/**
	@file
	ears.neq.c
 
	@name
	ears.!=~
 
	@realname
	ears.neq~
 
	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	Compare buffers for inequality
 
	@description
	Compares two buffers for inequality.
 
	@discussion
 
	@category
	ears basic
 
	@keywords
	buffer, unequal, inequality, condition, compare
 
	@seealso
	ears.==~
	
	@owner
	Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "foundation/llllobj.h"
#include "foundation/llll_commons_ext.h"
#include "math/bach_math_utilities.h"
#include "ears.object.h"

typedef struct _buf_neq {
    t_earsbufobj       e_ob;
} t_buf_neq;



// Prototypes
t_buf_neq*     buf_neq_new(t_symbol *s, short argc, t_atom *argv);
void			buf_neq_free(t_buf_neq *x);
void			buf_neq_bang(t_buf_neq *x);
void			buf_neq_anything(t_buf_neq *x, t_symbol *msg, long ac, t_atom *av);

void buf_neq_assist(t_buf_neq *x, void *b, long m, long a, char *s);
void buf_neq_inletinfo(t_buf_neq *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(neq)

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
    
    CLASS_NEW_CHECK_SIZE(c, "ears.neq~",
                         (method)buf_neq_new,
                         (method)buf_neq_free,
                         sizeof(t_buf_neq),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    
    // @method symbol/llll @digest Process buffers
    // @description A symbol or llll in any of the inlet sets the corresponding buffer for comparison.
    // If the inlet is the first one, then the comparison result is output: 0 if the buffers coincide
    // in all their properties (including the spectral ones, for spectral buffers) and in all their samples, and 1 otherwise,
    EARSBUFOBJ_DECLARE_COMMON_METHODS_HANDLETHREAD(neq)
    
    earsbufobj_class_add_blocking_attr(c);

    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
    return 0;
}

void buf_neq_assist(t_buf_neq *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        if (a == 0) // @in 0 @type symbol @digest First buffer name
            sprintf(s, "symbol: First Buffer");
        else // @in 0 @type symbol @digest Second buffer name
            sprintf(s, "symbol: Second Buffer");
    } else {
        sprintf(s, "int: 1 if Buffers Are different, 0 Otherwise"); // @out 0 @type int @digest 1 if Buffers Are different, 0 Otherwise
    }
}

void buf_neq_inletinfo(t_buf_neq *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_neq *buf_neq_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_neq *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_neq*)object_alloc_debug(s_tag_class);
    if (x) {
        earsbufobj_init((t_earsbufobj *)x,  EARSBUFOBJ_FLAG_NONE);
        
        t_llll *args = llll_parse(true_ac, argv);
        
        attr_args_process(x, argc, argv);

        earsbufobj_setup((t_earsbufobj *)x, "ee", "i", NULL);

        llll_free(args);
    }
    return x;
}


void buf_neq_free(t_buf_neq *x)
{
    earsbufobj_free((t_earsbufobj *)x);
}



void buf_neq_bang(t_buf_neq *x)
{
    long num_buffers1 = earsbufobj_get_instore_size((t_earsbufobj *)x, 0);
    long num_buffers2 = earsbufobj_get_instore_size((t_earsbufobj *)x, 1);

    if (num_buffers1 == 0 || num_buffers2 == 0) {
        object_error((t_object *)x, "Both buffers must be introduced for comparison.");
        return;
    }
    
    earsbufobj_mutex_lock((t_earsbufobj *)x);
    t_buffer_obj *buf1 = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, 0);
    t_buffer_obj *buf2 = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 1, 0);
    long res = 0;
    ears_buffer_neq((t_object *)x, buf1, buf2, &res);
    earsbufobj_mutex_unlock((t_earsbufobj *)x);

    earsbufobj_outlet_int((t_earsbufobj *)x, 0, res);
}



void buf_neq_anything(t_buf_neq *x, t_symbol *msg, long ac, t_atom *av)
{
    long inlet = earsbufobj_proxy_getinlet((t_earsbufobj *) x);

    t_llll *parsed = earsbufobj_parse_gimme((t_earsbufobj *) x, LLLL_OBJ_VANILLA, msg, ac, av);
    if (!parsed) return;
    
    if (parsed && parsed->l_head) {
        if (hatom_gettype(&parsed->l_head->l_hatom) == H_SYM) {
            long num_bufs = llll_get_num_symbols_root(parsed);
            earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, inlet, num_bufs, true);
            earsbufobj_store_buffer_list((t_earsbufobj *)x, parsed, inlet);
            
            if (inlet == 0)
                buf_neq_bang(x);
        }
    }
    llll_free(parsed);
}


