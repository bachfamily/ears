/**
 @file
 ears.mc.outtilde.c
 
 @name
 ears.mc.out~
 
 @realname
 ears.mc.out~
 
 @type
 object
 
 @module
 ears
 
 @author
 Andrea Agostini, partly based upon work by Alexander J. Harker
 
 @digest
 Signal output for a patch loaded by ears.process~
 
 @description
 Use the <o>ears.mc.out~</o> object inside a patch loaded by ears.process~
 to create a multichannel signal outlet writing data to an output buffer.
 
 @discussion
 
 @category
 ears process
 
 @keywords
 buffer, offline, patch, patcher, non-realtime, multichannel
 
 @seealso
 ears.in, ears.in~, ears.out, ears.tovector~
 
 @owner
 Andrea Agostini
 */

#include "ears.process_commons.h"
#include <z_dsp.h>

t_class *ears_mcouttilde_class;

typedef struct _ears_mcouttilde
{
    t_pxobject x_obj;
    long outlet;
    long firstChan;
    long chans;
    t_object* earsProcessParent;
    long ePPOutlets;
    audioChanMap* chanMap;
    t_atom_long position;
} t_ears_mcouttilde;

t_ears_mcouttilde *ears_mcouttilde_new(t_symbol *s, long ac, t_atom* av);
void ears_mcouttilde_free(t_ears_mcouttilde *x);

void ears_mcouttilde_assist(t_ears_mcouttilde *x, void *b, long m, long a, char *s);

void ears_mcouttilde_setchanmap(t_ears_mcouttilde *x, audioChanMap* map);
void ears_mcouttilde_multichannelsignal(t_ears_mcouttilde *x);

void ears_mcouttilde_dsp64(t_ears_mcouttilde *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags);
void ears_mcouttilde_perform64(t_ears_mcouttilde *x, t_dspchain *dsp64, double **ins, long numins, double **outs, long numouts, long vec_size, long flags, void *userparam);



int C74_EXPORT main()
{
    common_symbols_init();
    llllobj_common_symbols_init();
    
    ears_mcouttilde_class = class_new("ears.mc.out~",
                                    (method) ears_mcouttilde_new,
                                    (method) ears_mcouttilde_free,
                                    sizeof(t_ears_mcouttilde),
                                    NULL,
                                    A_DEFLONG,
                                    A_DEFLONG,
                                    0);
    
    class_addmethod(ears_mcouttilde_class, (method)ears_mcouttilde_dsp64, "dsp64", A_CANT, 0);

    class_addmethod(ears_mcouttilde_class, (method)ears_mcouttilde_assist, "assist", A_CANT, 0);
    
    class_addmethod(ears_mcouttilde_class, (method)ears_mcouttilde_setchanmap, "setchanmap", A_CANT, 0);
    
    CLASS_ATTR_LONG(ears_mcouttilde_class, "outlet", 0, t_ears_mcouttilde, outlet);
    CLASS_ATTR_LABEL(ears_mcouttilde_class, "outlet", 0, "ears.process~ Outlet Number");
    CLASS_ATTR_FILTER_CLIP(ears_mcouttilde_class, "outlet", 1, EARS_PROCESS_MAX_INPUT_BUFFERS);
    // @description Sets the number of the outlet of <o>ears.process~</o>
    // (conted from 1, starting from the leftmost)
    // to which buffer contents are to be passed.
    
    CLASS_ATTR_LONG(ears_mcouttilde_class, "firstchannel", 0, t_ears_mcouttilde, firstChan);
    CLASS_ATTR_LABEL(ears_mcouttilde_class, "firstchannel", 0, "First Channel");
    CLASS_ATTR_FILTER_MIN(ears_mcouttilde_class, "firstchannel", 1);
    // @description Sets the first channel of the output buffer.
    
    CLASS_ATTR_LONG(ears_mcouttilde_class, "chans", 0, t_ears_mcouttilde, chans);
    CLASS_ATTR_LABEL(ears_mcouttilde_class, "chans", 0, "Output Channels");
    CLASS_ATTR_FILTER_CLIP(ears_mcouttilde_class, "chans", 0, 1024);
    // @description Sets how many channels, starting from the value of the
    // <m>firstchannel</m> attribute, must be written to the buffer.
    // The default is 0, meaning that the all the incoming channels
    // will be written.
    
    class_dspinit(ears_mcouttilde_class);
    
    class_register(CLASS_BOX, ears_mcouttilde_class);
    
    return 0;
}


