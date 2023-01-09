/**
	@file
	ears.delace.c
 
	@name
	ears.delace~
 
	@realname
	ears.delace~
 
	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	De-interleave buffer channels
 
	@description
	De-interleave the channels of two buffers
 
	@discussion
 
	@category
	ears basic
 
	@keywords
	buffer, delace, channel, pack, interleave, buffer, combine
 
	@seealso
	ears.dedelace~, ears.channel~
	
	@owner
	Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "foundation/llllobj.h"
#include "foundation/llll_commons_ext.h"
#include "math/bach_math_utilities.h"
#include "ears.object.h"



typedef struct _buf_delace {
    t_earsbufobj       e_ob;
    long               e_count;
} t_buf_delace;



// Prototypes
t_buf_delace*         buf_delace_new(t_symbol *s, short argc, t_atom *argv);
void			buf_delace_free(t_buf_delace *x);
void			buf_delace_bang(t_buf_delace *x);
void			buf_delace_anything(t_buf_delace *x, t_symbol *msg, long ac, t_atom *av);

void buf_delace_assist(t_buf_delace *x, void *b, long m, long a, char *s);
void buf_delace_inletinfo(t_buf_delace *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(delace)




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
    
    CLASS_NEW_CHECK_SIZE(c, "ears.delace~",
                         (method)buf_delace_new,
                         (method)buf_delace_free,
                         sizeof(t_buf_delace),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    // @method symbol/llll @digest Process buffers
    // @description A symbol or an llll with a single symbol in the first inlet sets the buffer
    // to be de-interleaved; the de-interleaved buffers are then output from the
    // appropriate outlets.
    EARSBUFOBJ_DECLARE_COMMON_METHODS_HANDLETHREAD(delace)
    
    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_blocking_attr(c);
    earsbufobj_class_add_naming_attr(c);

    
    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
}

void buf_delace_assist(t_buf_delace *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        sprintf(s, "symbol: Input buffer"); // @in 0 @type symbol/llll @digest Input buffer
    } else {
        sprintf(s, "symbol: Deinterleaved buffer No. %ld", a + 1); // @out 0 @loop 1 @type symbol @digest De-interleaved buffers
    }
}

void buf_delace_inletinfo(t_buf_delace *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_delace *buf_delace_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_delace *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_delace*)object_alloc_debug(s_tag_class);
    if (x) {
        x->e_count = 2;
        earsbufobj_init((t_earsbufobj *)x, 0);
        
        t_llll *args = llll_parse(true_ac, argv);
        t_llll *names = earsbufobj_extract_names_from_args((t_earsbufobj *)x, args);

        // @arg 0 @name outnames @optional 1 @type symbol
        // @digest Output buffer names
        // @description @copy EARS_DOC_OUTNAME_ATTR

        // @arg 1 @name count @optional 1 @type int
        // @digest Number of output buffers

        if (args && args->l_head && hatom_gettype(&args->l_head->l_hatom) == H_LONG) {
            x->e_count = CLAMP(hatom_getlong(&args->l_head->l_hatom), 1, LLLL_MAX_OUTLETS);
        }

        char buf[LLLL_MAX_OUTLETS+1];
        long i = 0;
        for (; i < x->e_count; i++) {
            buf[i] = 'e';
        }
        buf[i] = 0;
        
        attr_args_process(x, argc, argv); // this must be called before llllobj_obj_setup

        earsbufobj_setup((t_earsbufobj *)x, "e", buf, names);
        
        llll_free(args);
        llll_free(names);
    }
    return x;
}


void buf_delace_free(t_buf_delace *x)
{
    earsbufobj_free((t_earsbufobj *)x);
}



void buf_delace_bang(t_buf_delace *x)
{
    long num_buffers = x->e_count;
    
    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
    for (long c = 0; c < num_buffers; c++)
        earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_OUT, c, 1, true);
    
    earsbufobj_mutex_lock((t_earsbufobj *)x);
    t_buffer_obj **bufs = (t_buffer_obj **)bach_newptr(num_buffers * sizeof(t_buffer_obj *));
    for (long i = 0; i < num_buffers; i++)
        bufs[i] = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, i, 0);
    t_buffer_obj *in = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, 0);
    
    ears_buffer_delace((t_object *)x, in, num_buffers, bufs);
    
    bach_freeptr(bufs);
    earsbufobj_mutex_unlock((t_earsbufobj *)x);

    for (long c = num_buffers-1; c >= 0; c--)
        earsbufobj_outlet_buffer((t_earsbufobj *)x, c);
}


void buf_delace_anything(t_buf_delace *x, t_symbol *msg, long ac, t_atom *av)
{
    long inlet = earsbufobj_proxy_getinlet((t_earsbufobj *) x);
        
    t_llll *parsed = earsbufobj_parse_gimme((t_earsbufobj *) x, LLLL_OBJ_VANILLA, msg, ac, av);
    if (!parsed) return;
    
    if (parsed && parsed->l_head && hatom_gettype(&parsed->l_head->l_hatom) == H_SYM) {
        earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, inlet, 1, true);
        earsbufobj_store_buffer((t_earsbufobj *)x, EARSBUFOBJ_IN, inlet, 0, hatom_getsym(&parsed->l_head->l_hatom));
        if (inlet == 0)
            buf_delace_bang(x);
    }
    llll_free(parsed);
}


