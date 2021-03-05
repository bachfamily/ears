/**
	@file
	ears.commons.h
	Common utilities header for the buffer ears sublibrary
 
	by Daniele Ghisi
 */

#ifndef _EARS_BUF_COMMONS_H_
#define _EARS_BUF_COMMONS_H_

#define EARS_ERROR_BUF_CANT_READ "Can't read from buffer"
#define EARS_ERROR_BUF_CANT_WRITE "Can't write to buffer"
#define EARS_ERROR_BUF_NO_BUFFER "No buffer given"
#define EARS_ERROR_BUF_EMPTY_BUFFER "Empty buffer"
#define EARS_ERROR_BUF_ZERO_AMP "Amplitude is zero"
#define EARS_ERROR_BUF_NO_SEGMENTS "There are no output segments"
#define EARS_ERROR_BUF_NO_BUFFER_NAMED "There is no buffer named '%s'"
#define EARS_ERROR_BUF_NO_FILE_NAMED "Can't find file '%s'"
#define EARS_WARNING_BUF_CANT_SEEK "Can't seek"

#define EARS_MAX_BUFFERS_SHOWN_ON_DOUBLECLICK 10
#define EARS_MAX_NUM_CHANNELS 2048 // max num channels per buffer
#define EARS_DEFAULT_SR 44100

#define EARS_FROMFILE_NATIVE_MP3_HANDLING

#include "ext.h"
#include "ext_obex.h"
#include "ext_buffer.h"
#include "ext_strings.h"

#include "ears.conversions.h" // llllstuff is included in here
#include "lexpr.h"
#include "bach_math_utilities.h"
#include "ears.object.h" // already included in previous one
#include "ears.utils.h"
#include "notation.h"


#ifdef EARS_FROMFILE_NATIVE_MP3_HANDLING
#include "mpg123.h"
#endif


typedef t_atom_long t_ears_err;		///< an integer value suitable to be returned as an error code  @ingroup misc


/** VBR Enconding types for mp3's
    @ingroup mp3 */
typedef enum {
    EARS_MP3_VBRMODE_CBR,    ///< Constant bit rate
    EARS_MP3_VBRMODE_ABR,    ///< Average bit rate
    EARS_MP3_VBRMODE_VBR,    ///< Variable bit rate
} e_ears_mp3_encoding_vbrmode;


/** Enconding settings for compressed stuff
    @ingroup mp3 */
typedef struct _ears_encoding_settings
{
    e_ears_mp3_encoding_vbrmode  vbr_type;
    int                         bitrate;
    int                         bitrate_max;
    int                         bitrate_min;
    
    // For wavpack
    char        use_correction_file;
    t_symbol    *format;
} t_ears_encoding_settings;


/** Standard values returned by function calls with a return type of #t_ears_err
	@ingroup misc */
typedef enum {
    EARS_ERR_NONE =          0,	///< No error
    EARS_ERR_GENERIC =		-1,	///< Generic error
    EARS_ERR_INVALID_PTR =	-2,	///< Invalid Pointer
    EARS_ERR_DUPLICATE =	-3,	///< Duplicate
    EARS_ERR_OUT_OF_MEM =	-4,	///< Out of memory
    EARS_ERR_CANT_WRITE =   -5, ///< Can't write buffer
    EARS_ERR_CANT_READ =    -6, ///< Can't read buffer
    EARS_ERR_NO_BUFFER =    -7,  ///< Can't find buffer
    EARS_ERR_EMPTY_BUFFER = -8,  ///< Empty buffer
    EARS_ERR_ZERO_AMP =     -9,  ///< Zero amplitude buffer (e.g. for normalization)
    EARS_ERR_NO_FILE =      -10  ///< Can't find file
} e_ears_errorcodes;

/** Fade types
	@ingroup misc */
typedef enum {
    EARS_FADE_NONE =          0,	///< No fade
    EARS_FADE_LINEAR =        1,	///< Linear fade (or cross fade)
    EARS_FADE_SINE =    2,	///< Equal power fade (or crossfade)
    EARS_FADE_CURVE =         3,	///< Generic curve fade. One additional parameter (-1 to 1, 0. being linear) is needed for the slope
    EARS_FADE_SCURVE =        4,	///< S-shaped curve fade. One additional parameter (-1 to 1, 0. being linear) is needed for the slope
} e_ears_fade_types;


