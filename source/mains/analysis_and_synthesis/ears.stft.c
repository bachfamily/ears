/**
	@file
	ears.stft.c
 
	@name
	ears.stft~
 
	@realname
	ears.stft~
 
	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	Compute spectrograms
 
	@description
	Computes the spectrogram of an incoming buffer via the Short-Time Fourier Transform
 
	@discussion
 
	@category
	ears spectral
 
	@keywords
	buffer, stft, fourier, transform, spectrum
 
	@seealso
	ears.istft~, ears.fft~, ears.window~
	
	@owner
	Daniele Ghisi
 */

//#define EARS_STFT_USE_ESSENTIA

#include "ext.h"
#include "ext_obex.h"
#include "foundation/llllobj.h"
#include "foundation/llll_commons_ext.h"
#include "math/bach_math_utilities.h"
#include "ears.object.h"
#include "ears.spectral.h"

#ifdef EARS_STFT_USE_ESSENTIA
#include "ears.essentia_commons.h"
#endif

typedef struct _buf_stft {
    t_earsbufobj       e_ob;

    long complex_input;
    
    long polar_input;
    long polar_output;
    long fullspectrum;
    long downmix;
    long unitary;
} t_buf_stft;



// Prototypes
t_buf_stft*     buf_stft_new(t_symbol *s, short argc, t_atom *argv);
void			buf_stft_free(t_buf_stft *x);
void			buf_stft_bang(t_buf_stft *x);
void			buf_stft_anything(t_buf_stft *x, t_symbol *msg, long ac, t_atom *av);

