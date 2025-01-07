/**
 @file
 ears.model.ptrack.synthesis.c
 
 @name
 ears.model.ptrack.synthesis~
 
 @realname
 ears.model.ptrack.synthesis~
 
 @type
 object
 
 @module
 ears
 
 @author
 Daniele Ghisi
 
 @digest
 Partial tracking synthesis model
 
 @description
 Applies a partial tracking synthesis model.
 
 @discussion
 
 @category
 ears spectral
 
 @keywords
 buffer, model, partial tracking, partials, sinusoid, synthesis, resynthesis
 
 @seealso
 ears.model.ptrack.analysis~
 
 @owner
 Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "foundation/llllobj.h"
#include "foundation/llll_commons_ext.h"
#include "math/bach_math_utilities.h"
#include "ears.scores.h"
#include "ears.object.h"
#include "ears.spectral.h"

typedef struct _buf_model_ptrack_synthesis {
    t_earsbufobj                e_ob;

    t_llll  *partials;
    
    char    normalize;
    
    double  e_sampleRate;
} t_buf_model_ptrack_synthesis;



// Prototypes
t_buf_model_ptrack_synthesis*     buf_model_ptrack_synthesis_new(t_symbol *s, short argc, t_atom *argv);
void			buf_model_ptrack_synthesis_free(t_buf_model_ptrack_synthesis *x);
void			buf_model_ptrack_synthesis_bang(t_buf_model_ptrack_synthesis *x);
void			buf_model_ptrack_synthesis_anything(t_buf_model_ptrack_synthesis *x, t_symbol *msg, long ac, t_atom *av);

void buf_model_ptrack_synthesis_assist(t_buf_model_ptrack_synthesis *x, void *b, long m, long a, char *s);
void buf_model_ptrack_synthesis_inletinfo(t_buf_model_ptrack_synthesis *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_model_ptrack_synthesis_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(model_ptrack_synthesis)

/**********************************************************************/
// Class Definition and Life Cycle

void C74_EXPORT ext_main(void* moduleRef)
{
    common_symbols_init();
    llllobj_common_symbols_init();
    
    if (llllobj_check_version(bach_get_current_llll_version()) || llllobj_test()) {
        ears_error_bachcheck();
        return;
    }
    
    t_class *c;
    
    CLASS_NEW_CHECK_SIZE(c, "ears.model.ptrack.synthesis~",
                         (method)buf_model_ptrack_synthesis_new,
                         (method)buf_model_ptrack_synthesis_free,
                         sizeof(t_buf_model_ptrack_synthesis),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    
    // @method list/llll @digest Process buffers
    // @description A list or llll with buffer names will trigger the buffer processing and output the processed
    // buffer names (depending on the <m>alloc</m> attribute).
    EARSBUFOBJ_DECLARE_COMMON_METHODS_HANDLETHREAD(model_ptrack_synthesis)
    
    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_blocking_attr(c);
    earsbufobj_class_add_alloc_attr(c);
//    earsbufobj_class_add_ampunit_attr(c);
//    earsbufobj_class_add_angleunit_attr(c);
//    earsbufobj_class_add_frequnit_attr(c);


    
    CLASS_ATTR_DOUBLE(c, "sr", 0, t_buf_model_ptrack_synthesis, e_sampleRate);
    CLASS_ATTR_BASIC(c, "sr", 0);
    CLASS_ATTR_STYLE_LABEL(c,"sr",0,"text","Output Sample Rate");
    // @description Sets the output sample rate; leave 0 to pick the current one from Max.

    CLASS_ATTR_CHAR(c, "normalize", 0, t_buf_model_ptrack_synthesis, normalize);
    CLASS_ATTR_STYLE_LABEL(c,"normalize",0,"enumindex","Normalize Output");
    CLASS_ATTR_ENUMINDEX(c,"normalize", 0, "Never Always Overload Protection Only");
    // @description Sets the normalization mode for the output buffer:
    // 0 = Never, does not normalize; 1 = Always, always normalizes to 1.; 2 (default) = Overload Protection Only, only
    // normalizes to 1. if some samples exceed in modulo 1.

    
    class_register(CLASS_BOX, c);
    s_model_ptrack_synthesis_class = c;
    ps_event = gensym("event");
}

void buf_model_ptrack_synthesis_assist(t_buf_model_ptrack_synthesis *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        // @in 0 @type llll @digest Partials @description List of partials (one level for each partial, then one level for each peak, containing: onset, frequency, amplitude, phase)
        switch (a) {
            case 0: sprintf(s, "llll: Partials");    break;
        }
    } else {
        // @out 0 @type symbol @digest Synthesized buffer
         sprintf(s, "symbol: Synthesized buffer");
    }
}

