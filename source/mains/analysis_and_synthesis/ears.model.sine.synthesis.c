/**
 @file
 ears.model.sine.synthesis.c
 
 @name
 ears.model.sine.synthesis~
 
 @realname
 ears.model.sine.synthesis~
 
 @type
 object
 
 @module
 ears
 
 @author
 Daniele Ghisi
 
 @digest
 Sinusoidal synthesis model
 
 @description
 Applies the Essentia sinusoidal synthesis model
 
 @discussion
 
 @category
 ears spectral
 
 @keywords
 buffer, model, synthesis, resynthesis
 
 @seealso
 ears.model.sine.analysis~
 
 @owner
 Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "foundation/llllobj.h"
#include "foundation/llll_commons_ext.h"
#include "bach_math_utilities.h"
#include "ears.object.h"
#include "ears.spectral.h"
#include "ears.essentia_models.h"


typedef struct _buf_model_sine_synthesis {
    t_earsbufobj                e_ob;

    t_llll  *freqs;
    t_llll  *mags;
    t_llll  *phases;
    
    double  e_sampleRate;
} t_buf_model_sine_synthesis;



// Prototypes
t_buf_model_sine_synthesis*     buf_model_sine_synthesis_new(t_symbol *s, short argc, t_atom *argv);
void			buf_model_sine_synthesis_free(t_buf_model_sine_synthesis *x);
void			buf_model_sine_synthesis_bang(t_buf_model_sine_synthesis *x);
void			buf_model_sine_synthesis_anything(t_buf_model_sine_synthesis *x, t_symbol *msg, long ac, t_atom *av);

void buf_model_sine_synthesis_assist(t_buf_model_sine_synthesis *x, void *b, long m, long a, char *s);
void buf_model_sine_synthesis_inletinfo(t_buf_model_sine_synthesis *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_model_sine_synthesis_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(model_sine_synthesis)

/**********************************************************************/
// Class Definition and Life Cycle

void C74_EXPORT ext_main(void* moduleRef)
{
    ears_essentia_init();
    common_symbols_init();
    llllobj_common_symbols_init();
    
    if (llllobj_check_version(bach_get_current_llll_version()) || llllobj_test()) {
        ears_error_bachcheck();
        return 1;
    }
    
    t_class *c;
    
    CLASS_NEW_CHECK_SIZE(c, "ears.model.sine.synthesis~",
                         (method)buf_model_sine_synthesis_new,
                         (method)buf_model_sine_synthesis_free,
                         sizeof(t_buf_model_sine_synthesis),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    
    // @method list/llll @digest Process buffers
    // @description A list or llll with buffer names will trigger the buffer processing and output the processed
    // buffer names (depending on the <m>naming</m> attribute).
    EARSBUFOBJ_DECLARE_COMMON_METHODS_HANDLETHREAD(model_sine_synthesis)
    
    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_blocking_attr(c);
    earsbufobj_class_add_naming_attr(c);
    earsbufobj_class_add_antimeunit_attr(c);
    earsbufobj_class_add_ampunit_attr(c);
    earsbufobj_class_add_angleunit_attr(c);
    earsbufobj_class_add_frequnit_attr(c);

    earsbufobj_class_add_framesize_attr(c);
    earsbufobj_class_add_hopsize_attr(c);


    
    CLASS_ATTR_DOUBLE(c, "sr", 0, t_buf_model_sine_synthesis, e_sampleRate);
    CLASS_ATTR_BASIC(c, "sr", 0);
    CLASS_ATTR_STYLE_LABEL(c,"sr",0,"text","Output Sample Rate");
    // @description Sets the output sample rate; leave 0 to pick the current one from Max.

    class_register(CLASS_BOX, c);
    s_model_sine_synthesis_class = c;
    ps_event = gensym("event");
    return 0;
}

void buf_model_sine_synthesis_assist(t_buf_model_sine_synthesis *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        // @in 0 @type llll @digest Frequencies @description List of frequencies (one level for each channel, then one level for each frame)
        // @in 1 @type llll @digest Magnitudes @description List of magnitudes (one level for each channel, then one level for each frame)
        // @in 2 @type llll @digest Phases @description List of phases (one level for each channel, then one level for each frame)
        switch (a) {
            case 0: sprintf(s, "llll: Frequencies");    break;
            case 1: sprintf(s, "llll: Magnitudes");    break;
            case 2: sprintf(s, "llll: Phases");    break;
        }
    } else {
        // @out 0 @type symbol @digest Synthesized buffer
         sprintf(s, "symbol: Synthesized buffer");
    }
}

void buf_model_sine_synthesis_inletinfo(t_buf_model_sine_synthesis *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}



