/**
 @file
 ears.tovector.c
 
 @name
 ears.tovector~
 
 @realname
 ears.tovector~
 
 @type
 object
 
 @module
 ears
 
 @author
 Andrea Agostini, partly based upon work by Alexander J. Harker
 
 @digest
 Convert synchronously a list of numbers into a signal vector
 
 @description
 Use the <o>ears.tovector~</o> object inside a patch loaded by <o>ears.process~</o>
 collects values into a signal vector. All the values for a single signal vector must be received
 within a single event triggered synchronously to the non-realtime audio vector
 as managed by <o>ears.process~</o>.
 This can be achieved by responding to messages produced by <o>ears.tovector~</o>
 or by the position message outlet of <o>ears.processinfo~</o>.
 
 @discussion
 
 @category
 ears process
 
 @keywords
 buffer, offline, patch, patcher, non-realtime
 
 @seealso
 ears.out~, ears.tovector~
 
 @owner
 Andrea Agostini
 */


#include "ears.process_commons.h"
#include <z_dsp.h>

t_class *ears_tovector_class;

typedef struct _ears_tovector
{
    t_pxobject x_obj;
    t_object* earsProcessParent;
    double vec[EARS_PROCESS_MAX_VS];
    long n;
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



void C74_EXPORT ext_main(void* moduleRef)
{
    common_symbols_init();
    llllobj_common_symbols_init();
    
    if (llllobj_check_version(bach_get_current_llll_version()) || llllobj_test()) {
        ears_error_bachcheck();
        return;
    }
    
    ears_tovector_class = class_new("ears.tovector~",
                                    (method) ears_tovector_new,
                                    (method) ears_tovector_free,
                                    sizeof(t_ears_tovector),
                                    NULL,
                                    A_GIMME,
                                    0);
    
    // @method int @digest Integer value to be collected
    // @description An integer is appended
    // to the signal vector currently being collected.
    // If enough samples to fill a vector
    // have already been collected, the incoming value is discarded.
    class_addmethod(ears_tovector_class, (method) ears_tovector_int, "int", A_LONG, 0);
    
    // @method float @digest Float value to be collected
    // @description A floating-point number is appended
    // to the signal vector currently being collected.
    // If enough samples to fill a vector
    // have already been collected, the incoming value is discarded.
    class_addmethod(ears_tovector_class, (method) ears_tovector_float, "float", A_FLOAT, 0);
    
    // @method list @digest List of values to be collected
    // @description The values contained in a list
    // are appended to the signal vector currently being collected.
    // Once enough samples to fill a vector have been collected,
    // the subsequent ones are discarded.
    class_addmethod(ears_tovector_class, (method) ears_tovector_list, "list",
                    A_GIMME, 0);
    
    // @method clear @digest Clear the vector
    // @description The <m>clear</m> message
    // sets all the samples of the current vector to 0.
    // If <m>clear</m> is not sent
    // (or the <m>autoclear</m> attribute is not set),
    // the samples collected in the previous vector are kept
    // and progressively replaced.
    class_addmethod(ears_tovector_class, (method) ears_tovector_clear, "clear", 0);
    
    // @method set @digest Set specific samples
    // @description The <m>set</m> message
    // followed by an integer
    // sets the position of the next sample
    // to be written in the vector (counting from 1).
    // Optional further values will be appended
    // to the vector starting from the new position.
    class_addmethod(ears_tovector_class, (method) ears_tovector_set, "set",
                    A_GIMME, 0);
    
    class_addmethod(ears_tovector_class, (method)ears_tovector_dsp64, "dsp64", A_CANT, 0);

    CLASS_ATTR_LONG(ears_tovector_class, "autoclear", 0, t_ears_tovector, autoclear);
    CLASS_ATTR_STYLE_LABEL(ears_tovector_class, "autoclear", 0, "onoff", "Auto Clear");
    CLASS_ATTR_FILTER_CLIP(ears_tovector_class, "autoclear", 0, 1);
    // @description When the <m>autoclear</m> attribute is set to 1 (the default),
    // all the samples in each new signal vector
    // are is initialized to 0.
    
    class_dspinit(ears_tovector_class);
    
    class_register(CLASS_BOX, ears_tovector_class);
}

void *ears_tovector_new(t_symbol *s, t_atom_long ac, t_atom* av)
{
    t_ears_tovector *x = (t_ears_tovector*) object_alloc(ears_tovector_class);
    x->earsProcessParent = getParentEarsProcess((t_object *) x);
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
    if (int n = x->n; n < EARS_PROCESS_MAX_VS) {
        x->vec[n++] = i;
    }
}

void ears_tovector_float(t_ears_tovector *x, double f)
{
    if (int n = x->n; n < EARS_PROCESS_MAX_VS) {
        x->vec[n++] = f;
    }
}

void ears_tovector_list(t_ears_tovector *x, t_symbol *s, long ac, t_atom *av)
{
    long end = MIN(ac, EARS_PROCESS_MAX_VS);
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
    if (ac == 0) {
        x->n = 0;
        return;
    }

    long n = atom_getlong(av++) - 1;
    if (n < 0 || n >= EARS_PROCESS_MAX_VS) {
        object_error((t_object *) x, "set: bad starting value");
        return;
    }

    long end = MIN(n + ac - 1, EARS_PROCESS_MAX_VS);
    
    for (int i = n - 1; i < end; i++) {
        x->vec[i] = atom_getfloat(av++);
    }
    x->n = end;
}
                    
void ears_tovector_assist(t_ears_tovector *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET)
        sprintf(s,"list/set/clear: Values to be collected or control messages"); // @out 0 @type list/set/clear @digest Values to be collected or control messages
}

void ears_tovector_perform64(t_ears_tovector *x, t_dspchain *dsp64, double **ins, long numins, double **outs, long numouts, long vec_size, long flags, void *userparam)
{
    memcpy(outs[0], x->vec, x->n * sizeof(double));
    long pad = MAX(vec_size - x->n, 0);
    memset(outs[0], 0, pad);
    memcpy(outs[0] + pad, x->vec, (vec_size - pad) * sizeof(double));
    if (x->autoclear)
        ears_tovector_clear(x);
}

void ears_tovector_dsp64(t_ears_tovector *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
    object_method(dsp64, gensym("dsp_add64"), x, ears_tovector_perform64, 0, NULL);
    if (!x->earsProcessParent) {
        object_warn((t_object *) x, "Can cause trouble if used outside ears.process~");
    }
}
