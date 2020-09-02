/**
	@file
	ears.read.c
 
	@name
	ears.read~
 
	@realname
	ears.read~
 
	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	Store files in buffers
 
	@description
	Creates a buffer for each incoming file
 
	@discussion
 
	@category
	ears buffer operations
 
	@keywords
	buffer, file, open, read, replace, import, load
 
	@seealso
	ears.write~
	
	@owner
	Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "llllobj.h"
#include "llll_commons_ext.h"
#include "bach_math_utilities.h"
#include "ears.object.h"


typedef struct _buf_read {
    t_earsbufobj       e_ob;
    
    char                native_mp3_handling;
    
    t_llll              *filenames;
} t_buf_read;



// Prototypes
t_buf_read*         buf_read_new(t_symbol *s, short argc, t_atom *argv);
void			buf_read_free(t_buf_read *x);
void			buf_read_bang(t_buf_read *x);
void			buf_read_anything(t_buf_read *x, t_symbol *msg, long ac, t_atom *av);
void            buf_read_append(t_buf_read *x, t_symbol *msg, long ac, t_atom *av);
void            buf_read_append_deferred(t_buf_read *x, t_symbol *msg, long ac, t_atom *av);
long            buf_read_acceptsdrag(t_buf_read *x, t_object *drag, t_object *view);

void buf_read_assist(t_buf_read *x, void *b, long m, long a, char *s);
void buf_read_inletinfo(t_buf_read *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(read)

/**********************************************************************/
// Class Definition and Life Cycle

int C74_EXPORT main(void)
{
    common_symbols_init();
    llllobj_common_symbols_init();
    
    if (llllobj_check_version(bach_get_current_llll_version()) || llllobj_test()) {
        ears_error_bachcheck();
        return 1;
    }
    
    t_class *c;
    
    CLASS_NEW_CHECK_SIZE(c, "ears.read~",
                         (method)buf_read_new,
                         (method)buf_read_free,
                         sizeof(t_buf_read),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    
    // @method list/llll @digest Import files
    // @description A list or llll of filenames will trigger the import of such files as buffers, and the whole set of output buffer names will be output.
    // If the "append" message attribute is set to 1, then the imported buffers are appended to the existing ones.
    // @mattr append @type int @default 0 @digest Append buffers
    EARSBUFOBJ_DECLARE_COMMON_METHODS_DEFER(read)

    // @method append @digest Import files as additional buffers
    // @description The <m>append</m> message, followed by a list of filenames, will import the files as buffers
    // appending them to the existing one, and then outputting the whole set of output buffer names.
    class_addmethod(c, (method)buf_read_append_deferred,             "append",            A_GIMME, 0);

    // @method (drag) @digest Drag-and-drop file loading
    // @description The specified file(s) are read from disk and converted into buffers.
    class_addmethod(c, (method)buf_read_acceptsdrag, "acceptsdrag_locked", A_CANT, 0);
    class_addmethod(c, (method)buf_read_acceptsdrag, "acceptsdrag_unlocked", A_CANT, 0);
    
    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_timeunit_attr(c);
    earsbufobj_class_add_naming_attr(c);

    CLASS_ATTR_LONG(c, "nativemp3",	0,	t_buf_read, native_mp3_handling);
    CLASS_ATTR_STYLE_LABEL(c, "nativemp3", 0, "onoff", "Native MP3 Handling");
    // @description Toggles native MP3 handling.

    
    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
    return 0;
}

long buf_read_acceptsdrag(t_buf_read *x, t_object *drag, t_object *view)
{
    if (jdrag_matchdragrole(drag, gensym("audiofile"), 0))  {
        jdrag_object_add(drag, (t_object *)x, gensym("append"));
        return true;
    }
    return false;
}

void buf_read_assist(t_buf_read *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        sprintf(s, "symbol/list/llll: Filenames"); // @in 0 @type symbol/list/llll @digest Filename
    } else {
        if (a == 0)
            sprintf(s, "symbol/list: Buffer Names"); // @out 0 @type symbol/list @digest Buffer names
        else
            sprintf(s, "symbol/list: Soundfile Names"); // @out 1 @type symbol/list @digest Soundfile names
    }
}

