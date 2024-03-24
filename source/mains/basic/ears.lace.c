/**
	@file
	ears.lace.c
 
	@name
	ears.lace~
 
	@realname
	ears.lace~
 
	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	Interleave buffer channels
 
	@description
	Interleave the channels of two buffers
 
	@discussion
 
	@category
	ears basic
 
	@keywords
	buffer, lace, channel, pack, interleave, buffer, combine
 
	@seealso
	ears.delace~, ears.channel~
	
	@owner
	Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "foundation/llllobj.h"
#include "foundation/llll_commons_ext.h"
#include "math/bach_math_utilities.h"
#include "ears.object.h"



typedef struct _buf_lace {
    t_earsbufobj       e_ob;
    long               e_count;
} t_buf_lace;



// Prototypes
t_buf_lace*         buf_lace_new(t_symbol *s, short argc, t_atom *argv);
void			buf_lace_free(t_buf_lace *x);
void			buf_lace_bang(t_buf_lace *x);
void			buf_lace_anything(t_buf_lace *x, t_symbol *msg, long ac, t_atom *av);

void buf_lace_assist(t_buf_lace *x, void *b, long m, long a, char *s);
void buf_lace_inletinfo(t_buf_lace *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(lace)




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
    
    CLASS_NEW_CHECK_SIZE(c, "ears.lace~",
                         (method)buf_lace_new,
                         (method)buf_lace_free,
                         sizeof(t_buf_lace),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    // @method symbol/llll @digest Process buffers
    // @description A symbol or an llll with a single symbol in any of the inlets will set the corresponding buffer.
    // The first inlet will trigger the interleaving of the channels of the buffers, and the output buffer name will be output.
    EARSBUFOBJ_DECLARE_COMMON_METHODS_HANDLETHREAD(lace)
    
    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_blocking_attr(c);
    earsbufobj_class_add_alloc_attr(c);

    
    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
}

void buf_lace_assist(t_buf_lace *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET)
        sprintf(s, "symbol/list/llll: Buffer %ld", a + 1); // @in 0 @loop 1 @type symbol/llll @digest Buffer to be interleaved
    else {
        sprintf(s, "symbol: Output buffer"); // @out 0 @type symbol @digest Output buffer
    }
}

void buf_lace_inletinfo(t_buf_lace *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_lace *buf_lace_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_lace *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_lace*)object_alloc_debug(s_tag_class);
    if (x) {
        x->e_count = 2;

        earsbufobj_init((t_earsbufobj *)x, 0);
        
        t_llll *args = llll_parse(true_ac, argv);
        t_llll *names = earsbufobj_extract_names_from_args((t_earsbufobj *)x, args);

        // @arg 0 @name outnames @optional 1 @type symbol
        // @digest Output buffer names
        // @description @copy EARS_DOC_OUTNAME_ATTR

        // @arg 1 @name count @optional 1 @type int
        // @digest Number of input buffers

        if (args && args->l_head && hatom_gettype(&args->l_head->l_hatom) == H_LONG) {
            x->e_count = CLAMP(hatom_getlong(&args->l_head->l_hatom), 1, LLLL_MAX_INLETS);
        }
        
        char buf[LLLL_MAX_INLETS+1];
        long i = 0;
        for (; i < x->e_count; i++) {
            buf[i] = 'e';
        }
        buf[i] = 0;
        
        attr_args_process(x, argc, argv); // this must be called before llllobj_obj_setup

        earsbufobj_setup((t_earsbufobj *)x, buf, "e", names);
        
        llll_free(args);
        llll_free(names);
    }
    return x;
}


void buf_lace_free(t_buf_lace *x)
{
    earsbufobj_free((t_earsbufobj *)x);
}



void buf_lace_bang(t_buf_lace *x)
{
    long num_buffers = x->e_count;
    
    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_OUT, 0, 1, true);
    
    earsbufobj_mutex_lock((t_earsbufobj *)x);
    t_buffer_obj **bufs = (t_buffer_obj **)bach_newptr(num_buffers * sizeof(t_buffer_obj *));
    for (long i = 0; i < num_buffers; i++)
        bufs[i] = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, i, 0);
    t_buffer_obj *out = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, 0);
    
    ears_buffer_lace((t_object *)x, num_buffers, bufs, out);
    
    bach_freeptr(bufs);
    earsbufobj_mutex_unlock((t_earsbufobj *)x);

    earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
}


void buf_lace_anything(t_buf_lace *x, t_symbol *msg, long ac, t_atom *av)
{
    long inlet = earsbufobj_proxy_getinlet((t_earsbufobj *) x);
        
    t_llll *parsed = earsbufobj_parse_gimme((t_earsbufobj *) x, LLLL_OBJ_VANILLA, msg, ac, av);
    if (!parsed) return;
    
    if (parsed && parsed->l_head && hatom_gettype(&parsed->l_head->l_hatom) == H_SYM) {
        earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, inlet, 1, true);
        earsbufobj_store_buffer((t_earsbufobj *)x, EARSBUFOBJ_IN, inlet, 0, hatom_getsym(&parsed->l_head->l_hatom));
        if (inlet == 0)
            buf_lace_bang(x);
    }
    llll_free(parsed);
}


