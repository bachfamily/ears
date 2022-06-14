/**
	@file
	ears.tosamps.c
 
	@name
	ears.tosamps~
 
	@realname
	ears.tosamps~
 
	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	Get buffer samples as llll
 
	@description
	Retrieves the samples of a buffer as llll
 
	@discussion
 
	@category
	ears basic
 
	@keywords
	buffer, samples, retrieve
 
	@seealso
	ears.fromsamps~
	
	@owner
	Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "foundation/llllobj.h"
#include "foundation/llll_commons_ext.h"
#include "bach_math_utilities.h"
#include "ears.object.h"



typedef struct _buf_tosamps {
    t_earsbufobj       e_ob;
} t_buf_tosamps;



// Prototypes
t_buf_tosamps*         buf_tosamps_new(t_symbol *s, short argc, t_atom *argv);
void			buf_tosamps_free(t_buf_tosamps *x);
void			buf_tosamps_bang(t_buf_tosamps *x);
void			buf_tosamps_anything(t_buf_tosamps *x, t_symbol *msg, long ac, t_atom *av);

void buf_tosamps_assist(t_buf_tosamps *x, void *b, long m, long a, char *s);
void buf_tosamps_inletinfo(t_buf_tosamps *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(tosamps)

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
    
    CLASS_NEW_CHECK_SIZE(c, "ears.tosamps~",
                         (method)buf_tosamps_new,
                         (method)buf_tosamps_free,
                         sizeof(t_buf_tosamps),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    // @method symbol/llll @digest Convert buffer to samples
    // @description A buffer name (as a symbol or llll) will trigger the output of its sample values, as llll,
    // organized with one parenthesis level for each channel.
    EARSBUFOBJ_DECLARE_COMMON_METHODS_HANDLETHREAD(tosamps)
    
    earsbufobj_class_add_blocking_attr(c);
    llllobj_class_add_out_attr(c, LLLL_OBJ_VANILLA);
    
    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
    return 0;
}



void buf_tosamps_assist(t_buf_tosamps *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        sprintf(s, "symbol/list/llll: Incoming Buffer Name"); // @in 0 @type symbol/llll @digest Incoming buffer name
    } else {
        char *type = NULL;
        llllobj_get_llll_outlet_type_as_string((t_object *) x, LLLL_OBJ_VANILLA, a, &type);
        sprintf(s, "llll (%s): Samples", type); // @out 0 @type llll @digest Samples
    }
}

void buf_tosamps_inletinfo(t_buf_tosamps *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_tosamps *buf_tosamps_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_tosamps *x;
//    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_tosamps*)object_alloc_debug(s_tag_class);
    if (x) {
        earsbufobj_init((t_earsbufobj *)x, 0);
        attr_args_process(x, argc, argv);
        earsbufobj_setup((t_earsbufobj *)x, "e", "4", NULL);
    }
    return x;
}


void buf_tosamps_free(t_buf_tosamps *x)
{
    earsbufobj_free((t_earsbufobj *)x);
}



void buf_tosamps_bang(t_buf_tosamps *x)
{
    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, 1, true);
    
    earsbufobj_mutex_lock((t_earsbufobj *)x);
    t_llll *ll = ears_buffer_to_llll((t_object *)x, earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, 0));
    earsbufobj_mutex_unlock((t_earsbufobj *)x);
    
    earsbufobj_outlet_llll((t_earsbufobj *)x, 0, ll);
    llll_free(ll);
}


void buf_tosamps_anything(t_buf_tosamps *x, t_symbol *msg, long ac, t_atom *av)
{
    long inlet = earsbufobj_proxy_getinlet((t_earsbufobj *) x);

    t_llll *parsed = earsbufobj_parse_gimme((t_earsbufobj *) x, LLLL_OBJ_VANILLA, msg, ac, av);
    if (!parsed) return;
    
    if (parsed && parsed->l_head) {
        if (inlet == 0) {
            if (hatom_gettype(&parsed->l_head->l_hatom) == H_SYM) {
                earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, 1, true);
                earsbufobj_store_buffer((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, 0, hatom_getsym(&parsed->l_head->l_hatom));
            }
            
            buf_tosamps_bang(x);
        }
    }
    llll_free(parsed);
}


