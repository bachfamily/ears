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
	ears basic
 
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
#include "ears.mp3.h"
#include "ears.wavpack.h"

#include "ears.libtag_commons.h"

#define LIBAIFF_NOCOMPAT 1 // do not use LibAiff 2 API compatibility
#include <libaiff/libaiff.h>

#include "AudioFile.h"

typedef struct _buf_read {
    t_earsbufobj       e_ob;
    
    char                native_mp3_handling;
    
    t_llll              *filenames;
    
    char                human_readable_frame_ids;
    
    char                mustclear;
} t_buf_read;



// Prototypes
t_buf_read*         buf_read_new(t_symbol *s, short argc, t_atom *argv);
void			buf_read_free(t_buf_read *x);
void			buf_read_bang(t_buf_read *x);
void			buf_read_anything(t_buf_read *x, t_symbol *msg, long ac, t_atom *av);
void            buf_read_append(t_buf_read *x, t_symbol *msg, long ac, t_atom *av);
void            buf_read_append_deferred(t_buf_read *x, t_symbol *msg, long ac, t_atom *av);
void            buf_read_load_deferred(t_buf_read *x, t_symbol *msg, long ac, t_atom *av);
long            buf_read_acceptsdrag(t_buf_read *x, t_object *drag, t_object *view);

t_llll *buf_read_tags(t_buf_read *x, t_symbol *filename);
t_llll *buf_read_markers_and_spectralannotation(t_buf_read *x, t_symbol *filename, t_ears_spectralbuf_metadata *data, bool *has_data, double *sr);

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

    // @method load @digest Import files
    // @description The <m>load</m> message, followed by a list of filenames, will import the files as buffers
    // appending them to the existing one, and then outputting the whole set of output buffer names.
    class_addmethod(c, (method)buf_read_load_deferred,             "load",            A_GIMME, 0);

    // @method (drag) @digest Drag-and-drop file loading
    // @description The specified file(s) are read from disk and converted into buffers.
    class_addmethod(c, (method)buf_read_acceptsdrag, "acceptsdrag_locked", A_CANT, 0);
    class_addmethod(c, (method)buf_read_acceptsdrag, "acceptsdrag_unlocked", A_CANT, 0);
    
    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_timeunit_attr(c);
    earsbufobj_class_add_naming_attr(c);

    CLASS_ATTR_CHAR(c, "nativemp3",	0,	t_buf_read, native_mp3_handling);
    CLASS_ATTR_STYLE_LABEL(c, "nativemp3", 0, "onoff", "Native MP3 Handling");
    // @description Toggles native MP3 handling.

    CLASS_ATTR_CHAR(c, "hr",    0,    t_buf_read, human_readable_frame_ids);
    CLASS_ATTR_BASIC(c, "hr", 0);
    CLASS_ATTR_STYLE_LABEL(c, "hr", 0, "onoff", "Human-Readable IDs");
    // @description Toggles the ability to output human-readable Frames and Tag IDs instead of four-letter codes
    
    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
    return 0;
}