void buf_model_ptrack_synthesis_inletinfo(t_buf_model_ptrack_synthesis *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}



t_buf_model_ptrack_synthesis *buf_model_ptrack_synthesis_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_model_ptrack_synthesis *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_model_ptrack_synthesis*)object_alloc_debug(s_model_ptrack_synthesis_class);
    if (x) {
        x->e_sampleRate = 0;
        x->partials = llll_get();

        earsbufobj_init((t_earsbufobj *)x, EARSBUFOBJ_FLAG_NONE); // EARSBUFOBJ_FLAG_SUPPORTS_COPY_NAMES);

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


void buf_model_ptrack_synthesis_free(t_buf_model_ptrack_synthesis *x)
{
    if (x->partials)
        llll_free(x->partials);
    earsbufobj_free((t_earsbufobj *)x);
}


void buf_model_ptrack_synthesis_bang(t_buf_model_ptrack_synthesis *x)
{
    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_OUT, 0, 1, true);
    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);

    earsbufobj_mutex_lock((t_earsbufobj *)x);
    
    t_buffer_obj *outbuf = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, 0);
    double sr = x->e_sampleRate > 0 ? x->e_sampleRate : sys_getsr();
    long numchannels = x->partials->l_size;
    if (numchannels > 0) {
        ears_buffer_set_sr((t_object *)x, outbuf, sr);
        ears_buffer_set_numchannels((t_object *)x, outbuf, numchannels);
        
        t_llll *roll_gs = ears_ptrack_to_roll((t_object *)x, x->partials, (e_ears_timeunit)x->e_ob.l_timeunit, (e_ears_frequnit)x->e_ob.l_frequnit, (e_ears_ampunit)x->e_ob.l_ampunit);

        char use_assembly_line = true;
        
        ears_roll_to_buffer((t_earsbufobj *)x, EARS_SCORETOBUF_MODE_SYNTHESIS, roll_gs, outbuf,
                            EARS_SYNTHMODE_SINUSOIDS, NULL, 0,
                            false, true, 1, //numchannels,
                            0, 0, 0, 0, 0, 0, 0, sr, (e_ears_normalization_modes)x->normalize, EARS_CHANNELCONVERTMODE_KEEP,
                            0, 0, EARS_FADE_NONE, EARS_FADE_NONE, 0., 0.,
//                            x->fadein_amount, x->fadeout_amount, (e_ears_fade_types)x->fadein_type, (e_ears_fade_types)x->fadeout_type, x->fadein_curve, x->fadeout_curve,
                            NULL, EARS_PAN_MODE_LINEAR, EARS_PAN_LAW_COSINE, 0., 1,
//                            x->panvoices,(e_ears_pan_modes)x->pan_mode, (e_ears_pan_laws)x->pan_law, x->multichannel_spread, x->compensate_multichannel_gain_to_avoid_clipping,
                            EARS_VELOCITY_TO_AMPLITUDE, 0, 1., 440., 0, x->e_ob.l_resamplingfilterwidth, false, use_assembly_line, true);
        
        llll_free(roll_gs);
        
        earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
    } else {
        object_error((t_object *)x, "Empty input data.");
    }

    
    earsbufobj_mutex_unlock((t_earsbufobj *)x);
}



void buf_model_ptrack_synthesis_anything(t_buf_model_ptrack_synthesis *x, t_symbol *msg, long ac, t_atom *av)
{
    long inlet = earsbufobj_proxy_getinlet((t_earsbufobj *) x);

    t_llll *parsed = earsbufobj_parse_gimme((t_earsbufobj *) x, LLLL_OBJ_VANILLA, msg, ac, av);
    if (!parsed) return;
    
    if (parsed && parsed->l_head) {
        if (inlet == 0) {
            earsbufobj_mutex_lock((t_earsbufobj *)x);
            llll_free(x->partials);
            x->partials = llll_clone(parsed);
            earsbufobj_mutex_unlock((t_earsbufobj *)x);
            
            buf_model_ptrack_synthesis_bang(x);
        }
    }
    llll_free(parsed);
}


