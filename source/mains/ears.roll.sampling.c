/**
	@file
	ears.roll.sampling.c
 
	@name
	ears.roll.sampling~
 
	@realname
	ears.roll.sampling~

	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	Convert a <o>bach.roll</o> into a montage of audio samples
 
	@description
	Bounce a <o>bach.roll</o> object containing soundfile names in a given slot into a buffer
 
	@discussion
 
	@category
	ears export
 
	@keywords
	buffer, roll, bounce, export
 
	@seealso
	ears.read~, ears.roll.toreaper~
	
	@owner
	Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "llllobj.h"
#include "llll_commons_ext.h"
#include "bach_math_utilities.h"
#include "ears.scores.h"


typedef struct _buf_roll_sampling {
    t_earsbufobj       e_ob;
    
    char        use_mute_solos;
    char        use_durations;

    double      sr;
    long        num_channels;
    char        normalization_mode;
    char        channelmode;
    long        oversampling;

    long        filename_slot;
    long        offset_slot;
    long        gain_slot;
    long        pan_slot;
    long        rate_slot;
    // these two are currently unused
    long        ps_slot; // pitch shift
    long        ts_slot; // time stretch
    

    // fades
    double      fadein_amount;
    double      fadeout_amount;
    char        fadein_type;
    char        fadeout_type;
    double      fadein_curve;
    double      fadeout_curve;
    
    // panning
    t_llll      *panvoices;
    long        pan_mode; // one of e_ears_pan_modes
    long        pan_law; // one of e_ears_pan_laws
    double      multichannel_spread;
    char        compensate_multichannel_gain_to_avoid_clipping;
    
    // velocity to gain
    e_ears_veltoamp_modes veltoamp_mode;
    double velrange[2];
    
    char        optimize_for_identical_samples;

} t_buf_roll_sampling;



// Prototypes
t_buf_roll_sampling*     buf_roll_sampling_new(t_symbol *s, short argc, t_atom *argv);
void			buf_roll_sampling_free(t_buf_roll_sampling *x);
void			buf_roll_sampling_bang(t_buf_roll_sampling *x);
void			buf_roll_sampling_anything(t_buf_roll_sampling *x, t_symbol *msg, long ac, t_atom *av);

void buf_roll_sampling_assist(t_buf_roll_sampling *x, void *b, long m, long a, char *s);
void buf_roll_sampling_inletinfo(t_buf_roll_sampling *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;


EARSBUFOBJ_ADD_IO_METHODS(roll_sampling)
DEFINE_LLLL_ATTR_DEFAULT_GETTER(t_buf_roll_sampling, panvoices, buf_roll_sampling_getattr_panvoices);
DEFINE_LLLL_ATTR_DEFAULT_SETTER(t_buf_roll_sampling, panvoices, buf_roll_sampling_setattr_panvoices);


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
    
    CLASS_NEW_CHECK_SIZE(c, "ears.roll.sampling~",
                         (method)buf_roll_sampling_new,
                         (method)buf_roll_sampling_free,
                         sizeof(t_buf_roll_sampling),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    EARSBUFOBJ_DECLARE_COMMON_METHODS_HANDLETHREAD(roll_sampling)
    
    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_timeunit_attr(c);
    earsbufobj_class_add_naming_attr(c);

    earsbufobj_class_add_resamplingpolicy_attr(c);
    earsbufobj_class_add_resamplingfiltersize_attr(c);

    
    CLASS_ATTR_CHAR(c, "mutesolo", 0, t_buf_roll_sampling, use_mute_solos);
    CLASS_ATTR_STYLE_LABEL(c,"mutesolo",0,"onoff","Account For Muting and Soloing");
    // @description Toggles the ability to account for muting and soloing while bouncing.

    CLASS_ATTR_CHAR(c, "usedurations", 0, t_buf_roll_sampling, use_durations);
    CLASS_ATTR_STYLE_LABEL(c,"usedurations",0,"onoff","Account For Note Durations");
    // @description Toggles the ability to account for note durations while bouncing.

    CLASS_STICKY_ATTR(c,"category",0,"Slots");

    CLASS_ATTR_LONG(c, "audioslot", 0, t_buf_roll_sampling, filename_slot);
    CLASS_ATTR_STYLE_LABEL(c,"audioslot",0,"text","Slot Containing File Names Or Buffer Names");
    CLASS_ATTR_BASIC(c, "audioslot", 0);
    // @description Sets the number of the slot containing file names or buffer names.

    CLASS_ATTR_LONG(c, "fileslot", 0, t_buf_roll_sampling, filename_slot);
    CLASS_ATTR_INVISIBLE(c, "fileslot", 0);

    CLASS_ATTR_LONG(c, "offsetslot", 0, t_buf_roll_sampling, offset_slot);
    CLASS_ATTR_STYLE_LABEL(c,"offsetslot",0,"text","Slot Containing Offset In File");
    CLASS_ATTR_BASIC(c, "offsetslot", 0);
    // @description Sets the number of the slot containing the offset from the beginning of the file.

    CLASS_ATTR_LONG(c, "gainslot", 0, t_buf_roll_sampling, gain_slot);
    CLASS_ATTR_STYLE_LABEL(c,"gainslot",0,"text","Slot Containing Gain");
    // @description Sets the number of the slot containing the gain or gain envelope.

    CLASS_ATTR_LONG(c, "panslot", 0, t_buf_roll_sampling, pan_slot);
    CLASS_ATTR_STYLE_LABEL(c,"panslot",0,"text","Slot Containing Pan");
    // @description Sets the number of the slot containing the file names (0 = none).

    CLASS_ATTR_LONG(c, "rateslot", 0, t_buf_roll_sampling, rate_slot);
    CLASS_ATTR_STYLE_LABEL(c,"rateslot",0,"text","Slot Containing Rate");
    // @description Sets the number of the slot containing the rate (0 = none).

//    CLASS_ATTR_LONG(c, "psslot", 0, t_buf_roll_sampling, ps_slot);
//    CLASS_ATTR_STYLE_LABEL(c,"panslot",0,"text","Slot Containing Pitch Shift");
    // @exclude all
    // @description Sets the number of slots containing the pitch shift in cents (0 = none).

//    CLASS_ATTR_LONG(c, "tsslot", 0, t_buf_roll_sampling, ts_slot);
//    CLASS_ATTR_STYLE_LABEL(c,"panslot",0,"text","Slot Containing Time Stretch");
    // @exclude all
    // @description Sets the number of slots containing the time stretch as ratio (0 = none).

    CLASS_STICKY_ATTR_CLEAR(c, "category");

    CLASS_ATTR_DOUBLE(c, "sr", 0, t_buf_roll_sampling, sr);
    CLASS_ATTR_STYLE_LABEL(c,"sr",0,"text","Output Sample Rate");
    CLASS_ATTR_BASIC(c, "sr", 0);
    // @description Sets the sample rate for the output buffer. If zero (default), the current Max sample rate is used.

    CLASS_ATTR_LONG(c, "oversampling", 0, t_buf_roll_sampling, oversampling);
    CLASS_ATTR_STYLE_LABEL(c,"oversampling",0,"text","Oversampling");
    CLASS_ATTR_CATEGORY(c, "oversampling", 0, "Resampling");
    CLASS_ATTR_INVISIBLE(c, "oversampling", 0);
    // @ignore all
    // @description Sets the oversampling factor for subsample processing.
    
    
    CLASS_ATTR_LONG(c, "numchannels", 0, t_buf_roll_sampling, num_channels);
    CLASS_ATTR_STYLE_LABEL(c,"numchannels",0,"text","Output Number Of Channels");
    CLASS_ATTR_BASIC(c, "numchannels", 0);
    // @description Sets the number of output channels.

    
    CLASS_ATTR_CHAR(c, "normalize", 0, t_buf_roll_sampling, normalization_mode);
    CLASS_ATTR_STYLE_LABEL(c,"normalize",0,"enumindex","Normalize Output");
    CLASS_ATTR_ENUMINDEX(c,"normalize", 0, "Never Always Overload Protection Only");
    // @description Sets the normalization mode for the output buffer:
    // 0 = Never, does not normalize; 1 = Always, always normalizes to 1.; 2 (default) = Overload Protection Only, only
    // normalizes to 1. if some samples exceed in modulo 1.

    CLASS_ATTR_CHAR(c, "channelmode",	0,	t_buf_roll_sampling, channelmode);
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
    
    CLASS_ATTR_DOUBLE(c, "fadein", 0, t_buf_roll_sampling, fadein_amount);
    CLASS_ATTR_STYLE_LABEL(c,"fadein",0,"text","Fade In Amount");
    // @description Sets a global amount of fade in for every sample (in units given by the <m>timeunit</m> attribute).

    CLASS_ATTR_DOUBLE(c, "fadeout", 0, t_buf_roll_sampling, fadeout_amount);
    CLASS_ATTR_STYLE_LABEL(c,"fadeout",0,"text","Fade Out Amount");
    // @description Sets a global amount of fade out for every sample (in units given by the <m>timeunit</m> attribute).

    CLASS_ATTR_CHAR(c, "fadeintype", 0, t_buf_roll_sampling, fadein_type);
    CLASS_ATTR_STYLE_LABEL(c,"fadeintype",0,"enumindex","Fade In Type");
    CLASS_ATTR_ENUMINDEX(c,"fadeintype", 0, "None Linear Sine Curve S-Curve");
    // @description Sets the fade in type: 0 = None, 1 = Linear (default), 2 = Sine, 3 = Curve, 4 = S-Curve;
    
    CLASS_ATTR_CHAR(c, "fadeouttype", 0, t_buf_roll_sampling, fadeout_type);
    CLASS_ATTR_STYLE_LABEL(c,"fadeouttype",0,"enumindex","Fade Out Type");
    CLASS_ATTR_ENUMINDEX(c,"fadeouttype", 0, "None Linear Sine Curve S-Curve");
    // @description Sets the fade in type: 0 = None, 1 = Linear (default), 2 = Sine, 3 = Curve, 4 = S-Curve;

    
    CLASS_ATTR_DOUBLE(c, "fadeincurve", 0, t_buf_roll_sampling, fadein_curve);
    CLASS_ATTR_STYLE_LABEL(c,"fadeincurve",0,"text","Fade In Curve");
    // @description Sets the curve parameter for the fade in (for fades of type Curve and S-Curve only).
    // The parameters goes from -1 to 1, 0 being linear (default).
    
    CLASS_ATTR_DOUBLE(c, "fadeoutcurve", 0, t_buf_roll_sampling, fadeout_curve);
    CLASS_ATTR_STYLE_LABEL(c,"fadeoutcurve",0,"text","Fade Out Curve");
    // @description Sets the curve parameter for the fade out (for fades of type Curve and S-Curve only).
    // The parameters goes from -1 to 1, 0 being linear (default).

    CLASS_STICKY_ATTR_CLEAR(c, "category");

    
    CLASS_STICKY_ATTR(c,"category",0,"Pan");
    
    
    CLASS_ATTR_LLLL(c, "panvoices", 0, t_buf_roll_sampling, panvoices, buf_roll_sampling_getattr_panvoices, buf_roll_sampling_setattr_panvoices);
    CLASS_ATTR_STYLE_LABEL(c,"panvoices",0,"text","Per-Voice Panning");
    CLASS_ATTR_BASIC(c, "panvoices", 0);
    // @description Sets the panning on a voice-by-voice basis (possibly overridden by the <m>panslot</m>).
    // A number for each voice is expected, between 0 (first loudspeaker) and 1 (last loudspeaker).
    // In a standard stereo 0 = left, 1 = right.
    
    CLASS_ATTR_LONG(c, "panmode", 0, t_buf_roll_sampling, pan_mode);
    CLASS_ATTR_STYLE_LABEL(c,"panmode",0,"text","Pan Mode");
    CLASS_ATTR_ENUMINDEX(c,"panmode", 0, "Linear Circular");
    CLASS_ATTR_BASIC(c, "panmode", 0);
    // @description Sets the panning mode: 0 = linear (default); 1 = circular.
    
    
    CLASS_ATTR_LONG(c, "panlaw", 0, t_buf_roll_sampling, pan_law);
    CLASS_ATTR_STYLE_LABEL(c,"panlaw",0,"text","Pan Law");
    CLASS_ATTR_ENUMINDEX(c,"panlaw", 0, "Nearest Neighbor Cosine");
    // @description Sets the panning law: 0 = nearest neighbor (panned on one loudspeaker at a time);
    // 1 = cosine law (default).
    
    
    CLASS_ATTR_DOUBLE(c, "spread", 0, t_buf_roll_sampling, multichannel_spread);
    CLASS_ATTR_STYLE_LABEL(c,"spread",0,"text","Multichannel Spread");
    // @description Sets the spread of the input channels when panning multichannels buffers. Default is 0 (downmix to mono).
    // A spread of 1 spreads a multichannel signal sent to the central loudspeaker up to the external ones.
    
    CLASS_ATTR_CHAR(c, "compensate", 0, t_buf_roll_sampling, compensate_multichannel_gain_to_avoid_clipping);
    CLASS_ATTR_STYLE_LABEL(c,"compensate",0,"onoff","Reduce Multichannel Gain To Avoid Clipping");
    // @description Toggles the ability to automatically reduce the gain of multichannel files by a factor of the number of channels, in order
    // to avoid possible clipping while panning then with low <m>spread</m> values. Defaults to 1.
    
    CLASS_STICKY_ATTR_CLEAR(c, "category");

    
    CLASS_STICKY_ATTR(c,"category",0,"Velocity");
    
    
    CLASS_ATTR_LONG(c, "velmode", 0, t_buf_roll_sampling, veltoamp_mode);
    CLASS_ATTR_STYLE_LABEL(c,"velmode",0,"text","Velocity Mode");
    CLASS_ATTR_ENUMINDEX(c,"velmode", 0, "Ignore Map To Amplitude Map To Decibels");
    CLASS_ATTR_BASIC(c, "velmode", 0);
    // @description Sets the velocity mode: 0 = ignore; 1 = map velocity to amplitude range (default); 2 = map velocity to decibels range.
    // ranges are defined in <m>velrange</m>
    
    
    CLASS_ATTR_DOUBLE_ARRAY(c, "velrange", 0, t_buf_roll_sampling, velrange, 2);
    CLASS_ATTR_STYLE_LABEL(c,"velrange",0,"text","Velocity Mapping Range");
    // @description Sets the mapping range for the velocity. This can be either an amplitude range or a decibels range, depending on <m>velmode</m>.
    
    CLASS_STICKY_ATTR_CLEAR(c, "category");


    
    
    
    
    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
    return 0;
}

void buf_roll_sampling_assist(t_buf_roll_sampling *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        if (a == 0) // @in 0 @type llll @digest Gathered Syntax of <o>bach.roll</o>
            sprintf(s, "llll: Gathered Syntax");
    } else {
        sprintf(s, "Output Buffer Name"); // @out 0 @type symbol/list @digest Output buffer names(s)
                                            // @description Name of the bounced buffer
    }
}

void buf_roll_sampling_inletinfo(t_buf_roll_sampling *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_roll_sampling *buf_roll_sampling_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_roll_sampling *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_roll_sampling*)object_alloc_debug(s_tag_class);
    if (x) {
        x->normalization_mode = EARS_NORMALIZE_OVERLOAD_PROTECTION_ONLY;
        x->use_mute_solos = true;
        x->use_durations = true;
        x->sr = ears_get_current_Max_sr();
        x->oversampling = 1;
        x->num_channels = 2;
        x->offset_slot = 0;
        x->filename_slot = 8;
        x->gain_slot = 0;
        x->pan_slot = 0;
        
        x->optimize_for_identical_samples = 1;

        x->panvoices = llll_from_text_buf("");

        x->pan_mode = EARS_PAN_MODE_LINEAR;
        x->pan_law = EARS_PAN_LAW_COSINE;
        x->multichannel_spread = 0.;
        x->compensate_multichannel_gain_to_avoid_clipping = true;
        x->channelmode = EARS_CHANNELCONVERTMODE_CYCLE;

        x->veltoamp_mode = EARS_VELOCITY_TO_AMPLITUDE;
        x->velrange[0] = 0.;
        x->velrange[1] = 1.;

        x->fadein_type = x->fadeout_type = EARS_FADE_LINEAR;
        x->fadein_amount = x->fadeout_amount = 10; // by default ms

        earsbufobj_init((t_earsbufobj *)x, 0);

        // @arg 0 @name outname @optional 1 @type symbol
        // @digest Output buffer name
        // @description @copy EARS_DOC_OUTNAME_ATTR
        
        t_llll *args = llll_parse(true_ac, argv);
        t_llll *names = earsbufobj_extract_names_from_args((t_earsbufobj *)x, args);
        
        attr_args_process(x, argc, argv);

        earsbufobj_setup((t_earsbufobj *)x, "4", "e", names);

        llll_free(args);
        llll_free(names);
    }
    return x;
}


void buf_roll_sampling_free(t_buf_roll_sampling *x)
{
    earsbufobj_free((t_earsbufobj *)x);
}

void buf_roll_sampling_bang(t_buf_roll_sampling *x)
{

    t_llll *roll_gs = llllobj_get_store_contents((t_object *)x, LLLL_OBJ_VANILLA, 0, false);
    t_buffer_obj *outbuf = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, 0);

    earsbufobj_mutex_lock((t_earsbufobj *)x);
    ears_roll_to_buffer((t_earsbufobj *)x, EARS_SCORETOBUF_MODE_SAMPLING, roll_gs, outbuf,
                        EARS_SYNTHMODE_NONE, NULL, 0, //< we're not using synthesis
                        x->use_mute_solos, x->use_durations, x->num_channels,
                        x->filename_slot, x->offset_slot, x->gain_slot, x->pan_slot, x->rate_slot, x->ps_slot, x->ts_slot,
                        x->sr > 0 ? x->sr : ears_get_current_Max_sr(), (e_ears_normalization_modes)x->normalization_mode,
                        (e_ears_channel_convert_modes)x->channelmode,
                        x->fadein_amount, x->fadeout_amount, (e_ears_fade_types)x->fadein_type, (e_ears_fade_types)x->fadeout_type,
                        x->fadein_curve, x->fadeout_curve,
                        x->panvoices,
                        (e_ears_pan_modes)x->pan_mode, (e_ears_pan_laws)x->pan_law, x->multichannel_spread, x->compensate_multichannel_gain_to_avoid_clipping,
                        (e_ears_veltoamp_modes)x->veltoamp_mode, x->velrange[0], x->velrange[1], 440, x->oversampling, EARS_DEFAULT_RESAMPLING_WINDOW_WIDTH, x->optimize_for_identical_samples);
    earsbufobj_mutex_unlock((t_earsbufobj *)x);
    
    earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
    
    llll_release(roll_gs);
    
}

void buf_roll_sampling_anything(t_buf_roll_sampling *x, t_symbol *msg, long ac, t_atom *av)
{
//    long inlet = earsbufobj_proxy_getinlet((t_earsbufobj *) x);
    
    if (msg != _sym_bang) {
        t_llll *parsed = earsbufobj_parse_gimme((t_earsbufobj *) x, LLLL_OBJ_VANILLA, msg, ac, av);
        llllobj_store_llll((t_object *)x, LLLL_OBJ_VANILLA, parsed, 0);
        buf_roll_sampling_bang(x);
    }
    
}


