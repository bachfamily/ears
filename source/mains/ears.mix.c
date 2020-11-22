/**
	@file
	ears.mix.c
 
	@name
	ears.mix~
 
	@realname
	ears.mix~
 
	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	Mix buffers
 
	@description
	Superposes audio buffers temporally
 
	@discussion
 
	@category
	ears basic
 
	@keywords
	buffer, mix, superpose, merge
 
	@seealso
	ears.join~, ears.crop~, ears.read~, ears.fade~
	
	@owner
	Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "llllobj.h"
#include "llll_commons_ext.h"
#include "bach_math_utilities.h"
#include "ears.object.h"



typedef struct _buf_mix {
    t_earsbufobj       e_ob;
    
    char               normalization_mode;
    
/*    double             fade_left;
    double             fade_right;
    char               fade_amount_mode; /// see the enum above
    char               fade_type;
    double             fade_curve;
    */
    
    t_llll              *gains;
    t_llll              *offsets;
} t_buf_mix;



// Prototypes
t_buf_mix*     buf_mix_new(t_symbol *s, short argc, t_atom *argv);
void			buf_mix_free(t_buf_mix *x);
void			buf_mix_bang(t_buf_mix *x);
void			buf_mix_anything(t_buf_mix *x, t_symbol *msg, long ac, t_atom *av);

void buf_mix_assist(t_buf_mix *x, void *b, long m, long a, char *s);
void buf_mix_inletinfo(t_buf_mix *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(mix)

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
    
    CLASS_NEW_CHECK_SIZE(c, "ears.mix~",
                         (method)buf_mix_new,
                         (method)buf_mix_free,
                         sizeof(t_buf_mix),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    // @method list/llll @digest Process buffers
    // @description A list or llll with buffer names will trigger the buffer processing and output the processed
    // buffer names (depending on the <m>naming</m> attribute).
    EARSBUFOBJ_DECLARE_COMMON_METHODS_DEFER(mix)
    
    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_timeunit_attr(c);
    earsbufobj_class_add_ampunit_attr(c);
    earsbufobj_class_add_envampunit_attr(c);
    earsbufobj_class_add_envtimeunit_attr(c);
    earsbufobj_class_add_naming_attr(c);
    
    CLASS_ATTR_CHAR(c, "normalize", 0, t_buf_mix, normalization_mode);
    CLASS_ATTR_STYLE_LABEL(c,"normalize",0,"enumindex","Normalize Output");
    CLASS_ATTR_ENUMINDEX(c,"normalize", 0, "Never Always Overload Protection Only");
    CLASS_ATTR_BASIC(c, "normalize", 0);
    // @description Sets the normalization mode for the output buffer:
    // 0 = Never, does not normalize; 1 = Always, always normalizes to 1.; 2 = Overload Protection Only, only
    // normalizes to 1. if some samples exceed in modulo 1.

    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
    return 0;
}

void buf_mix_assist(t_buf_mix *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        if (a == 0) // @in 0 @type list @digest Names of the buffers to be mixed
            sprintf(s, "list: Buffer Names");
        else if (a == 1) // @in 1 @type number @digest Gains
            sprintf(s, "list/float: Gains");
        else if (a == 2) // @in 2 @type number @digest Offsets
            sprintf(s, "list/float: Offsets");

    } else {
        sprintf(s, "Output Buffer Name"); // @out 1 @type symbol/list @description Name of the output buffer
    }
}

