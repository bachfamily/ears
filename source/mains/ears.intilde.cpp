//
//  ears.in.cpp
//  lib_ears
//
//  Created by andreaagostini on 03/04/2021.
//


#include "ears.map.h"
#include <z_dsp.h>
#include <ext_buffer.h>

t_class *ears_intilde_class;


typedef struct _ears_intilde
{
    t_pxobject x_obj;
    t_atom_long bufIndex;
    t_atom_long chan;
    t_object* earsMapParent;
    bufferData* bufs;
    t_atom_long position;
} t_ears_intilde;


void *ears_intilde_new(t_symbol *s, t_atom_long ac, t_atom* av);
void ears_intilde_assist(t_ears_intilde *x, void *b, long m, long a, char *s);

void ears_intilde_bang(t_ears_intilde *x);
void ears_intildet(t_ears_intilde *x, t_atom_long i);
void ears_intilde_float(t_ears_intilde *x, t_atom_float f);
void ears_intilde_anything(t_ears_intilde *x, t_symbol *s, long ac, t_atom *av);

void ears_intilde_setbuffers(t_ears_intilde *x, bufferData* bufs);

void ears_intilde_dsp64(t_ears_intilde *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags);
void ears_intilde_perform64(t_ears_intilde *x, t_dspchain *dsp64, double **ins, long numins, double **outs, long numouts, long vec_size, long flags, void *userparam);




int C74_EXPORT main()
{
    ears_intilde_class = class_new("ears.in~",
                              (method)ears_intilde_new,
                              NULL,
                              sizeof(t_ears_intilde),
                              NULL,
                              A_GIMME,
                              0);
    
    class_addmethod(ears_intilde_class, (method)ears_intilde_dsp64, "dsp64", A_CANT, 0);

    class_addmethod(ears_intilde_class, (method)ears_intilde_setbuffers, "setbuffers", A_CANT, 0);
    
    class_dspinit(ears_intilde_class);

    class_register(CLASS_BOX, ears_intilde_class);
    
    return 0;
}

void *ears_intilde_new(t_symbol *s, t_atom_long ac, t_atom* av)
{
    t_ears_intilde *x = (t_ears_intilde*) object_alloc(ears_intilde_class);
    x->earsMapParent = getParentEarsMap((t_object *) x);
    
    if (ac > 0) {
        x->bufIndex = atom_getlong(av);
        ac--; av++;
    }
    
    if (ac > 0) {
        x->chan = atom_getlong(av);
    }
    
    if (x->bufIndex < 1 || x->bufIndex > EARSMAP_MAX_INPUT_BUFFERS)
        x->bufIndex = 1;
    
    if (x->chan < 1)
        x->chan = 1;

    if (x->earsMapParent)
        object_method(x->earsMapParent, gensym("ears.in~_created"), x->bufIndex, x);
    
    outlet_new((t_object *) x, "signal");
    
    return x;
}

void ears_intilde_free(t_ears_intilde *x)
{
    if (x->earsMapParent)
        object_method(x->earsMapParent, gensym("ears.in~_deleted"), x);
    dsp_free((t_pxobject *) x);
}


void ears_intilde_assist(t_ears_intilde *x, void *b, long m, long a, char *s)
{

}

void ears_intilde_perform64(t_ears_intilde *x, t_dspchain *dsp64, double **ins, long numins, double **outs, long numouts, long vec_size, long flags, void *userparam)
{
    t_sample *out = outs[0];
    
    unsigned short bufIndex = x->bufIndex;
    t_atom_long chan = x->chan;
    bufferData *buf = x->bufs ? x->bufs + bufIndex - 1 : nullptr;
    int s;
    t_atom_long bufchans = buf ? buf->chans : 0;

    if (!buf || !buf->obj || bufchans < chan) {
        for (s = 0; s < vec_size; s++)
            *(out++) = 0;
        x->position += vec_size;
        return;
    }
    
    t_atom_long pos = x->position;
    float *tab = buf->samps + (pos * bufchans) + chan - 1;
    t_atom_long frames = buf->frames;
    
    for (s = 0; s < vec_size && pos < frames; s++, pos++) {
        *(out++) = *tab;
        tab += bufchans;
    }
    
    for ( ; s < vec_size; s++) {
        *(out++) = 0;
    }
    
    x->position += vec_size;
}

void ears_intilde_dsp64(t_ears_intilde *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
    x->position = 0;
    object_method(dsp64, gensym("dsp_add64"), x, ears_intilde_perform64, 0, NULL);
}

void ears_intilde_setbuffers(t_ears_intilde *x, bufferData* bufs)
{
    x->bufs = bufs;
}



