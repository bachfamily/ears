/**
	@file
	ears.mp3.h
	Common utilities for mp3 handling
 
	by Daniele Ghisi
 */

#ifndef _EARS_BUF_MP3_H_
#define _EARS_BUF_MP3_H_

#include "ears.commons.h"
#include <lame/lame.h> // only used to export mp3s

long ears_buffer_read_handle_mp3(t_object *ob, char *filename, double start, double end, t_buffer_obj *buf, e_ears_timeunit timeunit);

void ears_writemp3(t_object *buf, t_symbol *filename, t_ears_encoding_settings *settings);


#endif // _EARS_BUF_MP3_H_
