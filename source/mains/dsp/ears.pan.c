/**
	@file
	ears.pan.c
 
	@name
	ears.pan~
 
	@realname
	ears.pan~
 
	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	1D buffer panning
 
	@description
	Modify the panoramics of a given buffer
 
	@discussion
 
	@category
	ears pan
 
	@keywords
	buffer, pan, panoramics, stereo, mono
 
	@seealso
	ears.mixdown~, ears.gain~
	
	@owner
	Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "llllobj.h"
#include "llll_commons_ext.h"
#include "bach_math_utilities.h"
#include "ears.object.h"



typedef struct _buf_pan {
    t_earsbufobj       e_ob;
    
    t_llll             *pan;
    
    long                num_out_channels;
    long                pan_mode;
    long                pan_law;
    double              multichannel_spread;
    char                compensate_multichannel_gain_to_avoid_clipping;

    long                pan_rangetype;
    double              pan_range[2];

} t_buf_pan;


enum {
    EARS_PAN_RANGETYPE_CUSTOM = 0,
    EARS_PAN_RANGETYPE_0_1 = 1,
    EARS_PAN_RANGETYPE_m1_1 = 2,
    EARS_PAN_RANGETYPE_NUM_0based = 3,
    EARS_PAN_RANGETYPE_NUM_1based = 4,
};


// Prototypes
t_buf_pan*         buf_pan_new(t_symbol *s, short argc, t_atom *argv);
void			buf_pan_free(t_buf_pan *x);
void			buf_pan_bang(t_buf_pan *x);
void			buf_pan_anything(t_buf_pan *x, t_symbol *msg, long ac, t_atom *av);

void buf_pan_assist(t_buf_pan *x, void *b, long m, long a, char *s);
void buf_pan_inletinfo(t_buf_pan *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(pan)

/**********************************************************************/
// Class Definition and Life Cycle


t_max_err earsbufobj_setattr_numouts(t_buf_pan *x, void *attr, long argc, t_atom *argv)
{
    if (argc && argv) {
        if (atom_gettype(argv) == A_LONG) {
            x->num_out_channels = atom_getlong(argv);
            if (x->pan_rangetype == EARS_PAN_RANGETYPE_NUM_0based) {
                x->pan_range[0] = 0;
                x->pan_range[1] = x->num_out_channels - 1;
            } else if (x->pan_rangetype == EARS_PAN_RANGETYPE_NUM_1based) {
                x->pan_range[0] = 1;
                x->pan_range[1] = x->num_out_channels ;
            }
        }
    }
    return MAX_ERR_NONE;
}


t_max_err earsbufobj_setattr_rangetype(t_buf_pan *x, void *attr, long argc, t_atom *argv)
{
    if (argc && argv) {
        if (atom_gettype(argv) == A_LONG) {
            x->pan_rangetype = atom_getlong(argv);
            switch (x->pan_rangetype) {
                case EARS_PAN_RANGETYPE_0_1:
                    x->pan_range[0] = 0;
                    x->pan_range[1] = 1;
                    break;

                case EARS_PAN_RANGETYPE_m1_1:
                    x->pan_range[0] = -1;
                    x->pan_range[1] = 1;
                    break;
                
                case EARS_PAN_RANGETYPE_NUM_0based:
                    x->pan_range[0] = 0;
                    x->pan_range[1] = x->pan_mode == EARS_PAN_MODE_LINEAR ? x->num_out_channels - 1: x->num_out_channels;
                    break;

                case EARS_PAN_RANGETYPE_NUM_1based:
                    x->pan_range[0] = 1;
                    x->pan_range[1] = x->pan_mode == EARS_PAN_MODE_LINEAR ? x->num_out_channels : x->num_out_channels + 1;
                    break;

                default:
                    break;
            }
            
            object_attr_setdisabled((t_object *)x, gensym("range"), x->pan_rangetype != EARS_PAN_RANGETYPE_CUSTOM);
        }
    }
    return MAX_ERR_NONE;
}