void buf_stft_assist(t_buf_stft *x, void *b, long m, long a, char *s);
void buf_stft_inletinfo(t_buf_stft *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(stft)

/**********************************************************************/
// Class Definition and Life Cycle

t_max_err buf_stft_setattr_cpxin(t_buf_stft *x, void *attr, long argc, t_atom *argv)
{
    if (argc && argv) {
        if (!x->e_ob.l_is_creating)
            object_error((t_object *)x, "The 'cpxin' attribute can only be set in the object box.");
        else if (atom_gettype(argv) == A_LONG)
            x->complex_input = atom_getlong(argv);
    }
    return MAX_ERR_NONE;
}


void C74_EXPORT ext_main(void* moduleRef)
{
#ifdef EARS_STFT_USE_ESSENTIA
    ears_essentia_init();
#endif
    common_symbols_init();
    llllobj_common_symbols_init();
    
    if (llllobj_check_version(bach_get_current_llll_version()) || llllobj_test()) {
        ears_error_bachcheck();
        return;
    }
    
    t_class *c;
    
    CLASS_NEW_CHECK_SIZE(c, "ears.stft~",
                         (method)buf_stft_new,
                         (method)buf_stft_free,
                         sizeof(t_buf_stft),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    
    // @method list/llll @digest Process buffers
    // @description A list or llll with buffer names will trigger the buffer processing and output the processed
    // buffer names (depending on the <m>naming</m> attribute).
    EARSBUFOBJ_DECLARE_COMMON_METHODS_HANDLETHREAD(stft)
    
    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_blocking_attr(c);
    earsbufobj_class_add_naming_attr(c);
    earsbufobj_class_add_timeunit_attr(c);
    earsbufobj_class_add_antimeunit_attr(c);
    earsbufobj_class_add_angleunit_attr(c);
    earsbufobj_class_add_winstartfromzero_attr(c);

    earsbufobj_class_add_framesize_attr(c);
    earsbufobj_class_add_hopsize_attr(c);
    earsbufobj_class_add_numframes_attr(c);
    earsbufobj_class_add_overlap_attr(c);
#ifdef EARS_STFT_USE_ESSENTIA
    earsbufobj_class_add_wintype_attr_essentia(c);
#else
    earsbufobj_class_add_wintype_attr(c);
#endif
    earsbufobj_class_add_winstartfromzero_attr(c);
    
    earsbufobj_class_add_polyout_attr(c);

    CLASS_ATTR_LONG(c, "cpxin",    0,    t_buf_stft, complex_input);
    CLASS_ATTR_STYLE_LABEL(c, "cpxin", 0, "onoff", "Complex Input");
    CLASS_ATTR_ACCESSORS(c, "cpxin", NULL, buf_stft_setattr_cpxin);
    // @description Input complex data instead of real-valued audio samples.
    // This attribute is static and can only be entered in the object box.

    CLASS_ATTR_LONG(c, "polarin",    0,    t_buf_stft, polar_input);
    CLASS_ATTR_STYLE_LABEL(c, "polarin", 0, "onoff", "Polar Input");
    // @description Input data in polar coordinates, instead of cartesian ones.
    // Default is 0.

    
    CLASS_ATTR_LONG(c, "polarout",    0,    t_buf_stft, polar_output);
    CLASS_ATTR_STYLE_LABEL(c, "polarout", 0, "onoff", "Polar Output");
    CLASS_ATTR_BASIC(c, "polarout", 0);
    // @description Output data in polar coordinates, instead of cartesian ones.
    // Default is 1.

    CLASS_ATTR_LONG(c, "fullspectrum",    0,    t_buf_stft, fullspectrum);
    CLASS_ATTR_STYLE_LABEL(c, "fullspectrum", 0, "onoff", "Full Spectrum");
    CLASS_ATTR_BASIC(c, "fullspectrum", 0);
    // @description Output full spectrum; if not set, it will output the first half of the spectrum only.

    CLASS_ATTR_LONG(c, "downmix",    0,    t_buf_stft, downmix);
    CLASS_ATTR_STYLE_LABEL(c, "downmix", 0, "onoff", "Downmix to Mono");
    CLASS_ATTR_BASIC(c, "downmix", 0);
    // @description Toggles the ability to downmix all the channels into one. If this flag is not set, then
    // one buffer per channel is output.

    CLASS_ATTR_LONG(c, "unitary",    0,    t_buf_stft, unitary);
    CLASS_ATTR_STYLE_LABEL(c,"unitary",0,"onoff","Unitary");
    CLASS_ATTR_BASIC(c, "unitary", 0);
    // @description Toggles the unitary normalization of the Fourier Transform (so that the
    // if coincides with its inverse up to conjugation).

    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
}

void buf_stft_assist(t_buf_stft *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        // @out 0 @type symbol @digest Source audio buffer - or input magnitudes, or real (x) parts
        // @description If the <m>cpxin</m> attribute is not set, the object expects as input a buffer with the original audio content.
        // If the <m>cpxin</m> attribute is set, then the input expects a buffer with the real part or the magnitudes of the
        // signal (the cartesian or polar form, depends on the <m>polarin</m> attribute).
        // @out 1 @type symbol @digest Buffers containing input phases (in the <m>angleunit</m> coordinate) or imaginary (y) parts
        // @description If the <m>cpxin</m> attribute is set, then the second input expects a buffer with the imaginary part or the phases of the
        // signal (the cartesian or polar form, depends on the <m>polarin</m> attribute).
        if (x->complex_input) {
            switch (a) {
                case 0: sprintf(s, x->polar_input ? "symbol: Source Buffer Real (x) Parts" : "symbol: Source Buffer Real (x) Parts");    break;
                case 1: sprintf(s, x->polar_input ? "symbol: Source Buffer Magnitudes" : "symbol: Source Buffer Phases");    break;
            }
        } else {
            sprintf(s, "symbol: Source Buffer");
        }
    } else {
        // @out 0 @type symbol @digest Buffers containing output magnitudes or real (x) parts
        // @description Unless <m>downmix</m> is set, there are as many buffers as channels in the original audio file and each buffer contains
        // one bin per channel.
        // @out 1 @type symbol @digest Buffers containing output phases (in the <m>angleunit</m> coordinate) or imaginary (y) parts
        // @description Unless <m>downmix</m> is set, there are as many buffers as channels in the original audio file and each buffer contains
        // one bin per channel.
        switch (a) {
            case 0: sprintf(s, x->polar_output ? "symbol/list: Buffer(s) Containing STFT Magnitudes" : "symbol/list: Buffer(s) Containing STFT Real/x Parts");    break;
            case 1: sprintf(s, x->polar_output ? "symbol/list: Buffer(s) Containing STFT Phases" : "symbol/list: Buffer(s) Containing STFT Imaginary/y Parts");    break;
        }
    }
}

void buf_stft_inletinfo(t_buf_stft *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_stft *buf_stft_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_stft *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_stft*)object_alloc_debug(s_tag_class);
    if (x) {
        x->polar_input = 0;
        x->complex_input = 0;
        x->polar_input = 0;
        x->polar_output = 1;
        x->fullspectrum = 0;
        x->downmix = 0;
        x->unitary = 1;
        
        earsbufobj_init((t_earsbufobj *)x, EARSBUFOBJ_FLAG_NONE); // EARSBUFOBJ_FLAG_SUPPORTS_COPY_NAMES);

        // @arg 0 @name outname @optional 1 @type symbol
        // @digest Output buffer name
        // @description @copy EARS_DOC_OUTNAME_ATTR
        
        t_llll *args = llll_parse(true_ac, argv);
        t_llll *names = earsbufobj_extract_names_from_args((t_earsbufobj *)x, args);

        attr_args_process(x, argc, argv);

        earsbufobj_setup((t_earsbufobj *)x, x->complex_input ? "ee" : "e", "EE", names);

        object_attr_setdisabled((t_object *)x, gensym("cpxin"), 1);
        object_attr_setdisabled((t_object *)x, gensym("polarin"), x->complex_input == 0);

        llll_free(args);
        llll_free(names);
    }
    return x;
}


void buf_stft_free(t_buf_stft *x)
{
    earsbufobj_free((t_earsbufobj *)x);
}


