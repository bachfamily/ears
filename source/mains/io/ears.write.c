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

#define TAGLIB_STATIC

#include "ears.h"
#ifdef EARS_MP3_SUPPORT
#include "ears.mp3.h"
#endif
#include "ext.h"
#include "ext_obex.h"
#include "foundation/llllobj.h"
#include "foundation/llll_commons_ext.h"
#include "math/bach_math_utilities.h"
#include "ears.object.h"

#define LIBAIFF_NOCOMPAT 1 // do not use LibAiff 2 API compatibility
#include "libaiff.h"

#include "AudioFile.h"

#include "ears.libtag_commons.h"
#include "ears.spectralannotations.h"

typedef struct _buf_write {
    t_earsbufobj       e_ob;
    
    t_llll             *filenames;
    t_llll             *tags;
    t_llll             *markers;
    
    t_symbol           *sampleformat;
    
    t_symbol           *mp3_vbrmode;
    long               mp3_bitrate;
    long               mp3_bitrate_min;
    long               mp3_bitrate_max;
    
    char               use_correction_file;
    char               write_spectral_annotations;
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

void buf_write_markers_and_spectralannotation(t_buf_write *x, t_symbol *filename, t_llll *markers, t_buffer_obj *buf, double sr, t_ears_spectralbuf_metadata *data, t_symbol *sampleformat);
void buf_write_tags(t_buf_write *x, t_symbol *filename, t_llll *tags);

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

void C74_EXPORT ext_main(void* moduleRef)
{
#ifdef EARS_MP3_SUPPORT
    ears_mpg123_init();
#endif
    common_symbols_init();
    llllobj_common_symbols_init();
    
    if (llllobj_check_version(bach_get_current_llll_version()) || llllobj_test()) {
        ears_error_bachcheck();
        return;
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
    EARSBUFOBJ_DECLARE_COMMON_METHODS_HANDLETHREAD(write)

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

    CLASS_ATTR_CHAR(c, "spectralannotations", 0, t_buf_write, write_spectral_annotations);
    CLASS_ATTR_STYLE_LABEL(c, "spectralannotations", 0, "onoff", "Write Annotations for Spectral Buffers");
    // @description Toggles the ability to write annotations for spectral buffers that include the original
    // sampling rate, the bin size and offset, the bin unit and the individual bin positions.

    
    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
}



void buf_write_assist(t_buf_write *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        switch (a) {
            case 0:
                sprintf(s, "symbol/list/llll: Buffer names"); // @in 0 @type symbol/list/llll @digest Buffer names
                break;

            case 1:
                sprintf(s, "symbol/list/llll: Soundfile names"); // @in 1 @type symbol/list/llll @digest Soundfile names
                break;

            case 2:
                sprintf(s, "symbol/llll: Sample format"); // @in 2 @type symbol/llll @digest Sample format
                break;
                
            case 3:
                sprintf(s, "llll: Metadata Tags"); // @in 3 @type llll @digest Metadata tags
                break;
                
            case 4:
            default:
                sprintf(s, "llll: Markers"); // @in 4 @type llll @digest Markers
                break;
                
        }
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
        x->sampleformat = EARS_DEFAULT_WRITE_FORMAT;
        x->write_spectral_annotations = true;
        
        // @arg 0 @name filenames @optional 1 @type symbol
        // @digest Output file names
        // @description Sets the output file names for the incoming buffers
        
        t_llll *args = llll_parse(true_ac, argv);
        if (args)
            x->filenames = llll_clone(args);
        else
            x->filenames = llll_make();

        x->tags = llll_make();
        x->markers = llll_make();

        earsbufobj_init((t_earsbufobj *)x, 0);

        attr_args_process(x, argc, argv);
        
        earsbufobj_setup((t_earsbufobj *)x, "E4444", "zz", NULL);

        llll_free(args);
    }
    return x;
}


void buf_write_free(t_buf_write *x)
{
    llll_free(x->filenames);
    llll_free(x->markers);
    llll_free(x->tags);
    earsbufobj_free((t_earsbufobj *)x);
}

t_symbol *increment_symbol(t_symbol *base, long index)
{
    char outfilename[MAX_PATH_CHARS];
    const char *dot = strrchr(base->s_name, '.');
    if (!dot || dot == base->s_name) {
        snprintf_zero(outfilename, MAX_PATH_CHARS, "%s.%d", base->s_name, index);
        return gensym(outfilename);
    } else {
        long len = dot - base->s_name;
        strncpy(outfilename, base->s_name, len);
        snprintf_zero(outfilename + len, MAX_PATH_CHARS, ".%d.%s", index, dot+1);
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



void buf_write_bang(t_buf_write *x)
{
    long num_buffers = earsbufobj_get_num_inlet_stored_buffers((t_earsbufobj *)x, 0, false);
    
    earsbufobj_mutex_lock((t_earsbufobj *)x);
    if (!x->filenames->l_head) {
        object_error((t_object *)x, "No filenames inserted. Please input the soundfile names via the right inlet.");
        earsbufobj_mutex_unlock((t_earsbufobj *)x);
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
    
    t_llllelem *el, *tag_el, *mk_el; long i;
    t_llll *fullpaths = llll_get();
    for (i = 0, el = names->l_head, tag_el = x->tags ? x->tags->l_head : NULL, mk_el = x->markers ? x->markers->l_head : NULL;
         i < num_buffers && el;
         i++, el = el->l_next, tag_el = tag_el ? tag_el->l_next : NULL, mk_el = mk_el ? mk_el->l_next : NULL) {
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
            
            
            t_ears_spectralbuf_metadata *data = ears_spectralbuf_metadata_get((t_object *)x, buf);
            if ((x->write_spectral_annotations && data) || (mk_el && hatom_gettype(&mk_el->l_hatom) == H_LLLL))
                buf_write_markers_and_spectralannotation(x, filename,
                                                 mk_el && hatom_gettype(&mk_el->l_hatom) == H_LLLL ? hatom_getllll(&mk_el->l_hatom) : NULL,
                                                 buf, ears_buffer_get_sr((t_object *)x, buf),
                                                 data, x->sampleformat);
            

            if (tag_el && hatom_gettype(&tag_el->l_hatom) == H_LLLL)
                buf_write_tags(x, filename, hatom_getllll(&tag_el->l_hatom));
            
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
    for (i = 0, el = names->l_head, fullpath_el = fullpaths->l_head;
         el && i < num_buffers;
         el = el->l_next, fullpath_el = fullpath_el ? fullpath_el->l_next : NULL, i++) {
        names_sym[i] = hatom_getsym(&el->l_hatom);
        fullpath_sym[i] = fullpath_el ? hatom_getsym(&fullpath_el->l_hatom) : names_sym[i];
    }
    earsbufobj_mutex_unlock((t_earsbufobj *)x);

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
            
        } else if (inlet == 2) {
            earsbufobj_mutex_lock((t_earsbufobj *)x);
            if (parsed && parsed->l_head && hatom_gettype(&parsed->l_head->l_hatom) == H_SYM) {
                t_symbol *format = hatom_getsym(&parsed->l_head->l_hatom);
                if (format != _llllobj_sym_unknown && format != gensym("compressed"))
                    x->sampleformat = format;
                else if (format == _llllobj_sym_unknown)
                    object_warn((t_object *)x, "Using unknown sample type.");
            } else
                object_error((t_object *)x, "Invalid sample format");
            earsbufobj_mutex_unlock((t_earsbufobj *)x);
            
        } else if (inlet == 3) {
            earsbufobj_mutex_lock((t_earsbufobj *)x);
            llll_free(x->tags);
            x->tags = llll_clone(parsed);
            if (x->tags->l_depth == 2)
                llll_wrap_once(&x->tags);
            earsbufobj_mutex_unlock((t_earsbufobj *)x);

        } else if (inlet == 4) {
            earsbufobj_mutex_lock((t_earsbufobj *)x);
            llll_free(x->markers);
            x->markers = llll_clone(parsed);
            if (x->markers->l_depth == 2)
                llll_wrap_once(&x->markers);
            earsbufobj_mutex_unlock((t_earsbufobj *)x);

        }
    }
    llll_free(parsed);
}




double onset_to_fsamps(t_buf_write *x, double onset, long file_numsamps, double sr)
{
    switch (x->e_ob.l_timeunit) {
        case EARS_TIMEUNIT_MS:
            return ears_ms_to_samps(onset, sr);
            break;
        case EARS_TIMEUNIT_SECONDS:
            return ears_ms_to_samps(onset/1000., sr);
            break;
        case EARS_TIMEUNIT_DURATION_RATIO:
            return onset * file_numsamps;
            break;
        case EARS_TIMEUNIT_SAMPS:
        default:
            return onset;
            break;
    }
}


void buf_write_tags_ID3v1(t_buf_write *x, TagLib::ID3v1::Tag *tags, t_llll *ll)
{
    if (tags && ll) {
        for (t_llllelem *el = ll->l_head; el; el = el->l_next) {
            if (hatom_gettype(&el->l_hatom) == H_LLLL) {
                t_llll *el_ll = hatom_getllll(&el->l_hatom);
                if (el_ll && el_ll->l_size >= 2) {
                    t_symbol *s = hatom_getsym(&el_ll->l_head->l_hatom);
                    if (s) {
                        if (strcmp_case_insensitive(s->s_name, "title") == 0) {
                            t_symbol *val = hatom_getsym(&el_ll->l_head->l_next->l_hatom);
                            if (val)
                                tags->setTitle(val->s_name);
                        } else if (strcmp_case_insensitive(s->s_name, "album") == 0) {
                            t_symbol *val = hatom_getsym(&el_ll->l_head->l_next->l_hatom);
                            if (val)
                                tags->setAlbum(val->s_name);
                        } else if (strcmp_case_insensitive(s->s_name, "genre") == 0) {
                            if (hatom_gettype(&el_ll->l_head->l_next->l_hatom) == H_LONG) {
                                tags->setGenreNumber(hatom_getlong(&el_ll->l_head->l_next->l_hatom));
                            } else {
                                t_symbol *val = hatom_getsym(&el_ll->l_head->l_next->l_hatom);
                                if (val)
                                    tags->setGenre(val->s_name);
                            }
                        } else if (strcmp_case_insensitive(s->s_name, "artist") == 0) {
                            t_symbol *val = hatom_getsym(&el_ll->l_head->l_next->l_hatom);
                            if (val)
                                tags->setArtist(val->s_name);
                        } else if (strcmp_case_insensitive(s->s_name, "comment") == 0) {
                            t_symbol *val = hatom_getsym(&el_ll->l_head->l_next->l_hatom);
                            if (val)
                                tags->setComment(val->s_name);
                        } else if (strcmp_case_insensitive(s->s_name, "year") == 0) {
                            tags->setYear(hatom_getlong(&el_ll->l_head->l_next->l_hatom));
                        } else if (strcmp_case_insensitive(s->s_name, "track") == 0) {
                            tags->setTrack(hatom_getlong(&el_ll->l_head->l_next->l_hatom));
                        }
                    }
                }
            }
        }
    }
}


void buf_write_tags_XIPHCOMMENT(t_buf_write *x, TagLib::Ogg::XiphComment *tags, t_llll *ll)
{
    if (tags && ll) {
        for (t_llllelem *el = ll->l_head; el; el = el->l_next) {
            if (hatom_gettype(&el->l_hatom) == H_LLLL) {
                t_llll *el_ll = hatom_getllll(&el->l_hatom);
                if (el_ll && el_ll->l_size >= 2) {
                    t_symbol *s = hatom_getsym(&el_ll->l_head->l_hatom);
                    if (s) {
                        if (strcmp_case_insensitive(s->s_name, "title") == 0) {
                            t_symbol *val = hatom_getsym(&el_ll->l_head->l_next->l_hatom);
                            if (val)
                                tags->setTitle(val->s_name);
                        } else if (strcmp_case_insensitive(s->s_name, "album") == 0) {
                            t_symbol *val = hatom_getsym(&el_ll->l_head->l_next->l_hatom);
                            if (val)
                                tags->setAlbum(val->s_name);
                        } else if (strcmp_case_insensitive(s->s_name, "genre") == 0) {
                            t_symbol *val = hatom_getsym(&el_ll->l_head->l_next->l_hatom);
                            if (val)
                                tags->setGenre(val->s_name);
                        } else if (strcmp_case_insensitive(s->s_name, "artist") == 0) {
                            t_symbol *val = hatom_getsym(&el_ll->l_head->l_next->l_hatom);
                            if (val)
                                tags->setArtist(val->s_name);
                        } else if (strcmp_case_insensitive(s->s_name, "comment") == 0) {
                            t_symbol *val = hatom_getsym(&el_ll->l_head->l_next->l_hatom);
                            if (val)
                                tags->setComment(val->s_name);
                        } else if (strcmp_case_insensitive(s->s_name, "year") == 0) {
                            tags->setYear(hatom_getlong(&el_ll->l_head->l_next->l_hatom));
                        } else if (strcmp_case_insensitive(s->s_name, "track") == 0) {
                            tags->setTrack(hatom_getlong(&el_ll->l_head->l_next->l_hatom));
                        }
                    }
                }
            }
        }
    }
}

void buf_write_tags_APE(t_buf_write *x, TagLib::APE::Tag *tags, t_llll *ll)
{
    if (tags && ll) {
        for (t_llllelem *el = ll->l_head; el; el = el->l_next) {
            if (hatom_gettype(&el->l_hatom) == H_LLLL) {
                t_llll *el_ll = hatom_getllll(&el->l_hatom);
                if (el_ll && el_ll->l_size >= 2) {
                    t_symbol *s = hatom_getsym(&el_ll->l_head->l_hatom);
                    if (s) {
                        if (strcmp_case_insensitive(s->s_name, "title") == 0) {
                            t_symbol *val = hatom_getsym(&el_ll->l_head->l_next->l_hatom);
                            if (val)
                                tags->setTitle(val->s_name);
                        } else if (strcmp_case_insensitive(s->s_name, "album") == 0) {
                            t_symbol *val = hatom_getsym(&el_ll->l_head->l_next->l_hatom);
                            if (val)
                                tags->setAlbum(val->s_name);
                        } else if (strcmp_case_insensitive(s->s_name, "genre") == 0) {
                            t_symbol *val = hatom_getsym(&el_ll->l_head->l_next->l_hatom);
                            if (val)
                                tags->setGenre(val->s_name);
                        } else if (strcmp_case_insensitive(s->s_name, "artist") == 0) {
                            t_symbol *val = hatom_getsym(&el_ll->l_head->l_next->l_hatom);
                            if (val)
                                tags->setArtist(val->s_name);
                        } else if (strcmp_case_insensitive(s->s_name, "comment") == 0) {
                            t_symbol *val = hatom_getsym(&el_ll->l_head->l_next->l_hatom);
                            if (val)
                                tags->setComment(val->s_name);
                        } else if (strcmp_case_insensitive(s->s_name, "year") == 0) {
                            tags->setYear(hatom_getlong(&el_ll->l_head->l_next->l_hatom));
                        } else if (strcmp_case_insensitive(s->s_name, "track") == 0) {
                            tags->setTrack(hatom_getlong(&el_ll->l_head->l_next->l_hatom));
                        } else {
                            char *txtbuf = NULL;
                            hatom_to_text_buf(&el_ll->l_head->l_next->l_hatom, &txtbuf);
                            TagLib::APE::Item item = TagLib::APE::Item(s->s_name, txtbuf);
                            tags->setItem(s->s_name, item);
                            bach_freeptr(txtbuf);
                        }
                    }
                }
            }
        }
    }
}


void buf_write_tags_INFO(t_buf_write *x, TagLib::RIFF::Info::Tag *tags, t_llll *ll)
{
    if (tags && ll) {
        for (t_llllelem *el = ll->l_head; el; el = el->l_next) {
            if (hatom_gettype(&el->l_hatom) == H_LLLL) {
                t_llll *el_ll = hatom_getllll(&el->l_hatom);
                if (el_ll && el_ll->l_size >= 2) {
                    t_symbol *s = hatom_getsym(&el_ll->l_head->l_hatom);
                    if (s) {
                        if (strcmp_case_insensitive(s->s_name, "title") == 0) {
                            t_symbol *val = hatom_getsym(&el_ll->l_head->l_next->l_hatom);
                            if (val)
                                tags->setTitle(val->s_name);
                        } else if (strcmp_case_insensitive(s->s_name, "album") == 0) {
                            t_symbol *val = hatom_getsym(&el_ll->l_head->l_next->l_hatom);
                            if (val)
                                tags->setAlbum(val->s_name);
                        } else if (strcmp_case_insensitive(s->s_name, "genre") == 0) {
                            t_symbol *val = hatom_getsym(&el_ll->l_head->l_next->l_hatom);
                            if (val)
                                tags->setGenre(val->s_name);
                        } else if (strcmp_case_insensitive(s->s_name, "artist") == 0) {
                            t_symbol *val = hatom_getsym(&el_ll->l_head->l_next->l_hatom);
                            if (val)
                                tags->setArtist(val->s_name);
                        } else if (strcmp_case_insensitive(s->s_name, "comment") == 0) {
                            t_symbol *val = hatom_getsym(&el_ll->l_head->l_next->l_hatom);
                            if (val)
                                tags->setComment(val->s_name);
                        } else if (strcmp_case_insensitive(s->s_name, "year") == 0) {
                            tags->setYear(hatom_getlong(&el_ll->l_head->l_next->l_hatom));
                        } else if (strcmp_case_insensitive(s->s_name, "track") == 0) {
                            tags->setTrack(hatom_getlong(&el_ll->l_head->l_next->l_hatom));
                        } else {
                            char *txtbuf = NULL;
                            hatom_to_text_buf(&el_ll->l_head->l_next->l_hatom, &txtbuf);
                            tags->setFieldText(s->s_name, txtbuf);
                            bach_freeptr(txtbuf);
                        }
                    }
                }
            }
        }
    }
}


void buf_write_tags_ID3v2(t_buf_write *x, TagLib::ID3v2::Tag *tags, t_llll *ll)
{
    if (tags && ll) {
        for (t_llllelem *el = ll->l_head; el; el = el->l_next) {
            if (hatom_gettype(&el->l_hatom) == H_LLLL) {
                t_llll *el_ll = hatom_getllll(&el->l_hatom);
                if (el_ll && el_ll->l_size >= 2) {
                    t_symbol *s = hatom_getsym(&el_ll->l_head->l_hatom);
                    if (s && strlen(s->s_name) > 0) {
                        if (strcmp_case_insensitive(s->s_name, "title") == 0) {
                            t_symbol *val = hatom_getsym(&el_ll->l_head->l_next->l_hatom);
                            if (val)
                                tags->setTitle(val->s_name);
                        } else if (strcmp_case_insensitive(s->s_name, "album") == 0) {
                            t_symbol *val = hatom_getsym(&el_ll->l_head->l_next->l_hatom);
                            if (val)
                                tags->setAlbum(val->s_name);
                        } else if (strcmp_case_insensitive(s->s_name, "genre") == 0) {
                            t_symbol *val = hatom_getsym(&el_ll->l_head->l_next->l_hatom);
                            if (val)
                                tags->setGenre(val->s_name);
                        } else if (strcmp_case_insensitive(s->s_name, "artist") == 0) {
                            t_symbol *val = hatom_getsym(&el_ll->l_head->l_next->l_hatom);
                            if (val)
                                tags->setArtist(val->s_name);
                        } else if (strcmp_case_insensitive(s->s_name, "comment") == 0) {
                            t_symbol *val = hatom_getsym(&el_ll->l_head->l_next->l_hatom);
                            if (val)
                                tags->setComment(val->s_name);
                        } else if (strcmp_case_insensitive(s->s_name, "year") == 0) {
                            tags->setYear(hatom_getlong(&el_ll->l_head->l_next->l_hatom));
                        } else if (strcmp_case_insensitive(s->s_name, "track") == 0) {
                            tags->setTrack(hatom_getlong(&el_ll->l_head->l_next->l_hatom));
                        } else {
                            long l = strlen(s->s_name);
                            char *s_frameid = (char *)bach_newptr((l + 1) * sizeof(char));
                            for (long i = 0; i < l; i++)
                                s_frameid[i] = toupper(s->s_name[i]);
                            s_frameid[l] = 0;
                            

                            TagLib::ByteVector frameID = keyToFrameID(s_frameid);
                            if (frameID.isEmpty() && l == 4)
                                frameID = TagLib::ByteVector(s_frameid, 4);
                            
                            if (!frameID.isEmpty()) {
                                if (strcmp(s_frameid, "APIC") == 0) {
                                    object_warn((t_object *)x, "APIC frames are unsupported at writing time.");
                                } else if (strcmp(s_frameid, "CHAP") == 0) {
                                    object_warn((t_object *)x, "CHAP frames are unsupported at writing time.");
                                } else if (strcmp(s_frameid, "CTOC") == 0) {
                                    object_warn((t_object *)x, "CTOC frames are unsupported at writing time.");
                                } else if (strcmp(s_frameid, "ETCO") == 0) {
                                    object_warn((t_object *)x, "ETCO frames are unsupported at writing time.");
                                } else if (strcmp(s_frameid, "ETCO") == 0) {
                                    object_warn((t_object *)x, "ETCO frames are unsupported at writing time.");
                                } else if (strcmp(s_frameid, "GEOB") == 0) {
                                    object_warn((t_object *)x, "GEOB frames are unsupported at writing time.");
                                } else if (strcmp(s_frameid, "OWNE") == 0) {
                                    TagLib::ID3v2::OwnershipFrame *fr = new TagLib::ID3v2::OwnershipFrame();
                                    char *price = NULL, *date = NULL, *seller = NULL;
                                    if (el_ll->l_size > 1)
                                        hatom_to_text_buf(&el_ll->l_head->l_next->l_hatom, &price);
                                    if (el_ll->l_size > 2)
                                        hatom_to_text_buf(&el_ll->l_head->l_next->l_next->l_hatom, &date);
                                    if (el_ll->l_size > 3)
                                        hatom_to_text_buf(&el_ll->l_head->l_next->l_next->l_next->l_hatom, &seller);
                                    if (price) fr->setPricePaid(price);
                                    if (date) fr->setDatePurchased(date);
                                    if (seller) fr->setSeller(seller);
                                    tags->addFrame(fr);
                                    if (price) bach_freeptr(price);
                                    if (date) bach_freeptr(date);
                                    if (seller) bach_freeptr(seller);
                                } else if (strcmp(s_frameid, "PCST") == 0) {
                                    object_warn((t_object *)x, "PCST frames are unsupported at writing time.");
                                } else if (strcmp(s_frameid, "POPM") == 0) {
                                    TagLib::ID3v2::PopularimeterFrame *fr = new TagLib::ID3v2::PopularimeterFrame();
                                    char *email = NULL;
                                    long rating = 0, counter = 0;
                                    if (el_ll->l_size > 1)
                                        hatom_to_text_buf(&el_ll->l_head->l_next->l_hatom, &email);
                                    if (el_ll->l_size > 2)
                                        rating = hatom_getlong(&el_ll->l_head->l_next->l_next->l_hatom);
                                    if (el_ll->l_size > 3)
                                        counter = hatom_getlong(&el_ll->l_head->l_next->l_next->l_next->l_hatom);
                                    if (email) fr->setEmail(email);
                                    if (el_ll->l_size > 2) fr->setRating(rating);
                                    if (el_ll->l_size > 3) fr->setCounter(counter);
                                    tags->addFrame(fr);
                                    if (email) bach_freeptr(email);
                                } else if (strcmp(s_frameid, "PRIV") == 0) {
                                    TagLib::ID3v2::PrivateFrame *fr = new TagLib::ID3v2::PrivateFrame();
                                    char *owner = NULL, *data = NULL;
                                    if (el_ll->l_size > 1)
                                        hatom_to_text_buf(&el_ll->l_head->l_next->l_hatom, &owner);
                                    if (el_ll->l_size > 2)
                                        hatom_to_text_buf(&el_ll->l_head->l_next->l_next->l_hatom, &data);
                                    if (owner) fr->setOwner(owner);
                                    if (data) fr->setData(data);
                                    tags->addFrame(fr);
                                    if (owner) bach_freeptr(owner);
                                    if (data) bach_freeptr(data);
                                } else if (strcmp(s_frameid, "RVA2") == 0) {
                                    object_warn((t_object *)x, "RVA2 frames are unsupported at writing time.");
                                } else if (strcmp(s_frameid, "SYLT") == 0) {
                                    object_warn((t_object *)x, "SYLT frames are unsupported at writing time.");
                                } else if (strcmp(s_frameid, "CTOC") == 0) {
                                    object_warn((t_object *)x, "CTOC frames are unsupported at writing time.");
                                } else if (strcmp(s_frameid, "UFID") == 0) {
                                    char *owner = NULL, *identif = NULL;
                                    if (el_ll->l_size > 1)
                                        hatom_to_text_buf(&el_ll->l_head->l_next->l_hatom, &owner);
                                    if (el_ll->l_size > 2)
                                        hatom_to_text_buf(&el_ll->l_head->l_next->l_next->l_hatom, &identif);
                                    TagLib::ID3v2::UniqueFileIdentifierFrame *fr = new TagLib::ID3v2::UniqueFileIdentifierFrame(owner, TagLib::ByteVector(identif));
                                    tags->addFrame(fr);
                                    if (owner) bach_freeptr(owner);
                                    if (identif) bach_freeptr(identif);
                                } else {
                                    TagLib::StringList sl;
                                    for (t_llllelem *frel = el_ll->l_head->l_next; frel; frel = frel->l_next) {
                                        char *txtbuf = NULL;
                                        hatom_to_text_buf(&frel->l_hatom, &txtbuf);
                                        sl.append(txtbuf);
                                        bach_freeptr(txtbuf);
                                    }
                                    TagLib::ID3v2::Frame *fr = TagLib::ID3v2::Frame::createTextualFrame(frameIDToKey(frameID), sl);
                                    tags->addFrame(fr);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

t_llll *llll_key_root(t_llll *ll, t_symbol *s)
{
    for (t_llllelem *el = ll->l_head; el; el = el->l_next) {
        if (hatom_gettype(&el->l_hatom) == H_LLLL){
            t_llll *tmp = hatom_getllll(&el->l_hatom);
            if (tmp && tmp->l_head && hatom_gettype(&tmp->l_head->l_hatom) == H_SYM && hatom_getsym(&tmp->l_head->l_hatom) == s)
                return tmp;
        }
    }
    return NULL;
}

void buf_write_tags(t_buf_write *x, t_symbol *filename, t_llll *tags_ll)
{
    TagLib::FileRef f(filename->s_name);
    TagLib::File *file = f.file();
    
    t_llll *tags_APE = llll_key_root(tags_ll, gensym("APE"));
    t_llll *tags_ID3v1 = llll_key_root(tags_ll, gensym("ID3v1"));
    t_llll *tags_ID3v2 = llll_key_root(tags_ll, gensym("ID3v2"));
    t_llll *tags_INFO = llll_key_root(tags_ll, gensym("INFO"));
    t_llll *tags_XIPHCOMMENT = llll_key_root(tags_ll, gensym("XIPHCOMMENT"));

    TagLib::MPEG::File *MPEGfile = dynamic_cast<TagLib::MPEG::File *>(file);
    if (MPEGfile) {
        buf_write_tags_APE(x, MPEGfile->APETag(), tags_APE);
        buf_write_tags_ID3v1(x, MPEGfile->ID3v1Tag(), tags_ID3v1);
        buf_write_tags_ID3v2(x, MPEGfile->ID3v2Tag(), tags_ID3v2);
    }

    TagLib::WavPack::File *WAVPACKfile = dynamic_cast<TagLib::WavPack::File *>(file);
    if (WAVPACKfile) {
        buf_write_tags_APE(x, WAVPACKfile->APETag(), tags_APE);
        buf_write_tags_ID3v1(x, WAVPACKfile->ID3v1Tag(), tags_ID3v1);
    }
    
    TagLib::RIFF::File *RIFFfile = dynamic_cast<TagLib::RIFF::File *>(file);
    if (RIFFfile) {
        TagLib::RIFF::AIFF::File *RIFFAIFFfile = dynamic_cast<TagLib::RIFF::AIFF::File *>(file);
        if (RIFFAIFFfile) {
            buf_write_tags_ID3v2(x, RIFFAIFFfile->tag(), tags_ID3v2);
        }
        
        TagLib::RIFF::WAV::File *RIFFWAVfile = dynamic_cast<TagLib::RIFF::WAV::File *>(file);
        if (RIFFWAVfile) {
            buf_write_tags_INFO(x, RIFFWAVfile->InfoTag(), tags_INFO);
            buf_write_tags_ID3v2(x, RIFFWAVfile->tag(), tags_ID3v2);
        }
    }
    
    TagLib::Ogg::File *OGGfile = dynamic_cast<TagLib::Ogg::File *>(file);
    if (OGGfile) {
        TagLib::Ogg::FLAC::File *OGGFLACfile = dynamic_cast<TagLib::Ogg::FLAC::File *>(file);
        if (OGGFLACfile) {
            buf_write_tags_XIPHCOMMENT(x, OGGFLACfile->tag(), tags_XIPHCOMMENT);
        }
        
        TagLib::Ogg::Opus::File *OGGOPUSfile = dynamic_cast<TagLib::Ogg::Opus::File *>(file);
        if (OGGOPUSfile) {
            buf_write_tags_XIPHCOMMENT(x, OGGOPUSfile->tag(), tags_XIPHCOMMENT);
        }
        
        TagLib::Ogg::Speex::File *OGGSPEEXfile = dynamic_cast<TagLib::Ogg::Speex::File *>(file);
        if (OGGSPEEXfile) {
            buf_write_tags_XIPHCOMMENT(x, OGGSPEEXfile->tag(), tags_XIPHCOMMENT);
        }
        
        TagLib::Ogg::Vorbis::File *OGGVORBISfile = dynamic_cast<TagLib::Ogg::Vorbis::File *>(file);
        if (OGGVORBISfile) {
            buf_write_tags_XIPHCOMMENT(x, OGGVORBISfile->tag(), tags_XIPHCOMMENT);
        }
    }
    
    TagLib::FLAC::File *FLACfile = dynamic_cast<TagLib::FLAC::File *>(file);
    if (FLACfile) {
        buf_write_tags_ID3v1(x, FLACfile->ID3v1Tag(), tags_ID3v1);
        buf_write_tags_ID3v2(x, FLACfile->ID3v2Tag(), tags_ID3v2);
    }
    
    TagLib::APE::File *APEfile = dynamic_cast<TagLib::APE::File *>(file);
    if (APEfile) {
        buf_write_tags_APE(x, APEfile->APETag(), tags_APE);
        buf_write_tags_ID3v1(x, APEfile->ID3v1Tag(), tags_ID3v1);
    }
    
    f.save();
}


/*
void buf_write_tags(t_buf_write *x, t_symbol *filename, t_llll *tags)
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
 */

void buf_write_markers_and_spectralannotation_AIFF(t_buf_write *x, t_symbol *filename, t_llll *markers, t_buffer_obj *buf, double sr, t_ears_spectralbuf_metadata *data)
{
    if ((!markers || markers->l_size == 0) && !data)
        return;

    AIFF_Ref ref = AIFF_OpenFile(filename->s_name, F_RDONLY);
    uint64_t nSamples;
    int channels;
    double samplingRate;
    int bitsPerSample;
    int segmentSize;
    int32_t *samples = NULL;
    if (AIFF_GetAudioFormat(ref,&nSamples,&channels,
                            &samplingRate,&bitsPerSample,
                            &segmentSize) < 1 ) {
        object_error((t_object *)x, "Error writing markers.");
        AIFF_CloseFile(ref);
        return;
    } else {
        samples = (int32_t *)bach_newptr(channels * nSamples * sizeof(int32_t));
        int numReadSamples = AIFF_ReadSamples32Bit(ref, samples, channels * nSamples);
        if (numReadSamples != channels * nSamples) {
            object_warn((t_object *)x, "The output file may be corrupted. Try removing markers.");
        }
    }
    AIFF_CloseFile(ref);

    ref = AIFF_OpenFile(filename->s_name, F_WRONLY);
    if (ref) {
        // gotta re-write the samples
        AIFF_SetAudioFormat(ref, channels, sr, bitsPerSample);
        if (AIFF_StartWritingSamples(ref) < 1) {
            object_error((t_object *)x, "Error writing file");
        } else {
            if (AIFF_WriteSamples32Bit(ref, samples, channels * nSamples) < 1) {
                object_warn((t_object *)x, "The output file may be corrupted. Try removing markers.");
            }
            AIFF_EndWritingSamples(ref);
        }
        
        
        if (data) { // add spectral annotations
            char *annotation = ears_spectralannotation_get_string((t_object *)x, sr, data);
            AIFF_SetAttribute(ref, AIFF_ANNO, annotation);
            bach_freeptr(annotation);
        }
        
        if (markers && markers->l_head) {
            AIFF_StartWritingMarkers(ref);
            for (t_llllelem *mk = markers->l_head; mk; mk = mk->l_next) {
                t_llll *mk_ll = hatom_getllll(&mk->l_hatom);
                if (mk_ll && mk_ll->l_head) {
                    char *txtbuf = NULL;
                    uint64_t onset_samps = round(onset_to_fsamps(x, hatom_getdouble(&mk_ll->l_head->l_hatom), nSamples, samplingRate));
                    if (mk_ll->l_head->l_next)
                        hatom_to_text_buf(&mk_ll->l_head->l_next->l_hatom, &txtbuf);
                    
                    AIFF_WriteMarker(ref, onset_samps, txtbuf);
                    
                    if (txtbuf)
                        bach_freeptr(txtbuf);
                }
            }
            
            AIFF_EndWritingMarkers(ref);
        }
        
        AIFF_CloseFile(ref);
    }
    if (samples)
        bach_freeptr(samples);
}


void AudioCues_from_llll(t_buf_write *x, t_llll *ll, t_buffer_obj *buf, AudioFile<float> &audiofile)
{
    long count = 1;
    std::vector<AudioCue> cues;
    for (t_llllelem *el = ll->l_head; el; el = el->l_next) {
        if (hatom_gettype(&el->l_hatom) == H_LLLL) {
            t_llll *subll = hatom_getllll(&el->l_hatom);
            if (subll && subll->l_head) {
                if (hatom_gettype(&subll->l_head->l_hatom) == H_LLLL) {
                    //may be a region
                    t_llll *subsubll = hatom_getllll(&subll->l_head->l_hatom);
                    if (subsubll && subsubll->l_size >= 3 && hatom_getsym(&subsubll->l_head->l_hatom) == gensym("region")) {
                        AudioCue this_cue;
                        this_cue.cueID = count++;
                        this_cue.isRegion = true;
                        this_cue.sampleStart = earsbufobj_time_to_samps((t_earsbufobj *)x, hatom_getdouble(&subsubll->l_head->l_next->l_hatom), buf);
                        this_cue.sampleLength = earsbufobj_time_to_samps((t_earsbufobj *)x, hatom_getdouble(&subsubll->l_head->l_next->l_next->l_hatom), buf);
                        if (subll->l_head->l_next) {
                            char *txtbuf = NULL;
                            hatom_to_text_buf(&subll->l_head->l_next->l_hatom, &txtbuf);
                            this_cue.label = txtbuf;
                            bach_freeptr(txtbuf);
                        } else {
                            this_cue.label = "";
                        }
                        cues.push_back(this_cue);
                    }

                } else {
                    AudioCue this_cue;
                    this_cue.cueID = count++;
                    this_cue.isRegion = false;
                    this_cue.sampleStart = earsbufobj_time_to_samps((t_earsbufobj *)x, hatom_getdouble(&subll->l_head->l_hatom), buf);
                    this_cue.sampleLength = 0;
                    if (subll->l_head->l_next) {
                        char *txtbuf = NULL;
                        hatom_to_text_buf(&subll->l_head->l_next->l_hatom, &txtbuf);
                        this_cue.label = txtbuf;
                        bach_freeptr(txtbuf);
                    } else {
                        this_cue.label = "";
                    }
                    cues.push_back(this_cue);
                }
            }
        } else {
            // single marker
            AudioCue this_cue;
            this_cue.cueID = count++;
            this_cue.isRegion = false;
            this_cue.label = "";
            this_cue.sampleStart = earsbufobj_time_to_samps((t_earsbufobj *)x, hatom_getdouble(&el->l_hatom), buf);
            this_cue.sampleLength = 0;
            cues.push_back(this_cue);
        }
    }
    audiofile.setCues(cues);
}

AudioEncoding sample_format_to_encoding(t_symbol *sampleformat)
{
    if (sampleformat == _sym_int8 || sampleformat == _sym_int16 || sampleformat == _sym_int24 || sampleformat == _sym_int32)
        return AudioEncoding::Encoding_PCM;
    if (sampleformat == _sym_float32 || sampleformat == _sym_float64)
        return AudioEncoding::Encoding_IEEEFloat;
    if (sampleformat == _sym_mulaw)
        return AudioEncoding::Encoding_MULaw;
    if (sampleformat == gensym("alaw"))
        return AudioEncoding::Encoding_ALaw;
    if (sampleformat == gensym("compressed"))
        return AudioEncoding::Encoding_Compressed;
    return AudioEncoding::Encoding_Unknown;
}

void buf_write_markers_WAV(t_buf_write *x, t_symbol *filename, t_llll *markers,
                           t_buffer_obj *buf, double sr, t_ears_spectralbuf_metadata *data, t_symbol *sampleformat)
{
    AudioFile<float> audioFile;
    audioFile.load(filename->s_name);
    AudioCues_from_llll(x, markers, buf, audioFile);
    audioFile.save(filename->s_name, AudioFileFormat::Wave, sample_format_to_encoding(sampleformat));
}

void buf_write_markers_and_spectralannotation(t_buf_write *x, t_symbol *filename,
                                              t_llll *markers, t_buffer_obj *buf, double sr,
                                              t_ears_spectralbuf_metadata *data, t_symbol *sampleformat)
{
    if ((!markers || markers->l_size == 0) && !data)
        return;
    
    if (ears_symbol_ends_with(filename, ".aif", true) || ears_symbol_ends_with(filename, ".aiff", true)) {
        // we handle spectralannotation via ANNOTATIONS directly with markers,
        // so we don't read-write the same thing twice
        buf_write_markers_and_spectralannotation_AIFF(x, filename, markers, buf, sr, x->write_spectral_annotations ? data : NULL);
        
    } else if (ears_symbol_ends_with(filename, ".wav", true) || ears_symbol_ends_with(filename, ".wave", true)){
        if (x->write_spectral_annotations && data)
            ears_spectralannotation_write((t_object *)x, filename, sr, data);
        
        if (markers)
            buf_write_markers_WAV(x, filename, markers, buf, sr, data, sampleformat);
        
    } else if (ears_symbol_ends_with(filename, ".wv", true)){
        if (x->write_spectral_annotations && data)
            ears_spectralannotation_write((t_object *)x, filename, sr, data);
    } else {
        
    }
}
