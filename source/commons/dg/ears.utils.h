/**
	@file
	ears.utils.h
	Utilities header for the ears library
 
	by Daniele Ghisi
 */

#ifndef _EARS_UTILS_H_
#define _EARS_UTILS_H_

#include "ext.h"

// locate an existing file
t_symbol *ears_ezlocate_file(t_symbol *file_name, t_fourcc *file_type);

// resolve full path of an input file (whether it exists or not)
t_symbol *ears_ezresolve_file(t_symbol *file_name, bool force_extension, const char *ext);

void ears_symbol_split_unit(t_symbol *s, double *val, t_symbol **unit);
char ears_symbol_ends_with(t_symbol *filename, const char *pattern, char ignore_case);
char ears_file_exists(const char *filename, const short path);
long ears_saveasdialog(t_object *e_ob, const char *default_filename, t_fourcc *types, long numtypes, t_fourcc *outtype, t_symbol **filepath_sym, bool force_extension); // forced extension is inferred from default_filename

long ears_buffer_symbol_is_buffer(t_symbol *s);
t_object *ears_buffer_getobject(t_symbol *name);

long ears_polybuffer_symbol_is_polybuffer(t_symbol *s);
t_object *ears_polybuffer_getobject(t_symbol *name);


#endif // _EARS_CONVERSIONS_H_
