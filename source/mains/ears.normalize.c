/**
	@file
	ears.normalize.c
 
	@name
	ears.normalize~
 
	@realname
	ears.normalize~
 
	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	Normalize buffers
 
	@description
	Rescales buffer amplitudes
 
	@discussion
 
	@category
	ears basic
 
	@keywords
	buffer, normalize, scale
 
	@seealso
	ears.read~, ears.gain~
	
	@owner
	Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "llllobj.h"
#include "llll_commons_ext.h"
#include "bach_math_utilities.h"
#include "ears.object.h"



typedef struct _buf_normalize {
    t_earsbufobj       e_ob;
    
    double             level;
    char               rms_mode;
    
    double             mix;
} t_buf_normalize;



// Prototypes
t_buf_normalize*         buf_normalize_new(t_symbol *s, short argc, t_atom *argv);
void			buf_normalize_free(t_buf_normalize *x);
void			buf_normalize_bang(t_buf_normalize *x);
void			buf_normalize_anything(t_buf_normalize *x, t_symbol *msg, long ac, t_atom *av);

void buf_normalize_assist(t_buf_normalize *x, void *b, long m, long a, char *s);
void buf_normalize_inletinfo(t_buf_normalize *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(normalize)

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
    
    CLASS_NEW_CHECK_SIZE(c, "ears.normalize~",
                         (method)buf_normalize_new,
                         (method)buf_normalize_free,
                         sizeof(t_buf_normalize),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    // @method list/llll @digest Process buffers
    // @description A list or llll with buffer names will trigger the buffer processing and output the processed
    // buffer names (depending on the <m>naming</m> attribute).
    EARSBUFOBJ_DECLARE_COMMON_METHODS_DEFER(normalize)
    
    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_ampunit_attr(c);
    earsbufobj_class_add_naming_attr(c);

    
    CLASS_ATTR_CHAR(c, "rms",	0,	t_buf_normalize, rms_mode);
    CLASS_ATTR_STYLE_LABEL(c, "rms", 0, "onoff", "Use Root Mean Square");
    CLASS_ATTR_BASIC(c, "rms", 0);
    // @description Toggles usage of root mean square values.

    CLASS_ATTR_DOUBLE(c, "level",	0,	t_buf_normalize, level);
    CLASS_ATTR_STYLE_LABEL(c, "level", 0, "text", "Reference Level");
    CLASS_ATTR_BASIC(c, "level", 0);
    // @description Sets the amplitude reference level (unit given by the <m>ampunit</m> attribute)

    CLASS_ATTR_DOUBLE(c, "mix",	0,	t_buf_normalize, mix);
    CLASS_ATTR_STYLE_LABEL(c, "mix", 0, "text", "Dry/Wet Mix");
    // @description Sets a dry/wet factor to mix with the original sample, 0 being all dry and 1 (default)
    // being all wet.
    
    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
    return 0;
}

void buf_normalize_assist(t_buf_normalize *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        sprintf(s, "symbol/list/llll: Incoming Buffer Names"); // @in 0 @type symbol/list/llll @digest Incoming buffer names
    } else {
        sprintf(s, "symbol/list: Normalized Buffer Names"); // @out 0 @type symbol/list @digest Normalized buffer names
    }
}

void buf_normalize_inletinfo(t_buf_normalize *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_normalize *buf_normalize_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_normalize *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_normalize*)object_alloc_debug(s_tag_class);
    if (x) {
        x->rms_mode = 0;
        x->level = 1.;
        x->mix = 1.;
        
        earsbufobj_init((t_earsbufobj *)x,  EARSBUFOBJ_FLAG_SUPPORTS_COPY_NAMES);
        
        // @arg 0 @name outnames @optional 1 @type symbol
        // @digest Output buffer names
        // @description @copy EARS_DOC_OUTNAME_ATTR

        // @arg 1 @name reference_level @optional 1 @type number
        // @digest Normalization reference level
        // @description Sets the normalization reference level

        // @arg 2 @name amp_unit @optional 1 @type symbol
        // @digest Amplitude unit
        // @description Sets the amplitude unit (see <m>ampunit</m> attribute)

        t_llll *args = llll_parse(true_ac, argv);
        t_llll *names = earsbufobj_extract_names_from_args((t_earsbufobj *)x, args);
        
        if (args && args->l_head) {
            x->level = hatom_getdouble(&args->l_head->l_hatom);
            if (args->l_head->l_next && hatom_gettype(&args->l_head->l_next->l_hatom) == H_SYM) {
                t_atom av;
                atom_setsym(&av, hatom_getsym(&args->l_head->l_next->l_hatom));
                object_attr_setvalueof(x, gensym("ampunit"), 1, &av);
            }
        }
        
        attr_args_process(x, argc, argv);

        earsbufobj_setup((t_earsbufobj *)x, "E", "E", names);

        llll_free(args);
        llll_free(names);
    }
    return x;
}


void buf_normalize_free(t_buf_normalize *x)
{
    earsbufobj_free((t_earsbufobj *)x);
}


void buf_normalize_bang(t_buf_normalize *x)
{
    long num_buffers = ((t_earsbufobj *)x)->l_instore[0].num_stored_bufs;
    
    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_buffers, true);
    
    for (long count = 0; count < num_buffers; count++) {
        double this_linear_amp = earsbufobj_input_to_linear((t_earsbufobj *)x, x->level);

        t_buffer_obj *in = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, count);
        t_buffer_obj *out = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, count);
        
        if (x->rms_mode)
            ears_buffer_normalize_rms((t_object *)x, in, out, this_linear_amp, x->mix);
        else
            ears_buffer_normalize((t_object *)x, in, out, this_linear_amp, x->mix);
    }
    
    earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
}


void buf_normalize_anything(t_buf_normalize *x, t_symbol *msg, long ac, t_atom *av)
{
    long inlet = proxy_getinlet((t_object *) x);

    t_llll *parsed = earsbufobj_parse_gimme((t_earsbufobj *) x, LLLL_OBJ_VANILLA, msg, ac, av);
    if (!parsed) return;
    
    if (parsed && parsed->l_head) {
        if (inlet == 0) {
            
            earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, parsed->l_size, true);
            earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_OUT, 0, parsed->l_size, true);
            earsbufobj_store_buffer_list((t_earsbufobj *)x, parsed, 0, true);
            
            buf_normalize_bang(x);
        }
    }
    llll_free(parsed);
}


