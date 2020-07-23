/**
	@file
	ears.scores.h
	Utilities to convert scores into buffers
 
	by Daniele Ghisi
 */

#ifndef _EARS_BUF_SCORES_H_
#define _EARS_BUF_SCORES_H_

#include "ears.commons.h"


t_ears_err ears_roll_to_buffer(t_earsbufobj *e_ob, t_llll *roll_gs, t_buffer_obj *dest,
                               char use_mute_solos, char use_durations,
                               long num_channels,
                               long filename_slot, long offset_slot, long gain_slot, long pan_slot,
                               double sr, e_ears_normalization_modes normalization_mode, e_ears_channel_convert_modes convertchannelsmode,
                               double fadein_amount, double fadeout_amount, e_ears_fade_types fade_in_type, e_ears_fade_types fade_out_type,
                               double fade_in_curve, double fade_out_curve,
                               e_ears_pan_modes pan_mode, e_ears_pan_laws pan_law,
                               double multichannel_pan_aperture, char compensate_gain_for_multichannel_to_avoid_clipping,
                               e_ears_veltoamp_modes veltoamp_mode, double amp_vel_min, double amp_vel_max);

#endif // _EARS_BUF_SCORES_H_
