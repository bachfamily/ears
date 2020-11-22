/**
	@file
	ears.pack.c
 
	@name
	ears.pack~
 
	@realname
	ears.pack~
 
	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	Pack buffer channels
 
	@description
	Combines all buffer channels into a single buffer
 
	@discussion
 
	@category
	ears basic
 
	@keywords
	buffer, pack, combine, channel, buffer
 
	@seealso
	ears.unpack~, ears.channel~
	
	@owner
	Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "llllobj.h"
#include "llll_commons_ext.h"
#include "bach_math_utilities.h"
#include "ears.object.h"



typedef struct _buf_pack {
    t_earsbufobj       e_ob;
    
    t_llll				*n_triggers;
    t_bach_atomic_lock	n_triggers_lock;
    
} t_buf_pack;



// Prototypes
t_buf_pack*         buf_pack_new(t_symbol *s, short argc, t_atom *argv);
void			buf_pack_free(t_buf_pack *x);
void			buf_pack_bang(t_buf_pack *x);
void			buf_pack_anything(t_buf_pack *x, t_symbol *msg, long ac, t_atom *av);

void buf_pack_assist(t_buf_pack *x, void *b, long m, long a, char *s);
void buf_pack_inletinfo(t_buf_pack *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(pack)


DEFINE_LLLL_ATTR_DEFAULT_GETTER(t_buf_pack, n_triggers, buf_pack_getattr_triggers);
t_max_err buf_pack_setattr_triggers(t_buf_pack *x, t_object *attr, long ac, t_atom *av);
t_max_err buf_pack_check_triggers_llll(t_buf_pack *x, t_llll *ll);




/**********************************************************************/
// Class Definition and Life Cycle


t_max_err buf_pack_check_triggers_llll(t_buf_pack *x, t_llll *ll)
{
    t_llllelem *elem;
    for (elem = ll->l_head; elem; elem = elem->l_next) {
        if (!hatom_is_number(&elem->l_hatom)) {
            object_error((t_object *) x, "Bad triggers llll");
            return MAX_ERR_GENERIC;
        }
    }
    return MAX_ERR_NONE;
}

t_max_err buf_pack_setattr_triggers(t_buf_pack *x, t_object *attr, long ac, t_atom *av)
{
    t_llll *ll;
    if (ac == 0 || av) {
        if ((ll = llllobj_parse_llllattr_llll((t_object *) x, LLLL_OBJ_VANILLA, ac, av))) {
            t_llll *free_me;
            if (buf_pack_check_triggers_llll(x, ll) != MAX_ERR_NONE) {
                llll_free(ll);
                return MAX_ERR_NONE;
            }
            bach_atomic_lock(&x->n_triggers_lock);
            free_me = x->n_triggers;
            x->n_triggers = ll;
            bach_atomic_unlock(&x->n_triggers_lock);
            llll_free(free_me);
        }
    }
    return MAX_ERR_NONE;
}



int C74_EXPORT main(void)
{
    common_symbols_init();
    llllobj_common_symbols_init();
    
    if (llllobj_check_version(bach_get_current_llll_version()) || llllobj_test()) {
        ears_error_bachcheck();
        return 1;
    }
    
    t_class *c;
    
    CLASS_NEW_CHECK_SIZE(c, "ears.pack~",
                         (method)buf_pack_new,
                         (method)buf_pack_free,
                         sizeof(t_buf_pack),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    // @method list/llll @digest Process buffers
    // @description A list or llll with buffer names in the first inlet will trigger the packing of all channels
    // as a single buffer and output the resulting buffer name.
    // Alternatively, a buffer name for each inlet can be provided (in symbol or llll form); the first inlet
    // will also trigger the channel packing and the output.
    EARSBUFOBJ_DECLARE_COMMON_METHODS_DEFER(pack)
    
    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_naming_attr(c);

    
    CLASS_ATTR_LLLL(c, "triggers", 0, t_buf_pack, n_triggers, buf_pack_getattr_triggers, buf_pack_setattr_triggers);
    CLASS_ATTR_LABEL(c, "triggers", 0, "Triggers");
    // @description A list setting which inlets are "hot" (i.e., which will will trigger the result).
    // Inlets are counted from 1. 0 means that all inlets are hot.
    // Negative indices are counted from the right (e.g., -1 means the rightmost inlet).
    // <m>null</m> means that all inlets are cold,
    // but a <m>bang</m> in any inlet will still cause the llll to be output.
    // The default is 1.

    
    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
    return 0;
}

void buf_pack_assist(t_buf_pack *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET)
        sprintf(s, "symbol/list/llll: Buffer"); // @in 0 @loop 1 @type symbol/llll @digest Buffer to be combined to the other ones
    else {
        sprintf(s, "symbol: Output buffer"); // @out 0 @type symbol @digest Output buffer
    }
}

