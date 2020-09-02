/**
	@file
	ears.format.c
 
	@name
	ears.format~
 
	@realname
	ears.format~
 
	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	Modify buffers properties
 
	@description
	Changes number of channels, duration and sample rate
 
	@discussion
 
	@category
	ears buffer operations
 
	@keywords
	buffer, format, property, change
 
	@seealso
	ears.info~, ears.normalize~, ears.reg~, ears.resample~
	
	@owner
	Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "llllobj.h"
#include "llll_commons_ext.h"
#include "bach_math_utilities.h"
#include "ears.object.h"



typedef struct _buf_format {
    t_earsbufobj        e_ob;
    
    long                numchannels;
    double              duration;
    double              sr;
    
    char                resample;
    char                channelmode;
    
    double             mix;
} t_buf_format;




// Prototypes
t_buf_format*         buf_format_new(t_symbol *s, short argc, t_atom *argv);
void			buf_format_free(t_buf_format *x);
void			buf_format_bang(t_buf_format *x);
void			buf_format_anything(t_buf_format *x, t_symbol *msg, long ac, t_atom *av);

void buf_format_assist(t_buf_format *x, void *b, long m, long a, char *s);
void buf_format_inletinfo(t_buf_format *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(format)


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
    
    CLASS_NEW_CHECK_SIZE(c, "ears.format~",
                         (method)buf_format_new,
                         (method)buf_format_free,
                         sizeof(t_buf_format),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    // @method list/llll @digest Process buffers
    // @description A list or llll with buffer names will trigger the buffer processing and output the processed
    // buffer names (depending on the <m>naming</m> attribute).
    EARSBUFOBJ_DECLARE_COMMON_METHODS_DEFER(format)

    
    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_timeunit_attr(c);
    earsbufobj_class_add_naming_attr(c);
    
    
    CLASS_ATTR_LONG(c, "numchannels",	0,	t_buf_format, numchannels);
    CLASS_ATTR_STYLE_LABEL(c, "numchannels", 0, "text", "Number Of Output Channels");
    // @description Sets the number of output channels. 0 means: don't change.
    
    CLASS_ATTR_DOUBLE(c, "duration",	0,	t_buf_format, duration);
    CLASS_ATTR_STYLE_LABEL(c, "duration", 0, "text", "Duration");
    // @description Sets the buffer duration (unit given by the <m>timeunit</m> attribute)
    // 0 means: don't change.
    
    CLASS_ATTR_DOUBLE(c, "sr",	0,	t_buf_format, sr);
    CLASS_ATTR_STYLE_LABEL(c, "sr", 0, "text", "Sample Rate");
    // @description Sets the sample rate for the buffer
    // 0 means: don't change.
    
    CLASS_ATTR_CHAR(c, "resample",	0,	t_buf_format, resample);
    CLASS_ATTR_STYLE_LABEL(c, "resample", 0, "onoff", "Resample Buffers");
    // @description Toggles the ability to resample buffers when the sample rate has changed.
    
    CLASS_ATTR_CHAR(c, "channelmode",	0,	t_buf_format, channelmode);
    CLASS_ATTR_STYLE_LABEL(c, "channelmode", 0, "enumindex", "Channel Conversion Mode");
    CLASS_ATTR_ENUMINDEX(c,"channelmode", 0, "Clear Keep Pad Cycle Palindrome Pan");
    // @description Sets the channel conversion mode: <br />
    // 0 (Clear) = Delete all samples <br />
    // 1 (Keep) = Only keep existing channels <br />
    // 2 (Pad) = Pad last channel while upmixing <br />
    // 3 (Cycle, default) = Repeat all channels (cycling) while upmixing <br />
    // 4 (Palindrome) = Palindrome cycling of channels while upmixing <br />
    // 5 (Pan) = Pan channels to new configuration <br />
    
    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
    return 0;
}

void buf_format_assist(t_buf_format *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        sprintf(s, "symbol/list/llll: Incoming Buffer Names"); // @in 0 @type symbol/list/llll @digest Incoming buffer names
    } else {
        sprintf(s, "symbol/list: Formatted Buffer Names"); // @out 0 @type symbol/list @digest Formatted buffer names
    }
}

void buf_format_inletinfo(t_buf_format *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_format *buf_format_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_format *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_format*)object_alloc_debug(s_tag_class);
    if (x) {
        x->numchannels = 0;
        x->duration = 0;
        x->sr = 0;
        x->resample = true;
        x->channelmode = EARS_CHANNELCONVERTMODE_CYCLE;
        
        earsbufobj_init((t_earsbufobj *)x, EARSBUFOBJ_FLAG_SUPPORTS_COPY_NAMES);

        // @arg 0 @name outnames @optional 1 @type symbol
        // @digest Output buffer names
        // @description @copy EARS_DOC_OUTNAME_ATTR
        
        t_llll *args = llll_parse(true_ac, argv);
        t_llll *names = earsbufobj_extract_names_from_args((t_earsbufobj *)x, args);
        
        attr_args_process(x, argc, argv);

        earsbufobj_setup((t_earsbufobj *)x, "E", "E", names);

        llll_free(args);
        llll_free(names);
    }
    return x;
}


void buf_format_free(t_buf_format *x)
{
    earsbufobj_free((t_earsbufobj *)x);
}




void buf_format_bang(t_buf_format *x)
{
    long num_buffers = ((t_earsbufobj *)x)->l_instore[0].num_stored_bufs;
    double sr = x->sr;
    double duration = x->duration;
    long numchannels = x->numchannels;
    long channelmode = x->channelmode;
    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_buffers, true);
    
    for (long count = 0; count < num_buffers; count++) {
        t_buffer_obj *in = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, count);
        t_buffer_obj *out = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, count);
        
        if (in != out)
            ears_buffer_clone((t_object *)x, in, out);
        
        if (sr > 0) {
            double curr_sr = ears_buffer_get_sr((t_object *)x, out);
            if (curr_sr != sr) {
                if (x->resample)
                    ears_buffer_convert_sr((t_object *)x, out, x->sr);
                else
                    ears_buffer_set_sr((t_object *)x, out, x->sr);
            }
        }
        
        if (duration > 0) {
            long duration_samps = earsbufobj_input_to_samps((t_earsbufobj *)x, duration, out);
            ears_buffer_convert_size((t_object *)x, out, duration_samps);
        }
        
        if (numchannels > 0) {
            long curr_num_channels = ears_buffer_get_numchannels((t_object *)x, out);
            if (curr_num_channels != numchannels)
                ears_buffer_convert_numchannels((t_object *)x, out, numchannels, (e_ears_channel_convert_modes)channelmode);
        }
    }
    
    earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
}


void buf_format_anything(t_buf_format *x, t_symbol *msg, long ac, t_atom *av)
{
    long inlet = proxy_getinlet((t_object *) x);
    
    t_llll *parsed = earsbufobj_parse_gimme((t_earsbufobj *) x, LLLL_OBJ_VANILLA, msg, ac, av);
    if (!parsed) return;
    
    if (parsed && parsed->l_head) {
        if (inlet == 0) {
            
            earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, parsed->l_size, true);
            earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_OUT, 0, parsed->l_size, true);
            earsbufobj_store_buffer_list((t_earsbufobj *)x, parsed, 0, true);
            
            buf_format_bang(x);
        }
    }
    llll_free(parsed);
}


