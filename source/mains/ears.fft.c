/**
	@file
	ears.fft.c
 
	@name
	ears.fft~
 
	@realname
	ears.fft~
 
	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	Fast Fourier Transforms
 
	@description
	Apply the Fast Fourier Transform or its inverse to a buffer
 
	@discussion
 
	@category
	ears spectral
 
	@keywords
	buffer, fft, fourier, transform, spectrum
 
	@seealso
	ears.window~, ears.spectrogram~
	
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


typedef struct _buf_fft {
    t_earsbufobj       e_ob;

    long            inverse;
    long            polar;
    long            fullspectrum;

} t_buf_fft;



// Prototypes
t_buf_fft*     buf_fft_new(t_symbol *s, short argc, t_atom *argv);
void			buf_fft_free(t_buf_fft *x);
void			buf_fft_bang(t_buf_fft *x);
void			buf_fft_anything(t_buf_fft *x, t_symbol *msg, long ac, t_atom *av);

void buf_fft_assist(t_buf_fft *x, void *b, long m, long a, char *s);
void buf_fft_inletinfo(t_buf_fft *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(fft)

/**********************************************************************/
// Class Definition and Life Cycle

int C74_EXPORT main(void)
{
    common_symbols_init();
    llllobj_common_symbols_init();
    
    if (llllobj_check_version(bach_get_current_llll_version()) || llllobj_test()) {
        ears_error_bachcheck();
        return 1;
    }
    
    t_class *c;
    
    CLASS_NEW_CHECK_SIZE(c, "ears.fft~",
                         (method)buf_fft_new,
                         (method)buf_fft_free,
                         sizeof(t_buf_fft),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    
    // @method list/llll @digest Process buffers
    // @description A list or llll with buffer names will trigger the buffer processing and output the processed
    // buffer names (depending on the <m>naming</m> attribute).
    EARSBUFOBJ_DECLARE_COMMON_METHODS_HANDLETHREAD(fft)
    
    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_naming_attr(c);
    earsbufobj_class_add_angleunit_attr(c);

    
    CLASS_ATTR_LONG(c, "inv",    0,    t_buf_fft, inverse);
    CLASS_ATTR_STYLE_LABEL(c, "inv", 0, "onoff", "Perform Inverse FFT");
    CLASS_ATTR_BASIC(c, "inv", 0);
    // @description Perform the inverse FFT, instead of the direct one.
    
    CLASS_ATTR_LONG(c, "polar",    0,    t_buf_fft, polar);
    CLASS_ATTR_STYLE_LABEL(c, "polar", 0, "onoff", "Polar Input And Output");
    CLASS_ATTR_BASIC(c, "polar", 0);
    // @description Input and output data in polar coordinates, instead of cartesian ones.

    CLASS_ATTR_LONG(c, "fullspectrum",    0,    t_buf_fft, fullspectrum);
    CLASS_ATTR_STYLE_LABEL(c, "fullspectrum", 0, "onoff", "Full Spectrum");
    CLASS_ATTR_BASIC(c, "fullspectrum", 0);
    // @description Output full spectrum; if not set, it will output the first half of the spectrum only.

    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
    return 0;
}

void buf_fft_assist(t_buf_fft *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        if (a == 0)
            sprintf(s, x->polar ? "symbol: Buffer Containing Magnitudes" : "symbol: Buffer Containing Real/x Parts");
        // @in 0 @type symbol @digest Buffer containing input magnitudes or real (x) parts
        // @description Depending on the object "polar" attribute, the first inlet expects a buffer containing either the incoming
        //                magnitudes, or the incoming real parts of the samples
        else
            sprintf(s, x->polar ? "symbol: Buffer Containing Phases" : "symbol: Buffer Containing Imaginary/y Parts");
        // @in 1 @type symbol @digest Buffer containing input phases or imaginary (y) parts
        // @description Depending on the object "polar" attribute, the second inlet expects a buffer containing either the incoming
        //                phases (in the <m>angleunit</m> coordinate), or the incoming imaginary parts of the samples
    } else {
        switch (a) {
            case 0: sprintf(s, x->polar ? "symbol: Buffer Containing FFT Magnitudes" : "symbol: Buffer Containing FFT Real/x Parts");    break; // @out 0 @type symbol @digest Buffer containing output magnitudes or real (x) parts
            case 1: sprintf(s, x->polar ? "symbol: Buffer Containing FFT Phases" : "symbol: Buffer Containing FFT Imaginary/y Parts");    break; // @out 1 @type symbol @digest Buffer containing output phases (in the <m>angleunit</m> coordinate) or imaginary (y) parts
        }
    }
}

void buf_fft_inletinfo(t_buf_fft *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_fft *buf_fft_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_fft *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_fft*)object_alloc_debug(s_tag_class);
    if (x) {
        x->inverse = 0;
        x->polar = 0;
        x->fullspectrum = 1;
        
        earsbufobj_init((t_earsbufobj *)x,  EARSBUFOBJ_FLAG_SUPPORTS_COPY_NAMES);

        // @arg 0 @name outname @optional 1 @type symbol
        // @digest Output buffer name
        // @description @copy EARS_DOC_OUTNAME_ATTR
        
        t_llll *args = llll_parse(true_ac, argv);
        t_llll *names = earsbufobj_extract_names_from_args((t_earsbufobj *)x, args);

        attr_args_process(x, argc, argv);

        earsbufobj_setup((t_earsbufobj *)x, "EE", "EE", names);

        llll_free(args);
        llll_free(names);
    }
    return x;
}


void buf_fft_free(t_buf_fft *x)
{
    earsbufobj_free((t_earsbufobj *)x);
}



void buf_fft_bang(t_buf_fft *x)
{
    long num_buffers = earsbufobj_get_instore_size((t_earsbufobj *)x, 0);
    
    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_buffers, true);
    
    for (long count = 0; count < num_buffers; count++) {
        t_buffer_obj *in1 = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, count);
        t_buffer_obj *in2 = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 1, count);
        t_buffer_obj *out1 = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, count);
        t_buffer_obj *out2 = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 1, count);

        ears_buffer_fft((t_object *)x, in1, in2, out1, out2, x->polar, x->inverse, x->fullspectrum, (e_ears_angleunit)x->e_ob.l_angleunit);
    }
    
    earsbufobj_outlet_buffer((t_earsbufobj *)x, 1);
    earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
}



void buf_fft_anything(t_buf_fft *x, t_symbol *msg, long ac, t_atom *av)
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
                
                buf_fft_bang(x);
            } else {
                long num_bufs = llll_get_num_symbols_root(parsed);
                earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 1, num_bufs, true);
                earsbufobj_store_buffer_list((t_earsbufobj *)x, parsed, 1);
            }
        }
    }
    llll_free(parsed);
}


