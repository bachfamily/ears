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
    t_object x_obj;
    void *outlets[EARS_IN_MAX_OUTLETS];
    long inlet_nums[EARS_IN_MAX_OUTLETS];
    long nOutlets;
    t_object* earsProcessParent;
} t_ears_in;


t_ears_in *ears_in_new(t_symbol *s, t_atom_long ac, t_atom* av);
void ears_in_free(t_ears_in *x);
void ears_in_assist(t_ears_in *x, void *b, long m, long a, char *s);

void ears_in_bang(t_ears_in *x);
void ears_int(t_ears_in *x, t_atom_long i);
void ears_in_float(t_ears_in *x, t_atom_float f);
void ears_in_anything(t_ears_in *x, t_symbol *s, long ac, t_atom *av);


int C74_EXPORT main()
{
    common_symbols_init();
    llllobj_common_symbols_init();
    
    ears_in_class = class_new("ears.in",
                              (method)ears_in_new,
                              (method)ears_in_free,
                              sizeof(t_ears_in),
                              NULL,
                              A_GIMME,
                              0);
    
    class_addmethod(ears_in_class, (method)ears_in_assist, "assist", A_CANT, 0);

    class_register(CLASS_BOX, ears_in_class);
    
    return 0;
}

t_ears_in *ears_in_new(t_symbol *s, t_atom_long ac, t_atom* av)
{
    t_ears_in *x = (t_ears_in*) object_alloc(ears_in_class);
    x->earsProcessParent = getParentEarsProcess((t_object *) x);
    
    // @arg 0 @name inlets @optional 1 @type number/list @digest ears.process~ Message Inlet Numbers
    // @description The numbers of the message inlets of <o>ears.process~</o>
    // (conted from 1, starting from the leftmost)
    // to be passed to <o>ears.in</o>.
    // Default is 1.
    
    if (ac > EARS_IN_MAX_OUTLETS) {
        object_error((t_object *) x, "Too many outlets, cropping to %d", EARS_IN_MAX_OUTLETS);
        ac = EARS_IN_MAX_OUTLETS;
    }
    
    if (ac > 0) {
        x->nOutlets = ac;
        for (int i = ac - 1; i >= 0; i--) {
            t_atom_long v = atom_getlong(av+i);
            if (v < 1) {
                object_error((t_object *) x, "Invalid inlet number at position %d, setting to 1", i + 1);
                v = 1;
            }
            x->inlet_nums[i] = v;
            x->outlets[i] = outlet_new(x, NULL);
        }
    } else {
        x->nOutlets = 1;
        x->inlet_nums[0] = 1;
        x->outlets[0] = outlet_new(x, NULL);
    }
    
    if (x->earsProcessParent)
        object_method(x->earsProcessParent, gensym("ears.in_created"), x->nOutlets, x->inlet_nums, x->outlets);
    
    return x;
}

void ears_in_free(t_ears_in *x)
{
    if (x->earsProcessParent) {
        object_method(x->earsProcessParent, gensym("ears.in_deleted"), x->nOutlets, x->inlet_nums, x->outlets);
    }
}

void ears_in_assist(t_ears_in *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_OUTLET)
        sprintf(s,"(signal) From Message Input %ld of ears.process~", (long) x->inlet_nums[a]); // @out 0 @type signal @loop 1 @digest Input multichannel signal
    else
        sprintf(s,"(int) Dummy");
}
