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
	ears conversions
 
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
#include <vector>


typedef struct _buf_format {
    t_earsbufobj        e_ob;
    
    long                numchannels;
    double              duration;
    double              sr;
    
    t_symbol*           spectype;
    double              audiosr;
    double              binsize;
    double              binoffset;
    t_symbol*           binunit;
    t_llll*             bins;
    
    char                resample;
    char                channelmode_upmix;
    char                channelmode_downmix;

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

DEFINE_LLLL_ATTR_DEFAULT_GETTER(t_buf_format, bins, buf_format_getattr_bins);
DEFINE_LLLL_ATTR_DEFAULT_SETTER(t_buf_format, bins, buf_format_setattr_bins);


/**********************************************************************/
// Class Definition and Life Cycle

t_max_err buf_format_setattr_spectype(t_buf_format *x, void *attr, long argc, t_atom *argv)
{
    if (argc && argv) {
        if (atom_gettype(argv) == A_SYM) {
            x->spectype = atom_getsym(argv);
            object_attr_setdisabled((t_object *)x, gensym("hopsize"), x->spectype == _sym_none);
            object_attr_setdisabled((t_object *)x, gensym("framesize"), x->spectype == _sym_none);
            object_attr_setdisabled((t_object *)x, gensym("overlap"), x->spectype == _sym_none);
            object_attr_setdisabled((t_object *)x, gensym("antimeunit"), x->spectype == _sym_none);
        }
    }
    return MAX_ERR_NONE;
}

void C74_EXPORT ext_main(void* moduleRef)
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
    EARSBUFOBJ_DECLARE_COMMON_METHODS_HANDLETHREAD(format)

    
    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_blocking_attr(c);
    earsbufobj_class_add_timeunit_attr(c);
    earsbufobj_class_add_naming_attr(c);
    earsbufobj_class_add_polyout_attr(c);
    
    earsbufobj_class_add_antimeunit_attr(c);
    earsbufobj_class_add_hopsize_attr(c);
    earsbufobj_class_add_framesize_attr(c);
    earsbufobj_class_add_overlap_attr(c);

    earsbufobj_class_add_resamplingfiltersize_attr(c);
    earsbufobj_class_add_resamplingmode_attr(c);

    CLASS_ATTR_LONG(c, "numchannels",	0,	t_buf_format, numchannels);
    CLASS_ATTR_BASIC(c, "numchannels", 0);
    CLASS_ATTR_STYLE_LABEL(c, "numchannels", 0, "text", "Number Of Output Channels");
    // @description Sets the number of output channels. Negative or zero values mean: don't change.
    
    CLASS_ATTR_DOUBLE(c, "duration",	0,	t_buf_format, duration);
    CLASS_ATTR_BASIC(c, "duration", 0);
    CLASS_ATTR_STYLE_LABEL(c, "duration", 0, "text", "Duration");
    // @description Sets the buffer duration (unit given by the <m>timeunit</m> attribute)
    // Negative or zero values mean mean: don't change.
    
    CLASS_ATTR_DOUBLE(c, "sr",	0,	t_buf_format, sr);
    CLASS_ATTR_BASIC(c, "sr", 0);
    CLASS_ATTR_STYLE_LABEL(c, "sr", 0, "text", "Sample Rate");
    // @description Sets the sample rate for the buffer
    // Negative values mean: don't change.
    // Zero means: use current Max's sample rate
    
    CLASS_ATTR_CHAR(c, "resample",	0,	t_buf_format, resample);
    CLASS_ATTR_BASIC(c, "resample", 0);
    CLASS_ATTR_STYLE_LABEL(c, "resample", 0, "onoff", "Resample Buffers");
    // @description Toggles the ability to resample buffers when the sample rate has changed.
    
    CLASS_ATTR_CHAR(c, "channelmodeup",	0,	t_buf_format, channelmode_upmix);
    CLASS_ATTR_STYLE_LABEL(c, "channelmodeup", 0, "enumindex", "Up-Mixing Channel Conversion");
    CLASS_ATTR_ENUMINDEX(c,"channelmodeup", 0, "Clear Keep Pad Cycle Palindrome Pan");
    // @description Sets the channel conversion mode while upmixing: <br />
    // 0 (Clear) = Delete all samples <br />
    // 1 (Keep) = Only keep existing channels <br />
    // 2 (Pad) = Pad last channel while upmixing <br />
    // 3 (Cycle, default) = Repeat all channels (cycling) while upmixing <br />
    // 4 (Palindrome) = Palindrome cycling of channels while upmixing <br />
    // 5 (Pan) = Pan channels to new configuration <br />
    
    CLASS_ATTR_CHAR(c, "channelmodedown",    0,    t_buf_format, channelmode_downmix);
    CLASS_ATTR_STYLE_LABEL(c, "channelmodedown", 0, "enumindex", "Down-Mixing Channel Conversion");
    CLASS_ATTR_ENUMINDEX(c,"channelmodedown", 0, "Clear Keep Pad Cycle Palindrome Pan");
    // @description Sets the channel conversion mode while downmixing: <br />
    // 0 (Clear) = Delete all samples <br />
    // 1 (Keep) = Only keep existing channels <br />
    // 2 (Pad) = Pad last channel while upmixing <br />
    // 3 (Cycle, default) = Repeat all channels (cycling) while upmixing <br />
    // 4 (Palindrome) = Palindrome cycling of channels while upmixing <br />
    // 5 (Pan) = Pan channels to new configuration <br />
    
    
    CLASS_ATTR_SYM(c, "spectype",    0,    t_buf_format, spectype);
    CLASS_ATTR_STYLE_LABEL(c, "spectype", 0, "text", "Spectral Type");
    CLASS_ATTR_ACCESSORS(c, "spectype", NULL, buf_format_setattr_spectype);
    CLASS_ATTR_CATEGORY(c, "spectype", 0, "Spectral Type");
    // @description Sets the spectral type for spectral buffers.
    // A "keep" symbol means: don't change; a "none" symbol means: turn to ordinary buffer.
    // Any other symbol ("stft", "cqt", ...) will turn the buffer into a spectral buffer

    CLASS_ATTR_DOUBLE(c, "audiosr",    0,    t_buf_format, audiosr);
    CLASS_ATTR_STYLE_LABEL(c, "audiosr", 0, "text", "Audio Sample Rate for Spectral Buffers");
    CLASS_ATTR_CATEGORY(c, "audiosr", 0, "Spectral Buffers");
    // @description Sets the audio sample rate for spectral buffers
    // Negative or zero values mean: don't change.

    CLASS_ATTR_DOUBLE(c, "binsize",    0,    t_buf_format, binsize);
    CLASS_ATTR_STYLE_LABEL(c, "binsize", 0, "text", "Bin Size");
    CLASS_ATTR_CATEGORY(c, "binsize", 0, "Spectral Buffers");
    // @description Sets the bin size for spectral buffers.
    // Negative or zero values mean: don't change.

    CLASS_ATTR_DOUBLE(c, "binoffset",    0,    t_buf_format, binoffset);
    CLASS_ATTR_STYLE_LABEL(c, "binoffset", 0, "text", "Bin Offset");
    CLASS_ATTR_CATEGORY(c, "binoffset", 0, "Spectral Buffers");
    // @description Sets the bin offset for spectral buffers.
    // Negative values mean: don't change (beware: a 0 value means: change to zero, so use strictly
    // negative values to mean "don't change").

    CLASS_ATTR_SYM(c, "binunit", 0, t_buf_format, binunit);
    CLASS_ATTR_STYLE_LABEL(c,"binunit",0,"enumindex","Spectral Pitch Unit");
    CLASS_ATTR_ENUM(c,"binunit", 0, "keep cents MIDI hertz freqratio");
    CLASS_ATTR_CATEGORY(c, "binunit", 0, "Spectral Buffers");
    // @description Sets the unit for spectral bins: Keep (keep previous one) Cents, MIDI, Hertz (frequency), or frequency ratio.

    CLASS_ATTR_LLLL(c, "bins", 0, t_buf_format, bins, buf_format_getattr_bins, buf_format_setattr_bins);
    CLASS_ATTR_STYLE_LABEL(c,"bins",0,"text","Bin Positions");
    CLASS_ATTR_CATEGORY(c, "bins", 0, "Spectral Buffers");
    // @description Sets the position of each individual spectral bin, in the <m>binunit</m>.

    
    
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
        x->numchannels = -1;
        x->duration = -1;
        x->sr = -1;
        x->resample = true;
        x->spectype = gensym("keep");
        x->audiosr = -1;
        x->binunit = gensym("keep");
        x->binoffset = -1;
        x->binsize = -1;
        x->bins = llll_from_text_buf("keep");
        x->channelmode_upmix = EARS_CHANNELCONVERTMODE_CYCLE;
        x->channelmode_downmix = EARS_CHANNELCONVERTMODE_CYCLE;

        earsbufobj_init((t_earsbufobj *)x, EARSBUFOBJ_FLAG_SUPPORTS_COPY_NAMES);

        // @arg 0 @name outnames @optional 1 @type symbol
        // @digest Output buffer names
        // @description @copy EARS_DOC_OUTNAME_ATTR
        
        t_llll *args = llll_parse(true_ac, argv);
        t_llll *names = earsbufobj_extract_names_from_args((t_earsbufobj *)x, args);
        
        x->e_ob.a_hopsize = 0; // means: no change
        
        attr_args_process(x, argc, argv);

        earsbufobj_setup((t_earsbufobj *)x, "E", "E", names);

        llll_free(args);
        llll_free(names);
    }
    return x;
}


