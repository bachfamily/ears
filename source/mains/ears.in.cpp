//
//  ears.in.cpp
//  lib_ears
//
//  Created by andreaagostini on 03/04/2021.
//


#include "ears.process_commons.h"

t_class *ears_in_class;

const int EARS_IN_MAX_OUTLETS = 256;

typedef struct _ears_in
{
    t_object x_obj;
    void *outlets[EARS_IN_MAX_OUTLETS];
    long outlet_nums[EARS_IN_MAX_OUTLETS];
    long nOutlets;
    t_object* earsProcessParent;
} t_ears_in;


t_ears_in *ears_in_new(t_symbol *s, t_atom_long ac, t_atom* av);
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
                           NULL,
                           sizeof(t_ears_in),
                           NULL,
                           A_GIMME,
                           0);
    
    class_register(CLASS_BOX, ears_in_class);
    
    return 0;
}

t_ears_in *ears_in_new(t_symbol *s, t_atom_long ac, t_atom* av)
{
    t_ears_in *x = (t_ears_in*) object_alloc(ears_in_class);
    x->earsProcessParent = getParentEarsProcess((t_object *) x);
    
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
            x->outlet_nums[i] = v;
            x->outlets[i] = outlet_new(x, NULL);
        }
    } else {
        x->nOutlets = 1;
        x->outlet_nums[0] = 1;
        x->outlets[0] = outlet_new(x, NULL);
    }
    
    if (x->earsProcessParent)
        object_method(x->earsProcessParent, gensym("ears.in_created"), x->nOutlets, x->outlet_nums, x->outlets);
    
    return x;
}

void ears_in_free(t_ears_in *x)
{
    if (x->earsProcessParent) {
        object_method(x->earsProcessParent, gensym("ears.in_deleted"), x->nOutlets, x->outlet_nums, x->outlets);
    }
}


void ears_in_assist(t_ears_in *x, void *b, long m, long a, char *s)
{
    /*
    if (m == ASSIST_OUTLET)
        sprintf(s,"(signal) Signal Input %ld of Patcher", (long) x->inlet_num);
    else
        sprintf(s,"(int) Inlet Number");
     */
}


