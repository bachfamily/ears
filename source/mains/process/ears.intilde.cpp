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
 ears.process~, ears.in, ears.mc.in~, ears.out~
 
 @owner
 Andrea Agostini
 */


#include "ears.process_commons.h"
#include <z_dsp.h>
#include <ext_buffer.h>

t_class *ears_intilde_class;


typedef struct _ears_intilde
{
    t_ears_inouttilde io_obj;
    bufferData* bufs;
} t_ears_intilde;


void *ears_intilde_new(t_symbol *s, t_atom_long ac, t_atom* av);
void ears_intilde_free(t_ears_intilde *x);
void ears_intilde_assist(t_ears_intilde *x, void *b, long m, long a, char *s);
void ears_intilde_inletinfo(t_ears_intilde *x, void *b, long a, char *t);

void ears_intilde_int(t_ears_intilde *x, t_atom_long i);

void ears_intilde_setbuffers(t_ears_intilde *x, bufferData* bufs);

void ears_intilde_dsp64(t_ears_intilde *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags);
void ears_intilde_perform64(t_ears_intilde *x, t_dspchain *dsp64, double **ins, long numins, double **outs, long numouts, long vec_size, long flags, void *userparam);




void C74_EXPORT ext_main(void* moduleRef)
{
    common_symbols_init();
    llllobj_common_symbols_init();
    
    if (llllobj_check_version(bach_get_current_llll_version()) || llllobj_test()) {
        ears_error_bachcheck();
        return;
    }
    
    ears_intilde_class = class_new("ears.in~",
                              (method)ears_intilde_new,
                              (method)ears_intilde_free,
                              sizeof(t_ears_intilde),
                              NULL,
                              A_GIMME,
                              0);
    
    
    // @method llll @digest Set input buffers and channels
    // @description The llll is composed of two parts:<br/>
    // - an integer, setting the number of the
    // <o>ears.process~</o> object's inlet
    // from which samples will be passed to <o>ears.in~</o>; and<br/>
    // - a sublist containing the channel numbers of the incoming buffer
    // whose samples will be passed to each outlet of <o>ears.in~</o>.<br/>
    // As a shortcut, if just a flat llll is provided then
    // its first element is interpreted as the inlet number
    // and its subsequent elements are interpreted as the channel indices.<br/>
    // Both parts are optional. If not provided,
    // the inlet number defaults to 1
    // and only the first channel's samples will be output.
    class_addmethod(ears_intilde_class, (method)ears_inouttilde_anything, "anything", A_GIMME, 0);
    
    class_addmethod(ears_intilde_class, (method)ears_inouttilde_int, "int", A_LONG, 0);
    class_addmethod(ears_intilde_class, (method)ears_inouttilde_float, "float", A_FLOAT, 0);
    class_addmethod(ears_intilde_class, (method)ears_inouttilde_anything, "list", A_GIMME, 0);
    
    class_addmethod(ears_intilde_class, (method)ears_intilde_assist, "assist", A_CANT, 0);
    class_addmethod(ears_intilde_class, (method)ears_intilde_inletinfo, "inletinfo", A_CANT, 0);
    class_addmethod(ears_intilde_class, (method)ears_intilde_dsp64, "dsp64", A_CANT, 0);

    class_addmethod(ears_intilde_class, (method)ears_intilde_setbuffers, "setbuffers", A_CANT, 0);

    class_dspinit(ears_intilde_class);

    class_register(CLASS_BOX, ears_intilde_class);
}


void *ears_intilde_new(t_symbol *s, t_atom_long ac, t_atom* av)
{
    // @arg 0 @name inlet_and_channels @optional 1 @type llll
    // @digest The llll is composed of two parts:<br/>
    // - an integer, setting the number of the
    // <o>ears.process~</o> object's inlet
    // from which samples will be passed to the <o>ears.in~</o>; and<br/>
    // - a sublist containing the channel numbers of the incoming buffer
    // whose samples will be passed to each outlet of <o>ears.in~</o>.
    // A corresponding number of outlets will be created for <o>ears.in~</o><br/>
    // As a shortcut, if just a flat llll is provided then
    // its first element is interpreted as the inlet number
    // and its subsequent elements are interpreted as the channel indices.<br/>
    // Both parts are optional. If not provided,
    // the inlet number defaults to 1, only one input will be created
    // and only the first channel's samples will be output.
    
    t_ears_intilde *x = (t_ears_intilde*) object_alloc(ears_intilde_class);
    x->io_obj.earsProcessParent = getParentEarsProcess((t_object *) x);
    
    dsp_setup((t_pxobject *) x, 0);
    x->io_obj.ioNum = 1;
    x->io_obj.nChans = 1;
    x->io_obj.chan[0] = 1;

    long nChans = ears_inouttilde_anything((t_ears_inouttilde *) x, s, ac, av);
    if (nChans == -1) {
        object_free(x);
        return nullptr;
    }
    if (nChans > 0) {
        x->io_obj.nChans = nChans;
    }
    
    for (int i = 0; i < x->io_obj.nChans; i++) {
        outlet_new((t_object *) x, "signal");
    }

    if (x->io_obj.earsProcessParent)
        object_method((t_object *) x->io_obj.earsProcessParent, gensym("ears.in~_created"), x->io_obj.ioNum, x);
    return x;
}

void ears_intilde_free(t_ears_intilde *x)
{
    if (x->io_obj.earsProcessParent)
        object_method(x->io_obj.earsProcessParent, gensym("ears.in~_deleted"), x);
    dsp_free((t_pxobject *) x);
}

// TODO: parallelize

void ears_intilde_perform64(t_ears_intilde *x, t_dspchain *dsp64, double **ins, long numins, double **outs, long numouts, long vec_size, long flags, void *userparam)
{
    unsigned short inletNum = x->io_obj.ioNum;
    bufferData *buf = x->bufs ? x->bufs + inletNum - 1 : nullptr;
    int s;
    t_atom_long bufchans = buf ? buf->chans : 0;
    int nChans = x->io_obj.nChans;
    
    t_atom_long pos = x->io_obj.position;

    for (int i = 0; i < nChans; i++) {
        t_sample *out = outs[i];
        t_atom_long chan = x->io_obj.chan[i];
        if (!buf || !buf->obj || bufchans < chan) {
            for (s = 0; s < vec_size; s++)
                *(out++) = 0;
        } else {
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
    }
    
    x->io_obj.position += vec_size;
}

void ears_intilde_dsp64(t_ears_intilde *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
    x->io_obj.position = 0;
    if (x->io_obj.earsProcessParent)
        object_method(dsp64, gensym("dsp_add64"), x, ears_intilde_perform64, 0, NULL);
}

void ears_intilde_setbuffers(t_ears_intilde *x, bufferData* bufs)
{
    x->bufs = bufs;
}

void ears_intilde_inletinfo(t_ears_intilde *x, void *b, long a, char *t)
{
    *t = 1; // no hot inlets actually
}

void ears_intilde_assist(t_ears_intilde *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_OUTLET)
        sprintf(s,"(signal) Audio from Channel %ld of inlet %ld", (long) x->io_obj.chan[a], (long) x->io_obj.ioNum); // @out 0 @type signal @loop 1 @digest Input signal
    else
        sprintf(s,"(llll) Inlet and Channel");
}
