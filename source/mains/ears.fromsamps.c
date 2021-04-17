/**
	@file
	ears.fromsamps.c
 
	@name
	ears.fromsamps~
 
	@realname
	ears.fromsamps~
 
	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	Create a buffer from an llll
 
	@description
	Converts an llll into a buffer
 
	@discussion
 
	@category
	ears basic
 
	@keywords
	buffer, convert, create, samples
 
	@seealso
	ears.tosamps~
	
	@owner
	Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "llllobj.h"
#include "llll_commons_ext.h"
#include "bach_math_utilities.h"
#include "ears.object.h"


typedef struct _buf_fromsamps {
    t_earsbufobj       e_ob;
    
    double      sr;
} t_buf_fromsamps;



// Prototypes
t_buf_fromsamps*         buf_fromsamps_new(t_symbol *s, short argc, t_atom *argv);
void			buf_fromsamps_free(t_buf_fromsamps *x);
void			buf_fromsamps_bang(t_buf_fromsamps *x);
void			buf_fromsamps_anything(t_buf_fromsamps *x, t_symbol *msg, long ac, t_atom *av);

void buf_fromsamps_assist(t_buf_fromsamps *x, void *b, long m, long a, char *s);
void buf_fromsamps_inletinfo(t_buf_fromsamps *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(fromsamps)

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
    
    CLASS_NEW_CHECK_SIZE(c, "ears.fromsamps~",
                         (method)buf_fromsamps_new,
                         (method)buf_fromsamps_free,
                         sizeof(t_buf_fromsamps),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    // @method list/llll @digest Create buffer
    // @description A list or llll will be interpreted as a sequence of samples that will be put inside a buffer.
    // If the llll has depth 2, each levels of parentheses is considered to be a channel.
    // The buffer name will be then output.
    EARSBUFOBJ_DECLARE_COMMON_METHODS_HANDLETHREAD(fromsamps)
    
    CLASS_ATTR_DOUBLE(c, "sr", 0, t_buf_fromsamps, sr);
    CLASS_ATTR_STYLE_LABEL(c,"sr",0,"text","Output Sample Rate");
    CLASS_ATTR_BASIC(c, "sr", 0);
    // @description Sets the sample rate for the output buffer. If zero (default) then the current sample rate is used.

    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
    return 0;
}

void buf_fromsamps_assist(t_buf_fromsamps *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        sprintf(s, "llll: Samples"); // @in 0 @type llll @digest Samples
    } else {
        sprintf(s, "symbol: Output Buffer"); // @out 0 @type symbol @digest Output Buffer
    }
}

void buf_fromsamps_inletinfo(t_buf_fromsamps *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_fromsamps *buf_fromsamps_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_fromsamps *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_fromsamps*)object_alloc_debug(s_tag_class);
    if (x) {
        earsbufobj_init((t_earsbufobj *)x, 0);

        // @arg 0 @name outnames @optional 1 @type symbol
        // @digest Output buffer names
        // @description @copy EARS_DOC_OUTNAME_ATTR
        
        t_llll *args = llll_parse(true_ac, argv);
        t_llll *names = earsbufobj_extract_names_from_args((t_earsbufobj *)x, args);

        attr_args_process(x, argc, argv);

        earsbufobj_setup((t_earsbufobj *)x, "4", "e", names);

        llll_free(args);
        llll_free(names);
    }
    return x;
}


void buf_fromsamps_free(t_buf_fromsamps *x)
{
    earsbufobj_free((t_earsbufobj *)x);
}



void buf_fromsamps_bang(t_buf_fromsamps *x)
{
    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
    earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
}




void buf_fromsamps_anything(t_buf_fromsamps *x, t_symbol *msg, long ac, t_atom *av)
{
    long inlet = earsbufobj_proxy_getinlet((t_earsbufobj *) x);

    t_llll *parsed = earsbufobj_parse_gimme((t_earsbufobj *) x, LLLL_OBJ_VANILLA, msg, ac, av);
    if (!parsed) return;
    
    if (parsed) {
        if (inlet == 0) {
            if (parsed->l_depth == 1) // flat llll = 1 channel only
                llll_wrap_once(&parsed);
            earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_OUT, 0, 1, true);
            if (x->sr > 0)
                ears_buffer_set_sr((t_object *)x, earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, 0), x->sr);
            ears_buffer_from_llll((t_object *)x, earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, 0), parsed, true);
            buf_fromsamps_bang(x);
        }
    }
    llll_free(parsed);
}


