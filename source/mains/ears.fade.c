/**
	@file
	ears.fade.c
 
	@name
	ears.fade~
 
	@realname
	ears.fade~
 
	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	Apply fades
 
	@description
	Apply fade in and/or fade out to a buffer
 
	@discussion
 
	@category
	ears basic
 
	@keywords
	buffer, fade, fadein, fadeout, in, out
 
	@seealso
	ears.join~
	
	@owner
	Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "llllobj.h"
#include "llll_commons_ext.h"
#include "bach_math_utilities.h"
#include "ears.object.h"

typedef struct _buf_fade {
    t_earsbufobj       e_ob;
    double             fadein;
    double             fadeout;
    
    char               fadein_type;
    char               fadeout_type;
    
    double             fadein_curve;
    double             fadeout_curve;
    
} t_buf_fade;



// Prototypes
t_buf_fade*     buf_fade_new(t_symbol *s, short argc, t_atom *argv);
void			buf_fade_free(t_buf_fade *x);
void			buf_fade_bang(t_buf_fade *x);
void			buf_fade_anything(t_buf_fade *x, t_symbol *msg, long ac, t_atom *av);

void buf_fade_assist(t_buf_fade *x, void *b, long m, long a, char *s);
void buf_fade_inletinfo(t_buf_fade *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(fade)

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
    
    CLASS_NEW_CHECK_SIZE(c, "ears.fade~",
                         (method)buf_fade_new,
                         (method)buf_fade_free,
                         sizeof(t_buf_fade),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    
    // @method list/llll @digest Process buffers
    // @description A list or llll with buffer names will trigger the buffer processing and output the processed
    // buffer names (depending on the <m>naming</m> attribute).
    EARSBUFOBJ_DECLARE_COMMON_METHODS_DEFER(fade)
    
    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_timeunit_attr(c);
    earsbufobj_class_add_naming_attr(c);
    earsbufobj_class_add_slopemapping_attr(c);

    
    CLASS_ATTR_CHAR(c, "fadeintype", 0, t_buf_fade, fadein_type);
    CLASS_ATTR_STYLE_LABEL(c,"fadeintype",0,"enumindex","Fade In Type");
    CLASS_ATTR_ENUMINDEX(c,"fadeintype", 0, "None Linear Sine Curve S-Curve");
    // @description Sets the fade in type: 0 = None, 1 = Linear (default), 2 = Sine, 3 = Curve, 4 = S-Curve;
    
    CLASS_ATTR_CHAR(c, "fadeouttype", 0, t_buf_fade, fadeout_type);
    CLASS_ATTR_STYLE_LABEL(c,"fadeouttype",0,"enumindex","Fade Out Type");
    CLASS_ATTR_ENUMINDEX(c,"fadeouttype", 0, "None Linear Sine Curve S-Curve");
    // @description Sets the fade in type: 0 = None, 1 = Linear (default), 2 = Sine, 3 = Curve, 4 = S-Curve;

    
    CLASS_ATTR_DOUBLE(c, "fadeincurve", 0, t_buf_fade, fadein_curve);
    CLASS_ATTR_STYLE_LABEL(c,"fadeincurve",0,"text","Fade In Curve");
    // @description Sets the curve parameter for the fade in (for fades of type Curve and S-Curve only).
    // The parameters goes from -1 to 1, 0 being linear (default).
    
    CLASS_ATTR_DOUBLE(c, "fadeoutcurve", 0, t_buf_fade, fadeout_curve);
    CLASS_ATTR_STYLE_LABEL(c,"fadeoutcurve",0,"text","Fade Out Curve");
    // @description Sets the curve parameter for the fade out (for fades of type Curve and S-Curve only).
    // The parameters goes from -1 to 1, 0 being linear (default).
    
    
    
    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
    return 0;
}

void buf_fade_assist(t_buf_fade *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        if (a == 0) // @in 0 @type symbol @digest Buffer name
            sprintf(s, "symbol: Buffer Names");
        else if (a == 1) // @out 1 @type number @digest Fade in duration
            sprintf(s, "float: Fade In Duration"); // @description Duration of the fade in
        else if (a == 2) // @out 2 @type number @digest Fade out duration
            sprintf(s, "float: Fade Out Duration"); // @description Duration of the fade out
    } else {
        sprintf(s, "Output Buffer Names"); // @out 0 @type symbol/list @digest Output buffer names(s)
                                            // @description Name of the output buffer
    }
}

void buf_fade_inletinfo(t_buf_fade *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_fade *buf_fade_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_fade *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_fade*)object_alloc_debug(s_tag_class);
    if (x) {
        x->fadein_type = x->fadeout_type = EARS_FADE_LINEAR;
        
        earsbufobj_init((t_earsbufobj *)x,  EARSBUFOBJ_FLAG_SUPPORTS_COPY_NAMES);

        // @arg 0 @name outname @optional 1 @type symbol
        // @digest Output buffer name
        // @description @copy EARS_DOC_OUTNAME_ATTR
        
        t_llll *args = llll_parse(true_ac, argv);
        t_llll *names = earsbufobj_extract_names_from_args((t_earsbufobj *)x, args);
        t_llllelem *cur = args ? args->l_head : NULL;
        if (cur) {
            // @arg 1 @name fadein @optional 1 @type number
            // @digest Fade in duration
            // @description Duration of the fade in region (in milliseconds or in samples, depending on the <m>timeunit</m> attribute).
            if (cur && is_hatom_number(&cur->l_hatom)) {
                x->fadein = x->fadeout = hatom_getdouble(&cur->l_hatom);
                cur = cur ? cur->l_next : NULL;
            }

            // @arg 2 @name fadeout @optional 1 @type number
            // @digest Fade out duration
            // @description Duration of the fade out region (in milliseconds or in samples, depending on the <m>timeunit</m> attribute).
            if (cur && is_hatom_number(&cur->l_hatom))
                x->fadeout = hatom_getdouble(&cur->l_hatom);
        }
        
        attr_args_process(x, argc, argv);

        earsbufobj_setup((t_earsbufobj *)x, "Eff", "E", names);

        llll_free(args);
        llll_free(names);
    }
    return x;
}


void buf_fade_free(t_buf_fade *x)
{
    earsbufobj_free((t_earsbufobj *)x);
}



void buf_fade_bang(t_buf_fade *x)
{
    long num_buffers = ((t_earsbufobj *)x)->l_instore[0].num_stored_bufs;
    
    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_buffers, true);
    
    for (long count = 0; count < num_buffers; count++) {
        t_buffer_obj *in = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, count);
        t_buffer_obj *out = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, count);
        
        long fadein = earsbufobj_time_to_samps((t_earsbufobj *)x, x->fadein, in);
        long fadeout = earsbufobj_time_to_samps((t_earsbufobj *)x, x->fadeout, in);
        
        ears_buffer_fade((t_object *)x, in, out, fadein, fadeout, (e_ears_fade_types)x->fadein_type, (e_ears_fade_types)x->fadeout_type, x->fadein_curve, x->fadeout_curve, earsbufobj_get_slope_mapping((t_earsbufobj *)x));
    }
    
    earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
}



void buf_fade_anything(t_buf_fade *x, t_symbol *msg, long ac, t_atom *av)
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
                
                buf_fade_bang(x);
            }
        } else if (inlet == 1) {
            if (is_hatom_number(&parsed->l_head->l_hatom))
                x->fadein = hatom_getdouble(&parsed->l_head->l_hatom);
        } else {
            if (is_hatom_number(&parsed->l_head->l_hatom))
                x->fadeout = hatom_getdouble(&parsed->l_head->l_hatom);
        }
    }
    llll_free(parsed);
}


