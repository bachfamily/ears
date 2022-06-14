/**
	@file
	ears.onepole.c
 
	@name
	ears.onepole~
 
	@realname
	ears.onepole~
 
	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	Single pole lowpass or highpass filter for buffers
 
	@description
	Implements the simplest of IIR filters, providing a 6dB per octave attenuation
 
	@discussion
 
	@category
	ears filter
 
	@keywords
	buffer, onepole, filter, lowpass, hipass, highpass, attenuation
 
	@seealso
	onepole~, ears.biquad~
	
	@owner
	Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "foundation/llllobj.h"
#include "foundation/llll_commons_ext.h"
#include "bach_math_utilities.h"
#include "ears.object.h"



typedef struct _buf_onepole {
    t_earsbufobj       e_ob;
    
    char               highpass;
    t_llll             *cutoff_freq;
} t_buf_onepole;



// Prototypes
t_buf_onepole*         buf_onepole_new(t_symbol *s, short argc, t_atom *argv);
void			buf_onepole_free(t_buf_onepole *x);
void			buf_onepole_bang(t_buf_onepole *x);
void			buf_onepole_anything(t_buf_onepole *x, t_symbol *msg, long ac, t_atom *av);

void buf_onepole_assist(t_buf_onepole *x, void *b, long m, long a, char *s);
void buf_onepole_inletinfo(t_buf_onepole *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(onepole)

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
    
    CLASS_NEW_CHECK_SIZE(c, "ears.onepole~",
                         (method)buf_onepole_new,
                         (method)buf_onepole_free,
                         sizeof(t_buf_onepole),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    // @method list/llll @digest Process buffers
    // @description A list or llll with buffer names will trigger the buffer processing and output the processed
    // buffer names (depending on the <m>naming</m> attribute).
    EARSBUFOBJ_DECLARE_COMMON_METHODS_HANDLETHREAD(onepole)

    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_blocking_attr(c);
    earsbufobj_class_add_naming_attr(c);

    earsbufobj_class_add_polyout_attr(c);

    CLASS_ATTR_CHAR(c, "highpass", 0, t_buf_onepole, highpass);
    CLASS_ATTR_STYLE_LABEL(c,"highpass",0,"onoff","Highpass Filter");
    CLASS_ATTR_BASIC(c, "highpass", 0);
    // @description Toggles the ability to use a one-pole highpass filter instead of a one-pole lowpass filter.

    
    
    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
    return 0;
}

void buf_onepole_assist(t_buf_onepole *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        if (a == 0)
            sprintf(s, "symbol/list/llll: Incoming Buffer Names"); // @in 0 @type symbol/list/llll @digest Incoming buffer names
        else
            sprintf(s, "float/llll: Cutoff Frequency"); // @in 1 @type float/llll @digest Cutoff frequency
    } else {
        sprintf(s, "symbol/list: Output Buffer Names"); // @out 0 @type symbol/list @digest Output buffer names
    }
}

void buf_onepole_inletinfo(t_buf_onepole *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_onepole *buf_onepole_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_onepole *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_onepole*)object_alloc_debug(s_tag_class);
    if (x) {
        x->cutoff_freq = llll_from_text_buf("1.", false);
        
        earsbufobj_init((t_earsbufobj *)x,  EARSBUFOBJ_FLAG_SUPPORTS_COPY_NAMES);
        
        // @arg 0 @name outnames @optional 1 @type symbol
        // @digest Output buffer names
        // @description @copy EARS_DOC_OUTNAME_ATTR

        // @arg 1 @name cutoff_freq @optional 1 @type float/llll
        // @digest Cutoff frequency
        // @description Sets the filter cutoff frequency. If a list is given, each element is applied to one of the incoming buffers.

        t_llll *args = llll_parse(true_ac, argv);
        t_llll *names = earsbufobj_extract_names_from_args((t_earsbufobj *)x, args);
        
        if (args && args->l_head) {
            llll_free(x->cutoff_freq);
            x->cutoff_freq = llll_clone(args);
        }
        
        attr_args_process(x, argc, argv);

        earsbufobj_setup((t_earsbufobj *)x, "E4", "E", names);

        llll_free(args);
        llll_free(names);
        
    }
    return x;
}


void buf_onepole_free(t_buf_onepole *x)
{
    llll_free(x->cutoff_freq);
    earsbufobj_free((t_earsbufobj *)x);
}



void buf_onepole_bang(t_buf_onepole *x)
{
    long num_buffers = earsbufobj_get_instore_size((t_earsbufobj *)x, 0);
    long highpass = x->highpass;
    
    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_buffers, true);
    
    earsbufobj_mutex_lock((t_earsbufobj *)x);
    earsbufobj_init_progress((t_earsbufobj *)x, num_buffers);
    
    t_llllelem *el = x->cutoff_freq->l_head;
    for (long count = 0; count < num_buffers; count++, el = el && el->l_next ? el->l_next : el) {
        t_buffer_obj *in = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, count);
        t_buffer_obj *out = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, count);

        if (el) {
            double cutoff_freq = hatom_getdouble(&el->l_hatom);
            ears_buffer_onepole((t_object *)x, in, out, cutoff_freq, highpass);
        }

        if (earsbufobj_iter_progress((t_earsbufobj *)x, count, num_buffers)) break;
    }
    earsbufobj_mutex_unlock((t_earsbufobj *)x);
    
    earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
}


void buf_onepole_anything(t_buf_onepole *x, t_symbol *msg, long ac, t_atom *av)
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
            
            buf_onepole_bang(x);
            
        } else if (inlet == 1) {
            earsbufobj_mutex_lock((t_earsbufobj *)x);
            llll_free(x->cutoff_freq);
            x->cutoff_freq = llll_clone(parsed);
            earsbufobj_mutex_unlock((t_earsbufobj *)x);
        }
    }
    llll_free(parsed);
}


