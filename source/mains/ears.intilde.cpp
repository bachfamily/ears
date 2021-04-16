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

const int EARS_INTILDE_MAX_CHANS = 256;

typedef struct _ears_intilde
{
    t_pxobject x_obj;
    t_atom_long bufIndex;
    t_atom_long chan[EARS_INTILDE_MAX_CHANS];
    int nChans;
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

    class_addmethod(ears_intilde_class, (method)ears_intilde_anything, "list", A_CANT, 0);
    class_addmethod(ears_intilde_class, (method)ears_intilde_anything, "anything", A_CANT, 0);

    class_addmethod(ears_intilde_class, (method)ears_intilde_setbuffers, "setbuffers", A_CANT, 0);
    
    class_dspinit(ears_intilde_class);

    class_register(CLASS_BOX, ears_intilde_class);
    
    return 0;
}

void ears_intilde_anything(t_ears_intilde *x, t_symbol *s, long ac, t_atom *av)
{
    t_atom_long startCh;
    bach_assert_objerror_goto(x, ac >= 2, "Wrong format", ears_intilde_error);
    startCh = atom_getlong(av++);
    ac--;
    bach_assert_objerror_goto(x, startCh > 0, "Bad starting channel", ears_intilde_error);
    for (int i = 0; i < ac && i < EARS_INTILDE_MAX_CHANS; i++) {
        t_atom_long v = atom_getlong(av++);
        if (v < 1) {
            object_error((t_object *) x, "Wrong channel index, setting to 1");
            v = 1;
        }
        x->chan[i] = v;
    }
ears_intilde_error:
    return;
}

void *ears_intilde_new(t_symbol *s, t_atom_long ac, t_atom* av)
{
    t_ears_intilde *x = (t_ears_intilde*) object_alloc(ears_intilde_class);
    x->earsMapParent = getParentEarsMap((t_object *) x);
    
    dsp_setup((t_pxobject *) x, 0);
    
    if (ac > 0) {
        t_atom_long v = atom_getlong(av);
        if (v > 0 && v <= EARSMAP_MAX_INPUT_BUFFERS)
            x->bufIndex = v;
        else {
            object_error((t_object *) x, "Bad input buffer number, setting to 1");
            x->bufIndex = 1;
        }
        
        ac--; av++;
    }
    
    if (ac > EARS_INTILDE_MAX_CHANS) {
        object_error((t_object *) x, "Too many channels, cropping to %d", EARS_INTILDE_MAX_CHANS);
        ac = EARS_INTILDE_MAX_CHANS;
    }
    
    if (ac > 0) {
        x->nChans = ac;
        for (int i = 0; i < ac; i++) {
            t_atom_long v = atom_getlong(av++);
            if (v < 1) {
                object_error((t_object *) x, "Invalid channel number at position %d, setting to 1", i + 1);
                v = 1;
            }
            x->chan[i] = v;
            outlet_new((t_object *) x, "signal");
        }
    } else {
        x->nChans = 1;
        x->chan[0] = 1;
        outlet_new((t_object *) x, "signal");
    }

    if (x->earsMapParent)
        object_method(x->earsMapParent, gensym("ears.in~_created"), x->bufIndex, x);
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


// TODO: parallelize

void ears_intilde_perform64(t_ears_intilde *x, t_dspchain *dsp64, double **ins, long numins, double **outs, long numouts, long vec_size, long flags, void *userparam)
{
    unsigned short bufIndex = x->bufIndex;
    bufferData *buf = x->bufs ? x->bufs + bufIndex - 1 : nullptr;
    int s;
    t_atom_long bufchans = buf ? buf->chans : 0;
    int nChans = x->nChans;
    
    t_atom_long pos = x->position;

    for (int i = 0; i < nChans; i++) {
        t_sample *out = outs[i];
        t_atom_long chan = x->chan[i];
        if (!buf || !buf->obj || bufchans < chan) {
            for (s = 0; s < vec_size; s++)
                *(out++) = 0;
        }
        
        float *tab = buf->samps + (pos * bufchans) + i;
        t_atom_long frames = buf->frames;
        
        for (s = 0; s < vec_size && pos < frames; s++, pos++) {
            *(out++) = *tab;
            tab += bufchans;
        }
        
        for ( ; s < vec_size; s++) {
            *(out++) = 0;
        }
    }
    
    x->position += vec_size;
}

void ears_intilde_dsp64(t_ears_intilde *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
    x->position = 0;
    if (x->earsMapParent)
        object_method(dsp64, gensym("dsp_add64"), x, ears_intilde_perform64, 0, NULL);
}

void ears_intilde_setbuffers(t_ears_intilde *x, bufferData* bufs)
{
    x->bufs = bufs;
}