void buf_format_free(t_buf_format *x)
{
    llll_free(x->bins);
    earsbufobj_free((t_earsbufobj *)x);
}




void buf_format_bang(t_buf_format *x)
{
    long num_buffers = earsbufobj_get_instore_size((t_earsbufobj *)x, 0);
    double sr = x->sr;
    double duration = x->duration;
    long numchannels = x->numchannels;
    long channelmode_upmix = x->channelmode_upmix;
    long channelmode_downmix = x->channelmode_downmix;
    double audiosr = x->audiosr;
    t_symbol *spectype = x->spectype;
    t_symbol *binunit = x->binunit;
    double binsize = x->binsize;
    double binoffset = x->binoffset;
    t_llll *bins = x->bins;
    double hopsize = x->e_ob.a_hopsize;
    double framesize = x->e_ob.a_framesize;
    long must_be_spectral = (spectype != _llllobj_sym_empty_symbol && spectype != _llllobj_sym_none && spectype != gensym("keep"));

    if (num_buffers == 0) {
        // this is allowed if we use format as a pure generator
        earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, 1, false);
        num_buffers = earsbufobj_get_instore_size((t_earsbufobj *)x, 0);
    }
    
    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_buffers, true);
    
    earsbufobj_mutex_lock((t_earsbufobj *)x);
    earsbufobj_init_progress((t_earsbufobj *)x, num_buffers);

    for (long count = 0; count < num_buffers; count++) {
        t_buffer_obj *in = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, count);
        t_buffer_obj *out = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, count);
        
        if (in == NULL) { // this is allowed if we use format as a pure generator
            ears_buffer_setempty((t_object *)x, out, 1);
        } else if (in != out) {
            ears_buffer_clone((t_object *)x, in, out);
        }
        
        if (sr >= 0) {
            if (sr == 0) {
                sr = ears_get_current_Max_sr();
            }
            double curr_sr = ears_buffer_get_sr((t_object *)x, out);
            if (curr_sr != sr) {
                if (x->resample)
                    ears_buffer_convert_sr((t_object *)x, out, sr, x->e_ob.l_resamplingfilterwidth);
                else
                    ears_buffer_set_sr((t_object *)x, out, sr);
            }
        }
        
        if (duration > 0) {
            long duration_samps = earsbufobj_time_to_samps((t_earsbufobj *)x, duration, out);
            ears_buffer_convert_size((t_object *)x, out, duration_samps);
        }
        
        if (numchannels <= 0 && must_be_spectral && framesize > 0) {
            double framesize_samps = earsbufobj_time_to_fsamps((t_earsbufobj *)x, framesize, out, EARSBUFOBJ_CONVERSION_FLAG_ISANALYSIS | EARSBUFOBJ_CONVERSION_FLAG_USEORIGINALAUDIOSRFORSPECTRALBUFFERS);
            numchannels = framesize_samps/2 + 1;
        }
        
        if (numchannels > 0) {
            long curr_num_channels = ears_buffer_get_numchannels((t_object *)x, out);
            if (curr_num_channels != numchannels)
                ears_buffer_convert_numchannels((t_object *)x, out, numchannels, (e_ears_channel_convert_modes)channelmode_upmix, (e_ears_channel_convert_modes)channelmode_downmix);
        }
        
        // spectral ?
        e_ears_frequnit u = ears_frequnit_from_symbol(x->binunit);
        long is_spectral = ears_buffer_is_spectral((t_object *)x, out);
        if (is_spectral == 0 && must_be_spectral == 1) {
            t_ears_spectralbuf_metadata data;
            if (!bins->l_head || hatom_gettype(&bins->l_head->l_hatom) == H_SYM) {
                ears_spectralbuf_metadata_fill(&data, audiosr, binsize, binoffset, u, spectype, NULL, false);
            } else {
                ears_spectralbuf_metadata_fill(&data, audiosr, binsize, binoffset, u, spectype, bins, false);
            }
            ears_spectralbuf_metadata_set((t_object *)x, out, &data);
            
            // should we change the sampling rate?
            if (sr <= 0 && hopsize > 0) {
                double orig_audio_sr = ears_spectralbuf_get_original_audio_sr((t_object *)x, out);
                double hopsize_samps = earsbufobj_time_to_fsamps((t_earsbufobj *)x, hopsize, out, EARSBUFOBJ_CONVERSION_FLAG_ISANALYSIS |EARSBUFOBJ_CONVERSION_FLAG_USEORIGINALAUDIOSRFORSPECTRALBUFFERS);
                sr = orig_audio_sr/hopsize_samps;
                double curr_sr = ears_buffer_get_sr((t_object *)x, out);
                if (curr_sr != sr) {
                    if (x->resample)
                        ears_buffer_convert_sr((t_object *)x, out, sr, x->e_ob.l_resamplingfilterwidth);
                    else
                        ears_buffer_set_sr((t_object *)x, out, sr);
                }
            }


        } else if (is_spectral == 1 && must_be_spectral == 0) {
            ears_spectralbuf_metadata_remove((t_object *)x, out);
        } else {
            if (ears_buffer_is_spectral((t_object *)x, out)) {
                t_ears_spectralbuf_metadata *data = ears_spectralbuf_metadata_get((t_object *)x, out);
                if (binsize > 0)
                    data->binsize = binsize;
                if (binoffset >= 0)
                    data->binoffset = binoffset;
                if (spectype != gensym("keep"))
                    data->type = spectype;
                if (u != EARS_FREQUNIT_UNKNOWN)
                    data->binunit = u;
                if (audiosr > 0)
                    data->original_audio_signal_sr = audiosr;
                if (bins && !(bins->l_head && hatom_gettype(&bins->l_head->l_hatom) == H_SYM)) {
                    llll_free(data->bins);
                    data->bins = llll_clone(bins);
                }
                
                // should we change the sampling rate?
                if (sr <= 0 && hopsize > 0) {
                    double orig_audio_sr = ears_spectralbuf_get_original_audio_sr((t_object *)x, out);
                    double hopsize_samps = earsbufobj_time_to_fsamps((t_earsbufobj *)x, hopsize, out, EARSBUFOBJ_CONVERSION_FLAG_ISANALYSIS | EARSBUFOBJ_CONVERSION_FLAG_USEORIGINALAUDIOSRFORSPECTRALBUFFERS);
                    sr = orig_audio_sr/hopsize_samps;
                    double curr_sr = ears_buffer_get_sr((t_object *)x, out);
                    if (curr_sr != sr) {
                        if (x->resample)
                            ears_buffer_convert_sr((t_object *)x, out, sr, x->e_ob.l_resamplingfilterwidth);
                        else
                            ears_buffer_set_sr((t_object *)x, out, sr);
                    }
                }

            }
        }
        
        if (ears_buffer_is_spectral((t_object *)x, out)) {
            t_ears_spectralbuf_metadata *data = ears_spectralbuf_metadata_get((t_object *)x, out);
            if (spectype == gensym("stft")) {
                if (data->binsize < 0)
                    data->binsize = ears_spectralbuf_get_original_audio_sr((t_object *)x, out)/(2*(ears_buffer_get_numchannels((t_object *)x, out)-1));
                if (data->binoffset < 0)
                    data->binoffset = 0;
                if (data->binunit == EARS_FREQUNIT_UNKNOWN)
                    data->binunit = EARS_FREQUNIT_HERTZ;
/*            } else if (spectype == gensym("stc")) {
                if (data->binsize < 0)
                    data->binsize = ears_spectralbuf_get_original_audio_sr((t_object *)x, out)/(2*(ears_buffer_get_numchannels((t_object *)x, out)-1));
                if (data->binoffset < 0)
                    data->binoffset = 0;
                if (data->binunit == EARS_FREQUNIT_UNKNOWN)
                    data->binunit = EARS_FREQUNIT_QUEFRENCY_MS; */
            } else if (spectype == gensym("tempogram")) {
//                if (data->binsize < 0)
//                    data->binsize = ears_spectralbuf_get_original_audio_sr((t_object *)x, out)/(2*(ears_buffer_get_numchannels((t_object *)x, out)-1));
                if (data->binoffset < 0)
                    data->binoffset = 0;
                if (data->binunit == EARS_FREQUNIT_UNKNOWN)
                    data->binunit = EARS_FREQUNIT_BPM;
            } else if (spectype == gensym("cqt")) {
//                if (data->binsize < 0)
//                    data->binsize = ears_spectralbuf_get_original_audio_sr((t_object *)x, out)/(2*(ears_buffer_get_numchannels((t_object *)x, out)-1));
                if (data->binoffset < 0)
                    data->binoffset = 0;
                if (data->binunit == EARS_FREQUNIT_UNKNOWN)
                    data->binunit = EARS_FREQUNIT_CENTS;
            }
        }

        if (earsbufobj_iter_progress((t_earsbufobj *)x, count, num_buffers)) break;
    }
    
    earsbufobj_mutex_unlock((t_earsbufobj *)x);

    earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
}


void buf_format_anything(t_buf_format *x, t_symbol *msg, long ac, t_atom *av)
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
            
            buf_format_bang(x);
        }
    }
    llll_free(parsed);
}


