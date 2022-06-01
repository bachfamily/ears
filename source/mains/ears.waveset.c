/**
	@file
	ears.waveset.c
 
	@name
	ears.waveset~
 
	@realname
	ears.waveset~
 
	@type
	object
 
	@module
	ears

    @status
    experimental

	@author
	Daniele Ghisi
 
	@digest
	Waveset operations on buffer
 
	@description
	Performs operation based on zero-crossings
 
	@discussion
 
	@category
	ears basic
 
	@keywords
	buffer, waveset, zero-crossing
 
	@seealso
	ears.rubberband~, ears.soundtouch~
	
	@owner
	Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "llllobj.h"
#include "llll_commons_ext.h"
#include "bach_math_utilities.h"
#include "ears.object.h"


enum {
    EARS_WAVESET_MODE_REPEAT = 0,
};

typedef struct _buf_waveset {
    t_earsbufobj       e_ob;
    long               e_mode;
    long               e_group;
    double             e_normalize;
    t_llll             *args;
} t_buf_waveset;



// Prototypes
t_buf_waveset*         buf_waveset_new(t_symbol *s, short argc, t_atom *argv);
void			buf_waveset_free(t_buf_waveset *x);
void			buf_waveset_bang(t_buf_waveset *x);
void			buf_waveset_anything(t_buf_waveset *x, t_symbol *msg, long ac, t_atom *av);

void buf_waveset_assist(t_buf_waveset *x, void *b, long m, long a, char *s);
void buf_waveset_inletinfo(t_buf_waveset *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(waveset)

/**********************************************************************/
// Class Definition and Life Cycle


t_max_err buf_waveset_setattr_mode(t_buf_waveset *x, void *attr, long argc, t_atom *argv)
{
    if (argc && argv) {
        if (atom_gettype(argv) == A_SYM) {
            t_symbol *s = atom_getsym(argv);
            if (s == gensym("repeat") || s == gensym("Repeat"))
                x->e_mode = EARS_WAVESET_MODE_REPEAT;
            else
                object_error((t_object *)x, "Unsupported mode value.");
        } else if (atom_gettype(argv) == A_LONG){
            x->e_mode = atom_getlong(argv);
        } else {
            object_error((t_object *)x, "Unsupported mode value.");
        }
    }
    return MAX_ERR_NONE;
}

int C74_EXPORT main(void)
{
    common_symbols_init();
    llllobj_common_symbols_init();
    
    if (llllobj_check_version(bach_get_current_llll_version()) || llllobj_test()) {
        ears_error_bachcheck();
        return 1;
    }
    
    t_class *c;
    
    CLASS_NEW_CHECK_SIZE(c, "ears.waveset~",
                         (method)buf_waveset_new,
                         (method)buf_waveset_free,
                         sizeof(t_buf_waveset),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    // @method list/llll @digest Function depends on inlet
    // @description A list or llll in the first inlet is supposed to contain buffer names and will
    // trigger the buffer processing and output the processed buffer names (depending on the <m>naming</m> attribute). <br />
    // A number or an llll in the second inlet is expected to contain a parameter (depending on the <m>mode</m>).
    // For "repeat" mode, the parameter is the number of repetitions.
    EARSBUFOBJ_DECLARE_COMMON_METHODS_HANDLETHREAD(waveset)

    // @method number @digest Set waveset
    // @description A number in the second inlet sets the waveset parameter (depending on the <m>ampunit</m>).

    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_blocking_attr(c);
    earsbufobj_class_add_naming_attr(c);

    earsbufobj_class_add_polyout_attr(c);


    CLASS_ATTR_LONG(c, "group", 0, t_buf_waveset, e_group);
    CLASS_ATTR_STYLE_LABEL(c,"group",0,"number","Group Wavesets");
    CLASS_ATTR_BASIC(c, "group", 0);
    // @description Sets the number of negative-to-positive zero crossing regions that form a waveset (defaults to 1: a single
    // negative-to-positive zero-crossing to negative-to-positive zero-crossing region).

    CLASS_ATTR_DOUBLE(c, "normalize", 0, t_buf_waveset, e_normalize);
    CLASS_ATTR_STYLE_LABEL(c,"normalize",0,"number","Normalization Amount");
    // @description Sets the normalization amount, between 0 and 1
    // (0 = no normalization, 1 = fully normalized so that maximum absolute value of waveset samples is 1).

    
    CLASS_ATTR_CHAR(c, "mode", 0, t_buf_waveset, e_mode);
    CLASS_ATTR_STYLE_LABEL(c,"mode",0,"enumindex","Mode");
    CLASS_ATTR_ENUMINDEX(c,"mode", 0, "Repeat");
    CLASS_ATTR_ACCESSORS(c, "mode", NULL, buf_waveset_setattr_mode);
    CLASS_ATTR_BASIC(c, "mode", 0);
    // @description Sets the working mode: <br />
    // 0: Repeat - Repeats each waveset a certain number of times

    
    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
    return 0;
}

void buf_waveset_assist(t_buf_waveset *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        if (a == 0)
            sprintf(s, "symbol/list/llll: Incoming Buffer Names"); // @in 0 @type symbol/list/llll @digest Incoming buffer names
        else
            sprintf(s, "number/llll/symbol: Parameters"); // @in 1 @type number/llll/symbol @digest Parameters
    } else {
        sprintf(s, "symbol/list: Output Buffer Names"); // @out 0 @type symbol/list @digest Output buffer names
    }
}

