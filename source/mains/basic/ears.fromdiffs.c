/**
	@file
	ears.fromdiffs.c
 
	@name
	ears.fromdiffs~
 
	@realname
	ears.fromdiffs~
 
	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	Reconstruct signal from sample or frame differences
 
	@description
	Integrates differences between samples or spectral frames.
 
	@discussion
 
	@category
	ears basic
 
	@keywords
	buffer, todiffs, difference, dx2x, dx
 
	@seealso
	ears.todiffs~
	
	@owner
	Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "foundation/llllobj.h"
#include "foundation/llll_commons_ext.h"
#include "math/bach_math_utilities.h"
#include "ears.object.h"



typedef struct _buf_fromdiffs {
    t_earsbufobj       e_ob;
    t_llll             *e_initials;
} t_buf_fromdiffs;



// Prototypes
t_buf_fromdiffs*         buf_fromdiffs_new(t_symbol *s, short argc, t_atom *argv);
void			buf_fromdiffs_free(t_buf_fromdiffs *x);
void			buf_fromdiffs_bang(t_buf_fromdiffs *x);
void			buf_fromdiffs_anything(t_buf_fromdiffs *x, t_symbol *msg, long ac, t_atom *av);

void buf_fromdiffs_assist(t_buf_fromdiffs *x, void *b, long m, long a, char *s);
void buf_fromdiffs_inletinfo(t_buf_fromdiffs *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(fromdiffs)

/**********************************************************************/
// Class Definition and Life Cycle

void C74_EXPORT ext_main(void* moduleRef)
{
    common_symbols_init();
    llllobj_common_symbols_init();
    
    if (llllobj_check_version(bach_get_current_llll_version()) || llllobj_test()) {
        ears_error_bachcheck();
        return;
    }
    
    t_class *c;
    
    CLASS_NEW_CHECK_SIZE(c, "ears.fromdiffs~",
                         (method)buf_fromdiffs_new,
                         (method)buf_fromdiffs_free,
                         sizeof(t_buf_fromdiffs),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    // @method symbol/list/llll @digest Function depends on inlet
    // @description A list or llll in the first inlet is supposed to contain buffer names and will
    // trigger the buffer processing and output the processed buffer names (depending on the <m>alloc</m> attribute). <br />
    // A symbol in the second inlet is considered to be a buffer whose first sample is used to start the reconstruction.
    // A list in the second inlet is considered to be a set of samples (one for each channel) to start the reconstruction
    // (also see <m>float</m>).
    EARSBUFOBJ_DECLARE_COMMON_METHODS_HANDLETHREAD(fromdiffs)

    // @method float @digest Set initial value
    // @description A number in the second inlet sets the initial value for the integration (also see <m>list</m> and <m>symbol</m>).

    // @method int @digest Set initial value
    // @description A number in the second inlet sets the initial value for the integration (also see <m>list</m> and <m>symbol</m>).

    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_blocking_attr(c);
    earsbufobj_class_add_alloc_attr(c);
    earsbufobj_class_add_polyout_attr(c);

    
    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
}

void buf_fromdiffs_assist(t_buf_fromdiffs *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        if (a == 0)
            sprintf(s, "symbol/list/llll: Incoming Buffer Names"); // @in 0 @type symbol/list/llll @digest Incoming buffer names
    } else {
        sprintf(s, "symbol/list: Output Buffer Names"); // @out 0 @type symbol/list @digest Output buffer names
    }
}

void buf_fromdiffs_inletinfo(t_buf_fromdiffs *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_fromdiffs *buf_fromdiffs_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_fromdiffs *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_fromdiffs*)object_alloc_debug(s_tag_class);
    if (x) {
        x->e_initials = llll_from_text_buf("0.");
        
        earsbufobj_init((t_earsbufobj *)x, EARSBUFOBJ_FLAG_SUPPORTS_COPY_NAMES);
        
        // @arg 0 @name outnames @optional 1 @type symbol
        // @digest Output buffer names
        // @description @copy EARS_DOC_OUTNAME_ATTR

        // @arg 1 @name initials @optional 1 @type float/list
        // @digest Starting samples
        // @description Sets the initial sample or samples (one number per channel)

        t_llll *args = llll_parse(true_ac, argv);
        t_llll *names = earsbufobj_extract_names_from_args((t_earsbufobj *)x, args);
        
        if (args && args->l_head) {
            llll_free(x->e_initials);
            x->e_initials = llll_clone(args);
        }
        
        attr_args_process(x, argc, argv);
        
        earsbufobj_setup((t_earsbufobj *)x, "E4", "E", names);

        llll_free(args);
        llll_free(names);
    }
    return x;
}


void buf_fromdiffs_free(t_buf_fromdiffs *x)
{
    earsbufobj_free((t_earsbufobj *)x);
}



void buf_fromdiffs_bang(t_buf_fromdiffs *x)
{
    long num_buffers = earsbufobj_get_instore_size((t_earsbufobj *)x, 0);
    
    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_buffers, true);
    
    earsbufobj_mutex_lock((t_earsbufobj *)x);
    earsbufobj_init_progress((t_earsbufobj *)x, num_buffers);
    
    for (long count = 0; count < num_buffers; count++) {
        t_buffer_obj *in = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, count);
        t_buffer_obj *out = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, count);

        float *initial_fl = NULL;
        long size = 0;
        if (x->e_initials->l_head && hatom_gettype(&x->e_initials->l_head->l_hatom) == H_SYM) {
            // using a buffer
            t_buffer_obj *initialbuf = (t_buffer_obj *)ears_buffer_get_object(hatom_getsym(&x->e_initials->l_head->l_hatom));
            if (initialbuf) {
                ears_buffer_fromdiffs((t_object *)x, in, out, initialbuf);
            } else {
                object_warn((t_object *)x, "A symbol is introduced as starting point, but it does not seem to refer to any buffer.");
                ears_buffer_fromdiffs((t_object *)x, in, out, NULL, 9);
            }
        } else {
            size = x->e_initials->l_size;
            initial_fl = (size > 0) ? (float *)bach_newptrclear(size * sizeof(float)) : NULL;
            long i = 0;
            for (t_llllelem *el = x->e_initials->l_head; el && i < size; el = el->l_next, i++) {
                if (is_hatom_number(&el->l_hatom))
                    initial_fl[i] = hatom_getdouble(&el->l_hatom);
            }
            ears_buffer_fromdiffs((t_object *)x, in, out, initial_fl, size);
        }

        if (initial_fl)
            bach_freeptr(initial_fl);
        if (earsbufobj_iter_progress((t_earsbufobj *)x, count, num_buffers)) break;
    }
    earsbufobj_mutex_unlock((t_earsbufobj *)x);
    
    earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
}


void buf_fromdiffs_anything(t_buf_fromdiffs *x, t_symbol *msg, long ac, t_atom *av)
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
            
            buf_fromdiffs_bang(x);
        } else if (inlet == 1) {
            earsbufobj_mutex_lock((t_earsbufobj *)x);
            llll_free(x->e_initials);
            x->e_initials = llll_clone(parsed);
            earsbufobj_mutex_unlock((t_earsbufobj *)x);
        }
    }
    llll_free(parsed);
}


