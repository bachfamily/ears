/**
	@file
	ears.resample.c
 
	@name
	ears.resample~
 
	@realname
	ears.resample~
 
    @hiddenalias
    ears.resample

	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	Resample buffer
 
	@description
	Resample the buffer content, without changing its sample rate
 
	@discussion
    If you need to modify the buffer sample rate, use <o>ears.format</o>
 
	@category
	ears buffer operations
 
	@keywords
	buffer, resample, property, change, vinyl
 
	@seealso
	ears.format~, ears.reg~
	
	@owner
	Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "llllobj.h"
#include "llll_commons_ext.h"
#include "bach_math_utilities.h"
#include "ears.object.h"



typedef struct _buf_resample {
    t_earsbufobj        e_ob;
    
    long                window_width;
    
    t_llll              *resamplefactor;
} t_buf_resample;




// Prototypes
t_buf_resample*         buf_resample_new(t_symbol *s, short argc, t_atom *argv);
void			buf_resample_free(t_buf_resample *x);
void			buf_resample_bang(t_buf_resample *x);
void			buf_resample_anything(t_buf_resample *x, t_symbol *msg, long ac, t_atom *av);

void buf_resample_assist(t_buf_resample *x, void *b, long m, long a, char *s);
void buf_resample_inletinfo(t_buf_resample *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(resample)

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
    
    CLASS_NEW_CHECK_SIZE(c, "ears.resample~",
                         (method)buf_resample_new,
                         (method)buf_resample_free,
                         sizeof(t_buf_resample),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    // @method list/llll @digest Process buffers
    // @description A list or llll with buffer names will trigger the buffer processing and output the processed
    // buffer names (depending on the <m>naming</m> attribute).
    EARSBUFOBJ_DECLARE_COMMON_METHODS_DEFER(resample)
    
    earsbufobj_class_add_outname_attr(c);
    //    earsbufobj_class_add_timeunit_attr(c);
    earsbufobj_class_add_naming_attr(c);
    
    CLASS_ATTR_LONG(c, "winsize", 0, t_buf_resample, window_width);
    CLASS_ATTR_STYLE_LABEL(c,"winsize",0,"text","Window Width");
    CLASS_ATTR_BASIC(c, "winsize", 0);
    // @description Sets the resampling window width

    
    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
    return 0;
}

void buf_resample_assist(t_buf_resample *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        if (a == 0)
            sprintf(s, "symbol/list/llll: Incoming Buffer Names"); // @in 0 @type symbol/list/llll @digest Incoming buffer names
        else
            sprintf(s, "float/list/llll: Resampling factors"); // @in 0 @type float/list/llll @digest Resampling factors
    } else {
        sprintf(s, "symbol/list: Resampleted Buffer Names"); // @out 0 @type symbol/list @digest Resampled buffer names
    }
}

void buf_resample_inletinfo(t_buf_resample *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_resample *buf_resample_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_resample *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_resample*)object_alloc_debug(s_tag_class);
    if (x) {
        x->resamplefactor = llll_get();
        x->window_width = 11; // was: 7
        llll_appenddouble(x->resamplefactor, 1.);
        
        // @arg 0 @name outnames @optional 1 @type symbol
        // @digest Output buffer names
        // @description @copy EARS_DOC_OUTNAME_ATTR
        
        t_llll *args = llll_parse(true_ac, argv);
        t_llll *names = earsbufobj_extract_names_from_args((t_earsbufobj *)x, args);
        
        earsbufobj_init((t_earsbufobj *)x, EARSBUFOBJ_FLAG_DONT_DUPLICATE_INPUT_BUFFERS | EARSBUFOBJ_FLAG_SUPPORTS_COPY_NAMES);
        
        // @arg 1 @name resamplingfactor @optional 1 @type number
        // @digest Resampling factor
        // @description Sets the resampling factor
        
        if (args && args->l_head) {
            llll_clear(x->resamplefactor);
            llll_appenddouble(x->resamplefactor, hatom_getdouble(&args->l_head->l_hatom));
        }
        
        attr_args_process(x, argc, argv);
        
        earsbufobj_setup((t_earsbufobj *)x, "E4", "E", names);

        llll_free(args);
        llll_free(names);
        
    }
    return x;
}


void buf_resample_free(t_buf_resample *x)
{
    llll_free(x->resamplefactor);
    earsbufobj_free((t_earsbufobj *)x);
}



void buf_resample_bang(t_buf_resample *x)
{
    long num_buffers = ((t_earsbufobj *)x)->l_instore[0].num_stored_bufs;
    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_buffers, true);
    
    earsbufobj_mutex_lock((t_earsbufobj *)x);
    t_llllelem *el = x->resamplefactor->l_head;
    for (long count = 0; count < num_buffers; count++, el = el && el->l_next ? el->l_next : el) {
        t_buffer_obj *in = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, count);
        t_buffer_obj *out = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, count);
        
        double factor = el ? hatom_getdouble(&el->l_hatom) : 1.;
        if (in != out)
            ears_buffer_clone((t_object *)x, in, out);

        if (factor < 0) {
            ears_buffer_rev_inplace((t_object *)x, out);
            factor *= -1;
        }
        
        if (factor == 0) {
            object_error((t_object *)x, "Resampling factor cannot be zero.");
        } else if (factor != 1) {
            ears_buffer_resample((t_object *)x, out, factor, x->window_width);
        }
    }
    earsbufobj_mutex_unlock((t_earsbufobj *)x);
    
    earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
}


void buf_resample_anything(t_buf_resample *x, t_symbol *msg, long ac, t_atom *av)
{
    long inlet = proxy_getinlet((t_object *) x);
    
    t_llll *parsed = earsbufobj_parse_gimme((t_earsbufobj *) x, LLLL_OBJ_VANILLA, msg, ac, av);
    if (!parsed) return;
    
    if (parsed && parsed->l_head) {
        if (inlet == 0) {
            
            earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, parsed->l_size, true);
            earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_OUT, 0, parsed->l_size, true);
            earsbufobj_store_buffer_list((t_earsbufobj *)x, parsed, 0, true);
            
            buf_resample_bang(x);
        } else if (inlet == 1) {
            earsbufobj_mutex_lock((t_earsbufobj *)x);
            llll_free(x->resamplefactor);
            x->resamplefactor = llll_clone(parsed);
            earsbufobj_mutex_unlock((t_earsbufobj *)x);
        }
    }
    llll_free(parsed);
}