long buf_read_acceptsdrag(t_buf_read *x, t_object *drag, t_object *view)
{
//    long l = jdrag_itemcount(drag);
    x->mustclear = true;
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
        char *type = NULL;
        llllobj_get_llll_outlet_type_as_string((t_object *) x, LLLL_OBJ_VANILLA, a, &type);
        switch (a) {
            case 0:
                sprintf(s, "symbol/list: Buffer Names"); // @out 0 @type symbol/list @digest Buffer names
                break;
            case 1:
                sprintf(s, "symbol/list: Soundfile Names"); // @out 1 @type symbol/list @digest Soundfile names
                break;
            case 2:
                sprintf(s, "llll (%s): Metadata Tags", type); // @out 2 @type llll @digest Metadata tags
                break;
            case 3:
            default:
                sprintf(s, "llll (%s): Markers", type); // @out 3 @type llll @digest Markers
                break;
        }
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
        
        earsbufobj_setup((t_earsbufobj *)x, "4", "44aE", names);

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
        t_llll *tags = llll_get();
        t_llll *markers = llll_get();

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
                if (x->native_mp3_handling && ears_symbol_ends_with(filepath, ".mp3", true)) {
                    long startsamp = start >= 0 ? earsbufobj_time_to_samps((t_earsbufobj *)x, start,                                                                           earsbufobj_get_stored_buffer_obj((t_earsbufobj *)x, EARSBUFOBJ_OUT, 0, count + offset)) : -1;
                    long endsamp = end >= 0 ? earsbufobj_time_to_samps((t_earsbufobj *)x, end, earsbufobj_get_stored_buffer_obj((t_earsbufobj *)x, EARSBUFOBJ_OUT, 0, count + offset)) : -1;
                    ears_buffer_read_handle_mp3((t_object *)x, filepath->s_name, startsamp, endsamp, earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, count + offset));
                } else {
#endif
                    
                    if (ears_symbol_ends_with(filepath, ".wv", true)) { // WavPack files
                        long startsamp = start >= 0 ? earsbufobj_time_to_samps((t_earsbufobj *)x, start,                                                                           earsbufobj_get_stored_buffer_obj((t_earsbufobj *)x, EARSBUFOBJ_OUT, 0, count + offset)) : -1;
                        long endsamp = end >= 0 ? earsbufobj_time_to_samps((t_earsbufobj *)x, end, earsbufobj_get_stored_buffer_obj((t_earsbufobj *)x, EARSBUFOBJ_OUT, 0, count + offset)) : -1;
                        ears_buffer_read_handle_wavpack((t_object *)x, filepath->s_name, startsamp, endsamp, earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, count + offset));
                    } else {
                        // trying to load file into input buffer
                        earsbufobj_importreplace_buffer((t_earsbufobj *)x, EARSBUFOBJ_OUT, 0, count + offset, filepath);
                        
                        if (start > 0 || end > 0) {
                            double start_samps = start < 0 ? -1 : earsbufobj_time_to_samps((t_earsbufobj *)x, start, earsbufobj_get_stored_buffer_obj((t_earsbufobj *)x, EARSBUFOBJ_OUT, 0, count + offset));
                            double end_samps = end < 0 ? -1 : earsbufobj_time_to_samps((t_earsbufobj *)x, end, earsbufobj_get_stored_buffer_obj((t_earsbufobj *)x, EARSBUFOBJ_OUT, 0, count + offset));
                            ears_buffer_crop_inplace((t_object *)x, earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, count + offset), start_samps, end_samps);
                        }
                    }
                    
#ifdef EARS_FROMFILE_NATIVE_MP3_HANDLING
                }
#endif
                
                // cleaning spectral data
                ears_spectralbuf_metadata_remove((t_object *)x, earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, count + offset));
                
                // READING TAGS
                llll_appendllll(tags, buf_read_tags(x, filepath));
                
                // READING SPECTRAL METADATA AND MARKERS
                t_ears_spectralbuf_metadata data = spectralbuf_metadata_get_empty();
                bool has_data = false;
                double sr = DBL_MIN;
                llll_appendllll(markers, buf_read_markers_and_spectralannotation(x, filepath, &data, &has_data, &sr));

                if (has_data){
                    t_buffer_obj *buf = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, count + offset);
                    ears_spectralbuf_metadata_set((t_object *)x, buf, &data);
                    if (sr != DBL_MIN)
                        ears_buffer_set_sr((t_object *)x, buf, sr);
                    llll_free(data.bins);
                }
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
            earsbufobj_outlet_llll((t_earsbufobj *)x, 3, markers);
            earsbufobj_outlet_llll((t_earsbufobj *)x, 2, tags);
            
            long numsyms = x->filenames->l_size;
            t_symbol **syms = (t_symbol **)bach_newptr(numsyms * sizeof(t_symbol *));
            t_llllelem *el; long i;
            for (i = 0, el = x->filenames->l_head; i < numsyms && el; i++, el = el->l_next)
                syms[i] = (hatom_gettype(&el->l_hatom) == H_SYM) ? hatom_getsym(&el->l_hatom) : _llllobj_sym_none;
            earsbufobj_outlet_symbol_list((t_earsbufobj *)x, 1, numsyms, syms);
            bach_freeptr(syms);
            
            earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
        }
        
        llll_free(tags);
        llll_free(markers);

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
    buf_read_load(x, parsed, x->mustclear ? false : true);
    if (x->mustclear)
        x->mustclear = false;
    llll_free(parsed);
}


