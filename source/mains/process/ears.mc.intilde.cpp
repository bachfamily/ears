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
    long inlet;
    long inletOk;
    long firstChan;
    long firstChanOk;
    long chans;
    long chansOk;
    t_object* earsProcessParent;
    bufferData* bufs;
    t_atom_long position;
} t_ears_mcintilde;


t_ears_mcintilde *ears_mcintilde_new(t_symbol *s, long ac, t_atom* av);
void ears_mcintilde_free(t_ears_mcintilde *x);
void ears_mcintilde_assist(t_ears_mcintilde *x, void *b, long m, long a, char *s);

void ears_mcintilde_bang(t_ears_mcintilde *x);
void ears_mcintilde_start(t_ears_mcintilde *x, t_atom_long i);
void ears_mcintilde_int(t_ears_mcintilde *x, t_atom_long i);
void ears_mcintilde_anything(t_ears_mcintilde *x, t_symbol *s, long ac, t_atom *av);

void ears_mcintilde_setbuffers(t_ears_mcintilde *x, bufferData* bufs);
long ears_mcintilde_multichanneloutputs(t_ears_mcintilde *x, long outletindex);

void ears_mcintilde_dsp64(t_ears_mcintilde *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags);
void ears_mcintilde_perform64(t_ears_mcintilde *x, t_dspchain *dsp64, double **ins, long numins, double **outs, long numouts, long vec_size, long flags, void *userparam);




void C74_EXPORT ext_main(void* moduleRef)
{
    common_symbols_init();
    llllobj_common_symbols_init();
    
    if (llllobj_check_version(bach_get_current_llll_version()) || llllobj_test()) {
        ears_error_bachcheck();
        return;
    }
    
    ears_mcintilde_class = class_new("ears.mc.in~",
                                   (method)ears_mcintilde_new,
                                   (method)ears_mcintilde_free,
                                   sizeof(t_ears_mcintilde),
                                   NULL,
                                   A_GIMME,
                                   0);
    
    class_addmethod(ears_mcintilde_class, (method)ears_mcintilde_dsp64, "dsp64", A_CANT, 0);
    
    class_addmethod(ears_mcintilde_class, (method)ears_mcintilde_assist, "assist", A_CANT, 0);

    CLASS_ATTR_LONG(ears_mcintilde_class, "inlet", 0, t_ears_mcintilde, inlet);
    CLASS_ATTR_LABEL(ears_mcintilde_class, "inlet", 0, "ears.process~ Inlet Number");
    CLASS_ATTR_FILTER_MIN(ears_mcintilde_class, "inlet", 1);
    // @description Sets the number of the inlet of <o>ears.process~</o>
    // (conted from 1, starting from the leftmost)
    // whose buffer contents are to be passed to <o>ears.mc.in~</o>.
    
    CLASS_ATTR_LONG(ears_mcintilde_class, "firstchannel", 0, t_ears_mcintilde, firstChan);
    CLASS_ATTR_LABEL(ears_mcintilde_class, "firstchannel", 0, "First Channel");
    CLASS_ATTR_FILTER_MIN(ears_mcintilde_class, "firstchannel", 1);
    // @description Sets the first channel of the incoming buffer.
    
    CLASS_ATTR_LONG(ears_mcintilde_class, "chans", 0, t_ears_mcintilde, chans);
    CLASS_ATTR_LABEL(ears_mcintilde_class, "chans", 0, "Output Channels");
    CLASS_ATTR_FILTER_MIN(ears_mcintilde_class, "chans", 0);
    // @description Sets how many channels, starting from the value of the
    // <m>firstchannel</m> attribute, must be retrieved from the buffer and output.
    // The default is 0, meaning that the all the buffer channels
    // will be passed, up to a maximum to 1024
    // (that is, the maximum channel count for a multichannel signal).
    
    class_addmethod(ears_mcintilde_class, (method)ears_mcintilde_setbuffers, "setbuffers", A_CANT, 0);
    class_addmethod(ears_mcintilde_class, (method)ears_mcintilde_multichanneloutputs, "multichanneloutputs", A_CANT, 0);
    
    class_dspinit(ears_mcintilde_class);
    
    class_register(CLASS_BOX, ears_mcintilde_class);
}



