/**
	@file
	ears.rev.c
 
	@name
	ears.rev~
 
	@realname
	ears.rev~
 
	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	Reverse buffer
 
	@description
	Performs a sample-by-sample time reversal of a buffer
 
	@discussion
 
	@category
	ears basic
 
	@keywords
	buffer, rev, reverse, retrograde, retrogradation, time
 
	@seealso
	ears.rot~, ears.repeat~, ears.slice~
	
	@owner
	Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "llllobj.h"
#include "llll_commons_ext.h"
#include "bach_math_utilities.h"
#include "ears.object.h"



typedef struct _buf_rev {
    t_earsbufobj        e_ob;
} t_buf_rev;




// Prototypes
t_buf_rev*         buf_rev_new(t_symbol *s, short argc, t_atom *argv);
void			buf_rev_free(t_buf_rev *x);
void			buf_rev_bang(t_buf_rev *x);
void			buf_rev_anything(t_buf_rev *x, t_symbol *msg, long ac, t_atom *av);

void buf_rev_assist(t_buf_rev *x, void *b, long m, long a, char *s);
void buf_rev_inletinfo(t_buf_rev *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(rev)

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
    
    CLASS_NEW_CHECK_SIZE(c, "ears.rev~",
                         (method)buf_rev_new,
                         (method)buf_rev_free,
                         sizeof(t_buf_rev),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    // @method list/llll @digest Process buffers
    // @description A list or llll with buffer names will trigger the buffer processing and output the processed
    // buffer names (depending on the <m>naming</m> attribute).
    EARSBUFOBJ_DECLARE_COMMON_METHODS_HANDLETHREAD(rev)
    
    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_blocking_attr(c);
    earsbufobj_class_add_naming_attr(c);
    
    earsbufobj_class_add_poly_attr(c);

    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
    return 0;
}

void buf_rev_assist(t_buf_rev *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        sprintf(s, "symbol/list/llll: Incoming Buffer Names"); // @in 0 @type symbol/list/llll @digest Incoming buffer names
    } else {
        sprintf(s, "symbol/list: revted Buffer Names"); // @out 0 @type symbol/list @digest revd buffer names
    }
}

void buf_rev_inletinfo(t_buf_rev *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_rev *buf_rev_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_rev *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_rev*)object_alloc_debug(s_tag_class);
    if (x) {
        
        earsbufobj_init((t_earsbufobj *)x,  EARSBUFOBJ_FLAG_SUPPORTS_COPY_NAMES);
        
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


void buf_rev_free(t_buf_rev *x)
{
    earsbufobj_free((t_earsbufobj *)x);
}




void buf_rev_bang(t_buf_rev *x)
{
    long num_buffers = earsbufobj_get_instore_size((t_earsbufobj *)x, 0);
    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_buffers, true);
    
    earsbufobj_mutex_lock((t_earsbufobj *)x);
    earsbufobj_init_progress((t_earsbufobj *)x, num_buffers);
    
    for (long count = 0; count < num_buffers; count++) {
        t_buffer_obj *in = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, count);
        t_buffer_obj *out = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, count);
        
        if (in != out)
            ears_buffer_clone((t_object *)x, in, out);

        ears_buffer_rev_inplace((t_object *)x, out);

        if (earsbufobj_iter_progress((t_earsbufobj *)x, count, num_buffers)) break;
    }
    
    earsbufobj_mutex_unlock((t_earsbufobj *)x);

    earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
}


void buf_rev_anything(t_buf_rev *x, t_symbol *msg, long ac, t_atom *av)
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
            
            buf_rev_bang(x);
        }
    }
    llll_free(parsed);
}


