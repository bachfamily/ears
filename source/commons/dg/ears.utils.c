#include "ears.utils.h"
#include "llll_files.h"


char ears_filename_ends_with_char(const char *filename, const char *pattern, char ignore_case)
{
    if (filename && strlen(filename)>=strlen(pattern) &&
        ((ignore_case && strcasecmp(filename + strlen(filename) - strlen(pattern), pattern) == 0) ||
         (!ignore_case && strcmp(filename + strlen(filename) - strlen(pattern), pattern) == 0)))
        return 1;
    return 0;
}


char ears_filename_ends_with(t_symbol *filename, const char *pattern, char ignore_case)
{
    if (filename && filename->s_name && strlen(filename->s_name)>=strlen(pattern) &&
        ((ignore_case && strcasecmp(filename->s_name + strlen(filename->s_name) - strlen(pattern), pattern) == 0) ||
         (!ignore_case && strcmp(filename->s_name + strlen(filename->s_name) - strlen(pattern), pattern) == 0)))
        return 1;
    return 0;
}

void ears_ezlocate_file_char(const char *filename_in, char *filename_out, t_fourcc *file_type)
{
    char filename[MAX_FILENAME_CHARS];
    short path = 0;
    
    if (!filename_in)
        return;
    
    if (file_type) *file_type = 0;
    
    if (path_frompathname(filename_in, &path, filename)) {
        t_fourcc type;
        char file_path_str[MAX_FILENAME_CHARS];
        strncpy_zero(file_path_str, filename_in, MAX_FILENAME_CHARS);
        if (!locatefile_extended(file_path_str, &path, &type, &type, -1))  {
            path_topathname(path, file_path_str, filename);
            path_nameconform(filename, filename_out, PATH_STYLE_MAX, PATH_TYPE_BOOT);
            if (file_type) *file_type = type;
        }
    } else {
        char filenameok[MAX_FILENAME_CHARS];
        path_topathname(path, filename, filenameok);
        path_nameconform(filenameok, filename_out, PATH_STYLE_MAX, PATH_TYPE_BOOT);
    }
}

t_symbol *ears_ezlocate_file(t_symbol *file_name, t_fourcc *file_type)
{
    char filename[MAX_FILENAME_CHARS];
    short path = 0;
    
    if (!file_name)
        return NULL;
    
    if (file_type) *file_type = 0;
    
    if (path_frompathname(file_name->s_name, &path, filename)) {
        t_fourcc type;
        char file_path_str[MAX_FILENAME_CHARS];
        strncpy_zero(file_path_str, file_name->s_name, MAX_FILENAME_CHARS);
        if (!locatefile_extended(file_path_str, &path, &type, &type, -1))  {
            char filenameok2[MAX_FILENAME_CHARS];
            path_topathname(path, file_path_str, filename);
            path_nameconform(filename, filenameok2, PATH_STYLE_MAX, PATH_TYPE_BOOT);
            if (file_type) *file_type = type;
            return gensym(filenameok2);
        }
    } else {
        char filenameok[MAX_FILENAME_CHARS];
        char filenameok2[MAX_FILENAME_CHARS];
        path_topathname(path, filename, filenameok);
        path_nameconform(filenameok, filenameok2, PATH_STYLE_MAX, PATH_TYPE_BOOT);
        return gensym(filenameok2);
    }
    
    return NULL;
}


t_symbol *ears_ezresolve_file(t_symbol *file_name, bool force_extension, const char *ext)
{
    char filename[MAX_FILENAME_CHARS];
    char outfilepath[MAX_FILENAME_CHARS];
    char outfilepath_conform[MAX_FILENAME_CHARS];
    short path = 0;
    
    if (!file_name)
        return NULL;
    
    if (path_frompotentialpathname(file_name->s_name, &path, filename)) {
        path = path_getdefault();
        path_topotentialname(path, file_name->s_name, outfilepath, false);
    } else {
        path_topotentialname(path, filename, outfilepath, false);
    }

    if (force_extension) {
        if (!strchr(outfilepath, '.') || !ears_filename_ends_with_char(outfilepath, ext, true)) {
            strncat_zero(outfilepath, ext, MAX_PATH_CHARS);
        }
    }

    path_nameconform(outfilepath, outfilepath_conform, PATH_STYLE_MAX, PATH_TYPE_BOOT);
    return gensym(outfilepath_conform);

    return NULL;
}




char ears_file_exists(const char *filename, const short path)
{
    t_fileinfo info;
    if (path_fileinfo(filename, path, &info) == 0)
        return true;
    else
        return false;
}




long ears_saveasdialog(t_object *e_ob, const char *default_filename, t_fourcc *types, long numtypes, t_fourcc *outtype, t_symbol **filepath_sym, bool force_extension)
{
    char filename[MAX_PATH_CHARS];
    char filepath[MAX_PATH_CHARS];
    short path = 0;
    strncpy_zero(filename, default_filename, MAX_PATH_CHARS);
    if (saveasdialog_extended(filename, &path, outtype, types, numtypes)) {    // non-zero: user cancelled
        return FILE_ERR_CANCELED;
    }
    
    // check
    if (force_extension) {
        const char *ext = strrchr(default_filename, '.');
        if (!strchr(filename, '.') || !ears_filename_ends_with_char(filename, ext, true)) {
            strncat_zero(filename, ext, MAX_PATH_CHARS);
        }
    }
    
    if (path_topotentialname(path, filename, filepath, false)) {
        object_error((t_object *)e_ob, "Can't open file.");
        return FILE_ERR_CANTOPEN;
    }
    *filepath_sym = gensym(filepath);
    return FILE_ERR_NONE;
}





/// POLYBUFFERS
long ears_polybuffer_symbol_is_polybuffer(t_symbol *s)
{
    if (s && s->s_thing && !NOGOOD(s->s_thing) && ob_sym(s->s_thing) == gensym("polybuffer"))
        return (1);
    else
        return (0);
}

t_object *ears_polybuffer_getobject(t_symbol *name)
{
    t_object *rtn = NULL;
    
    if (ears_polybuffer_symbol_is_polybuffer(name)) {
        rtn = (t_object *)(name->s_thing);
    }
    
    return (rtn);
}

long ears_buffer_symbol_is_buffer(t_symbol *s)
{
    if (s && s->s_thing && !NOGOOD(s->s_thing) && ob_sym(s->s_thing) == gensym("buffer~"))
        return (1);
    else
        return (0);
}

t_object *ears_buffer_getobject(t_symbol *name)
{
    t_object *rtn = NULL;
    
    if (ears_buffer_symbol_is_buffer(name)) {
        rtn = (t_object *)(name->s_thing);
    }
    
    return (rtn);
}

