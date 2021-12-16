/**
	@file
	ears.hoa.shift.c
 
	@name
	ears.hoa.shift~
 
	@realname
	ears.hoa.shift~
 
	@type
	object
 
	@module
	ears
 
    @status
    experimental
 
	@author
	Daniele Ghisi
 
	@digest
	Shift higher-order ambisonic buffers
 
	@description
	Performs a translation in the higher-order ambisonic domain
 
	@discussion
    Experimental, doesn't work!!!
    I've tried to add translational capabilities to the HOA library, without success
    The normalization convention is SN2D or SN3D (depending on the dimension),
    the channel order is ACN.

	@category
	ears ambisonic
 
	@keywords
	buffer, ambisonic, shift, translation, hoa, 3d, move
 
	@seealso
	ears.hoa.encode~, ears.hoa.decode~, ears.hoa.mirror~, rotate~
	
	@owner
	Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "llllobj.h"
#include "llll_commons_ext.h"
#include "bach_math_utilities.h"
#include "ears.object.h"
#include "ears.hoa.h"



typedef struct _buf_hoashift {
    t_earsbufobj       e_ob;
    
    t_symbol           *dimension;

    t_llll             *delta_x;
    t_llll             *delta_y;
    t_llll             *delta_z;
} t_buf_hoashift;



// Prototypes
t_buf_hoashift*         buf_hoashift_new(t_symbol *s, short argc, t_atom *argv);
void			buf_hoashift_free(t_buf_hoashift *x);
void			buf_hoashift_bang(t_buf_hoashift *x);
void			buf_hoashift_anything(t_buf_hoashift *x, t_symbol *msg, long ac, t_atom *av);

void buf_hoashift_assist(t_buf_hoashift *x, void *b, long m, long a, char *s);
void buf_hoashift_inletinfo(t_buf_hoashift *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(hoashift)



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
    
    CLASS_NEW_CHECK_SIZE(c, "ears.hoa.shift~",
                         (method)buf_hoashift_new,
                         (method)buf_hoashift_free,
                         sizeof(t_buf_hoashift),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    // @method list/llll @digest Function depends on inlet
    // @description A list or llll in the first inlet is supposed to contain buffer names and will
    // trigger the buffer processing and output the processed buffer names (depending on the <m>naming</m> attribute). <br />
    // A number or an llll in the second, third, or fourth inlet is expected to receive the yaw, pitch and roll parameters
    // or envelopes (also see <m>envampunit</m>).
    EARSBUFOBJ_DECLARE_COMMON_METHODS_HANDLETHREAD(hoashift)

    // @method number @digest Set shift amount
    // @description A number in the second, third or fourth inlet respectively sets the shift amount in the X, Y or Z coordinate

    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_naming_attr(c);
    earsbufobj_class_add_timeunit_attr(c);
    earsbufobj_class_add_angleunit_attr(c);
    earsbufobj_class_add_slopemapping_attr(c);

    CLASS_ATTR_SYM(c, "dimension", 0, t_buf_hoashift, dimension);
    CLASS_ATTR_STYLE_LABEL(c,"dimension",0,"enum","Dimension");
    CLASS_ATTR_ENUM(c,"dimension", 0, "2D 3D");
    CLASS_ATTR_BASIC(c, "dimension", 0);
    // @description Sets the dimension (order will then be inferred from the number of input channels).
    
    
    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
    return 0;
}

void buf_hoashift_assist(t_buf_hoashift *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        if (a == 0)
            sprintf(s, "symbol/list/llll: Incoming Buffer Names"); // @in 0 @type symbol/list/llll @digest Incoming buffer names
        else if (a == 1)
            sprintf(s, "number/llll: Delta X"); // @in 1 @type number/llll @digest Shift in X direction
        else if (a == 2)
            sprintf(s, "number/llll: Delta Y"); // @in 2 @type number/llll @digest Shift in Y direction
        else if (a == 3)
            sprintf(s, "number/llll: Delta Z"); // @in 3 @type number/llll @digest Shift in Z direction
    } else {
        sprintf(s, "symbol/list: Output Buffer Names"); // @out 0 @type symbol/list @digest Output buffer names
    }
}

void buf_hoashift_inletinfo(t_buf_hoashift *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_hoashift *buf_hoashift_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_hoashift *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_hoashift*)object_alloc_debug(s_tag_class);
    if (x) {
        x->dimension = gensym("3D");
        x->delta_x = llll_from_text_buf("0");
        x->delta_y = llll_from_text_buf("0");
        x->delta_z = llll_from_text_buf("0");
        
        earsbufobj_init((t_earsbufobj *)x, EARSBUFOBJ_FLAG_SUPPORTS_COPY_NAMES);
        
        // @arg 0 @name outnames @optional 1 @type symbol
        // @digest Output buffer names
        // @description @copy EARS_DOC_OUTNAME_ATTR

        t_llll *args = llll_parse(true_ac, argv);
        t_llll *names = earsbufobj_extract_names_from_args((t_earsbufobj *)x, args);
        
        if (args && args->l_head) {
            llll_clear(x->delta_x);
            llll_appendhatom_clone(x->delta_x, &args->l_head->l_hatom);
            if (args->l_head->l_next) {
                llll_clear(x->delta_y);
                llll_appendhatom_clone(x->delta_y, &args->l_head->l_next->l_hatom);
                if (args->l_head->l_next->l_next) {
                    llll_clear(x->delta_z);
                    llll_appendhatom_clone(x->delta_z, &args->l_head->l_next->l_next->l_hatom);
                }
            }
        }
        
        attr_args_process(x, argc, argv);
        
        earsbufobj_setup((t_earsbufobj *)x, "E444", "E", names);

        llll_free(args);
        llll_free(names);
    }
    return x;
}


void buf_hoashift_free(t_buf_hoashift *x)
{
    llll_free(x->delta_z);
    llll_free(x->delta_y);
    llll_free(x->delta_x);
    earsbufobj_free((t_earsbufobj *)x);
}




void buf_hoashift_bang(t_buf_hoashift *x)
{
    long num_buffers = earsbufobj_get_instore_size((t_earsbufobj *)x, 0);
    
    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_buffers, true);
    
    earsbufobj_mutex_lock((t_earsbufobj *)x);
    t_llllelem *delta_x_el = x->delta_x->l_head;
    t_llllelem *delta_y_el = x->delta_y->l_head;
    t_llllelem *delta_z_el = x->delta_z->l_head;
    for (long count = 0; count < num_buffers; count++,
         delta_x_el = delta_x_el && delta_x_el->l_next ? delta_x_el->l_next : delta_x_el,
         delta_y_el = delta_y_el && delta_y_el->l_next ? delta_y_el->l_next : delta_y_el,
         delta_z_el = delta_z_el && delta_z_el->l_next ? delta_z_el->l_next : delta_z_el) {

        t_buffer_obj *in = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, count);
        t_buffer_obj *out = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, count);

        t_llll *delta_x_env = earsbufobj_llllelem_to_env_samples((t_earsbufobj *)x, delta_x_el, in);
        t_llll *delta_y_env = earsbufobj_llllelem_to_env_samples((t_earsbufobj *)x, delta_y_el, in);
        t_llll *delta_z_env = earsbufobj_llllelem_to_env_samples((t_earsbufobj *)x, delta_z_el, in);

        ears_buffer_hoa_shift((t_object *)x, in, out, ears_hoa_get_dimension_as_long(x->dimension), delta_x_env, delta_y_env, delta_z_env, earsbufobj_get_slope_mapping((t_earsbufobj *)x));
        
        llll_free(delta_x_env);
        llll_free(delta_y_env);
        llll_free(delta_z_env);
    }
    earsbufobj_mutex_unlock((t_earsbufobj *)x);
    
    earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
}


void buf_hoashift_anything(t_buf_hoashift *x, t_symbol *msg, long ac, t_atom *av)
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
            
            buf_hoashift_bang(x);
            
        } else if (inlet == 1) {
            earsbufobj_mutex_lock((t_earsbufobj *)x);
            llll_free(x->delta_x);
            x->delta_x = llll_clone(parsed);
            earsbufobj_mutex_unlock((t_earsbufobj *)x);
            
        } else if (inlet == 2) {
            earsbufobj_mutex_lock((t_earsbufobj *)x);
            llll_free(x->delta_y);
            x->delta_y = llll_clone(parsed);
            earsbufobj_mutex_unlock((t_earsbufobj *)x);
            
        } else if (inlet == 3) {
            earsbufobj_mutex_lock((t_earsbufobj *)x);
            llll_free(x->delta_z);
            x->delta_z = llll_clone(parsed);
            earsbufobj_mutex_unlock((t_earsbufobj *)x);
        }
    }
    llll_free(parsed);
}



