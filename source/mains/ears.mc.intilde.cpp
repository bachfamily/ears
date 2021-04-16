//
//  ears.in.cpp
//  lib_ears
//
//  Created by andreaagostini on 03/04/2021.
//


#include "ears.map.h"
#include <z_dsp.h>
#include <ext_buffer.h>

t_class *ears_mcintilde_class;

const int EARS_mcintilde_MAX_CHANS = 256;

typedef struct _ears_mcintilde
{
    t_pxobject x_obj;
    t_atom_long bufIndex;
    t_atom_long offset;
    t_object* earsMapParent;
    bufferData* bufs;
    t_atom_long position;
} t_ears_mcintilde;


void *ears_mcintilde_new(t_symbol *s, t_atom_long ac, t_atom* av);
void ears_mcintilde_assist(t_ears_mcintilde *x, void *b, long m, long a, char *s);

void ears_mcintilde_bang(t_ears_mcintilde *x);
void ears_mcintilde_int(t_ears_mcintilde *x, t_atom_long i);
void ears_mcintilde_anything(t_ears_mcintilde *x, t_symbol *s, long ac, t_atom *av);

void ears_mcintilde_setbuffers(t_ears_mcintilde *x, bufferData* bufs);
long myobject_multichanneloutputs(t_ears_mcintilde *x, long outletindex);

void ears_mcintilde_dsp64(t_ears_mcintilde *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags);
void ears_mcintilde_perform64(t_ears_mcintilde *x, t_dspchain *dsp64, double **ins, long numins, double **outs, long numouts, long vec_size, long flags, void *userparam);




int C74_EXPORT main()
{
    ears_mcintilde_class = class_new("ears.mc.in~",
                                   (method)ears_mcintilde_new,
                                   NULL,
                                   sizeof(t_ears_mcintilde),
                                   NULL,
                                   A_DEFLONG, A_DEFLONG,
                                   0);
    
    class_addmethod(ears_mcintilde_class, (method)ears_mcintilde_dsp64, "dsp64", A_CANT, 0);
    
    class_addmethod(ears_mcintilde_class, (method)ears_mcintilde_int, "int", A_CANT, 0);
    
    class_addmethod(ears_mcintilde_class, (method)ears_mcintilde_setbuffers, "setbuffers", A_CANT, 0);
    class_addmethod(ears_mcintilde_class, (method)myobject_multichanneloutputs, "multichanneloutputs", A_CANT, 0);
    
    class_dspinit(ears_mcintilde_class);
    
    class_register(CLASS_BOX, ears_mcintilde_class);
    
    return 0;
}

void ears_mcintilde_int(t_ears_mcintilde *x, t_atom_long i)
{
    if (i >= 0)
        x->offset = i;
    else {
        object_error((t_object *) x, "Bad channel offset, setting to 0");
        x->offset = 0;
    }
}

void *ears_mcintilde_new(long buf, long offset)
{
    t_ears_mcintilde *x = (t_ears_mcintilde*) object_alloc(ears_mcintilde_class);
    x->earsMapParent = getParentEarsMap((t_object *) x);
    
    dsp_setup((t_pxobject *) x, 0);
    
    x->bufIndex = buf >= 1 && buf <= EARSMAP_MAX_INPUT_BUFFERS ? buf : 1;
    ears_mcintilde_int(x, offset);
    outlet_new(x, "multichannelsignal");
    
    if (x->earsMapParent)
        object_method(x->earsMapParent, gensym("ears.in~_created"), x->bufIndex, x);
    return x;
}

void ears_mcintilde_free(t_ears_mcintilde *x)
{
    if (x->earsMapParent)
        object_method(x->earsMapParent, gensym("ears.in~_deleted"), x);
    dsp_free((t_pxobject *) x);
}

void ears_mcintilde_assist(t_ears_mcintilde *x, void *b, long m, long a, char *s)
{
    
}


// TODO: parallelize

void ears_mcintilde_perform64(t_ears_mcintilde *x, t_dspchain *dsp64, double **ins, long numins, double **outs, long numouts, long vec_size, long flags, void *userparam)
{
    unsigned short bufIndex = x->bufIndex;
    bufferData *buf = x->bufs ? x->bufs + bufIndex - 1 : nullptr;
    int s;
    long offset = x->offset;
    if (!buf || offset > buf->chans - 1) {
        t_sample *out = outs[0];
        for (s = 0; s < vec_size; s++)
            *(out++) = 0;
        x->position += vec_size;
        return;
    }
    
    long startChan = offset + 1;
    long endChan = MAX(buf->chans, startChan + numouts - 1);
    t_atom_long pos = x->position;

    int chan, outNum;
    for (chan = startChan, outNum = 0; chan <= endChan; chan++, outNum++) {
        t_sample *out = outs[outNum];
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

void ears_mcintilde_dsp64(t_ears_mcintilde *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
    x->position = 0;
    if (x->earsMapParent)
        object_method(dsp64, gensym("dsp_add64"), x, ears_mcintilde_perform64, 0, NULL);
}

void ears_mcintilde_setbuffers(t_ears_mcintilde *x, bufferData* bufs)
{
    x->bufs = bufs;
}

long myobject_multichanneloutputs(t_ears_mcintilde *x, long outletindex)
{
    if (auto b = &x->bufs[x->bufIndex - 1]) {
        if (t_atom_long c = b->chans - x->offset >= 1)
            return MIN(c, 1024);
        else {
            object_error((t_object *) x, "Invalid channel offset");
            return 1;
        }
    } else {
        object_error((t_object *) x, "Invalid buffer");
        return 1;
    }
}