t_ears_mcintilde *ears_mcintilde_new(t_symbol *s, long ac, t_atom* av)
{
    t_ears_mcintilde *x = (t_ears_mcintilde*) object_alloc(ears_mcintilde_class);
    
    // @arg 0 @name inlet @optional 1 @type number @digest ears.process~ Signal Inlet Number
    // @description The number of the inlet of <o>ears.process~</o>
    // (conted from 1, starting from the leftmost)
    // whose buffer contents are to be passed to <o>ears.mc.in~</o>.
    // It corresponds to the <m>inlet</m> attribute, default is 1.

    // @arg 1 @name first_channel @optional 1 @type number @digest First Channel
    // @description the first channel of the incoming buffer
    // to be passed to <o>ears.process~</o>.
    // It corresponds to the <m>inlet</m> attribute, default is 1.
    
    // @arg 2 @name channels @optional 1 @type number @digest Output Channels
    // @description The number of the output channels, starting from the value of the second argument or the
    // <m>firstchannel</m> attribute, must be passed to <o>ears.process~</o>.
    // It corresponds to the <m>chans</m> attribute.
    // The default is 0, meaning that the all the buffer channels
    // will have be passed, up to a maximum to 1024
    // (that is, the maximum channel count for a multichannel signal).
    
    x->earsProcessParent = getParentEarsProcess((t_object *) x);
    
    dsp_setup((t_pxobject *) x, 0);
    
    x->inlet = 1;
    x->firstChan = 1;
    
    long true_ac = attr_args_offset(ac, av);
    
    if (true_ac > 0) {
        long b = atom_getlong(av);
        if (b > 0 && b <= EARS_PROCESS_MAX_INPUT_BUFFERS)
            x->inlet = b;
        else
            object_error((t_object *) x, "Bad inlet number");
        true_ac--;
        ac--;
        av++;
    }
    
    if (true_ac > 0) {
        long f = atom_getlong(av);
        if (f > 0)
            x->firstChan = f;
        else
            object_error((t_object *) x, "Bad first channel");
        true_ac--;
        ac--;
        av++;
    }
    
    if (true_ac > 0) {
        long c = atom_getlong(av);
        if (c > 0 && c <= 1024)
            x->chans = c;
        else
            object_error((t_object *) x, "Bad channel count");
        true_ac--;
        ac--;
        av++;
    }
    
    attr_args_process(x, ac, av);
  
    outlet_new(x, "multichannelsignal");
    
    if (x->earsProcessParent)
        object_method(x->earsProcessParent, gensym("ears.in~_created"), x->inlet, x);
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
    if (m == ASSIST_INLET) {
        switch (a) {
            case 0: sprintf(s, "Dummy"); break;
        }
    } else {
        sprintf(s, "(signal) Input %ld", a); // @out 0 @type signal @loop 1 @digest Input multichannel signal
    }
}

// TODO: parallelize

void ears_mcintilde_perform64(t_ears_mcintilde *x, t_dspchain *dsp64, double **ins, long numins, double **outs, long numouts, long vec_size, long flags, void *userparam)
{
    unsigned short inlet = x->inlet;
    bufferData *buf = x->bufs ? x->bufs + inlet - 1 : nullptr;
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
    
    long startChan = offset;
    long bufchans = buf->chans;
    long endChan = MIN(bufchans, startChan + numouts);
    t_atom_long pos = x->position;

    int chan, outNum;
    for (chan = startChan, outNum = 0; chan < endChan; chan++, outNum++) {
        t_sample *out = outs[outNum];
        float *tab = buf->samps + (pos * bufchans) + chan;
        t_atom_long frames = buf->frames;
        
        t_atom_long leeway = MIN(vec_size, frames - pos);
        for (s = 0; s < leeway; s++) {
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

long ears_mcintilde_multichanneloutputs(t_ears_mcintilde *x, long outletindex)
{
    if (!x->earsProcessParent)
        return 1;
    x->inletOk = x->inlet;
    x->firstChanOk = x->firstChan;
    x->chansOk = x->chans;
    if (x->bufs) {
        if (auto b = &x->bufs[x->inletOk - 1]) {
            if (x->chansOk > 0)
                return x->chansOk;
            else {
                return MIN(MAX(b->chans - x->firstChan, 0) + 1, 1024);
            }
        }
    }
    //object_error((t_object *) x, "Invalid buffer");
    return 1;
}
