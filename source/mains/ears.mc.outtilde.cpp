//
//  ears.in.cpp
//  lib_ears
//
//  Created by andreaagostini on 03/04/2021.
//


#include "ears.map.h"
#include <z_dsp.h>

t_class *ears_mcouttilde_class;

const int ears_mcouttilde_MAX_CHANS = 256;

typedef struct _ears_mcouttilde
{
    t_pxobject x_obj;
    t_atom_long bufIndex;
    t_atom_long offset;
    t_object* earsMapParent;
    audioChanMap* chanMap;
    t_atom_long position;
} t_ears_mcouttilde;


void *ears_mcouttilde_new(long buf, long offset);
void ears_mcouttilde_free(t_ears_mcouttilde *x);

void ears_mcouttilde_assist(t_ears_mcouttilde *x, void *b, long m, long a, char *s);

void ears_mcouttilde_int(t_ears_mcouttilde *x, t_atom_long i);

void ears_mcouttilde_setchanmap(t_ears_mcouttilde *x, audioChanMap* map);
void ears_mcouttilde_multichannelsignal(t_ears_mcouttilde *x);

void ears_mcouttilde_dsp64(t_ears_mcouttilde *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags);
void ears_mcouttilde_perform64(t_ears_mcouttilde *x, t_dspchain *dsp64, double **ins, long numins, double **outs, long numouts, long vec_size, long flags, void *userparam);



int C74_EXPORT main()
{
    ears_mcouttilde_class = class_new("ears.mc.out~",
                                    (method) ears_mcouttilde_new,
                                    (method) ears_mcouttilde_free,
                                    sizeof(t_ears_mcouttilde),
                                    NULL,
                                    A_DEFLONG,
                                    A_DEFLONG,
                                    0);
    
    class_addmethod(ears_mcouttilde_class, (method)ears_mcouttilde_dsp64, "dsp64", A_CANT, 0);
    
    class_addmethod(ears_mcouttilde_class, (method)ears_mcouttilde_int, "int", A_LONG, 0);
    
    class_addmethod(ears_mcouttilde_class, (method)ears_mcouttilde_setchanmap, "setchanmap", A_CANT, 0);
    
    class_dspinit(ears_mcouttilde_class);
    
    class_register(CLASS_BOX, ears_mcouttilde_class);
    
    return 0;
}

void ears_mcouttilde_int(t_ears_mcouttilde *x, t_atom_long i)
{
    if (i >= 0)
        x->offset = i;
    else {
        object_error((t_object *) x, "Bad channel offset, setting to 0");
        x->offset = 0;
    }
}

void *ears_mcouttilde_new(long buf, long offset)
{
    t_ears_mcouttilde *x = (t_ears_mcouttilde*) object_alloc(ears_mcouttilde_class);
    x->earsMapParent = getParentEarsMap((t_object *) x);
    
    dsp_setup((t_pxobject *) x, 1);

    x->bufIndex = buf >= 1 && buf <= EARSMAP_MAX_INPUT_BUFFERS ? buf : 1;
    ears_mcouttilde_int(x, offset);
    
    if (x->earsMapParent)
        object_method(x->earsMapParent, gensym("ears.out~_created"), x->bufIndex, x);
    
    x->x_obj.z_misc |= Z_MC_INLETS;

    return x;
}

void ears_mcouttilde_free(t_ears_mcouttilde *x)
{
    if (x->earsMapParent)
        object_method(x->earsMapParent, gensym("ears.out~_deleted"), x);
    dsp_free((t_pxobject*) x);
}


void ears_mcouttilde_assist(t_ears_mcouttilde *x, void *b, long m, long a, char *s)
{
    
}

void ears_mcouttilde_perform64(t_ears_mcouttilde *x, t_dspchain *dsp64, double **ins, long numins, double **outs, long numouts, long vec_size, long flags, void *userparam)
{
    if (!x->chanMap)
        return;
    
    t_atom_long pos = x->position;
    
    long startChan = x->offset + 1;
    long endChan = numins + x->offset;
    if (endChan == 0)
        endChan = 1;
    long inIdx, i;
    
    for (inIdx = 0, i = startChan; i <= endChan; i++, inIdx++) {
        audioChannel *ch = x->chanMap->getChannel(x->bufIndex, i);
        ch->skipTo(pos);
        ch->insert(vec_size, ins[inIdx]);
    }
    x->position += vec_size;
}

void ears_mcouttilde_dsp64(t_ears_mcouttilde *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
    if (x->earsMapParent)
        object_method(dsp64, gensym("dsp_add64"), x, ears_mcouttilde_perform64, 0, NULL);
    x->position = 0;
}

void ears_mcouttilde_setchanmap(t_ears_mcouttilde *x, audioChanMap* map)
{
    x->chanMap = map;
}

