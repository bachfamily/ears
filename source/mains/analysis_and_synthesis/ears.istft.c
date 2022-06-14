/**
	@file
	ears.istft.c
 
	@name
	ears.istft~
 
	@realname
	ears.istft~
 
	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	Inverse Short-Time Fourier Transform
 
	@description
	Computes the inverse of a Short-Time Fourier Transform via overlap-add
 
	@discussion
 
	@category
	ears spectral
 
	@keywords
	buffer, istft, fourier, transform, spectrum, inverse
 
	@seealso
	ears.stft~, ears.griffinlim~, ears.window~
	
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


typedef struct _buf_istft {
    t_earsbufobj       e_ob;

    long            complex_output;
    long            polar_input;
    long            polar_output;
    long            fullspectrum;
    long            unitary;

    double             sr;

} t_buf_istft;



// Prototypes
t_buf_istft*     buf_istft_new(t_symbol *s, short argc, t_atom *argv);
void			buf_istft_free(t_buf_istft *x);
void			buf_istft_bang(t_buf_istft *x);
void			buf_istft_anything(t_buf_istft *x, t_symbol *msg, long ac, t_atom *av);

void buf_istft_assist(t_buf_istft *x, void *b, long m, long a, char *s);
void buf_istft_inletinfo(t_buf_istft *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(istft)

/**********************************************************************/
// Class Definition and Life Cycle


t_max_err buf_istft_setattr_cpxout(t_buf_istft *x, void *attr, long argc, t_atom *argv)
{
    if (argc && argv) {
        if (!x->e_ob.l_is_creating)
            object_error((t_object *)x, "The 'cpxout' attribute can only be set in the object box.");
        else if (atom_gettype(argv) == A_LONG)
            x->complex_output = atom_getlong(argv);
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
        return 1;
    }
    
    t_class *c;
    
    CLASS_NEW_CHECK_SIZE(c, "ears.istft~",
                         (method)buf_istft_new,
                         (method)buf_istft_free,
                         sizeof(t_buf_istft),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    
    // @method list/llll @digest Process buffers
    // @description A list or llll with buffer names will trigger the buffer processing and output the processed
    // buffer names (depending on the <m>naming</m> attribute).
    EARSBUFOBJ_DECLARE_COMMON_METHODS_HANDLETHREAD(istft)
    
    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_blocking_attr(c);
    earsbufobj_class_add_naming_attr(c);
    earsbufobj_class_add_timeunit_attr(c);
    earsbufobj_class_add_angleunit_attr(c);

#ifdef EARS_STFT_USE_ESSENTIA
    earsbufobj_class_add_wintype_attr_essentia(c);
#else
    earsbufobj_class_add_wintype_attr(c);
#endif
    earsbufobj_class_add_winstartfromzero_attr(c);
    
    CLASS_ATTR_DOUBLE(c, "sr", 0, t_buf_istft, sr);
    CLASS_ATTR_STYLE_LABEL(c,"sr",0,"text","Sample Rate");
    CLASS_ATTR_BASIC(c, "sr", 0);
    CLASS_ATTR_CATEGORY(c, "sr", 0, "Synthesis");
    // @description Sets the sample rate. Leave 0 to infer it from the input STFT buffers.
    
    CLASS_ATTR_LONG(c, "cpxout",    0,    t_buf_istft, complex_output);
    CLASS_ATTR_STYLE_LABEL(c, "cpxout", 0, "onoff", "Complex Output");
    CLASS_ATTR_ACCESSORS(c, "cpxout", NULL, buf_istft_setattr_cpxout);
    // @description Output complex data instead of real-valued audio samples.
    // This attribute is static and can only be entered in the object box.

    CLASS_ATTR_LONG(c, "polarout",    0,    t_buf_istft, polar_output);
    CLASS_ATTR_STYLE_LABEL(c, "polarout", 0, "onoff", "Polar Output");
    // @description Output data in polar coordinates, instead of cartesian ones.
    // Default is 0.

    
    CLASS_ATTR_LONG(c, "polarin",    0,    t_buf_istft, polar_input);
    CLASS_ATTR_STYLE_LABEL(c, "polarin", 0, "onoff", "Polar Input");
    CLASS_ATTR_BASIC(c, "polarin", 0);
    // @description Input data in polar coordinates, instead of cartesian ones.
    // Default is 1.

    CLASS_ATTR_LONG(c, "fullspectrum",    0,    t_buf_istft, fullspectrum);
    CLASS_ATTR_STYLE_LABEL(c, "fullspectrum", 0, "onoff", "Full Spectrum");
    CLASS_ATTR_BASIC(c, "fullspectrum", 0);
    // @description Output full spectrum; if not set, it will output the first half of the spectrum only.

    CLASS_ATTR_LONG(c, "unitary",    0,    t_buf_istft, unitary);
    CLASS_ATTR_STYLE_LABEL(c,"unitary",0,"onoff","Unitary");
    CLASS_ATTR_BASIC(c, "unitary", 0);
    // @description Toggles the unitary normalization of the Fourier Transform (so that the
    // if coincides with its inverse up to conjugation).

    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
    return 0;
}

void buf_istft_assist(t_buf_istft *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        // @in 0 @type symbol @digest Buffer containing input magnitudes or real (x) parts, one bin per channel
        // @in 1 @type symbol @digest Buffer containing input phases (in the <m>angleunit</m> coordinate) or imaginary (y) parts, one bin per channel
        switch (a) {
            case 0: sprintf(s, x->polar_input ? "symbol: Buffer Containing STFT Magnitudes" : "symbol: Buffer Containing STFT Real/x Parts");    break;
            case 1: sprintf(s, x->polar_input ? "symbol: Buffer Containing STFT Phases" : "symbol: Buffer Containing STFT Imaginary/y Parts");    break;
        }
    } else {
        // @out 0 @type symbol @digest Output audio buffer - or output magnitudes, or real (x) parts
        // @description If the <m>cpxout</m> attribute is not set, the object outputs a buffer with the reconstructed audio content.
        // If the <m>cpxout</m> attribute is set, then the output is the first component of the complex signal (in cartesian
        // or polar form, depending on the <m>polarin</m> attribute).
        // @out 1 @type symbol @digest Output phases or imaginary (y) parts
        // @description If the <m>cpxout</m> attribute is set, then the second inlet outputs the second component of the
        // complex signal (in cartesian
        // or polar form, depending on the <m>polarin</m> attribute). Phases are output according to the <m>angleunit</m> coordinate.
        if (x->complex_output) {
            switch (a) {
                case 0: sprintf(s, x->polar_output ? "symbol: Output Audio Buffer Magnitudes" : "symbol: Output Audio Buffer Real/x Parts");    break;
                case 1: sprintf(s, x->polar_output ? "symbol: Output Audio Buffer Phases" : "symbol: Output Audio Buffer Imaginary/x Parts");    break;
            }
        } else {
            sprintf(s, "symbol: Output Audio Buffer");
        }

        sprintf(s, "symbol: Buffer Containing Reconstructed Audio Signal");
    }
}

