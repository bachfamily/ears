/**
	@file
	ears.roll.sinusoids.c
 
	@name
	ears.roll.sinusoids~
 
	@realname
	ears.roll.sinusoids~

	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	Convert a <o>bach.roll</o> into a mix of sinusoids
 
	@description
	Bounces a <o>bach.roll</o> object into a buffer considering each note a sinusoid
 
	@discussion
 
	@category
	ears export
 
	@keywords
	buffer, roll, bounce, export, sinusoids, cycle
 
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


typedef struct _buf_roll_sinusoids {
    t_earsbufobj       e_ob;
    
    char        use_mute_solos;

    double      sr;
    long        num_channels;
    char        normalization_mode;

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
    long        pan_mode; // one of e_ears_pan_modes
    long        pan_law; // one of e_ears_pan_laws
    double      multichannel_spread;
    char        compensate_multichannel_gain_to_avoid_clipping;
    
    // velocity to gain
    e_ears_veltoamp_modes veltoamp_mode;
    double velrange[2];

    // tuning
    double      middleAtuning;
} t_buf_roll_sinusoids;



// Prototypes
t_buf_roll_sinusoids*     buf_roll_sinusoids_new(t_symbol *s, short argc, t_atom *argv);
void			buf_roll_sinusoids_free(t_buf_roll_sinusoids *x);
void			buf_roll_sinusoids_bang(t_buf_roll_sinusoids *x);
void			buf_roll_sinusoids_anything(t_buf_roll_sinusoids *x, t_symbol *msg, long ac, t_atom *av);

void buf_roll_sinusoids_assist(t_buf_roll_sinusoids *x, void *b, long m, long a, char *s);
void buf_roll_sinusoids_inletinfo(t_buf_roll_sinusoids *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;


EARSBUFOBJ_ADD_IO_METHODS(roll_sinusoids)


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
    
    CLASS_NEW_CHECK_SIZE(c, "ears.roll.sinusoids~",
                         (method)buf_roll_sinusoids_new,
                         (method)buf_roll_sinusoids_free,
                         sizeof(t_buf_roll_sinusoids),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    EARSBUFOBJ_DECLARE_COMMON_METHODS_DEFER(roll_sinusoids)
    
    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_timeunit_attr(c);
    earsbufobj_class_add_naming_attr(c);
    
    
    CLASS_ATTR_CHAR(c, "mutesolo", 0, t_buf_roll_sinusoids, use_mute_solos);
    CLASS_ATTR_STYLE_LABEL(c,"mutesolo",0,"onoff","Account For Muting and Soloing");
    // @description Toggles the ability to account for muting and soloing while bouncing.

    CLASS_STICKY_ATTR(c,"category",0,"Slots");

    CLASS_ATTR_LONG(c, "gainslot", 0, t_buf_roll_sinusoids, gain_slot);
    CLASS_ATTR_STYLE_LABEL(c,"gainslot",0,"text","Slot Containing Gain");
    // @description Sets the number of slots containing the gain or gain envelope.

    CLASS_ATTR_LONG(c, "panslot", 0, t_buf_roll_sinusoids, pan_slot);
    CLASS_ATTR_STYLE_LABEL(c,"panslot",0,"text","Slot Containing Pan");
    // @description Sets the number of slots containing the file names (0 = none).

    CLASS_STICKY_ATTR_CLEAR(c, "category");

    CLASS_ATTR_DOUBLE(c, "sr", 0, t_buf_roll_sinusoids, sr);
    CLASS_ATTR_STYLE_LABEL(c,"sr",0,"text","Output Sample Rate");
    CLASS_ATTR_BASIC(c, "sr", 0);
    // @description Sets the sample rate for the output buffer.

    CLASS_ATTR_LONG(c, "numchannels", 0, t_buf_roll_sinusoids, num_channels);
    CLASS_ATTR_STYLE_LABEL(c,"numchannels",0,"text","Output Number Of Channels");
    CLASS_ATTR_BASIC(c, "numchannels", 0);
    // @description Sets the number of output channels.

    CLASS_ATTR_DOUBLE(c, "tuning", 0, t_buf_roll_sinusoids, middleAtuning);
    CLASS_ATTR_STYLE_LABEL(c,"tuning",0,"text","Middle A Tuning");
    CLASS_ATTR_BASIC(c, "tuning", 0);
    // @description Sets the frequency of middle A.

    
    CLASS_ATTR_CHAR(c, "normalize", 0, t_buf_roll_sinusoids, normalization_mode);
    CLASS_ATTR_STYLE_LABEL(c,"normalize",0,"enumindex","Normalize Output");
    CLASS_ATTR_ENUMINDEX(c,"normalize", 0, "Never Always Overload Protection Only");
    // @description Sets the normalization mode for the output buffer:
    // 0 = Never, does not normalize; 1 = Always, always normalizes to 1.; 2 (default) = Overload Protection Only, only
    // normalizes to 1. if some samples exceed in modulo 1.

    CLASS_STICKY_ATTR(c,"category",0,"Fade");
    
    CLASS_ATTR_DOUBLE(c, "fadein", 0, t_buf_roll_sinusoids, fadein_amount);
    CLASS_ATTR_STYLE_LABEL(c,"fadein",0,"text","Fade In Amount");
    // @description Sets a global amount of fade in for every sample (in units given by the <m>timeunit</m> attribute).

    CLASS_ATTR_DOUBLE(c, "fadeout", 0, t_buf_roll_sinusoids, fadeout_amount);
    CLASS_ATTR_STYLE_LABEL(c,"fadeout",0,"text","Fade Out Amount");
    // @description Sets a global amount of fade out for every sample (in units given by the <m>timeunit</m> attribute).

    CLASS_ATTR_CHAR(c, "fadeintype", 0, t_buf_roll_sinusoids, fadein_type);
    CLASS_ATTR_STYLE_LABEL(c,"fadeintype",0,"enumindex","Fade In Type");
    CLASS_ATTR_ENUMINDEX(c,"fadeintype", 0, "None Linear Sine Curve S-Curve");
    // @description Sets the fade in type: 0 = None, 1 = Linear (default), 2 = Sine, 3 = Curve, 4 = S-Curve;
    
    CLASS_ATTR_CHAR(c, "fadeouttype", 0, t_buf_roll_sinusoids, fadeout_type);
    CLASS_ATTR_STYLE_LABEL(c,"fadeouttype",0,"enumindex","Fade Out Type");
    CLASS_ATTR_ENUMINDEX(c,"fadeouttype", 0, "None Linear Sine Curve S-Curve");
    // @description Sets the fade in type: 0 = None, 1 = Linear (default), 2 = Sine, 3 = Curve, 4 = S-Curve;

    
    CLASS_ATTR_DOUBLE(c, "fadeincurve", 0, t_buf_roll_sinusoids, fadein_curve);
    CLASS_ATTR_STYLE_LABEL(c,"fadeincurve",0,"text","Fade In Curve");
    // @description Sets the curve parameter for the fade in (for fades of type Curve and S-Curve only).
    // The parameters goes from -1 to 1, 0 being linear (default).
    
    CLASS_ATTR_DOUBLE(c, "fadeoutcurve", 0, t_buf_roll_sinusoids, fadeout_curve);
    CLASS_ATTR_STYLE_LABEL(c,"fadeoutcurve",0,"text","Fade Out Curve");
    // @description Sets the curve parameter for the fade out (for fades of type Curve and S-Curve only).
    // The parameters goes from -1 to 1, 0 being linear (default).

    CLASS_STICKY_ATTR_CLEAR(c, "category");

    
    CLASS_STICKY_ATTR(c,"category",0,"Pan");
    
    
    CLASS_ATTR_LONG(c, "panmode", 0, t_buf_roll_sinusoids, pan_mode);
    CLASS_ATTR_STYLE_LABEL(c,"panmode",0,"text","Pan Mode");
    CLASS_ATTR_ENUMINDEX(c,"panmode", 0, "Linear Circular");
    CLASS_ATTR_BASIC(c, "panmode", 0);
    // @description Sets the panning mode: 0 = linear (default); 1 = circular.
    
    
    CLASS_ATTR_LONG(c, "panlaw", 0, t_buf_roll_sinusoids, pan_law);
    CLASS_ATTR_STYLE_LABEL(c,"panlaw",0,"text","Pan Law");
    CLASS_ATTR_ENUMINDEX(c,"panlaw", 0, "Nearest Neighbor Cosine");
    // @description Sets the panning law: 0 = nearest neighbor (panned on one loudspeaker at a time);
    // 1 = cosine law (default).
    
    
    CLASS_ATTR_DOUBLE(c, "spread", 0, t_buf_roll_sinusoids, multichannel_spread);
    CLASS_ATTR_STYLE_LABEL(c,"spread",0,"text","Multichannel Spread");
    // @description Sets the spread of the input channels when panning multichannels buffers. Default is 0 (downmix to mono).
    // A spread of 1 spreads a multichannel signal sent to the central loudspeaker up to the external ones.
    
    CLASS_ATTR_CHAR(c, "compensate", 0, t_buf_roll_sinusoids, compensate_multichannel_gain_to_avoid_clipping);
    CLASS_ATTR_STYLE_LABEL(c,"compensate",0,"onoff","Reduce Multichannel Gain To Avoid Clipping");
    // @description Toggles the ability to automatically reduce the gain of multichannel files by a factor of the number of channels, in order
    // to avoid possible clipping while panning then with low <m>spread</m> values. Defaults to 1.
    
    CLASS_STICKY_ATTR_CLEAR(c, "category");

    
    CLASS_STICKY_ATTR(c,"category",0,"Velocity");
    
    
    CLASS_ATTR_LONG(c, "velmode", 0, t_buf_roll_sinusoids, veltoamp_mode);
    CLASS_ATTR_STYLE_LABEL(c,"velmode",0,"text","Velocity Mode");
    CLASS_ATTR_ENUMINDEX(c,"velmode", 0, "Ignore Map To Amplitude Map To Decibels");
    CLASS_ATTR_BASIC(c, "velmode", 0);
    // @description Sets the velocity mode: 0 = ignore; 1 = map velocity to amplitude range (default); 2 = map velocity to decibels range.
    // ranges are defined in <m>velrange</m>
    
    
    CLASS_ATTR_DOUBLE_ARRAY(c, "velrange", 0, t_buf_roll_sinusoids, velrange, 2);
    CLASS_ATTR_STYLE_LABEL(c,"velrange",0,"text","Velocity Mapping Range");
    // @description Sets the mapping range for the velocity. This can be either an amplitude range or a decibels range, depending on <m>velmode</m>.
    
    CLASS_STICKY_ATTR_CLEAR(c, "category");


    
    
    
    
    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
    return 0;
}

void buf_roll_sinusoids_assist(t_buf_roll_sinusoids *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        if (a == 0) // @in 0 @type llll @digest Gathered Syntax of <o>bach.roll</o>
            sprintf(s, "llll: Gathered Syntax");
    } else {
        sprintf(s, "Output Buffer Name"); // @out 0 @type symbol/list @digest Output buffer names(s)
                                            // @description Name of the bounced buffer
    }
}

void buf_roll_sinusoids_inletinfo(t_buf_roll_sinusoids *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_roll_sinusoids *buf_roll_sinusoids_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_roll_sinusoids *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_roll_sinusoids*)object_alloc_debug(s_tag_class);
    if (x) {
        x->normalization_mode = EARS_NORMALIZE_OVERLOAD_PROTECTION_ONLY;
        x->use_mute_solos = true;
        x->sr = 44100;
        x->num_channels = 2;
        x->gain_slot = 0;
        x->pan_slot = 0;
        
        x->pan_mode = EARS_PAN_MODE_LINEAR;
        x->pan_law = EARS_PAN_LAW_COSINE;
        x->multichannel_spread = 0.;
        x->compensate_multichannel_gain_to_avoid_clipping = true;
        
        x->veltoamp_mode = EARS_VELOCITY_TO_AMPLITUDE;
        x->velrange[0] = 0.;
        x->velrange[1] = 1.;
        
        x->middleAtuning = 440;

        x->fadein_type = x->fadeout_type = EARS_FADE_LINEAR;
        x->fadein_amount = x->fadeout_amount = 10;
        
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


void buf_roll_sinusoids_free(t_buf_roll_sinusoids *x)
{
    earsbufobj_free((t_earsbufobj *)x);
}

void buf_roll_sinusoids_bang(t_buf_roll_sinusoids *x)
{

    t_llll *roll_gs = llllobj_get_store_contents((t_object *)x, LLLL_OBJ_VANILLA, 0, false);
    t_buffer_obj *outbuf = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, 0);

    earsbufobj_mutex_lock((t_earsbufobj *)x);
    ears_roll_to_buffer((t_earsbufobj *)x, EARS_SCORETOBUF_MODE_SINUSOIDS, roll_gs, outbuf, x->use_mute_solos, true, x->num_channels,
                        0, 0, x->gain_slot, x->pan_slot, x->sr, (e_ears_normalization_modes)x->normalization_mode, EARS_CHANNELCONVERTMODE_PAN,
                        x->fadein_amount, x->fadeout_amount, (e_ears_fade_types)x->fadein_type, (e_ears_fade_types)x->fadeout_type,
                        x->fadein_curve, x->fadeout_curve,
                        (e_ears_pan_modes)x->pan_mode, (e_ears_pan_laws)x->pan_law, x->multichannel_spread, x->compensate_multichannel_gain_to_avoid_clipping,
                        (e_ears_veltoamp_modes)x->veltoamp_mode, x->velrange[0], x->velrange[1], x->middleAtuning);
    earsbufobj_mutex_unlock((t_earsbufobj *)x);
    
    earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
    
    llll_release(roll_gs);
    
}

void buf_roll_sinusoids_anything(t_buf_roll_sinusoids *x, t_symbol *msg, long ac, t_atom *av)
{
//    long inlet = proxy_getinlet((t_object *) x);
    
    if (msg != _sym_bang) {
        t_llll *parsed = earsbufobj_parse_gimme((t_earsbufobj *) x, LLLL_OBJ_VANILLA, msg, ac, av);
        llllobj_store_llll((t_object *)x, LLLL_OBJ_VANILLA, parsed, 0);
        buf_roll_sinusoids_bang(x);
    }
    
}


