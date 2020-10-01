/**
	@file
	ears.hoarotate.c
 
	@name
	ears.hoarotate~
 
	@realname
	ears.hoarotate~
 
	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	Rotate higher-order ambisonic buffers
 
	@description
	Performs a rotation in the higher-order ambisonic domain
 
	@discussion
    The object uses the HOALibrary (https://github.com/CICM/HoaLibrary-Light)
    in turns using the Eigen library (http://eigen.tuxfamily.org)
    (the 3D rotation capability has been added to the HOALibrary by Daniele Ghisi,
    with the purpose of having this ears object).
    The normalization convention is SN2D or SN3D (depending on the dimension),
    the channel order is ACN.

	@category
	ears ambisonic
 
	@keywords
	buffer, ambisonic, rotate, hoa, 3d, rotation
 
	@seealso
	ears.hoa.encode~, ears.hoa.decode~
	
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



typedef struct _buf_hoarotate {
    t_earsbufobj       e_ob;
    
    t_symbol           *dimension;

    t_llll             *yaw;
    t_llll             *pitch;
    t_llll             *roll;
} t_buf_hoarotate;



// Prototypes
t_buf_hoarotate*         buf_hoarotate_new(t_symbol *s, short argc, t_atom *argv);
void			buf_hoarotate_free(t_buf_hoarotate *x);
void			buf_hoarotate_bang(t_buf_hoarotate *x);
void			buf_hoarotate_anything(t_buf_hoarotate *x, t_symbol *msg, long ac, t_atom *av);

void buf_hoarotate_assist(t_buf_hoarotate *x, void *b, long m, long a, char *s);
void buf_hoarotate_inletinfo(t_buf_hoarotate *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(hoarotate)



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
    
    CLASS_NEW_CHECK_SIZE(c, "ears.hoa.rotate~",
                         (method)buf_hoarotate_new,
                         (method)buf_hoarotate_free,
                         sizeof(t_buf_hoarotate),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    // @method list/llll @digest Function depends on inlet
    // @description A list or llll in the first inlet is supposed to contain buffer names and will
    // trigger the buffer processing and output the processed buffer names (depending on the <m>naming</m> attribute). <br />
    // A number or an llll in the second, third, or fourth inlet is expected to receive the yaw, pitch and roll parameters
    // or envelopes (also see <m>envampunit</m>).
    EARSBUFOBJ_DECLARE_COMMON_METHODS_DEFER(hoarotate)

    // @method number @digest Set yaw, pitch, roll
    // @description A number in the second, third or fourth inlet respectively sets the yaw, pitch and roll

    
    // @method quaternion @digest Define rotation from quaternion
    // @description A <m>quaternion</m> message, followed by four arguments (W, X, Y, Z) sets the rotation
    // from a quaternion (W being the real part).
    // @marg 0 @name w @optional 0 @type float
    // @marg 1 @name x @optional 0 @type float
    // @marg 2 @name y @optional 0 @type float
    // @marg 3 @name z @optional 0 @type float
    class_addmethod(c, (method)buf_hoarotate_anything_deferred, "quaternion", A_GIMME, 0);

    
    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_naming_attr(c);
    earsbufobj_class_add_timeunit_attr(c);
    earsbufobj_class_add_angleunit_attr(c);

    CLASS_ATTR_SYM(c, "dimension", 0, t_buf_hoarotate, dimension);
    CLASS_ATTR_STYLE_LABEL(c,"dimension",0,"enum","Dimension");
    CLASS_ATTR_ENUM(c,"dimension", 0, "2D 3D");
    CLASS_ATTR_BASIC(c, "dimension", 0);
    // @description Sets the dimension (order will then be inferred from the number of input channels).
    
    
    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
    return 0;
}

void buf_hoarotate_assist(t_buf_hoarotate *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        if (a == 0)
            sprintf(s, "symbol/list/llll: Incoming Buffer Names"); // @in 0 @type symbol/list/llll @digest Incoming buffer names
        else if (a == 1)
            sprintf(s, "number/llll: Yaw"); // @in 1 @type number/llll @digest Rotation angle around Z axis (yaw)
        else if (a == 2)
            sprintf(s, "number/llll: Pitch"); // @in 2 @type number/llll @digest Rotation angle around X axis (pitch)
        else if (a == 3)
            sprintf(s, "number/llll: Roll"); // @in 3 @type number/llll @digest Rotation angle around Y axis (roll)
    } else {
        sprintf(s, "symbol/list: Output Buffer Names"); // @out 0 @type symbol/list @digest Output buffer names
    }
}

void buf_hoarotate_inletinfo(t_buf_hoarotate *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_hoarotate *buf_hoarotate_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_hoarotate *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_hoarotate*)object_alloc_debug(s_tag_class);
    if (x) {
        x->dimension = gensym("3D");
        x->yaw = llll_from_text_buf("0");
        x->pitch = llll_from_text_buf("0");
        x->roll = llll_from_text_buf("0");
        
        earsbufobj_init((t_earsbufobj *)x, EARSBUFOBJ_FLAG_SUPPORTS_COPY_NAMES);
        
        // @arg 0 @name outnames @optional 1 @type symbol
        // @digest Output buffer names
        // @description @copy EARS_DOC_OUTNAME_ATTR

        t_llll *args = llll_parse(true_ac, argv);
        t_llll *names = earsbufobj_extract_names_from_args((t_earsbufobj *)x, args);
        
        attr_args_process(x, argc, argv);
        
        earsbufobj_setup((t_earsbufobj *)x, "E444", "E", names);

        llll_free(args);
        llll_free(names);
    }
    return x;
}


void buf_hoarotate_free(t_buf_hoarotate *x)
{
    llll_free(x->yaw);
    llll_free(x->pitch);
    llll_free(x->roll);
    earsbufobj_free((t_earsbufobj *)x);
}


long buf_hoarotate_get_dimension_as_long(t_buf_hoarotate *x)
{
    if (x->dimension == gensym("2D"))
        return 2;
    if (x->dimension == gensym("3D"))
        return 3;
    return 0;
}


void buf_hoarotate_bang(t_buf_hoarotate *x)
{
    long num_buffers = ((t_earsbufobj *)x)->l_instore[0].num_stored_bufs;
    
    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_buffers, true);
    
    earsbufobj_mutex_lock((t_earsbufobj *)x);
    t_llllelem *yaw_el = x->yaw->l_head;
    t_llllelem *pitch_el = x->pitch->l_head;
    t_llllelem *roll_el = x->roll->l_head;
    for (long count = 0; count < num_buffers; count++,
         yaw_el = yaw_el && yaw_el->l_next ? yaw_el->l_next : yaw_el,
         pitch_el = pitch_el && pitch_el->l_next ? pitch_el->l_next : pitch_el,
         roll_el = roll_el && roll_el->l_next ? roll_el->l_next : roll_el) {

        t_buffer_obj *in = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, count);
        t_buffer_obj *out = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, count);

        t_llll *yaw_env = earsbufobj_llllelem_to_env_samples((t_earsbufobj *)x, yaw_el, in);
        t_llll *pitch_env = earsbufobj_llllelem_to_env_samples((t_earsbufobj *)x, pitch_el, in);
        t_llll *roll_env = earsbufobj_llllelem_to_env_samples((t_earsbufobj *)x, roll_el, in);

        ears_llll_to_radians(yaw_env, x->e_ob.l_angleunit);
        ears_llll_to_radians(pitch_env, x->e_ob.l_angleunit);
        ears_llll_to_radians(roll_env, x->e_ob.l_angleunit);

        ears_buffer_hoa_rotate((t_object *)x, in, out, buf_hoarotate_get_dimension_as_long(x), yaw_env, pitch_env, roll_env);
        
        llll_free(yaw_env);
        llll_free(pitch_env);
        llll_free(roll_env);
    }
    earsbufobj_mutex_unlock((t_earsbufobj *)x);
    
    earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
}


void buf_hoarotate_anything(t_buf_hoarotate *x, t_symbol *msg, long ac, t_atom *av)
{
    long inlet = proxy_getinlet((t_object *) x);

    t_llll *parsed = earsbufobj_parse_gimme((t_earsbufobj *) x, LLLL_OBJ_VANILLA, msg, ac, av);
    if (!parsed) return;
    
    if (parsed && parsed->l_head) {
        if (hatom_gettype(&parsed->l_head->l_hatom) == H_SYM && hatom_getsym(&parsed->l_head->l_hatom) == gensym("quaternion")){
            if (parsed->l_size < 5) {
                object_error((t_object *)x, "Not enough arguments; quaterion is expected in the form W X Y Z");
            } else {
                double yaw = 0, pitch = 0, roll = 0;
                quaternion_to_yawpitchroll(hatom_getdouble(&parsed->l_head->l_hatom),
                                           hatom_getdouble(&parsed->l_head->l_next->l_hatom),
                                           hatom_getdouble(&parsed->l_head->l_next->l_next->l_hatom),
                                           hatom_getdouble(&parsed->l_head->l_next->l_next->l_next->l_hatom),
                                           &yaw, &pitch, &roll);

                earsbufobj_mutex_lock((t_earsbufobj *)x);
                llll_clear(x->yaw);
                llll_clear(x->pitch);
                llll_clear(x->roll);
                llll_appenddouble(x->yaw, yaw);
                llll_appenddouble(x->pitch, pitch);
                llll_appenddouble(x->roll, roll);
                earsbufobj_mutex_unlock((t_earsbufobj *)x);
            }
        } else {
            if (inlet == 0) {
                
                earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, parsed->l_size, true);
                earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_OUT, 0, parsed->l_size, true);
                
                earsbufobj_store_buffer_list((t_earsbufobj *)x, parsed, 0, true);
                
                buf_hoarotate_bang(x);
                
            } else if (inlet == 1) {
                earsbufobj_mutex_lock((t_earsbufobj *)x);
                llll_free(x->yaw);
                x->yaw = llll_clone(parsed);
                earsbufobj_mutex_unlock((t_earsbufobj *)x);
                
            } else if (inlet == 2) {
                earsbufobj_mutex_lock((t_earsbufobj *)x);
                llll_free(x->pitch);
                x->pitch = llll_clone(parsed);
                earsbufobj_mutex_unlock((t_earsbufobj *)x);
                
            } else if (inlet == 3) {
                earsbufobj_mutex_lock((t_earsbufobj *)x);
                llll_free(x->roll);
                x->roll = llll_clone(parsed);
                earsbufobj_mutex_unlock((t_earsbufobj *)x);
            }
        }
    }
    llll_free(parsed);
}



