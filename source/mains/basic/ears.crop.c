/**
	@file
	ears.crop.c
 
	@name
	ears.crop~
 
	@realname
	ears.crop~
 
	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	Crop a buffer
 
	@description
	Extracts a portion of a buffer
 
	@discussion
 
	@category
	ears basic
 
	@keywords
	buffer, crop, cut, extract, portion, subbuffer
 
	@seealso
	ears.trim~, ears.fade~, ears.repeat~
	
	@owner
	Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "llllobj.h"
#include "llll_commons_ext.h"
#include "bach_math_utilities.h"
#include "ears.object.h"
#include "ears.commons.h"

typedef struct _buf_crop {
    t_earsbufobj       e_ob;
    t_llll             *from;
    t_llll             *to;
    
} t_buf_crop;



// Prototypes
t_buf_crop*         buf_crop_new(t_symbol *s, short argc, t_atom *argv);
void			buf_crop_free(t_buf_crop *x);
void			buf_crop_bang(t_buf_crop *x);
void			buf_crop_anything(t_buf_crop *x, t_symbol *msg, long ac, t_atom *av);

void buf_crop_assist(t_buf_crop *x, void *b, long m, long a, char *s);
void buf_crop_inletinfo(t_buf_crop *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(crop)



/**********************************************************************/
// Class Definition and Life Cycle

void C74_EXPORT ext_main(void* moduleRef)
{
    common_symbols_init();
    llllobj_common_symbols_init();
    
    if (llllobj_check_version(bach_get_current_llll_version()) || llllobj_test()) {
        ears_error_bachcheck();
        return 1;
    }
    
    t_class *c;
    
    CLASS_NEW_CHECK_SIZE(c, "ears.crop~",
                         (method)buf_crop_new,
                         (method)buf_crop_free,
                         sizeof(t_buf_crop),
                         (method)NULL,
                         A_GIMME,
                         0L);

    // @method list/llll @digest Process buffers
    // @description A list or llll with buffer names will trigger the buffer processing and output the processed
    // buffer names (depending on the <m>naming</m> attribute).
    EARSBUFOBJ_DECLARE_COMMON_METHODS_HANDLETHREAD(crop)
    
    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_blocking_attr(c);
    earsbufobj_class_add_timeunit_attr(c);
    earsbufobj_class_add_naming_attr(c);
    earsbufobj_class_add_polyout_attr(c);

    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
    return 0;
}

void buf_crop_assist(t_buf_crop *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        if (a == 0) // @in 0 @type symbol @digest Buffer name(s)
            sprintf(s, "symbol/llll: Buffer Names");
        else if (a == 1) // @in 1 @type number @digest Starting point
            sprintf(s, "float: Start"); // @description Starting point for cropping
        else if (a == 2) // @in 2 @type number @digest Ending point
            sprintf(s, "float: End"); // @description Ending point for cropping
    } else {
        sprintf(s, "Cropped Buffer Names"); // @out 0 @type symbol/list @digest Output buffer names(s)
                                            // @description Name of the cropped buffer
    }
}

void buf_crop_inletinfo(t_buf_crop *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_crop *buf_crop_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_crop *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_crop*)object_alloc_debug(s_tag_class);
    if (x) {
        x->from = llll_from_text_buf("0.", false);
        x->to = llll_from_text_buf("0.", false);

        // @arg 0 @name outname @optional 1 @type symbol
        // @digest Output buffer name
        // @description @copy EARS_DOC_OUTNAME_ATTR
        
        earsbufobj_init((t_earsbufobj *)x,  EARSBUFOBJ_FLAG_SUPPORTS_COPY_NAMES);

        t_llll *args = llll_parse(true_ac, argv);
        t_llll *names = earsbufobj_extract_names_from_args((t_earsbufobj *)x, args);
        t_llllelem *cur = args ? args->l_head : NULL;

        // @arg 1 @name options @optional 1 @type list
        // @digest Start and end options
        // @description If a single number is provided, it is considered to be the ending point of the cropping
        // (which would start at zero). If two numbers are provided, then they are the starting and ending point
        // for cropping. Units always depend on the <m>timeunit</m> attribute.
        if (args->l_size == 1) {
            if (cur) {
                if (hatom_gettype(&cur->l_hatom) == H_LLLL) {
                    llll_free(x->to);
                    x->to = llll_clone(hatom_getllll(&cur->l_hatom));
                } else {
                    llll_clear(x->to);
                    llll_appendhatom_clone(x->to, &cur->l_hatom);
                }
            }
        } else {
            if (cur) {
                if (hatom_gettype(&cur->l_hatom) == H_LLLL) {
                    llll_free(x->from);
                    x->from = llll_clone(hatom_getllll(&cur->l_hatom));
                } else {
                    llll_clear(x->from);
                    llll_appendhatom_clone(x->from, &cur->l_hatom);
                }
                cur = cur ? cur->l_next : NULL;
            }
            
            if (cur) {
                if (hatom_gettype(&cur->l_hatom) == H_LLLL) {
                    llll_free(x->to);
                    x->to = llll_clone(hatom_getllll(&cur->l_hatom));
                } else {
                    llll_clear(x->to);
                    llll_appendhatom_clone(x->to, &cur->l_hatom);
                }
            }
        }
        
        attr_args_process(x, argc, argv);
        
        earsbufobj_setup((t_earsbufobj *)x, "Eff", "E", names);

        llll_free(args);
        llll_free(names);
        
    }
    return x;
}


void buf_crop_free(t_buf_crop *x)
{
    llll_free(x->from);
    llll_free(x->to);
    earsbufobj_free((t_earsbufobj *)x);
}




void buf_crop_bang(t_buf_crop *x)
{
    long num_buffers = earsbufobj_get_instore_size((t_earsbufobj *)x, 0);
    
    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_buffers, true);
    
    earsbufobj_mutex_lock((t_earsbufobj *)x);
    earsbufobj_init_progress((t_earsbufobj *)x, num_buffers);
    
    t_llllelem *el_from = x->from->l_head;
    t_llllelem *el_to = x->to->l_head;
    for (long count = 0; count < num_buffers; count++,
         el_from = el_from && el_from->l_next ? el_from->l_next : el_from,
         el_to = el_to && el_to->l_next ? el_to->l_next : el_to) {
        t_buffer_obj *in = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, count);
        t_buffer_obj *out = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, count);
        
        long from_sample = earsbufobj_time_to_samps((t_earsbufobj *)x, el_from ? hatom_getdouble(&el_from->l_hatom) : 0, in);
        long to_sample = earsbufobj_time_to_samps((t_earsbufobj *)x, el_to ? hatom_getdouble(&el_to->l_hatom) : 0, in);
        
        ears_buffer_crop((t_object *)x, in, out, from_sample, to_sample);

        if (earsbufobj_iter_progress((t_earsbufobj *)x, count, num_buffers)) break;
    }
    earsbufobj_mutex_unlock((t_earsbufobj *)x);

    earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
}

void buf_crop_anything(t_buf_crop *x, t_symbol *msg, long ac, t_atom *av)
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
                
                buf_crop_bang(x);
            }
        } else if (inlet == 1) {
            earsbufobj_mutex_lock((t_earsbufobj *)x);
            llll_free(x->from);
            x->from = llll_clone(parsed);
            earsbufobj_mutex_unlock((t_earsbufobj *)x);
        } else {
            earsbufobj_mutex_lock((t_earsbufobj *)x);
            llll_free(x->to);
            x->to = llll_clone(parsed);
            earsbufobj_mutex_unlock((t_earsbufobj *)x);
        }
    }
    llll_free(parsed);
}



