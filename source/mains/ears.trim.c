/**
	@file
	ears.trim.c
 
	@name
	ears.trim~
 
	@realname
	ears.trim~
 
	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	Trim silence from a buffer
 
	@description
    Removes the silence at the beginning and/or at the end of a buffer, depending on an amplitude threshold.
 
	@discussion
 
 
	@category
	ears basic
 
	@keywords
	buffer, trim, silence, delete, remove
 
	@seealso
	ears.split~, ears.crop~
	
	@owner
	Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "llllobj.h"
#include "llll_commons_ext.h"
#include "bach_math_utilities.h"
#include "ears.object.h"
#include "ears.commons.h"

typedef struct _buf_trim {
    t_earsbufobj       e_ob;
    double             e_ampthreshold;
    long               e_trim_start;
    long               e_trim_end;

} t_buf_trim;



// Prototypes
t_buf_trim*         buf_trim_new(t_symbol *s, short argc, t_atom *argv);
void			buf_trim_free(t_buf_trim *x);
void			buf_trim_bang(t_buf_trim *x);
void			buf_trim_anything(t_buf_trim *x, t_symbol *msg, long ac, t_atom *av);

void buf_trim_assist(t_buf_trim *x, void *b, long m, long a, char *s);
void buf_trim_inletinfo(t_buf_trim *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(trim)



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
    
    CLASS_NEW_CHECK_SIZE(c, "ears.trim~",
                         (method)buf_trim_new,
                         (method)buf_trim_free,
                         sizeof(t_buf_trim),
                         (method)NULL,
                         A_GIMME,
                         0L);

    // @method list/llll @digest Process buffers
    // @description A list or llll with buffer names will trigger the buffer processing and output the processed
    // buffer names (depending on the <m>naming</m> attribute).
    EARSBUFOBJ_DECLARE_COMMON_METHODS_HANDLETHREAD(trim)
    
    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_ampunit_attr(c);
    earsbufobj_class_add_naming_attr(c);

    CLASS_ATTR_DOUBLE(c, "thresh", 0, t_buf_trim, e_ampthreshold);
    CLASS_ATTR_STYLE_LABEL(c,"thresh",0,"text","Amplitude Threshold");
    CLASS_ATTR_BASIC(c, "thresh", 0);
    // @description Sets the amplitude threshold at which and below which samples are considered as silence (defaults to 0).

    CLASS_ATTR_LONG(c, "start", 0, t_buf_trim, e_trim_start);
    CLASS_ATTR_STYLE_LABEL(c,"start",0,"onoff","Trim Beginning");
    CLASS_ATTR_BASIC(c, "start", 0);
    // @description Toggles trimming for the beginning of the buffer (defaults to 1).

    CLASS_ATTR_LONG(c, "end", 0, t_buf_trim, e_trim_end);
    CLASS_ATTR_STYLE_LABEL(c,"end",0,"onoff","Trim End");
    CLASS_ATTR_BASIC(c, "end", 0);
    // @description Toggles trimming for the end of the buffer (defaults to 1).

    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
    return 0;
}

void buf_trim_assist(t_buf_trim *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        if (a == 0) // @in 0 @type symbol/list/llll @digest Buffer name(s)
            sprintf(s, "symbol/llll: Buffer Names");
    } else {
        sprintf(s, "Shifted Buffer Names"); // @out 0 @type symbol/list @digest Trimmed buffer name(s)
    }
}

void buf_trim_inletinfo(t_buf_trim *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_trim *buf_trim_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_trim *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_trim*)object_alloc_debug(s_tag_class);
    if (x) {
        x->e_ampthreshold = 0.;
        x->e_trim_start = x->e_trim_end = 1;

        earsbufobj_init((t_earsbufobj *)x,  EARSBUFOBJ_FLAG_SUPPORTS_COPY_NAMES);
        
        // @arg 0 @name outname @optional 1 @type symbol
        // @digest Output buffer name
        // @description @copy EARS_DOC_OUTNAME_ATTR
        
        t_llll *args = llll_parse(true_ac, argv);
        t_llll *names = earsbufobj_extract_names_from_args((t_earsbufobj *)x, args);
        
        // @arg 1 @name ampthresh @optional 1 @type float
        // @digest Amplitude threshold
        // @description Sets the amplitude threshold below which samples are considered as silence.
        
        if (args && args->l_head && is_hatom_number(&args->l_head->l_hatom))
            x->e_ampthreshold = hatom_getdouble(&args->l_head->l_hatom);
        
        attr_args_process(x, argc, argv);
        
        earsbufobj_setup((t_earsbufobj *)x, "E", "E", names);

        llll_free(args);
        llll_free(names);
        
    }
    return x;
}


void buf_trim_free(t_buf_trim *x)
{
    earsbufobj_free((t_earsbufobj *)x);
}




void buf_trim_bang(t_buf_trim *x)
{
    long num_buffers = earsbufobj_get_instore_size((t_earsbufobj *)x, 0);
    
    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_buffers, true);
    
    earsbufobj_mutex_lock((t_earsbufobj *)x);
    for (long count = 0; count < num_buffers; count++) {
        t_buffer_obj *in = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, count);
        t_buffer_obj *out = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, count);
        
        double thresh = earsbufobj_amplitude_to_linear((t_earsbufobj *)x, x->e_ampthreshold);
        
        ears_buffer_trim((t_object *)x, in, out, thresh, x->e_trim_start, x->e_trim_end);
    }
    earsbufobj_mutex_unlock((t_earsbufobj *)x);

    earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
}

void buf_trim_anything(t_buf_trim *x, t_symbol *msg, long ac, t_atom *av)
{
    long inlet = earsbufobj_proxy_getinlet((t_earsbufobj *) x);
    
    t_llll *parsed = earsbufobj_parse_gimme((t_earsbufobj *) x, LLLL_OBJ_VANILLA, msg, ac, av);
    if (!parsed) return;
    
    if (parsed && parsed->l_head) {
        if (inlet == 0) {
            if (hatom_gettype(&parsed->l_head->l_hatom) == H_SYM) {
                long num_bufs = llll_get_num_symbols_root(parsed);

                earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_bufs, true);
                earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_OUT, 0, num_bufs, true);
                earsbufobj_store_buffer_list((t_earsbufobj *)x, parsed, 0);
                
                buf_trim_bang(x);
            }
        }
    }
    llll_free(parsed);
}



