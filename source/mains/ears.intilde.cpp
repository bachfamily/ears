/**
 @file
 ears.intilde.c
 
 @name
 ears.in~
 
 @realname
 ears.in~
 
 @type
 object
 
 @module
 ears
 
 @author
 Andrea Agostini, partly based upon work by Alexander J. Harker
 
 @digest
 Signal input for a patch loaded by ears.process~
 
 @description
 Use the <o>ears.in~</o> object inside a patch loaded by ears.process~
 to create a signal inlet receiving data from an input buffer.
 
 @discussion
 
 @category
 ears process
 
 @keywords
 buffer, offline, patch, patcher, non-realtime
 
 @seealso
 ears.in, ears.mc.in~, ears.out~, ears.tovector~
 
 @owner
 Andrea Agostini
 */


#include "ears.process_commons.h"
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
    t_object* earsProcessParent;
    bufferData* bufs;
    t_atom_long position;
} t_ears_intilde;


void *ears_intilde_new(t_symbol *s, t_atom_long ac, t_atom* av);
void ears_intilde_assist(t_ears_intilde *x, void *b, long m, long a, char *s);
void ears_intilde_inletinfo(t_ears_intilde *x, void *b, long a, char *t);

void ears_intilde_int(t_ears_intilde *x, t_atom_long i);
void ears_intilde_list(t_ears_intilde *x, t_symbol *s, long ac, t_atom *av);

void ears_intilde_setbuffers(t_ears_intilde *x, bufferData* bufs);

void ears_intilde_dsp64(t_ears_intilde *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags);
void ears_intilde_perform64(t_ears_intilde *x, t_dspchain *dsp64, double **ins, long numins, double **outs, long numouts, long vec_size, long flags, void *userparam);




int C74_EXPORT main()
{
    common_symbols_init();
    llllobj_common_symbols_init();
    
    ears_intilde_class = class_new("ears.in~",
                              (method)ears_intilde_new,
                              NULL,
                              sizeof(t_ears_intilde),
                              NULL,
                              A_GIMME,
                              0);
    
    class_addmethod(ears_intilde_class, (method)ears_intilde_dsp64, "dsp64", A_CANT, 0);
    
    
    // @method list @digest Set input buffers and channels
    // @description The first element of the list sets
    // the inlet of <o>ears.process~</o> whose buffer data
    // will be passed to the <o>ears.in~</o> object.<br/>
    // Each subsequent element sets a buffer channel
    // whose samples will be output from the corresponding outlet.
    class_addmethod(ears_intilde_class, (method)ears_intilde_list, "list", A_GIMME, 0);
    
    // @method int @digest Set input buffer
    // @description An integer sets the inlet of
    // <o>ears.process~</o> whose data will be passed
    // to the <o>ears.in~</o> object.<br/>
    // The output channels are not changed.
    class_addmethod(ears_intilde_class, (method)ears_intilde_int, "int", A_GIMME, 0);

    
    class_addmethod(ears_intilde_class, (method)ears_intilde_setbuffers, "setbuffers", A_CANT, 0);
    
    class_addmethod(ears_intilde_class, (method)ears_intilde_assist, "assist", A_CANT, 0);
    class_addmethod(ears_intilde_class, (method)ears_intilde_inletinfo, "inletinfo", A_CANT, 0);
    
    class_dspinit(ears_intilde_class);

    class_register(CLASS_BOX, ears_intilde_class);
    
    return 0;
}

void ears_intilde_list(t_ears_intilde *x, t_symbol *s, long ac, t_atom *av)
{
    t_atom_long bufIdx;
    bach_assert_objerror_goto(x, ac >= 2, "Wrong format", ears_intilde_list_error);
    bufIdx = atom_getlong(av++);
    
    bach_assert_objerror_goto(x, bufIdx > 0, "Bad buffer index", ears_intilde_list_error);

    x->bufIndex = bufIdx;
    ac--;
    
    for (int i = 0; i < ac && i < EARS_INTILDE_MAX_CHANS; i++) {
        t_atom_long v = atom_getlong(av++);
        if (v < 1) {
            object_error((t_object *) x, "Wrong channel index, setting to 1");
            v = 1;
        }
        x->chan[i] = v;
    }
ears_intilde_list_error:
    return;
}

void ears_intilde_int(t_ears_intilde *x, t_atom_long i)
{
    bach_assert_objerror_goto(x, i > 0, "Bad buffer index", ears_intilde_int_error);
    
    x->bufIndex = i;

ears_intilde_int_error:
    return;
}


void *ears_intilde_new(t_symbol *s, t_atom_long ac, t_atom* av)
{
 
    // @arg 0 @name bufchans @optional 1 @type int/list
    // @digest Buffer and channel indices
    // @description The first (or only) value
    // sets the inlet of <o>ears.process~</o> whose buffer data
    // will be passed to the object, defaulting to 1.<br/>
    // The subsequent values set the channel numbers
    // whose samples will be output from each outlet.
    // If none is provided, only the first channel will be output.
    
    
    t_ears_intilde *x = (t_ears_intilde*) object_alloc(ears_intilde_class);
    x->earsProcessParent = getParentEarsProcess((t_object *) x);
    
    dsp_setup((t_pxobject *) x, 0);
    
    x->bufIndex = 1;

    if (ac > 0) {
        t_atom_long v = atom_getlong(av);
        if (v > 0 && v <= EARS_PROCESS_MAX_INPUT_BUFFERS)
            x->bufIndex = v;
        else {
            object_error((t_object *) x, "Bad input buffer number, setting to 1");
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

    if (x->earsProcessParent)
        object_method(x->earsProcessParent, gensym("ears.in~_created"), x->bufIndex, x);
    return x;
}

void ears_intilde_free(t_ears_intilde *x)
{
    if (x->earsProcessParent)
        object_method(x->earsProcessParent, gensym("ears.in~_deleted"), x);
    dsp_free((t_pxobject *) x);
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
        } else {
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
    }
    
    x->position += vec_size;
}


void ears_intilde_dsp64(t_ears_intilde *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
    x->position = 0;
    if (x->earsProcessParent)
        object_method(dsp64, gensym("dsp_add64"), x, ears_intilde_perform64, 0, NULL);
}


void ears_intilde_setbuffers(t_ears_intilde *x, bufferData* bufs)
{
    x->bufs = bufs;
}


void ears_intilde_assist(t_ears_intilde *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        sprintf(s, "list: Buffer index and channels"); // @in 1 @type list @digest Buffer index and channels
    } else {
        sprintf(s, "signal: Audio from buffer"); // @in 1 @loop 1 @type signal @digest Audio from buffer
    }
}

void ears_intilde_inletinfo(t_ears_intilde *x, void *b, long a, char *t)
{
    *t = 1; // no hot inlets actually
}
