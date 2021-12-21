/**
 @file
 ears.mc.intilde.c
 
 @name
 ears.mc.in~
 
 @realname
 ears.mc.in~
 
 @type
 object
 
 @module
 ears
 
 @author
 Andrea Agostini, partly based upon work by Alexander J. Harker
 
 @digest
 Signal input for a patch loaded by ears.process~
 
 @description
 Use the <o>ears.mc.in~</o> object inside a patch loaded by ears.process~
 to create a multichannel signal inlet receiving data from an input buffer.
 
 @discussion
 
 @category
 ears process
 
 @keywords
 buffer, offline, patch, patcher, non-realtime, multichannel
 
 @seealso
 ears.in, ears.in~, ears.out~, ears.tovector~
 
 @owner
 Andrea Agostini
 */


#include "ears.process_commons.h"
#include <z_dsp.h>
#include <ext_buffer.h>

t_class *ears_mcintilde_class;

typedef struct _ears_mcintilde
{
    t_pxobject x_obj;
    long bufIndex;
    long bufIndexOk;
    long firstChan;
    long firstChanOk;
    long chans;
    long chansOk;
    t_object* earsProcessParent;
    bufferData* bufs;
    t_atom_long position;
} t_ears_mcintilde;


t_ears_mcintilde *ears_mcintilde_new(t_symbol *s, long ac, t_atom* av);
void ears_mcintilde_assist(t_ears_mcintilde *x, void *b, long m, long a, char *s);

void ears_mcintilde_bang(t_ears_mcintilde *x);
void ears_mcintilde_start(t_ears_mcintilde *x, t_atom_long i);
void ears_mcintilde_int(t_ears_mcintilde *x, t_atom_long i);
void ears_mcintilde_anything(t_ears_mcintilde *x, t_symbol *s, long ac, t_atom *av);

void ears_mcintilde_setbuffers(t_ears_mcintilde *x, bufferData* bufs);
long myobject_multichanneloutputs(t_ears_mcintilde *x, long outletindex);

void ears_mcintilde_dsp64(t_ears_mcintilde *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags);
void ears_mcintilde_perform64(t_ears_mcintilde *x, t_dspchain *dsp64, double **ins, long numins, double **outs, long numouts, long vec_size, long flags, void *userparam);




int C74_EXPORT main()
{
    common_symbols_init();
    llllobj_common_symbols_init();
    
    ears_mcintilde_class = class_new("ears.mc.in~",
                                   (method)ears_mcintilde_new,
                                   NULL,
                                   sizeof(t_ears_mcintilde),
                                   NULL,
                                   A_GIMME,
                                   0);
    
    class_addmethod(ears_mcintilde_class, (method)ears_mcintilde_dsp64, "dsp64", A_CANT, 0);
    
    // @method offset @digest Set channel offset
    // @description The message <m>offset</m> followed
    // by an integer sets the first channel
    // to be output from <o>ears.mc.in~</o>'s outlet.
    
    // <o>ears.process~</o> whose data will be passed
    // to the <o>ears.mc.in~</o> object.<br/>
    // The channel offset is not changed.
    //class_addmethod(ears_mcintilde_class, (method)ears_mcintilde_offset, "offset", A_LONG, 0);

    // @method int @digest Set input buffer
    // @description An integer sets the inlet of
    // <o>ears.process~</o> whose data will be passed
    // to the <o>ears.mc.in~</o> object.<br/>
    // The channel offset is not changed.
    //class_addmethod(ears_mcintilde_class, (method)ears_mcintilde_int, "int", A_LONG, 0);

    
    CLASS_ATTR_LONG(ears_mcintilde_class, "bufferindex", 0, t_ears_mcintilde, bufIndex);
    CLASS_ATTR_LABEL(ears_mcintilde_class, "bufferindex", 0, "Buffer Index");
    CLASS_ATTR_FILTER_MIN(ears_mcintilde_class, "bufferindex", 1);
    
    CLASS_ATTR_LONG(ears_mcintilde_class, "firstchannel", 0, t_ears_mcintilde, firstChan);
    CLASS_ATTR_LABEL(ears_mcintilde_class, "firstchannel", 0, "First Channel");
    CLASS_ATTR_FILTER_MIN(ears_mcintilde_class, "firstchannel", 1);
    
    CLASS_ATTR_LONG(ears_mcintilde_class, "chans", 0, t_ears_mcintilde, chans);
    CLASS_ATTR_LABEL(ears_mcintilde_class, "chans", 0, "Output Channels");
    CLASS_ATTR_FILTER_MIN(ears_mcintilde_class, "chans", 0);
    
    class_addmethod(ears_mcintilde_class, (method)ears_mcintilde_setbuffers, "setbuffers", A_CANT, 0);
    class_addmethod(ears_mcintilde_class, (method)myobject_multichanneloutputs, "multichanneloutputs", A_CANT, 0);
    
    class_dspinit(ears_mcintilde_class);
    
    class_register(CLASS_BOX, ears_mcintilde_class);
    
    return 0;
}