void buf_read_load_do(t_buf_read *x, t_symbol *msg, long ac, t_atom *av)
{
    t_llll *parsed = earsbufobj_parse_gimme((t_earsbufobj *) x, LLLL_OBJ_VANILLA, NULL, ac, av);
    if (!parsed) return;
    buf_read_load(x, parsed, false);
    llll_free(parsed);
}


void buf_read_load_deferred(t_buf_read *x, t_symbol *msg, long ac, t_atom *av)
{
    defer(x, (method)buf_read_load_do, msg, ac, av);
}

void buf_read_anything(t_buf_read *x, t_symbol *msg, long ac, t_atom *av)
{
    long inlet = earsbufobj_proxy_getinlet((t_earsbufobj *) x);
    long append = false;
    
    t_llll *parsed = earsbufobj_parse_gimme((t_earsbufobj *) x, LLLL_OBJ_VANILLA, msg, ac, av);
    if (!parsed) return;

    llll_parseargs_and_attrs((t_object *) x, parsed, "i", _sym_append, &append);

    if (inlet == 0)
        buf_read_load(x, parsed, append);

    llll_free(parsed);
}


t_llll *buf_get_tags_INFO(t_buf_read *x, TagLib::RIFF::Info::Tag *tags)
{
    t_llll *tags_ll = llll_get();
    llll_appendsym(tags_ll, gensym("INFO"));
    if (tags) {
        TagLib::RIFF::Info::FieldListMap map = tags->fieldListMap();
        for(auto it=map.begin(); it!=map.end(); ++it)
        {
            TagLib::ByteVector key = it->first;
            TagLib::String fr = it->second;
            t_llll *this_tags_ll = llll_get();
            if (strcasecmp(key.data(), "ICRD") == 0) {
                llll_appendsym(this_tags_ll, x->human_readable_frame_ids ? gensym("YEAR") : gensym(key.data()));
                llll_appendlong(this_tags_ll, tags->year());
            } else if (strcasecmp(key.data(), "IPRT") == 0) {
                llll_appendsym(this_tags_ll, x->human_readable_frame_ids ? gensym("TRACK") : gensym(key.data()));
                llll_appendlong(this_tags_ll, tags->track());
            } else if (strcasecmp(key.data(), "IPRD") == 0) {
                llll_appendsym(this_tags_ll, x->human_readable_frame_ids ? gensym("ALBUM") : gensym(key.data()));
                llll_appendsym(this_tags_ll, gensym(fr.toCString()));
            } else if (strcasecmp(key.data(), "INAM") == 0) {
                llll_appendsym(this_tags_ll, x->human_readable_frame_ids ? gensym("TITLE") : gensym(key.data()));
                llll_appendsym(this_tags_ll, gensym(fr.toCString()));
            } else if (strcasecmp(key.data(), "IART") == 0) {
                llll_appendsym(this_tags_ll, x->human_readable_frame_ids ? gensym("ARTIST") : gensym(key.data()));
                llll_appendsym(this_tags_ll, gensym(fr.toCString()));
            } else if (strcasecmp(key.data(), "IGNR") == 0) {
                llll_appendsym(this_tags_ll, x->human_readable_frame_ids ? gensym("GENRE") : gensym(key.data()));
                llll_appendsym(this_tags_ll, gensym(fr.toCString()));
            } else {
                llll_appendsym(this_tags_ll, gensym(key.data()));
                llll_appendsym(this_tags_ll, gensym(fr.toCString()));
            }
            llll_appendllll(tags_ll, this_tags_ll);
        }
        /*
        llll_appendllll(tags_ll, symbol_and_symbol_to_llll(gensym("TITLE"), gensym(tags->title().toCString())));
        llll_appendllll(tags_ll, symbol_and_symbol_to_llll(gensym("ARTIST"), gensym(tags->artist().toCString())));
        llll_appendllll(tags_ll, symbol_and_symbol_to_llll(gensym("COMMENT"), gensym(tags->comment().toCString())));
        llll_appendllll(tags_ll, symbol_and_symbol_to_llll(gensym("GENRE"), gensym(tags->genre().toCString())));
        llll_appendllll(tags_ll, symbol_and_long_to_llll(gensym("YEAR"), tags->year()));
        llll_appendllll(tags_ll, symbol_and_long_to_llll(gensym("TRACK"), tags->track()));
         */
    }
    
    return tags_ll;
}