void buf_istft_inletinfo(t_buf_istft *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_istft *buf_istft_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_istft *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_istft*)object_alloc_debug(s_tag_class);
    if (x) {
        x->polar_input = 1;
        x->complex_output = 0;
        x->polar_output = 0;
        x->fullspectrum = 0;
        x->sr = 0;
        x->unitary = 1;
        
        earsbufobj_init((t_earsbufobj *)x, EARSBUFOBJ_FLAG_NONE); // EARSBUFOBJ_FLAG_SUPPORTS_COPY_NAMES);

        // @arg 0 @name outname @optional 1 @type symbol
        // @digest Output buffer name
        // @description @copy EARS_DOC_OUTNAME_ATTR
        
        t_llll *args = llll_parse(true_ac, argv);
        t_llll *names = earsbufobj_extract_names_from_args((t_earsbufobj *)x, args);

        x->e_ob.a_wintype = gensym("square");

        attr_args_process(x, argc, argv);

        earsbufobj_setup((t_earsbufobj *)x, "EE", x->complex_output ? "ee" : "e", names);

        object_attr_setdisabled((t_object *)x, gensym("cpxout"), 1);

        llll_free(args);
        llll_free(names);
    }
    return x;
}


void buf_istft_free(t_buf_istft *x)
{
    earsbufobj_free((t_earsbufobj *)x);
}

#ifdef EARS_STFT_USE_ESSENTIA

t_ears_essentia_analysis_params buf_istft_get_params(t_buf_istft *x, t_buffer_obj *buf)
{
    t_ears_essentia_analysis_params params = earsbufobj_get_essentia_analysis_params((t_earsbufobj *)x, buf);
    return params;
}
#endif

void buf_istft_bang(t_buf_istft *x)
{
    long num_buffers = earsbufobj_get_instore_size((t_earsbufobj *)x, 0);
    long cpx = x->complex_output;
    
    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
    
    if (num_buffers > 0) {
        t_buffer_obj *in1[num_buffers];
        t_buffer_obj *in2[num_buffers];
        t_buffer_obj *dest = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, 0);
        t_buffer_obj *dest2 = cpx ? earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 1, 0) : NULL;

        for (long i = 0; i < num_buffers; i++) {
            in1[i] = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, i);
            in2[i] = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 1, i);
        }
        
#ifdef EARS_STFT_USE_ESSENTIA

        t_ears_essentia_analysis_params params = buf_istft_get_params(x, in1[0]);
        
        ears_specbuffer_istft_essentia((t_object *)x, num_buffers, in1, in2, dest, x->polar, x->fullspectrum, &params, (e_ears_angleunit)x->e_ob.l_angleunit, x->sr);
#else
        
        ears_buffer_istft((t_object *)x, num_buffers, in1, in2, dest, dest2, x->e_ob.a_wintype ? x->e_ob.a_wintype->s_name : "rect", x->polar_input, x->polar_output, x->fullspectrum, (e_ears_angleunit)x->e_ob.l_angleunit, x->sr, x->e_ob.a_winstartfromzero, x->unitary);
        
#endif
        
    }
    
    if (cpx)
        earsbufobj_outlet_buffer((t_earsbufobj *)x, 1);
    earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
}



void buf_istft_anything(t_buf_istft *x, t_symbol *msg, long ac, t_atom *av)
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
                earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_OUT, 0, 1, true);
                earsbufobj_store_buffer_list((t_earsbufobj *)x, parsed, 0);
                
                buf_istft_bang(x);
            } else {
                long num_bufs = llll_get_num_symbols_root(parsed);
                earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 1, num_bufs, true);
                earsbufobj_store_buffer_list((t_earsbufobj *)x, parsed, 1);
            }
        }
    }
    llll_free(parsed);
}


