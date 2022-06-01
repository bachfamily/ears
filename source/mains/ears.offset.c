/**
	@file
	ears.offset.c
 
	@name
	ears.offset~
 
	@realname
	ears.offset~
 
	@type
	object
 
	@module
	ears
 
    @hiddenalias
    ears.shift~
 
	@author
	Daniele Ghisi
 
	@digest
	Temporally offset a buffer
 
	@description
	Adds or remove time at the beginning of a buffer
 
	@discussion
 
	@category
	ears basic
 
	@keywords
	buffer, offset, add, offset
 
	@seealso
	ears.crop~, ears.rev~
	
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

typedef struct _buf_offset {
    t_earsbufobj       e_ob;
    t_llll             *amount;
    char               interp_offsets;
} t_buf_offset;



// Prototypes
t_buf_offset*         buf_offset_new(t_symbol *s, short argc, t_atom *argv);
void			buf_offset_free(t_buf_offset *x);
void			buf_offset_bang(t_buf_offset *x);
void			buf_offset_anything(t_buf_offset *x, t_symbol *msg, long ac, t_atom *av);

void buf_offset_assist(t_buf_offset *x, void *b, long m, long a, char *s);
void buf_offset_inletinfo(t_buf_offset *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(offset)



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
    
    CLASS_NEW_CHECK_SIZE(c, "ears.offset~",
                         (method)buf_offset_new,
                         (method)buf_offset_free,
                         sizeof(t_buf_offset),
                         (method)NULL,
                         A_GIMME,
                         0L);

    // @method list/llll @digest Process buffers or Set Offset
    // @description A list or llll in the first inlet with buffer names will trigger the buffer processing and output the processed
    // buffer names (depending on the <m>naming</m> attribute). <br />
    // A number, list or llll in the second inlet sets the offset in the
    // defined <m>timeunit</m>. Non-integer sample offsets are accounted for only if the <m>interp</m> attribute is
    // active, otherwise they are rounded to the nearest sample.
    EARSBUFOBJ_DECLARE_COMMON_METHODS_HANDLETHREAD(offset)
    
    // @method number @digest Set Offset
    // See <m>list</m> method, for second inlet.

    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_blocking_attr(c);
    earsbufobj_class_add_timeunit_attr(c);
    earsbufobj_class_add_naming_attr(c);
    earsbufobj_class_add_resamplingfiltersize_attr(c);
    
    earsbufobj_class_add_polyout_attr(c);

    CLASS_ATTR_CHAR(c, "interp", 0, t_buf_offset, interp_offsets);
    CLASS_ATTR_STYLE_LABEL(c,"interp",0,"onoff","Interpolate Non-Integer Offset");
    // @description Toggles the ability to perform band-limited interpolation via resampling for non-integer offsets.

    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
    return 0;
}

void buf_offset_assist(t_buf_offset *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        if (a == 0) // @in 0 @type symbol/list/llll @digest Buffer name(s)
            sprintf(s, "symbol/llll: Buffer Names");
        else if (a == 1) // @out 1 @type number/list/llll @digest Shift amount
            sprintf(s, "float: Shift Amount"); 
    } else {
        sprintf(s, "Shifted Buffer Names"); // @out 0 @type symbol/llll @digest Shifted buffer name(s)
    }
}

void buf_offset_inletinfo(t_buf_offset *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_offset *buf_offset_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_offset *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_offset*)object_alloc_debug(s_tag_class);
    if (x) {
        x->amount = llll_from_text_buf("0.", false);
        x->interp_offsets = 0;
        
        earsbufobj_init((t_earsbufobj *)x,  EARSBUFOBJ_FLAG_SUPPORTS_COPY_NAMES);

        // @arg 0 @name outname @optional 1 @type symbol
        // @digest Output buffer name
        // @description @copy EARS_DOC_OUTNAME_ATTR
        
        t_llll *args = llll_parse(true_ac, argv);
        t_llll *names = earsbufobj_extract_names_from_args((t_earsbufobj *)x, args);
        t_llllelem *cur = args ? args->l_head : NULL;

        // @arg 1 @name initial_amount @optional 1 @type number
        // @digest Initial Offset
        // @description Initial amount of offseting (unit depends on the <m>timeunit</m> attribute).
        // See <m>int</m> or <m>float</m> messages.
        if (cur) {
            if (hatom_gettype(&cur->l_hatom) == H_LLLL) {
                llll_free(x->amount);
                x->amount = llll_clone(hatom_getllll(&cur->l_hatom));
            } else {
                llll_clear(x->amount);
                llll_appendhatom_clone(x->amount, &cur->l_hatom);
            }
            cur = cur ? cur->l_next : NULL;
        }
        
        attr_args_process(x, argc, argv);
        
        earsbufobj_setup((t_earsbufobj *)x, "Ef", "E", names);

        llll_free(args);
        llll_free(names);
        
    }
    return x;
}


void buf_offset_free(t_buf_offset *x)
{
    llll_free(x->amount);
    earsbufobj_free((t_earsbufobj *)x);
}




void buf_offset_bang(t_buf_offset *x)
{
    long num_buffers = earsbufobj_get_instore_size((t_earsbufobj *)x, 0);
    
    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_buffers, true);
    
    earsbufobj_mutex_lock((t_earsbufobj *)x);
    earsbufobj_init_progress((t_earsbufobj *)x, num_buffers);
    
    t_llllelem *el = x->amount->l_head;
    for (long count = 0; count < num_buffers; count++, el = el && el->l_next ? el->l_next : el) {
        t_buffer_obj *in = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, count);
        t_buffer_obj *out = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, count);
        
        if (x->interp_offsets)
            ears_buffer_offset_subsampleprec((t_object *)x, in, out, earsbufobj_time_to_fsamps((t_earsbufobj *)x, el ? hatom_getdouble(&el->l_hatom) : 0, in), x->e_ob.l_resamplingfilterwidth);
        else
            ears_buffer_offset((t_object *)x, in, out, earsbufobj_time_to_samps((t_earsbufobj *)x, el ? hatom_getdouble(&el->l_hatom) : 0, in));
        
        if (earsbufobj_iter_progress((t_earsbufobj *)x, count, num_buffers)) break;
    }
    earsbufobj_mutex_unlock((t_earsbufobj *)x);

    earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
}

void buf_offset_anything(t_buf_offset *x, t_symbol *msg, long ac, t_atom *av)
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
                
                buf_offset_bang(x);
            }
        } else {
            earsbufobj_mutex_lock((t_earsbufobj *)x);
            llll_free(x->amount);
            x->amount = llll_clone(parsed);
            earsbufobj_mutex_unlock((t_earsbufobj *)x);
        }
    }
    llll_free(parsed);
}