t_llll *buf_get_tags_XIPHCOMMENT(t_buf_read *x, TagLib::Ogg::XiphComment::Tag *tags)
{
    t_llll *tags_ll = llll_get();
    llll_appendsym(tags_ll, gensym("XIPHCOMMENT"));
    if (tags) {
        llll_appendllll(tags_ll, symbol_and_symbol_to_llll(gensym("TITLE"), gensym(tags->title().toCString())));
        llll_appendllll(tags_ll, symbol_and_symbol_to_llll(gensym("ARTIST"), gensym(tags->artist().toCString())));
        llll_appendllll(tags_ll, symbol_and_symbol_to_llll(gensym("COMMENT"), gensym(tags->comment().toCString())));
        llll_appendllll(tags_ll, symbol_and_symbol_to_llll(gensym("GENRE"), gensym(tags->genre().toCString())));
        llll_appendllll(tags_ll, symbol_and_long_to_llll(gensym("YEAR"), tags->year()));
        llll_appendllll(tags_ll, symbol_and_long_to_llll(gensym("TRACK"), tags->track()));
    }
    
    return tags_ll;
}


t_llll *buf_get_tags_APE(t_buf_read *x, TagLib::APE::Tag *tags)
{
    t_llll *tags_ll = llll_get();
    llll_appendsym(tags_ll, gensym("APE"));
    if (tags) {
        TagLib::APE::ItemListMap map = tags->itemListMap();
        
        for(auto it=map.begin(); it!=map.end(); ++it)
        {
            TagLib::String key = it->first;
            TagLib::APE::Item fr = it->second;
            t_llll *this_tags_ll = llll_get();
            llll_appendsym(this_tags_ll, gensym(key.toCString()));
            llll_appendsym(this_tags_ll, gensym(fr.key().toCString()));
            llll_appendsym(this_tags_ll, gensym(fr.toString().toCString()));
            llll_appendllll(tags_ll, this_tags_ll);
        }
        
    }
    
    return tags_ll;
}


t_llll *buf_get_tags_ID3v1(t_buf_read *x, TagLib::ID3v1::Tag *tags)
{
    t_llll *tags_ll = llll_get();
    llll_appendsym(tags_ll, gensym("ID3v1"));
    
    if (tags) {
        llll_appendllll(tags_ll, symbol_and_symbol_to_llll(gensym("TITLE"), gensym(tags->title().toCString())));
        llll_appendllll(tags_ll, symbol_and_symbol_to_llll(gensym("ARTIST"), gensym(tags->artist().toCString())));
        llll_appendllll(tags_ll, symbol_and_symbol_to_llll(gensym("COMMENT"), gensym(tags->comment().toCString())));
        llll_appendllll(tags_ll, symbol_and_symbol_to_llll(gensym("GENRE"), gensym(tags->genre().toCString())));
        llll_appendllll(tags_ll, symbol_and_long_to_llll(gensym("YEAR"), tags->year()));
        llll_appendllll(tags_ll, symbol_and_long_to_llll(gensym("TRACK"), tags->track()));
    }
    
    return tags_ll;
}


