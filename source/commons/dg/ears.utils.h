/**
	@file
	ears.utils.h
	Utilities header for the ears library
 
	by Daniele Ghisi
 */

#ifndef _EARS_UTILS_H_
#define _EARS_UTILS_H_

#include "ext.h"

t_symbol *ears_ezlocate_file(t_symbol *file_name, t_fourcc *file_type);
char ears_filename_ends_with(t_symbol *filename, const char *pattern, char ignore_case);
char ears_file_exists(const char *filename, const short path);

long ears_buffer_symbol_is_buffer(t_symbol *s);
t_object *ears_buffer_getobject(t_symbol *name);

long ears_polybuffer_symbol_is_polybuffer(t_symbol *s);
t_object *ears_polybuffer_getobject(t_symbol *name);

#endif // _EARS_CONVERSIONS_H_