/** Pan modes
	@ingroup misc */
typedef enum {
    EARS_PAN_MODE_LINEAR =          0,	///< Linear panning
    EARS_PAN_MODE_CIRCULAR =        1,	///< Circular panning
} e_ears_pan_modes;


/** Pan laws
	@ingroup misc */
typedef enum {
    EARS_PAN_LAW_NEAREST_NEIGHBOR =          0,	///< Pan the sound to a single loudspeaker at a time
    EARS_PAN_LAW_COSINE =                    1, ///< Cosine panning
} e_ears_pan_laws;


/** Split modes
    @ingroup misc */
typedef enum _ears_split_modes {
    EARS_SPLIT_MODE_DURATION = 0,
    EARS_SPLIT_MODE_NUMBER = 1,
    EARS_SPLIT_MODE_LIST = 2,
    EARS_SPLIT_MODE_SILENCE = 3,
    EARS_SPLIT_MODE_ONSET = 4,
} e_ears_split_modes;


/** Velocity to amplitude conversions
	@ingroup velocity */
typedef enum {
    EARS_VELOCITY_IGNORE =          0,	///< Ignore velocity
    EARS_VELOCITY_TO_AMPLITUDE =    1, ///< Map velocity on amplitude range
    EARS_VELOCITY_TO_DECIBEL =      2, ///< Map velocity on decibel range
} e_ears_veltoamp_modes;



/** Normalization modes
	@ingroup misc */
typedef enum {
    EARS_NORMALIZE_DONT =                       0,	///< Don't normalize
    EARS_NORMALIZE_DO =                         1,	///< Do normalize
    EARS_NORMALIZE_OVERLOAD_PROTECTION_ONLY =   2,	///< Only normalize if some samples have modulo > 1.
} e_ears_normalization_modes;



/** Channel conversion modes
	@ingroup misc */
typedef enum {
    EARS_CHANNELCONVERTMODE_CLEAR =         0,	///< Delete content
    EARS_CHANNELCONVERTMODE_KEEP =          1,	///< Only keep the channels that were already there, and/or add empty ones
    EARS_CHANNELCONVERTMODE_PAD =           2,	///< Pad with last channel when upsampling
    EARS_CHANNELCONVERTMODE_CYCLE =         3,	///< Cycle channels while upsampling
    EARS_CHANNELCONVERTMODE_PALINDROME =    4,	///< Palindrome cycling of channels while upsampling
    EARS_CHANNELCONVERTMODE_PAN =           5,	///< Re-pan channels
} e_ears_channel_convert_modes;




typedef struct _ears_envelope_iterator
{
    t_llll      *env;               ///< The envelope
    
    t_llllelem  *left_el;
    t_llllelem  *right_el;
    t_pts       left_pts;
    t_pts       right_pts;
    double      default_val;
    
    char        use_decibels;
    
    e_slope_mapping slopemapping;
} t_ears_envelope_iterator;




t_symbol *ears_bufferref_to_name(t_buffer_ref *ref);

t_ears_err ears_buffer_crop(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, long start_sample, long end_sample);
t_ears_err ears_buffer_crop_ms(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, double start_ms, double end_ms);
t_ears_err ears_buffer_crop_inplace(t_object *ob, t_buffer_obj *buf, long start_sample, long end_sample);
t_ears_err ears_buffer_crop_ms_inplace(t_object *ob, t_buffer_obj *buf, double start_ms, double end_ms);
t_ears_err ears_buffer_crop_ms_inplace_maxapi(t_object *ob, t_buffer_obj *buf, long start_ms, long end_ms);
t_ears_err ears_buffer_offset(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, long shift_samps);
t_ears_err ears_buffer_trim(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, double amp_thresh_linear, char trim_start, char trim_end);


t_ears_err ears_buffer_clone(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest);

