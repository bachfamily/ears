/**
	@file
	ears.model.stochastic.analysis.c
 
	@name
	ears.model.stochastic.analysis~
 
	@realname
	ears.model.stochastic.analysis~
 
	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	Sinusoidal-plus-residual analysis model
 
	@description
	Applies the Essentia sinusoidal-plus-residual analysis model
 
	@discussion
 
	@category
	ears spectral
 
	@keywords
	buffer, model, analysis, resynthesis
 
	@seealso
	ears.model.stochastic.synthesis~
	
	@owner
	Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "llllobj.h"
#include "llll_commons_ext.h"
#include "bach_math_utilities.h"
#include "ears.object.h"
#include "ears.spectral.h"
#include "ears.essentia_models.h"


typedef struct _buf_model_stochastic_analysis {
    t_earsbufobj                e_ob;
        
    double  e_sampleRate;
    double  e_stocf;

    long    e_downmix;
} t_buf_model_stochastic_analysis;



// Prototypes
t_buf_model_stochastic_analysis*     buf_model_stochastic_analysis_new(t_symbol *s, short argc, t_atom *argv);
void			buf_model_stochastic_analysis_free(t_buf_model_stochastic_analysis *x);
void			buf_model_stochastic_analysis_bang(t_buf_model_stochastic_analysis *x);
void			buf_model_stochastic_analysis_anything(t_buf_model_stochastic_analysis *x, t_symbol *msg, long ac, t_atom *av);

void buf_model_stochastic_analysis_assist(t_buf_model_stochastic_analysis *x, void *b, long m, long a, char *s);
void buf_model_stochastic_analysis_inletinfo(t_buf_model_stochastic_analysis *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_model_stochastic_analysis_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(model_stochastic_analysis)

/**********************************************************************/
// Class Definition and Life Cycle

int C74_EXPORT main(void)
{
    ears_essentia_init();
    common_symbols_init();
    llllobj_common_symbols_init();
    
    if (llllobj_check_version(bach_get_current_llll_version()) || llllobj_test()) {
        ears_error_bachcheck();
        return 1;
    }
    
    t_class *c;
    
    CLASS_NEW_CHECK_SIZE(c, "ears.model.stochastic.analysis~",
                         (method)buf_model_stochastic_analysis_new,
                         (method)buf_model_stochastic_analysis_free,
                         sizeof(t_buf_model_stochastic_analysis),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    
    // @method list/llll @digest Process buffers
    // @description A list or llll with buffer names will trigger the buffer processing and output the processed
    // buffer names (depending on the <m>naming</m> attribute).
    EARSBUFOBJ_DECLARE_COMMON_METHODS_HANDLETHREAD(model_stochastic_analysis)
    
    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_blocking_attr(c);
    earsbufobj_class_add_naming_attr(c);
    earsbufobj_class_add_timeunit_attr(c);
    earsbufobj_class_add_antimeunit_attr(c);
    earsbufobj_class_add_ampunit_attr(c);

    earsbufobj_class_add_framesize_attr(c);
    earsbufobj_class_add_hopsize_attr(c);
    earsbufobj_class_add_numframes_attr(c);
    earsbufobj_class_add_overlap_attr(c);

    

    CLASS_ATTR_DOUBLE(c, "stocf", 0, t_buf_model_stochastic_analysis, e_stocf);
    CLASS_ATTR_STYLE_LABEL(c,"stocf",0,"text","Stochastic Decimation Factor");
    CLASS_ATTR_CATEGORY(c, "stocf", 0, "Analysis");
    // @description Sets the decimation factor used for the stochastic approximation.

    
    CLASS_ATTR_LONG(c, "downmix",    0,    t_buf_model_stochastic_analysis, e_downmix);
    CLASS_ATTR_STYLE_LABEL(c, "downmix", 0, "onoff", "Downmix to Mono");
    CLASS_ATTR_BASIC(c, "downmix", 0);
    // @description Toggles the ability to downmix all the channels into one. If this flag is not set, then
    // one buffer per channel is output. By default downmix is 0.

    class_register(CLASS_BOX, c);
    s_model_stochastic_analysis_class = c;
    ps_event = gensym("event");
    return 0;
}

void buf_model_stochastic_analysis_assist(t_buf_model_stochastic_analysis *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
            // @in 0 @type symbol @digest Input buffer
            // @description Source audio buffer
        sprintf(s, "symbol: Source Buffer");
    } else {
        char *type = NULL;
        llllobj_get_llll_outlet_type_as_string((t_object *) x, LLLL_OBJ_VANILLA, a, &type);
        // @out 0 @type llll @digest Stochastic envelopes @description Stochastic envelope (one level for each channel, then one level for each frame).
        switch (a) {
            case 0: sprintf(s, "llll (%s): Stochastic envelopes", type);    break;
        }
    }
}

