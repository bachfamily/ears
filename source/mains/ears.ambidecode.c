/**
	@file
	ears.ambidecode.c
 
	@name
	ears.ambidecode~
 
	@realname
	ears.ambidecode~
 
	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	Decode higher-order ambisonic buffers
 
	@description
	Decodes higher-order ambisonic buffers for a set of loudspeakers or for binaural listening
 
	@discussion
    The object uses the HOALibrary (https://github.com/CICM/HoaLibrary-Light)
    in turns using the Eigen library (http://eigen.tuxfamily.org)
    The normalization convention is SN2D or SN3D (depending on the dimension),
    the channel order is ACN.

	@category
	ears buffer operations
 
	@keywords
	buffer, ambisonic, decode, binaural, hoa, 3d
 
	@seealso
	ears.ambiencode~
	
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



typedef struct _buf_ambidecode {
    t_earsbufobj       e_ob;
    
    t_symbol           *dimension;
    
    char               binaural;
    
    long               num_loudspeakers;
    t_atom_float       loudspeakers_azimuth[EARS_AMBISONIC_MAX_LOUDSPEAKERS];
    t_atom_float       loudspeakers_elevation[EARS_AMBISONIC_MAX_LOUDSPEAKERS];
} t_buf_ambidecode;



// Prototypes
t_buf_ambidecode*         buf_ambidecode_new(t_symbol *s, short argc, t_atom *argv);
void			buf_ambidecode_free(t_buf_ambidecode *x);
void			buf_ambidecode_bang(t_buf_ambidecode *x);
void			buf_ambidecode_anything(t_buf_ambidecode *x, t_symbol *msg, long ac, t_atom *av);

void buf_ambidecode_assist(t_buf_ambidecode *x, void *b, long m, long a, char *s);
void buf_ambidecode_inletinfo(t_buf_ambidecode *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(ambidecode)


t_max_err buf_ambidecode_setattr_binaural(t_buf_ambidecode *x, void *attr, long argc, t_atom *argv)
{
    if (argc && argv) {
        if (is_atom_number(argv)) {
            x->binaural = atom_getlong(argv);
            object_attr_setdisabled((t_object *)x, gensym("azimuth"), x->binaural);
            object_attr_setdisabled((t_object *)x, gensym("elevation"), x->binaural || x->dimension == gensym("2D"));
        }
    }
    return MAX_ERR_NONE;
}

t_max_err buf_ambidecode_setattr_dimension(t_buf_ambidecode *x, void *attr, long argc, t_atom *argv)
{
    if (argc && argv) {
        if (atom_gettype(argv) == A_SYM) {
            x->dimension = atom_getsym(argv);
            object_attr_setdisabled((t_object *)x, gensym("elevation"), x->binaural || x->dimension == gensym("2D"));
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
    
    CLASS_NEW_CHECK_SIZE(c, "ears.ambidecode~",
                         (method)buf_ambidecode_new,
                         (method)buf_ambidecode_free,
                         sizeof(t_buf_ambidecode),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    // @method list/llll @digest Function depends on inlet
    // @description A list or llll in the first inlet is supposed to contain buffer names and will
    // trigger the buffer processing and output the processed buffer names (depending on the <m>naming</m> attribute). <br />
    // A number or an llll in the second inlet is expected to contain a ambidecode parameter (depending on the <m>ampunit</m>) or
    // an envelope (also see <m>envampunit</m>).
    EARSBUFOBJ_DECLARE_COMMON_METHODS_DEFER(ambidecode)

    // @method number @digest Set ambidecode
    // @description A number in the second inlet sets the ambidecode parameter (depending on the <m>ampunit</m>).

    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_naming_attr(c);

    CLASS_ATTR_CHAR(c, "binaural", 0, t_buf_ambidecode, binaural);
    CLASS_ATTR_STYLE_LABEL(c,"binaural",0,"onoff","Binaural Decoding");
    CLASS_ATTR_ACCESSORS(c, "binaural", NULL, buf_ambidecode_setattr_binaural);
    CLASS_ATTR_BASIC(c, "binaural", 0);
    // @description Toggles the ability to decode into a binaural buffer

    CLASS_ATTR_SYM(c, "dimension", 0, t_buf_ambidecode, dimension);
    CLASS_ATTR_STYLE_LABEL(c,"dimension",0,"enum","Dimension");
    CLASS_ATTR_ACCESSORS(c, "dimension", NULL, buf_ambidecode_setattr_dimension);
    CLASS_ATTR_ENUM(c,"dimension", 0, "2D 3D");
    CLASS_ATTR_BASIC(c, "dimension", 0);
    // @description Sets the dimension (order will then be inferred from the number of input channels).
    
    
    CLASS_ATTR_DOUBLE_VARSIZE(c, "azimuth", 0, t_buf_ambidecode, loudspeakers_azimuth, num_loudspeakers, EARS_AMBISONIC_MAX_LOUDSPEAKERS);
    CLASS_ATTR_STYLE_LABEL(c,"azimuth",0,"text_large","Loudspeakers Azimuth");
    CLASS_ATTR_BASIC(c, "azimuth", 0);
    // @description Sets the azimuth of each loudspeaker
    
    CLASS_ATTR_DOUBLE_VARSIZE(c, "elevation", 0, t_buf_ambidecode, loudspeakers_elevation, num_loudspeakers, EARS_AMBISONIC_MAX_LOUDSPEAKERS);
    CLASS_ATTR_STYLE_LABEL(c,"elevation",0,"text_large","Loudspeakers Elevation");
    CLASS_ATTR_BASIC(c, "elevation", 0);
    // @description Sets the elevation of each loudspeaker

    
    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
    return 0;
}

void buf_ambidecode_assist(t_buf_ambidecode *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        if (a == 0)
            sprintf(s, "symbol/list/llll: Incoming Buffer Names"); // @in 0 @type symbol/list/llll @digest Incoming buffer names
    } else {
        sprintf(s, "symbol/list: Output Buffer Names"); // @out 0 @type symbol/list @digest Output buffer names
    }
}

void buf_ambidecode_inletinfo(t_buf_ambidecode *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_ambidecode *buf_ambidecode_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_ambidecode *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_ambidecode*)object_alloc_debug(s_tag_class);
    if (x) {
        x->dimension = gensym("3D");
        for (long i = 0; i < EARS_AMBISONIC_MAX_LOUDSPEAKERS; i++) {
            x->loudspeakers_azimuth[i] = 0;
            x->loudspeakers_elevation[i] = 0;
        }
        x->loudspeakers_azimuth[0] = -PIOVERTWO/2.;
        x->loudspeakers_azimuth[1] = PIOVERTWO/2.;
        x->loudspeakers_azimuth[2] = PIOVERTWO*3./2.;
        x->loudspeakers_azimuth[3] = PIOVERTWO*5./2.;
        x->num_loudspeakers = 4;

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


void buf_ambidecode_free(t_buf_ambidecode *x)
{
    earsbufobj_free((t_earsbufobj *)x);
}


long buf_ambidecode_get_dimension_as_long(t_buf_ambidecode *x)
{
    if (x->dimension == gensym("2D"))
        return 2;
    if (x->dimension == gensym("3D"))
        return 3;
    return 0;
}

void buf_ambidecode_bang(t_buf_ambidecode *x)
{
    long num_buffers = ((t_earsbufobj *)x)->l_instore[0].num_stored_bufs;
    
    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_buffers, true);
    
    earsbufobj_mutex_lock((t_earsbufobj *)x);
    for (long count = 0; count < num_buffers; count++) {
        t_buffer_obj *in = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, count);
        t_buffer_obj *out = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, count);

            if (x->binaural)
                ears_buffer_hoa_decode_binaural((t_object *)x, in, out, buf_ambidecode_get_dimension_as_long(x));
            else
                ears_buffer_hoa_decode((t_object *)x, in, out, buf_ambidecode_get_dimension_as_long(x), x->num_loudspeakers, x->loudspeakers_azimuth, x->loudspeakers_elevation);
    }
    earsbufobj_mutex_unlock((t_earsbufobj *)x);
    
    earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
}


void buf_ambidecode_anything(t_buf_ambidecode *x, t_symbol *msg, long ac, t_atom *av)
{
    long inlet = proxy_getinlet((t_object *) x);

    t_llll *parsed = earsbufobj_parse_gimme((t_earsbufobj *) x, LLLL_OBJ_VANILLA, msg, ac, av);
    if (!parsed) return;
    
    if (parsed && parsed->l_head) {
        if (inlet == 0) {
            
            earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, parsed->l_size, true);
            earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_OUT, 0, parsed->l_size, true);
            
            earsbufobj_store_buffer_list((t_earsbufobj *)x, parsed, 0, true);
            
            buf_ambidecode_bang(x);
            
        }
    }
    llll_free(parsed);
}



