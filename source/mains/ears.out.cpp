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
    t_object x_obj;
    void *earsprocess_outlets[EARS_OUT_MAX_INLETS]; // the outlet of the host object
    void *proxies[EARS_OUT_MAX_INLETS];
    long proxy_num;
    long nInlets;
    long outlet_nums[EARS_OUT_MAX_INLETS];
    t_object* earsProcessParent;
} t_ears_out;


void *ears_out_new(t_symbol *s, t_atom_long ac, t_atom* av);
void ears_out_free(t_ears_out *x);
void ears_out_assist(t_ears_out *x, void *b, long m, long a, char *s);

void ears_out_bang(t_ears_out *x);
void ears_out_int(t_ears_out *x, t_atom_long i);
void ears_out_float(t_ears_out *x, t_atom_float f);
void ears_out_anything(t_ears_out *x, t_symbol *s, long ac, t_atom *av);

void ears_out_setoutlets(t_ears_out *x, long n, void** outlets);


int C74_EXPORT main()
{
    common_symbols_init();
    llllobj_common_symbols_init();
    
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
    
    class_addmethod(ears_out_class, (method) ears_out_setoutlets, "setoutlets", A_CANT, 0);

    class_register(CLASS_BOX, ears_out_class);
    
    return 0;
}

void ears_out_bang(t_ears_out *x)
{
    long inlet = proxy_getinlet((t_object *) x);
    if (void *o = x->earsprocess_outlets[inlet]; o != nullptr)
        outlet_bang(o);
}

void ears_out_int(t_ears_out *x, t_atom_long i)
{
    long inlet = proxy_getinlet((t_object *) x);
    if (void *o = x->earsprocess_outlets[inlet]; o != nullptr)
        outlet_int(o, i);
}

void ears_out_float(t_ears_out *x, t_atom_float f)
{
    long inlet = proxy_getinlet((t_object *) x);
    if (void *o = x->earsprocess_outlets[inlet]; o != nullptr)
        outlet_float(o, f);
}

void ears_out_anything(t_ears_out *x, t_symbol *s, long ac, t_atom *av)
{
    long inlet = proxy_getinlet((t_object *) x);
    if (void *o = x->earsprocess_outlets[inlet]; o != nullptr)
        outlet_anything(o, s, ac, av);
}

void ears_out_setoutlets(t_ears_out *x, long n, void** out)
{
    for (int i = 0; i < x->nInlets; i++) {
        long o = x->outlet_nums[i];
        if (o <= n)
            x->earsprocess_outlets[i] = out[o - 1];
    }
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
    
    if (ac > EARS_OUT_MAX_INLETS) {
        object_error((t_object *) x, "Too many inlets, cropping to %d", EARS_OUT_MAX_INLETS);
        ac = EARS_OUT_MAX_INLETS;
    }
    
    long maxidx = 0;
    if (ac > 0) {
        x->nInlets = ac;
        for (int i = ac - 1; i > 0; i--) {
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
    
    if (x->earsProcessParent)
        object_method(x->earsProcessParent, gensym("ears.out_created"),
                      maxidx, x);
    return x;
}

void ears_out_free(t_ears_out *x)
{
    if (x->earsProcessParent)
        object_method(x->earsProcessParent, gensym("ears.out_deleted"), x);
}

void ears_out_assist(t_ears_out *x, void *b, long m, long a, char *s)
{
    sprintf(s,"To Message Output %ld of ears.process~", (long) x->outlet_nums[a]); // @out 0 @type signal @loop 1 @digest Input multichannel signal
}