#ifdef EARS_STFT_USE_ESSENTIA
t_ears_essentia_analysis_params buf_stft_get_params(t_buf_stft *x, t_buffer_obj *buf)
{
    t_ears_essentia_analysis_params params = earsbufobj_get_essentia_analysis_params((t_earsbufobj *)x, buf);
    return params;
}
#endif

void buf_stft_bang(t_buf_stft *x)
{
    long cpxin = x->complex_input;
    long num_buffers = earsbufobj_get_instore_size((t_earsbufobj *)x, 0);
    long num_buffers2 = cpxin ? earsbufobj_get_instore_size((t_earsbufobj *)x, 1) : 0;
    long downmix = x->downmix;

    if (num_buffers == 0 || (cpxin && num_buffers2 == 0)) {
        object_error((t_object *)x, "Not all bufferst have been input.");
        return;
    }

    t_buffer_obj *in = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, 0);
    t_buffer_obj *in2 = cpxin ? earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 1, 0) : NULL;
    long num_channels = ears_buffer_get_numchannels((t_object *)x, in);
    long num_channels2 = cpxin ? ears_buffer_get_numchannels((t_object *)x, in2) : 0;

    if (cpxin && num_channels2 != num_channels) {
        object_error((t_object *)x, "Mismatch in number of buffer channels.");
        return;
    }

    if (downmix) {
        earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_OUT, 0, 1, true);
        earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_OUT, 1, 1, true);
        earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
        
        t_buffer_obj *out1 = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, 0);
        t_buffer_obj *out2 = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 1, 0);
        
#ifdef EARS_STFT_USE_ESSENTIA
        std::vector<float> data = ears_buffer_get_sample_vector_mono((t_object *)x, in);
        
        t_ears_essentia_analysis_params params = buf_stft_get_params(x, in);
        
        ears_vector_stft_essentia((t_object *)x, data, ears_buffer_get_sr((t_object *)x, in), out1, out2, x->polar_output, x->fullspectrum, &params, (e_ears_angleunit)x->e_ob.l_angleunit);
#else
        ears_buffer_stft((t_object *)x, in, in2, -1 /* -1 means downmixing */,
                         out1, out2,
                         earsbufobj_time_to_samps((t_earsbufobj *)x, x->e_ob.a_framesize, in, EARSBUFOBJ_CONVERSION_FLAG_ISANALYSIS),
                         earsbufobj_time_to_samps((t_earsbufobj *)x, x->e_ob.a_hopsize, in, EARSBUFOBJ_CONVERSION_FLAG_ISANALYSIS),
                         x->e_ob.a_wintype ? x->e_ob.a_wintype->s_name : "rect",
                         x->polar_input, x->polar_output, x->fullspectrum, (e_ears_angleunit)x->e_ob.l_angleunit, x->e_ob.a_winstartfromzero, x->unitary);
#endif
        
    } else if (num_channels > 0){
        earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_OUT, 0, num_channels, true);
        earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_OUT, 1, num_channels, true);
        earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
        
        for (long c = 0; c < num_channels; c++) {
            t_buffer_obj *out1 = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, c);
            t_buffer_obj *out2 = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 1, c);
            
#ifdef EARS_STFT_USE_ESSENTIA
            std::vector<float> data = ears_buffer_get_sample_vector_channel((t_object *)x, in, c);
            
            t_ears_essentia_analysis_params params = buf_stft_get_params(x, in);
            
            ears_vector_stft_essentia((t_object *)x, data, ears_buffer_get_sr((t_object *)x, in), out1, out2, x->polar_output, x->fullspectrum, &params, (e_ears_angleunit)x->e_ob.l_angleunit);
#else
            ears_buffer_stft((t_object *)x, in, in2, c, out1, out2,
                             earsbufobj_time_to_samps((t_earsbufobj *)x, x->e_ob.a_framesize, in, EARSBUFOBJ_CONVERSION_FLAG_ISANALYSIS),
                             earsbufobj_time_to_samps((t_earsbufobj *)x, x->e_ob.a_hopsize, in, EARSBUFOBJ_CONVERSION_FLAG_ISANALYSIS),
                             x->e_ob.a_wintype ? x->e_ob.a_wintype->s_name : "rect",
                             x->polar_input, x->polar_output, x->fullspectrum, (e_ears_angleunit)x->e_ob.l_angleunit, x->e_ob.a_winstartfromzero, x->unitary);
#endif
        }
    } else {
        object_error((t_object *)x, "No channels in buffer!");
    }
    
    earsbufobj_outlet_buffer((t_earsbufobj *)x, 1);
    earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
}



void buf_stft_anything(t_buf_stft *x, t_symbol *msg, long ac, t_atom *av)
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
                
                buf_stft_bang(x);
            } else {
                long num_bufs = llll_get_num_symbols_root(parsed);
                earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 1, num_bufs, true);
                earsbufobj_store_buffer_list((t_earsbufobj *)x, parsed, 1);
            }
        }
    }
    llll_free(parsed);
}


