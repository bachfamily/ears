/**
	@file
	ears.hoa.mirror.c
 
	@name
	ears.hoa.mirror~
 
	@realname
	ears.hoa.mirror~
 
	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	Mirror higher-order ambisonic buffers
 
	@description
	Performs a mirroring in the higher-order ambisonic domain
 
	@discussion
    The normalization convention is SN2D or SN3D (depending on the dimension),
    the channel order is ACN.

	@category
	ears ambisonic
 
	@keywords
	buffer, ambisonic, mirror, hoa, 3d, invert, swap, transform
 
	@seealso
	ears.hoa.encode~, ears.hoa.decode~, ears.hoa.rotate~
	
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



typedef struct _buf_hoamirror {
    t_earsbufobj       e_ob;
    
    t_symbol           *dimension;
    long               axis;
} t_buf_hoamirror;



// Prototypes
t_buf_hoamirror*         buf_hoamirror_new(t_symbol *s, short argc, t_atom *argv);
void			buf_hoamirror_free(t_buf_hoamirror *x);
void			buf_hoamirror_bang(t_buf_hoamirror *x);
void			buf_hoamirror_anything(t_buf_hoamirror *x, t_symbol *msg, long ac, t_atom *av);

void buf_hoamirror_assist(t_buf_hoamirror *x, void *b, long m, long a, char *s);
void buf_hoamirror_inletinfo(t_buf_hoamirror *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(hoamirror)


t_max_err buf_hoamirror_setattr_axis(t_buf_hoamirror *x, void *attr, long argc, t_atom *argv)
{
    if (argc && argv) {
        if (atom_gettype(argv) == A_LONG) {
            x->axis = atom_getlong(argv);
        } else if (atom_gettype(argv) == A_SYM) {
            t_symbol *s = atom_getsym(argv);
            if (s == gensym("x") || s == gensym("X"))
                x->axis = 1;
            else if (s == gensym("y") || s == gensym("Y"))
                x->axis = 2;
            else if (s == gensym("z") || s == gensym("Z"))
                x->axis = 3;
            else if (s == gensym("none") || s == gensym("0"))
                x->axis = 0;
            else
                object_error((t_object *)x, "Unknown axis!");
        } else {
            object_error((t_object *)x, "Wrong axis format.");
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
    
    CLASS_NEW_CHECK_SIZE(c, "ears.hoa.mirror~",
                         (method)buf_hoamirror_new,
                         (method)buf_hoamirror_free,
                         sizeof(t_buf_hoamirror),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    // @method list/llll @digest Function depends on inlet
    // @description A list or llll in the first inlet is supposed to contain buffer names and will
    // trigger the buffer processing and output the processed buffer names (depending on the <m>naming</m> attribute). <br />
    // A number or an llll in the second, third, or fourth inlet is expected to receive the yaw, pitch and roll parameters
    // or envelopes (also see <m>envampunit</m>).
    EARSBUFOBJ_DECLARE_COMMON_METHODS_DEFER(hoamirror)

    // @method number @digest Set yaw, pitch, roll
    // @description A number in the second, third or fourth inlet respectively sets the yaw, pitch and roll
    
    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_naming_attr(c);

    CLASS_ATTR_SYM(c, "dimension", 0, t_buf_hoamirror, dimension);
    CLASS_ATTR_STYLE_LABEL(c,"dimension",0,"enum","Dimension");
    CLASS_ATTR_ENUM(c,"dimension", 0, "2D 3D");
    CLASS_ATTR_BASIC(c, "dimension", 0);
    // @description Sets the dimension (order will then be inferred from the number of input channels).

    CLASS_ATTR_LONG(c, "axis", 0, t_buf_hoamirror, axis);
    CLASS_ATTR_STYLE_LABEL(c,"axis",0,"enum","Mirroring Axis");
    CLASS_ATTR_ENUMINDEX(c,"axis", 0, "None X Y Z");
    CLASS_ATTR_ACCESSORS(c, "axis", NULL, buf_hoamirror_setattr_axis);
    CLASS_ATTR_BASIC(c, "axis", 0);
    // @description Sets the mirroring axis.
    // None = no mirroring; X mirrors left-right; Y mirrors front-back; Z mirrors top-bottom.

    
    
    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
    return 0;
}

void buf_hoamirror_assist(t_buf_hoamirror *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        if (a == 0)
            sprintf(s, "symbol/list/llll: Incoming Buffer Names"); // @in 0 @type symbol/list/llll @digest Incoming buffer names
    } else {
        sprintf(s, "symbol/list: Output Buffer Names"); // @out 0 @type symbol/list @digest Output buffer names
    }
}

void buf_hoamirror_inletinfo(t_buf_hoamirror *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_hoamirror *buf_hoamirror_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_hoamirror *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_hoamirror*)object_alloc_debug(s_tag_class);
    if (x) {
        x->dimension = gensym("3D");
        x->axis = 0;
        
        earsbufobj_init((t_earsbufobj *)x, EARSBUFOBJ_FLAG_SUPPORTS_COPY_NAMES);
        
        // @arg 0 @name outnames @optional 1 @type symbol
        // @digest Output buffer names
        // @description @copy EARS_DOC_OUTNAME_ATTR

        t_llll *args = llll_parse(true_ac, argv);
        t_llll *names = earsbufobj_extract_names_from_args((t_earsbufobj *)x, args);
        
        attr_args_process(x, argc, argv);
        
        earsbufobj_setup((t_earsbufobj *)x, "E", "E", names);

        llll_free(args);
        llll_free(names);
    }
    return x;
}


void buf_hoamirror_free(t_buf_hoamirror *x)
{
    earsbufobj_free((t_earsbufobj *)x);
}



void buf_hoamirror_bang(t_buf_hoamirror *x)
{
    long num_buffers = ((t_earsbufobj *)x)->l_instore[0].num_stored_bufs;
    
    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_buffers, true);
    
    earsbufobj_mutex_lock((t_earsbufobj *)x);
    for (long count = 0; count < num_buffers; count++) {
        t_buffer_obj *in = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, count);
        t_buffer_obj *out = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, count);

        ears_buffer_hoa_mirror((t_object *)x, in, out, ears_hoa_get_dimension_as_long(x->dimension), x->axis);
    }
    earsbufobj_mutex_unlock((t_earsbufobj *)x);
    
    earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
}


void buf_hoamirror_anything(t_buf_hoamirror *x, t_symbol *msg, long ac, t_atom *av)
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
            
            buf_hoamirror_bang(x);
        }
    }
    llll_free(parsed);
}