// Fades
t_ears_err ears_buffer_fade(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, long fade_in_samples, long fade_out_samples, e_ears_fade_types fade_in_type, e_ears_fade_types fade_out_type, double fade_in_curve, double fade_out_curve, e_slope_mapping slopemapping);
t_ears_err ears_buffer_fade_ms(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, long fade_in_ms, long fade_out_ms, e_ears_fade_types fade_in_type, e_ears_fade_types fade_out_type, double fade_in_curve, double fade_out_curve, e_slope_mapping slopemapping);
t_ears_err ears_buffer_fade_inplace(t_object *ob, t_buffer_obj *buf, long fade_in_samples, long fade_out_samples, e_ears_fade_types fade_in_type, e_ears_fade_types fade_out_type, double fade_in_curve, double fade_out_curve, e_slope_mapping slopemapping);
t_ears_err ears_buffer_fade_ms_inplace(t_object *ob, t_buffer_obj *buf, long fade_in_ms, long fade_out_ms, e_ears_fade_types fade_in_type, e_ears_fade_types fade_out_type, double fade_in_curve, double fade_out_curve, e_slope_mapping slopemapping);


t_ears_err ears_buffer_concat(t_object *ob, t_buffer_obj **source, long num_sources, t_buffer_obj *dest,
                              long *xfade_samples, char also_fade_boundaries,
                              e_ears_fade_types fade_type, double fade_curve, e_slope_mapping slopemapping);
t_ears_err ears_buffer_gain(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, double gain_factor, char use_decibels); // also work inplace, with source == dest
t_ears_err ears_buffer_gain_envelope(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, t_llll *thresh, char thresh_is_in_decibel, e_slope_mapping slopemapping); // also work inplace, with source == dest
t_ears_err ears_buffer_clip(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, double gain_factor, char use_decibels); // also work inplace, with source == dest
t_ears_err ears_buffer_clip_envelope(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, t_llll *thresh, char thresh_is_in_decibel, e_slope_mapping slopemapping); // also work inplace, with source == dest
t_ears_err ears_buffer_overdrive(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, double drive); // also work inplace, with source == dest
t_ears_err ears_buffer_overdrive_envelope(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, t_llll *drive, e_slope_mapping slopemapping); // also work inplace, with source == dest
t_ears_err ears_buffer_normalize(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, double linear_amp_level, double mix); // also work inplace, with source == dest
t_ears_err ears_buffer_normalize_rms(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, double linear_amp_level, double mix); // also work inplace, with source == dest
t_ears_err ears_buffer_mix(t_object *ob, t_buffer_obj **source, long num_sources, t_buffer_obj *dest, t_llll *gains, long *offset_samps, e_ears_normalization_modes normalization_mode, e_slope_mapping slopemapping);
t_ears_err ears_buffer_mix_from_llll(t_object *ob, t_llll *sources_ll, t_buffer_obj *dest, t_llll *gains, t_llll *offset_samps_ll, e_ears_normalization_modes normalization_mode, e_slope_mapping slopemapping);
t_ears_err ears_buffer_apply_window(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, t_symbol *window_type);

/// Panning operations
t_ears_err ears_buffer_pan1d(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, long num_out_channels, double pan, e_ears_pan_modes pan_mode, e_ears_pan_laws pan_law, double multichannel_pan_aperture, char compensate_gain_for_multichannel_to_avoid_clipping);
t_ears_err ears_buffer_pan1d_envelope(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, long num_out_channels, t_llll *env, e_ears_pan_modes pan_mode, e_ears_pan_laws pan_law, double multichannel_pan_aperture, char compensate_gain_for_multichannel_to_avoid_clipping, e_slope_mapping slopemapping);
t_ears_err ears_buffer_pan1d_buffer(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, long num_out_channels, t_buffer_obj *pan, e_ears_pan_modes pan_mode, e_ears_pan_laws pan_law, double multichannel_pan_aperture, char compensate_gain_for_multichannel_to_avoid_clipping);


// operations: DESTRUCTIVE: buf is modified 
t_ears_err ears_buffer_sum_inplace(t_object *ob, t_buffer_obj *buf, t_buffer_obj *addend);
t_ears_err ears_buffer_multiply_inplace(t_object *ob, t_buffer_obj *buf, t_buffer_obj *factor);

