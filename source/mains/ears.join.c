/**
	@file
	ears.join.c
 
	@name
	ears.join~
 
	@realname
	ears.join~
 
	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	Concatenate buffers
 
	@description
	Puts audio buffers in a linear sequence
 
	@discussion
 
	@category
	ears basic
 
	@keywords
	buffer, crossfade, joinenate, sequence, merge
 
	@seealso
	ears.crop~, ears.read~, ears.fade~, ears.mix~
	
	@owner
	Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "llllobj.h"
#include "llll_commons_ext.h"
#include "bach_math_utilities.h"
#include "ears.object.h"


typedef struct _buf_join {
    t_earsbufobj       e_ob;
    
    double             xfade;
    
    char               xfade_type;
    double             xfade_curve;
    
    char               also_fade_first_and_last;
} t_buf_join;



// Prototypes
t_buf_join*     buf_join_new(t_symbol *s, short argc, t_atom *argv);
void			buf_join_free(t_buf_join *x);
void			buf_join_bang(t_buf_join *x);
void			buf_join_anything(t_buf_join *x, t_symbol *msg, long ac, t_atom *av);

void buf_join_assist(t_buf_join *x, void *b, long m, long a, char *s);
void buf_join_inletinfo(t_buf_join *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(join)

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
    
    CLASS_NEW_CHECK_SIZE(c, "ears.join~",
                         (method)buf_join_new,
                         (method)buf_join_free,
                         sizeof(t_buf_join),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    // @method list/llll @digest Process buffers
    // @description A list or llll with buffer names will trigger the buffer processing and output the processed
    // buffer names (depending on the <m>naming</m> attribute).
    EARSBUFOBJ_DECLARE_COMMON_METHODS_DEFER(join)

    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_timeunit_attr(c);
    earsbufobj_class_add_naming_attr(c);
    earsbufobj_class_add_slopemapping_attr(c);

    
    CLASS_ATTR_CHAR(c, "xfadetype", 0, t_buf_join, xfade_type);
    CLASS_ATTR_STYLE_LABEL(c,"xfadetype",0,"enumindex","Crossfade Type");
    CLASS_ATTR_ENUMINDEX(c,"xfadetype", 0, "None Linear Sine (Equal Power) Curve S-Curve");
    CLASS_ATTR_BASIC(c, "xfadetype", 0);
    // @description Sets the cross fade type: 0 = None, 1 = Linear, 2 = Sine (Equal Power, default), 3 = Curve, 4 = S-Curve

    
    CLASS_ATTR_DOUBLE(c, "xfadecurve", 0, t_buf_join, xfade_curve);
    CLASS_ATTR_STYLE_LABEL(c,"xfadecurve",0,"text","Crossfade Curve");
    // @description Sets the curve parameter for the crossfade (for fades of type Curve and S-Curve only).
    // The parameters goes from -1 to 1, 0 being linear (default).

    CLASS_ATTR_DOUBLE(c, "xfade", 0, t_buf_join, xfade);
    CLASS_ATTR_STYLE_LABEL(c,"xfade",0,"text","Crossfade Duration");
    CLASS_ATTR_BASIC(c, "xfade", 0);
    // @description Sets the duration of the crossfade.
    // Unit is set via the <m>timeunit</m> attribute.

    CLASS_ATTR_CHAR(c, "fadeboundaries", 0, t_buf_join, also_fade_first_and_last);
    CLASS_ATTR_STYLE_LABEL(c,"fadeboundaries",0,"text","onoff");
    CLASS_ATTR_STYLE_LABEL(c,"fadeboundaries",0,"text","Fade Beginning And End");
    // @description Toggles the ability to also fade the beginning of the first sample and the end of the last sample


    
    
    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
    return 0;
}

void buf_join_assist(t_buf_join *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        if (a == 0) // @in 0 @type list @digest Names of the buffers to joinenate
            sprintf(s, "list: Buffer Names");
    } else {
        sprintf(s, "Output Buffer Name"); // @out 0 @type symbol/list @digest Output buffer names(s)
                                            // @description Name of the output buffer
    }
}

void buf_join_inletinfo(t_buf_join *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_join *buf_join_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_join *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_join*)object_alloc_debug(s_tag_class);
    if (x) {
        x->xfade_type = EARS_FADE_SINE;
        x->xfade = 0;
        x->also_fade_first_and_last = true;
        
        earsbufobj_init((t_earsbufobj *)x, 0);

        // @arg 0 @name outname @optional 1 @type symbol
        // @digest Output buffer name
        // @description @copy EARS_DOC_OUTNAME_ATTR
        
        t_llll *args = llll_parse(true_ac, argv);
        t_llll *names = earsbufobj_extract_names_from_args((t_earsbufobj *)x, args);
        t_llllelem *cur = args ? args->l_head : NULL;
        if (cur) {
            // @arg 1 @name xfade_options @optional 1 @type list
            // @digest Crossfade options
            // @description If a single number is provided, it is the duration of the cross fade region, both at left and at right
            // of the junction point (unit depends on the <m>timeunit</m> attribute). <br />
            // If two numbers are provided, they are the duration of the left and right cross fade regions (units always depend
            // on the <m>timeunit</m> attribute). <br />
            if (cur && is_hatom_number(&cur->l_hatom)) {
                x->xfade = hatom_getdouble(&cur->l_hatom);
                cur = cur ? cur->l_next : NULL;
            }
        }
        
        attr_args_process(x, argc, argv);

        earsbufobj_setup((t_earsbufobj *)x, "E", "e", names);

        llll_free(args);
        llll_free(names);
    }
    return x;
}


void buf_join_free(t_buf_join *x)
{
    earsbufobj_free((t_earsbufobj *)x);
}



void buf_join_bang(t_buf_join *x)
{
    long num_buffers = ((t_earsbufobj *)x)->l_instore[0].num_stored_bufs;
    
    t_buffer_obj **inbufs = (t_buffer_obj **)bach_newptrclear(num_buffers * sizeof(t_buffer_obj *));
    long *xfade_samps = (long *)bach_newptrclear(num_buffers * sizeof(long));
    
    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_buffers, true);
    
    for (long count = 0; count < num_buffers; count++) {
        xfade_samps[count] = earsbufobj_time_to_samps((t_earsbufobj *)x, x->xfade, earsbufobj_get_stored_buffer_obj((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, count));
        inbufs[count] = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, count);
    }
    
    t_buffer_obj *out = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, 0);
    
    ears_buffer_concat((t_object *)x, inbufs, num_buffers, out, xfade_samps, x->also_fade_first_and_last, (e_ears_fade_types)x->xfade_type, x->xfade_curve, earsbufobj_get_slope_mapping((t_earsbufobj *)x));
    
    earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
    
    bach_freeptr(inbufs);
    bach_freeptr(xfade_samps);
    
}

void buf_join_anything(t_buf_join *x, t_symbol *msg, long ac, t_atom *av)
{
    long inlet = earsbufobj_proxy_getinlet((t_earsbufobj *) x);

    t_llll *parsed = earsbufobj_parse_gimme((t_earsbufobj *) x, LLLL_OBJ_VANILLA, msg, ac, av);
    if (!parsed) return;
    
    if (parsed && parsed->l_head) {
        if (inlet == 0) {
            long count = 0, num_buffers = parsed->l_size;
            
//            earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
            earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_buffers, true);
            
            for (t_llllelem *elem = parsed->l_head; elem; elem = elem->l_next, count++) {
                if (hatom_gettype(&elem->l_hatom) == H_SYM) {
                    t_symbol *buf = hatom_getsym(&elem->l_hatom);

                    // storing input buffer(s)
                    earsbufobj_store_buffer((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, count, buf);
                    
                    if (count == 0)
                        earsbufobj_store_copy_format((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, 0, EARSBUFOBJ_OUT, 0, 0);
                    
                }
            }
            
            buf_join_bang(x);
        }
    }
    llll_free(parsed);
}


