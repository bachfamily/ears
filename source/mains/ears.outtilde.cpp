//
//  ears.in.cpp
//  lib_ears
//
//  Created by andreaagostini on 03/04/2021.
//


#include "ears.map.h"
#include <z_dsp.h>

t_class *ears_outtilde_class;


typedef struct _ears_outtilde
{
    t_pxobject x_obj;
    t_atom_long bufIndex;
    t_atom_long chan;
    t_object* earsMapParent;
    audioChanMap* chanMap;
} t_ears_outtilde;


void *ears_outtilde_new(t_symbol *s, t_atom_long ac, t_atom* av);
void ears_outtilde_free(t_ears_outtilde *x);

void ears_outtilde_assist(t_ears_outtilde *x, void *b, long m, long a, char *s);

void ears_outtilde_bang(t_ears_outtilde *x);
void ears_outtildet(t_ears_outtilde *x, t_atom_long i);
void ears_outtilde_float(t_ears_outtilde *x, t_atom_float f);
void ears_outtilde_anything(t_ears_outtilde *x, t_symbol *s, long ac, t_atom *av);

void ears_outtilde_setchanmap(t_ears_outtilde *x, audioChanMap* map);

void ears_outtilde_dsp64(t_ears_outtilde *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags);
void ears_outtilde_perform64(t_ears_outtilde *x, t_dspchain *dsp64, double **ins, long numins, double **outs, long numouts, long vec_size, long flags, void *userparam);



int C74_EXPORT main()
{
    ears_outtilde_class = class_new("ears.out~",
                                   (method) ears_outtilde_new,
                                   (method) ears_outtilde_free,
                                   sizeof(t_ears_outtilde),
                                   NULL,
                                   A_GIMME,
                                   0);
    
    class_addmethod(ears_outtilde_class, (method)ears_outtilde_dsp64, "dsp64", A_CANT, 0);
    
    class_addmethod(ears_outtilde_class, (method)ears_outtilde_setchanmap, "setchanmap", A_CANT, 0);

    class_dspinit(ears_outtilde_class);
    
    class_register(CLASS_BOX, ears_outtilde_class);
    
    return 0;
}

void *ears_outtilde_new(t_symbol *s, t_atom_long ac, t_atom* av)
{
    t_ears_outtilde *x = (t_ears_outtilde*) object_alloc(ears_outtilde_class);
    x->earsMapParent = getParentEarsMap((t_object *) x);
    
    if (ac > 0) {
        x->bufIndex = atom_getlong(av);
        ac--; av++;
    }
    
    if (ac > 0) {
        x->chan = atom_getlong(av);
    }
    
    if (x->bufIndex < 1 || x->bufIndex > EARSMAP_MAX_OUTPUT_BUFFERS)
        x->bufIndex = 1;
    
    if (x->chan < 1)
        x->chan = 1;
    
    if (x->earsMapParent)
        object_method(x->earsMapParent, gensym("ears.out~_created"), x->chan, x);
    
    dsp_setup((t_pxobject *) x, 1);

    return x;
}

void ears_outtilde_free(t_ears_outtilde *x)
{
    if (x->earsMapParent)
        object_method(x->earsMapParent, gensym("ears.out~_deleted"), x);
    dsp_free((t_pxobject*) x);
}


void ears_outtilde_assist(t_ears_outtilde *x, void *b, long m, long a, char *s)
{

}

void ears_outtilde_perform64(t_ears_outtilde *x, t_dspchain *dsp64, double **ins, long numins, double **outs, long numouts, long vec_size, long flags, void *userparam)
{
    if (!x->chanMap)
        return;
    
    audioChannel *ch = x->chanMap->getChannel(x->bufIndex, x->chan);
    ch->insert(vec_size, ins[0]);
}

void ears_outtilde_dsp64(t_ears_outtilde *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
    object_method(dsp64, gensym("dsp_add64"), x, ears_outtilde_perform64, 0, NULL);
}

void ears_outtilde_setchanmap(t_ears_outtilde *x, audioChanMap* map)
{
    x->chanMap = map;
}