t_llll *buf_get_tags_ID3v2(t_buf_read *x, TagLib::ID3v2::Tag *tags)
{
    t_llll *tags_ll = llll_get();
    llll_appendsym(tags_ll, gensym("ID3v2"));
    if(tags) {
        TagLib::ID3v2::FrameList fl = tags->frameList();
        
        TagLib::ID3v2::FrameList::ConstIterator it;
        for(it=fl.begin(); it!=fl.end(); ++it)
        {
            TagLib::ID3v2::Frame *fr = (*it);
            t_llll *this_tags_ll = llll_get();
            if (x->human_readable_frame_ids) {
                TagLib::String key = frameIDToKey(fr->frameID());
                if (key.isEmpty())
                    llll_appendsym(this_tags_ll, gensym(fr->frameID().data()));
                else
                    llll_appendsym(this_tags_ll, gensym(key.toCString()));
            } else {
                llll_appendsym(this_tags_ll, gensym(fr->frameID().data()));
            }
            
            // We handle some frames differently
            if (strcmp(fr->frameID().data(), "OWNE") == 0) {
                TagLib::ID3v2::OwnershipFrame *ufr = dynamic_cast<TagLib::ID3v2::OwnershipFrame *>(fr);
                if (ufr) {
                    llll_appendsym(this_tags_ll, gensym(ufr->pricePaid().toCString()));
                    llll_appendsym(this_tags_ll, gensym(ufr->datePurchased().toCString()));
                    llll_appendsym(this_tags_ll, gensym(ufr->seller().toCString()));
                }
            } else if (strcmp(fr->frameID().data(), "POPM") == 0) {
                TagLib::ID3v2::PopularimeterFrame *ufr = dynamic_cast<TagLib::ID3v2::PopularimeterFrame *>(fr);
                if (ufr) {
                    llll_appendsym(this_tags_ll, gensym(ufr->email().toCString()));
                    llll_appendlong(this_tags_ll, ufr->rating());
                    llll_appendlong(this_tags_ll, ufr->counter());
                }
            } else if (strcmp(fr->frameID().data(), "PRIV") == 0) {
                TagLib::ID3v2::PrivateFrame *ufr = dynamic_cast<TagLib::ID3v2::PrivateFrame *>(fr);
                if (ufr) {
                    llll_appendsym(this_tags_ll, gensym(ufr->owner().toCString()));
                    llll_appendsym(this_tags_ll, gensym(ufr->data().data()));
                }
            } else if (strcmp(fr->frameID().data(), "UFID") == 0) {
                TagLib::ID3v2::UniqueFileIdentifierFrame *ufr = dynamic_cast<TagLib::ID3v2::UniqueFileIdentifierFrame *>(fr);
                if (ufr) {
                    llll_appendsym(this_tags_ll, gensym(ufr->owner().toCString()));
                    llll_appendsym(this_tags_ll, gensym(ufr->identifier().data()));
                }
            } else if (strcmp(fr->frameID().data(), "TXXX") == 0) {
                // user text identification frame
                // We parsed this separately because the .toString() TagLib method puts the description inside brackets,
                // which may be not what one wants,
                // and flattens the list of items onto a single one
                TagLib::ID3v2::UserTextIdentificationFrame *ufr = dynamic_cast<TagLib::ID3v2::UserTextIdentificationFrame *>(fr);
                if (ufr) {
                    llll_appendsym(this_tags_ll, gensym(ufr->description().toCString()));
                    TagLib::StringList l = ufr->fieldList();
                    long count = 0;
                    for(TagLib::StringList::Iterator it = l.begin(); it != l.end(); ++it) {
                        if (count > 0) {
                            llll_appendsym(this_tags_ll, gensym((*it).toCString()));
                        }
                        count++;
                    }
                }
            } else {
                llll_appendsym(this_tags_ll, gensym(fr->toString().toCString()));
            }
            llll_appendllll(tags_ll, this_tags_ll);
        }

    }
    
    return tags_ll;
}



