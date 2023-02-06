/**
	@file
	ears.soundtouch_commons.h
	Soundtouch bridge for ears
 
	by Daniele Ghisi
 */

#ifndef _EARS_BUF_SOUNDTOUCH_COMMONS_H_
#define _EARS_BUF_SOUNDTOUCH_COMMONS_H_

#include "ears.commons.h"
#include "SoundTouch.h"

t_ears_err ears_buffer_soundtouch(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, double stretch_factor, double pitchshift_semitones, int quickSeek, int noAntiAlias, bool speech);

#endif // _EARS_BUF_SOUNDTOUCH_COMMONS_H_
