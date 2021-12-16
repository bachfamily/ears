/**
	@file
	ears.cqt.c
 
	@name
	ears.cqt~
 
	@realname
	ears.cqt~
 
	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	Constant-Q Transform
 
	@description
	Computes the constant-Q transform of an incoming buffer
 
	@discussion
 
	@category
	ears spectral
 
	@keywords
	buffer, cqt, q, constant, fourier, transform, spectrum
 
	@seealso
	ears.window~, ears.stft~
	
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
#include "ears.essentia_commons.h"

typedef struct _buf_cqt {
    t_earsbufobj       e_ob;

    long polar;
    long fullspectrum;
    long downmix;
    
    long               a_binsPerOctave;
    double             a_minPitch;
    long               a_numberBins;
    double             a_threshold;
    long               a_minimumKernelSize;
    long               a_scale;

} t_buf_cqt;



// Prototypes
t_buf_cqt*     buf_cqt_new(t_symbol *s, short argc, t_atom *argv);
void			buf_cqt_free(t_buf_cqt *x);
void			buf_cqt_bang(t_buf_cqt *x);
void			buf_cqt_anything(t_buf_cqt *x, t_symbol *msg, long ac, t_atom *av);

void buf_cqt_assist(t_buf_cqt *x, void *b, long m, long a, char *s);
void buf_cqt_inletinfo(t_buf_cqt *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(cqt)

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
    
    CLASS_NEW_CHECK_SIZE(c, "ears.cqt~",
                         (method)buf_cqt_new,
                         (method)buf_cqt_free,
                         sizeof(t_buf_cqt),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    
    // @method list/llll @digest Process buffers
    // @description A list or llll with buffer names will trigger the buffer processing and output the processed
    // buffer names (depending on the <m>naming</m> attribute).
    EARSBUFOBJ_DECLARE_COMMON_METHODS_HANDLETHREAD(cqt)
    
    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_naming_attr(c);
    earsbufobj_class_add_timeunit_attr(c);
    earsbufobj_class_add_antimeunit_attr(c);
    earsbufobj_class_add_angleunit_attr(c);
    earsbufobj_class_add_pitchunit_attr(c);

    earsbufobj_class_add_framesize_attr(c);
    earsbufobj_class_add_hopsize_attr(c);
    earsbufobj_class_add_numframes_attr(c);
    earsbufobj_class_add_overlap_attr(c);
    earsbufobj_class_add_wintype_attr(c);
    earsbufobj_class_add_winstartfromzero_attr(c);

    CLASS_ATTR_LONG(c, "binsperoctave", 0, t_buf_cqt, a_binsPerOctave);
    CLASS_ATTR_STYLE_LABEL(c,"binsperoctave",0,"text","Bins Per Octave");
    CLASS_ATTR_BASIC(c, "binsperoctave", 0);
    CLASS_ATTR_CATEGORY(c, "binsperoctave", 0, "Analysis");
    // @description Sets the number of bins per octave

    CLASS_ATTR_LONG(c, "numbins", 0, t_buf_cqt, a_numberBins);
    CLASS_ATTR_STYLE_LABEL(c,"numbins",0,"text","Total Number of Bins");
    CLASS_ATTR_BASIC(c, "numbins", 0);
    CLASS_ATTR_CATEGORY(c, "numbins", 0, "Analysis");
    // @description Sets the total number of bins.

    CLASS_ATTR_DOUBLE(c, "minpitch", 0, t_buf_cqt, a_minPitch);
    CLASS_ATTR_STYLE_LABEL(c,"minpitch",0,"text","Minimum Pitch");
    CLASS_ATTR_BASIC(c, "minpitch", 0);
    CLASS_ATTR_CATEGORY(c, "minpitch", 0, "Analysis");
    // @description Sets the minimum pitch (in the <m>pitchunit</m>).

    CLASS_ATTR_DOUBLE(c, "thresh", 0, t_buf_cqt, a_threshold);
    CLASS_ATTR_STYLE_LABEL(c,"thresh",0,"text","Threshold");
    CLASS_ATTR_CATEGORY(c, "thresh", 0, "Analysis");
    // @description Bins whose magnitude is below this quantile are discarded.

    CLASS_ATTR_DOUBLE(c, "scale", 0, t_buf_cqt, a_scale);
    CLASS_ATTR_STYLE_LABEL(c,"scale",0,"text","Filters Scale");
    CLASS_ATTR_CATEGORY(c, "scale", 0, "Analysis");
    // @description Filters scale (larger values use longer windows).

    CLASS_ATTR_LONG(c, "minkernelsize", 0, t_buf_cqt, a_minimumKernelSize);
    CLASS_ATTR_STYLE_LABEL(c,"minkernelsize",0,"text","Minimum Kernel Size");
    CLASS_ATTR_CATEGORY(c, "minkernelsize", 0, "Analysis");
    // @description Sets the minimum size allowed for frequency kernels.
    
    
    CLASS_ATTR_LONG(c, "polar",    0,    t_buf_cqt, polar);
    CLASS_ATTR_STYLE_LABEL(c, "polar", 0, "onoff", "Polar Output");
    CLASS_ATTR_BASIC(c, "polar", 0);
    // @description Output data in polar coordinates, instead of cartesian ones.
    // Default is 1.

    CLASS_ATTR_LONG(c, "downmix",    0,    t_buf_cqt, downmix);
    CLASS_ATTR_STYLE_LABEL(c, "downmix", 0, "onoff", "Downmix to Mono");
    CLASS_ATTR_BASIC(c, "downmix", 0);
    // @description Toggles the ability to downmix all the channels into one. If this flag is not set, then
    // one buffer per channel is output. By default downmix is 1.

    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
    return 0;
}

void buf_cqt_assist(t_buf_cqt *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
            // @in 0 @type symbol @digest Buffer containing audio
            // @description Source audio buffer
        sprintf(s, x->polar ? "symbol: Source Buffer or Buffer with Magnitude Bins" : "symbol: Source Buffer or Buffer with Real/x Bins");
    } else {
        // @out 0 @type symbol @digest Buffer containing output magnitudes or real (x) parts, one bin per channel
        // @out 1 @type symbol @digest Buffer containing output phases (in the <m>angleunit</m> coordinate) or imaginary (y) parts, one bin per channel
        switch (a) {
            case 0: sprintf(s, x->polar ? "symbol: Buffer Containing STFT Magnitudes" : "symbol: Buffer Containing STFT Real/x Parts");    break;
            case 1: sprintf(s, x->polar ? "symbol: Buffer Containing STFT Phases" : "symbol: Buffer Containing STFT Imaginary/y Parts");    break;
        }
    }
}

void buf_cqt_inletinfo(t_buf_cqt *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_cqt *buf_cqt_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_cqt *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_cqt*)object_alloc_debug(s_tag_class);
    if (x) {
        x->polar = 1;
        x->fullspectrum = 0;
        x->downmix = 1;
        
        x->a_binsPerOctave = 12;
        x->a_minPitch = 2400;
        x->a_minimumKernelSize = 4;
        x->a_numberBins = 84;
        x->a_threshold = 0.01;
        x->a_scale = 1;

        earsbufobj_init((t_earsbufobj *)x, EARSBUFOBJ_FLAG_NONE); // EARSBUFOBJ_FLAG_SUPPORTS_COPY_NAMES);

        // @arg 0 @name outname @optional 1 @type symbol
        // @digest Output buffer name
        // @description @copy EARS_DOC_OUTNAME_ATTR
        
        t_llll *args = llll_parse(true_ac, argv);
        t_llll *names = earsbufobj_extract_names_from_args((t_earsbufobj *)x, args);

        attr_args_process(x, argc, argv);

        earsbufobj_setup((t_earsbufobj *)x, "e", "EE", names);

        llll_free(args);
        llll_free(names);
    }
    return x;
}


void buf_cqt_free(t_buf_cqt *x)
{
    earsbufobj_free((t_earsbufobj *)x);
}


t_ears_essentia_analysis_params buf_cqt_get_params(t_buf_cqt *x, t_buffer_obj *buf)
{
    t_ears_essentia_analysis_params params = earsbufobj_get_essentia_analysis_params((t_earsbufobj *)x, buf);
    
    params.CQT_binsPerOctave = x->a_binsPerOctave;
    params.CQT_minFrequency = ears_cents_to_hz(earsbufobj_pitch_to_cents((t_earsbufobj *)x, x->a_minPitch), EARS_MIDDLE_A_TUNING);
    params.CQT_numberBins = x->a_numberBins;
    params.CQT_threshold = x->a_threshold;
    params.CQT_minimumKernelSize = x->a_minimumKernelSize;
    params.CQT_scale = x->a_scale;
    return params;
}

void buf_cqt_bang(t_buf_cqt *x)
{
    long num_buffers = earsbufobj_get_instore_size((t_earsbufobj *)x, 0);
    long downmix = x->downmix;
    
    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_buffers, true);

    if (num_buffers > 0) { // must be just 1 buffer
        t_buffer_obj *in = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, 0);
        long num_channels = ears_buffer_get_numchannels((t_object *)x, in);

        if (downmix) {
            earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_OUT, 0, 1, true);
            earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_OUT, 1, 1, true);
            earsbufobj_refresh_outlet_names((t_earsbufobj *)x);

            t_buffer_obj *out1 = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, 0);
            t_buffer_obj *out2 = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 1, 0);
            std::vector<float> data = ears_buffer_get_sample_vector_mono((t_object *)x, in);
            
            t_ears_essentia_analysis_params params = buf_cqt_get_params(x, in);

            ears_vector_cqt((t_object *)x, data, ears_buffer_get_sr((t_object *)x, in), out1, out2, x->polar, &params, (e_ears_angleunit)x->e_ob.l_angleunit);
        } else if (num_channels > 0){
            earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_OUT, 0, num_channels, true);
            earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_OUT, 1, num_channels, true);
            earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
            
            for (long c = 0; c < num_channels; c++) {
                t_buffer_obj *out1 = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, c);
                t_buffer_obj *out2 = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 1, c);
                std::vector<float> data = ears_buffer_get_sample_vector_channel((t_object *)x, in, c);
                
                t_ears_essentia_analysis_params params = buf_cqt_get_params(x, in);

                ears_vector_cqt((t_object *)x, data, ears_buffer_get_sr((t_object *)x, in), out1, out2, x->polar, &params, (e_ears_angleunit)x->e_ob.l_angleunit);
            }
        } else {
            object_error((t_object *)x, "No channels in buffer!");
        }
    }
    
    earsbufobj_outlet_buffer((t_earsbufobj *)x, 1);
    earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
}



void buf_cqt_anything(t_buf_cqt *x, t_symbol *msg, long ac, t_atom *av)
{
    long inlet = earsbufobj_proxy_getinlet((t_earsbufobj *) x);

    t_llll *parsed = earsbufobj_parse_gimme((t_earsbufobj *) x, LLLL_OBJ_VANILLA, msg, ac, av);
    if (!parsed) return;
    
    if (parsed && parsed->l_head) {
        if (hatom_gettype(&parsed->l_head->l_hatom) == H_SYM) {
            
//                earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
            
            if (inlet == 0) {
                long num_bufs = llll_get_num_symbols_root(parsed);
                earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_bufs, true);
                earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_OUT, 0, num_bufs, true);
                earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_OUT, 1, num_bufs, true);
                earsbufobj_store_buffer_list((t_earsbufobj *)x, parsed, 0);
                
                buf_cqt_bang(x);
            } else {
                long num_bufs = llll_get_num_symbols_root(parsed);
                earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 1, num_bufs, true);
                earsbufobj_store_buffer_list((t_earsbufobj *)x, parsed, 1);
            }
        }
    }
    llll_free(parsed);
}


