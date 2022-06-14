/**
 @file
 ears.model.stochastic.synthesis.c
 
 @name
 ears.model.stochastic.synthesis~
 
 @realname
 ears.model.stochastic.synthesis~
 
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
 ears.model.stochastic.analysis~
 
 @owner
 Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "foundation/llllobj.h"
#include "foundation/llll_commons_ext.h"
#include "math/bach_math_utilities.h"
#include "ears.object.h"
#include "ears.spectral.h"
#include "ears.essentia_models.h"


typedef struct _buf_model_stochastic_synthesis {
    t_earsbufobj                e_ob;

    t_llll  *stocenv;

    double  e_sampleRate;
    double  e_stocf;
} t_buf_model_stochastic_synthesis;



// Prototypes
t_buf_model_stochastic_synthesis*     buf_model_stochastic_synthesis_new(t_symbol *s, short argc, t_atom *argv);
void			buf_model_stochastic_synthesis_free(t_buf_model_stochastic_synthesis *x);
void			buf_model_stochastic_synthesis_bang(t_buf_model_stochastic_synthesis *x);
void			buf_model_stochastic_synthesis_anything(t_buf_model_stochastic_synthesis *x, t_symbol *msg, long ac, t_atom *av);

void buf_model_stochastic_synthesis_assist(t_buf_model_stochastic_synthesis *x, void *b, long m, long a, char *s);
void buf_model_stochastic_synthesis_inletinfo(t_buf_model_stochastic_synthesis *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_model_stochastic_synthesis_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(model_stochastic_synthesis)

/**********************************************************************/
// Class Definition and Life Cycle

void C74_EXPORT ext_main(void* moduleRef)
{
    ears_essentia_init();
    common_symbols_init();
    llllobj_common_symbols_init();
    
    if (llllobj_check_version(bach_get_current_llll_version()) || llllobj_test()) {
        ears_error_bachcheck();
        return;
    }
    
    t_class *c;
    
    CLASS_NEW_CHECK_SIZE(c, "ears.model.stochastic.synthesis~",
                         (method)buf_model_stochastic_synthesis_new,
                         (method)buf_model_stochastic_synthesis_free,
                         sizeof(t_buf_model_stochastic_synthesis),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    
    // @method list/llll @digest Process buffers
    // @description A list or llll with buffer names will trigger the buffer processing and output the processed
    // buffer names (depending on the <m>naming</m> attribute).
    EARSBUFOBJ_DECLARE_COMMON_METHODS_HANDLETHREAD(model_stochastic_synthesis)
    
    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_blocking_attr(c);
    earsbufobj_class_add_naming_attr(c);
    earsbufobj_class_add_antimeunit_attr(c);
    earsbufobj_class_add_ampunit_attr(c);

    earsbufobj_class_add_framesize_attr(c);
    earsbufobj_class_add_hopsize_attr(c);


    
    CLASS_ATTR_DOUBLE(c, "sr", 0, t_buf_model_stochastic_synthesis, e_sampleRate);
    CLASS_ATTR_BASIC(c, "sr", 0);
    CLASS_ATTR_STYLE_LABEL(c,"sr",0,"text","Output Sample Rate");
    // @description Sets the output sample rate; leave 0 to pick the current one from Max.

    
    CLASS_ATTR_DOUBLE(c, "stocf", 0, t_buf_model_stochastic_synthesis, e_stocf);
    CLASS_ATTR_STYLE_LABEL(c,"stocf",0,"text","Stochastic Decimation Factor");
    CLASS_ATTR_CATEGORY(c, "stocf", 0, "Synthesis");
    // @description Sets the decimation factor of the stochastic approximation.

    
    class_register(CLASS_BOX, c);
    s_model_stochastic_synthesis_class = c;
    ps_event = gensym("event");
    return 0;
}

void buf_model_stochastic_synthesis_assist(t_buf_model_stochastic_synthesis *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        // @in 0 @type llll @digest Stochastic envelopes @description Stochastic envelopes (one level for each channel, then one level for each frame)
        switch (a) {
            case 0: sprintf(s, "llll: Stochastic envelope");    break;
        }
    } else {
        // @out 0 @type symbol @digest Synthesized buffer
        switch (a) {
            case 0: sprintf(s, "symbol: Synthesized buffer"); break;
        }
    }
}

void buf_model_stochastic_synthesis_inletinfo(t_buf_model_stochastic_synthesis *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}



t_buf_model_stochastic_synthesis *buf_model_stochastic_synthesis_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_model_stochastic_synthesis *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_model_stochastic_synthesis*)object_alloc_debug(s_model_stochastic_synthesis_class);
    if (x) {
        x->e_sampleRate = 0;
        x->stocenv = llll_get();
        x->e_stocf = 0.2;

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

        earsbufobj_setup((t_earsbufobj *)x, "4", "e", names);

        llll_free(args);
        llll_free(names);
    }
    return x;
}


void buf_model_stochastic_synthesis_free(t_buf_model_stochastic_synthesis *x)
{
    llll_free(x->stocenv);
    earsbufobj_free((t_earsbufobj *)x);
}


void buf_model_stochastic_synthesis_bang(t_buf_model_stochastic_synthesis *x)
{
    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_OUT, 0, 1, true);
    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);

    earsbufobj_mutex_lock((t_earsbufobj *)x);
    
    t_ears_essentia_analysis_params params = earsbufobj_get_essentia_analysis_params((t_earsbufobj *)x, NULL);
    t_buffer_obj *outbuf = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, 0);
    
    double sr = x->e_sampleRate > 0 ? x->e_sampleRate : sys_getsr();
    long numchannels = x->stocenv->l_size;
    if (numchannels > 0) {
        ears_buffer_set_sr((t_object *)x, outbuf, sr);
        ears_buffer_set_numchannels((t_object *)x, outbuf, numchannels);

        t_llllelem *stocenv_el = x->stocenv->l_head;
        for (long i = 0; i < numchannels && stocenv_el; stocenv_el = stocenv_el->l_next, i++) {
            t_ears_err err = ears_model_stochastic_synthesis((t_object *)x, sr,
                                                      hatom_getllll(&stocenv_el->l_hatom),
                                                      outbuf,
                                                      &params,
                                                      (e_ears_ampunit) x->e_ob.l_ampunit,
                                                      i,
                                                      x->e_stocf);
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



void buf_model_stochastic_synthesis_anything(t_buf_model_stochastic_synthesis *x, t_symbol *msg, long ac, t_atom *av)
{
    long inlet = earsbufobj_proxy_getinlet((t_earsbufobj *) x);

    t_llll *parsed = earsbufobj_parse_gimme((t_earsbufobj *) x, LLLL_OBJ_VANILLA, msg, ac, av);
    if (!parsed) return;
    
    if (parsed && parsed->l_head) {
        if (inlet == 0) {
            earsbufobj_mutex_lock((t_earsbufobj *)x);
            llll_free(x->stocenv);
            x->stocenv = llll_clone(parsed);
            earsbufobj_mutex_unlock((t_earsbufobj *)x);
            
            buf_model_stochastic_synthesis_bang(x);
        }
    }
    llll_free(parsed);
}


