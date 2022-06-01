/**
	@file
	ears.slice.c
 
	@name
	ears.slice~
 
	@realname
	ears.slice~
 
    @type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	Slice a buffer into two
 
	@description
	Splits a buffer according to a temporal position.
 
	@discussion
 
	@category
	ears basic
 
	@keywords
	buffer, slice, split
 
	@seealso
	ears.rev~, ears.rot~, ears.split~
	
	@owner
	Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "llllobj.h"
#include "llll_commons_ext.h"
#include "bach_math_utilities.h"
#include "ears.object.h"



typedef struct _buf_slice {
    t_earsbufobj       e_ob;
    
    t_llll             *where;
} t_buf_slice;



// Psliceotypes
t_buf_slice*         buf_slice_new(t_symbol *s, short argc, t_atom *argv);
void			buf_slice_free(t_buf_slice *x);
void			buf_slice_bang(t_buf_slice *x);
void			buf_slice_anything(t_buf_slice *x, t_symbol *msg, long ac, t_atom *av);

void buf_slice_assist(t_buf_slice *x, void *b, long m, long a, char *s);
void buf_slice_inletinfo(t_buf_slice *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(slice)

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
    
    CLASS_NEW_CHECK_SIZE(c, "ears.slice~",
                         (method)buf_slice_new,
                         (method)buf_slice_free,
                         sizeof(t_buf_slice),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    // @method list/llll @digest Process buffers
    // @description A list or llll in the first inlet with buffer names will trigger the slicing; the right portion of buffers is
    // output from the left outlet, and left portion of buffers is output from the left outlet.

    // @method number @digest Set slice position
    // @description A number in the second inlet sets the slice position (in the unit defined by the <m>timeunit</m> attribute).
    EARSBUFOBJ_DECLARE_COMMON_METHODS_HANDLETHREAD(slice)


    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_blocking_attr(c);
    earsbufobj_class_add_timeunit_attr(c);
    earsbufobj_class_add_naming_attr(c);

    earsbufobj_class_add_polyout_attr(c);

    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
    return 0;
}

void buf_slice_assist(t_buf_slice *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        if (a == 0)
            sprintf(s, "symbol/list/llll: Incoming Buffer Names"); // @in 0 @type symbol/list/llll @digest Incoming buffer names
        else
            sprintf(s, "number: Slice Position"); // @in 1 @type number/llll/symbol @digest Shift amount
    } else {
        if (a == 0)
            sprintf(s, "symbol/list: Output Left Buffer Names"); // @out 0 @type symbol/list @digest Output left buffer names
        else
            sprintf(s, "symbol/list: Output Right Buffer Names"); // @out 1 @type symbol/list @digest Output right buffer names
    }
}

void buf_slice_inletinfo(t_buf_slice *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_slice *buf_slice_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_slice *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_slice*)object_alloc_debug(s_tag_class);
    if (x) {
        x->where = llll_from_text_buf("0", false);

        earsbufobj_init((t_earsbufobj *)x, 0);

        // @arg 0 @name outnames @optional 1 @type symbol
        // @digest Output buffer names
        // @description @copy EARS_DOC_OUTNAME_ATTR

        // @arg 1 @name slice_point @optional 1 @type number
        // @digest Slice position
        // @description Set the slicing position, depending on the <m>timeunit</m> attribute

        t_llll *args = llll_parse(true_ac, argv);
        t_llll *names = earsbufobj_extract_names_from_args((t_earsbufobj *)x, args);
        
        if (args && args->l_head) {
            llll_clear(x->where);
            llll_appendhatom_clone(x->where, &args->l_head->l_hatom);
        }
        
        attr_args_process(x, argc, argv);

        earsbufobj_setup((t_earsbufobj *)x, "E4", "EE", names);

        llll_free(args);
        llll_free(names);
        
    }
    return x;
}


void buf_slice_free(t_buf_slice *x)
{
    llll_free(x->where);
    earsbufobj_free((t_earsbufobj *)x);
}




void buf_slice_bang(t_buf_slice *x)
{
    long num_buffers = earsbufobj_get_instore_size((t_earsbufobj *)x, 0);
    
    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_buffers, true);
    
    earsbufobj_mutex_lock((t_earsbufobj *)x);
    earsbufobj_init_progress((t_earsbufobj *)x, num_buffers);

    t_llllelem *el = x->where->l_head;
    for (long count = 0; count < num_buffers; count++, el = el && el->l_next ? el->l_next : el) {
        t_buffer_obj *in = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, count);
        t_buffer_obj *out_left = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, count);
        t_buffer_obj *out_right = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 1, count);

        long split_sample = earsbufobj_time_to_samps((t_earsbufobj *)x, hatom_getdouble(&el->l_hatom), in);
        
        ears_buffer_slice((t_object *)x, in, out_left, out_right, split_sample);

        if (earsbufobj_iter_progress((t_earsbufobj *)x, count, num_buffers)) break;
    }
    
    earsbufobj_mutex_unlock((t_earsbufobj *)x);

    earsbufobj_outlet_buffer((t_earsbufobj *)x, 1);
    earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
}


void buf_slice_anything(t_buf_slice *x, t_symbol *msg, long ac, t_atom *av)
{
    long inlet = earsbufobj_proxy_getinlet((t_earsbufobj *) x);

    t_llll *parsed = earsbufobj_parse_gimme((t_earsbufobj *) x, LLLL_OBJ_VANILLA, msg, ac, av);
    if (!parsed) return;
    
    if (parsed && parsed->l_head) {
        if (inlet == 0) {
            long num_bufs = llll_get_num_symbols_root(parsed);

            earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_bufs, true);
            earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_OUT, 0, num_bufs, true);
            earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_OUT, 1, num_bufs, true);

            earsbufobj_store_buffer_list((t_earsbufobj *)x, parsed, 0);
            
            buf_slice_bang(x);
            
        } else if (inlet == 1) {
            llll_free(x->where);
            x->where = llll_clone(parsed);
        }
    }
    llll_free(parsed);
}


