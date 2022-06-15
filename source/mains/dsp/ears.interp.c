/**
	@file
	ears.interp.c
 
	@name
	ears.interp~
 
	@realname
	ears.interp~
 
	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	Buffer linear interpolation
 
	@description
	Perform a linear interpolation between buffers, sample by sample
 
	@discussion
 
	@category
	ears distorsion
 
	@keywords
	buffer, interp, interpolate, linear
 
	@seealso
	ears.waveset.interp~
	
	@owner
	Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "foundation/llllobj.h"
#include "foundation/llll_commons_ext.h"
#include "math/bach_math_utilities.h"
#include "ears.object.h"



typedef struct _buf_interp {
    t_earsbufobj       e_ob;
    
    long               e_num_steps;
    char               e_keep_orig;
    char               e_eqp;
} t_buf_interp;



// Prototypes
t_buf_interp*         buf_interp_new(t_symbol *s, short argc, t_atom *argv);
void			buf_interp_free(t_buf_interp *x);
void			buf_interp_bang(t_buf_interp *x);
void			buf_interp_anything(t_buf_interp *x, t_symbol *msg, long ac, t_atom *av);

void buf_interp_assist(t_buf_interp *x, void *b, long m, long a, char *s);
void buf_interp_inletinfo(t_buf_interp *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(interp)

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
    
    CLASS_NEW_CHECK_SIZE(c, "ears.interp~",
                         (method)buf_interp_new,
                         (method)buf_interp_free,
                         sizeof(t_buf_interp),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    // @method list/llll @digest Function depends on inlet
    // @description A list or llll in the first inlet is supposed to contain buffer names and will
    // trigger the buffer processing and output the processed buffer names (depending on the <m>naming</m> attribute). <br />
    // A number or an llll in the second inlet is expected to contain a interp threshold (depending on the <m>ampunit</m>) or
    // an envelope (also see <m>envampunit</m>).
    EARSBUFOBJ_DECLARE_COMMON_METHODS_HANDLETHREAD(interp)

    // @method number @digest Set interp
    // @description A number in the second inlet sets the interp parameter (depending on the <m>ampunit</m>).

    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_blocking_attr(c);
    earsbufobj_class_add_naming_attr(c);
    earsbufobj_class_add_polyout_attr(c);
    
    
    CLASS_ATTR_LONG(c, "numsteps", 0, t_buf_interp, e_num_steps);
    CLASS_ATTR_STYLE_LABEL(c,"numsteps",0,"text","Number Of Steps");
    CLASS_ATTR_BASIC(c, "numsteps", 0);
    // @description Sets the number of interpolation steps including the starting and ending point,
    // unless the <m>keep</m> attribute is set to 0.

    
    CLASS_ATTR_CHAR(c, "keep", 0, t_buf_interp, e_keep_orig);
    CLASS_ATTR_STYLE_LABEL(c,"keep",0,"onoff","Keep Starting and Ending Buffers");
    CLASS_ATTR_BASIC(c, "keep", 0);
    // @description Toggles the ability to keep the starting and ending interpolation buffers in the output, and count them
    // in the <m>numsteps</m> attribute

    
    CLASS_ATTR_CHAR(c, "eqp", 0, t_buf_interp, e_eqp);
    CLASS_ATTR_STYLE_LABEL(c,"eqp",0,"onoff","Equal Power Interpolation");
    CLASS_ATTR_BASIC(c, "eqp", 0);
    // @description Toggles equal power interpolation.

    
    
    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
}

void buf_interp_assist(t_buf_interp *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        if (a == 0)
            sprintf(s, "symbol/list/llll: Initial Buffer Name"); // @in 0 @type symbol @digest Initial buffer names
        else if (a == 1)
            sprintf(s, "symbol/list/llll: Final Buffer Name"); // @in 1 @type symbol @digest Final buffer names
        else
            sprintf(s, "int: Number of Interpolation Steps"); // @in 1 @type int @digest Number of interpolation steps
    } else {
        sprintf(s, "symbol/list: Output Buffer Names"); // @out 0 @type symbol/list @digest Output buffer names
    }
}

void buf_interp_inletinfo(t_buf_interp *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_interp *buf_interp_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_interp *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_interp*)object_alloc_debug(s_tag_class);
    if (x) {
        x->e_keep_orig = true;
        x->e_num_steps = 0;
        
        earsbufobj_init((t_earsbufobj *)x, EARSBUFOBJ_FLAG_NONE);
        
        // @arg 0 @name outnames @optional 1 @type symbol
        // @digest Output buffer names
        // @description @copy EARS_DOC_OUTNAME_ATTR

        // @arg 1 @name numsteps @optional 1 @type int
        // @digest Number of interpolation steps
        // @description Sets the number of interpolation steps

        t_llll *args = llll_parse(true_ac, argv);
        t_llll *names = earsbufobj_extract_names_from_args((t_earsbufobj *)x, args);
        
        if (args && args->l_head) {
            if (is_hatom_number(&args->l_head->l_hatom))
                x->e_num_steps = MAX(0, hatom_getlong(&args->l_head->l_hatom));
        }
        
        attr_args_process(x, argc, argv);
        
        earsbufobj_setup((t_earsbufobj *)x, "ee4", "E", names);

        llll_free(args);
        llll_free(names);
    }
    return x;
}


void buf_interp_free(t_buf_interp *x)
{
    earsbufobj_free((t_earsbufobj *)x);
}



void buf_interp_bang(t_buf_interp *x)
{
    long num_buffers1 = earsbufobj_get_instore_size((t_earsbufobj *)x, 0);
    long num_buffers2 = earsbufobj_get_instore_size((t_earsbufobj *)x, 1);
    bool keep = x->e_keep_orig;
    bool eqp = x->e_eqp;
    long numinterp = x->e_num_steps;
    long actualnuminterp = keep ? numinterp - 2 : numinterp;
    
    if (num_buffers1 <= 0 || num_buffers2 <= 0 || actualnuminterp < 0) {
        return;
    }
    
    long numoutbuffers = numinterp;
    
    if (numoutbuffers <= 0) {
        return;
    }
    
    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_OUT, 0, numoutbuffers, true);
    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);

    earsbufobj_mutex_lock((t_earsbufobj *)x);

    t_buffer_obj *in1 = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, 0);
    t_buffer_obj *in2 = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 1, 0);
    t_buffer_obj **out = (t_buffer_obj **)bach_newptr(numoutbuffers * sizeof(t_buffer_obj *));
    
    for (long i = 0; i < numoutbuffers; i++)
        out[i] = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, i);
    if (keep) {
        ears_buffer_clone((t_object *)x, in1, out[0]);
        ears_buffer_clone((t_object *)x, in2, out[numoutbuffers-1]);
    }
    
    ears_buffer_interp((t_object *)x, in1, in2, actualnuminterp, keep ? out+1 : out, x->e_ob.l_resamplingfilterwidth, x->e_ob.l_resamplingmode, eqp);
    
    
    bach_freeptr(out);
    
    earsbufobj_mutex_unlock((t_earsbufobj *)x);
    
    earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
}


void buf_interp_anything(t_buf_interp *x, t_symbol *msg, long ac, t_atom *av)
{
    long inlet = earsbufobj_proxy_getinlet((t_earsbufobj *) x);

    t_llll *parsed = earsbufobj_parse_gimme((t_earsbufobj *) x, LLLL_OBJ_VANILLA, msg, ac, av);
    if (!parsed) return;
    
    if (parsed && parsed->l_head) {
        if (inlet == 0 || inlet == 1) {
            long num_bufs = llll_get_num_symbols_root(parsed);

            earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, inlet, num_bufs, true);
            
            earsbufobj_store_buffer_list((t_earsbufobj *)x, parsed, inlet);
            
            if (inlet == 0)
                buf_interp_bang(x);
            
        } else if (inlet == 2) {
            earsbufobj_mutex_lock((t_earsbufobj *)x);
            if (is_hatom_number(&parsed->l_head->l_hatom))
                x->e_num_steps = MAX(0, hatom_getlong(&parsed->l_head->l_hatom));
            earsbufobj_mutex_unlock((t_earsbufobj *)x);
        }
    }
    llll_free(parsed);
}


