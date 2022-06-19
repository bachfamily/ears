/**
 @file
 ears.outtilde.c
 
 @name
 ears.out~
 
 @realname
 ears.out~
 
 @type
 object
 
 @module
 ears
 
 @author
 Andrea Agostini, partly based upon work by Alexander J. Harker
 
 @digest
 Signal output for a patch loaded by ears.process~
 
 @description
 Use the <o>ears.in~</o> object inside a patch loaded by ears.process~
 to create a signal inlet receiving data from an input buffer.
 
 @discussion
 
 @category
 ears process
 
 @keywords
 buffer, offline, patch, patcher, non-realtime
 
 @seealso
 ears.in, ears.mc.in~, ears.in~, ears.tovector~
 
 @owner
 Andrea Agostini
 */


#include "ears.process_commons.h"
#include <z_dsp.h>

t_class *ears_outtilde_class;


typedef struct _ears_outtilde
{
    t_ears_inouttilde io_obj;
    audioChanMap* chanMap;
} t_ears_outtilde;


void *ears_outtilde_new(t_symbol *s, t_atom_long ac, t_atom* av);
void ears_outtilde_free(t_ears_outtilde *x);

void ears_outtilde_assist(t_ears_outtilde *x, void *b, long m, long a, char *s);

void ears_outtilde_int(t_ears_outtilde *x, t_atom_long i);

void ears_outtilde_setchanmap(t_ears_outtilde *x, audioChanMap* map);

void ears_outtilde_dsp64(t_ears_outtilde *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags);
void ears_outtilde_perform64(t_ears_outtilde *x, t_dspchain *dsp64, double **ins, long numins, double **outs, long numouts, long vec_size, long flags, void *userparam);



void C74_EXPORT ext_main(void* moduleRef)
{
    common_symbols_init();
    llllobj_common_symbols_init();
    
    if (llllobj_check_version(bach_get_current_llll_version()) || llllobj_test()) {
        ears_error_bachcheck();
        return;
    }
    
    ears_outtilde_class = class_new("ears.out~",
                                   (method) ears_outtilde_new,
                                   (method) ears_outtilde_free,
                                   sizeof(t_ears_outtilde),
                                   NULL,
                                   A_GIMME,
                                   0);
    
    // @method llll @digest Set output buffers and channels
    // @description The llll is composed of two parts:<br/>
    // - an integer, setting the number of the
    // <o>ears.process~</o> object's outlet
    // to which samples will be passed by <o>ears.out~</o>; and<br/>
    // - a sublist containing the channel numbers of the output buffer
    // <o>ears.out~</o> will write to.<br/>
    // As a shortcut, if just a flat llll is provided then
    // its first element is interpreted as the outlet number
    // and its subsequent elements are interpreted as the channel indices.<br/>
    // Both parts are optional. If not provided,
    // the output number defaults to 1
    // and only the first channel will be written to.
    class_addmethod(ears_outtilde_class, (method)ears_inouttilde_anything, "anything", A_GIMME, 0);
    
    class_addmethod(ears_outtilde_class, (method)ears_inouttilde_int, "int", A_LONG, 0);
    class_addmethod(ears_outtilde_class, (method)ears_inouttilde_float, "float", A_FLOAT, 0);
    class_addmethod(ears_outtilde_class, (method)ears_inouttilde_anything, "list", A_GIMME, 0);
    
    class_addmethod(ears_outtilde_class, (method)ears_outtilde_dsp64, "dsp64", A_CANT, 0);
    class_addmethod(ears_outtilde_class, (method)ears_outtilde_setchanmap, "setchanmap", A_CANT, 0);

    class_dspinit(ears_outtilde_class);
    
    class_register(CLASS_BOX, ears_outtilde_class);
    
    return 0;
}

void *ears_outtilde_new(t_symbol *s, t_atom_long ac, t_atom* av)
{
    t_ears_outtilde *x = (t_ears_outtilde*) object_alloc(ears_outtilde_class);
    
    // @arg 0 @name outlet_and_channels @optional 1 @type llll
    // @description The llll is composed of two parts:<br/>
    // - an integer, setting the number of the
    // <o>ears.process~</o> object's outlet
    // to which samples will be passed by <o>ears.out~</o>; and<br/>
    // - a sublist containing the channel numbers of the output buffer
    // <o>ears.out~</o> will write to.
    // A corresponding number of outlets will be created for <o>ears.in~</o><br/>
    // As a shortcut, if just a flat llll is provided then
    // its first element is interpreted as the outlet number
    // and its subsequent elements are interpreted as the channel indices.<br/>
    // Both parts are optional. If not provided,
    // the output number defaults to 1
    // and only the first channel will be written to.
    
    x->io_obj.earsProcessParent = getParentEarsProcess((t_object *) x);
    
    x->io_obj.ioNum = 1;
    x->io_obj.nChans = 1;
    x->io_obj.chan[0] = 1;
    
    long nChans = ears_inouttilde_anything((t_ears_inouttilde*) x, nullptr, attr_args_offset(ac, av), av);
    
    if (nChans == -1) {
        object_free(x);
        return nullptr;
    }
    if (nChans > 0) {
        x->io_obj.nChans = nChans;
    }
    
    dsp_setup((t_pxobject *) x, x->io_obj.nChans);
    
    if (x->io_obj.earsProcessParent)
        object_method(x->io_obj.earsProcessParent, gensym("ears.out~_created"), x->io_obj.ioNum, x);
    
    return x;
}

void ears_outtilde_free(t_ears_outtilde *x)
{
    if (x->io_obj.earsProcessParent)
        object_method(x->io_obj.earsProcessParent, gensym("ears.out~_deleted"), x);
    dsp_free((t_pxobject*) x);
}

void ears_outtilde_assist(t_ears_outtilde *x, void *b, long m, long a, char *s)
{
    sprintf(s, "(signal) To buffer channel %ld", x->io_obj.chan[a]); // @out 0 @type signal @loop 1 @digest Output signal
}

void ears_outtilde_perform64(t_ears_outtilde *x, t_dspchain *dsp64, double **ins, long numins, double **outs, long numouts, long vec_size, long flags, void *userparam)
{
    if (!x->chanMap)
        return;
    
    t_atom_long pos = x->io_obj.position;
    
    for (int i = 0; i < x->io_obj.nChans; i++) {
        audioChannel *ch = x->chanMap->getChannel(x->io_obj.ioNum, x->io_obj.chan[i]);
        ch->skipTo(pos);
        ch->insert(vec_size, ins[i]);
    }
    x->io_obj.position += vec_size;
}

void ears_outtilde_dsp64(t_ears_outtilde *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
    if (x->io_obj.earsProcessParent)
        object_method(dsp64, gensym("dsp_add64"), x, ears_outtilde_perform64, 0, NULL);
    x->io_obj.position = 0;
}

void ears_outtilde_setchanmap(t_ears_outtilde *x, audioChanMap* map)
{
    x->chanMap = map;
}

 
