//
//  ears.fromvector~.cpp
//  lib_ears
//
//  Created by andreaagostini on 03/04/2021.
//


#include "ears.process_commons.h"
#include <z_dsp.h>

t_class *ears_fromvector_class;

typedef struct _ears_fromvector
{
    t_pxobject x_obj;
    void *outlet;
    t_object* earsProcessParent;
} t_ears_fromvector;


void *ears_fromvector_new(t_symbol *s, t_atom_long ac, t_atom* av);
void ears_fromvector_free(t_ears_fromvector *x);

void ears_fromvector_assist(t_ears_fromvector *x, void *b, long m, long a, char *s);

void ears_fromvector_dsp64(t_ears_fromvector *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags);
void ears_fromvector_perform64(t_ears_fromvector *x, t_dspchain *dsp64, double **ins, long numins, double **outs, long numouts, long vec_size, long flags, void *userparam);



int C74_EXPORT main()
{
    common_symbols_init();
    llllobj_common_symbols_init();
    
    ears_fromvector_class = class_new("ears.fromvector~",
                                    (method) ears_fromvector_new,
                                    (method) ears_fromvector_free,
                                    sizeof(t_ears_fromvector),
                                    NULL,
                                    0);
    
    class_addmethod(ears_fromvector_class, (method)ears_fromvector_dsp64, "dsp64", A_CANT, 0);
    
    class_dspinit(ears_fromvector_class);
    
    class_register(CLASS_BOX, ears_fromvector_class);
    
    return 0;
}

void *ears_fromvector_new(t_symbol *s, t_atom_long ac, t_atom* av)
{
    t_ears_fromvector *x = (t_ears_fromvector*) object_alloc(ears_fromvector_class);
    x->earsProcessParent = getParentEarsProcess((t_object *) x);
    
    dsp_setup((t_pxobject *) x, 1);
    x->x_obj.z_misc |= Z_PUT_FIRST;

    x->outlet = listout(x);
    
    return x;
}

void ears_fromvector_free(t_ears_fromvector *x)
{
    dsp_free((t_pxobject*) x);
}

void ears_fromvector_assist(t_ears_fromvector *x, void *b, long m, long a, char *s)
{
    
}

void ears_fromvector_perform64(t_ears_fromvector *x, t_dspchain *dsp64, double **ins, long numins, double **outs, long numouts, long vec_size, long flags, void *userparam)
{
    t_atom l[EARS_PROCESS_MAX_VS];
    double *in = ins[0];
    t_atom *lNow = l;
    int i;
    for (i = 0; i < vec_size && i < EARS_PROCESS_MAX_VS; i++) {
        atom_setfloat(lNow++, *(in++));
    }
    outlet_list(x->outlet, nullptr, i, l);
}

void ears_fromvector_dsp64(t_ears_fromvector *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
    object_method(dsp64, gensym("dsp_add64"), x, ears_fromvector_perform64, 0, NULL);
    if (!x->earsProcessParent) {
        object_warn((t_object *) x, "Can cause trouble if used outside ears.process~");
    }
}
