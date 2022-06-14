/**
	@file
	ears.griffinlim.c
 
	@name
	ears.griffinlim~
 
	@realname
	ears.griffinlim~
 
	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	Reconstruct signal from magnitude spectrum
 
	@description
    Computes approximate phases and from a magnitude spectrum
    using the Griffin-Lim iterative algorithm and outputs the
    reconstructed signal

	@discussion
 
	@category
	ears spectral
 
	@keywords
	buffer, griffinlim, spectrum, inverse, reconstruct, griffin, lim, phases, phase retrieval
 
	@seealso
	ears.window~, ears.istft~, ears.istft~
	
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


typedef struct _buf_griffinlim {
    t_earsbufobj       e_ob;

    long            num_iterations;
    long            fullspectrum;
    long            unitary;

} t_buf_griffinlim;



// Prototypes
t_buf_griffinlim*     buf_griffinlim_new(t_symbol *s, short argc, t_atom *argv);
void			buf_griffinlim_free(t_buf_griffinlim *x);
void			buf_griffinlim_bang(t_buf_griffinlim *x);
void			buf_griffinlim_anything(t_buf_griffinlim *x, t_symbol *msg, long ac, t_atom *av);

void buf_griffinlim_assist(t_buf_griffinlim *x, void *b, long m, long a, char *s);
void buf_griffinlim_inletinfo(t_buf_griffinlim *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(griffinlim)

/**********************************************************************/
// Class Definition and Life Cycle

void C74_EXPORT ext_main(void* moduleRef)
{
    
    common_symbols_init();
    llllobj_common_symbols_init();
    
    if (llllobj_check_version(bach_get_current_llll_version()) || llllobj_test()) {
        ears_error_bachcheck();
        return 1;
    }
    
    t_class *c;
    
    CLASS_NEW_CHECK_SIZE(c, "ears.griffinlim~",
                         (method)buf_griffinlim_new,
                         (method)buf_griffinlim_free,
                         sizeof(t_buf_griffinlim),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    
    // @method list/llll @digest Process buffers
    // @description A list or llll with buffer names will trigger the buffer processing and output the processed
    // buffer names (depending on the <m>naming</m> attribute).
    EARSBUFOBJ_DECLARE_COMMON_METHODS_HANDLETHREAD(griffinlim)
    
    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_blocking_attr(c);
    earsbufobj_class_add_naming_attr(c);
    earsbufobj_class_add_timeunit_attr(c);
    earsbufobj_class_add_angleunit_attr(c);

    earsbufobj_class_add_wintype_attr_ansyn(c);
    earsbufobj_class_add_winstartfromzero_attr(c);

    CLASS_ATTR_LONG(c, "numiter", 0, t_buf_griffinlim, num_iterations);
    CLASS_ATTR_STYLE_LABEL(c,"numiter",0,"text","Number of Iterations");
    CLASS_ATTR_BASIC(c, "numiter", 0);
    // @description Number of iterations.

    
    CLASS_ATTR_LONG(c, "fullspectrum",    0,    t_buf_griffinlim, fullspectrum);
    CLASS_ATTR_STYLE_LABEL(c, "fullspectrum", 0, "onoff", "Full Spectrum");
    CLASS_ATTR_BASIC(c, "fullspectrum", 0);
    // @description Output full spectrum; if not set, it will output the first half of the spectrum only.

    CLASS_ATTR_LONG(c, "unitary",    0,    t_buf_griffinlim, unitary);
    CLASS_ATTR_STYLE_LABEL(c,"unitary",0,"onoff","Unitary");
    CLASS_ATTR_BASIC(c, "unitary", 0);
    // @description Toggles the unitary normalization of the Fourier Transform (so that the
    // if coincides with its inverse up to conjugation).

    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
    return 0;
}

void buf_griffinlim_assist(t_buf_griffinlim *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
            // @in 0 @type symbol @digest Buffer containing magnitude bins
            // @description Buffer containing magnitude bins
        sprintf(s, "symbol: Buffer with Magnitude Bins" );
    } else {
        // @out 0 @type symbol @digest Buffer containing reconstructed audio
        // @out 1 @type symbol @digest Buffer containing retrieved phases (in the <m>angleunit</m> coordinate)
        switch (a) {
            case 0: sprintf(s, "symbol: Buffer Containing Reconstructed Audio");    break;
            case 1: sprintf(s, "symbol: Buffer Containing Retrieved Phases");    break;
        }
    }
}

void buf_griffinlim_inletinfo(t_buf_griffinlim *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_griffinlim *buf_griffinlim_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_griffinlim *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_griffinlim*)object_alloc_debug(s_tag_class);
    if (x) {
        x->fullspectrum = 0;
        x->unitary = 1;
        x->num_iterations = 10;
        
        earsbufobj_init((t_earsbufobj *)x, EARSBUFOBJ_FLAG_NONE); // EARSBUFOBJ_FLAG_SUPPORTS_COPY_NAMES);

        // @arg 0 @name outname @optional 1 @type symbol
        // @digest Output buffer name
        // @description @copy EARS_DOC_OUTNAME_ATTR
        
        t_llll *args = llll_parse(true_ac, argv);
        t_llll *names = earsbufobj_extract_names_from_args((t_earsbufobj *)x, args);

        x->e_ob.a_wintype_ansyn[0] = gensym("sqrthann");
        x->e_ob.a_wintype_ansyn[1] = gensym("sqrthann");

        attr_args_process(x, argc, argv);

        earsbufobj_setup((t_earsbufobj *)x, "E", "eE", names);

        llll_free(args);
        llll_free(names);
    }
    return x;
}


void buf_griffinlim_free(t_buf_griffinlim *x)
{
    earsbufobj_free((t_earsbufobj *)x);
}

void buf_griffinlim_bang(t_buf_griffinlim *x)
{
    long num_buffers = earsbufobj_get_instore_size((t_earsbufobj *)x, 0);
    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_OUT, 1, num_buffers, true);
    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
    
    if (num_buffers > 0) {
        t_buffer_obj *in_amp[num_buffers];
        t_buffer_obj *out_ph[num_buffers];
        t_buffer_obj *dest = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, 0);

        for (long i = 0; i < num_buffers; i++) {
            in_amp[i] = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, i);
            out_ph[i] = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 1, i);
        }
        
        ears_buffer_griffinlim((t_object *)x, num_buffers, in_amp, out_ph, NULL, dest, 
                               x->e_ob.a_wintype_ansyn[0] ? x->e_ob.a_wintype_ansyn[0]->s_name : "rect",
                               x->e_ob.a_wintype_ansyn[1] ? x->e_ob.a_wintype_ansyn[1]->s_name : "rect",
                               x->fullspectrum, x->e_ob.a_winstartfromzero, x->unitary, x->num_iterations, true);
        
    }
    
    earsbufobj_outlet_buffer((t_earsbufobj *)x, 1);
    earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
}



void buf_griffinlim_anything(t_buf_griffinlim *x, t_symbol *msg, long ac, t_atom *av)
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
                
                buf_griffinlim_bang(x);
            } else {
                long num_bufs = llll_get_num_symbols_root(parsed);
                earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 1, num_bufs, true);
                earsbufobj_store_buffer_list((t_earsbufobj *)x, parsed, 1);
            }
        }
    }
    llll_free(parsed);
}