t_llll *buf_read_tags(t_buf_read *x, t_symbol *filename)
{
    t_llll *out = llll_get();
    
    TagLib::FileRef f(filename->s_name);
    TagLib::File *file = f.file();
    
    TagLib::MPEG::File *MPEGfile = dynamic_cast<TagLib::MPEG::File *>(file);
    if (MPEGfile) {
        llll_appendllll(out, buf_get_tags_APE(x, MPEGfile->APETag()));
        llll_appendllll(out, buf_get_tags_ID3v1(x, MPEGfile->ID3v1Tag()));
        llll_appendllll(out, buf_get_tags_ID3v2(x, MPEGfile->ID3v2Tag()));
    }

    TagLib::WavPack::File *WAVPACKfile = dynamic_cast<TagLib::WavPack::File *>(file);
    if (WAVPACKfile) {
        llll_appendllll(out, buf_get_tags_APE(x, WAVPACKfile->APETag()));
        llll_appendllll(out, buf_get_tags_ID3v1(x, WAVPACKfile->ID3v1Tag()));
    }

    TagLib::RIFF::File *RIFFfile = dynamic_cast<TagLib::RIFF::File *>(file);
    if (RIFFfile) {
        TagLib::RIFF::AIFF::File *RIFFAIFFfile = dynamic_cast<TagLib::RIFF::AIFF::File *>(file);
        if (RIFFAIFFfile) {
            llll_appendllll(out, buf_get_tags_ID3v2(x, RIFFAIFFfile->tag()));
        }

        TagLib::RIFF::WAV::File *RIFFWAVfile = dynamic_cast<TagLib::RIFF::WAV::File *>(file);
        if (RIFFWAVfile) {
            llll_appendllll(out, buf_get_tags_INFO(x, RIFFWAVfile->InfoTag()));
            llll_appendllll(out, buf_get_tags_ID3v2(x, RIFFWAVfile->ID3v2Tag()));
        }
    }
    
    TagLib::Ogg::File *OGGfile = dynamic_cast<TagLib::Ogg::File *>(file);
    if (OGGfile) {
        TagLib::Ogg::FLAC::File *OGGFLACfile = dynamic_cast<TagLib::Ogg::FLAC::File *>(file);
        if (OGGFLACfile) {
            llll_appendllll(out, buf_get_tags_XIPHCOMMENT(x, OGGFLACfile->tag()));
        }

        TagLib::Ogg::Opus::File *OGGOPUSfile = dynamic_cast<TagLib::Ogg::Opus::File *>(file);
        if (OGGOPUSfile) {
            llll_appendllll(out, buf_get_tags_XIPHCOMMENT(x, OGGOPUSfile->tag()));
        }

        TagLib::Ogg::Speex::File *OGGSPEEXfile = dynamic_cast<TagLib::Ogg::Speex::File *>(file);
        if (OGGSPEEXfile) {
            llll_appendllll(out, buf_get_tags_XIPHCOMMENT(x, OGGSPEEXfile->tag()));
        }

        TagLib::Ogg::Vorbis::File *OGGVORBISfile = dynamic_cast<TagLib::Ogg::Vorbis::File *>(file);
        if (OGGVORBISfile) {
            llll_appendllll(out, buf_get_tags_XIPHCOMMENT(x, OGGVORBISfile->tag()));
        }
    }
    
    TagLib::FLAC::File *FLACfile = dynamic_cast<TagLib::FLAC::File *>(file);
    if (FLACfile) {
        llll_appendllll(out, buf_get_tags_ID3v1(x, FLACfile->ID3v1Tag()));
        llll_appendllll(out, buf_get_tags_ID3v2(x, FLACfile->ID3v2Tag()));
    }

    TagLib::APE::File *APEfile = dynamic_cast<TagLib::APE::File *>(file);
    if (APEfile) {
        llll_appendllll(out, buf_get_tags_APE(x, APEfile->APETag()));
        llll_appendllll(out, buf_get_tags_ID3v1(x, APEfile->ID3v1Tag()));
    }
    
    return out;
}