t_max_err earsbufobj_setattr_mode(t_buf_pan *x, void *attr, long argc, t_atom *argv)
{
    if (argc && argv) {
        if (atom_gettype(argv) == A_LONG) {
            x->pan_mode = atom_getlong(argv);
        } else if (atom_gettype(argv) == A_SYM) {
            if (atom_getsym(argv) == _sym_linear)
                x->pan_mode = EARS_PAN_MODE_LINEAR;
            else if (atom_getsym(argv) == gensym("circular"))
                x->pan_mode = EARS_PAN_MODE_CIRCULAR;
        }
        if (x->pan_rangetype == EARS_PAN_RANGETYPE_NUM_0based) {
            x->pan_range[0] = 0;
            x->pan_range[1] = x->pan_mode == EARS_PAN_MODE_LINEAR ? x->num_out_channels - 1: x->num_out_channels;
        } else  if (x->pan_rangetype == EARS_PAN_RANGETYPE_NUM_1based) {
            x->pan_range[0] = 1;
            x->pan_range[1] = x->pan_mode == EARS_PAN_MODE_LINEAR ? x->num_out_channels: x->num_out_channels + 1;
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
    
    CLASS_NEW_CHECK_SIZE(c, "ears.pan~",
                         (method)buf_pan_new,
                         (method)buf_pan_free,
                         sizeof(t_buf_pan),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    // @method list/llll @digest Process buffers
    // @description A list or llll with buffer names will trigger the buffer processing and output the processed
    // buffer names (depending on the <m>naming</m> attribute).
    EARSBUFOBJ_DECLARE_COMMON_METHODS_HANDLETHREAD(pan)
    
    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_blocking_attr(c);
    earsbufobj_class_add_envtimeunit_attr(c);
    earsbufobj_class_add_naming_attr(c);
    earsbufobj_class_add_slopemapping_attr(c);

    earsbufobj_class_add_polyout_attr(c);

    CLASS_ATTR_LONG(c, "numchannels", 0, t_buf_pan, num_out_channels);
    CLASS_ATTR_STYLE_LABEL(c,"numchannels",0,"text","Number of Output Channels");
    CLASS_ATTR_ACCESSORS(c, "numchannels", NULL, earsbufobj_setattr_numouts);
    CLASS_ATTR_BASIC(c, "numchannels", 0);
    // @description Sets the number of channels (loudspeakers) for panning


    CLASS_ATTR_LONG(c, "rangetype", 0, t_buf_pan, pan_rangetype);
    CLASS_ATTR_STYLE_LABEL(c,"rangetype",0,"text","Range Type");
    CLASS_ATTR_ENUMINDEX(c,"rangetype", 0, "Custom 0 to 1 -1 to 1 Loudspeaker Number (0-based) Loudspeaker Number (1-based)");
    CLASS_ATTR_ACCESSORS(c, "rangetype", NULL, earsbufobj_setattr_rangetype);
    CLASS_ATTR_BASIC(c, "rangetype", 0);
    // @description Sets the range type, i.e. the minimum and maximum value for the pan: <br />
    // 0: Custom <br />
    // 1: Between 0 and 1 (default) <br />
    // 2: Between -1 and 1 <br />
    // 3: Integers correspond to the number of loudspeaker (first loudspeaker is 0th)  <br />
    // 4: Integers correspond to the number of loudspeaker (first loudspeaker is 1st)  <br />

    CLASS_ATTR_DOUBLE_ARRAY(c, "range", 0, t_buf_pan, pan_range, 2);
    CLASS_ATTR_STYLE_LABEL(c,"range",0,"text","Range");
    // @description Sets the pan range. You can use the <m>rangetype</m> attribute instead, for the simplest cases.
    

    CLASS_ATTR_LONG(c, "panmode", 0, t_buf_pan, pan_mode);
    CLASS_ATTR_STYLE_LABEL(c,"panmode",0,"text","Pan Mode");
    CLASS_ATTR_ENUMINDEX(c,"panmode", 0, "Linear Circular");
    CLASS_ATTR_ACCESSORS(c, "panmode", NULL, earsbufobj_setattr_mode);
    CLASS_ATTR_BASIC(c, "panmode", 0);
    // @description Sets the panning mode: 0 = linear (default); 1 = circular.

    
    CLASS_ATTR_LONG(c, "panlaw", 0, t_buf_pan, pan_law);
    CLASS_ATTR_STYLE_LABEL(c,"panlaw",0,"text","Pan Law");
    CLASS_ATTR_ENUMINDEX(c,"panlaw", 0, "Nearest Neighbor Cosine");
    // @description Sets the panning law: 0 = nearest neighbor (panned on one loudspeaker at a time);
    // 1 = cosine law (default).

    
    CLASS_ATTR_DOUBLE(c, "spread", 0, t_buf_pan, multichannel_spread);
    CLASS_ATTR_STYLE_LABEL(c,"spread",0,"text","Multichannel Spread");
    // @description Sets the spread of the input channels when panning multichannels buffers. Default is 0 (downmix to mono).
    // A spread of 1 spreads a multichannel signal sent to the central loudspeaker up to the external ones.

    CLASS_ATTR_CHAR(c, "compensate", 0, t_buf_pan, compensate_multichannel_gain_to_avoid_clipping);
    CLASS_ATTR_STYLE_LABEL(c,"compensate",0,"onoff","Reduce Multichannel Gain To Avoid Clipping");
    // @description Toggles the ability to automatically reduce the gain of multichannel files by a factor of the number of channels, in order
    // to avoid possible clipping while panning then with low <m>spread</m> values. Defaults to 1.

    
    
    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
    return 0;
}

void buf_pan_assist(t_buf_pan *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        if (a == 0)
            sprintf(s, "symbol/list/llll: Incoming Buffer Names"); // @in 0 @type symbol/list/llll @digest Incoming buffer names
        else
            sprintf(s, "number/llll/symbol: pan position"); // @in 1 @type number @digest Pan position, envelope or buffer
    } else {
        sprintf(s, "symbol/list: Output Buffer Names"); // @out 0 @type symbol/list @digest Output buffer names
    }
}

void buf_pan_inletinfo(t_buf_pan *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_pan *buf_pan_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_pan *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_pan*)object_alloc_debug(s_tag_class);
    if (x) {
        x->pan = llll_from_text_buf("0.5", false);
        x->pan_rangetype = EARS_PAN_RANGETYPE_0_1;
        x->pan_range[0] = 0;
        x->pan_range[1] = 1;
        x->num_out_channels = 2;
        x->pan_mode = EARS_PAN_MODE_LINEAR;
        x->pan_law = EARS_PAN_LAW_COSINE;
        x->multichannel_spread = 0.;
        x->compensate_multichannel_gain_to_avoid_clipping = true;
        
        // @arg 0 @name outnames @optional 1 @type symbol
        // @digest Output buffer names
        // @description @copy EARS_DOC_OUTNAME_ATTR

        // @arg 1 @name numchannels @optional 1 @type int
        // @digest Num channels
        // @description Sets the number of output channels

        // @arg 2 @name pan @optional 1 @type float
        // @digest Pan amount
        // @description Sets the initial pan amount

        earsbufobj_init((t_earsbufobj *)x, 0);
        
        t_llll *args = llll_parse(true_ac, argv);
        t_llll *names = earsbufobj_extract_names_from_args((t_earsbufobj *)x, args);
        
        if (args && args->l_head) {
            x->num_out_channels = MAX(1, hatom_getlong(&args->l_head->l_hatom));
            
            if (args->l_head->l_next) {
                llll_clear(x->pan);
                llll_appendhatom_clone(x->pan, &args->l_head->l_next->l_hatom);
            }
        }
        
        attr_args_process(x, argc, argv);
        
        earsbufobj_setup((t_earsbufobj *)x, "E4", "E", names);

        llll_free(args);
        llll_free(names);
        
    }
    return x;
}


void buf_pan_free(t_buf_pan *x)
{
    llll_free(x->pan);
    earsbufobj_free((t_earsbufobj *)x);
}




void buf_pan_bang(t_buf_pan *x)
{
    long num_buffers = earsbufobj_get_instore_size((t_earsbufobj *)x, 0);
    long num_out_channels = x->num_out_channels;
    e_ears_pan_modes pan_mode = (e_ears_pan_modes)x->pan_mode;
    e_ears_pan_laws pan_law = (e_ears_pan_laws)x->pan_law;
    
    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_buffers, true);

    earsbufobj_mutex_lock((t_earsbufobj *)x);
    earsbufobj_init_progress((t_earsbufobj *)x, num_buffers);

    t_llllelem *el = x->pan->l_head;
    for (long count = 0; count < num_buffers; count++, el = el && el->l_next ? el->l_next : el) {
        t_buffer_obj *in = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, count);
        t_buffer_obj *out = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, count);

        t_llll *pans01 = earsbufobj_llllelem_convert_envtimeunit_and_normalize_range((t_earsbufobj *)x, el, in, EARS_TIMEUNIT_SAMPS, x->pan_range[0], x->pan_range[1], false);
        

        
        if (pans01->l_depth == 1 && pans01->l_head) {
            
            if (hatom_gettype(&pans01->l_head->l_hatom) == H_SYM) {
                // pan is another buffer!
                t_buffer_ref *ref = buffer_ref_new((t_object *)x, hatom_getsym(&pans01->l_head->l_hatom));
                ears_buffer_pan1d_buffer((t_object *)x, in, out, num_out_channels, buffer_ref_getobject(ref), pan_mode, pan_law, x->multichannel_spread, x->compensate_multichannel_gain_to_avoid_clipping);
                object_free(ref);
            } else {
                // pan is a single number
                ears_buffer_pan1d((t_object *)x, in, out, num_out_channels, hatom_getdouble(&pans01->l_head->l_hatom), pan_mode, pan_law, x->multichannel_spread, x->compensate_multichannel_gain_to_avoid_clipping);
            }
        } else {
            // pan is an envelope in llll form
            ears_buffer_pan1d_envelope((t_object *)x, in, out, num_out_channels, pans01, pan_mode, pan_law, x->multichannel_spread, x->compensate_multichannel_gain_to_avoid_clipping, earsbufobj_get_slope_mapping((t_earsbufobj *)x));
        }
        
        llll_free(pans01);

        if (earsbufobj_iter_progress((t_earsbufobj *)x, count, num_buffers)) break;
    }
    
    earsbufobj_mutex_unlock((t_earsbufobj *)x);

    earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
}


void buf_pan_anything(t_buf_pan *x, t_symbol *msg, long ac, t_atom *av)
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
            
            buf_pan_bang(x);
            
        } else if (inlet == 1) {
            llll_free(x->pan);
            x->pan = llll_clone(parsed);
        }
    }
    llll_free(parsed);
}