void buf_mix_inletinfo(t_buf_mix *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_mix *buf_mix_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_mix *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_mix*)object_alloc_debug(s_tag_class);
    if (x) {
/*        x->xfade_type = EARS_FADE_SINE;
        x->xfade_left = x->xfade_right = 0;
        x->xfade_amount_mode = EARS_mix_XFADE_AMOUNT_MS;
*/
        
        x->gains = llll_get();
        x->offsets = llll_get();
        x->normalization_mode = EARS_NORMALIZE_OVERLOAD_PROTECTION_ONLY;
        
        earsbufobj_init((t_earsbufobj *)x, 0);
        

        // @arg 0 @name outname @optional 1 @type symbol
        // @digest Output buffer name
        // @description @copy EARS_DOC_OUTNAME_ATTR
        
        t_llll *args = llll_parse(true_ac, argv);
        t_llll *names = earsbufobj_extract_names_from_args((t_earsbufobj *)x, args);
        
        attr_args_process(x, argc, argv);
        
        earsbufobj_setup((t_earsbufobj *)x, "E44", "e", names);

        llll_free(args);
        llll_free(names);
    }
    return x;
}


void buf_mix_free(t_buf_mix *x)
{
    llll_free(x->gains);
    llll_free(x->offsets);
    earsbufobj_free((t_earsbufobj *)x);
}


void buf_mix_bang(t_buf_mix *x)
{
    long num_buffers = ((t_earsbufobj *)x)->l_instore[0].num_stored_bufs;
    t_llll *gains = llll_clone(x->gains);
    t_llll *offsets = llll_clone(x->offsets);
    char normalization_mode = x->normalization_mode;
    
    t_llll *gains_linear = llll_get();
    
    t_buffer_obj **inbufs = (t_buffer_obj **)bach_newptrclear(num_buffers * sizeof(t_buffer_obj *));
    long *offsets_samps = (long *)bach_newptrclear(num_buffers * sizeof(long));

    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_buffers, true);
    
    t_llllelem *gain_el, *offset_el;
    long count;
    double last_offset_samps = 0, last_diff_samps = 0;
    for (count = 0, gain_el = gains->l_head, offset_el = offsets->l_head; count < num_buffers;
         count++, gain_el = (gain_el && gain_el->l_next) ? gain_el->l_next : gain_el, offset_el = offset_el ? offset_el->l_next : NULL) {
        t_buffer_obj *buf = earsbufobj_get_stored_buffer_obj((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, count);
        if (gain_el)
            llll_appendllll(gains_linear, earsbufobj_llllelem_to_linear_and_samples((t_earsbufobj *)x, gain_el, buf));
        else
            llll_appenddouble(gains_linear, 1.);
        
        if (offset_el) {
            double this_offset_samps = earsbufobj_input_to_samps((t_earsbufobj *)x, hatom_getdouble(&offset_el->l_hatom), buf);
            offsets_samps[count] = MAX(0, this_offset_samps);
            
            last_diff_samps = this_offset_samps - last_offset_samps;
            last_offset_samps = this_offset_samps;
        } else {
            // padding with last difference, so that one can write just 0 1000 to mean 0 1000 2000 3000...
            offsets_samps[count] = MAX(0, last_offset_samps + last_diff_samps);
            last_offset_samps += last_diff_samps;
        }
        
        inbufs[count] = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, count);
    }
    
    t_buffer_obj *out = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, 0);
    
    ears_buffer_mix((t_object *)x, inbufs, num_buffers, out, gains_linear, offsets_samps, (e_ears_normalization_modes)normalization_mode);
    
    earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
    
    llll_free(gains);
    llll_free(gains_linear);
    llll_free(offsets);
    bach_freeptr(inbufs);
    bach_freeptr(offsets_samps);
    
}

void buf_mix_anything(t_buf_mix *x, t_symbol *msg, long ac, t_atom *av)
{
    long inlet = proxy_getinlet((t_object *) x);

    t_llll *parsed = earsbufobj_parse_gimme((t_earsbufobj *) x, LLLL_OBJ_VANILLA, msg, ac, av);
    if (!parsed) return;
    
    if (parsed && parsed->l_head) {
        if (inlet == 0) {
            long count = 0, num_buffers = parsed->l_size;
            
            earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
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
            
            buf_mix_bang(x);
        } else if (inlet == 1) {
            llll_free(x->gains);
            x->gains = llll_clone(parsed);
        } else {
            llll_free(x->offsets);
            x->offsets = llll_clone(parsed);
        }
    }
    llll_free(parsed);
}