t_ears_mcouttilde *ears_mcouttilde_new(t_symbol *s, long ac, t_atom* av)
{
    t_ears_mcouttilde *x = (t_ears_mcouttilde*) object_alloc(ears_mcouttilde_class);
    x->earsProcessParent = getParentEarsProcess((t_object *) x);
    
    // @arg 0 @name outlet @optional 1 @type number @digest ears.process~ Signal Outlet Number
    // @description The number of the outlet of <o>ears.process~</o>
    // (conted from 1, starting from the leftmost)
    // to which buffer contents are to be passed.
    // It corresponds to the <m>outlet</m> attribute, default is 1.

    // @arg 1 @name first_channel @optional 1 @type number @digest First Channel
    // @description The first channel of the output buffer
    // It corresponds to the <m>inlet</m> attribute, default is 1.
    
    // @arg 2 @name channels @optional 1 @type number @digest Output Channels
    // @description How many channels, starting from the value of the
    // second argument or the
    // <m>firstchannel</m> attribute, must be written to the buffer.
    // The default is 0, meaning that the all the incoming channels
    // will be written.
    
    dsp_setup((t_pxobject *) x, 1);

    x->outlet = 1;
    x->firstChan = 1;
    
    long true_ac = attr_args_offset(ac, av);

    if (true_ac > 0) {
        long b = atom_getlong(av);
        if (b > 0 && b <= EARS_PROCESS_MAX_OUTPUT_BUFFERS)
            x->outlet = b;
        else
            object_error((t_object *) x, "Bad outlet number");
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

    if (x->earsProcessParent)
        object_method(x->earsProcessParent, gensym("ears.out~_created"), x->outlet, x);
    
    x->x_obj.z_misc |= Z_MC_INLETS;

    return x;
}

void ears_mcouttilde_free(t_ears_mcouttilde *x)
{
    if (x->earsProcessParent)
        object_method(x->earsProcessParent, gensym("ears.out~_deleted"), x);
    dsp_free((t_pxobject*) x);
}

void ears_mcouttilde_assist(t_ears_mcouttilde *x, void *b, long m, long a, char *s)
{
    sprintf(s, "(signal) Output %ld", a); // @in 0 @type signal @loop 1 @digest Output multichannel signal
}

void ears_mcouttilde_perform64(t_ears_mcouttilde *x, t_dspchain *dsp64, double **ins, long numins, double **outs, long numouts, long vec_size, long flags, void *userparam)
{
    t_atom_long pos = x->position;
    
    long outlet = x->outlet;
    long startChan = x->firstChan;
    long endChan = x->firstChan + MIN(x->chans, numins);
    
    if (endChan == 0)
        endChan = 1;
    long inIdx, i;
    
    for (inIdx = 0, i = startChan; i <= endChan; i++, inIdx++) {
        audioChannel *ch = x->chanMap->getChannel(outlet, i);
        ch->skipTo(pos);
        ch->insert(vec_size, ins[inIdx]);
    }
    x->position += vec_size;
}

void ears_mcouttilde_dsp64(t_ears_mcouttilde *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
    if (x->earsProcessParent && x->chanMap && count[0]) {
        object_method(dsp64, gensym("dsp_add64"), x, ears_mcouttilde_perform64, 0, NULL);
    }
    x->position = 0;
}

void ears_mcouttilde_setchanmap(t_ears_mcouttilde *x, audioChanMap* map)
{
    x->chanMap = map;
}

