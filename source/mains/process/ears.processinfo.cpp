/**
 @file
 ears.processinfo.c
 
 @name
 ears.processinfo~
 
 @realname
 ears.processinfo~
 
 @type
 object
 
 @module
 ears
 
 @author
 Andrea Agostini, partly based upon work by Alexander J. Harker
 
 @digest
 Report information about the process running
 in ears.process~
 
 @description
 Reports information about the current state of the process running in <o>ears.process~</o>.
 
 @discussion
 
 @category
 ears process
 
 @keywords
 buffer, offline, patch, patcher, non-realtime
 
 @seealso
 ears.process~
 
 @owner
 Andrea Agostini
 */

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
void ears_processinfo_output_position(t_ears_processinfo *x);

void ears_processinfo_end(t_ears_processinfo *x);

void ears_processinfo_dsp64(t_ears_processinfo *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags);

void ears_processinfo_perform64(t_ears_processinfo *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam);

void C74_EXPORT ext_main(void* moduleRef)
{
    common_symbols_init();
    llllobj_common_symbols_init();
    
    if (llllobj_check_version(bach_get_current_llll_version()) || llllobj_test()) {
        ears_error_bachcheck();
        return;
    }
    
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
    class_addmethod(this_class, (method)ears_processinfo_output_position, "output_position", A_CANT, 0);

    // @method stop @digest Stop process
    // @description The <m>stop</m> message
    // stops the process, if it is running.
    class_addmethod(this_class, (method)ears_processinfo_stop, "stop", 0);
    class_addmethod(this_class, (method)ears_processinfo_dsp64, "dsp64", A_CANT, 0);
    
    //class_addmethod(this_class, (method)ears_processinfo_bang, "bang", 0);
    
    class_register(CLASS_BOX, this_class);
}


void *ears_processinfo_new(t_symbol *s, long ac, t_atom *av)
{
    t_ears_processinfo *x = (t_ears_processinfo *) object_alloc(this_class);
    
    x->clock_out = outlet_new(x, "clock");
    x->dur_out = outlet_new(x, nullptr);
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
    if (bufs == 1)
        outlet_float(x->dur_out, atom_getfloat(bufDurs));
    else if (bufs > 1)
        outlet_list(x->dur_out, _sym_list, bufs, bufDurs);
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
        for (s = 0; s < sampleframes; s++) {
            *(out++) = (position++) * invsr;
        }
    }
    x->position = position;
}

void ears_processinfo_output_position(t_ears_processinfo *x)
{
    outlet_float(x->pos_out, x->position * x->invsr);
}


void ears_processinfo_assist(t_ears_processinfo *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_OUTLET)
    {
        switch (a)
        {
            case 0:
                sprintf(s, "signal: position (ms)"); // @out 0 @type signal @digest Position as signal (ms)
                break;
            case 1: sprintf(s, "float: position (ms)"); // @out 1 @type float @digest Position as float (ms)
                break;
            case 2: sprintf(s, "int: on/off"); // @out 2 @type int @digest 1 If process is running, 0 if it is not
                break;
            case 3: sprintf(s, "float: sampling rate"); // @out 3 @type float @digest Sampling rate
                break;
            case 4: sprintf(s, "int: vector size"); // @out 4 @type int @digest Vector size
                break;
            case 5: sprintf(s, "float/list: input durations (ms)"); // @out 5 @type float/list @digest Durations of the input buffers
                break;
            case 6: sprintf(s, "clock message"); // @out 6 @type clock @digest Message for objects supporting <b>setclock</b>
                break;
        }
    }
    else
    {
        sprintf(s, "stop"); // @in 0 @type stop @digest Stop the process
    }
}