long buf_pack_ishot(t_buf_pack *x, long inlet)
{
    long hot = 0;
    bach_atomic_lock(&x->n_triggers_lock);
    long size = x->n_triggers->l_size;
    if (size == 0)
        hot = 0;
    else {
        long numinlets = x->e_ob.l_numins;
        t_llllelem *elem;
        for (elem = x->n_triggers->l_head; elem; elem = elem->l_next) {
            long this_trigger = hatom_getlong(&elem->l_hatom);
            if (this_trigger == 0 ||
                this_trigger == inlet + 1 ||
                (this_trigger < 0 && this_trigger == inlet - numinlets))
                hot = 1;
        }
    }
    bach_atomic_unlock(&x->n_triggers_lock);
    return hot;
}


void buf_pack_inletinfo(t_buf_pack *x, void *b, long a, char *t)
{
    if (a)
        *t = !buf_pack_ishot(x, a);
}


t_buf_pack *buf_pack_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_pack *x;
    long true_ac = attr_args_offset(argc, argv);
    long proxies = 0;
    
    x = (t_buf_pack*)object_alloc_debug(s_tag_class);
    if (x) {
        earsbufobj_init((t_earsbufobj *)x, 0);
        
        t_llll *args = llll_parse(true_ac, argv);
        t_llll *names = earsbufobj_extract_names_from_args((t_earsbufobj *)x, args);
        
        // @arg 0 @name outnames @optional 1 @type symbol
        // @digest Output buffer names
        // @description @copy EARS_DOC_OUTNAME_ATTR

        // @arg 1 @name pack @optional 1 @type number
        // @digest Number of inlets
        // @description Sets the number of inlets (use 1, or nothing, to have a single inlet, in which you can feed a list of buffers).
        
        proxies = args && args->l_head && hatom_gettype(&args->l_head->l_hatom) == H_LONG ? MAX(hatom_getlong(&args->l_head->l_hatom) - 1, 0) : 0;
        
        attr_args_process(x, argc, argv); // this must be called before llllobj_obj_setup

        proxies = MIN(proxies, LLLL_MAX_INLETS);
        char temp[LLLL_MAX_INLETS + 2];
        long i = 0;
        temp[0] = 'E';
        for (i = 0; i < proxies; i++)
            temp[i+1] = 'E';
        temp[i+1] = 0;
        
        earsbufobj_setup((t_earsbufobj *)x, temp, "e", names);
        
        llll_free(args);
        llll_free(names);
        
        if (x->n_triggers == NULL) {
            x->n_triggers = llll_get();
            llll_appendlong(x->n_triggers, 1);
        }
    }
    return x;
}


void buf_pack_free(t_buf_pack *x)
{
    llll_free(x->n_triggers);
    earsbufobj_free((t_earsbufobj *)x);
}



void buf_pack_bang(t_buf_pack *x)
{
    t_llll *buffers = llll_get();
    
    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_OUT, 0, 1, true);
    
    for (long i = 0; i < x->e_ob.l_numins; i++) {
        long num_buffers = ((t_earsbufobj *)x)->l_instore[i].num_stored_bufs;
        for (long count = 0; count < num_buffers; count++) {
            llll_appendobj(buffers, earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, i, count));
        }
    }

    t_buffer_obj *out = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, 0);
    ears_buffer_pack_from_llll((t_object *)x, buffers, out);
    
    earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
    llll_free(buffers);
}


void buf_pack_anything(t_buf_pack *x, t_symbol *msg, long ac, t_atom *av)
{
    long inlet = proxy_getinlet((t_object *) x);
        
    t_llll *parsed = earsbufobj_parse_gimme((t_earsbufobj *) x, LLLL_OBJ_VANILLA, msg, ac, av);
    if (!parsed) return;
    
    if (parsed && parsed->l_head) {
        long ishot = buf_pack_ishot(x, inlet);
        earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, parsed->l_size, true);
        earsbufobj_store_buffer_list((t_earsbufobj *)x, parsed, inlet);
        
        if (ishot)
            buf_pack_bang(x);
    }
    llll_free(parsed);
}


