//
//  ears.processinfo.cpp
//  ears
//
//  Created by andreaagostini on 03/04/2021.
//
//
//  ears_processinfo~.cpp
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

#include "ears.process_commons.h"
#include <z_dsp.h>


#define object_method_direct_cpp(rt, sig, x, s, ...) ((rt (*)sig)object_method_direct_getmethod((t_object *)x, s))((t_object*)object_method_direct_getobject((t_object *)x, s), __VA_ARGS__)

t_class *this_class;


typedef struct _ears_processinfo
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
    
    t_object* earsProcessParent;

} t_ears_processinfo;




void *ears_processinfo_new(t_symbol *s, long ac, t_atom *av);
void ears_processinfo_free(t_ears_processinfo *x);

void ears_processinfo_stop(t_ears_processinfo *x);
void ears_processinfo_assist(t_ears_processinfo *x, void *b, long m, long a, char *s);

void ears_processinfo_prepare(t_ears_processinfo *x, t_atom *clock_name);
void ears_processinfo_start(t_ears_processinfo *x, long bufs, t_atom *bufDurs);

void ears_processinfo_end(t_ears_processinfo *x);

void ears_processinfo_dsp64(t_ears_processinfo *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags);

void ears_processinfo_perform64(t_ears_processinfo *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam);

int C74_EXPORT main()
{
    common_symbols_init();
    llllobj_common_symbols_init();
    
    this_class = class_new("ears.processinfo~",
                           (method)ears_processinfo_new,
                           (method)ears_processinfo_free,
                           sizeof(t_ears_processinfo),
                           NULL,
                           A_GIMME,
                           0);
    
    class_addmethod(this_class, (method)ears_processinfo_assist, "assist", A_CANT, 0);
    //class_addmethod(this_class, (method)ears_processinfo_loadbang, "loadbang", A_CANT, 0);
    
    class_addmethod(this_class, (method)ears_processinfo_prepare, "prepare", A_CANT, 0);
    class_addmethod(this_class, (method)ears_processinfo_start, "start", A_CANT, 0);
    class_addmethod(this_class, (method)ears_processinfo_end, "end", A_CANT, 0);
    
    class_addmethod(this_class, (method)ears_processinfo_stop, "stop", 0);
    class_addmethod(this_class, (method)ears_processinfo_dsp64, "dsp64", A_CANT, 0);
    
    //class_addmethod(this_class, (method)ears_processinfo_bang, "bang", 0);
    
    class_register(CLASS_BOX, this_class);
    
    return 0;
}


void *ears_processinfo_new(t_symbol *s, long ac, t_atom *av)
{
    t_ears_processinfo *x = (t_ears_processinfo *) object_alloc(this_class);
    
    x->clock_out = outlet_new(x, "clock");
    x->dur_out = listout(x);
    x->vs_out = intout(x);
    x->sr_out = floatout(x);
    x->onoff_out = intout(x);
    x->pos_out = floatout(x);
    outlet_new(x, "signal");
    
    x->earsProcessParent = getParentEarsProcess((t_object *) x);

    if (x->earsProcessParent)
        object_method(x->earsProcessParent, gensym("ears.processinfo~_created"), (t_object *) x);
    
    return (x);
}

void ears_processinfo_free(t_ears_processinfo *x)
{
    if (x->earsProcessParent)
        object_method(x->earsProcessParent, gensym("ears.processinfo~_deleted"), (t_object *) x);
    dsp_free((t_pxobject *) x);
}

void ears_processinfo_stop(t_ears_processinfo *x)
{
    if (x->earsProcessParent) {
        object_method(x->earsProcessParent, gensym("stop"), (t_object *) x);
    }
}

void ears_processinfo_end(t_ears_processinfo *x)
{
    outlet_int(x->onoff_out, 0);
}

void ears_processinfo_prepare(t_ears_processinfo *x, t_atom *clock_name)
{
    outlet_anything(x->clock_out, gensym("clock"), 1, clock_name);
}

void ears_processinfo_start(t_ears_processinfo *x, long bufs, t_atom *bufDurs)
{
    outlet_list(x->dur_out, NULL, bufs, bufDurs);
}

void ears_processinfo_dsp64(t_ears_processinfo *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
    if (x->earsProcessParent) {
        outlet_int(x->vs_out, maxvectorsize);
        outlet_float(x->sr_out, samplerate);
        outlet_int(x->onoff_out, 1);
        
        x->invsr = 1000. / samplerate;
        x->position = 0;
        object_method(dsp64, gensym("dsp_add64"), x, ears_processinfo_perform64, 0, NULL);
    }
}

void ears_processinfo_perform64(t_ears_processinfo *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
    int s;
    double *out = outs[0];
    t_atom_long position = x->position;
    double invsr = x->invsr;
    if (!x->earsProcessParent) {
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


void ears_processinfo_assist(t_ears_processinfo *x, void *b, long m, long a, char *s)
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