t_buf_model_sine_synthesis *buf_model_sine_synthesis_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_model_sine_synthesis *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_model_sine_synthesis*)object_alloc_debug(s_model_sine_synthesis_class);
    if (x) {
        x->e_sampleRate = 0;
        x->freqs = llll_get();
        x->mags = llll_get();
        x->phases = llll_get();

        earsbufobj_init((t_earsbufobj *)x, EARSBUFOBJ_FLAG_NONE); // EARSBUFOBJ_FLAG_SUPPORTS_COPY_NAMES);

        x->e_ob.a_framesize = 2048;
        x->e_ob.a_hopsize = 512;
        x->e_ob.a_overlap = 4;

        // @arg 0 @name outname @optional 1 @type symbol
        // @digest Output buffer name
        // @description @copy EARS_DOC_OUTNAME_ATTR
        
        t_llll *args = llll_parse(true_ac, argv);
        t_llll *names = earsbufobj_extract_names_from_args((t_earsbufobj *)x, args);

        attr_args_process(x, argc, argv);

        earsbufobj_setup((t_earsbufobj *)x, "444", "e", names);

        llll_free(args);
        llll_free(names);
    }
    return x;
}


void buf_model_sine_synthesis_free(t_buf_model_sine_synthesis *x)
{
    llll_free(x->freqs);
    llll_free(x->mags);
    llll_free(x->phases);
    earsbufobj_free((t_earsbufobj *)x);
}


void buf_model_sine_synthesis_bang(t_buf_model_sine_synthesis *x)
{
    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_OUT, 0, 1, true);
    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);

    earsbufobj_mutex_lock((t_earsbufobj *)x);
    
    t_ears_essentia_analysis_params params = earsbufobj_get_essentia_analysis_params((t_earsbufobj *)x, NULL);
    t_buffer_obj *outbuf = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, 0);
    double sr = x->e_sampleRate > 0 ? x->e_sampleRate : sys_getsr();
    long numchannels = x->freqs->l_size;
    if (numchannels > 0) {
        ears_buffer_set_sr((t_object *)x, outbuf, sr);
        ears_buffer_set_numchannels((t_object *)x, outbuf, numchannels);
        
        t_llllelem *freqs_el = x->freqs->l_head, *mags_el = x->mags->l_head, *phases_el = x->phases->l_head;
        for (long i = 0; i < numchannels && freqs_el && mags_el && phases_el;
             freqs_el = freqs_el->l_next, mags_el = mags_el->l_next, phases_el = phases_el->l_next, i++) {
            t_ears_err err = ears_model_sine_synthesis((t_object *)x, sr,
                                                 hatom_getllll(&freqs_el->l_hatom),
                                                 hatom_getllll(&mags_el->l_hatom),
                                                 hatom_getllll(&phases_el->l_hatom),
                                                 outbuf,
                                                 &params,
                                                 (e_ears_angleunit) x->e_ob.l_angleunit,
                                                 (e_ears_ampunit) x->e_ob.l_ampunit,
                                                 (e_ears_frequnit) x->e_ob.l_frequnit,
                                                 i);
            if (err) {
                object_error((t_object *)x, "Error during synthesis.");
            }
        }
        earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
    } else {
        object_error((t_object *)x, "Empty input data.");
    }

    
    earsbufobj_mutex_unlock((t_earsbufobj *)x);
}



void buf_model_sine_synthesis_anything(t_buf_model_sine_synthesis *x, t_symbol *msg, long ac, t_atom *av)
{
    long inlet = earsbufobj_proxy_getinlet((t_earsbufobj *) x);

    t_llll *parsed = earsbufobj_parse_gimme((t_earsbufobj *) x, LLLL_OBJ_VANILLA, msg, ac, av);
    if (!parsed) return;
    
    if (parsed && parsed->l_head) {
        if (inlet == 0) {
            earsbufobj_mutex_lock((t_earsbufobj *)x);
            llll_free(x->freqs);
            x->freqs = llll_clone(parsed);
            earsbufobj_mutex_unlock((t_earsbufobj *)x);
            
            buf_model_sine_synthesis_bang(x);
        } else if (inlet == 1) {
            earsbufobj_mutex_lock((t_earsbufobj *)x);
            llll_free(x->mags);
            x->mags = llll_clone(parsed);
            earsbufobj_mutex_unlock((t_earsbufobj *)x);
        } else if (inlet == 2) {
            earsbufobj_mutex_lock((t_earsbufobj *)x);
            llll_free(x->phases);
            x->phases = llll_clone(parsed);
            earsbufobj_mutex_unlock((t_earsbufobj *)x);
        }
    }
    llll_free(parsed);
}


