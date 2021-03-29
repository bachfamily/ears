/**
	@file
	ears.window.c
 
	@name
	ears.window~
 
	@realname
	ears.window~
 
	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	Apply an analysis window
 
	@description
	Applies an analysis window to the incoming buffer
 
	@discussion
 
	@category
	ears basic
 
	@keywords
	buffer, window, apply, hanning, hamming, triangular, tri
 
	@seealso
	ears.envelope~
	
	@owner
	Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "llllobj.h"
#include "llll_commons_ext.h"
#include "bach_math_utilities.h"
#include "ears.object.h"



typedef struct _buf_window {
    t_earsbufobj       e_ob;
    
    t_symbol           *window_type;
} t_buf_window;



// Prototypes
t_buf_window*         buf_window_new(t_symbol *s, short argc, t_atom *argv);
void			buf_window_free(t_buf_window *x);
void			buf_window_bang(t_buf_window *x);
void			buf_window_anything(t_buf_window *x, t_symbol *msg, long ac, t_atom *av);

void buf_window_assist(t_buf_window *x, void *b, long m, long a, char *s);
void buf_window_inletinfo(t_buf_window *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(window)

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
    
    CLASS_NEW_CHECK_SIZE(c, "ears.window~",
                         (method)buf_window_new,
                         (method)buf_window_free,
                         sizeof(t_buf_window),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    // @method list/llll @digest Function depends on inlet
    // @description A list or llll in the first inlet is supposed to contain buffer names and will
    // trigger the buffer processing and output the processed buffer names (depending on the <m>naming</m> attribute). <br />
    EARSBUFOBJ_DECLARE_COMMON_METHODS_DEFER(window)

    // @method symbol/llll @digest Set window type
    // @description A symbol (or an llll containing a symbol) in the second inlet sets the window type.
    // Available windows are: "rect", "tri", "sine", "hann", "hamming", "blackman", "nuttall",
    // "blackmannuttall", "blackmanharris", "gaussian"

    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_naming_attr(c);

    
    CLASS_ATTR_SYM(c, "wintype", 0, t_buf_window, window_type);
    CLASS_ATTR_ENUM(c,"wintype", 0, "rect tri sine hann hamming blackman nuttall blackmannuttall blackmanharris gaussian");
    CLASS_ATTR_BASIC(c, "wintype", 0);
    // @description Sets the window type.
    // Available windows are: "rect", "tri", "sine", "hann", "hamming", "blackman", "nuttall",
    // "blackmannuttall", "blackmanharris", "gaussian"

    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
    return 0;
}

void buf_window_assist(t_buf_window *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        if (a == 0)
            sprintf(s, "symbol/list/llll: Incoming Buffer Names"); // @in 0 @type symbol/list/llll @digest Incoming buffer names
        else
            sprintf(s, "symbol/llll: Window Type"); // @in 1 @type symbol/llll @digest Window Type
    } else {
        sprintf(s, "symbol/list: Output Buffer Names"); // @out 0 @type symbol/list @digest Output buffer names
    }
}

void buf_window_inletinfo(t_buf_window *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_window *buf_window_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_window *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_window*)object_alloc_debug(s_tag_class);
    if (x) {
        x->window_type = gensym("hann");
        
        earsbufobj_init((t_earsbufobj *)x,  EARSBUFOBJ_FLAG_SUPPORTS_COPY_NAMES);
        
        // @arg 0 @name outnames @optional 1 @type symbol
        // @digest Output buffer names
        // @description @copy EARS_DOC_OUTNAME_ATTR

        t_llll *args = llll_parse(true_ac, argv);
        t_llll *names = earsbufobj_extract_names_from_args((t_earsbufobj *)x, args);
        
        attr_args_process(x, argc, argv);
        
        earsbufobj_setup((t_earsbufobj *)x, "E4", "E", names);

        llll_free(args);
        llll_free(names);
    }
    return x;
}


void buf_window_free(t_buf_window *x)
{
    earsbufobj_free((t_earsbufobj *)x);
}



void buf_window_bang(t_buf_window *x)
{
    long num_buffers = ((t_earsbufobj *)x)->l_instore[0].num_stored_bufs;
    
    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_buffers, true);
    
    earsbufobj_mutex_lock((t_earsbufobj *)x);
    for (long count = 0; count < num_buffers; count++) {
        t_buffer_obj *in = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, count);
        t_buffer_obj *out = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, count);
//        ears_buffer_clone((t_object *)x, in, out);
        ears_buffer_apply_window((t_object *)x, in, out, x->window_type);
    }
    earsbufobj_mutex_unlock((t_earsbufobj *)x);
    
    earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
}


void buf_window_anything(t_buf_window *x, t_symbol *msg, long ac, t_atom *av)
{
    long inlet = earsbufobj_proxy_getinlet((t_earsbufobj *) x);

    t_llll *parsed = earsbufobj_parse_gimme((t_earsbufobj *) x, LLLL_OBJ_VANILLA, msg, ac, av);
    if (!parsed) return;
    
    if (parsed && parsed->l_head) {
        if (inlet == 0) {
            
            earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, parsed->l_size, true);
            earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_OUT, 0, parsed->l_size, true);
            
            earsbufobj_store_buffer_list((t_earsbufobj *)x, parsed, 0);
            
            buf_window_bang(x);
            
        } else if (inlet == 1) {
            earsbufobj_mutex_lock((t_earsbufobj *)x);
            if (hatom_gettype(&parsed->l_head->l_hatom) == H_SYM)
                x->window_type = hatom_getsym(&parsed->l_head->l_hatom);
            earsbufobj_mutex_unlock((t_earsbufobj *)x);
        }
    }
    llll_free(parsed);
}


