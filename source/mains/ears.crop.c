/**
	@file
	ears.crop.c
 
	@name
	ears.crop~
 
	@realname
	ears.crop~
 
    @hiddenalias
    ears.crop

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
	ears buffer operations
 
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

int C74_EXPORT main(void)
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
    EARSBUFOBJ_DECLARE_COMMON_METHODS_DEFER(crop)
    
    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_timeunit_attr(c);
    earsbufobj_class_add_naming_attr(c);

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
        else if (a == 1) // @out 1 @type number @digest Starting point
            sprintf(s, "float: Start"); // @description Starting point for cropping
        else if (a == 2) // @out 2 @type number @digest Ending point
            sprintf(s, "float: End"); // @description Ending point for cropping
    } else {
        sprintf(s, "Cropped Buffer Names"); // @description Name of the cropped buffer
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
        
        t_llll *args = llll_parse(true_ac, argv);
        t_llll *names = NULL;
        t_llllelem *cur = args ? args->l_head : NULL;
        if (cur) {
            if (hatom_gettype(&cur->l_hatom) == H_LLLL) {
                names = llll_clone(hatom_getllll(&cur->l_hatom));
                cur = cur ? cur->l_next : NULL;
            } else if (hatom_gettype(&cur->l_hatom) == H_SYM) {
                names = llll_get();
                llll_appendhatom_clone(names, &cur->l_hatom);
                cur = cur ? cur->l_next : NULL;
            }
            
            // @arg 1 @name start @optional 1 @type number
            // @digest Starting point
            // @description Starting point for cropping (unit depends on the <m>timeunit</m> attribute).
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

            // @arg 2 @name end @optional 1 @type number
            // @digest Ending point
            // @description Ending point for cropping (unit depends on the <m>timeunit</m> attribute).
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
        
        earsbufobj_init((t_earsbufobj *)x, EARSBUFOBJ_FLAG_DONT_DUPLICATE_INPUT_BUFFERS | EARSBUFOBJ_FLAG_SUPPORTS_COPY_NAMES);
        
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



/*

void buf_crop_bang(t_buf_crop *x)
{
    earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
}

void buf_crop_anything(t_buf_crop *x, t_symbol *msg, long ac, t_atom *av)
{
    long inlet = proxy_getinlet((t_object *) x);

    t_llll *parsed = llllobj_parse_llll((t_object *) x, LLLL_OBJ_VANILLA, msg, ac, av, LLLL_PARSE_CLONE);
    if (!parsed) return;
    
    if (parsed && parsed->l_head) {
        if (inlet == 0) {
            if (hatom_gettype(&parsed->l_head->l_hatom) == H_SYM) {
                t_symbol *buf = hatom_getsym(&parsed->l_head->l_hatom);
                
                earsbufobj_refresh_outlet_names((t_earsbufobj *)x);

                // storing input buffer
                earsbufobj_store_buffer((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, 0, buf);
                earsbufobj_store_copy_format((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, 0, EARSBUFOBJ_OUT, 0, 0);
                
                long from_sample = x->from < 0 ? -1 : earsbufobj_input_to_samps((t_earsbufobj *)x, x->from, earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, 0));
                long to_sample = x->to < 0 ? -1 : earsbufobj_input_to_samps((t_earsbufobj *)x, x->to, earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, 0));

                // cloning inlet store to outlet store, and then cropping
                t_buffer_obj *in = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, 0);
                t_buffer_obj *out = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, 0);
                ears_buffer_crop((t_object *)x, in, out, from_sample, to_sample);

                earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
            }
        } else if (inlet == 1) {
            if (is_hatom_number(&parsed->l_head->l_hatom))
                x->from = hatom_getdouble(&parsed->l_head->l_hatom);
        } else {
            if (is_hatom_number(&parsed->l_head->l_hatom))
                x->to = hatom_getdouble(&parsed->l_head->l_hatom);
        }
    }
    llll_free(parsed);
}
*/


void buf_crop_bang(t_buf_crop *x)
{
    long num_buffers = ((t_earsbufobj *)x)->l_instore[0].num_stored_bufs;
    
    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_buffers, true);
    
    t_llllelem *el_from = x->from->l_head;
    t_llllelem *el_to = x->to->l_head;
    for (long count = 0; count < num_buffers; count++,
         el_from = el_from && el_from->l_next ? el_from->l_next : el_from,
         el_to = el_to && el_to->l_next ? el_to->l_next : el_to) {
        t_buffer_obj *in = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, count);
        t_buffer_obj *out = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, count);
        
        long from_sample = earsbufobj_input_to_samps((t_earsbufobj *)x, el_from ? hatom_getdouble(&el_from->l_hatom) : 0, in);
        long to_sample = earsbufobj_input_to_samps((t_earsbufobj *)x, el_to ? hatom_getdouble(&el_to->l_hatom) : 0, in);
        
        ears_buffer_crop((t_object *)x, in, out, from_sample, to_sample);
    }
    
    earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
}

void buf_crop_anything(t_buf_crop *x, t_symbol *msg, long ac, t_atom *av)
{
    long inlet = proxy_getinlet((t_object *) x);
    
    t_llll *parsed = earsbufobj_parse_gimme((t_earsbufobj *) x, LLLL_OBJ_VANILLA, msg, ac, av);
    if (!parsed) return;
    
    if (parsed && parsed->l_head) {
        if (inlet == 0) {
            if (hatom_gettype(&parsed->l_head->l_hatom) == H_SYM) {
                
                //                earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
                
                earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, parsed->l_size, true);
                earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_OUT, 0, parsed->l_size, true);
                earsbufobj_store_buffer_list((t_earsbufobj *)x, parsed, 0, true);
                
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



