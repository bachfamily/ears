/**
 @file
 ears.out.c
 
 @name
 ears.out
 
 @realname
 ears.out
 
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

t_class *ears_out_class;

const int EARS_OUT_MAX_INLETS = 256;

typedef struct _ears_out
{
    t_llllobj_object n_obj;
    void *proxies[EARS_OUT_MAX_INLETS];
    long proxy_num;
    long nInlets;
    long outlet_nums[EARS_OUT_MAX_INLETS];
    long direct;
    long outwrap;
    t_llll *collected[EARS_OUT_MAX_INLETS];
    t_bach_atomic_lock lock;
    t_object* earsProcessParent;
} t_ears_out;


void *ears_out_new(t_symbol *s, t_atom_long ac, t_atom* av);
void ears_out_free(t_ears_out *x);
void ears_out_assist(t_ears_out *x, void *b, long m, long a, char *s);

void ears_out_bang(t_ears_out *x);
void ears_out_int(t_ears_out *x, t_atom_long i);
void ears_out_float(t_ears_out *x, t_atom_float f);
void ears_out_anything(t_ears_out *x, t_symbol *s, long ac, t_atom *av);

void ears_out_finalize(t_ears_out *x, long n);
void ears_out_iteration(t_ears_out *x, long n);


void C74_EXPORT ext_main(void* moduleRef)
{
    common_symbols_init();
    llllobj_common_symbols_init();
    
    if (llllobj_check_version(bach_get_current_llll_version()) || llllobj_test()) {
        ears_error_bachcheck();
        return;
    }
    
    ears_out_class = class_new("ears.out",
                               (method)ears_out_new,
                               (method)ears_out_free,
                               sizeof(t_ears_out),
                               NULL,
                               A_GIMME,
                               0);
    
    // @method bang @digest Output bangs from the corresponding outlet of <o>ears.process~</o>
    // @description
    // A bang sent in any input of <o>ears.out</o> is sent out
    // from the corresponding outlet of the <o>ears.process~</o> containing the patch
    class_addmethod(ears_out_class, (method) ears_out_bang, "bang", 0);
    
    // @method int @digest Output integers from the corresponding outlet of <o>ears.process~</o>
    // @description
    // An integer sent in any input of <o>ears.out</o> is sent out
    // from the corresponding outlet of the <o>ears.process~</o> containing the patch
    class_addmethod(ears_out_class, (method) ears_out_int, "int", A_LONG, 0);
    
    // @method float @digest Output floats from the corresponding outlet of <o>ears.process~</o>
    // @description
    // A float sent in any input of <o>ears.out</o> is sent out
    // from the corresponding outlet of the <o>ears.process~</o> containing the patch
    class_addmethod(ears_out_class, (method) ears_out_float, "float", A_FLOAT, 0);
    
    // @method list @digest Output lists from the corresponding outlet of <o>ears.process~</o>
    // @description
    // A list sent in any input of <o>ears.out</o> is sent out
    // from the corresponding outlet of the <o>ears.process~</o> containing the patch
    class_addmethod(ears_out_class, (method) ears_out_anything, "list", A_GIMME, 0);
    
    // @method anything @digest Output messages from the corresponding outlet of <o>ears.process~</o>
    // @description
    // A message sent in any input of <o>ears.out</o> is sent out
    // from the corresponding outlet of the <o>ears.process~</o> containing the patch
    class_addmethod(ears_out_class, (method) ears_out_anything, "anything", A_GIMME, 0);
    
    class_addmethod(ears_out_class, (method)ears_out_iteration, "iteration", A_CANT, 0);
    class_addmethod(ears_out_class, (method)ears_out_finalize, "finalize", A_CANT, 0);

    CLASS_ATTR_LONG(ears_out_class, "direct", 0, t_ears_out, direct);
    CLASS_ATTR_STYLE_LABEL(ears_out_class, "direct", 0, "onoff", "Direct");
    CLASS_ATTR_FILTER_CLIP(ears_out_class, "direct", 0, 1);
    // @description
    // When the <m>direct</m> attribute is set to 0 (as per the default),
    // at each iteration triggered by <o>ears.process~</o>
    // an llll is collected and, at the end of all the iterations,
    // they are all chained together and output by <o>ears.process~</o>.
    // If more than one llll is received by <o>ears.out</o> during one iteration,
    // only the last one is collected.<br/>.
    // When the <m>direct</m> attribute is set to 1,
    // each llll received by <o>ears.out</o> is immediately output
    // from the corresponding outlet of <o>ears.process~</o>.
    
    CLASS_ATTR_LONG(ears_out_class, "outwrap", 0, t_ears_out, outwrap);
    CLASS_ATTR_STYLE_LABEL(ears_out_class, "outwrap", 0, "onoff", "Output Wrap");
    CLASS_ATTR_FILTER_CLIP(ears_out_class, "outwrap", 0, 1);
    // @description If the <m>outwrap</m> attribute is set to 1
    // and the <m>direct</m> attribute is set to 0,
    // the lllls collected at each iteration are returned as sublists,
    // each wrapped in parentheses, rather than just chained together
    // as per the default behavior, with <m>outwrap</m> set to 0.<br/>
    // If the <m>direct</m> attribute is set to 1, the <m>outwrap</m> attribute has no effect.
    
    class_register(CLASS_BOX, ears_out_class);
    
    return 0;
}

void ears_out_bang(t_ears_out *x)
{
    ears_out_anything(x, _sym_bang, 0, nullptr);
}

void ears_out_int(t_ears_out *x, t_atom_long i)
{
    t_atom a[1];
    atom_setlong(a, i);
    ears_out_anything(x, _sym_int, 1, a);
}

void ears_out_float(t_ears_out *x, t_atom_float f)
{
    t_atom a[1];
    atom_setfloat(a, f);
    ears_out_anything(x, _sym_float, 1, a);
}

void ears_out_list(t_ears_out *x, t_symbol *s, long ac, t_atom *av)
{
    ears_out_anything(x, s, ac, av);
}


void ears_out_anything(t_ears_out *x, t_symbol *s, long ac, t_atom *av)
{
    long inlet = proxy_getinlet((t_object *) x);
    
    if (x->direct) {
        t_llll *ll = llllobj_parse_llll((t_object *) x, LLLL_OBJ_VANILLA, s, ac, av, LLLL_PARSE_RETAIN);
        if (!ll)
            return;
        llllobj_outlet_llll(x->earsProcessParent, LLLL_OBJ_VANILLA, x->outlet_nums[inlet] - 1, ll);
        llll_release(ll);
    } else {
        llllobj_parse_and_store((t_object *) x, LLLL_OBJ_VANILLA, s, ac, av, inlet);
    }
}

void ears_out_iteration(t_ears_out *x, long n)
{
    if (x->direct)
        return;
    bach_atomic_lock(&x->lock);
    for (int i = 0; i < x->nInlets; i++) {
        t_llll *ll = llllobj_get_store_contents((t_object *) x, LLLL_OBJ_VANILLA, i, 1);
        if (x->outwrap)
            llll_appendllll(x->collected[i], ll);
        else
            llll_chain(x->collected[i], ll);
        llllobj_store_llll((t_object *) x, LLLL_OBJ_VANILLA, llll_get(), i);
    }
    bach_atomic_unlock(&x->lock);
}

void ears_out_finalize(t_ears_out *x, long n)
{
    if (x->direct)
        return;
    t_object *process = x->earsProcessParent;
    bach_atomic_lock(&x->lock);
    int i;
    for (i = 0; i < x->nInlets; i++, n++) {
        llllobj_gunload_llll((t_object *) process, LLLL_OBJ_VANILLA, x->collected[i], x->outlet_nums[i] - 1 + n);
        x->collected[i] = llll_get();
    }
    bach_atomic_unlock(&x->lock);
}

void *ears_out_new(t_symbol *s, t_atom_long ac, t_atom* av)
{
    t_ears_out *x = (t_ears_out *) object_alloc(ears_out_class);
    x->earsProcessParent = getParentEarsProcess((t_object *) x);
    
    // @arg 0 @name outlets @optional 1 @type number/list @digest ears.process~ Message Outlet Numbers
    // @description The numbers of the message outlets of <o>ears.process~</o>
    // (conted from 1, starting from the leftmost)
    // to which to pass messages received by <o>ears.in</o>.
    // Default is 1.
    
    long true_ac = attr_args_offset(ac, av);
    
    if (true_ac > EARS_OUT_MAX_INLETS) {
        object_error((t_object *) x, "Too many inlets, cropping to %d", EARS_OUT_MAX_INLETS);
        true_ac = EARS_OUT_MAX_INLETS;
    }
    
    long maxidx = 0;
    if (true_ac > 0) {
        x->nInlets = true_ac;
        for (int i = true_ac - 1; i > 0; i--) {
            t_atom_long v = atom_getlong(av+i);
            if (v < 1) {
                object_error((t_object *) x, "Invalid outlet number at position %d, setting to 1", i + 1);
                v = 1;
            }
            x->outlet_nums[i] = v;
            x->proxies[i] = proxy_new(x, i, &x->proxy_num);
            if (v > maxidx)
                maxidx = v;
        }
        t_atom_long v = atom_getlong(av);
        if (v < 1) {
            object_error((t_object *) x, "Invalid outlet number at position 1, setting to 1");
            v = 1;
        }
        if (v > maxidx)
            maxidx = v;
        x->outlet_nums[0] = v;
    } else {
        x->nInlets = 1;
        x->outlet_nums[0] = 1;
        maxidx = 1;
    }
    
    for (int i = 0; i < x->nInlets; i++)
        x->collected[i] = llll_get();
    
    x->outwrap = 1;
    
    llllobj_obj_setup((t_llllobj_object *) x, x->nInlets, "");
    attr_args_process(x, ac, av);

    if (x->earsProcessParent)
        object_method(x->earsProcessParent, gensym("ears.out_created"),
                      maxidx, x);
    return x;
}

void ears_out_free(t_ears_out *x)
{
    if (x->earsProcessParent)
        object_method(x->earsProcessParent, gensym("ears.out_deleted"), x);
    for (int i = 0; i < x->nInlets; i++)
        llll_free(x->collected[i]);
    llllobj_obj_free((t_llllobj_object *) x);
}

void ears_out_assist(t_ears_out *x, void *b, long m, long a, char *s)
{
    sprintf(s,"To Message Output %ld of ears.process~", (long) x->outlet_nums[a]); // @out 0 @type signal @loop 1 @digest Input multichannel signal
}


