//
//  ears.mapinfo.cpp
//  ears
//
//  Created by andreaagostini on 03/04/2021.
//
//
//  ears_mapinfo~.cpp
//  dynamicdsp~
//
//  Created by andreaagostini on 29/03/2021.
//

/*
 outlets (l2r):
 - signal: position (ms)
 - float: position (ms)
 - int: on/off
 - float: sampling rate
 - int: vector size
 - list: durations of all the buffers (ms)
 - symbol: clock name
 */

#include "ears.map.h"
#include <z_dsp.h>


#define object_method_direct_cpp(rt, sig, x, s, ...) ((rt (*)sig)object_method_direct_getmethod((t_object *)x, s))((t_object*)object_method_direct_getobject((t_object *)x, s), __VA_ARGS__)

t_class *this_class;


typedef struct _ears_mapinfo
{
    t_pxobject x_obj;
    
    void *clock_out;
    void *dur_out;
    void *vs_out;
    void *sr_out;
    void *onoff_out;
    void *pos_out;
    
    t_atom_long position;
    double invsr;
    
    t_object* earsMapParent;

} t_ears_mapinfo;




void *ears_mapinfo_new(t_symbol *s, long ac, t_atom *av);
void ears_mapinfo_free(t_ears_mapinfo *x);

void ears_mapinfo_stop(t_ears_mapinfo *x);
void ears_mapinfo_assist(t_ears_mapinfo *x, void *b, long m, long a, char *s);

void ears_mapinfo_prepare(t_ears_mapinfo *x, t_atom *clock_name);
void ears_mapinfo_start(t_ears_mapinfo *x, long bufs, t_atom *bufDurs);

void ears_mapinfo_end(t_ears_mapinfo *x);

void ears_mapinfo_dsp64(t_ears_mapinfo *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags);

void ears_mapinfo_perform64(t_ears_mapinfo *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam);

int C74_EXPORT main()
{
    this_class = class_new("ears.mapinfo~",
                           (method)ears_mapinfo_new,
                           (method)ears_mapinfo_free,
                           sizeof(t_ears_mapinfo),
                           NULL,
                           A_GIMME,
                           0);
    
    class_addmethod(this_class, (method)ears_mapinfo_assist, "assist", A_CANT, 0);
    //class_addmethod(this_class, (method)ears_mapinfo_loadbang, "loadbang", A_CANT, 0);
    
    class_addmethod(this_class, (method)ears_mapinfo_prepare, "prepare", A_CANT, 0);
    class_addmethod(this_class, (method)ears_mapinfo_start, "start", A_CANT, 0);
    class_addmethod(this_class, (method)ears_mapinfo_end, "end", A_CANT, 0);
    
    class_addmethod(this_class, (method)ears_mapinfo_stop, "stop", 0);
    class_addmethod(this_class, (method)ears_mapinfo_dsp64, "dsp64", A_CANT, 0);
    
    //class_addmethod(this_class, (method)ears_mapinfo_bang, "bang", 0);
    
    class_register(CLASS_BOX, this_class);
    
    return 0;
}


void *ears_mapinfo_new(t_symbol *s, long ac, t_atom *av)
{
    t_ears_mapinfo *x = (t_ears_mapinfo *) object_alloc(this_class);
    
    x->clock_out = outlet_new(x, "clock");
    x->dur_out = listout(x);
    x->vs_out = intout(x);
    x->sr_out = floatout(x);
    x->onoff_out = intout(x);
    x->pos_out = floatout(x);
    outlet_new(x, "signal");
    
    x->earsMapParent = getParentEarsMap((t_object *) x);

    if (x->earsMapParent)
        object_method(x->earsMapParent, gensym("ears.mapinfo~_created"), (t_object *) x);
    
    return (x);
}

void ears_mapinfo_free(t_ears_mapinfo *x)
{
    if (x->earsMapParent)
        object_method(x->earsMapParent, gensym("ears.mapinfo~_deleted"), (t_object *) x);
    dsp_free((t_pxobject *) x);
}

void ears_mapinfo_stop(t_ears_mapinfo *x)
{
    if (x->earsMapParent) {
        object_method(x->earsMapParent, gensym("stop"), (t_object *) x);
    }
}

void ears_mapinfo_end(t_ears_mapinfo *x)
{
    outlet_int(x->onoff_out, 0);
}

void ears_mapinfo_prepare(t_ears_mapinfo *x, t_atom *clock_name)
{
    outlet_anything(x->clock_out, gensym("clock"), 1, clock_name);
}

void ears_mapinfo_start(t_ears_mapinfo *x, long bufs, t_atom *bufDurs)
{
    outlet_list(x->dur_out, NULL, bufs, bufDurs);
}

void ears_mapinfo_dsp64(t_ears_mapinfo *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
    if (x->earsMapParent) {
        outlet_int(x->vs_out, maxvectorsize);
        outlet_float(x->sr_out, samplerate);
        outlet_int(x->onoff_out, 1);
        
        x->invsr = 1000. / samplerate;
        x->position = 0;
        object_method(dsp64, gensym("dsp_add64"), x, ears_mapinfo_perform64, 0, NULL);
    }
}

void ears_mapinfo_perform64(t_ears_mapinfo *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
    int s;
    double *out = outs[0];
    t_atom_long position = x->position;
    double invsr = x->invsr;
    if (!x->earsMapParent) {
        for (s = 0; s < sampleframes; s++) {
            *(out++) = 0;
        }
    } else {
        outlet_float(x->pos_out, position * invsr);
        for (s = 0; s < sampleframes; s++) {
            *(out++) = (position++) * invsr;
        }
    }
    x->position = position;
}


void ears_mapinfo_assist(t_ears_mapinfo *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_OUTLET)
    {
        switch (a)
        {
            case 0: sprintf(s, "signal: position (ms)");        break;
            case 1: sprintf(s, "float: position (ms)");         break;
            case 2: sprintf(s, "int: on/off");                  break;
            case 3: sprintf(s, "float: sampling rate");         break;
            case 4: sprintf(s, "int: vector size");             break;
            case 5: sprintf(s, "list: in/out durations (ms)");  break;
            case 6: sprintf(s, "clock message");                break;
        }
    }
    else
    {
        sprintf(s, "stop");
    }
}
