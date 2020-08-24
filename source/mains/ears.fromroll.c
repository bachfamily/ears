/**
	@file
	ears.fromroll.c
 
	@name
	ears.fromroll~
 
	@realname
	ears.fromroll~

    @hiddenalias
    ears.fromroll

	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	Convert a <o>bach.roll</o> into a buffer
 
	@description
	Bounce a <o>bach.roll</o> object containing soundfile names in a given slot into a buffer
 
	@discussion
 
	@category
	ears buffer operations
 
	@keywords
	buffer, roll, bounce, export
 
	@seealso
	ears.read~
	
	@owner
	Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "llllobj.h"
#include "llll_commons_ext.h"
#include "bach_math_utilities.h"
#include "ears.scores.h"


typedef struct _buf_fromroll {
    t_earsbufobj       e_ob;
    
    char        use_mute_solos;
    char        use_durations;

    double      sr;
    long        num_channels;
    char        normalization_mode;
    char        channelmode;

    long        filename_slot;
    long        offset_slot;
    long        gain_slot;
    long        pan_slot;

    // fades
    double      fadein_amount;
    double      fadeout_amount;
    char        fadein_type;
    char        fadeout_type;
    double      fadein_curve;
    double      fadeout_curve;
    
    // panning
    e_ears_pan_modes    pan_mode;
    e_ears_pan_laws     pan_law;
    double              multichannel_spread;
    char                compensate_multichannel_gain_to_avoid_clipping;
    
    // velocity to gain
    e_ears_veltoamp_modes veltoamp_mode;
    double velrange[2];

} t_buf_fromroll;



// Prototypes
t_buf_fromroll*     buf_fromroll_new(t_symbol *s, short argc, t_atom *argv);
void			buf_fromroll_free(t_buf_fromroll *x);
void			buf_fromroll_bang(t_buf_fromroll *x);
void			buf_fromroll_anything(t_buf_fromroll *x, t_symbol *msg, long ac, t_atom *av);

void buf_fromroll_assist(t_buf_fromroll *x, void *b, long m, long a, char *s);
void buf_fromroll_inletinfo(t_buf_fromroll *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;


EARSBUFOBJ_ADD_IO_METHODS(fromroll)


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
    
    CLASS_NEW_CHECK_SIZE(c, "ears.fromroll~",
                         (method)buf_fromroll_new,
                         (method)buf_fromroll_free,
                         sizeof(t_buf_fromroll),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    EARSBUFOBJ_DECLARE_COMMON_METHODS_DEFER(fromroll)
    
    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_timeunit_attr(c);
    earsbufobj_class_add_naming_attr(c);
    
    
    CLASS_ATTR_CHAR(c, "mutesolo", 0, t_buf_fromroll, use_mute_solos);
    CLASS_ATTR_STYLE_LABEL(c,"mutesolo",0,"onoff","Account For Muting and Soloing");
    // @description Toggles the ability to account for muting and soloing while bouncing.

    CLASS_ATTR_CHAR(c, "durations", 0, t_buf_fromroll, use_durations);
    CLASS_ATTR_STYLE_LABEL(c,"durations",0,"onoff","Account For Note Durations");
    // @description Toggles the ability to account for note durations while bouncing.

    CLASS_STICKY_ATTR(c,"category",0,"Slots");

    CLASS_ATTR_LONG(c, "filenameslot", 0, t_buf_fromroll, filename_slot);
    CLASS_ATTR_STYLE_LABEL(c,"filenameslot",0,"text","Slot Containing File Names");
    CLASS_ATTR_BASIC(c, "filenameslot", 0);
    // @description Sets the number of slots containing the file names.

    CLASS_ATTR_LONG(c, "offsetslot", 0, t_buf_fromroll, offset_slot);
    CLASS_ATTR_STYLE_LABEL(c,"offsetslot",0,"text","Slot Containing Offset In File");
    CLASS_ATTR_BASIC(c, "offsetslot", 0);
    // @description Sets the number of slots containing the offset from the beginning of the file.

    CLASS_ATTR_LONG(c, "gainslot", 0, t_buf_fromroll, gain_slot);
    CLASS_ATTR_STYLE_LABEL(c,"gainslot",0,"text","Slot Containing Gain");
    // @description Sets the number of slots containing the gain or gain envelope.

    CLASS_ATTR_LONG(c, "panslot", 0, t_buf_fromroll, pan_slot);
    CLASS_ATTR_STYLE_LABEL(c,"panslot",0,"text","Slot Containing Pan");
    // @description Sets the number of slots containing the file names (0 = none).

    CLASS_STICKY_ATTR_CLEAR(c, "category");

    CLASS_ATTR_DOUBLE(c, "sr", 0, t_buf_fromroll, sr);
    CLASS_ATTR_STYLE_LABEL(c,"sr",0,"text","Output Sample Rate");
    CLASS_ATTR_BASIC(c, "sr", 0);
    // @description Sets the sample rate for the output buffer.

    CLASS_ATTR_LONG(c, "numchannels", 0, t_buf_fromroll, num_channels);
    CLASS_ATTR_STYLE_LABEL(c,"numchannels",0,"text","Output Number Of Channels");
    CLASS_ATTR_BASIC(c, "numchannels", 0);
    // @description Sets the number of output channels.

    
    CLASS_ATTR_CHAR(c, "normalize", 0, t_buf_fromroll, normalization_mode);
    CLASS_ATTR_STYLE_LABEL(c,"normalize",0,"enumindex","Normalize Output");
    CLASS_ATTR_ENUMINDEX(c,"normalize", 0, "Never Always Overload Protection Only");
    // @description Sets the normalization mode for the output buffer:
    // 0 = Never, does not normalize; 1 = Always, always normalizes to 1.; 2 (default) = Overload Protection Only, only
    // normalizes to 1. if some samples exceed in modulo 1.

    CLASS_ATTR_CHAR(c, "channelmode",	0,	t_buf_fromroll, channelmode);
    CLASS_ATTR_STYLE_LABEL(c, "channelmode", 0, "enumindex", "Channel Conversion Mode");
    CLASS_ATTR_ENUMINDEX(c,"channelmode", 0, "Clear Keep Pad Cycle Palindrome Pan");
    // @description Sets the channel conversion mode: <br />
    // 0 (Clear) = Delete all samples <br />
    // 1 (Keep) = Only keep existing channels <br />
    // 2 (Pad) = Pad last channel while upmixing <br />
    // 3 (Cycle, default) = Repeat all channels (cycling) while upmixing <br />
    // 4 (Palindrome) = Palindrome cycling of channels while upmixing <br />
    // 5 (Pan) = Pan channels to new configuration <br />
    

    CLASS_STICKY_ATTR(c,"category",0,"Fade");
    
    CLASS_ATTR_DOUBLE(c, "fadein", 0, t_buf_fromroll, fadein_amount);
    CLASS_ATTR_STYLE_LABEL(c,"fadein",0,"text","Fade In Amount");
    // @description Sets a global amount of fade in for every sample (in units given by the <m>timeunit</m> attribute).

    CLASS_ATTR_DOUBLE(c, "fadeout", 0, t_buf_fromroll, fadeout_amount);
    CLASS_ATTR_STYLE_LABEL(c,"fadeout",0,"text","Fade Out Amount");
    // @description Sets a global amount of fade out for every sample (in units given by the <m>timeunit</m> attribute).

    CLASS_ATTR_CHAR(c, "fadeintype", 0, t_buf_fromroll, fadein_type);
    CLASS_ATTR_STYLE_LABEL(c,"fadeintype",0,"enumindex","Fade In Type");
    CLASS_ATTR_ENUMINDEX(c,"fadeintype", 0, "None Linear Equal Power Curve S-Curve");
    // @description Sets the fade in type: 0 = None, 1 = Linear, 2 = Equal Power (default).
    
    
    CLASS_ATTR_CHAR(c, "fadeouttype", 0, t_buf_fromroll, fadeout_type);
    CLASS_ATTR_STYLE_LABEL(c,"fadeouttype",0,"enumindex","Fade Out Type");
    CLASS_ATTR_ENUMINDEX(c,"fadeouttype", 0, "None Linear Equal Power Curve S-Curve");
    // @description Sets the fade out type: 0 = None, 1 = Linear, 2 = Equal Power (default).
    
    
    CLASS_ATTR_DOUBLE(c, "fadeincurve", 0, t_buf_fromroll, fadein_curve);
    CLASS_ATTR_STYLE_LABEL(c,"fadeincurve",0,"text","Fade In Curve");
    // @description Sets the curve parameter for the fade in (for fades of type Curve and S-Curve only).
    // The parameters goes from -1 to 1, 0 being linear (default).
    
    CLASS_ATTR_DOUBLE(c, "fadeoutcurve", 0, t_buf_fromroll, fadeout_curve);
    CLASS_ATTR_STYLE_LABEL(c,"fadeoutcurve",0,"text","Fade Out Curve");
    // @description Sets the curve parameter for the fade out (for fades of type Curve and S-Curve only).
    // The parameters goes from -1 to 1, 0 being linear (default).

    CLASS_STICKY_ATTR_CLEAR(c, "category");

    
    CLASS_STICKY_ATTR(c,"category",0,"Pan");
    
    
    CLASS_ATTR_LONG(c, "panmode", 0, t_buf_fromroll, pan_mode);
    CLASS_ATTR_STYLE_LABEL(c,"panmode",0,"text","Pan Mode");
    CLASS_ATTR_ENUMINDEX(c,"panmode", 0, "Linear Circular");
    CLASS_ATTR_BASIC(c, "panmode", 0);
    // @description Sets the panning mode: 0 = linear (default); 1 = circular.
    
    
    CLASS_ATTR_LONG(c, "panlaw", 0, t_buf_fromroll, pan_law);
    CLASS_ATTR_STYLE_LABEL(c,"panlaw",0,"text","Pan Law");
    CLASS_ATTR_ENUMINDEX(c,"panlaw", 0, "Nearest Neighbor Cosine");
    // @description Sets the panning law: 0 = nearest neighbor (panned on one loudspeaker at a time);
    // 1 = cosine law (default).
    
    
    CLASS_ATTR_DOUBLE(c, "spread", 0, t_buf_fromroll, multichannel_spread);
    CLASS_ATTR_STYLE_LABEL(c,"spread",0,"text","Multichannel Spread");
    // @description Sets the spread of the input channels when panning multichannels buffers. Default is 0 (downmix to mono).
    // A spread of 1 spreads a multichannel signal sent to the central loudspeaker up to the external ones.
    
    CLASS_ATTR_CHAR(c, "compensate", 0, t_buf_fromroll, compensate_multichannel_gain_to_avoid_clipping);
    CLASS_ATTR_STYLE_LABEL(c,"compensate",0,"onoff","Reduce Multichannel Gain To Avoid Clipping");
    // @description Toggles the ability to automatically reduce the gain of multichannel files by a factor of the number of channels, in order
    // to avoid possible clipping while panning then with low <m>spread</m> values. Defaults to 1.
    
    CLASS_STICKY_ATTR_CLEAR(c, "category");

    
    CLASS_STICKY_ATTR(c,"category",0,"Velocity");
    
    
    CLASS_ATTR_LONG(c, "velmode", 0, t_buf_fromroll, veltoamp_mode);
    CLASS_ATTR_STYLE_LABEL(c,"velmode",0,"text","Velocity Mode");
    CLASS_ATTR_ENUMINDEX(c,"velmode", 0, "Ignore Map To Amplitude Map To Decibels");
    CLASS_ATTR_BASIC(c, "velmode", 0);
    // @description Sets the velocity mode: 0 = ignore; 1 = map velocity to amplitude range (default); 2 = map velocity to decibels range.
    // ranges are defined in <m>velrange</m>
    
    
    CLASS_ATTR_DOUBLE_ARRAY(c, "velrange", 0, t_buf_fromroll, velrange, 2);
    CLASS_ATTR_STYLE_LABEL(c,"velrange",0,"text","Velocity Mapping Range");
    // @description Sets the mapping range for the velocity. This can be either an amplitude range or a decibels range, depending on <m>velmode</m>.
    
    CLASS_STICKY_ATTR_CLEAR(c, "category");


    
    
    
    
    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
    return 0;
}

void buf_fromroll_assist(t_buf_fromroll *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        if (a == 0) // @in 0 @type llll @digest Gathered Syntax of <o>bach.roll</o>
            sprintf(s, "llll: Gathered Syntax");
    } else {
        sprintf(s, "Output Buffer Name"); // @description Name of the bounced buffer
    }
}

void buf_fromroll_inletinfo(t_buf_fromroll *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_fromroll *buf_fromroll_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_fromroll *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_fromroll*)object_alloc_debug(s_tag_class);
    if (x) {
        x->normalization_mode = EARS_NORMALIZE_OVERLOAD_PROTECTION_ONLY;
        x->use_mute_solos = true;
        x->use_durations = true;
        x->sr = 44100;
        x->num_channels = 2;
        x->offset_slot = 0;
        x->filename_slot = 8;
        x->gain_slot = 0;
        x->pan_slot = 0;
        
        x->pan_mode = EARS_PAN_MODE_LINEAR;
        x->pan_law = EARS_PAN_LAW_COSINE;
        x->multichannel_spread = 0.;
        x->compensate_multichannel_gain_to_avoid_clipping = true;
        
        x->veltoamp_mode = EARS_VELOCITY_TO_AMPLITUDE;
        x->velrange[0] = 0.;
        x->velrange[1] = 1.;

        x->fadein_type = x->fadeout_type = EARS_FADE_EQUALPOWER;
        
        earsbufobj_init((t_earsbufobj *)x, 0);

        // @arg 0 @name outname @optional 1 @type symbol
        // @digest Output buffer name
        // @description @copy EARS_DOC_OUTNAME_ATTR
        
        t_llll *args = llll_parse(true_ac, argv);
        t_llll *names = NULL;
        t_llllelem *cur = args ? args->l_head : NULL;
        if (cur) {
            if (hatom_gettype(&cur->l_hatom) == H_LLLL) {
                names = llll_clone(hatom_getllll(&cur->l_hatom));
                cur = cur ? cur->l_next : NULL;
            } else if (hatom_gettype(&cur->l_hatom) == H_SYM) {
                names = llll_get();
                llll_appendhatom_clone(names, &cur->l_hatom);
                cur = cur ? cur->l_next : NULL;
            }
        }
        
        attr_args_process(x, argc, argv);

        earsbufobj_setup((t_earsbufobj *)x, "4", "e", names);

        llll_free(args);
        llll_free(names);
    }
    return x;
}


void buf_fromroll_free(t_buf_fromroll *x)
{
    earsbufobj_free((t_earsbufobj *)x);
}

void buf_fromroll_bang(t_buf_fromroll *x)
{

    t_llll *roll_gs = llllobj_get_store_contents((t_object *)x, LLLL_OBJ_VANILLA, 0, false);
    t_buffer_obj *outbuf = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, 0);

    earsbufobj_mutex_lock((t_earsbufobj *)x);
    ears_roll_to_buffer((t_earsbufobj *)x, roll_gs, outbuf, x->use_mute_solos, x->use_durations, x->num_channels,
                        x->filename_slot, x->offset_slot, x->gain_slot, x->pan_slot, x->sr, (e_ears_normalization_modes)x->normalization_mode,
                        (e_ears_channel_convert_modes)x->channelmode,
                        x->fadein_amount, x->fadeout_amount, (e_ears_fade_types)x->fadein_type, (e_ears_fade_types)x->fadeout_type,
                        x->fadein_curve, x->fadeout_curve,
                        (e_ears_pan_modes)x->pan_mode, (e_ears_pan_laws)x->pan_law, x->multichannel_spread, x->compensate_multichannel_gain_to_avoid_clipping,
                        (e_ears_veltoamp_modes)x->veltoamp_mode, x->velrange[0], x->velrange[1]);
    earsbufobj_mutex_unlock((t_earsbufobj *)x);
    
    earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
    
    llll_free(roll_gs);
    
}

void buf_fromroll_anything(t_buf_fromroll *x, t_symbol *msg, long ac, t_atom *av)
{
//    long inlet = proxy_getinlet((t_object *) x);
    
    if (msg != _sym_bang) {
        t_llll *parsed = earsbufobj_parse_gimme((t_earsbufobj *) x, LLLL_OBJ_VANILLA, msg, ac, av);
        buf_fromroll_bang(x);
        llll_free(parsed);
    }
    
}


