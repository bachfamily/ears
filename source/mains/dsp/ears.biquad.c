/**
	@file
	ears.biquad.c
 
	@name
	ears.biquad~
 
	@realname
	ears.biquad~
 
	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	Two-pole two-zero filter for buffers.
 
	@description
    Implements a two-pole two-zero filter using the following equation:
    y[n] = a0 * x[n] + a1 * x[n-1] + a2 * x[n-2] - b1 * y[n-1] - b2 * y[n-2]
 
	@discussion
 
	@category
	ears filter
 
	@keywords
	buffer, biquad, filter, lowpass, hipass, highpass, bandpass, bandreject, hishelf, lowshelf, resonant, attenuation
 
	@seealso
	biquad~, ears.onepole~
	
	@owner
	Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "foundation/llllobj.h"
#include "foundation/llll_commons_ext.h"
#include "math/bach_math_utilities.h"
#include "ears.object.h"



typedef struct _buf_biquad {
    t_earsbufobj       e_ob;
    
    t_llll             *coefficients;
} t_buf_biquad;



// Prototypes
t_buf_biquad*         buf_biquad_new(t_symbol *s, short argc, t_atom *argv);
void			buf_biquad_free(t_buf_biquad *x);
void			buf_biquad_bang(t_buf_biquad *x);
void			buf_biquad_anything(t_buf_biquad *x, t_symbol *msg, long ac, t_atom *av);

void buf_biquad_assist(t_buf_biquad *x, void *b, long m, long a, char *s);
void buf_biquad_inletinfo(t_buf_biquad *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(biquad)

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
    
    CLASS_NEW_CHECK_SIZE(c, "ears.biquad~",
                         (method)buf_biquad_new,
                         (method)buf_biquad_free,
                         sizeof(t_buf_biquad),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    // @method list/llll @digest Process buffers
    // @description A list or llll with buffer names will trigger the buffer processing and output the processed
    // buffer names (depending on the <m>alloc</m> attribute).
    EARSBUFOBJ_DECLARE_COMMON_METHODS_HANDLETHREAD(biquad)

    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_blocking_attr(c);
    earsbufobj_class_add_alloc_attr(c);
    earsbufobj_class_add_polyout_attr(c);

    
    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
}

void buf_biquad_assist(t_buf_biquad *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        if (a == 0)
            sprintf(s, "symbol/list/llll: Incoming Buffer Names"); // @in 0 @type symbol/list/llll @digest Incoming buffer names
        else
            sprintf(s, "list/llll: Coefficients"); // @in 1 @type float/llll @digest List of five biquad coefficients
    } else {
        sprintf(s, "symbol/list: Output Buffer Names"); // @out 0 @type symbol/list @digest Output buffer names
    }
}

void buf_biquad_inletinfo(t_buf_biquad *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_biquad *buf_biquad_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_biquad *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_biquad*)object_alloc_debug(s_tag_class);
    if (x) {
//        x->coefficients = llll_from_text_buf("0.999999 -1.999997 0.999999 -1.999997 0.999997", false);
        x->coefficients = llll_from_text_buf("1. -2. 1. -2. 1.", false);

        // @arg 0 @name outnames @optional 1 @type symbol
        // @digest Output buffer names
        // @description @copy EARS_DOC_OUTNAME_ATTR

        // @arg 1 @name coefficients @optional 1 @type list/llll
        // @digest Biquad coefficients
        // @description Sets the biquad coefficients

        earsbufobj_init((t_earsbufobj *)x, EARSBUFOBJ_FLAG_SUPPORTS_COPY_NAMES);

        t_llll *args = llll_parse(true_ac, argv);
        t_llll *names = earsbufobj_extract_names_from_args((t_earsbufobj *)x, args);
        
        if (args && args->l_head) {
            llll_free(x->coefficients);
            x->coefficients = llll_clone(args);
        }
        
        attr_args_process(x, argc, argv);

        earsbufobj_setup((t_earsbufobj *)x, "E4", "E", names);

        llll_free(args);
        llll_free(names);
        
    }
    return x;
}


void buf_biquad_free(t_buf_biquad *x)
{
    llll_free(x->coefficients);
    earsbufobj_free((t_earsbufobj *)x);
}

void llll_to_coefficients(t_llll *ll, double *a0, double *a1, double *a2, double *b1, double *b2)
{
    if (ll) {
        t_llllelem *el = ll->l_head;
        if (el) {
            *a0 = hatom_getdouble(&el->l_hatom);
            el = el->l_next;
        }
        if (el) {
            *a1 = hatom_getdouble(&el->l_hatom);
            el = el->l_next;
        }
        if (el) {
            *a2 = hatom_getdouble(&el->l_hatom);
            el = el->l_next;
        }
        if (el) {
            *b1 = hatom_getdouble(&el->l_hatom);
            el = el->l_next;
        }
        if (el) {
            *b2 = hatom_getdouble(&el->l_hatom);
        }
    }
}

void buf_biquad_bang(t_buf_biquad *x)
{
    long num_buffers = earsbufobj_get_instore_size((t_earsbufobj *)x, 0);
    double a0 = 1, a1 = -2, a2 = 1, b1 = -2, b2 = 1;
    
    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_buffers, true);
    
    earsbufobj_mutex_lock((t_earsbufobj *)x);
    earsbufobj_init_progress((t_earsbufobj *)x, num_buffers);
    
    t_llllelem *el = x->coefficients->l_depth >= 2 ? x->coefficients->l_head : NULL;
    for (long count = 0; count < num_buffers; count++, el = el && el->l_next ? el->l_next : el) {
        t_buffer_obj *in = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, count);
        t_buffer_obj *out = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, count);

        if (el || x->coefficients) {
            t_llll *coefficients = el ? hatom_getllll(&el->l_hatom) : x->coefficients;
            llll_to_coefficients(coefficients, &a0, &a1, &a2, &b1, &b2);
            ears_buffer_biquad((t_object *)x, in, out, a0, a1, a2, b1, b2);
        }
        
        if (earsbufobj_iter_progress((t_earsbufobj *)x, count, num_buffers)) break;
    }
    earsbufobj_mutex_unlock((t_earsbufobj *)x);
    
    earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
}


void buf_biquad_anything(t_buf_biquad *x, t_symbol *msg, long ac, t_atom *av)
{
    long inlet = earsbufobj_proxy_getinlet((t_earsbufobj *) x);

    t_llll *parsed = earsbufobj_parse_gimme((t_earsbufobj *) x, LLLL_OBJ_VANILLA, msg, ac, av);
    if (!parsed) return;
    
    if (parsed && parsed->l_head) {
        if (inlet == 0) {
            long num_bufs = llll_get_num_symbols_root(parsed);

            earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_bufs, true);
            earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_OUT, 0, num_bufs, true);
            
            earsbufobj_store_buffer_list((t_earsbufobj *)x, parsed, 0);
            
            buf_biquad_bang(x);
            
        } else if (inlet == 1) {
            earsbufobj_mutex_lock((t_earsbufobj *)x);
            llll_free(x->coefficients);
            x->coefficients = llll_clone(parsed);
            earsbufobj_mutex_unlock((t_earsbufobj *)x);
        }
    }
    llll_free(parsed);
}


