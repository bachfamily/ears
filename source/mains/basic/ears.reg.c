/**
	@file
	ears.reg.c
 
	@name
	ears.reg~
 
	@realname
	ears.reg~
 
	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	Store or clone buffers
 
	@description
	Stores or clones buffers to be retrieved with a bang
 
	@discussion
 
	@category
	ears basic
 
	@keywords
	buffer, reg, store, keep
 
	@seealso
	ears.read~, ears.convert~
	
	@owner
	Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "llllobj.h"
#include "llll_commons_ext.h"
#include "bach_math_utilities.h"
#include "ears.object.h"

/*

#include <lame/lame.h> // only used to export mp3s
void ears_test_lame()
{
    lame_t lame = lame_init();
}

#include "mpg123.h"
void ears_test_mpg123()
{
    if (mpg123_init() != MPG123_OK)
        error("Error while loading mpg123 library.");
}
*/



typedef struct _buf_reg {
    t_earsbufobj       e_ob;
    
    double             level;
    char               rms_mode;
    
    long               embed;
    
    double             mix;
} t_buf_reg;



// Prototypes
t_buf_reg*         buf_reg_new(t_symbol *s, short argc, t_atom *argv);
void			buf_reg_free(t_buf_reg *x);
void			buf_reg_bang(t_buf_reg *x);
void			buf_reg_anything(t_buf_reg *x, t_symbol *msg, long ac, t_atom *av);

void buf_reg_assist(t_buf_reg *x, void *b, long m, long a, char *s);
void buf_reg_inletinfo(t_buf_reg *x, void *b, long a, char *t);
void buf_reg_appendtodictionary(t_buf_reg *x, t_dictionary *d);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;


EARSBUFOBJ_ADD_IO_METHODS(reg)

/**********************************************************************/
// Class Definition and Life Cycle

void C74_EXPORT ext_main(void* moduleRef)
{
//    ears_test_lame();
//    ears_test_mpg123();
    common_symbols_init();
    llllobj_common_symbols_init();
    
    if (llllobj_check_version(bach_get_current_llll_version()) || llllobj_test()) {
        ears_error_bachcheck();
        return 1;
    }
    
    t_class *c;
    
    CLASS_NEW_CHECK_SIZE(c, "ears.reg~",
                         (method)buf_reg_new,
                         (method)buf_reg_free,
                         sizeof(t_buf_reg),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    // @method list/llll @digest Store buffers
    // @description A list or llll with buffer names will be considered as the names of the buffers to be stored and output
    // (also according on the <m>naming</m> attribute).
    EARSBUFOBJ_DECLARE_COMMON_METHODS_HANDLETHREAD(reg) // TO DO: should we NOT defer this?

    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_blocking_attr(c);
    earsbufobj_class_add_naming_attr(c);

    earsbufobj_class_add_polyout_attr(c);

    class_addmethod(c, (method)buf_reg_appendtodictionary,    "appendtodictionary", A_CANT, 0);

    CLASS_ATTR_LONG(c, "embed",    0,    t_buf_reg, embed);
    CLASS_ATTR_FILTER_CLIP(c, "embed", 0, 1);
    CLASS_ATTR_LABEL(c, "embed", 0, "Save Buffer With Patcher");
    CLASS_ATTR_STYLE(c, "embed", 0, "onoff");
    CLASS_ATTR_SAVE(c, "embed", 0);
    CLASS_ATTR_BASIC(c, "embed", 0);
    // @description When set to 1, the stored buffer is saved with the patcher
    // and will be available next time the patch is loaded.
    
    
    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
    return 0;
}

void buf_reg_assist(t_buf_reg *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        if (a == 0)
            sprintf(s, "symbol/list/llll: Store Buffer Names And Output"); // @in 0 @type symbol/list/llll @digest Store buffer names and output
        else
            sprintf(s, "symbol/list/llll: Store Buffer Names Without Output"); // @in 0 @type symbol/list/llll @digest Store buffer names without output
    } else {
        sprintf(s, "symbol/list: Output Buffer Names"); // @out 0 @type symbol/list @digest Output buffer names
    }
}