t_ears_mcintilde *ears_mcintilde_new(t_symbol *s, long ac, t_atom* av)
{
    t_ears_mcintilde *x = (t_ears_mcintilde*) object_alloc(ears_mcintilde_class);
    x->earsProcessParent = getParentEarsProcess((t_object *) x);
    
    dsp_setup((t_pxobject *) x, 0);
    
    x->bufIndex = 1;
    x->firstChan = 1;
    
    if (ac > 0) {
        long b = atom_getlong(av);
        if (b > 0 && b <= EARS_PROCESS_MAX_INPUT_BUFFERS)
            x->bufIndex = b;
        else
            object_error((t_object *) x, "Bad buffer index");
        ac--;
        av++;
    }
    
    if (ac > 0) {
        long f = atom_getlong(av);
        if (f > 0)
            x->firstChan = f;
        else
            object_error((t_object *) x, "Bad first channel");
        ac--;
        av++;
    }
    
    attr_args_process(x, ac, av);
  
    outlet_new(x, "multichannelsignal");
    
    if (x->earsProcessParent)
        object_method(x->earsProcessParent, gensym("ears.in~_created"), x->bufIndex, x);
    return x;
}

void ears_mcintilde_free(t_ears_mcintilde *x)
{
    if (x->earsProcessParent)
        object_method(x->earsProcessParent, gensym("ears.in~_deleted"), x);
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
    long offset = x->firstChanOk - 1;
    if (!buf || offset >= buf->chans) {
        for (int ch = 0; ch < numouts; ch++) {
            t_sample *out = *(outs++);
            for (s = 0; s < vec_size; s++)
                *(out++) = 0;
            x->position += vec_size;
        }
        return;
    }
    
    long startChan = offset + 1;
    long endChan = MAX(buf->chans, startChan + numouts - 1);
    long bufchans = endChan - startChan - 1;
    t_atom_long pos = x->position;

    int chan, outNum;
    for (chan = startChan, outNum = 0; chan <= endChan; chan++, outNum++) {
        t_sample *out = outs[outNum];
        float *tab = buf->samps + (pos * bufchans) + chan;
        t_atom_long frames = buf->frames;
        
        for (s = 0; s < vec_size && pos < frames; s++, pos++) {
            *(out++) = *tab;
            tab += bufchans;
        }
        
        for ( ; s < vec_size; s++) {
            *(out++) = 0;
        }
    }
    
    for ( ; outNum < numouts; outNum++) {
        t_sample *out = outs[outNum];
        for (s = 0; s < vec_size; s++)
            *(out++) = 0;
        x->position += vec_size;
    }
    
    x->position += vec_size;
}

void ears_mcintilde_dsp64(t_ears_mcintilde *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
    x->position = 0;
    if (x->earsProcessParent && count[0])
        object_method(dsp64, gensym("dsp_add64"), x, ears_mcintilde_perform64, 0, NULL);
}

void ears_mcintilde_setbuffers(t_ears_mcintilde *x, bufferData* bufs)
{
    x->bufs = bufs;
}

long myobject_multichanneloutputs(t_ears_mcintilde *x, long outletindex)
{
    x->bufIndexOk = x->bufIndexOk;
    x->firstChanOk = x->firstChan;
    x->chansOk = x->chans;
    if (auto b = &x->bufs[x->bufIndexOk - 1]) {
        if (x->chansOk != 0)
            return x->chansOk;
        else {
            return MIN(MIN(b->chans - x->firstChan, 0) + 1, 1024);
        }
    } else {
        object_error((t_object *) x, "Invalid buffer");
        return 1;
    }
}


