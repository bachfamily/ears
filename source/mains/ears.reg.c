/**
	@file
	ears.reg.c
 
	@name
	ears.reg~
 
	@realname
	ears.reg~
 
	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	Store or clone buffers
 
	@description
	Stores or clones buffers to be retrieved with a bang
 
	@discussion
 
	@category
	ears basic
 
	@keywords
	buffer, reg, store, keep
 
	@seealso
	ears.read~, ears.convert~
	
	@owner
	Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "llllobj.h"
#include "llll_commons_ext.h"
#include "bach_math_utilities.h"
#include "ears.object.h"



typedef struct _buf_reg {
    t_earsbufobj       e_ob;
    
    double             level;
    char               rms_mode;
    
    double             mix;
} t_buf_reg;



// Prototypes
t_buf_reg*         buf_reg_new(t_symbol *s, short argc, t_atom *argv);
void			buf_reg_free(t_buf_reg *x);
void			buf_reg_bang(t_buf_reg *x);
void			buf_reg_anything(t_buf_reg *x, t_symbol *msg, long ac, t_atom *av);

void buf_reg_assist(t_buf_reg *x, void *b, long m, long a, char *s);
void buf_reg_inletinfo(t_buf_reg *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;


EARSBUFOBJ_ADD_IO_METHODS(reg)

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
    
    CLASS_NEW_CHECK_SIZE(c, "ears.reg~",
                         (method)buf_reg_new,
                         (method)buf_reg_free,
                         sizeof(t_buf_reg),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    // @method list/llll @digest Store buffers
    // @description A list or llll with buffer names will be considered as the names of the buffers to be stored and output
    // (also according on the <m>naming</m> attribute).
    EARSBUFOBJ_DECLARE_COMMON_METHODS_DEFER(reg) // TO DO: should we NOT defer this?

    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_naming_attr(c);

    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
    return 0;
}

void buf_reg_assist(t_buf_reg *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        if (a == 0)
            sprintf(s, "symbol/list/llll: Store Buffer Names And Output"); // @in 0 @type symbol/list/llll @digest Store buffer names and output
        else
            sprintf(s, "symbol/list/llll: Store Buffer Names Without Output"); // @in 0 @type symbol/list/llll @digest Store buffer names without output
    } else {
        sprintf(s, "symbol/list: Output Buffer Names"); // @out 0 @type symbol/list @digest Output buffer names
    }
}

void buf_reg_inletinfo(t_buf_reg *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_reg *buf_reg_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_reg *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_reg*)object_alloc_debug(s_tag_class);
    if (x) {
        x->rms_mode = 0;
        x->level = 1.;
        
        earsbufobj_init((t_earsbufobj *)x,  EARSBUFOBJ_FLAG_SUPPORTS_COPY_NAMES);
        
        // @arg 0 @name outnames @optional 1 @type symbol
        // @digest Output buffer names
        // @description @copy EARS_DOC_OUTNAME_ATTR

        t_llll *args = llll_parse(true_ac, argv);
        t_llll *names = earsbufobj_extract_names_from_args((t_earsbufobj *)x, args);
        
        attr_args_process(x, argc, argv);

        earsbufobj_setup((t_earsbufobj *)x, "EE", "E", names);
       
        llll_free(args);
        llll_free(names);
        
    }
    return x;
}


void buf_reg_free(t_buf_reg *x)
{
    earsbufobj_free((t_earsbufobj *)x);
}


void buf_reg_bang(t_buf_reg *x)
{
    long num_buffers = ((t_earsbufobj *)x)->l_instore[0].num_stored_bufs;

    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
    
    for (long count = 0; count < num_buffers; count++) {
        t_symbol *name = earsbufobj_get_inlet_buffer_name((t_earsbufobj *)x, 0, count);
        if (name) {
            earsbufobj_store_buffer((t_earsbufobj *)x, EARSBUFOBJ_OUT, 0, count, name);
        } else {
            // not a big deal: one may use [ears.reg~] just to create an empty buffer
        }
    }
    earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
}


void buf_reg_anything(t_buf_reg *x, t_symbol *msg, long ac, t_atom *av)
{
    long inlet = earsbufobj_proxy_getinlet((t_earsbufobj *) x);

    t_llll *parsed = earsbufobj_parse_gimme((t_earsbufobj *) x, LLLL_OBJ_VANILLA, msg, ac, av);
    if (!parsed) return;
    
    if (parsed && parsed->l_head) {
        earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, parsed->l_size, true);
        earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_OUT, 0, parsed->l_size, true);
        
        earsbufobj_store_buffer_list((t_earsbufobj *)x, parsed, 0);
        
        if (inlet == 0)
            buf_reg_bang(x);
    }
    llll_free(parsed);
}