std::vector<std::string> string_split(const char *str, char c = ' ')
{
    std::vector<std::string> result;
    
    do
    {
        const char *begin = str;
        
        while(*str != c && *str)
            str++;
        
        result.push_back(std::string(begin, str));
    } while (0 != *str++);
    
    return result;
}

void parse_annotation(const char *annotation, bool *has_data, double *sr, t_ears_spectralbuf_metadata *data)
{
    std::vector<std::string> params = string_split(annotation, ';');
    for (long i = 0; i < params.size(); i++) {
        if (params[i].rfind("earsspectral", 0) == 0)
            *has_data = std::stoi(params[i].substr(strlen("earsspectral")+1));
        else if (params[i].rfind("sr", 0) == 0)
            *sr = std::stod(params[i].substr(strlen("sr")+1));
        else if (params[i].rfind("audiosr", 0) == 0)
            data->original_audio_signal_sr = std::stod(params[i].substr(strlen("audiosr")+1));
        else if (params[i].rfind("spectype", 0) == 0)
            data->type = gensym(params[i].substr(strlen("spectype")+1).c_str());
        else if (params[i].rfind("binsize", 0) == 0)
            data->binsize = std::stod(params[i].substr(strlen("binsize")+1));
        else if (params[i].rfind("binoffset", 0) == 0)
            data->binoffset = std::stod(params[i].substr(strlen("binoffset")+1));
        else if (params[i].rfind("binunit", 0) == 0)
            data->binunit = ears_frequnit_from_symbol(gensym(params[i].substr(strlen("binunit")+1).c_str()));
        else if (params[i].rfind("bins", 0) == 0) {
            data->bins = llll_from_text_buf(params[i].substr(strlen("bins")+1).c_str());
        }
    }
}

void buf_read_llll_append_position(t_buf_read *x, t_llll *this_marker_ll, uint64_t position_samps, double sr, double num_samps)
{
    switch (x->e_ob.l_timeunit) {
        case EARS_TIMEUNIT_SAMPS:
            llll_appendlong(this_marker_ll, position_samps);
            break;
            
        case EARS_TIMEUNIT_MS:
            llll_appenddouble(this_marker_ll, ears_samps_to_ms(position_samps, sr));
            break;
            
        case EARS_TIMEUNIT_DURATION_RATIO:
            llll_appenddouble(this_marker_ll, ((double)position_samps)/num_samps);
            break;
            
        case EARS_TIMEUNIT_SECONDS:
            llll_appenddouble(this_marker_ll, ears_samps_to_ms(position_samps, sr)/1000.);
            break;
            
        default:
            break;
    }
}

t_llll *buf_read_markers_and_spectralannotation_AIFF(t_buf_read *x, t_symbol *filename, t_ears_spectralbuf_metadata *data, bool *has_data, double *sr)
{
    t_llll *out = llll_get();
    
    AIFF_Ref ref = AIFF_OpenFile(filename->s_name, F_RDONLY) ;
    if (ref)
    {
        // getting sampling rate
        uint64_t nSamples ;
        int channels ;
        double samplingRate ;
        int bitsPerSample ;
        int segmentSize ;
        
        if (AIFF_GetAudioFormat(ref,&nSamples,&channels,
                                &samplingRate,&bitsPerSample,
                                &segmentSize) < 1 ) {
            object_error((t_object *)x, "Error reading markers.");
        } else {
            
            while (true) {
                int id;
                uint64_t position;
                
                char *annotation = NULL;
                annotation = AIFF_GetAttribute(ref, AIFF_ANNO);
                if (annotation) {
                    parse_annotation(annotation, has_data, sr, data);
                    free(annotation);
                }
                
                char* name = NULL;
                if (AIFF_ReadMarker(ref, &id, &position, &name) < 1)
                    break ;
                
                t_llll *this_marker_ll = llll_get();
                buf_read_llll_append_position(x, this_marker_ll, position, samplingRate, nSamples);
                if (name)
                    llll_appendsym(this_marker_ll, gensym(name));

                llll_appendllll(out, this_marker_ll);
                
                if (name)
                    free(name);
            }
        }
        AIFF_CloseFile(ref);
    }
    
    return out;
}

