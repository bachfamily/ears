/**
	@file
	ears.rot.c
 
	@name
	ears.rot~
 
	@realname
	ears.rot~
 
    @hiddenalias
    ears.rot

    @type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	Circular rotation of a buffer
 
	@description
	Rotates a sound buffer circularly
 
	@discussion
 
	@category
	ears buffer operations
 
	@keywords
	buffer, rotate, circular, circularly, loop
 
	@seealso
	ears.rev~, ears.repeat~, ears.slice~
	
	@owner
	Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "llllobj.h"
#include "llll_commons_ext.h"
#include "bach_math_utilities.h"
#include "ears.object.h"



typedef struct _buf_rot {
    t_earsbufobj       e_ob;
    
    t_llll             *byhowmuch;
} t_buf_rot;



// Prototypes
t_buf_rot*         buf_rot_new(t_symbol *s, short argc, t_atom *argv);
void			buf_rot_free(t_buf_rot *x);
void			buf_rot_bang(t_buf_rot *x);
void			buf_rot_anything(t_buf_rot *x, t_symbol *msg, long ac, t_atom *av);

void buf_rot_assist(t_buf_rot *x, void *b, long m, long a, char *s);
void buf_rot_inletinfo(t_buf_rot *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(rot)

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
    
    CLASS_NEW_CHECK_SIZE(c, "ears.rot~",
                         (method)buf_rot_new,
                         (method)buf_rot_free,
                         sizeof(t_buf_rot),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    // @method list/llll @digest Process buffers
    // @description A list or llll in the first inlet with buffer names will trigger the buffer processing and output the processed
    // buffer names (depending on the <m>naming</m> attribute).

    // @method number @digest Set shift amount
    // @description A number in the second inlet sets the shift amount (in the unit defined by the <m>timeunit</m> attribute).
    EARSBUFOBJ_DECLARE_COMMON_METHODS_DEFER(rot)


    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_timeunit_attr(c);
    earsbufobj_class_add_naming_attr(c);

    
    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
    return 0;
}

void buf_rot_assist(t_buf_rot *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        if (a == 0)
            sprintf(s, "symbol/list/llll: Incoming Buffer Names"); // @in 0 @type symbol/list/llll @digest Incoming buffer names
        else
            sprintf(s, "number: Shift Amount"); // @in 1 @type number/llll/symbol @digest Shift amount
    } else {
        sprintf(s, "symbol/list: Output Buffer Names"); // @out 0 @type symbol/list @digest Output buffer names
    }
}

void buf_rot_inletinfo(t_buf_rot *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_rot *buf_rot_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_rot *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_rot*)object_alloc_debug(s_tag_class);
    if (x) {
        x->byhowmuch = llll_from_text_buf("0", false);
        
        // @arg 0 @name outnames @optional 1 @type symbol
        // @digest Output buffer names
        // @description @copy EARS_DOC_OUTNAME_ATTR

        // @arg 1 @name shift @optional 1 @type number
        // @digest Shift
        // @description Set the shift time, depending on the <m>timeunit</m> attribute

        t_llll *args = llll_parse(true_ac, argv);
        t_llll *names = earsbufobj_extract_names_from_args((t_earsbufobj *)x, args);
        
        earsbufobj_init((t_earsbufobj *)x, EARSBUFOBJ_FLAG_DONT_DUPLICATE_INPUT_BUFFERS | EARSBUFOBJ_FLAG_SUPPORTS_COPY_NAMES);
        
        if (args && args->l_head) {
            llll_clear(x->byhowmuch);
            llll_appendhatom_clone(x->byhowmuch, &args->l_head->l_hatom);
        }
        
        attr_args_process(x, argc, argv);

        earsbufobj_setup((t_earsbufobj *)x, "E4", "E", names);

        llll_free(args);
        llll_free(names);
        
    }
    return x;
}


void buf_rot_free(t_buf_rot *x)
{
    llll_free(x->byhowmuch);
    earsbufobj_free((t_earsbufobj *)x);
}




void buf_rot_bang(t_buf_rot *x)
{
    long num_buffers = ((t_earsbufobj *)x)->l_instore[0].num_stored_bufs;
    
    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_buffers, true);
    
    t_llllelem *el = x->byhowmuch->l_head;
    for (long count = 0; count < num_buffers; count++, el = el && el->l_next ? el->l_next : el) {
        t_buffer_obj *in = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, count);
        t_buffer_obj *out = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, count);

        long out_samples = earsbufobj_input_to_samps((t_earsbufobj *)x, hatom_getdouble(&el->l_hatom), in);
        
        if (in != out)
            ears_buffer_clone((t_object *)x, in, out);

        ears_buffer_rot_inplace((t_object *)x, out, out_samples);
    }
    
    earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
}


void buf_rot_anything(t_buf_rot *x, t_symbol *msg, long ac, t_atom *av)
{
    long inlet = proxy_getinlet((t_object *) x);

    t_llll *parsed = earsbufobj_parse_gimme((t_earsbufobj *) x, LLLL_OBJ_VANILLA, msg, ac, av);
    if (!parsed) return;
    
    if (parsed && parsed->l_head) {
        if (inlet == 0) {
            
            earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, parsed->l_size, true);
            earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_OUT, 0, parsed->l_size, true);
            
            earsbufobj_store_buffer_list((t_earsbufobj *)x, parsed, 0, true);
            
            buf_rot_bang(x);
            
        } else if (inlet == 1) {
            llll_free(x->byhowmuch);
            x->byhowmuch = llll_clone(parsed);
        }
    }
    llll_free(parsed);
}


