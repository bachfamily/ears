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

typedef struct _buf_istft {
    t_earsbufobj       e_ob;

    long polar;
    long fullspectrum;
    
    double             sr;

    long               a_numGriffinLimIterations;

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

    earsbufobj_class_add_wintype_attr(c);
    earsbufobj_class_add_winstartfromzero_attr(c);

    CLASS_ATTR_LONG(c, "griffinlim", 0, t_buf_istft, a_numGriffinLimIterations);
    CLASS_ATTR_STYLE_LABEL(c,"griffinlim",0,"text","Number of Griffin-Lim Iterations");
    CLASS_ATTR_CATEGORY(c, "griffinlim", 0, "Synthesis");
    // @description Sets the number of Griffin-Lim iterations used in case no phases are provided.

    
    CLASS_ATTR_DOUBLE(c, "sr", 0, t_buf_istft, sr);
    CLASS_ATTR_STYLE_LABEL(c,"sr",0,"text","Sample Rate");
    CLASS_ATTR_BASIC(c, "sr", 0);
    CLASS_ATTR_CATEGORY(c, "sr", 0, "Synthesis");
    // @description Sets the sample rate. Leave 0 to infer it from the input STFT buffers.
    
    CLASS_ATTR_LONG(c, "polar",    0,    t_buf_istft, polar);
    CLASS_ATTR_STYLE_LABEL(c, "polar", 0, "onoff", "Polar Output");
    CLASS_ATTR_BASIC(c, "polar", 0);
    // @description Output data in polar coordinates, instead of cartesian ones.
    // Default is 1.

    CLASS_ATTR_LONG(c, "fullspectrum",    0,    t_buf_istft, fullspectrum);
    CLASS_ATTR_STYLE_LABEL(c, "fullspectrum", 0, "onoff", "Full Spectrum");
    CLASS_ATTR_BASIC(c, "fullspectrum", 0);
    // @description Output full spectrum; if not set, it will output the first half of the spectrum only.

    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
    return 0;
}

void buf_istft_assist(t_buf_istft *x, void *b, long m, long a, char *s)
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
        x->polar = 1;
        x->fullspectrum = 0;
        x->sr = 0;
        
        earsbufobj_init((t_earsbufobj *)x, EARSBUFOBJ_FLAG_NONE); // EARSBUFOBJ_FLAG_SUPPORTS_COPY_NAMES);

        // @arg 0 @name outname @optional 1 @type symbol
        // @digest Output buffer name
        // @description @copy EARS_DOC_OUTNAME_ATTR
        
        t_llll *args = llll_parse(true_ac, argv);
        t_llll *names = earsbufobj_extract_names_from_args((t_earsbufobj *)x, args);

        x->e_ob.a_wintype = gensym("square");

        attr_args_process(x, argc, argv);

        earsbufobj_setup((t_earsbufobj *)x, "EE", "e", names);

        llll_free(args);
        llll_free(names);
    }
    return x;
}


void buf_istft_free(t_buf_istft *x)
{
    earsbufobj_free((t_earsbufobj *)x);
}


t_ears_essentia_analysis_params buf_istft_get_params(t_buf_istft *x, t_buffer_obj *buf)
{
    t_ears_essentia_analysis_params params = earsbufobj_get_essentia_analysis_params((t_earsbufobj *)x, buf);

    params.numGriffinLimIterations = x->a_numGriffinLimIterations;
    return params;
}

void buf_istft_bang(t_buf_istft *x)
{
    long num_buffers = earsbufobj_get_instore_size((t_earsbufobj *)x, 0);
    
    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
    
    if (num_buffers > 0) {
        t_buffer_obj *in1[num_buffers];
        t_buffer_obj *in2[num_buffers];
        t_buffer_obj *dest = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, 0);

        for (long i = 0; i < num_buffers; i++) {
            in1[i] = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, i);
            in2[i] = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 1, i);
        }
        
        t_ears_essentia_analysis_params params = buf_istft_get_params(x, in1[0]);
        
        ears_specbuffer_istft_essentia((t_object *)x, num_buffers, in1, in2, dest, x->polar, x->fullspectrum, &params, (e_ears_angleunit)x->e_ob.l_angleunit, x->sr);
    }
    
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


