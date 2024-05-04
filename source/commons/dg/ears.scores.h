/**
	@file
	ears.scores.h
	Utilities to convert scores into buffers
 
	by Daniele Ghisi
 */

#ifndef _EARS_BUF_SCORES_H_
#define _EARS_BUF_SCORES_H_

#include "ears.commons.h"


typedef enum _ears_scoretobuf_mode
{
    EARS_SCORETOBUF_MODE_NONE = 0,
    EARS_SCORETOBUF_MODE_SYNTHESIS = 1,
    EARS_SCORETOBUF_MODE_SAMPLING = 2,
} e_ears_scoretobuf_mode;



typedef struct _ears_note_buffer
{
    t_buffer_obj    *buffer;
    
    t_symbol    *filename;
    double      rate;
    double      start_ms;
    double      end_ms;
    t_llll      *breakpoints;
    t_llll      *gain_env;
    t_llll      *pan_env;
    double      voice_pan;
    
    double      fadein_amount;
    double      fadeout_amount;
    
    double      transp_mc;
    double      stretch_factor;
} t_ears_note_buffer;



t_ears_err ears_roll_to_buffer(t_earsbufobj *e_ob, e_ears_scoretobuf_mode mode, t_llll *roll_gs, t_buffer_obj *dest,
                               e_ears_synthmode synthmode, float *wavetable, long wavetable_length,
                               char use_mute_solos, char use_durations,
                               long num_channels,
                               long filename_slot, long offset_slot, long gain_slot, long pan_slot, long rate_slot,
                               // these two below are currently unused:
                               long ps_slot, long ts_slot,
                               double sr, e_ears_normalization_modes normalization_mode, e_ears_channel_convert_modes convertchannelsmode,
                               double fadein_amount, double fadeout_amount,
                               e_ears_fade_types fade_in_type, e_ears_fade_types fade_out_type,
                               double fade_in_curve, double fade_out_curve,
                               t_llll *voice_pan, e_ears_pan_modes pan_mode, e_ears_pan_laws pan_law,
                               double multichannel_pan_aperture, char compensate_gain_for_multichannel_to_avoid_clipping,
                               e_ears_veltoamp_modes veltoamp_mode, double amp_vel_min, double amp_vel_max,
                               double middleAtuning, long oversampling, long resamplingfiltersize,
                               bool optimize_for_identical_samples, bool use_assembly_line, bool each_voice_has_own_channels);


t_ears_err ears_roll_to_reaper(t_earsbufobj *e_ob, t_symbol *filename_sym, t_symbol *reaper_header,
                               e_ears_scoretobuf_mode mode, t_llll *roll_gs,
                               char use_durations, char pitch_is_transposition, long base_midicents,
                               long filename_slot, long offset_slot, long gain_slot, long pan_slot,
                               long transp_slot, long timestretch_slot, long fade_slot, long color_slot,
                               double default_fadein_amount, double default_fadeout_amount,
                               e_ears_fade_types fade_in_type, e_ears_fade_types fade_out_type,
                               double fade_in_curve, double fade_out_curve,
                               e_ears_veltoamp_modes veltoamp_mode, double amp_vel_min, double amp_vel_max,
                               t_llll *number_of_channels_per_voice, char auto_xfade,
                               char copy_media, t_symbol *media_folder_name, t_symbol *buffer_format, t_symbol *buffer_filetype);

#endif // _EARS_BUF_SCORES_H_