void buf_read_inletinfo(t_buf_read *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_read *buf_read_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_read *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_read*)object_alloc_debug(s_tag_class);
    if (x) {
        x->native_mp3_handling = 1;
        x->filenames = llll_make();
        
        earsbufobj_init((t_earsbufobj *)x, 0);
        
        // @arg 0 @name outnames @optional 1 @type symbol
        // @digest Output buffer names
        // @description @copy EARS_DOC_OUTNAME_ATTR
        
        t_llll *args = llll_parse(true_ac, argv);
        t_llll *names = earsbufobj_extract_names_from_args((t_earsbufobj *)x, args);

        attr_args_process(x, argc, argv);
        
        earsbufobj_setup((t_earsbufobj *)x, "4", "E4", names);

        llll_free(args);
        llll_free(names);
    }
    return x;
}


void buf_read_free(t_buf_read *x)
{
#ifdef EARS_FROMFILE_NATIVE_MP3_HANDLING
    //    mpg123_exit(); // can't call this here, should call it on Max quit... TO DO!
#endif
    
    llll_free(x->filenames);
    earsbufobj_free((t_earsbufobj *)x);
}



void buf_read_bang(t_buf_read *x)
{
    earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
}


// position is 0-based
void buf_read_addpathsym(t_buf_read *x, t_symbol *path, long position)
{
    while (x->filenames->l_size <= position)
        llll_appendsym(x->filenames, _llllobj_sym_none);
    
    t_llllelem *el = llll_getindex(x->filenames, position + 1, I_STANDARD);
    if (el)
        hatom_setsym(&el->l_hatom, path);
}