void buf_reg_inletinfo(t_buf_reg *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


void buf_reg_appendtodictionary(t_buf_reg *x, t_dictionary *d)
{
    if (x->embed) {
        char entryname[2048];
        long buffer_count = earsbufobj_get_outstore_size((t_earsbufobj *)x, 0);
        dictionary_appendlong(d, gensym("reg_buffer_count"), buffer_count);
        for (long b = 0; b < buffer_count; b++) {
            t_buffer_obj *buf = earsbufobj_get_stored_buffer_obj((t_earsbufobj *)x, EARSBUFOBJ_OUT, 0, b);
            if (buf) {
                t_dictionary *subdict = dictionary_new();
                sprintf(entryname, "reg_buffer_%010ld", b);
                earsbufobj_store_buffer_in_dictionary((t_earsbufobj *)x, buf, subdict);
                dictionary_appenddictionary(d, gensym(entryname), (t_object *)subdict);
            }
        }
    }
}

void buf_reg_retrieve_buffers_from_dictionary(t_buf_reg *x)
{
    char entryname[2048];
    t_dictionary *d = (t_dictionary *)gensym("#D")->s_thing;
    if (d) {
        t_atom_long buffer_count = 0;
        t_max_err err = dictionary_getlong(d, gensym("reg_buffer_count"), &buffer_count);
        
        if (!err && buffer_count > 0) {
            earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_OUT, 0, buffer_count, true);
            
            for (long b = 0; b < buffer_count; b++) {
                t_buffer_obj *buf = earsbufobj_get_stored_buffer_obj((t_earsbufobj *)x, EARSBUFOBJ_OUT, 0, b);
                t_dictionary *subdict = NULL;
                sprintf(entryname, "reg_buffer_%010ld", b);
                dictionary_getdictionary(d, gensym(entryname), (t_object **)&subdict);
                if (subdict && buf) {
                    earsbufobj_retrieve_buffer_from_dictionary((t_earsbufobj *)x, subdict, buf);
                }
            }
        }
    }
}

t_buf_reg *buf_reg_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_reg *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_reg*)object_alloc_debug(s_tag_class);
    if (x) {
        x->rms_mode = 0;
        x->level = 1.;
        x->embed = 0;
        
        earsbufobj_init((t_earsbufobj *)x,  EARSBUFOBJ_FLAG_SUPPORTS_COPY_NAMES);
        
        // @arg 0 @name outnames @optional 1 @type symbol
        // @digest Output buffer names
        // @description @copy EARS_DOC_OUTNAME_ATTR

        t_llll *args = llll_parse(true_ac, argv);
        t_llll *names = earsbufobj_extract_names_from_args((t_earsbufobj *)x, args);
        
        attr_args_process(x, argc, argv);

        earsbufobj_setup((t_earsbufobj *)x, "EE", "E", names);
       
        buf_reg_retrieve_buffers_from_dictionary(x);
        
        llll_free(args);
        llll_free(names);
        
    }
    return x;
}


void buf_reg_free(t_buf_reg *x)
{
    earsbufobj_free((t_earsbufobj *)x);
}


void buf_reg_bang(t_buf_reg *x)
{
    long num_buffers = earsbufobj_get_instore_size((t_earsbufobj *)x, 0);

    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
    
    earsbufobj_mutex_lock((t_earsbufobj *)x);
    earsbufobj_init_progress((t_earsbufobj *)x, num_buffers);

    for (long count = 0; count < num_buffers; count++) {
        t_symbol *name = earsbufobj_get_inlet_buffer_name((t_earsbufobj *)x, 0, count);
        if (name) {
            earsbufobj_store_buffer((t_earsbufobj *)x, EARSBUFOBJ_OUT, 0, count, name);
        } else {
            // not a big deal: one may use [ears.reg~] just to create an empty buffer
        }

        if (earsbufobj_iter_progress((t_earsbufobj *)x, count, num_buffers)) break;
    }

    earsbufobj_mutex_unlock((t_earsbufobj *)x);

    earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
}


void buf_reg_anything(t_buf_reg *x, t_symbol *msg, long ac, t_atom *av)
{
    long inlet = earsbufobj_proxy_getinlet((t_earsbufobj *) x);

    t_llll *parsed = earsbufobj_parse_gimme((t_earsbufobj *) x, LLLL_OBJ_VANILLA, msg, ac, av);
    if (!parsed) return;
    
    if (parsed && parsed->l_head) {
        long num_bufs = llll_get_num_symbols_root(parsed);
        earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_bufs, true);
        earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_OUT, 0, num_bufs, true);
        
        earsbufobj_store_buffer_list((t_earsbufobj *)x, parsed, 0);
        
        if (inlet == 0)
            buf_reg_bang(x);
    }
    llll_free(parsed);
}


