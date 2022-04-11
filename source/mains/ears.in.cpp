/**
 @file
 ears.in.c
 
 @name
 ears.in
 
 @realname
 ears.in
 
 @type
 object
 
 @module
 ears
 
 @author
 Andrea Agostini, partly based upon work by Alexander J. Harker
 
 @digest
 Message input for a patch loaded by ears.process~
 
 @description
 Use the <o>ears.in~</o> object inside a patch loaded by ears.process~
 to create a multichannel signal inlet receiving data from the parent patch.
 
 @discussion
 
 @category
 ears process
 
 @keywords
 buffer, offline, patch, patcher, non-realtime
 
 @seealso
 ears.in~, ears.out, ears.out~
 
 @owner
 Andrea Agostini
 */


#include "ears.process_commons.h"

t_class *ears_in_class;

const int EARS_IN_MAX_OUTLETS = 256;

typedef struct _ears_in
{
    t_llllobj_object n_obj;
    long inlet_nums[EARS_IN_MAX_OUTLETS];
    long nOutlets;
    long direct;
    t_object* earsProcessParent;
} t_ears_in;


t_ears_in *ears_in_new(t_symbol *s, t_atom_long ac, t_atom* av);
void ears_in_free(t_ears_in *x);
void ears_in_assist(t_ears_in *x, void *b, long m, long a, char *s);

void ears_in_bang(t_ears_in *x);
void ears_int(t_ears_in *x, t_atom_long i);
void ears_in_float(t_ears_in *x, t_atom_float f);
void ears_in_anything(t_ears_in *x, t_symbol *s, long ac, t_atom *av);

void ears_in_llll(t_ears_in *x, t_llll *ll, long inlet);
void ears_in_iteration(t_ears_in *x, long n);

int C74_EXPORT main()
{
    common_symbols_init();
    llllobj_common_symbols_init();
    
    if (llllobj_check_version(bach_get_current_llll_version()) || llllobj_test()) {
        ears_error_bachcheck();
        return 1;
    }
    
    CLASS_NEW_CHECK_SIZE(ears_in_class, "ears.in",
                         (method)ears_in_new,
                         (method)ears_in_free,
                         sizeof(t_ears_in),
                         (method) ears_in_free,
                         A_GIMME,
                         0);
    
    class_addmethod(ears_in_class, (method)ears_in_llll, "llll", A_CANT, 0);
    class_addmethod(ears_in_class, (method)ears_in_iteration, "iteration", A_CANT, 0);

    class_addmethod(ears_in_class, (method)ears_in_assist, "assist", A_CANT, 0);

    CLASS_ATTR_LONG(ears_in_class, "direct", 0, t_ears_in, direct);
    CLASS_ATTR_STYLE_LABEL(ears_in_class, "direct", 0, "onoff", "Direct");
    CLASS_ATTR_FILTER_CLIP(ears_in_class, "direct", 0, 1);
    // @description
    // When set to 1, if a buffer inlet receives a single buffer
    // while other inlets receive lists of buffers,
    // then the single buffer will be iterated repeatedly against the list of buffers, until the end of the shortest list.<br/>
    // When set to 0 (as per the default), if a buffer inlet receives a single buffer
    // no iterator will be performed, and only the first buffer of each inlet will be processed.
    
    llllobj_class_add_default_bach_attrs_and_methods(ears_in_class, LLLL_OBJ_VANILLA);

    class_register(CLASS_BOX, ears_in_class);
    
    return 0;
}

void ears_in_llll(t_ears_in *x, t_llll *ll, long inlet)
{
    if (x->direct) {
        for (int o = x->nOutlets - 1; o >= 0; o--) {
            if (x->inlet_nums[o] == inlet)
                llllobj_outlet_llll((t_object *) x, LLLL_OBJ_VANILLA, o, ll);
        }
    } else {
        for (int o = 0; o < x->nOutlets; o++) {
            if (x->inlet_nums[o] == inlet)
                llllobj_store_llll((t_object *) x, LLLL_OBJ_VANILLA, llll_retain(ll), o);
        }
    }
}

void ears_in_iteration(t_ears_in *x, long n)
{
    if (x->direct)
        return;
    for (int s = x->n_obj.l_numstores - 1; s >= 0; s--) {
        t_llll *ll = llllobj_get_store_contents((t_object *) x, LLLL_OBJ_VANILLA, s, 0);
        t_llllelem *el = llll_getindex(ll, n, I_STANDARD);
        if (el) {
            t_hatom *h = &el->l_hatom;
            t_llll *outll = llll_get();
            llll_appendhatom_clone(outll, h);
            llllobj_outlet_llll((t_object *) x, LLLL_OBJ_VANILLA, s, outll);
            llll_free(outll);
        }
    }
}

t_ears_in *ears_in_new(t_symbol *s, t_atom_long ac, t_atom* av)
{
    t_ears_in *x = (t_ears_in*) object_alloc(ears_in_class);
    x->earsProcessParent = getParentEarsProcess((t_object *) x);

    
    long true_ac = attr_args_offset(ac, av);

    // @arg 0 @name inlets @optional 1 @type number/list @digest ears.process~ Message Inlet Numbers
    // @description The numbers of the message inlets of <o>ears.process~</o>
    // (conted from 1, starting from the leftmost)
    // to be passed to <o>ears.in</o>.
    // Default is 1.
    
    if (true_ac > EARS_IN_MAX_OUTLETS) {
        object_error((t_object *) x, "Too many outlets, cropping to %d", EARS_IN_MAX_OUTLETS);
        ac = EARS_IN_MAX_OUTLETS;
    }
    
    if (true_ac > 0) {
        x->nOutlets = true_ac;
        for (int i = true_ac - 1; i >= 0; i--) {
            t_atom_long v = atom_getlong(av+i);
            if (v < 1) {
                object_error((t_object *) x, "Invalid inlet number at position %d, setting to 1", i + 1);
                v = 1;
            }
            x->inlet_nums[i] = v;
        }
    } else {
        x->nOutlets = 1;
        x->inlet_nums[0] = 1;
    }
    attr_args_process(x, ac, av);
    
    char outlets[LLLL_MAX_OUTLETS];
    int i;
    for (i = 0; i < x->nOutlets; i++) {
        outlets[i] = '4';
    }
    outlets[i] = 0;
    llllobj_obj_setup((t_llllobj_object *) x, x->nOutlets, outlets);
    
    if (x->earsProcessParent) {
        object_method(x->earsProcessParent, gensym("ears.in_created"), x, x->nOutlets, x->inlet_nums);
    } else {
        object_warn((t_object *) x, "ears.in only works in ears.process~");
    }

    
    return x;
}

void ears_in_free(t_ears_in *x)
{
    if (x->earsProcessParent) {
        object_method(x->earsProcessParent, gensym("ears.in_deleted"), x, x->nOutlets, x->inlet_nums);
    }
    llllobj_obj_free((t_llllobj_object *) x);
}

void ears_in_assist(t_ears_in *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_OUTLET)
        sprintf(s,"(signal) From Message Input %ld of ears.process~", (long) x->inlet_nums[a]); // @out 0 @type signal @loop 1 @digest Input multichannel signal
    else
        sprintf(s,"(int) Dummy");
}
