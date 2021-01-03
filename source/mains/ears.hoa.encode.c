/**
	@file
	ears.hoa.encode.c
 
	@name
	ears.hoa.encode~
 
	@realname
	ears.hoa.encode~
 
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
	ears ambisonic
 
	@keywords
	buffer, ambisonic, encode, decode, HOA, 3D
 
	@seealso
	ears.hoa.decode~, ears.hoa.rotate~, ears.hoa.mirror~
	
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



typedef struct _buf_hoaencode {
    t_earsbufobj       e_ob;
    
    t_symbol           *dimension;
    long               order;
    t_symbol           *coordinate_type_sym;
    e_ears_coordinate_type coordinate_type;

    t_llll             *coord1;
    t_llll             *coord2;
    t_llll             *coord3;
} t_buf_hoaencode;



// Prototypes
t_buf_hoaencode*         buf_hoaencode_new(t_symbol *s, short argc, t_atom *argv);
void			buf_hoaencode_free(t_buf_hoaencode *x);
void			buf_hoaencode_bang(t_buf_hoaencode *x);
void			buf_hoaencode_anything(t_buf_hoaencode *x, t_symbol *msg, long ac, t_atom *av);

void buf_hoaencode_assist(t_buf_hoaencode *x, void *b, long m, long a, char *s);
void buf_hoaencode_inletinfo(t_buf_hoaencode *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(hoaencode)

t_max_err buf_hoaencode_setattr_coordtype(t_buf_hoaencode *x, void *attr, long argc, t_atom *argv)
{
    if (argc && argv) {
        if (atom_gettype(argv) == A_SYM) {
            x->coordinate_type_sym = atom_getsym(argv);
            if (x->coordinate_type_sym == gensym("xyz"))
                x->coordinate_type = EARS_COORDINATES_XYZ;
            else if (x->coordinate_type_sym == gensym("azr"))
                x->coordinate_type = EARS_COORDINATES_AZR;
            else
                x->coordinate_type = EARS_COORDINATES_AED;
        }
    }
    return MAX_ERR_NONE;
}


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
    
    CLASS_NEW_CHECK_SIZE(c, "ears.hoa.encode~",
                         (method)buf_hoaencode_new,
                         (method)buf_hoaencode_free,
                         sizeof(t_buf_hoaencode),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    // @method list/llll @digest Function depends on inlet
    // @description A list or llll in the first inlet is supposed to contain buffer names and will
    // trigger the buffer processing and output the processed buffer names (depending on the <m>naming</m> attribute). <br />
    // A number or an llll in the second inlet is expected to contain a hoaencode parameter (depending on the <m>ampunit</m>) or
    // an envelope (also see <m>envampunit</m>).
    EARSBUFOBJ_DECLARE_COMMON_METHODS_DEFER(hoaencode)

    // @method number @digest Set hoaencode
    // @description A number in the second inlet sets the hoaencode parameter (depending on the <m>ampunit</m>).

    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_envtimeunit_attr(c);
    earsbufobj_class_add_naming_attr(c);
    earsbufobj_class_add_angleunit_attr(c);

    
    CLASS_ATTR_SYM(c, "dimension", 0, t_buf_hoaencode, dimension);
    CLASS_ATTR_STYLE_LABEL(c,"dimension",0,"enum","Dimension");
    CLASS_ATTR_ENUM(c,"dimension", 0, "2D 3D");
    CLASS_ATTR_BASIC(c, "dimension", 0);
    // @description Sets the dimension
    
    CLASS_ATTR_LONG(c, "order", 0, t_buf_hoaencode, order);
    CLASS_ATTR_STYLE_LABEL(c,"order",0,"text","Order");
    CLASS_ATTR_BASIC(c, "order", 0);
    // @description Sets the order

    CLASS_ATTR_SYM(c, "coordtype", 0, t_buf_hoaencode, coordinate_type_sym);
    CLASS_ATTR_STYLE_LABEL(c,"coordtype",0,"enum","Coordinate Type");
    CLASS_ATTR_ENUM(c,"coordtype", 0, "aed xyz azr");
    CLASS_ATTR_ACCESSORS(c, "coordtype", NULL, buf_hoaencode_setattr_coordtype);
    CLASS_ATTR_BASIC(c, "coordtype", 0);
    // @description Sets the input coordinate type: <br />
    // - "aed": azimuth, Elevation, distance (spherical coordinates); <br />
    // - "xyz": cartesian coordinates; <br />
    // - "azr": azimuth, Z, axial radius (cylindrical coordinates). <br />
    // @copy EARS_DOC_COORDINATE_CONVENTION

    
    
    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
    return 0;
}

void buf_hoaencode_assist(t_buf_hoaencode *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        if (a == 0)
            sprintf(s, "symbol/list/llll: Incoming Buffer Names"); // @in 0 @type symbol/list/llll @digest Incoming buffer names
        else if (a == 1)
            sprintf(s, x->coordinate_type == EARS_COORDINATES_XYZ ?
                    "number/llll: X Coordinate" : "number/llll: Azimuth"); // @in 1 @type number/llll @digest Azimuth or X coordinate
        else if (a == 2)
            sprintf(s, x->coordinate_type == EARS_COORDINATES_XYZ ?
                    "number/llll: Y Coordinate" : (x->coordinate_type == EARS_COORDINATES_AZR ?
                    "number/llll: Z Coordinate" : "number/llll: Elevation")); // @in 2 @type number/llll @digest Elevation or Y coordinate
        else
            sprintf(s, x->coordinate_type == EARS_COORDINATES_XYZ ?
                    "number/llll: Z Coordinate" : (x->coordinate_type == EARS_COORDINATES_AZR ?
                    "number/llll: Axial radius" : "number/llll: Distance")); // @in 3 @type number/llll @digest Distance or Z coordinate
    } else {
        sprintf(s, "symbol/list: Output Buffer Names"); // @out 0 @type symbol/list @digest Output buffer names
    }
}

void buf_hoaencode_inletinfo(t_buf_hoaencode *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_hoaencode *buf_hoaencode_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_hoaencode *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_hoaencode*)object_alloc_debug(s_tag_class);
    if (x) {
        x->dimension = gensym("3D");
        x->order = 1;
        x->coordinate_type_sym = gensym("aed");
        x->coordinate_type = EARS_COORDINATES_AED;
        x->coord1 = llll_from_text_buf("0.", false);
        x->coord2 = llll_from_text_buf("0.", false);
        x->coord3 = llll_from_text_buf("1.", false);

        earsbufobj_init((t_earsbufobj *)x,  EARSBUFOBJ_FLAG_SUPPORTS_COPY_NAMES);
        
        // @arg 0 @name outnames @optional 1 @type symbol
        // @digest Output buffer names
        // @description @copy EARS_DOC_OUTNAME_ATTR

        // @arg 1 @name coordinates @optional 1 @type list/llll
        // @digest Initial Coordinates
        // @description Sets the initial coordinates, depending on the <m>coordtype</m> attribute (by default: azimuth, elevation, distance)

        t_llll *args = llll_parse(true_ac, argv);
        t_llll *names = earsbufobj_extract_names_from_args((t_earsbufobj *)x, args);
        
        if (args && args->l_head) {
            llll_clear(x->coord1);
            llll_appendhatom_clone(x->coord1, &args->l_head->l_hatom);
            if (args->l_head->l_next) {
                llll_clear(x->coord2);
                llll_appendhatom_clone(x->coord2, &args->l_head->l_next->l_hatom);
                if (args->l_head->l_next->l_next) {
                    llll_clear(x->coord3);
                    llll_appendhatom_clone(x->coord3, &args->l_head->l_next->l_next->l_hatom);
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


void buf_hoaencode_free(t_buf_hoaencode *x)
{
    llll_free(x->coord1);
    llll_free(x->coord2);
    llll_free(x->coord3);
    earsbufobj_free((t_earsbufobj *)x);
}


void buf_hoaencode_bang(t_buf_hoaencode *x)
{
    long num_buffers = ((t_earsbufobj *)x)->l_instore[0].num_stored_bufs;
    
    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_buffers, true);
    
    earsbufobj_mutex_lock((t_earsbufobj *)x);
    t_llllelem *coord1_el = x->coord1->l_head;
    t_llllelem *coord2_el = x->coord2->l_head;
    t_llllelem *coord3_el = x->coord3->l_head;
    for (long count = 0; count < num_buffers; count++,
         coord1_el = coord1_el && coord1_el->l_next ? coord1_el->l_next : coord1_el,
         coord2_el = coord2_el && coord2_el->l_next ? coord2_el->l_next : coord2_el,
         coord3_el = coord3_el && coord3_el->l_next ? coord3_el->l_next : coord3_el) {
        t_buffer_obj *in = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, count);
        t_buffer_obj *out = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, count);

        t_llll *coord1_env = earsbufobj_llllelem_to_env_samples((t_earsbufobj *)x, coord1_el, in);
        t_llll *coord2_env = earsbufobj_llllelem_to_env_samples((t_earsbufobj *)x, coord2_el, in);
        t_llll *coord3_env = earsbufobj_llllelem_to_env_samples((t_earsbufobj *)x, coord3_el, in);

        switch (x->coordinate_type) {
            case EARS_COORDINATES_AED:
                ears_llll_to_radians(coord1_env, x->e_ob.l_angleunit);
                ears_llll_to_radians(coord2_env, x->e_ob.l_angleunit);
                break;

            case EARS_COORDINATES_AZR:
                ears_llll_to_radians(coord1_env, x->e_ob.l_angleunit);
                break;

            default:
                break;
        }
        
        if (coord1_env->l_size == 0) {
            object_error((t_object *)x, x->coordinate_type == EARS_COORDINATES_XYZ ? "No X coordinate defined." : "No azimuth defined.");
        } else if (coord2_env->l_size == 0) {
            object_error((t_object *)x, x->coordinate_type == EARS_COORDINATES_XYZ ? "No Y coordinate defined." :
                         (x->coordinate_type == EARS_COORDINATES_AZR ? "No Z coordinate defined." : "No elevation defined."));
        } else if (coord3_env->l_size == 0) {
            object_error((t_object *)x, x->coordinate_type == EARS_COORDINATES_XYZ ? "No Z coordinate defined." :
                         (x->coordinate_type == EARS_COORDINATES_AZR ? "No axial radius defined." : "No distance defined."));
        } else {
            ears_buffer_hoa_encode((t_object *)x, in, out, ears_hoa_get_dimension_as_long(x->dimension), x->order,
                                   x->coordinate_type, coord1_env, coord2_env, coord3_env);
        }
        
        llll_free(coord1_env);
        llll_free(coord2_env);
        llll_free(coord3_env);
    }
    earsbufobj_mutex_unlock((t_earsbufobj *)x);
    
    earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
}


void buf_hoaencode_anything(t_buf_hoaencode *x, t_symbol *msg, long ac, t_atom *av)
{
    long inlet = earsbufobj_proxy_getinlet((t_earsbufobj *) x);

    t_llll *parsed = earsbufobj_parse_gimme((t_earsbufobj *) x, LLLL_OBJ_VANILLA, msg, ac, av);
    if (!parsed) return;
    
    if (parsed && parsed->l_head) {
        if (inlet == 0) {
            
            earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, parsed->l_size, true);
            earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_OUT, 0, parsed->l_size, true);
            
            earsbufobj_store_buffer_list((t_earsbufobj *)x, parsed, 0);
            
            buf_hoaencode_bang(x);
            
        } else if (inlet == 1) {
            earsbufobj_mutex_lock((t_earsbufobj *)x);
            llll_free(x->coord1);
            x->coord1 = llll_clone(parsed);
            earsbufobj_mutex_unlock((t_earsbufobj *)x);
        } else if (inlet == 2) {
            earsbufobj_mutex_lock((t_earsbufobj *)x);
            llll_free(x->coord2);
            x->coord2 = llll_clone(parsed);
            earsbufobj_mutex_unlock((t_earsbufobj *)x);
        } else if (inlet == 3) {
            earsbufobj_mutex_lock((t_earsbufobj *)x);
            llll_free(x->coord3);
            x->coord3 = llll_clone(parsed);
            earsbufobj_mutex_unlock((t_earsbufobj *)x);
        }
    }
    llll_free(parsed);
}


