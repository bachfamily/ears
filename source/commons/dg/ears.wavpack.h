/**
	@file
	ears.wavpack.h
	Common utilities header for wavpack handling
 
	by Daniele Ghisi
 */

#ifndef _EARS_BUF_WAVPACK_H_
#define _EARS_BUF_WAVPACK_H_

#include "ears.commons.h"
#include "wavpack.h"

long ears_buffer_read_handle_wavpack(t_object *ob, char *filename, long start_sample, long end_sample, t_buffer_obj *buf);
void ears_writewavpack(t_object *buf, t_symbol *filename, t_ears_encoding_settings *settings);


#endif // _EARS_BUF_WAVPACK_H_