t_llll *AudioCues_to_llll(t_buf_read *x, AudioFile<double> &audioFile)
{
    t_llll *out = llll_get();
    std::vector<AudioCue> cues = audioFile.getCues();
    for (long i = 0; i < cues.size(); i++) {
        t_llll *this_marker_ll = llll_get();
        if (cues[i].isRegion) {
            t_llll *sub_ll = llll_get();
            llll_appendsym(sub_ll, gensym("region")); // as in bach's markers
            buf_read_llll_append_position(x, sub_ll, cues[i].sampleStart, audioFile.getSampleRate(), audioFile.getNumSamplesPerChannel());
            buf_read_llll_append_position(x, sub_ll, cues[i].sampleLength, audioFile.getSampleRate(), audioFile.getNumSamplesPerChannel());
        } else {
            buf_read_llll_append_position(x, this_marker_ll, cues[i].sampleStart, audioFile.getSampleRate(), audioFile.getNumSamplesPerChannel());
        }
        if (i < cues[i].label.size() > 0)
            llll_appendsym(this_marker_ll, gensym(cues[i].label.c_str()));
        llll_appendllll(out, this_marker_ll);
    }
    return out;
}

t_llll *buf_read_markers_and_spectralannotation(t_buf_read *x, t_symbol *filename, t_ears_spectralbuf_metadata *data, bool *has_data, double *sr)
{
    t_llll *out = NULL;
    *has_data = false;
    
    if (ears_symbol_ends_with(filename, ".aif", true) || ears_symbol_ends_with(filename, ".aiff", true)) {
        // spectral information is embedded in the ANNOTATION
        out = buf_read_markers_and_spectralannotation_AIFF(x, filename, data, has_data, sr);
    } else if (ears_symbol_ends_with(filename, ".wav", true) || ears_symbol_ends_with(filename, ".wave", true)){
        // Spectral information is embedded in the INFO tag
        if (data) {
            TagLib::FileRef f(filename->s_name);
            TagLib::File *file = f.file();
            if (file) {
                TagLib::RIFF::File *RIFFfile = dynamic_cast<TagLib::RIFF::File *>(file);
                if (RIFFfile) {
                    TagLib::RIFF::WAV::File *RIFFWAVfile = dynamic_cast<TagLib::RIFF::WAV::File *>(file);
                    if (RIFFWAVfile) {
                        TagLib::String s = RIFFWAVfile->InfoTag()->fieldText(TagLib::ByteVector("EARS"));
                        if (strlen(s.toCString()) > 0) {
                            parse_annotation(s.toCString(), has_data, sr, data);
                        }
                    }
                }
            }
        }
        
        // MARKERS:
        AudioFile<double> audioFile;
        audioFile.load(filename->s_name);
        out = AudioCues_to_llll(x, audioFile);
        
    } else if (ears_symbol_ends_with(filename, ".wv", true)){
        
        // Spectral information is in the APE tag
        if (data) {
            TagLib::FileRef f(filename->s_name);
            TagLib::File *file = f.file();
            if (file) {
                TagLib::WavPack::File *WVfile = dynamic_cast<TagLib::WavPack::File *>(file);
                if (WVfile) {
                    TagLib::APE::ItemListMap map = WVfile->APETag()->itemListMap();
                    for(auto it=map.begin(); it!=map.end(); ++it)
                    {
                        TagLib::String key = it->first;
                        TagLib::APE::Item fr = it->second;
                        if (strcmp(key.toCString(), "EARS") == 0) {
                            parse_annotation(fr.toString().toCString(), has_data, sr, data);
                            break;
                        }
                    }
                }
            }
        }

        out = llll_get();

    } else {
        out = llll_get();
    }
    
    return out;
}




