/**
	@file
	ears.write.c
 
	@name
	ears.write~
 
	@realname
	ears.write~
 
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
	ears basic
 
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


#include <tlist.h>
#include <fileref.h>
#include <tfile.h>
#include <tag.h>
#include <tpropertymap.h>


typedef struct _buf_write {
    t_earsbufobj       e_ob;
    
    t_llll             *filenames;
    
    t_symbol           *sampleformat;
    
    t_symbol           *mp3_vbrmode;
    long               mp3_bitrate;
    long               mp3_bitrate_min;
    long               mp3_bitrate_max;
    
    char               use_correction_file;
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

t_max_err buf_write_setattr_format(t_buf_write *x, void *attr, long argc, t_atom *argv)
{
    t_max_err err = MAX_ERR_NONE;
    if (argc && argv) {
        if (atom_gettype(argv) == A_LONG) {
            long bitdepth = atom_getlong(argv);
            if (bitdepth == 8)
                x->sampleformat = _sym_int8;
            else if (bitdepth == 16)
                x->sampleformat = _sym_int16;
            else if (bitdepth == 24)
                x->sampleformat = _sym_int24;
            else  if (bitdepth == 32)
                x->sampleformat = _sym_int32;
            else {
                object_error((t_object *)x, "Unknown sample format.");
                err = MAX_ERR_GENERIC;
            }
        } else if (atom_gettype(argv) == A_SYM) {
            t_symbol *s = atom_getsym(argv);
            if (s == _sym_int8 || s == _sym_int16 || s == _sym_int24 || s == _sym_int32 || s == _sym_float32 || s == _sym_float64 || s == _sym_mulaw || s == gensym("alaw"))
                x->sampleformat = s;
            else {
                object_error((t_object *)x, "Unknown sample format.");
                err = MAX_ERR_GENERIC;
            }
        } else {
            object_error((t_object *)x, "Invalid sample format.");
            err = MAX_ERR_GENERIC;
        }
    }
    return err;
}

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

    CLASS_ATTR_SYM(c, "format", 0, t_buf_write, sampleformat);
    CLASS_ATTR_STYLE_LABEL(c, "format", 0, "enum", "Sample Format");
    CLASS_ATTR_ENUM(c,"format", 0, "int8 int16 int24 int32 float32 float64 mulaw alaw");
    CLASS_ATTR_ACCESSORS(c, "format", NULL, buf_write_setattr_format);
    CLASS_ATTR_BASIC(c, "format", 0);
    // @description Sets the bit depth or sample type, just like for the <o>buffer~</o> object. <br />
    // @copy EARS_DOC_ACCEPTED_SAMPLETYPES

    CLASS_ATTR_SYM(c, "vbrmode", 0, t_buf_write, mp3_vbrmode);
    CLASS_ATTR_STYLE_LABEL(c, "vbrmode", 0, "enum", "MP3 Variable Bitrate Mode");
    CLASS_ATTR_ENUM(c,"vbrmode", 0, "CBR ABR VBR");
    // @description Sets the variable bitrate mode for MP3 encoding: <br />
    // "CBR": constant bit rate (set via the <m>bitrate</m> attribute); <br />
    // "ABR": average bit rate (set via the <m>bitrate</m> attribute, possibly with
    // maximum and minimum rate set via the <m>maxbitrate</m> and <m>minbitrate</m> attributes); <br />
    // "VBR" (default): variable bit rate (with
    // maximum and minimum rate set via the <m>maxbitrate</m> and <m>minbitrate</m> attributes).

    CLASS_ATTR_LONG(c, "bitrate", 0, t_buf_write, mp3_bitrate);
    CLASS_ATTR_STYLE_LABEL(c, "bitrate", 0, "text", "MP3 Bitrate in kbps");
    // @description Sets the constant or average bitrate for MP3 or WavPack encoding, in kbps.

    CLASS_ATTR_LONG(c, "minbitrate", 0, t_buf_write, mp3_bitrate_min);
    CLASS_ATTR_STYLE_LABEL(c, "minbitrate", 0, "text", "MP3 Minimum Bitrate in kbps");
    // @description Sets the minimum bitrate for MP3 encoding (if applicable), in kbps.

    CLASS_ATTR_LONG(c, "maxbitrate", 0, t_buf_write, mp3_bitrate_max);
    CLASS_ATTR_STYLE_LABEL(c, "maxbitrate", 0, "text", "MP3 Maximum Bitrate in kbps");
    // @description Sets the minimum bitrate for MP3 encoding (if applicable), in kbps.

    CLASS_ATTR_CHAR(c, "correction", 0, t_buf_write, use_correction_file);
    CLASS_ATTR_STYLE_LABEL(c, "correction", 0, "onoff", "Write WavPack correction file");
    // @description Toggles the ability to write WavPack correction file (according to a <m>bitrate</m> that
    // needs to be set via the corresponding attribute).

    
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
        if (a == 0)
            sprintf(s, "llll: Names of Soundfiles"); // @out 0 @type llll @digest Names of soundfiles
             // @description Names of the soundfiles as introduced by the user, possibly modified for incremental counting.
        else
            sprintf(s, "llll: Full Paths of Saved Soundfiles"); // @out 1 @type llll @digest Full path of saved soundfiles
            // @description Complete paths of the saved soundfiles
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
        x->mp3_vbrmode = gensym("VBR");
        x->sampleformat = _sym_int16;
        
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
        
        earsbufobj_setup((t_earsbufobj *)x, "E4", "aa", NULL);

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

void buf_write_fill_encode_settings(t_buf_write *x, t_ears_encoding_settings *settings)
{
    // default:
    settings->vbr_type = EARS_MP3_VBRMODE_VBR;
    // 0 = use LAME defaults
    settings->bitrate = 0;
    settings->bitrate_min = 0;
    settings->bitrate_max = 0;

    if (x->mp3_vbrmode == gensym("CBR") || x->mp3_vbrmode == gensym("cbr"))
        settings->vbr_type = EARS_MP3_VBRMODE_CBR;
    else if (x->mp3_vbrmode == gensym("ABR") || x->mp3_vbrmode == gensym("abr"))
        settings->vbr_type = EARS_MP3_VBRMODE_ABR;
    else
        settings->vbr_type = EARS_MP3_VBRMODE_VBR;
    
    if (x->mp3_bitrate > 0)
        settings->bitrate = x->mp3_bitrate;
    if (x->mp3_bitrate_min > 0)
        settings->bitrate_min = x->mp3_bitrate_min;
    if (x->mp3_bitrate_min > 0)
        settings->bitrate_max = x->mp3_bitrate_max;
    
    settings->format = x->sampleformat;
    settings->use_correction_file = x->use_correction_file;
}



void checkForRejectedProperties(t_buf_write *x, const TagLib::PropertyMap &tags)
{
    return ;
    if(tags.size() > 0) {
        unsigned int longest = 0;
        for(TagLib::PropertyMap::ConstIterator i = tags.begin(); i != tags.end(); ++i) {
            if(i->first.size() > longest) {
                longest = i->first.size();
            }
        }
        //        cout << "-- rejected TAGs (properties) --" << endl;
        for(TagLib::PropertyMap::ConstIterator i = tags.begin(); i != tags.end(); ++i) {
            for(TagLib::StringList::ConstIterator j = i->second.begin(); j != i->second.end(); ++j) {
                object_warn((t_object *)x, "Rejected property: %s", i->first.to8Bit().c_str());
                //                cout << left << std::setw(longest) << i->first << " - " << '"' << *j << '"' << endl;
            }
        }
    }
}

void buf_write_write_tags(t_buf_write *x, t_symbol *filename, t_llll *tags)
{
    earsbufobj_mutex_lock((t_earsbufobj *)x);
    TagLib::FileRef f(filename->s_name);
    
    if(!f.isNull() && f.tag()) {
        for (t_llllelem *tag_el = tags->l_head; tag_el; tag_el = tag_el->l_next) {
            if (hatom_gettype(&tag_el->l_hatom) != H_LLLL) {
                object_error((t_object *)x, "A wrongly formatted tag has been dropped");
                continue;
            }
            t_llll *tag_ll = hatom_getllll(&tag_el->l_hatom);
            if (tag_ll->l_size < 1 || hatom_gettype(&tag_ll->l_head->l_hatom) != H_SYM) {
                object_error((t_object *)x, "A wrongly formatted tag has been dropped");
                continue;
            }
            
            t_symbol *s = hatom_getsym(&tag_ll->l_head->l_hatom);
            char mode = 'R';
            if (!s) {
                object_error((t_object *)x, "A wrongly formatted tag has been dropped");
                continue;
            }
            
            if (s == gensym("+")) {
                mode = 'I';
                if (!tag_ll->l_head->l_next) {
                    object_error((t_object *)x, "A wrongly formatted tag has been dropped");
                    continue;
                }
                s = hatom_getsym(&tag_ll->l_head->l_next->l_hatom);
                if (!s) {
                    object_error((t_object *)x, "A wrongly formatted tag has been dropped");
                    continue;
                }
            } else if  (s == gensym("-")) {
                mode = 'D';
                if (!tag_ll->l_head->l_next) {
                    object_error((t_object *)x, "A wrongly formatted tag has been dropped");
                    continue;
                }
                s = hatom_getsym(&tag_ll->l_head->l_next->l_hatom);
                if (!s) {
                    object_error((t_object *)x, "A wrongly formatted tag has been dropped");
                    continue;
                }
            }
            
            
            char field = mode;
            TagLib::String value = s->s_name;
            
            char *txtbuf = NULL;
            if (tag_ll->l_size > 1)
                hatom_to_text_buf(&tag_ll->l_head->l_next->l_hatom, &txtbuf);
            
            if (strcmp(s->s_name, "title") == 0)
                field = 't';
            else if (strcmp(s->s_name, "artist") == 0)
                field = 'a';
            else if (strcmp(s->s_name, "album") == 0)
                field = 'A';
            else if (strcmp(s->s_name, "comment") == 0)
                field = 'c';
            else if (strcmp(s->s_name, "genre") == 0)
                field = 'g';
            else if (strcmp(s->s_name, "year") == 0)
                field = 'y';
            else if (strcmp(s->s_name, "track") == 0)
                field = 'T';
            
            TagLib::Tag *t = f.tag();
            
            switch (field) {
                case 't':
                    t->setTitle(txtbuf ? txtbuf : "");
                    break;
                case 'a':
                    t->setArtist(txtbuf ? txtbuf : "");
                    break;
                case 'A':
                    t->setAlbum(txtbuf ? txtbuf : "");
                    break;
                case 'c':
                    t->setComment(txtbuf ? txtbuf : "");
                    break;
                case 'g':
                    t->setGenre(txtbuf ? txtbuf : "");
                    break;
                case 'y':
                    t->setYear(txtbuf ? atoi(txtbuf) : 0);
                    break;
                case 'T':
                    t->setTrack(txtbuf ? atoi(txtbuf) : 0);
                    break;
                case 'R':
                case 'I':
                    if (txtbuf) {
                        TagLib::PropertyMap map = f.file()->properties ();
                        if(field == 'R') {
                            map.replace(value, TagLib::String(txtbuf));
                        }
                        else {
                            map.insert(value, TagLib::String(txtbuf));
                        }
                        checkForRejectedProperties(x, f.file()->setProperties(map));
                    } else {
                        object_warn((t_object *)x, "Tag requested to be inserted or replaced, but no content given.");
                    }
                    break;
                case 'D': {
                    TagLib::PropertyMap map = f.file()->properties();
                    map.erase(value);
                    checkForRejectedProperties(x, f.file()->setProperties(map));
                    break;
                }
                default:
                    break;
            }
            
            bach_freeptr(txtbuf);
            
        }
        
        f.file()->save();
    }
    earsbufobj_mutex_unlock((t_earsbufobj *)x);
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
    
    t_ears_encoding_settings settings;
    buf_write_fill_encode_settings(x, &settings);
    
    t_llllelem *el; long i;
    t_llll *fullpaths = llll_get();
    for (i = 0, el = names->l_head; i < num_buffers && el; i++, el = el->l_next) {
        t_object *buf = earsbufobj_get_stored_buffer_obj((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, i);
        if (buf) {
            t_symbol *filename = hatom_getsym(&el->l_hatom);
            t_symbol *sampleformat = ears_buffer_get_sampleformat((t_object *)x, buf);
            t_symbol *orig_format = NULL;
            if (sampleformat != x->sampleformat) {
                orig_format = sampleformat;
                ears_buffer_set_sampleformat((t_object *)x, buf, x->sampleformat);
                settings.format = sampleformat;
            }
            
            ears_buffer_write(buf, filename, (t_object *)x, &settings);
            llll_appendsym(fullpaths, get_conformed_resolved_path(filename));
            
            if (orig_format)
                ears_buffer_set_sampleformat((t_object *)x, buf, orig_format);
        } else {
            object_error((t_object *)x, EARS_ERROR_BUF_NO_BUFFER);
        }
    }
    
    t_symbol **names_sym = (t_symbol **)bach_newptr(num_buffers * sizeof(t_symbol));
    t_symbol **fullpath_sym = (t_symbol **)bach_newptr(num_buffers * sizeof(t_symbol));
    t_llllelem *fullpath_el;
    for (i = 0, el = names->l_head, fullpath_el = fullpaths->l_head; el; el = el->l_next, fullpath_el = fullpath_el ? fullpath_el->l_next : NULL, i++) {
        names_sym[i] = hatom_getsym(&el->l_hatom);
        fullpath_sym[i] = fullpath_el ? hatom_getsym(&fullpath_el->l_hatom) : names_sym[i];
    }

    earsbufobj_outlet_symbol_list((t_earsbufobj *)x, 1, num_buffers, fullpath_sym);
    earsbufobj_outlet_symbol_list((t_earsbufobj *)x, 0, num_buffers, names_sym);

    earsbufobj_outlet_llll((t_earsbufobj *)x, 0, names);
    llll_free(names);
    llll_free(fullpaths);
    bach_freeptr(names_sym);
    bach_freeptr(fullpath_sym);
}


void buf_write_anything(t_buf_write *x, t_symbol *msg, long ac, t_atom *av)
{
    long inlet = earsbufobj_proxy_getinlet((t_earsbufobj *) x);

    t_llll *parsed = earsbufobj_parse_gimme((t_earsbufobj *) x, LLLL_OBJ_VANILLA, msg, ac, av);
    if (parsed && parsed->l_head) {
        if (inlet == 0) {
            long num_bufs = llll_get_num_symbols_root(parsed);
            earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_bufs, true);
            earsbufobj_store_buffer_list((t_earsbufobj *)x, parsed, 0);
            
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
