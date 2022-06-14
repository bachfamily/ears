/**
	@file
	ears.roll.toreaper.c
 
	@name
	ears.roll.toreaper~
 
	@realname
	ears.roll.toreaper~

	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	Export a <o>bach.roll</o> as a Reaper project
 
	@description
	Convert a <o>bach.roll</o> into a Reaper project containing a montage of audio samples
 
	@discussion
 
	@category
	ears export
 
	@keywords
	buffer, roll, bounce, export, Reaper
 
	@seealso
	ears.roll.sampling~, ears.roll.sinusoids~
	
	@owner
	Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "foundation/llllobj.h"
#include "foundation/llll_commons_ext.h"
#include "math/bach_math_utilities.h"
#include "ears.scores.h"


typedef struct _buf_roll_toreaper {
    t_earsbufobj       e_ob;
    
    t_symbol    *outfile;
    
    // export buffer options
    t_symbol    *buffer_format;
    t_symbol    *buffer_filetype;
    
    // Reaper specs
    t_symbol    *reaper_header;
    
    // use pitch as transposition?
    char        pitch_is_transposition;
    long        base_pitch_mc;
    
    char        auto_xfade;
    t_llll      *num_channels; // number of channels per voice
    
    char        copy_media;
    t_symbol    *media_folder_name;
    
    char        use_durations;

    long        filename_slot;
    long        offset_slot;
    long        gain_slot;
    long        pan_slot;
    long        transp_slot;
    long        timestretch_slot;
    long        fade_slot;
    long        color_slot;

    // fades
    double      fadein_amount;
    double      fadeout_amount;
    char        fadein_type;
    char        fadeout_type;
    double      fadein_curve;
    double      fadeout_curve;
    
    // velocity to gain
    e_ears_veltoamp_modes veltoamp_mode;
    double velrange[2];

} t_buf_roll_toreaper;



// Prototypes
t_buf_roll_toreaper*     buf_roll_toreaper_new(t_symbol *s, short argc, t_atom *argv);
void			buf_roll_toreaper_free(t_buf_roll_toreaper *x);
void			buf_roll_toreaper_bang(t_buf_roll_toreaper *x);
void			buf_roll_toreaper_anything(t_buf_roll_toreaper *x, t_symbol *msg, long ac, t_atom *av);

void buf_roll_toreaper_assist(t_buf_roll_toreaper *x, void *b, long m, long a, char *s);
void buf_roll_toreaper_inletinfo(t_buf_roll_toreaper *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;


EARSBUFOBJ_ADD_IO_METHODS(roll_toreaper)
DEFINE_LLLL_ATTR_DEFAULT_GETTER(t_buf_roll_toreaper, num_channels, buf_roll_toreaper_getattr_numchannels);
DEFINE_LLLL_ATTR_DEFAULT_SETTER(t_buf_roll_toreaper, num_channels, buf_roll_toreaper_setattr_numchannels);


t_max_err buf_roll_toreaper_setattr_format(t_buf_roll_toreaper *x, void *attr, long argc, t_atom *argv)
{
    t_max_err err = MAX_ERR_NONE;
    if (argc && argv) {
        if (atom_gettype(argv) == A_LONG) {
            long bitdepth = atom_getlong(argv);
            if (bitdepth == 8)
                x->buffer_format = _sym_int8;
            else if (bitdepth == 16)
                x->buffer_format = _sym_int16;
            else if (bitdepth == 24)
                x->buffer_format = _sym_int24;
            else  if (bitdepth == 32)
                x->buffer_format = _sym_int32;
            else {
                object_error((t_object *)x, "Unknown sample format.");
                err = MAX_ERR_GENERIC;
            }
        } else if (atom_gettype(argv) == A_SYM) {
            t_symbol *s = atom_getsym(argv);
            if (s == _sym_int8 || s == _sym_int16 || s == _sym_int24 || s == _sym_int32 || s == _sym_float32 || s == _sym_float64 || s == _sym_mulaw || s == gensym("alaw"))
                x->buffer_format = s;
            else {
                object_error((t_object *)x, "Unknown sample format.");
                err = MAX_ERR_GENERIC;
            }
        } else {
            object_error((t_object *)x, "Invalid sample format.");
            err = MAX_ERR_GENERIC;
        }
    }
    return err;
}


void buf_roll_toreaper_open(t_buf_roll_toreaper *x)
{
    char cmd[2048];
    t_symbol *outfile_resolved = x->outfile && strlen(x->outfile->s_name) > 0 ? ears_ezlocate_file(x->outfile, NULL) : NULL;
    if (outfile_resolved && strlen(outfile_resolved->s_name) > 0) {
        snprintf_zero(cmd, 2048, "open %s", outfile_resolved->s_name);
        system(cmd);
    }
}

void buf_roll_toreaper_dblclick(t_buf_roll_toreaper *x)
{
    buf_roll_toreaper_open(x);
}


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
    
    CLASS_NEW_CHECK_SIZE(c, "ears.roll.toreaper~",
                         (method)buf_roll_toreaper_new,
                         (method)buf_roll_toreaper_free,
                         sizeof(t_buf_roll_toreaper),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    EARSBUFOBJ_DECLARE_COMMON_METHODS_HANDLETHREAD(roll_toreaper)
    
    // @method (mouse) @digest Open the exported session.
    // @description Double-clicking opens the exported session in Reaper.
    class_addmethod(c, (method)buf_roll_toreaper_dblclick,                    "dblclick",                A_CANT,   0);

    // @method open @digest Open the exported session.
    // @description Opens the exported session in Reaper.
    class_addmethod(c, (method)buf_roll_toreaper_open,                    "open", 0);

    
    earsbufobj_class_add_blocking_attr(c);
    earsbufobj_class_add_timeunit_attr(c);
    
    CLASS_STICKY_ATTR(c,"category",0,"Export");

    CLASS_ATTR_CHAR(c, "copymedia", 0, t_buf_roll_toreaper, copy_media);
    CLASS_ATTR_STYLE_LABEL(c,"copymedia",0,"onoff","Copy Media");
    CLASS_ATTR_BASIC(c, "copymedia", 0);
    // @description Toggles the ability to copy all files locally in a media folder
    
    CLASS_ATTR_SYM(c, "mediafolder", 0, t_buf_roll_toreaper, media_folder_name);
    CLASS_ATTR_STYLE_LABEL(c,"mediafolder",0,"text","Media Folder Name");
    // @description Sets the name of the media folder (only relevant if <m>copymedia</m> is on).

    CLASS_ATTR_SYM(c, "bufferformat", 0, t_buf_roll_toreaper, buffer_format);
    CLASS_ATTR_STYLE_LABEL(c, "bufferformat", 0, "enum", "Sample Format for Buffer Export");
    CLASS_ATTR_ENUM(c,"bufferformat", 0, "int8 int16 int24 int32 float32 float64 mulaw alaw");
    CLASS_ATTR_ACCESSORS(c, "bufferformat", NULL, buf_roll_toreaper_setattr_format);
    CLASS_ATTR_BASIC(c, "bufferformat", 0);
    // @description Sets the bit depth or sample type when exporting buffers, just like for the <o>buffer~</o> object.
    // Note that this is only relevant if <m>copymedia</m> is on, and if you are using buffers instead of files. <br />
    // @copy EARS_DOC_ACCEPTED_SAMPLETYPES

    CLASS_ATTR_SYM(c, "bufferfiletype", 0, t_buf_roll_toreaper, buffer_filetype);
    CLASS_ATTR_STYLE_LABEL(c, "bufferfiletype", 0, "enum", "Sample Type for Buffer Export");
    CLASS_ATTR_ENUM(c,"bufferfiletype", 0, "aiff wav au flac raw");
    CLASS_ATTR_ACCESSORS(c, "bufferfiletype", NULL, buf_roll_toreaper_setattr_format);
    CLASS_ATTR_BASIC(c, "bufferfiletype", 0);
    // @description Sets the file type for buffer export, just like for the <o>buffer~</o> object.
    // Note that this is only relevant if <m>copymedia</m> is on, and if you are using buffers instead of files.

    CLASS_STICKY_ATTR_CLEAR(c, "export");

    
    
    CLASS_STICKY_ATTR(c,"category",0,"Reaper");

    CLASS_ATTR_SYM(c, "reaperheader", 0, t_buf_roll_toreaper, reaper_header);
    CLASS_ATTR_STYLE_LABEL(c,"reaperheader",0,"text","Reaper Header");
    // @description Sets the Reaper header (containing version number) in textual form

    CLASS_STICKY_ATTR_CLEAR(c, "export");

    
    
    CLASS_STICKY_ATTR(c,"category",0,"Behavior");

    CLASS_ATTR_CHAR(c, "usedurations", 0, t_buf_roll_toreaper, use_durations);
    CLASS_ATTR_STYLE_LABEL(c,"usedurations",0,"onoff","Account For Note Durations");
    // @description Toggles the ability to account for note durations while bouncing.

    CLASS_ATTR_CHAR(c, "autoxfade", 0, t_buf_roll_toreaper, auto_xfade);
    CLASS_ATTR_STYLE_LABEL(c,"autoxfade",0,"onoff","Crossfades When Items Overlap");
    CLASS_ATTR_BASIC(c, "autoxfade", 0);
    // @description Toggles the ability to automatically crossfade items when they overlap
    
    CLASS_STICKY_ATTR_CLEAR(c, "export");

    
    
    CLASS_STICKY_ATTR(c,"category",0,"Transposition");
    
    CLASS_ATTR_CHAR(c, "usepitch", 0, t_buf_roll_toreaper, pitch_is_transposition);
    CLASS_ATTR_STYLE_LABEL(c,"usepitch",0,"onoff","Use Pitch For Transposition");
    // @description Toggles the ability to use the pitch as transposition (also see <m>pitchbase</m>)

    CLASS_ATTR_LONG(c, "pitchbase", 0, t_buf_roll_toreaper, base_pitch_mc);
    CLASS_ATTR_STYLE_LABEL(c,"pitchbase",0,"text","Pitch Base For Transposition");
    // @description Sets the pitch (in midicents) corresponding to no transposition (only meaningful if
    // <m>usepitch</m> is on).
    
    CLASS_STICKY_ATTR_CLEAR(c, "category");

    
    

    CLASS_STICKY_ATTR(c,"category",0,"Slots");

    CLASS_ATTR_LONG(c, "fileslot", 0, t_buf_roll_toreaper, filename_slot);
    CLASS_ATTR_STYLE_LABEL(c,"fileslot",0,"text","Slot Containing File Names");
    CLASS_ATTR_BASIC(c, "fileslot", 0);
    // @description Sets the number of slots containing the file names.

    CLASS_ATTR_LONG(c, "offsetslot", 0, t_buf_roll_toreaper, offset_slot);
    CLASS_ATTR_STYLE_LABEL(c,"offsetslot",0,"text","Slot Containing Offset In File");
    CLASS_ATTR_BASIC(c, "offsetslot", 0);
    // @description Sets the number of slots containing the offset from the beginning of the file.

    CLASS_ATTR_LONG(c, "gainslot", 0, t_buf_roll_toreaper, gain_slot);
    CLASS_ATTR_STYLE_LABEL(c,"gainslot",0,"text","Slot Containing Gain");
    // @description Sets the number of slots containing the gain or gain envelope.

    CLASS_ATTR_LONG(c, "panslot", 0, t_buf_roll_toreaper, pan_slot);
    CLASS_ATTR_STYLE_LABEL(c,"panslot",0,"text","Slot Containing Pan");
    // @description Sets the number of slots containing the file names (0 = none).

    CLASS_ATTR_LONG(c, "transpositionslot", 0, t_buf_roll_toreaper, transp_slot);
    CLASS_ATTR_STYLE_LABEL(c,"transpositionslot",0,"text","Slot Containing Transposition Amount");
    // @description Sets the number of slots containing the transposition amount, in midicents (0 = none).

    CLASS_ATTR_LONG(c, "timestretchslot", 0, t_buf_roll_toreaper, timestretch_slot);
    CLASS_ATTR_STYLE_LABEL(c,"timestretchslot",0,"text","Slot Containing Timestretch Ratio");
    // @description Sets the number of slots containing the timestretch ratio (0 = none).

    CLASS_ATTR_LONG(c, "fadeslot", 0, t_buf_roll_toreaper, fade_slot);
    CLASS_ATTR_STYLE_LABEL(c,"fadeslot",0,"text","Slot Containing Fade Amount");
    // @description Sets the number of slots containing the fade amount (0 = none).
    // If slots contains a single number, it will be used both for in and out fade; if slot
    // contains two numbers, they will be assigned respectively to fade in and fade out
    // The unit can be defined by the <m>timeunit</m> attribute, although milliseconds
    // should be used preferably, for best CPU performances.

    CLASS_ATTR_LONG(c, "colorslot", 0, t_buf_roll_toreaper, color_slot);
    CLASS_ATTR_STYLE_LABEL(c,"colorslot",0,"text","Slot Containing Color Information");
    // @description Sets the number of slots containing the color information (0 = none).
    // It is expected to be a slot of type color, or floatlist, containing at least three items,
    // corresponding to the R, G, B channels (each 0. to 1.)
    
    CLASS_STICKY_ATTR_CLEAR(c, "category");

    
    
    CLASS_STICKY_ATTR(c,"category",0,"Multichannel");

    CLASS_ATTR_LLLL(c, "numchannels", 0, t_buf_roll_toreaper, num_channels, buf_roll_toreaper_getattr_numchannels, buf_roll_toreaper_setattr_numchannels);
    CLASS_ATTR_STYLE_LABEL(c,"numchannels",0,"text","Number Of Channels Per Voice");
    CLASS_ATTR_BASIC(c, "numchannels", 0);
    // @description Sets the number of channels for each voice (each voice will be converted into a track with that
    // number of channels)

    CLASS_STICKY_ATTR_CLEAR(c, "category");

    
    
    CLASS_STICKY_ATTR(c,"category",0,"Fade");
    
    CLASS_ATTR_DOUBLE(c, "fadein", 0, t_buf_roll_toreaper, fadein_amount);
    CLASS_ATTR_STYLE_LABEL(c,"fadein",0,"text","Fade In Amount");
    // @description Sets a global amount of fade in for every sample (in units given by the <m>timeunit</m> attribute).

    CLASS_ATTR_DOUBLE(c, "fadeout", 0, t_buf_roll_toreaper, fadeout_amount);
    CLASS_ATTR_STYLE_LABEL(c,"fadeout",0,"text","Fade Out Amount");
    // @description Sets a global amount of fade out for every sample (in units given by the <m>timeunit</m> attribute).

    CLASS_ATTR_CHAR(c, "fadeintype", 0, t_buf_roll_toreaper, fadein_type);
    CLASS_ATTR_STYLE_LABEL(c,"fadeintype",0,"enumindex","Fade In Type");
    CLASS_ATTR_ENUMINDEX(c,"fadeintype", 0, "None Linear Sine Curve S-Curve");
    // @description Sets the fade in type: 0 = None, 1 = Linear (default), 2 = Sine, 3 = Curve, 4 = S-Curve;
    
    CLASS_ATTR_CHAR(c, "fadeouttype", 0, t_buf_roll_toreaper, fadeout_type);
    CLASS_ATTR_STYLE_LABEL(c,"fadeouttype",0,"enumindex","Fade Out Type");
    CLASS_ATTR_ENUMINDEX(c,"fadeouttype", 0, "None Linear Sine Curve S-Curve");
    // @description Sets the fade in type: 0 = None, 1 = Linear (default), 2 = Sine, 3 = Curve, 4 = S-Curve;

    
    CLASS_ATTR_DOUBLE(c, "fadeincurve", 0, t_buf_roll_toreaper, fadein_curve);
    CLASS_ATTR_STYLE_LABEL(c,"fadeincurve",0,"text","Fade In Curve");
    // @description Sets the curve parameter for the fade in (for fades of type Curve and S-Curve only).
    // The parameters goes from -1 to 1, 0 being linear (default).
    
    CLASS_ATTR_DOUBLE(c, "fadeoutcurve", 0, t_buf_roll_toreaper, fadeout_curve);
    CLASS_ATTR_STYLE_LABEL(c,"fadeoutcurve",0,"text","Fade Out Curve");
    // @description Sets the curve parameter for the fade out (for fades of type Curve and S-Curve only).
    // The parameters goes from -1 to 1, 0 being linear (default).

    CLASS_STICKY_ATTR_CLEAR(c, "category");

    
    
    CLASS_STICKY_ATTR(c,"category",0,"Velocity");
    
    
    CLASS_ATTR_LONG(c, "velmode", 0, t_buf_roll_toreaper, veltoamp_mode);
    CLASS_ATTR_STYLE_LABEL(c,"velmode",0,"text","Velocity Mode");
    CLASS_ATTR_ENUMINDEX(c,"velmode", 0, "Ignore Map To Amplitude Map To Decibels");
    CLASS_ATTR_BASIC(c, "velmode", 0);
    // @description Sets the velocity mode: 0 = ignore; 1 = map velocity to amplitude range (default); 2 = map velocity to decibels range.
    // ranges are defined in <m>velrange</m>
    
    
    CLASS_ATTR_DOUBLE_ARRAY(c, "velrange", 0, t_buf_roll_toreaper, velrange, 2);
    CLASS_ATTR_STYLE_LABEL(c,"velrange",0,"text","Velocity Mapping Range");
    // @description Sets the mapping range for the velocity. This can be either an amplitude range or a decibels range, depending on <m>velmode</m>.
    
    CLASS_STICKY_ATTR_CLEAR(c, "category");


    
    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
    return 0;
}

void buf_roll_toreaper_assist(t_buf_roll_toreaper *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        if (a == 0) // @in 0 @type llll @digest Gathered Syntax of <o>bach.roll</o>
            sprintf(s, "llll: Gathered Syntax");
        else
            sprintf(s, "symbol: File path");
    } else {
        sprintf(s, "bang When Done"); // @out 0 @type bang @digest bang When Done
    }
}

void buf_roll_toreaper_inletinfo(t_buf_roll_toreaper *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_roll_toreaper *buf_roll_toreaper_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_roll_toreaper *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_roll_toreaper*)object_alloc_debug(s_tag_class);
    if (x) {
        x->auto_xfade = 1;
        x->outfile = NULL;
        x->reaper_header = gensym("0.1 \"6.12c/OSX64\" 1603116636");
        
        x->copy_media = false;
        x->media_folder_name = gensym("audio");
        x->buffer_format = _sym_float32;
        x->buffer_filetype = gensym("aiff");
        
        x->base_pitch_mc = 6000;
        x->pitch_is_transposition = false;
        
        x->use_durations = true;
        x->num_channels = llll_from_text_buf("");
        x->offset_slot = 0;
        x->filename_slot = 8;
        x->gain_slot = 0;
        x->pan_slot = 0;

        x->veltoamp_mode = EARS_VELOCITY_TO_AMPLITUDE;
        x->velrange[0] = 0.;
        x->velrange[1] = 1.;

        x->fadein_type = x->fadeout_type = EARS_FADE_LINEAR;
        x->fadein_amount = x->fadeout_amount = 10; // by default ms

        earsbufobj_init((t_earsbufobj *)x, 0);

        // @arg 0 @name file @optional 1 @type symbol
        // @digest Output file name or path
        
        t_llll *args = llll_parse(true_ac, argv);
        if (args && args->l_head && hatom_gettype(&args->l_head->l_hatom) == H_SYM)
            x->outfile = hatom_getsym(&args->l_head->l_hatom);
        
        attr_args_process(x, argc, argv);

        earsbufobj_setup((t_earsbufobj *)x, "4", "b", NULL);

        llll_free(args);
    }
    return x;
}


void buf_roll_toreaper_free(t_buf_roll_toreaper *x)
{
    earsbufobj_free((t_earsbufobj *)x);
}

void buf_roll_toreaper_bang(t_buf_roll_toreaper *x)
{
    if (!x->outfile){
        object_error((t_object *)x, "Output file name or path has not been defined.");
        return;
    }
    
    t_llll *roll_gs = llllobj_get_store_contents((t_object *)x, LLLL_OBJ_VANILLA, 0, true);
    earsbufobj_mutex_lock((t_earsbufobj *)x);
    
    ears_roll_to_reaper((t_earsbufobj *)x, x->outfile, x->reaper_header, EARS_SCORETOBUF_MODE_SAMPLING,
                        roll_gs, x->use_durations, x->pitch_is_transposition, x->base_pitch_mc,
                        x->filename_slot, x->offset_slot, x->gain_slot, x->pan_slot,
                        x->transp_slot, x->timestretch_slot, x->fade_slot, x->color_slot,
                        x->fadein_amount, x->fadeout_amount, (e_ears_fade_types)x->fadein_type, (e_ears_fade_types)x->fadeout_type,
                        x->fadein_curve, x->fadeout_curve,
                        (e_ears_veltoamp_modes)x->veltoamp_mode, x->velrange[0], x->velrange[1],
                        x->num_channels, x->auto_xfade, x->copy_media, x->media_folder_name, x->buffer_format, x->buffer_filetype);
    
    earsbufobj_mutex_unlock((t_earsbufobj *)x);
    
    earsbufobj_outlet_bang((t_earsbufobj *)x, 0);
    
    llll_free(roll_gs);
        
    
}

void buf_roll_toreaper_anything(t_buf_roll_toreaper *x, t_symbol *msg, long ac, t_atom *av)
{
    long inlet = earsbufobj_proxy_getinlet((t_earsbufobj *) x);
    
    t_llll *parsed = earsbufobj_parse_gimme((t_earsbufobj *) x, LLLL_OBJ_VANILLA, msg, ac, av);
    if (!parsed) return;
    
    if (parsed && parsed->l_head) {
        if (inlet == 0) {
            llllobj_store_llll((t_object *)x, LLLL_OBJ_VANILLA, parsed, 0);
            buf_roll_toreaper_bang(x);
            // won't free parsed llll, it's up to the store
        } else if (inlet == 1) {
            earsbufobj_mutex_lock((t_earsbufobj *)x);
            if (parsed->l_head && hatom_gettype(&parsed->l_head->l_hatom) == H_SYM)
                x->outfile = hatom_getsym(&parsed->l_head->l_hatom);
            else
                object_warn((t_object *)x, "Wrong file format");
            earsbufobj_mutex_unlock((t_earsbufobj *)x);
            llll_free(parsed);
        }
    }
}


