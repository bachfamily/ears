/**
    @file
    ears.waveset.split.c
 
    @name
    ears.waveset.split~
 
    @realname
    ears.waveset.split~
 
    @type
    object
 
    @module
    ears

    @author
    Daniele Ghisi
 
    @digest
    Waveset splitting
 
    @description
    Splits buffer according to zero-crossings
 
    @discussion
 
    @category
    ears basic
 
    @keywords
    buffer, waveset, zero-crossing
 
    @seealso
    ears.waveset.repeat~, ears.waveset.subs~, ears.split~
    
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

typedef struct _buf_waveset_split {
    t_earsbufobj       e_ob;
    long               e_span;
    long               e_normalize;
    t_llll             *args;
} t_buf_waveset_split;



// Prototypes
t_buf_waveset_split*         buf_waveset_split_new(t_symbol *s, short argc, t_atom *argv);
void            buf_waveset_split_free(t_buf_waveset_split *x);
void            buf_waveset_split_bang(t_buf_waveset_split *x);
void            buf_waveset_split_anything(t_buf_waveset_split *x, t_symbol *msg, long ac, t_atom *av);

void buf_waveset_split_assist(t_buf_waveset_split *x, void *b, long m, long a, char *s);
void buf_waveset_split_inletinfo(t_buf_waveset_split *x, void *b, long a, char *t);


// Globals and Statics
static t_class    *s_tag_class = NULL;
static t_symbol    *ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(waveset_split)

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
    
    CLASS_NEW_CHECK_SIZE(c, "ears.waveset.split~",
                         (method)buf_waveset_split_new,
                         (method)buf_waveset_split_free,
                         sizeof(t_buf_waveset_split),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    // @method list/llll @digest Function depends on inlet
    // @description A list or llll in the first inlet is supposed to contain buffer names and will
    // trigger the buffer processing and output the processed buffer names (depending on the <m>naming</m> attribute). <br />
    // A number or an llll in the second inlet is expected to contain a parameter (depending on the <m>mode</m>).
    // For "split" mode, the parameter is the number of repetitions.
    EARSBUFOBJ_DECLARE_COMMON_METHODS_HANDLETHREAD(waveset_split)

    // @method number @digest Set waveset
    // @description A number in the second inlet sets the waveset parameter (depending on the <m>ampunit</m>).

    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_blocking_attr(c);
    earsbufobj_class_add_naming_attr(c);

    earsbufobj_class_add_polyout_attr(c);


    CLASS_ATTR_LONG(c, "span", 0, t_buf_waveset_split, e_span);
    CLASS_ATTR_STYLE_LABEL(c,"span",0,"number","Group Wavesets");
    CLASS_ATTR_BASIC(c, "span", 0);
    // @description Sets the number of negative-to-positive zero crossing regions that form a waveset (defaults to 1: a single
    // negative-to-positive zero-crossing to negative-to-positive zero-crossing region).

    CLASS_ATTR_LONG(c, "normalize", 0, t_buf_waveset_split, e_normalize);
    CLASS_ATTR_STYLE_LABEL(c,"normalize",0,"onoff","Normalize Wavesets");
    CLASS_ATTR_BASIC(c, "normalize", 0);
    // @description Toggles the normalization of wavesets.
    
    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
}

void buf_waveset_split_assist(t_buf_waveset_split *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        if (a == 0)
            sprintf(s, "symbol/list/llll: Incoming Buffer Names"); // @in 0 @type symbol/list/llll @digest Incoming buffer names
    } else {
        sprintf(s, "symbol/list: Output Buffer Names"); // @out 0 @type symbol/list @digest Output buffer names
    }
}

void buf_waveset_split_inletinfo(t_buf_waveset_split *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_waveset_split *buf_waveset_split_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_waveset_split *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_waveset_split*)object_alloc_debug(s_tag_class);
    if (x) {
        x->args = llll_from_text_buf("1", false);
        x->e_span = 1;
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
        
        earsbufobj_setup((t_earsbufobj *)x, "E", "E", names);

        llll_free(args);
        llll_free(names);
    }
    return x;
}


void buf_waveset_split_free(t_buf_waveset_split *x)
{
    llll_free(x->args);
    earsbufobj_free((t_earsbufobj *)x);
}



void buf_waveset_split_bang(t_buf_waveset_split *x)
{
    long num_buffers = earsbufobj_get_instore_size((t_earsbufobj *)x, 0);
    long group = x->e_span;
    long normalize = x->e_normalize;
    
    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_buffers, true);
    
    earsbufobj_mutex_lock((t_earsbufobj *)x);
    earsbufobj_init_progress((t_earsbufobj *)x, num_buffers);

    // dry run to count wavesets
    std::vector<long> numwavesets;
    long totnumwaveset = 0;
    for (long count = 0; count < num_buffers; count++) {
        t_buffer_obj *in = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, count);
        long thisnum = 0;
        for (long c = 0; c < ears_buffer_get_numchannels((t_object *)x, in); c++) {
            ears_buffer_waveset_getnum((t_object *)x, in, c, group, &thisnum);
            numwavesets.push_back(thisnum);
            totnumwaveset += thisnum;
        }
    }
    
    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_OUT, 0, totnumwaveset, true);

    // actual cropping
    for (long count = 0; count < num_buffers; count++) {
        t_buffer_obj *in = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, count);
        long thisnum = 0;
        long cur = 0;
        long channelcount = ears_buffer_get_numchannels((t_object *)x, in);
        for (long c = 0; c < channelcount; c++) {
            long this_numwaveset = numwavesets[cur];
            if (this_numwaveset > 0) {
                t_buffer_obj **outs = (t_buffer_obj **)bach_newptrclear(this_numwaveset * sizeof(t_buffer_obj *));
                for (long i = thisnum; i < thisnum+this_numwaveset; i++)
                    outs[i-thisnum] = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, i);
                ears_buffer_waveset_split((t_object *)x, in, outs, c, this_numwaveset, group, normalize);
                bach_freeptr(outs);
            }
            cur++;
            thisnum += this_numwaveset;
        }
        if (earsbufobj_iter_progress((t_earsbufobj *)x, count, num_buffers)) break;
    }

    earsbufobj_mutex_unlock((t_earsbufobj *)x);
    
    earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
}


void buf_waveset_split_anything(t_buf_waveset_split *x, t_symbol *msg, long ac, t_atom *av)
{
    long inlet = earsbufobj_proxy_getinlet((t_earsbufobj *) x);

    t_llll *parsed = earsbufobj_parse_gimme((t_earsbufobj *) x, LLLL_OBJ_VANILLA, msg, ac, av);
    if (!parsed) return;
    
    if (parsed && parsed->l_head) {
        if (inlet == 0) {
            long num_bufs = llll_get_num_symbols_root(parsed);

            earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_bufs, true);
            
            earsbufobj_store_buffer_list((t_earsbufobj *)x, parsed, 0);
            
            buf_waveset_split_bang(x);
        }
    }
    llll_free(parsed);
}