void buf_read_load(t_buf_read *x, t_llll *files, char append)
{
    if (files && files->l_head) {
        long count = 0;
        long num_stored_buffers = earsbufobj_get_num_outlet_stored_buffers((t_earsbufobj *)x, 0, append);
        long offset = append ? num_stored_buffers : 0;
        t_llllelem *elem;
        
        earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
        
        earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_OUT, 0, files->l_size + offset, true);
        for (elem = files->l_head; elem; elem = elem->l_next, count++) {
            t_symbol *filepath = NULL;
            double start = -1, end = -1;
            
            if (hatom_gettype(&elem->l_hatom) == H_SYM)
                filepath = ears_ezlocate_file(hatom_getsym(&elem->l_hatom), NULL);
            else if (hatom_gettype(&elem->l_hatom) == H_LLLL) {
                t_llll *ll = hatom_getllll(&elem->l_hatom);
                if (ll->l_size >= 3 && hatom_gettype(&ll->l_head->l_hatom) == H_SYM && is_hatom_number(&ll->l_head->l_next->l_hatom) && is_hatom_number(&ll->l_head->l_next->l_next->l_hatom)) {
                    filepath = ears_ezlocate_file(hatom_getsym(&ll->l_head->l_hatom), NULL);
                    start = hatom_getdouble(&ll->l_head->l_next->l_hatom);
                    end = hatom_getdouble(&ll->l_head->l_next->l_next->l_hatom);
                }
            }
            
            if (filepath) {
                
                buf_read_addpathsym(x, filepath, count+offset);
                
#ifdef EARS_FROMFILE_NATIVE_MP3_HANDLING
                if (x->native_mp3_handling && ears_filename_ends_with(filepath, ".mp3", true)) {
                    long startsamp = start >= 0 ? earsbufobj_input_to_samps((t_earsbufobj *)x, start,                                                                           earsbufobj_get_stored_buffer_obj((t_earsbufobj *)x, EARSBUFOBJ_OUT, 0, count + offset)) : -1;
                    long endsamp = end >= 0 ? earsbufobj_input_to_samps((t_earsbufobj *)x, end, earsbufobj_get_stored_buffer_obj((t_earsbufobj *)x, EARSBUFOBJ_OUT, 0, count + offset)) : -1;
                    ears_buffer_read_handle_mp3((t_object *)x, filepath->s_name, startsamp, endsamp, earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, count + offset));
                } else {
#endif
                    // trying to load file into input buffer
                    earsbufobj_importreplace_buffer((t_earsbufobj *)x, EARSBUFOBJ_OUT, 0, count + offset, filepath);
                    
                    if (start > 0 || end > 0) {
                        double start_samps = start < 0 ? -1 : earsbufobj_input_to_samps((t_earsbufobj *)x, start, earsbufobj_get_stored_buffer_obj((t_earsbufobj *)x, EARSBUFOBJ_OUT, 0, count + offset));
                        double end_samps = end < 0 ? -1 : earsbufobj_input_to_samps((t_earsbufobj *)x, end, earsbufobj_get_stored_buffer_obj((t_earsbufobj *)x, EARSBUFOBJ_OUT, 0, count + offset));
                        ears_buffer_crop_inplace((t_object *)x, earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, count + offset), start_samps, end_samps);
                    }
                    
#ifdef EARS_FROMFILE_NATIVE_MP3_HANDLING
                }
#endif
                
            } else {
                // empty buffer will do.
                char *txtbuf = NULL;
                hatom_to_text_buf(&elem->l_hatom, &txtbuf);
                object_warn((t_object *)x, "Error while importing file %s; empty buffer created.", txtbuf);
                buf_read_addpathsym(x, _llllobj_sym_none, count+offset);
                earsbufobj_store_empty_buffer((t_earsbufobj *)x, EARSBUFOBJ_OUT, 0, count + offset);
                bach_freeptr(txtbuf);
            }
            
        }
        if (count > 0) {
            long numsyms = x->filenames->l_size;
            t_symbol **syms = (t_symbol **)bach_newptr(numsyms * sizeof(t_symbol *));
            t_llllelem *el; long i;
            for (i = 0, el = x->filenames->l_head; i < numsyms && el; i++, el = el->l_next)
                syms[i] = (hatom_gettype(&el->l_hatom) == H_SYM) ? hatom_getsym(&el->l_hatom) : _llllobj_sym_none;
            earsbufobj_outlet_symbol_list((t_earsbufobj *)x, 1, numsyms, syms);
            bach_freeptr(syms);
            
            earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
        }
        
    } else if (files) {
        // null llll
        if (!append) {
            buf_read_addpathsym(x, _llllobj_sym_none, 0);
            earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_OUT, 0, 1, true);
            earsbufobj_store_empty_buffer((t_earsbufobj *)x, EARSBUFOBJ_OUT, 0, 0);
        }
    }
}


void buf_read_append_deferred(t_buf_read *x, t_symbol *msg, long ac, t_atom *av)
{
    defer(x, (method)buf_read_append, msg, ac, av);
}

void buf_read_append(t_buf_read *x, t_symbol *msg, long ac, t_atom *av)
{
    t_llll *parsed = earsbufobj_parse_gimme((t_earsbufobj *) x, LLLL_OBJ_VANILLA, NULL, ac, av);
    if (!parsed) return;
    buf_read_load(x, parsed, true);
    llll_free(parsed);
}


void buf_read_anything(t_buf_read *x, t_symbol *msg, long ac, t_atom *av)
{
    long inlet = proxy_getinlet((t_object *) x);
    long append = false;
    
    t_llll *parsed = earsbufobj_parse_gimme((t_earsbufobj *) x, LLLL_OBJ_VANILLA, msg, ac, av);
    if (!parsed) return;

    llll_parseargs_and_attrs((t_object *) x, parsed, "i", _sym_append, &append);

    if (inlet == 0)
        buf_read_load(x, parsed, append);

    llll_free(parsed);
}