void buf_model_stochastic_analysis_inletinfo(t_buf_model_stochastic_analysis *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}



t_buf_model_stochastic_analysis *buf_model_stochastic_analysis_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_model_stochastic_analysis *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_model_stochastic_analysis*)object_alloc_debug(s_model_stochastic_analysis_class);
    if (x) {
        x->e_stocf = 0.2;
        x->e_downmix = 0;

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

        earsbufobj_setup((t_earsbufobj *)x, "e", "4", names);

        llll_free(args);
        llll_free(names);
    }
    return x;
}


void buf_model_stochastic_analysis_free(t_buf_model_stochastic_analysis *x)
{
    earsbufobj_free((t_earsbufobj *)x);
}


void buf_model_stochastic_analysis_bang(t_buf_model_stochastic_analysis *x)
{
    long num_buffers = ((t_earsbufobj *)x)->l_instore[0].num_stored_bufs;
    long downmix = x->e_downmix;
    
    earsbufobj_mutex_lock((t_earsbufobj *)x);

    if (num_buffers >= 1) {
        t_buffer_obj *inbuf = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, 0);
        t_llll *stocenv = NULL;
        long num_channels;
        
        if (!inbuf) {
            object_error((t_object *)x, "No input buffer!");
            earsbufobj_mutex_unlock((t_earsbufobj *)x);
            return;
        }
        
        t_ears_essentia_analysis_params params = earsbufobj_get_essentia_analysis_params((t_earsbufobj *)x, inbuf);
        std::vector<std::vector<float>> samps;
        if (downmix) {
            samps.push_back(ears_buffer_get_sample_vector_mono((t_object *)x, inbuf));
        } else {
            samps = ears_buffer_get_sample_vector((t_object *)x, inbuf);
        }
        num_channels = samps.size();
        stocenv = llll_get();
        
        double sr = ears_buffer_get_sr((t_object *)x, inbuf);
        std::vector<std::vector<std::vector<Real>>> framewiseResiduals;
        for (long i = 0; i < num_channels; i++) {
            t_llll *channel_stocenv = NULL;
            t_ears_err err = ears_model_stochastic_analysis((t_object *)x, samps[i], sr,
                                                            &channel_stocenv,
                                                            i,
                                                            &params,
                                                            (e_ears_ampunit) x->e_ob.l_ampunit,
                                                            x->e_stocf
                                                            );
            if (err == EARS_ERR_NONE) {
                llll_appendllll(stocenv, channel_stocenv);
            }
        }

        
        earsbufobj_outlet_llll((t_earsbufobj *)x, 0, stocenv);
        llll_free(stocenv);
    }
    
    earsbufobj_mutex_unlock((t_earsbufobj *)x);
}



void buf_model_stochastic_analysis_anything(t_buf_model_stochastic_analysis *x, t_symbol *msg, long ac, t_atom *av)
{
    long inlet = earsbufobj_proxy_getinlet((t_earsbufobj *) x);

    t_llll *parsed = earsbufobj_parse_gimme((t_earsbufobj *) x, LLLL_OBJ_VANILLA, msg, ac, av);
    if (!parsed) return;
    
    if (parsed && parsed->l_head) {
        if (hatom_gettype(&parsed->l_head->l_hatom) == H_SYM) {
            if (inlet == 0) {
                long num_bufs = llll_get_num_symbols_root(parsed);
                earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_bufs, true);
                earsbufobj_store_buffer_list((t_earsbufobj *)x, parsed, 0);
                
                buf_model_stochastic_analysis_bang(x);
            }
        }
    }
    llll_free(parsed);
}


