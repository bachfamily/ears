/**
	@file
	ears.paulstretch.c
 
	@name
	ears.paulstretch~
 
	@realname
	ears.paulstretch~
 
    @hiddenalias
    ears.paulstretch

	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	Paulstretch timestretching
 
	@description
    Implements the public domain 'Paulstretch' time stretching algorithm
 
	@discussion
    The original algorithm is by Nasca Octavian PAUL, Targu Mures, Romania, http://www.paulnasca.com/

	@category
	ears buffer operations
 
	@keywords
	buffer, stretch, timestretch, expand, compress
 
	@seealso
	ears.freeverb~
	
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


typedef struct _buf_paulstretch {
    t_earsbufobj       e_ob;
    
    t_llll             *e_factor;
    double             e_winsize;
    char               e_spectral;
} t_buf_paulstretch;



// Prototypes
t_buf_paulstretch*         buf_paulstretch_new(t_symbol *s, short argc, t_atom *argv);
void			buf_paulstretch_free(t_buf_paulstretch *x);
void			buf_paulstretch_bang(t_buf_paulstretch *x);
void			buf_paulstretch_anything(t_buf_paulstretch *x, t_symbol *msg, long ac, t_atom *av);

void buf_paulstretch_assist(t_buf_paulstretch *x, void *b, long m, long a, char *s);
void buf_paulstretch_inletinfo(t_buf_paulstretch *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(paulstretch)

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
    
    CLASS_NEW_CHECK_SIZE(c, "ears.paulstretch~",
                         (method)buf_paulstretch_new,
                         (method)buf_paulstretch_free,
                         sizeof(t_buf_paulstretch),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    // @method list/llll @digest Process buffers
    // @description A list or llll with buffer names will trigger the buffer processing and output the processed
    // buffer names (depending on the <m>naming</m> attribute).
    EARSBUFOBJ_DECLARE_COMMON_METHODS_DEFER(paulstretch)

    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_naming_attr(c);
    earsbufobj_class_add_timeunit_attr(c);
    earsbufobj_class_add_envtimeunit_attr(c);

    CLASS_ATTR_DOUBLE(c, "winsize", 0, t_buf_paulstretch, e_winsize);
    CLASS_ATTR_STYLE_LABEL(c,"winsize",0,"text","Windows Size");
    CLASS_ATTR_BASIC(c, "winsize", 0);
    // @description Sets the windows size (the unit depends on the <m>timeunit</m> attribute)

    CLASS_ATTR_CHAR(c, "spectral", 0, t_buf_paulstretch, e_spectral);
    CLASS_ATTR_STYLE_LABEL(c,"spectral",0,"onoff","Frequency Domain");
    // @description If toggled, the algorithm works in the frequency domain, by overlap-adding FFT windows (default);
    // if untoggled, it works in the time domain, by overlap-adding time grains

    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
    return 0;
}

void buf_paulstretch_assist(t_buf_paulstretch *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        if (a == 0)
            sprintf(s, "symbol/list/llll: Incoming Buffer Names"); // @in 0 @type symbol/list/llll @digest Incoming buffer names
        else
            sprintf(s, "float/list/llll: Stretch Factor or Envelope");
        // @in 1 @type float/list/llll @digest Stretch factor or envelope
        // @description Sets the stretch factor, either as a single number or as an llll
        // containing an envelope in the form <b>[[<m>x</m> <m>factor</m> <m>slope</m>]
        // [<m>x</m> <m>factor</m> <m>slope</m>]...]</b>.
        // where <m>x</m> values' range depends on the <m>envtimeunit</m> attribute.
    } else {
        sprintf(s, "symbol/list: Output Buffer Names"); // @out 0 @type symbol/list @digest Output buffer names
    }
}

void buf_paulstretch_inletinfo(t_buf_paulstretch *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_paulstretch *buf_paulstretch_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_paulstretch *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_paulstretch*)object_alloc_debug(s_tag_class);
    if (x) {
        // @arg 0 @name outnames @optional 1 @type symbol
        // @digest Output buffer names
        // @description @copy EARS_DOC_OUTNAME_ATTR

        // @arg 1 @name factor @optional 1 @type float/llll
        // @digest Stretch factor
        // @description Sets the stretch factor, either as a single number or as an llll
        // containing an envelope in the form <b>[[<m>x</m> <m>factor</m> <m>slope</m>]
        // [<m>x</m> <m>factor</m> <m>slope</m>]...]</b>.
        // where <m>x</m> values' range depends on the <m>envtimeunit</m> attribute.

        x->e_factor = llll_from_text_buf("1.");
        x->e_winsize = 250; // 250 ms as default
        x->e_spectral = true;
        
        t_llll *args = llll_parse(true_ac, argv);
        t_llll *names = earsbufobj_extract_names_from_args((t_earsbufobj *)x, args);
        
        if (args && args->l_head) {
            llll_clear(x->e_factor);
            llll_appendhatom_clone(x->e_factor, &args->l_head->l_hatom);
        }
        
        earsbufobj_init((t_earsbufobj *)x, EARSBUFOBJ_FLAG_DONT_DUPLICATE_INPUT_BUFFERS | EARSBUFOBJ_FLAG_SUPPORTS_COPY_NAMES);

        attr_args_process(x, argc, argv);

        earsbufobj_setup((t_earsbufobj *)x, "E4", "E", names);

        llll_free(args);
        llll_free(names);
        
    }
    return x;
}


void buf_paulstretch_free(t_buf_paulstretch *x)
{
    llll_free(x->e_factor);
    earsbufobj_free((t_earsbufobj *)x);
}

void buf_paulstretch_bang(t_buf_paulstretch *x)
{
    long num_buffers = ((t_earsbufobj *)x)->l_instore[0].num_stored_bufs;
    
    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_buffers, true);
    
    earsbufobj_mutex_lock((t_earsbufobj *)x);

    t_llllelem *el = x->e_factor->l_head;
    for (long count = 0; count < num_buffers; count++, el = el && el->l_next ? el->l_next : el) {
        t_buffer_obj *in = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, count);
        t_buffer_obj *out = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, count);
        
        t_llll *env = earsbufobj_llllelem_to_env_samples((t_earsbufobj *)x, el, in);

        if (env->l_size == 0) {
            object_error((t_object *)x, "No stretch factor defined.");
            if (in != out)
                ears_buffer_clone((t_object *)x, in, out);
        } if (env->l_depth == 1 && env->l_head) {
            ears_buffer_paulstretch((t_object *)x, in, out, hatom_getdouble(&env->l_head->l_hatom), earsbufobj_input_to_samps((t_earsbufobj *)x, x->e_winsize, in), x->e_spectral);
        } else {
            ears_buffer_paulstretch_envelope((t_object *)x, in, out, env, earsbufobj_input_to_samps((t_earsbufobj *)x, x->e_winsize, in), x->e_spectral);
        }
        
        llll_free(env);
    }
    earsbufobj_mutex_unlock((t_earsbufobj *)x);
    
    earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
}


void buf_paulstretch_anything(t_buf_paulstretch *x, t_symbol *msg, long ac, t_atom *av)
{
    long inlet = proxy_getinlet((t_object *) x);

    t_llll *parsed = earsbufobj_parse_gimme((t_earsbufobj *) x, LLLL_OBJ_VANILLA, msg, ac, av);
    if (!parsed) return;
    
    if (parsed && parsed->l_head) {
        if (inlet == 0) {
            
            earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, parsed->l_size, true);
            earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_OUT, 0, parsed->l_size, true);
            
            earsbufobj_store_buffer_list((t_earsbufobj *)x, parsed, 0, true);
            
            buf_paulstretch_bang(x);
         
        } else if (inlet == 1) {
            earsbufobj_mutex_lock((t_earsbufobj *)x);
            llll_free(x->e_factor);
            x->e_factor = llll_clone(parsed);
            earsbufobj_mutex_unlock((t_earsbufobj *)x);
        }
        
    }
    llll_free(parsed);
}


