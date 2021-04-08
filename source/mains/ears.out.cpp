//
//  ears.out.cpp
//  lib_ears
//
//  Created by andreaagostini on 03/04/2021.
//


#include "ears.map.h"

t_class *ears_out_class;


typedef struct _ears_out
{
    t_object x_obj;
    void *x_outlet; // the outlet of the host object
    long outlet_num;
    t_object* earsMapParent;
} t_ears_out;


void *ears_out_new(t_atom_long outlet_num);
void ears_out_assist(t_ears_out *x, void *b, long m, long a, char *s);

void ears_out_bang(t_ears_out *x);
void ears_out_int(t_ears_out *x, t_atom_long i);
void ears_out_float(t_ears_out *x, t_atom_float f);
void ears_out_anything(t_ears_out *x, t_symbol *s, long ac, t_atom *av);

void ears_out_setoutlet(t_ears_out *x, t_atom_long index, void* out);


int C74_EXPORT main()
{
    ears_out_class = class_new("ears.out",
                           (method)ears_out_new,
                           NULL,
                           sizeof(t_ears_out),
                           NULL,
                           A_LONG,
                           0);
    
    
    class_addmethod(ears_out_class, (method) ears_out_bang, "bang", 0);
    class_addmethod(ears_out_class, (method) ears_out_int, "int", A_LONG, 0);
    class_addmethod(ears_out_class, (method) ears_out_float, "float", A_FLOAT, 0);
    class_addmethod(ears_out_class, (method) ears_out_anything, "list", A_GIMME, 0);
    class_addmethod(ears_out_class, (method) ears_out_anything, "anything", A_GIMME, 0);
    
    class_addmethod(ears_out_class, (method) ears_out_setoutlet, "setoutlet", A_CANT, 0);

    class_register(CLASS_BOX, ears_out_class);
    
    return 0;
}

void ears_out_bang(t_ears_out *x)
{
    if (x->x_outlet)
        outlet_bang(x->x_outlet);
}

void ears_out_int(t_ears_out *x, t_atom_long i)
{
    if (x->x_outlet)
        outlet_int(x->x_outlet, i);
}

void ears_out_float(t_ears_out *x, t_atom_float f)
{
    if (x->x_outlet)
        outlet_float(x->x_outlet, f);
}

void ears_out_anything(t_ears_out *x, t_symbol *s, long ac, t_atom *av)
{
    if (x->x_outlet)
        outlet_anything(x->x_outlet, s, ac, av);
}

void ears_out_setoutlet(t_ears_out *x, t_atom_long index, void* out)
{
    x->x_outlet = out;
    // index will be useful with multiple-outlet ears.out objects
}


void *ears_out_new(t_atom_long outlet_num)
{
    t_ears_out *x = (t_ears_out *) object_alloc(ears_out_class);
    x->earsMapParent = getParentEarsMap((t_object *) x);
    x->outlet_num = outlet_num;
    
    dsp_setup((t_pxobject *) x, 1);

    if (x->earsMapParent)
        object_method(x->earsMapParent, gensym("ears.out_created"), x->outlet_num, x->x_outlet);
    
    return x;
}

void ears_out_free(t_ears_out *x)
{
    if (x->earsMapParent)
        object_method(x->earsMapParent, gensym("ears.out_deleted"), x->outlet_num, x->x_outlet);
}

void ears_out_assist(t_ears_out *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_OUTLET)
        sprintf(s,"(signal) Signal Input %ld of Patcher", (long) x->outlet_num);
    else
        sprintf(s,"(int) Inlet Number");
}


