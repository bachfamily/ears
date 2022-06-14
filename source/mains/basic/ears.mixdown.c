/**
	@file
	ears.mixdown.c
 
	@name
	ears.mixdown~
 
	@realname
	ears.mixdown~
 
	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	Reduce number of channels
 
	@description
	Reduce the number of channel by downmixing them
 
	@discussion
 
	@category
	ears basic
 
	@keywords
	buffer, mixdown, downmix, mix, down, reduce, pan
 
	@seealso
	ears.mix~, ears.pan~
	
	@owner
	Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "llllobj.h"
#include "llll_commons_ext.h"
#include "bach_math_utilities.h"
#include "ears.object.h"

typedef struct _buf_mixdown {
    t_earsbufobj       e_ob;
    long               numchannels;
    char               autogain;
    char               channelmode_downmix;

} t_buf_mixdown;



// Prototypes
t_buf_mixdown*     buf_mixdown_new(t_symbol *s, short argc, t_atom *argv);
void			buf_mixdown_free(t_buf_mixdown *x);
void			buf_mixdown_bang(t_buf_mixdown *x);
void			buf_mixdown_anything(t_buf_mixdown *x, t_symbol *msg, long ac, t_atom *av);

void buf_mixdown_assist(t_buf_mixdown *x, void *b, long m, long a, char *s);
void buf_mixdown_inletinfo(t_buf_mixdown *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(mixdown)

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
    
    CLASS_NEW_CHECK_SIZE(c, "ears.mixdown~",
                         (method)buf_mixdown_new,
                         (method)buf_mixdown_free,
                         sizeof(t_buf_mixdown),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    
    // @method list/llll @digest Process buffers
    // @description A list or llll with buffer names will trigger the buffer processing and output the processed
    // buffer names (depending on the <m>naming</m> attribute).
    EARSBUFOBJ_DECLARE_COMMON_METHODS_HANDLETHREAD(mixdown)
    
    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_blocking_attr(c);
    earsbufobj_class_add_naming_attr(c);
    
    earsbufobj_class_add_polyout_attr(c);

    CLASS_ATTR_LONG(c, "numchannels",    0,    t_buf_mixdown, numchannels);
    CLASS_ATTR_BASIC(c, "numchannels", 0);
    CLASS_ATTR_STYLE_LABEL(c, "numchannels", 0, "text", "Number Of Output Channels");
    // @description Sets the number of output channels (defaults to 2).

    CLASS_ATTR_CHAR(c, "autogain",    0,    t_buf_mixdown, autogain);
    CLASS_ATTR_BASIC(c, "autogain", 0);
    CLASS_ATTR_STYLE_LABEL(c, "autogain", 0, "onoff", "Gain Compensation");
    // @description Compensate the gain according to the ratio between input channels and output channels.

    CLASS_ATTR_CHAR(c, "channelmode",    0,    t_buf_mixdown, channelmode_downmix);
    CLASS_ATTR_STYLE_LABEL(c, "channelmode", 0, "enumindex", "Down-Mixing Channel Conversion");
    CLASS_ATTR_ENUMINDEX(c,"channelmode", 0, "Clear Keep Pad Cycle Palindrome Pan");
    // @description Sets the channel conversion mode while downmixing: <br />
    // 0 (Clear) = Delete all samples <br />
    // 1 (Keep) = Only keep existing channels <br />
    // 2 (Pad) = Pad last channel (only useful while upmixing, useless for <o>ears.mixdown~</o>) <br />
    // 3 (Cycle, default) = Repeat all channels (cycling) while upmixing <br />
    // 4 (Palindrome) = Palindrome cycling of channels while upmixing <br />
    // 5 (Pan) = Pan channels to new configuration <br />
    
    
    
    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
    return 0;
}

void buf_mixdown_assist(t_buf_mixdown *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        if (a == 0) // @in 0 @type symbol @digest Buffer name
            sprintf(s, "symbol: Buffer Names");
        else if (a == 1) // @out 1 @type number @digest Number of output channels
            sprintf(s, "int: Number of Output Channels");
    } else {
        sprintf(s, "Output Buffer Names"); // @out 0 @type symbol/list @digest Output buffer names(s)
                                            // @description Name of the output buffer
    }
}

void buf_mixdown_inletinfo(t_buf_mixdown *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_mixdown *buf_mixdown_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_mixdown *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_mixdown*)object_alloc_debug(s_tag_class);
    if (x) {
        x->channelmode_downmix = EARS_CHANNELCONVERTMODE_CYCLE;
        x->numchannels = 2; // stereo is default

        earsbufobj_init((t_earsbufobj *)x,  EARSBUFOBJ_FLAG_SUPPORTS_COPY_NAMES);

        // @arg 0 @name outname @optional 1 @type symbol
        // @digest Output buffer name
        // @description @copy EARS_DOC_OUTNAME_ATTR
        
        t_llll *args = llll_parse(true_ac, argv);
        t_llll *names = earsbufobj_extract_names_from_args((t_earsbufobj *)x, args);
        
        // @arg 1 @name numchannels @optional 1 @type int
        // @digest Number of Output Channels
        // @description Sets the initial number of output channels.
        
        if (args && args->l_head) {
            x->numchannels = hatom_getlong(&args->l_head->l_hatom);
            if (x->numchannels <= 0) {
                object_error((t_object *)x, "The number of channels must be at least one. Defaulting to stereo.");
                x->numchannels = 2;
            }
        }

        attr_args_process(x, argc, argv);

        earsbufobj_setup((t_earsbufobj *)x, "Ei", "E", names);

        llll_free(args);
        llll_free(names);
    }
    return x;
}


void buf_mixdown_free(t_buf_mixdown *x)
{
    earsbufobj_free((t_earsbufobj *)x);
}



void buf_mixdown_bang(t_buf_mixdown *x)
{
    long num_buffers = earsbufobj_get_instore_size((t_earsbufobj *)x, 0);
    long channelmode_downmix = x->channelmode_downmix;
    long channelmode_upmix = EARS_CHANNELCONVERTMODE_CYCLE;
    long numchannels = x->numchannels;
    char autogain = x->autogain;
    
    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_buffers, true);
    
    earsbufobj_mutex_lock((t_earsbufobj *)x);
    earsbufobj_init_progress((t_earsbufobj *)x, num_buffers);
    
    for (long count = 0; count < num_buffers; count++) {
        t_buffer_obj *in = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, count);
        t_buffer_obj *out = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, count);
        
        long curr_num_channels = ears_buffer_get_numchannels((t_object *)x, in);
        if (curr_num_channels != numchannels) {
            ears_buffer_clone((t_object *)x, in, out);
            ears_buffer_convert_numchannels((t_object *)x, out, numchannels, (e_ears_channel_convert_modes)channelmode_upmix, (e_ears_channel_convert_modes)channelmode_downmix);
            if (autogain)
                ears_buffer_gain((t_object *)x, out, out, numchannels*1./curr_num_channels, false);
        } else {
            ears_buffer_clone((t_object *)x, in, out);
        }

        if (earsbufobj_iter_progress((t_earsbufobj *)x, count, num_buffers)) break;
    }
    earsbufobj_mutex_unlock((t_earsbufobj *)x);

    earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
}



void buf_mixdown_anything(t_buf_mixdown *x, t_symbol *msg, long ac, t_atom *av)
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
                
                buf_mixdown_bang(x);
            }
        } else {
            earsbufobj_mutex_lock((t_earsbufobj *)x);
            if (is_hatom_number(&parsed->l_head->l_hatom)) {
                x->numchannels = hatom_getlong(&parsed->l_head->l_hatom);
                if (x->numchannels <= 0) {
                    object_error((t_object *)x, "The number of channels must be at least one. Defaulting to stereo.");
                    x->numchannels = 2;
                }
            }
            earsbufobj_mutex_unlock((t_earsbufobj *)x);
        }
    }
    llll_free(parsed);
}


