//
//  ears.tovector~.cpp
//  lib_ears
//
//  Created by andreaagostini on 03/04/2021.
//


#include "ears.map.h"
#include <z_dsp.h>

t_class *ears_tovector_class;

typedef struct _ears_tovector
{
    t_pxobject x_obj;
    t_object* earsMapParent;
    double vec[EARSMAP_MAX_VS];
    long n;
    long leftalign;
    long autoclear;
} t_ears_tovector;


void *ears_tovector_new(t_symbol *s, t_atom_long ac, t_atom* av);
void ears_tovector_free(t_ears_tovector *x);

void ears_tovector_assist(t_ears_tovector *x, void *b, long m, long a, char *s);

void ears_tovector_int(t_ears_tovector *x, t_atom_long i);
void ears_tovector_float(t_ears_tovector *x, t_atom_float f);
void ears_tovector_list(t_ears_tovector *x, t_symbol *s, long ac, t_atom *av);
void ears_tovector_clear(t_ears_tovector *x);
void ears_tovector_set(t_ears_tovector *x, t_symbol *s, long ac, t_atom *av);



void ears_tovector_dsp64(t_ears_tovector *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags);
void ears_tovector_perform64(t_ears_tovector *x, t_dspchain *dsp64, double **ins, long numins, double **outs, long numouts, long vec_size, long flags, void *userparam);



int C74_EXPORT main()
{
    ears_tovector_class = class_new("ears.tovector~",
                                    (method) ears_tovector_new,
                                    (method) ears_tovector_free,
                                    sizeof(t_ears_tovector),
                                    NULL,
                                    A_GIMME,
                                    0);
    
    class_addmethod(ears_tovector_class, (method) ears_tovector_int, "int", A_LONG, 0);
    class_addmethod(ears_tovector_class, (method) ears_tovector_float, "float", A_FLOAT, 0);
    class_addmethod(ears_tovector_class, (method) ears_tovector_list, "list",
                    A_GIMME, 0);
    class_addmethod(ears_tovector_class, (method) ears_tovector_clear, "clear", 0);
    class_addmethod(ears_tovector_class, (method) ears_tovector_set, "set",
                    A_GIMME, 0);
    
    class_addmethod(ears_tovector_class, (method)ears_tovector_dsp64, "dsp64", A_CANT, 0);
    
    CLASS_ATTR_LONG(ears_tovector_class, "leftalign", 0, t_ears_tovector, leftalign);
    CLASS_ATTR_STYLE_LABEL(ears_tovector_class, "leftalign", 0, "onoff", "Left Align");
    CLASS_ATTR_FILTER_CLIP(ears_tovector_class, "leftalign", 0, 1);
    
    CLASS_ATTR_LONG(ears_tovector_class, "autoclear", 0, t_ears_tovector, leftalign);
    CLASS_ATTR_STYLE_LABEL(ears_tovector_class, "autoclear", 0, "onoff", "Left Align");
    CLASS_ATTR_FILTER_CLIP(ears_tovector_class, "autoclear", 0, 1);
    
    class_dspinit(ears_tovector_class);
    
    class_register(CLASS_BOX, ears_tovector_class);
    
    return 0;
}

void *ears_tovector_new(t_symbol *s, t_atom_long ac, t_atom* av)
{
    t_ears_tovector *x = (t_ears_tovector*) object_alloc(ears_tovector_class);
    x->earsMapParent = getParentEarsMap((t_object *) x);
    x->autoclear = 1;
    attr_args_process(x, ac, av);
    dsp_setup((t_pxobject *) x, 0);
    x->x_obj.z_misc |= Z_PUT_LAST;

    outlet_new(x, "signal");
    return x;
}

void ears_tovector_free(t_ears_tovector *x)
{
    dsp_free((t_pxobject*) x);
}

void ears_tovector_int(t_ears_tovector *x, t_atom_long i)
{
    if (int n = x->n; n < EARSMAP_MAX_VS) {
        x->vec[n++] = i;
    }
}

void ears_tovector_float(t_ears_tovector *x, double f)
{
    if (int n = x->n; n < EARSMAP_MAX_VS) {
        x->vec[n++] = f;
    }
}

void ears_tovector_list(t_ears_tovector *x, t_symbol *s, long ac, t_atom *av)
{
    long end = MIN(ac, EARSMAP_MAX_VS);
    for (int n = x->n; n < end; n++) {
        x->vec[n] = atom_getfloat(av++);
    }
    x->n = end;
}

void ears_tovector_clear(t_ears_tovector *x)
{
    memset(x->vec, 0, x->n * sizeof(double));
    x->n = 0;
}

void ears_tovector_set(t_ears_tovector *x, t_symbol *s, long ac, t_atom *av)
{
    if (ac < 2) {
        object_error((t_object *) x, "set: not enough values");
    }
    long n = atom_getlong(av++) - 1;
    if (n < 0 || n >= EARSMAP_MAX_VS) {
        object_error((t_object *) x, "set: bad starting value");
    }

    long end = MIN(n + ac - 1, EARSMAP_MAX_VS);
    
    for (int i = n - 1; i < end; i++) {
        x->vec[i] = atom_getfloat(av++);
    }
    x->n = end;
}
                    
void ears_tovector_assist(t_ears_tovector *x, void *b, long m, long a, char *s)
{
    
}

void ears_tovector_perform64(t_ears_tovector *x, t_dspchain *dsp64, double **ins, long numins, double **outs, long numouts, long vec_size, long flags, void *userparam)
{
    if (x->leftalign) {
        memcpy(outs[0], x->vec, x->n * sizeof(double));
    } else {
        long pad = MAX(vec_size - x->n, 0);
        memset(outs[0], 0, pad);
        memcpy(outs[0] + pad, x->vec, (vec_size - pad) * sizeof(double));
    }
    if (x->autoclear)
        ears_tovector_clear(x);
}

void ears_tovector_dsp64(t_ears_tovector *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
    object_method(dsp64, gensym("dsp_add64"), x, ears_tovector_perform64, 0, NULL);
    if (!x->earsMapParent) {
        object_warn((t_object *) x, "Can cause trouble if used outside ears.map~");
    }
}
