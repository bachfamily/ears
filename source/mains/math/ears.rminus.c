/**
	@file
	ears.rminus.c
 
	@name
	ears.!-~
 
	@realname
	ears.rminus~
 
	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	Subtract buffer samples with reversed inlets
 
	@description
    Functions like the ears.-~ object but with inlets reversed.

	@discussion
 
	@category
	ears math
 
	@keywords
	buffer, subtract, minus, reversed, reverse
 
	@seealso
	ears.-~, ears.+~, ears./~, ears.!-~, ears.!/~, ears.gain~

	@owner
	Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "foundation/llllobj.h"
#include "foundation/llll_commons_ext.h"
#include "math/bach_math_utilities.h"
#include "ears.object.h"



typedef struct _buf_rminus {
    t_earsbufobj       e_ob;
    
    char               e_scalarmode;
    t_llll             *e_operand;
} t_buf_rminus;



// Prototypes
t_buf_rminus*         buf_rminus_new(t_symbol *s, short argc, t_atom *argv);
void			buf_rminus_free(t_buf_rminus *x);
void			buf_rminus_bang(t_buf_rminus *x);
void			buf_rminus_anything(t_buf_rminus *x, t_symbol *msg, long ac, t_atom *av);

void buf_rminus_assist(t_buf_rminus *x, void *b, long m, long a, char *s);
void buf_rminus_inletinfo(t_buf_rminus *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(rminus)

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
    
    CLASS_NEW_CHECK_SIZE(c, "ears.rminus~",
                         (method)buf_rminus_new,
                         (method)buf_rminus_free,
                         sizeof(t_buf_rminus),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    // @method symbol/list/llll @digest Set operands
    // @description A symbol, list or llll in the first inlet is supposed to contain buffer names and will
    // trigger the buffer processing and output the processed buffer names (depending on the <m>naming</m> attribute). <br />
    // A symbol in the second inlet is expected to contain the name of the buffer to be used as a second operand.
    // A number or an llll in the second inlet is expected to contain respectively the numeric operand or the operand
    // in the form of an envelope.
    EARSBUFOBJ_DECLARE_COMMON_METHODS_HANDLETHREAD(rminus)
    
    // @method number @digest Set operand
    // @description A number in the second inlet sets a constant second operand

    
    CLASS_ATTR_CHAR(c, "scalarmode",    0,    t_buf_rminus, e_scalarmode);
    CLASS_ATTR_FILTER_CLIP(c, "scalarmode", 0, 1);
    CLASS_ATTR_STYLE(c, "scalarmode", 0, "onoff");
    CLASS_ATTR_LABEL(c, "scalarmode", 0, "Scalar Mode");
    CLASS_ATTR_BASIC(c, "scalarmode", 0);
    // @description If set, a single number, envelope or buffer is iterated against all the incoming buffers.

    
    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_blocking_attr(c);
    earsbufobj_class_add_envtimeunit_attr(c);
    earsbufobj_class_add_naming_attr(c);
    earsbufobj_class_add_slopemapping_attr(c);
    earsbufobj_class_add_resamplingfiltersize_attr(c);
    earsbufobj_class_add_resamplingpolicy_attr(c);
    earsbufobj_class_add_resamplingmode_attr(c);
    earsbufobj_class_add_polyout_attr(c);

    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
    return 0;
}

void buf_rminus_assist(t_buf_rminus *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        if (a == 0)
            sprintf(s, "symbol/list/llll: Incoming Buffer Names"); // @in 0 @type symbol/list/llll @digest Buffer names for first operand(s)
        else
            sprintf(s, "number/llll/symbol: Second operands"); // @in 1 @type number/llll/symbol @digest Second operand factor(s), envelope(s) or buffer(s)
    } else {
        sprintf(s, "symbol/list: Output Buffer Names"); // @out 0 @type symbol/list @digest Output buffer names
    }
}

void buf_rminus_inletinfo(t_buf_rminus *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_rminus *buf_rminus_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_rminus *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_rminus*)object_alloc_debug(s_tag_class);
    if (x) {
        x->e_operand = llll_from_text_buf("0.", false);
        x->e_scalarmode = 1;
        
        earsbufobj_init((t_earsbufobj *)x,  EARSBUFOBJ_FLAG_SUPPORTS_COPY_NAMES);
        
        // @arg 0 @name outnames @optional 1 @type symbol
        // @digest Output buffer names
        // @description @copy EARS_DOC_OUTNAME_ATTR

        // @arg 1 @name operand @optional 1 @type number/llll
        // @digest Second operand
        // @description Sets an optional initial number or envelope to be used as second operand.

        t_llll *args = llll_parse(true_ac, argv);
        t_llll *names = earsbufobj_extract_names_from_args((t_earsbufobj *)x, args);
        
        if (args && args->l_head) {
            llll_clear(x->e_operand);
            llll_appendhatom_clone(x->e_operand, &args->l_head->l_hatom);
        }
        
        attr_args_process(x, argc, argv);
        
        earsbufobj_setup((t_earsbufobj *)x, "EE", "E", names);

        llll_free(args);
        llll_free(names);
    }
    return x;
}


void buf_rminus_free(t_buf_rminus *x)
{
    llll_free(x->e_operand);
    earsbufobj_free((t_earsbufobj *)x);
}



void buf_rminus_bang(t_buf_rminus *x)
{
    long num_buffers1 = earsbufobj_get_instore_size((t_earsbufobj *)x, 0);
    long num_buffers2 = earsbufobj_get_instore_size((t_earsbufobj *)x, 1);
    long scalarmode = x->e_scalarmode;
    
    if (num_buffers1 > 0 && num_buffers2 > 0) {
        // operation between buffers
        long numoutbuffers = MIN(num_buffers1, num_buffers2);
        if (scalarmode) {
            if (num_buffers1 > 1 && num_buffers2 == 1) {
                numoutbuffers = num_buffers1;
            }
            if (num_buffers1 == 1 && num_buffers2 > 1) {
                numoutbuffers = num_buffers2;
            }
        }
                
        earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
        earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_OUT, 0, numoutbuffers, true);

        earsbufobj_mutex_lock((t_earsbufobj *)x);
        earsbufobj_init_progress((t_earsbufobj *)x, numoutbuffers);
        for (long count = 0; count < numoutbuffers; count++) {
            t_buffer_obj *in1 = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, count < num_buffers1 ? count : 0);
            t_buffer_obj *in2 = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 1, count < num_buffers2 ? count : 0);
            t_buffer_obj *out = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, count);
            ears_buffer_op((t_object *)x, in1, in2, out, EARS_OP_RMINUS, (e_ears_resamplingpolicy) x->e_ob.l_resamplingpolicy, x->e_ob.l_resamplingfilterwidth, (e_ears_resamplingmode)x->e_ob.l_resamplingmode);
            if (earsbufobj_iter_progress((t_earsbufobj *)x, count, numoutbuffers)) break;
        }
        earsbufobj_mutex_unlock((t_earsbufobj *)x);
        earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);

    } else if (num_buffers1 > 0 || num_buffers2 > 0) {
        if (x->e_operand->l_size > 0) {
            // operation between a buffer and a value
            long numoutbuffers = MAX(num_buffers1, num_buffers2);

            earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
            earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_OUT, 0, numoutbuffers, true);

            earsbufobj_mutex_lock((t_earsbufobj *)x);
            earsbufobj_init_progress((t_earsbufobj *)x, numoutbuffers);
            long inlet = (num_buffers1 > 0 ? 0 : 1);
            t_llllelem *el = x->e_operand->l_head;
            for (long count = 0; count < numoutbuffers; count++, el = (el && el->l_next ? el->l_next : el)) {
                t_buffer_obj *in1 = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, inlet, count);
                t_buffer_obj *out = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, count);
                if (el && is_hatom_number(&el->l_hatom)) {
                    ears_buffer_number_op((t_object *)x, in1, hatom_getdouble(&el->l_hatom), out, EARS_OP_RMINUS);
                } else if (el && hatom_gettype(&el->l_hatom) == H_LLLL){
                    t_llll *env = earsbufobj_llllelem_to_env_samples((t_earsbufobj *)x, el, in1);
                    ears_buffer_envelope_op((t_object *)x, in1, env, out, EARS_OP_RMINUS, (e_slope_mapping) x->e_ob.l_slopemapping);
                    llll_free(env);
                } else {
                    object_error((t_object *)x, "Wrong syntax.");
                }
                if (earsbufobj_iter_progress((t_earsbufobj *)x, count, numoutbuffers)) break;
            }
            earsbufobj_mutex_unlock((t_earsbufobj *)x);
            earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
        } else {
            object_error((t_object *)x, "Not enough terms introduced.");
        }
    } else {
        object_error((t_object *)x, "Not enough terms introduced.");
    }
}


void buf_rminus_anything(t_buf_rminus *x, t_symbol *msg, long ac, t_atom *av)
{
    long inlet = earsbufobj_proxy_getinlet((t_earsbufobj *) x);

    t_llll *parsed = earsbufobj_parse_gimme((t_earsbufobj *) x, LLLL_OBJ_VANILLA, msg, ac, av);
    if (!parsed) return;
    
    if (parsed && parsed->l_head) {
        if (hatom_gettype(&parsed->l_head->l_hatom) == H_SYM) {
            long num_bufs = llll_get_num_symbols_root(parsed);

            earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, inlet, num_bufs, true);
            earsbufobj_store_buffer_list((t_earsbufobj *)x, parsed, inlet);
            
            if (inlet == 0)
                buf_rminus_bang(x);
            
        } else if (inlet == 1) {
            earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, inlet, 0, false);
            earsbufobj_mutex_lock((t_earsbufobj *)x);
            llll_free(x->e_operand);
            x->e_operand = llll_clone(parsed);
            earsbufobj_mutex_unlock((t_earsbufobj *)x);
        }
    }
    llll_free(parsed);
}


