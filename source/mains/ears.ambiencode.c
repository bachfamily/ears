/**
	@file
	ears.ambiencode.c
 
	@name
	ears.ambiencode~
 
	@realname
	ears.ambiencode~
 
    @hiddenalias
    ears.ambiencode

	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	Encode 2D or 3D higher-order ambisonic buffers
 
	@description
	Pans the incoming buffers and encodes the result in higher-order ambisonic buffers
 
	@discussion
    The object uses the HOALibrary (https://github.com/CICM/HoaLibrary-Light)
    in turns using the Eigen library (http://eigen.tuxfamily.org)
    The normalization convention is SN2D or SN3D (depending on the dimension),
    the channel order is ACN.
 
	@category
	ears buffer operations
 
	@keywords
	buffer, ambisonic, encode, decode, HOA, 3D
 
	@seealso
	ears.ambidecode~
	
	@owner
	Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "llllobj.h"
#include "llll_commons_ext.h"
#include "bach_math_utilities.h"
#include "ears.object.h"
#include "ears.ambisonic.h"



typedef struct _buf_ambiencode {
    t_earsbufobj       e_ob;
    
    t_symbol           *dimension;
    long               order;
    
    t_llll             *azimuth;
    t_llll             *elevation;
    t_llll             *distance;
} t_buf_ambiencode;



// Prototypes
t_buf_ambiencode*         buf_ambiencode_new(t_symbol *s, short argc, t_atom *argv);
void			buf_ambiencode_free(t_buf_ambiencode *x);
void			buf_ambiencode_bang(t_buf_ambiencode *x);
void			buf_ambiencode_anything(t_buf_ambiencode *x, t_symbol *msg, long ac, t_atom *av);

void buf_ambiencode_assist(t_buf_ambiencode *x, void *b, long m, long a, char *s);
void buf_ambiencode_inletinfo(t_buf_ambiencode *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(ambiencode)

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
    
    CLASS_NEW_CHECK_SIZE(c, "ears.ambiencode~",
                         (method)buf_ambiencode_new,
                         (method)buf_ambiencode_free,
                         sizeof(t_buf_ambiencode),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    // @method list/llll @digest Function depends on inlet
    // @description A list or llll in the first inlet is supposed to contain buffer names and will
    // trigger the buffer processing and output the processed buffer names (depending on the <m>naming</m> attribute). <br />
    // A number or an llll in the second inlet is expected to contain a ambiencode parameter (depending on the <m>ampunit</m>) or
    // an envelope (also see <m>envampunit</m>).
    EARSBUFOBJ_DECLARE_COMMON_METHODS_DEFER(ambiencode)

    // @method number @digest Set ambiencode
    // @description A number in the second inlet sets the ambiencode parameter (depending on the <m>ampunit</m>).

    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_envtimeunit_attr(c);
    earsbufobj_class_add_naming_attr(c);

    
    CLASS_ATTR_SYM(c, "dimension", 0, t_buf_ambiencode, dimension);
    CLASS_ATTR_STYLE_LABEL(c,"dimension",0,"enum","Dimension");
    CLASS_ATTR_ENUM(c,"dimension", 0, "2D 3D");
    CLASS_ATTR_BASIC(c, "dimension", 0);
    // @description Sets the dimension
    
    CLASS_ATTR_LONG(c, "order", 0, t_buf_ambiencode, order);
    CLASS_ATTR_STYLE_LABEL(c,"order",0,"text","Order");
    CLASS_ATTR_BASIC(c, "order", 0);
    // @description Sets the order


    
    
    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
    return 0;
}

void buf_ambiencode_assist(t_buf_ambiencode *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        if (a == 0)
            sprintf(s, "symbol/list/llll: Incoming Buffer Names"); // @in 0 @type symbol/list/llll @digest Incoming buffer names
        else if (a == 1)
            sprintf(s, "number/llll: Azimuth"); // @in 1 @type number/llll @digest Azimuth
        else if (a == 2)
            sprintf(s, "number/llll: Elevation"); // @in 2 @type number/llll @digest Elevation
        else
            sprintf(s, "number/llll: Distance"); // @in 3 @type number/llll @digest Distance
    } else {
        sprintf(s, "symbol/list: Output Buffer Names"); // @out 0 @type symbol/list @digest Output buffer names
    }
}

void buf_ambiencode_inletinfo(t_buf_ambiencode *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_ambiencode *buf_ambiencode_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_ambiencode *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_ambiencode*)object_alloc_debug(s_tag_class);
    if (x) {
        x->dimension = gensym("3D");
        x->order = 1;
        x->azimuth = llll_from_text_buf("0.", false);
        x->elevation = llll_from_text_buf("0.", false);
        x->distance = llll_from_text_buf("1.", false);

        earsbufobj_init((t_earsbufobj *)x,  EARSBUFOBJ_FLAG_SUPPORTS_COPY_NAMES);
        
        // @arg 0 @name outnames @optional 1 @type symbol
        // @digest Output buffer names
        // @description @copy EARS_DOC_OUTNAME_ATTR

        // @arg 1 @name azimuth @optional 1 @type number/llll
        // @digest Azimuth
        // @description Sets the initial azimuth

        // @arg 1 @name elevation @optional 1 @type number/llll
        // @digest Elevation
        // @description Sets the initial elevation

        // @arg 1 @name distance @optional 1 @type number/llll
        // @digest Distance
        // @description Sets the initial distance


        t_llll *args = llll_parse(true_ac, argv);
        t_llll *names = earsbufobj_extract_names_from_args((t_earsbufobj *)x, args);
        
        if (args && args->l_head) {
            llll_clear(x->azimuth);
            llll_appendhatom_clone(x->azimuth, &args->l_head->l_hatom);
            if (args->l_head->l_next) {
                llll_clear(x->elevation);
                llll_appendhatom_clone(x->elevation, &args->l_head->l_next->l_hatom);
                if (args->l_head->l_next->l_next) {
                    llll_clear(x->distance);
                    llll_appendhatom_clone(x->distance, &args->l_head->l_next->l_next->l_hatom);
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


void buf_ambiencode_free(t_buf_ambiencode *x)
{
    llll_free(x->azimuth);
    llll_free(x->elevation);
    llll_free(x->distance);
    earsbufobj_free((t_earsbufobj *)x);
}


long buf_ambiencode_get_dimension_as_long(t_buf_ambiencode *x)
{
    if (x->dimension == gensym("2D"))
        return 2;
    if (x->dimension == gensym("3D"))
        return 3;
    return 0;
}

void buf_ambiencode_bang(t_buf_ambiencode *x)
{
    long num_buffers = ((t_earsbufobj *)x)->l_instore[0].num_stored_bufs;
    
    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_buffers, true);
    
    earsbufobj_mutex_lock((t_earsbufobj *)x);
    t_llllelem *azimuth_el = x->azimuth->l_head;
    t_llllelem *elevation_el = x->elevation->l_head;
    t_llllelem *distance_el = x->distance->l_head;
    for (long count = 0; count < num_buffers; count++,
         azimuth_el = azimuth_el && azimuth_el->l_next ? azimuth_el->l_next : azimuth_el,
         elevation_el = elevation_el && elevation_el->l_next ? elevation_el->l_next : elevation_el,
         distance_el = distance_el && distance_el->l_next ? distance_el->l_next : distance_el) {
        t_buffer_obj *in = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, count);
        t_buffer_obj *out = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, count);

        t_llll *azimuth_env = earsbufobj_llllelem_to_env_samples((t_earsbufobj *)x, azimuth_el, in);
        t_llll *elevation_env = earsbufobj_llllelem_to_env_samples((t_earsbufobj *)x, elevation_el, in);
        t_llll *distance_env = earsbufobj_llllelem_to_env_samples((t_earsbufobj *)x, distance_el, in);

        if (azimuth_env->l_size == 0) {
            object_error((t_object *)x, "No azimuth defined.");
        } else if (elevation_env->l_size == 0) {
            object_error((t_object *)x, "No elevation defined.");
        } else if (distance_env->l_size == 0) {
            object_error((t_object *)x, "No distance defined.");
        } else {
            ears_buffer_hoa_encode((t_object *)x, in, out, buf_ambiencode_get_dimension_as_long(x), x->order, azimuth_env, elevation_env, distance_env);
        }
        
        llll_free(azimuth_env);
        llll_free(elevation_env);
        llll_free(distance_env);
    }
    earsbufobj_mutex_unlock((t_earsbufobj *)x);
    
    earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
}


void buf_ambiencode_anything(t_buf_ambiencode *x, t_symbol *msg, long ac, t_atom *av)
{
    long inlet = proxy_getinlet((t_object *) x);

    t_llll *parsed = earsbufobj_parse_gimme((t_earsbufobj *) x, LLLL_OBJ_VANILLA, msg, ac, av);
    if (!parsed) return;
    
    if (parsed && parsed->l_head) {
        if (inlet == 0) {
            
            earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, parsed->l_size, true);
            earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_OUT, 0, parsed->l_size, true);
            
            earsbufobj_store_buffer_list((t_earsbufobj *)x, parsed, 0, true);
            
            buf_ambiencode_bang(x);
            
        } else if (inlet == 1) {
            earsbufobj_mutex_lock((t_earsbufobj *)x);
            llll_free(x->azimuth);
            x->azimuth = llll_clone(parsed);
            earsbufobj_mutex_unlock((t_earsbufobj *)x);
        } else if (inlet == 2) {
            earsbufobj_mutex_lock((t_earsbufobj *)x);
            llll_free(x->elevation);
            x->elevation = llll_clone(parsed);
            earsbufobj_mutex_unlock((t_earsbufobj *)x);
        } else if (inlet == 3) {
            earsbufobj_mutex_lock((t_earsbufobj *)x);
            llll_free(x->distance);
            x->distance = llll_clone(parsed);
            earsbufobj_mutex_unlock((t_earsbufobj *)x);
        }
    }
    llll_free(parsed);
}


