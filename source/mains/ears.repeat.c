/**
	@file
	ears.repeat.c
 
	@name
	ears.repeat~
 
	@realname
	ears.repeat~
 
    @type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	Repeat buffer
 
	@description
	Loops a given buffer
 
	@discussion
 
	@category
	ears basic
 
	@keywords
	buffer, repeat, loop, iterate
 
	@seealso
	ears.rev~, ears.rot~, ears.crop~
	
	@owner
	Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "llllobj.h"
#include "llll_commons_ext.h"
#include "bach_math_utilities.h"
#include "ears.object.h"



typedef struct _buf_repeat {
    t_earsbufobj       e_ob;
    
    t_llll             *howmany;
} t_buf_repeat;



// Prototypes
t_buf_repeat*         buf_repeat_new(t_symbol *s, short argc, t_atom *argv);
void			buf_repeat_free(t_buf_repeat *x);
void			buf_repeat_bang(t_buf_repeat *x);
void			buf_repeat_anything(t_buf_repeat *x, t_symbol *msg, long ac, t_atom *av);

void buf_repeat_assist(t_buf_repeat *x, void *b, long m, long a, char *s);
void buf_repeat_inletinfo(t_buf_repeat *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(repeat)

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
    
    CLASS_NEW_CHECK_SIZE(c, "ears.repeat~",
                         (method)buf_repeat_new,
                         (method)buf_repeat_free,
                         sizeof(t_buf_repeat),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    // @method list/llll @digest Process buffers
    // @description A list or llll in the first inlet with buffer names will trigger the buffer processing and output the processed
    // buffer names (depending on the <m>naming</m> attribute).

    // @method int @digest Set number of repetitions
    // @description An integer number in the second inlet sets the number of repetitions.
    EARSBUFOBJ_DECLARE_COMMON_METHODS_HANDLETHREAD(repeat)


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

void buf_repeat_assist(t_buf_repeat *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        if (a == 0)
            sprintf(s, "symbol/list/llll: Incoming Buffer Names"); // @in 0 @type symbol/list/llll @digest Incoming buffer names
        else
            sprintf(s, "number: repeat factor"); // @in 1 @type number/llll/symbol @digest repeat factor, envelope or buffer
    } else {
        sprintf(s, "symbol/list: Output Buffer Names"); // @out 0 @type symbol/list @digest Output buffer names
    }
}

void buf_repeat_inletinfo(t_buf_repeat *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_repeat *buf_repeat_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_repeat *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_repeat*)object_alloc_debug(s_tag_class);
    if (x) {
        x->howmany = llll_from_text_buf("1", false);
        
        earsbufobj_init((t_earsbufobj *)x,  EARSBUFOBJ_FLAG_SUPPORTS_COPY_NAMES);
        
        // @arg 0 @name outnames @optional 1 @type symbol
        // @digest Output buffer names
        // @description @copy EARS_DOC_OUTNAME_ATTR

        // @arg 1 @name howmany @optional 1 @type number
        // @digest Number of Repetitions or Target Time
        // @description Depending on the <m>timeunit</m> attribute, this might set the number of
        // times the input buffers are repeated (default), or a target time in milliseconds or samples

        t_llll *args = llll_parse(true_ac, argv);
        t_llll *names = earsbufobj_extract_names_from_args((t_earsbufobj *)x, args);
        
        x->e_ob.l_timeunit = EARS_TIMEUNIT_DURATION_RATIO;
        
        if (args && args->l_head) {
            llll_clear(x->howmany);
            llll_appendhatom_clone(x->howmany, &args->l_head->l_hatom);
        }
        
        attr_args_process(x, argc, argv);

        earsbufobj_setup((t_earsbufobj *)x, "E4", "E", names);

        llll_free(args);
        llll_free(names);
        
    }
    return x;
}


void buf_repeat_free(t_buf_repeat *x)
{
    llll_free(x->howmany);
    earsbufobj_free((t_earsbufobj *)x);
}




void buf_repeat_bang(t_buf_repeat *x)
{
    long num_buffers = earsbufobj_get_instore_size((t_earsbufobj *)x, 0);
    
    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_buffers, true);
    
    earsbufobj_mutex_lock((t_earsbufobj *)x);
    earsbufobj_init_progress((t_earsbufobj *)x, num_buffers);

    t_llllelem *el = x->howmany->l_head;
    for (long count = 0; count < num_buffers; count++, el = el && el->l_next ? el->l_next : el) {
        t_buffer_obj *in = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, count);
        t_buffer_obj *out = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, count);

        long out_samples = earsbufobj_time_to_samps((t_earsbufobj *)x, hatom_getdouble(&el->l_hatom), in);
        
        ears_buffer_repeat((t_object *)x, in, out, out_samples);

        if (earsbufobj_iter_progress((t_earsbufobj *)x, count, num_buffers)) break;
    }
    
    earsbufobj_mutex_unlock((t_earsbufobj *)x);

    earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
}


void buf_repeat_anything(t_buf_repeat *x, t_symbol *msg, long ac, t_atom *av)
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
            
            buf_repeat_bang(x);
            
        } else if (inlet == 1) {
            llll_free(x->howmany);
            x->howmany = llll_clone(parsed);
        }
    }
    llll_free(parsed);
}