t_ears_err ears_buffer_expr(t_object *ob, t_lexpr *expr,
                            t_hatom *arguments, long num_arguments,
                            t_buffer_obj *dest, e_ears_normalization_modes normalization_mode, char envtimeunit, e_slope_mapping slopemapping);


// Buffers <-> llll or array conversions
t_llll *ears_buffer_to_llll(t_object *ob, t_buffer_obj *buf);
t_ears_err ears_buffer_from_llll(t_object *ob, t_buffer_obj *buf, t_llll *ll, char reformat);
t_atom_long ears_buffer_channel_to_array(t_object *ob, t_buffer_obj *buf, long channel, float **outsamples);
t_atom_long ears_buffer_channel_to_double_array(t_object *ob, t_buffer_obj *buf, long channel, double **outsamples);


// Basic operations
t_ears_err ears_buffer_setempty(t_object *ob, t_buffer_obj *buf, long num_channels);
t_ears_err ears_buffer_copychannel(t_object *ob, t_buffer_obj *source, long source_channel, t_buffer_obj *dest, long dest_channel);
t_ears_err ears_buffer_pack(t_object *ob, long num_sources, t_buffer_obj **source, t_buffer_obj *dest);
t_ears_err ears_buffer_pack_from_llll(t_object *ob, t_llll *sources_ll, t_buffer_obj *dest);
t_ears_err ears_buffer_lace(t_object *ob, t_buffer_obj *left, t_buffer_obj *right, t_buffer_obj *dest);
t_ears_err ears_buffer_slice(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest_left, t_buffer_obj *dest_right, long split_sample);
t_ears_err ears_buffer_split(t_object *ob, t_buffer_obj *source, t_buffer_obj **dest, long *start_samples, long *end_samples, long num_regions);


t_ears_err ears_buffer_get_minmax(t_object *ob, t_buffer_obj *source, double *ampmin, double *ampmax);
t_ears_err ears_buffer_get_maxabs(t_object *ob, t_buffer_obj *source, double *maxabs);
t_ears_err ears_buffer_get_rms(t_object *ob, t_buffer_obj *source, double *rms);;
t_ears_err ears_buffer_extractchannels(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, long num_channels, long *channels);
t_ears_err ears_buffer_extractchannels_from_llll(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, t_llll *channels);

// these both also work inplace with source == dest
t_ears_err ears_buffer_repeat(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, long new_numsamples);
t_ears_err ears_buffer_repeat_times(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, double num_times);
t_ears_err ears_buffer_rev_inplace(t_object *ob, t_buffer_obj *buf);
t_ears_err ears_buffer_rot_inplace(t_object *ob, t_buffer_obj *buf, long shift_in_samps);
t_ears_err ears_buffer_fill_inplace(t_object *ob, t_buffer_obj *buf, float val);

// passing messages through
t_ears_err ears_buffer_send_message(t_object *ob, t_buffer_obj *buf, t_symbol *s, long ac, t_atom *av);

t_ears_err ears_buffer_get_split_points_samps_silence(t_object *ob, t_buffer_obj *buf, double thresh_linear, double min_silence_samps, t_llll **samp_start, t_llll **samp_end, char keep_silence);

t_ears_err ears_buffer_get_split_points_samps_onset(t_object *ob, t_buffer_obj *buf, double attack_thresh_linear, double release_thresh_linear, double min_silence_samps, long lookahead_samps, long smoothingwin_samps, t_llll **samp_start, t_llll **samp_end, char keep_first);

// Filtering
t_ears_err ears_buffer_onepole(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, double cutoff_freq, char highpass); // also works inplace
t_ears_err ears_buffer_biquad(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, double a0, double a1, double a2, double b1, double b2); // also works inplace



// GET properties
t_atom_long ears_buffer_get_size_samps(t_object *ob, t_buffer_obj *buf);
double ears_buffer_get_size_ms(t_object *ob, t_buffer_obj *buf);
t_atom_float ears_buffer_get_sr(t_object *ob, t_buffer_obj *buf);
t_atom_long ears_buffer_get_numchannels(t_object *ob, t_buffer_obj *buf);
t_symbol *ears_buffer_get_sampleformat(t_object *ob, t_buffer_obj *buf);


