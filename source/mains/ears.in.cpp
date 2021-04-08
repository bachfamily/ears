//
//  ears.in.cpp
//  lib_ears
//
//  Created by andreaagostini on 03/04/2021.
//


#include "ears.map.h"

t_class *ears_in_class;


typedef struct _ears_in
{
    t_object x_obj;
    void *x_outlet;
    long inlet_num;
    t_object* earsMapParent;
} t_ears_in;


t_ears_in *ears_in_new(t_atom_long inlet_num);
void ears_in_assist(t_ears_in *x, void *b, long m, long a, char *s);

void ears_in_bang(t_ears_in *x);
void ears_int(t_ears_in *x, t_atom_long i);
void ears_in_float(t_ears_in *x, t_atom_float f);
void ears_in_anything(t_ears_in *x, t_symbol *s, long ac, t_atom *av);


int C74_EXPORT main()
{
    ears_in_class = class_new("ears.in",
                           (method)ears_in_new,
                           NULL,
                           sizeof(t_ears_in),
                           NULL,
                           A_LONG,
                           0);
    
    class_register(CLASS_BOX, ears_in_class);
    
    return 0;
}

t_ears_in *ears_in_new(t_atom_long inlet_num)
{
    t_ears_in *x = (t_ears_in*) object_alloc(ears_in_class);
    x->earsMapParent = getParentEarsMap((t_object *) x);
    x->x_outlet = outlet_new(x, NULL);
    x->inlet_num = inlet_num;
    
    dsp_setup((t_pxobject *) x, 0);
    outlet_new(x, "signal");

    if (x->earsMapParent)
        object_method(x->earsMapParent, gensym("ears.in_created"), inlet_num, x->x_outlet);
    
    return x;
}

void ears_in_free(t_ears_in *x)
{
    if (x->earsMapParent)
        object_method(x->earsMapParent, gensym("ears.in_deleted"), x->inlet_num, x->x_outlet);
}


void ears_in_assist(t_ears_in *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_OUTLET)
        sprintf(s,"(signal) Signal Input %ld of Patcher", (long) x->inlet_num);
    else
        sprintf(s,"(int) Inlet Number");
}


