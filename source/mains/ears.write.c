/**
	@file
	ears.write.c
 
	@name
	ears.write~
 
	@realname
	ears.write~
 
    @hiddenalias
    ears.tofile

	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	Save buffers to files
 
	@description
	Creates a sound file for each incoming buffer name
 
	@discussion
 
	@category
	ears buffer operations
 
	@keywords
	buffer, file, write, save, export
 
	@seealso
	ears.read~
	
	@owner
	Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "llllobj.h"
#include "llll_commons_ext.h"
#include "bach_math_utilities.h"
#include "ears.object.h"


typedef struct _buf_write {
    t_earsbufobj       e_ob;
    
    t_llll             *filenames;
    char                native_mp3_handling;
} t_buf_write;



// Prototypes
t_buf_write*         buf_write_new(t_symbol *s, short argc, t_atom *argv);
void			buf_write_free(t_buf_write *x);
void			buf_write_bang(t_buf_write *x);
void			buf_write_anything(t_buf_write *x, t_symbol *msg, long ac, t_atom *av);
void            buf_write_append(t_buf_write *x, t_symbol *msg, long ac, t_atom *av);
void            buf_write_append_deferred(t_buf_write *x, t_symbol *msg, long ac, t_atom *av);

void buf_write_assist(t_buf_write *x, void *b, long m, long a, char *s);
void buf_write_inletinfo(t_buf_write *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(write)

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
    
    CLASS_NEW_CHECK_SIZE(c, "ears.write~",
                         (method)buf_write_new,
                         (method)buf_write_free,
                         sizeof(t_buf_write),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    
    // @method list/llll @digest Save buffers
    // @description A list or llll in the first inlet is expected to be a list of buffer names;
    // a list or llll in the second inlet is expected to be a list of file names.
    // When the list or llll in the leftmost inlet is received, each of the buffers is saved on disk on the
    // corresponding file name
    EARSBUFOBJ_DECLARE_COMMON_METHODS_DEFER(write)

    
//    earsbufobj_class_add_outname_attr(c);
//    earsbufobj_class_add_timeunit_attr(c);
//    earsbufobj_class_add_naming_attr(c);

    CLASS_ATTR_LONG(c, "nativemp3",	0,	t_buf_write, native_mp3_handling);
    CLASS_ATTR_STYLE_LABEL(c, "nativemp3", 0, "onoff", "Native MP3 Handling");
    // @description Toggles native MP3 handling.

    
    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
    return 0;
}



void buf_write_assist(t_buf_write *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        if (a == 0)
            sprintf(s, "symbol/list/llll: Buffer names"); // @in 0 @type symbol/list/llll @digest Buffer names
        else
            sprintf(s, "symbol/list/llll: Soundfile names"); // @in 1 @type symbol/list/llll @digest Soundfile names
    } else {
        sprintf(s, "bang When Done"); // @out 0 @type bang @digest bang When Export is Done
    }
}

void buf_write_inletinfo(t_buf_write *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_write *buf_write_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_write *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_write*)object_alloc_debug(s_tag_class);
    if (x) {
        x->native_mp3_handling = 1;
        
        // @arg 0 @name filenames @optional 1 @type symbol
        // @digest Output file names
        // @description Sets the output file names for the incoming buffers
        
        t_llll *args = llll_parse(true_ac, argv);
        if (args)
            x->filenames = llll_clone(args);
        else
            x->filenames = llll_make();

        earsbufobj_init((t_earsbufobj *)x, 0);

        attr_args_process(x, argc, argv);
        
        earsbufobj_setup((t_earsbufobj *)x, "E4", "b", NULL);

        llll_free(args);
    }
    return x;
}


void buf_write_free(t_buf_write *x)
{
#ifdef EARS_FROMFILE_NATIVE_MP3_HANDLING
    //    mpg123_exit(); // can't call this here, should call it on Max quit... TO DO!
#endif
    llll_free(x->filenames);
    earsbufobj_free((t_earsbufobj *)x);
}

t_symbol *increment_symbol(t_symbol *base, long index)
{
    char outfilename[PATH_MAX];
    const char *dot = strrchr(base->s_name, '.');
    if (!dot || dot == base->s_name) {
        snprintf_zero(outfilename, PATH_MAX, "%s.%d", base->s_name, index);
        return gensym(outfilename);
    } else {
        long len = dot - base->s_name;
        strncpy(outfilename, base->s_name, len);
        snprintf_zero(outfilename + len, PATH_MAX, ".%d.%s", index, dot+1);
        return gensym(outfilename);
    }
}

void buf_write_bang(t_buf_write *x)
{
    long num_buffers = earsbufobj_get_num_inlet_stored_buffers((t_earsbufobj *)x, 0, false);
    
    if (!x->filenames->l_head) {
        object_error((t_object *)x, "No filenames inserted. Please input the soundfile names via the right inlet.");
        return;
    }

    if (x->filenames->l_depth > 1) {
        object_warn((t_object *)x, "Filenames llll is not flat; will be flattened.");
        llll_flatten(x->filenames, 0, 0);
    }

    t_llll *names = llll_clone(x->filenames);
    // converting all names to symbols
    for (t_llllelem *el = names->l_head; el; el = el->l_next) {
        if (hatom_gettype(&el->l_hatom) != H_SYM)
            hatom_setsym(&el->l_hatom, hatom_to_symbol(&el->l_hatom));
    }
    
    if (names->l_size < num_buffers) {
        // less names than buffers inserted: will add an incrementing index
        long count = 1;
        t_symbol *lastname = hatom_getsym(&names->l_tail->l_hatom);
        hatom_setsym(&names->l_tail->l_hatom, increment_symbol(lastname, count++));
        while (names->l_size < num_buffers)
            llll_appendsym(names, increment_symbol(lastname, count++));
    }
    
    t_llllelem *el; long i;
    for (i = 0, el = names->l_head; i < num_buffers && el; i++, el = el->l_next) {
        t_object *buf = earsbufobj_get_stored_buffer_obj((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, i);
        if (buf) {
            t_symbol *filename = hatom_getsym(&el->l_hatom);
            ears_write_buffer(buf, filename, (t_object *)x);
        } else {
            object_error((t_object *)x, EARS_ERROR_BUF_NO_BUFFER);
        }
    }
    
    llll_free(names);
    earsbufobj_outlet_bang((t_earsbufobj *)x, 0);
}


void buf_write_anything(t_buf_write *x, t_symbol *msg, long ac, t_atom *av)
{
    long inlet = proxy_getinlet((t_object *) x);

    t_llll *parsed = earsbufobj_parse_gimme((t_earsbufobj *) x, LLLL_OBJ_VANILLA, msg, ac, av);
    if (parsed && parsed->l_head) {
        if (inlet == 0) {
            earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, parsed->l_size, true);
            earsbufobj_store_buffer_list((t_earsbufobj *)x, parsed, 0, true);
            
            buf_write_bang(x);
            
        } else if (inlet == 1) {
            earsbufobj_mutex_lock((t_earsbufobj *)x);
            llll_free(x->filenames);
            x->filenames = llll_clone(parsed);
            earsbufobj_mutex_unlock((t_earsbufobj *)x);
        }
    }
    llll_free(parsed);
}