// SET properties
t_ears_err ears_buffer_set_size(t_object *ob, t_buffer_obj *buf, long num_frames);
t_ears_err ears_buffer_set_sr(t_object *ob, t_buffer_obj *buf, double sr);
t_ears_err ears_buffer_set_numchannels(t_object *ob, t_buffer_obj *buf, long numchannels);
t_ears_err ears_buffer_set_size_and_numchannels(t_object *ob, t_buffer_obj *buf, long num_frames, long numchannels);
t_ears_err ears_buffer_set_sampleformat(t_object *ob, t_buffer_obj *buf, t_symbol *sampleformat);
t_ears_err ears_buffer_copy_format(t_object *ob, t_buffer_obj *orig, t_buffer_obj *dest);
t_ears_err ears_buffer_crop_ms_inplace(t_object *ob, t_buffer_obj *buf, double ms_start, long ms_end);

// CONVERSIONS
t_ears_err ears_buffer_convert_numchannels(t_object *ob, t_buffer_obj *buf, long numchannels, e_ears_channel_convert_modes channelmode_upmix, e_ears_channel_convert_modes channelmode_downmix);
t_ears_err ears_buffer_convert_sr(t_object *ob, t_buffer_obj *buf, double sr);
t_ears_err ears_buffer_convert_size(t_object *ob, t_buffer_obj *buf, long sizeinsamps);
t_ears_err ears_buffer_convert_format(t_object *ob, t_buffer_obj *orig, t_buffer_obj *dest, e_ears_channel_convert_modes channelmode_upmix, e_ears_channel_convert_modes channelmode_downmix);
t_ears_err ears_buffer_resample(t_object *ob, t_buffer_obj *buf, double resampling_factor, long window_width);
t_ears_err ears_buffer_resample_envelope(t_object *ob, t_buffer_obj *buf, t_llll *resampling_factor, long window_width, e_slope_mapping slopemapping);


/// WRITE FILES
void ears_write_buffer(t_object *buf, t_symbol *filename, t_object *culprit, t_ears_encoding_settings *settings);
void ears_writeaiff(t_object *buf, t_symbol *filename);
void ears_writeflac(t_object *buf, t_symbol *filename);
void ears_writewave(t_object *buf, t_symbol *filename);
void ears_writeraw(t_object *buf, t_symbol *filename);
t_symbol *get_conformed_resolved_path(t_symbol *filename);




/// Helper tools
t_ears_envelope_iterator ears_envelope_iterator_create(t_llll *envelope, double default_val, char use_decibels, e_slope_mapping slopemapping);
t_ears_envelope_iterator ears_envelope_iterator_create_from_llllelem(t_llllelem *envelope, double fallback_val, char use_decibels, e_slope_mapping slopemapping); // also accounts for static numbers
double ears_envelope_iterator_walk_interp(t_ears_envelope_iterator *eei, long sample_num, long tot_num_samples);
void ears_envelope_get_max_x(t_llllelem *el, t_atom *a_max);
void ears_envelope_iterator_reset(t_ears_envelope_iterator *eei);
double ears_envelope_iterator_get_min_y(t_ears_envelope_iterator *eei);
double ears_envelope_iterator_get_max_y(t_ears_envelope_iterator *eei);




// These two functions are to be uses with caution, they do not create a buffer reference, only a buffer object, to be used and then freed:
t_ears_err ears_buffer_from_file(t_object *ob, t_buffer_obj **dest, t_symbol *file, double start_ms, double end_ms, double sr, long buffer_idx);
t_ears_err ears_buffer_synth_from_duration_line(t_object *e_ob, t_buffer_obj **dest,
                                                double midicents, double duration_ms, double velocity, t_llll *breakpoints,
                                                e_ears_veltoamp_modes veltoamp_mode, double amp_vel_min, double amp_vel_max,
                                                double middleAtuning, double sr, long buffer_idx, e_slope_mapping slopemapping);



#endif // _EARS_BUF_COMMONS_H_
