/**
	@file
	ears.freeverb_commons.h
	Freeverb bridge for ears
 
	by Daniele Ghisi
 */

#ifndef _EARS_BUF_FREEVERB_COMMONS_H_
#define _EARS_BUF_FREEVERB_COMMONS_H_

#include "revmodel.hpp"
#include "ears.commons.h"

t_ears_err ears_buffer_freeverb(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, revmodel *model, long tail_samps);

#endif // _EARS_BUF_FREEVERB_COMMONS_H_
