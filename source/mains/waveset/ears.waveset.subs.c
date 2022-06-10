/**
	@file
	ears.waveset.subs.c
 
	@name
	ears.waveset.subs~
 
	@realname
	ears.waveset.subs~
 
	@type
	object
 
	@module
	ears

	@author
	Daniele Ghisi
 
	@digest
	Waveset substitution
 
	@description
	Substitute portions of signal between zero crossings with another waveform
 
	@discussion
 
	@category
	ears basic
 
	@keywords
	buffer, waveset, zero-crossing
 
	@seealso
	ears.waveset.split~, ears.waveset.repeat~
	
	@owner
	Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "llllobj.h"
#include "llll_commons_ext.h"
#include "bach_math_utilities.h"
#include "ears.object.h"
#include "ears.waveset.h"

typedef struct _buf_waveset_subs {
    t_earsbufobj       e_ob;
    long               e_span;
} t_buf_waveset_subs;



// Prototypes
t_buf_waveset_subs*         buf_waveset_subs_new(t_symbol *s, short argc, t_atom *argv);
void			buf_waveset_subs_free(t_buf_waveset_subs *x);
void			buf_waveset_subs_bang(t_buf_waveset_subs *x);
void			buf_waveset_subs_anything(t_buf_waveset_subs *x, t_symbol *msg, long ac, t_atom *av);

void buf_waveset_subs_assist(t_buf_waveset_subs *x, void *b, long m, long a, char *s);
void buf_waveset_subs_inletinfo(t_buf_waveset_subs *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(waveset_subs)

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
    
    CLASS_NEW_CHECK_SIZE(c, "ears.waveset.subs~",
                         (method)buf_waveset_subs_new,
                         (method)buf_waveset_subs_free,
                         sizeof(t_buf_waveset_subs),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    // @method list/llll @digest Function depends on inlet
    // @description A list or llll in the first inlet is supposed to contain buffer names and will
    // trigger the buffer processing and output the processed buffer names (depending on the <m>naming</m> attribute). <br />
    // A number or an llll in the second inlet is expected to contain a parameter (depending on the <m>mode</m>).
    // For "subs" mode, the parameter is the number of repetitions.
    EARSBUFOBJ_DECLARE_COMMON_METHODS_HANDLETHREAD(waveset_subs)

    // @method number @digest Set number of repetitions
    // @description A number in the second inlet sets the number of repeetitions.

    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_blocking_attr(c);
    earsbufobj_class_add_naming_attr(c);
    earsbufobj_class_add_resamplingfiltersize_attr(c);

    earsbufobj_class_add_polyout_attr(c);


    CLASS_ATTR_LONG(c, "span", 0, t_buf_waveset_subs, e_span);
    CLASS_ATTR_STYLE_LABEL(c,"span",0,"number","Group Wavesets");
    CLASS_ATTR_BASIC(c, "span", 0);
    // @description Sets the number of negative-to-positive zero crossing regions that form a waveset (defaults to 1: a single
    // negative-to-positive zero-crossing to negative-to-positive zero-crossing region).

    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
    return 0;
}

void buf_waveset_subs_assist(t_buf_waveset_subs *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        if (a == 0)
            sprintf(s, "symbol/list/llll: Incoming Buffer Names"); // @in 0 @type symbol/list/llll @digest Incoming buffer names
        else
            sprintf(s, "symbol: Buffer with Waveform to Substitute"); // @in 1 @type symbol @digest Buffer with waveform to substitute
    } else {
        sprintf(s, "symbol/list: Output Buffer Names"); // @out 0 @type symbol/list @digest Output buffer names
    }
}

void buf_waveset_subs_inletinfo(t_buf_waveset_subs *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_waveset_subs *buf_waveset_subs_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_waveset_subs *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_waveset_subs*)object_alloc_debug(s_tag_class);
    if (x) {
        x->e_span = 1;
        
        earsbufobj_init((t_earsbufobj *)x,  EARSBUFOBJ_FLAG_SUPPORTS_COPY_NAMES);
        
        // @arg 0 @name outnames @optional 1 @type symbol
        // @digest Output buffer names
        // @description @copy EARS_DOC_OUTNAME_ATTR

        t_llll *args = llll_parse(true_ac, argv);
        t_llll *names = earsbufobj_extract_names_from_args((t_earsbufobj *)x, args);
        
        attr_args_process(x, argc, argv);
        
        earsbufobj_setup((t_earsbufobj *)x, "Ee", "E", names);

        llll_free(args);
        llll_free(names);
    }
    return x;
}


void buf_waveset_subs_free(t_buf_waveset_subs *x)
{
    earsbufobj_free((t_earsbufobj *)x);
}



void buf_waveset_subs_bang(t_buf_waveset_subs *x)
{
    long num_buffers = earsbufobj_get_instore_size((t_earsbufobj *)x, 0);
    long group = x->e_span;
    long resamplingfiltersize = x->e_ob.l_resamplingfilterwidth;
    t_buffer_obj *waveform = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 1, 0);

    if (!waveform || ears_buffer_get_size_samps((t_object *)x, waveform) <= 0 || ears_buffer_get_numchannels((t_object *)x, waveform) != 1) {
        object_error((t_object *)x, "Invalid waveform introduced: must be a mono, non-empty buffer.");
        return;
    }
    
    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_buffers, true);
    
    earsbufobj_mutex_lock((t_earsbufobj *)x);
    earsbufobj_init_progress((t_earsbufobj *)x, num_buffers);
    
    for (long count = 0; count < num_buffers; count++) {
        t_buffer_obj *in = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, count);
        t_buffer_obj *out = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, count);
        ears_buffer_waveset_subs((t_object *)x, in, out, waveform, group, resamplingfiltersize);
        
        if (earsbufobj_iter_progress((t_earsbufobj *)x, count, num_buffers)) break;
    }
    earsbufobj_mutex_unlock((t_earsbufobj *)x);
    
    earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
}


void buf_waveset_subs_anything(t_buf_waveset_subs *x, t_symbol *msg, long ac, t_atom *av)
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
            
            buf_waveset_subs_bang(x);
            
        } else if (inlet == 1) {
            if (hatom_gettype(&parsed->l_head->l_hatom) == H_SYM) {
                earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 1, 1, true);
                earsbufobj_store_buffer((t_earsbufobj *)x, EARSBUFOBJ_IN, 1, 0, hatom_getsym(&parsed->l_head->l_hatom));
            }
        }
    }
    llll_free(parsed);
}


