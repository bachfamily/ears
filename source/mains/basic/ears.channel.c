/**
	@file
	ears.channel.c
 
	@name
	ears.channel~
 
	@realname
	ears.channel~
 
	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	Get buffer channels
 
	@description
	Extracts a single channel or a set of channels from input buffers
 
	@discussion
 
	@category
	ears basic
 
	@keywords
	buffer, get, channel, pick, nth, obtain, extract
 
	@seealso
	ears.pack~
	
	@owner
	Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "foundation/llllobj.h"
#include "foundation/llll_commons_ext.h"
#include "math/bach_math_utilities.h"
#include "ears.object.h"



typedef struct _buf_channel {
    t_earsbufobj       e_ob;
    
    t_llll             *channel;
    char               all_channels_mode;
} t_buf_channel;



// Prototypes
t_buf_channel*         buf_channel_new(t_symbol *s, short argc, t_atom *argv);
void			buf_channel_free(t_buf_channel *x);
void			buf_channel_bang(t_buf_channel *x);
void			buf_channel_anything(t_buf_channel *x, t_symbol *msg, long ac, t_atom *av);

void buf_channel_assist(t_buf_channel *x, void *b, long m, long a, char *s);
void buf_channel_inletinfo(t_buf_channel *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;


EARSBUFOBJ_ADD_IO_METHODS(channel)


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
    
    CLASS_NEW_CHECK_SIZE(c, "ears.channel~",
                         (method)buf_channel_new,
                         (method)buf_channel_free,
                         sizeof(t_buf_channel),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    // @method list/llll @digest Process buffers
    // @description A list or llll with buffer names will trigger the buffer processing and output the processed
    // buffer names (depending on the <m>naming</m> attribute).
    EARSBUFOBJ_DECLARE_COMMON_METHODS_HANDLETHREAD(channel)
    
    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_blocking_attr(c);
    earsbufobj_class_add_naming_attr(c);
    earsbufobj_class_add_polyout_attr(c);

    CLASS_ATTR_CHAR(c, "all", 0, t_buf_channel, all_channels_mode);
    CLASS_ATTR_STYLE_LABEL(c,"all",0,"onoff","All-channels mode");
    CLASS_ATTR_BASIC(c, "all", 0);
    // @description Toggles the ability to output all the channels of the first incoming buffer as separate buffers.

    
    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
}

void buf_channel_assist(t_buf_channel *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        if (a == 0)
            sprintf(s, "symbol/list/llll: Incoming Buffer Names"); // @in 0 @type symbol/list/llll @digest Incoming buffer names
        else
            sprintf(s, "number/llll: Channel(s)"); // @in 1 @type number/llll @digest Numbers of channels to extract
    } else {
        sprintf(s, "symbol/list: Output Buffer Names"); // @out 0 @type symbol/list @digest Output buffer names
    }
}

void buf_channel_inletinfo(t_buf_channel *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_channel *buf_channel_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_channel *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_channel*)object_alloc_debug(s_tag_class);
    if (x) {
        x->channel = llll_from_text_buf("1", false);
        
        // @arg 0 @name outnames @optional 1 @type symbol
        // @digest Output buffer names
        // @description @copy EARS_DOC_OUTNAME_ATTR

        // @arg 1 @name channels @optional 1 @type number/list
        // @digest Channel(s) to extract
        // @description Sets the number of channel or the list of channels to be extracted

        earsbufobj_init((t_earsbufobj *)x, 0);

        t_llll *args = llll_parse(true_ac, argv);
        t_llll *names = earsbufobj_extract_names_from_args((t_earsbufobj *)x, args);
        
        if (args) {
            llll_free(x->channel);
            x->channel = llll_clone(args);
        }

        attr_args_process(x, argc, argv);

        earsbufobj_setup((t_earsbufobj *)x, "E4", "E", names);

        llll_free(args);
        llll_free(names);
    }
    return x;
}


void buf_channel_free(t_buf_channel *x)
{
    llll_free(x->channel);
    earsbufobj_free((t_earsbufobj *)x);
}


void buf_channel_bang(t_buf_channel *x)
{
    long num_buffers = earsbufobj_get_instore_size((t_earsbufobj *)x, 0);
    t_llll *channel = NULL;
    
    earsbufobj_mutex_lock((t_earsbufobj *)x);
    channel = llll_clone(x->channel);
    earsbufobj_mutex_unlock((t_earsbufobj *)x);

    char all_channels_mode = x->all_channels_mode;
    
    if (all_channels_mode) {
        // only process first incoming buffer and output all of its channels one by one
        if (num_buffers > 0) {
            t_buffer_obj *in = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, 0);
            long num_channels = ears_buffer_get_numchannels((t_object *)x, in);
            
            earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_OUT, 0, num_channels, true);
            earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
            
            for (long count = 0; count < num_channels; count++) {
                t_llll *temp = llll_get();
                llll_appendlong(temp, count + 1);
                t_buffer_obj *out = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, count);
                ears_buffer_extractchannels_from_llll((t_object *)x, in, out, temp);
                llll_free(temp);
            }

            if (num_buffers > 1) {
                object_warn((t_object *)x, "More than one buffer input while object is in 'all channels' mode:");
                object_warn((t_object *)x, "    buffers other than the first one are ignored.");
            }
        }
        
    } else {
        earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
        earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_buffers, true);
        
        earsbufobj_init_progress((t_earsbufobj *)x, num_buffers);

        for (long count = 0; count < num_buffers; count++) {
            t_buffer_obj *in = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, count);
            t_buffer_obj *out = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, count);
            
            ears_buffer_extractchannels_from_llll((t_object *)x, in, out, channel);

            if (earsbufobj_iter_progress((t_earsbufobj *)x, count, num_buffers)) break;
        }
    }
    llll_free(channel);
    
    earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
}

void buf_channel_anything(t_buf_channel *x, t_symbol *msg, long ac, t_atom *av)
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
            
            buf_channel_bang(x);
            
        } else if (inlet == 1) {
            earsbufobj_mutex_lock((t_earsbufobj *)x);
            llll_free(x->channel);
            x->channel = llll_clone(parsed);
            earsbufobj_mutex_unlock((t_earsbufobj *)x);
        }
    }
    llll_free(parsed);
}


