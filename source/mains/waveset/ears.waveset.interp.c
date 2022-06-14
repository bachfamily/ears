/**
	@file
	ears.waveset.interp.c
 
	@name
	ears.waveset.interp~
 
	@realname
	ears.waveset.interp~
 
	@type
	object
 
	@module
	ears

	@author
	Daniele Ghisi
 
	@digest
	Waveset interpolation
 
	@description
	Interpolates portions of signal between zero crossings
 
	@discussion
 
	@category
	ears basic
 
	@keywords
	buffer, waveset, zero-crossing, interpolate
 
	@seealso
	ears.waveset.split~, ears.waveset.average~, ears.waveset.decimate~, ears.waveset.subs~
	
	@owner
	Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "foundation/llllobj.h"
#include "foundation/llll_commons_ext.h"
#include "math/bach_math_utilities.h"
#include "ears.object.h"
#include "ears.waveset.h"


typedef struct _buf_waveset_interp {
    t_earsbufobj       e_ob;
    long               e_span;
    long               e_normalize;
    long               e_eqp;
    t_llll             *e_howmany;
} t_buf_waveset_interp;



// Prototypes
t_buf_waveset_interp*         buf_waveset_interp_new(t_symbol *s, short argc, t_atom *argv);
void			buf_waveset_interp_free(t_buf_waveset_interp *x);
void			buf_waveset_interp_bang(t_buf_waveset_interp *x);
void			buf_waveset_interp_anything(t_buf_waveset_interp *x, t_symbol *msg, long ac, t_atom *av);

void buf_waveset_interp_assist(t_buf_waveset_interp *x, void *b, long m, long a, char *s);
void buf_waveset_interp_inletinfo(t_buf_waveset_interp *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(waveset_interp)

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
    
    CLASS_NEW_CHECK_SIZE(c, "ears.waveset.interp~",
                         (method)buf_waveset_interp_new,
                         (method)buf_waveset_interp_free,
                         sizeof(t_buf_waveset_interp),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    // @method list/llll @digest Function depends on inlet
    // @description A list or llll in the first inlet is supposed to contain buffer names and will
    // trigger the buffer processing and output the processed buffer names (depending on the <m>naming</m> attribute). <br />
    // A number or an llll in the second inlet is expected to contain a parameter (depending on the <m>mode</m>).
    // For "interp" mode, the parameter is the number of repetitions.
    EARSBUFOBJ_DECLARE_COMMON_METHODS_HANDLETHREAD(waveset_interp)

    // @method number @digest Set number of interpolations for each couple of successive wavesets
    // @description A number in the second inlet sets the number of interpolations

    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_blocking_attr(c);
    earsbufobj_class_add_naming_attr(c);
    earsbufobj_class_add_resamplingfiltersize_attr(c);
    earsbufobj_class_add_resamplingmode_attr(c);

    earsbufobj_class_add_polyout_attr(c);


    CLASS_ATTR_LONG(c, "span", 0, t_buf_waveset_interp, e_span);
    CLASS_ATTR_STYLE_LABEL(c,"span",0,"number","Group Wavesets");
    CLASS_ATTR_BASIC(c, "span", 0);
    // @description Sets the number of negative-to-positive zero crossing regions that form a waveset (defaults to 1: a single
    // negative-to-positive zero-crossing to negative-to-positive zero-crossing region).

    CLASS_ATTR_LONG(c, "normalize", 0, t_buf_waveset_interp, e_normalize);
    CLASS_ATTR_STYLE_LABEL(c,"normalize",0,"onoff","Normalize Wavesets");
    CLASS_ATTR_BASIC(c, "normalize", 0);
    // @description Toggles the normalization of wavesets.

    CLASS_ATTR_LONG(c, "eqp", 0, t_buf_waveset_interp, e_eqp);
    CLASS_ATTR_STYLE_LABEL(c,"eqp",0,"onoff","Equal Power Interpolation");
    CLASS_ATTR_BASIC(c, "eqp", 0);
    // @description Toggles equal power interpolation.

    
    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
    return 0;
}

void buf_waveset_interp_assist(t_buf_waveset_interp *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        if (a == 0)
            sprintf(s, "symbol/list/llll: Incoming Buffer Names"); // @in 0 @type symbol/list/llll @digest Incoming buffer names
        else
            sprintf(s, "number: Number Of Interpolations"); // @in 1 @type number @digest Number Of Interpolations
    } else {
        sprintf(s, "symbol/list: Output Buffer Names"); // @out 0 @type symbol/list @digest Output buffer names
    }
}

void buf_waveset_interp_inletinfo(t_buf_waveset_interp *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_waveset_interp *buf_waveset_interp_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_waveset_interp *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_waveset_interp*)object_alloc_debug(s_tag_class);
    if (x) {
        x->e_howmany = llll_from_text_buf("1", false);
        x->e_span = 1;
        x->e_normalize = 0.;
        x->e_eqp = 0;
        
        earsbufobj_init((t_earsbufobj *)x,  EARSBUFOBJ_FLAG_SUPPORTS_COPY_NAMES);
        
        // @arg 0 @name outnames @optional 1 @type symbol
        // @digest Output buffer names
        // @description @copy EARS_DOC_OUTNAME_ATTR

        t_llll *args = llll_parse(true_ac, argv);
        t_llll *names = earsbufobj_extract_names_from_args((t_earsbufobj *)x, args);
        
        if (args && args->l_head) {
            llll_clear(x->e_howmany);
            llll_appendhatom_clone(x->e_howmany, &args->l_head->l_hatom);
        }
        
        attr_args_process(x, argc, argv);
        
        earsbufobj_setup((t_earsbufobj *)x, "E4", "E", names);

        llll_free(args);
        llll_free(names);
    }
    return x;
}


void buf_waveset_interp_free(t_buf_waveset_interp *x)
{
    llll_free(x->e_howmany);
    earsbufobj_free((t_earsbufobj *)x);
}



void buf_waveset_interp_bang(t_buf_waveset_interp *x)
{
    long num_buffers = earsbufobj_get_instore_size((t_earsbufobj *)x, 0);
    
    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_buffers, true);
    
    earsbufobj_mutex_lock((t_earsbufobj *)x);
    earsbufobj_init_progress((t_earsbufobj *)x, num_buffers);
    
    t_llllelem *el = x->e_howmany->l_head;
    for (long count = 0; count < num_buffers; count++, el = el && el->l_next ? el->l_next : el) {
        t_buffer_obj *in = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, count);
        t_buffer_obj *out = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, count);
        
        ears_buffer_waveset_interp((t_object *)x, in, out, x->e_span, x->e_normalize, el && is_hatom_number(&el->l_hatom) ? hatom_getlong(&el->l_hatom) : 1, x->e_ob.l_resamplingfilterwidth, (e_ears_resamplingmode)x->e_ob.l_resamplingmode, x->e_eqp);
        
        if (earsbufobj_iter_progress((t_earsbufobj *)x, count, num_buffers)) break;
    }
    earsbufobj_mutex_unlock((t_earsbufobj *)x);
    
    earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
}


void buf_waveset_interp_anything(t_buf_waveset_interp *x, t_symbol *msg, long ac, t_atom *av)
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
            
            buf_waveset_interp_bang(x);
            
        } else if (inlet == 1) {
            earsbufobj_mutex_lock((t_earsbufobj *)x);
            llll_free(x->e_howmany);
            x->e_howmany = llll_clone(parsed);
            earsbufobj_mutex_unlock((t_earsbufobj *)x);
        }
    }
    llll_free(parsed);
}