void buf_waveset_inletinfo(t_buf_waveset *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_waveset *buf_waveset_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_waveset *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_waveset*)object_alloc_debug(s_tag_class);
    if (x) {
        x->args = llll_from_text_buf("1", false);
        x->e_mode = EARS_WAVESET_MODE_REPEAT;
        x->e_group = 1;
        x->e_normalize = 0.;
        
        earsbufobj_init((t_earsbufobj *)x,  EARSBUFOBJ_FLAG_SUPPORTS_COPY_NAMES);
        
        // @arg 0 @name outnames @optional 1 @type symbol
        // @digest Output buffer names
        // @description @copy EARS_DOC_OUTNAME_ATTR

        t_llll *args = llll_parse(true_ac, argv);
        t_llll *names = earsbufobj_extract_names_from_args((t_earsbufobj *)x, args);
        
        if (args && args->l_head) {
            llll_clear(x->args);
            llll_appendhatom_clone(x->args, &args->l_head->l_hatom);
        }
        
        attr_args_process(x, argc, argv);
        
        earsbufobj_setup((t_earsbufobj *)x, "E4", "E", names);

        llll_free(args);
        llll_free(names);
    }
    return x;
}


void buf_waveset_free(t_buf_waveset *x)
{
    llll_free(x->args);
    earsbufobj_free((t_earsbufobj *)x);
}



void buf_waveset_bang(t_buf_waveset *x)
{
    long num_buffers = earsbufobj_get_instore_size((t_earsbufobj *)x, 0);
    long mode = x->e_mode;
    
    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_buffers, true);
    
    earsbufobj_mutex_lock((t_earsbufobj *)x);
    earsbufobj_init_progress((t_earsbufobj *)x, num_buffers);
    
    t_llllelem *el = x->args->l_head;
    for (long count = 0; count < num_buffers; count++, el = el && el->l_next ? el->l_next : el) {
        t_buffer_obj *in = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, count);
        t_buffer_obj *out = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, count);
        
        switch (mode) {
            case EARS_WAVESET_MODE_REPEAT:
            default:
                ears_buffer_waveset_repeat((t_object *)x, in, out, el && is_hatom_number(&el->l_hatom) ? hatom_getlong(&el->l_hatom) : 1, x->e_group, x->e_normalize);
                break;
        }
        
        if (earsbufobj_iter_progress((t_earsbufobj *)x, count, num_buffers)) break;
    }
    earsbufobj_mutex_unlock((t_earsbufobj *)x);
    
    earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
}


void buf_waveset_anything(t_buf_waveset *x, t_symbol *msg, long ac, t_atom *av)
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
            
            buf_waveset_bang(x);
            
        } else if (inlet == 1) {
            earsbufobj_mutex_lock((t_earsbufobj *)x);
            llll_free(x->args);
            x->args = llll_clone(parsed);
            earsbufobj_mutex_unlock((t_earsbufobj *)x);
        }
    }
    llll_free(parsed);
}


