/**
	@file
	ears.readtags.c
 
	@name 
	ears.readtags
 
	@realname
	ears.readtags
 
	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest 
	Get metadata tags
 
	@description
	Reads id3 and id3v2 metadata tags from mp3 files
 
	@discussion
    The source code uses the id3lib library to handle mp3 metadata.
 
	@category
	ears metadata
 
	@keywords
	id3, tag, id3v2, id3tool, metadata, mp3, read, open
 
	@seealso
	ears.writetags
	
	@owner
	Daniele Ghisi
*/

#include "ext.h"
#include "ext_obex.h"
#include "llllobj.h"
#include "llll_commons_ext.h"
#include "bach_math_utilities.h"

#include "ears.commons.h"
#include "ears.utils.h"

#include "id3.h"
#include "id3add.h" // includes all the remaining id3 stuff
#include "genre.h"


typedef struct _readtags {
    t_llllobj_object	d_obj;
    
    char                add_descriptions;
    char                add_version;
    
    t_systhread_mutex   c_mutex;
    
    char       output_id3v1_tags; // ID3TT_ID3V1
    char       output_id3v2_tags; // ID3TT_ID3V2
    char       output_lyrics3_tags; // ID3TT_LYRICS3
    char       output_lyrics3v2_tags; // ID3TT_LYRICS3V2
    char       output_musicmatch_tags; // ID3TT_MUSICMATCH
    
    char        ufid_are_numbers; // treat unique file ids "UFID" fields as containing number (this will not fill up Max symbol table)

    char                verbose_errors;
    
} t_readtags;



// Prototypes
t_readtags*      readtags_new(t_symbol *s, short argc, t_atom *argv);
void			readtags_free(t_readtags *x);
void			readtags_bang(t_readtags *x);
void			readtags_anything(t_readtags *x, t_symbol *msg, long ac, t_atom *av);
void			readtags_int(t_readtags *x, t_atom_long num);

void readtags_assist(t_readtags *x, void *b, long m, long a, char *s);
void readtags_inletinfo(t_readtags *x, void *b, long a, char *t);


t_llll *tag_to_llll(t_readtags *x, const ID3_Tag &myTag, const char *filename);
t_llll *filename_to_llll_tags(t_readtags *x, t_symbol *filename);
t_llll *filename_to_llll_mp3info(t_readtags *x, t_symbol *filename);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;

/**********************************************************************/
// Class Definition and Life Cycle

int C74_EXPORT main(void)
{
	common_symbols_init();
	llllobj_common_symbols_init();

    if (llllobj_check_version(bach_get_current_llll_version()) || llllobj_test()) {
        if (!gensym("bach")->s_thing) {
            error("error: ears needs bach to be installed in order to work (www.bachproject.net).");
        } else {
            error("error: your bach version (%s) is not supported by this ears version.", bach_get_current_version_string_verbose());
            error("   to fix this, please upgrade both bach and ears to their latest version.");
        }
        return 1;
    }
	
	t_class *c;
	
	CLASS_NEW_CHECK_SIZE(c, "ears.readtags",
				  (method)readtags_new,
				  (method)readtags_free,
				  sizeof(t_readtags),
				  (method)NULL,
				  A_GIMME,
				  0L);
	
    // @method symbol/list/llll @digest Retrieve MP3 tags
    // @description A symbol, a list or an llll is treated as a filename or a collection of filenames and
    // triggers the retrieval of the MP3 tags for the files
	class_addmethod(c, (method)readtags_int,				"int",			A_LONG, 0);
	class_addmethod(c, (method)readtags_anything,			"anything",			A_GIMME, 0);
	class_addmethod(c, (method)readtags_anything,			"list",			A_GIMME, 0);
    class_addmethod(c, (method)readtags_bang,			"bang",	 0);

    class_addmethod(c, (method)readtags_assist,					"assist",				A_CANT,		0);
    class_addmethod(c, (method)readtags_inletinfo,					"inletinfo",			A_CANT,		0);

    llllobj_class_add_out_attr(c, LLLL_OBJ_VANILLA);
    
	CLASS_STICKY_ATTR(c,"category",0,"Settings");

    CLASS_ATTR_CHAR(c, "version",	0,	t_readtags, add_version);
    CLASS_ATTR_STYLE_LABEL(c, "version", 0, "onoff", "Output ID3 Field Version");
    // @description When set to 1, also outputs tag field version as integer number (1 for ID3V1, 2 for ID3V2)
    
    CLASS_ATTR_CHAR(c, "description",	0,	t_readtags, add_descriptions);
    CLASS_ATTR_STYLE_LABEL(c, "description", 0, "onoff", "Output Field Descriptions");
    // @description When set to 1, also outputs tag field descriptions for ID3V2 tags.

    CLASS_ATTR_CHAR(c, "id3v1",	0,	t_readtags, output_id3v1_tags);
    CLASS_ATTR_STYLE_LABEL(c, "id3v1", 0, "onoff", "Output ID3V1 tags");
    // @description When set to 1, outputs ID3V1 tags.

    CLASS_ATTR_CHAR(c, "id3v2",	0,	t_readtags, output_id3v2_tags);
    CLASS_ATTR_STYLE_LABEL(c, "id3v2", 0, "onoff", "Output ID3V2 tags");
    // @description When set to 1, outputs ID3V2 tags.

    CLASS_ATTR_CHAR(c, "verbose",	0,	t_readtags, verbose_errors);
    CLASS_ATTR_STYLE_LABEL(c, "verbose", 0, "onoff", "Verbose Errors");
    // @description When set to 1, outputs errors in the Max window.

    CLASS_ATTR_CHAR(c, "numericufid",	0,	t_readtags, ufid_are_numbers);
    CLASS_ATTR_STYLE_LABEL(c, "numericufid", 0, "onoff", "Treat 'UFID' fields as numbers");
    // @description When set to 1, treats unique file ID fields ('UFID') as numeric. This will prevent
    // filling up Max symbol table if a lot of IDs are used.

    
    
/*
    CLASS_ATTR_CHAR(c, "lyrics3",	0,	t_readtags, output_lyrics3_tags);
    CLASS_ATTR_STYLE_LABEL(c, "lyrics3", 0, "onoff", "Output Lyrics3 tags");
    // @description When set to 1, outputs Lyrics3 tags.
    // @exclude ears.readtags

    CLASS_ATTR_CHAR(c, "lyrics3v2",	0,	t_readtags, output_lyrics3v2_tags);
    CLASS_ATTR_STYLE_LABEL(c, "lyrics3v2", 0, "onoff", "Output Lyrics3V2 tags");
    // @description When set to 1, outputs Lyrics3V2 tags.
    // @exclude ears.readtags

    CLASS_ATTR_CHAR(c, "musicmatch",	0,	t_readtags, output_musicmatch_tags);
    CLASS_ATTR_STYLE_LABEL(c, "musicmatch", 0, "onoff", "Output MusicMatch tags");
    // @description When set to 1, outputs MusicMatch tags.
    // @exclude ears.readtags
*/
    
    CLASS_STICKY_ATTR_CLEAR(c, "category");

	

	class_register(CLASS_BOX, c);
	s_tag_class = c;
	ps_event = gensym("event");
	return 0;
}

void readtags_assist(t_readtags *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        // @in 0 @type symbol/llll @digest Filename(s)
        sprintf(s, "llll: Filename(s)");
    } else {
        char *type = NULL;
        llllobj_get_llll_outlet_type_as_string((t_object *) x, LLLL_OBJ_VANILLA, a, &type);
        if (a == 0) // @out 0 @type llll @digest Tags
            sprintf(s, "llll (%s): Tags", type); // @description Outputs metadata tags
        else // @out 1 @type bang @digest MP3 Info
            sprintf(s, "llll (%s): Mp3 Information", type); // @description Outputs MP3 information
    }
}

void readtags_inletinfo(t_readtags *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_readtags* readtags_new(t_symbol *s, short argc, t_atom *argv)
{
	t_readtags* x;
	
//	long true_ac = attr_args_offset(argc, argv);

	x = (t_readtags*)object_alloc_debug(s_tag_class);
	if (x) {
		
        x->output_id3v1_tags = true;
        x->output_id3v2_tags = true;
        x->output_lyrics3_tags = true;
        x->output_lyrics3v2_tags = true;
        x->output_musicmatch_tags = true;
        x->verbose_errors = true;
        
		llllobj_obj_setup((t_llllobj_object *)x, 1, "44");
		
        attr_args_process(x, argc, argv);

		systhread_mutex_new_debug(&x->c_mutex, 0);
	}
	return x;
}


void readtags_free(t_readtags *x)
{
	systhread_mutex_free_debug(x->c_mutex);
}


void readtags_int(t_readtags *x, t_atom_long num)
{
	t_atom argv[1]; 
	atom_setlong(argv, num);
	readtags_anything(x, _sym_list, 1, argv);
}


void readtags_bang(t_readtags *x)
{
    readtags_anything(x, _sym_bang, 0, NULL);
}



long get_tag_for_file_fn(void *data, t_hatom *a, const t_llll *address)
{
    t_readtags *x = (t_readtags *) data;
    t_llll *ll = filename_to_llll_tags(x, hatom_getsym(a));
    hatom_change_to_llll_and_free(a, ll);
    return 0;
}


long get_mp3info_for_file_fn(void *data, t_hatom *a, const t_llll *address)
{
    t_readtags *x = (t_readtags *) data;
    t_llll *ll = filename_to_llll_mp3info(x, hatom_getsym(a));
    hatom_change_to_llll_and_free(a, ll);
    return 0;
}


void readtags_anything(t_readtags *x, t_symbol *msg, long ac, t_atom *av)
{
    t_llll *parsed = llllobj_parse_llll((t_object *) x, LLLL_OBJ_VANILLA, msg, ac, av, LLLL_PARSE_CLONE);
    if (!parsed) return;
    
    // get tags for incoming file(s)
    t_llll *mp3info = llll_clone(parsed);
    llll_funall(parsed, (fun_fn) get_tag_for_file_fn, x, 1, -1, FUNALL_ONLY_PROCESS_ATOMS);
    llll_funall(mp3info, (fun_fn) get_mp3info_for_file_fn, x, 1, -1, FUNALL_ONLY_PROCESS_ATOMS);
    
    llllobj_outlet_llll((t_object *)x, LLLL_OBJ_VANILLA, 1, mp3info);
    llllobj_outlet_llll((t_object *)x, LLLL_OBJ_VANILLA, 0, parsed);

    llll_free(mp3info);
    llll_free(parsed);
}




///// LLLL WRAPPERS


t_llll *filename_to_llll_mp3info(t_readtags *x, t_symbol *filename)
{
    filename = ears_ezlocate_file(filename, NULL);

    if (!filename)
        return llll_get();
    
    ID3_Tag myTag;
    flags_t which = 0;
    if (x->output_id3v1_tags) which += ID3TT_ID3V1;
    if (x->output_id3v2_tags) which += ID3TT_ID3V2;
    if (x->output_lyrics3_tags) which += ID3TT_LYRICS3;
    if (x->output_lyrics3v2_tags) which += ID3TT_LYRICS3V2;
    if (x->output_musicmatch_tags) which += ID3TT_MUSICMATCH;

    myTag.Link(filename->s_name, which); // ID3TT_NONE will suffice???
    t_llll *mp3info_ll = llll_get();
    
    const Mp3_Headerinfo* mp3info;
    mp3info = myTag.GetMp3HeaderInfo();
    
    if (mp3info) {
        //            cout << "*** mp3 info\n";
        
        t_llll *version_ll = llll_get();
        llll_appendsym(version_ll, gensym("version"));
        switch (mp3info->version)
        {
            case MPEGVERSION_2_5:
                llll_appendsym(version_ll, gensym("MPEG2.5"));
                //                    cout << "MPEG2.5/";
                break;
            case MPEGVERSION_2:
                llll_appendsym(version_ll, gensym("MPEG2"));
                //                    cout << "MPEG2/";
                break;
            case MPEGVERSION_1:
                llll_appendsym(version_ll, gensym("MPEG1"));
                //                    cout << "MPEG1/";
                break;
            default:
                break;
        }
        
        t_llll *layer_ll = llll_get();
        llll_appendsym(layer_ll, gensym("layer"));
        switch (mp3info->layer)
        {
            case MPEGLAYER_III:
                llll_appendlong(layer_ll, 3);
                //                    cout << "layer III\n";
                break;
            case MPEGLAYER_II:
                llll_appendlong(layer_ll, 2);
                //                    cout << "layer II\n";
                break;
            case MPEGLAYER_I:
                llll_appendlong(layer_ll, 1);
                //                    cout << "layer I\n";
                break;
            default:
                break;
        }
        
        
        llll_appendllll(mp3info_ll, version_ll);
        llll_appendllll(mp3info_ll, layer_ll);
        
        llll_appendllll(mp3info_ll, symbol_and_long_to_llll(gensym("bitrate"), mp3info->bitrate)); // in bps
        llll_appendllll(mp3info_ll, symbol_and_long_to_llll(gensym("samplerate"), mp3info->frequency)); // in Hz
        llll_appendllll(mp3info_ll, symbol_and_long_to_llll(gensym("numframes"), mp3info->frames));
        llll_appendllll(mp3info_ll, symbol_and_long_to_llll(gensym("time"), mp3info->time));
        
    }
    
    return mp3info_ll;
}

t_llll *filename_to_llll_tags(t_readtags *x, t_symbol *filename)
{
    filename = ears_ezlocate_file(filename, NULL);
    
    if (!filename)
        return llll_get();
    
    ID3_Tag myTag;
    flags_t which = 0;
    //    if (x->output_id3v1_tags) which += ID3TT_ID3V1; // NO: will be handled separately in tag_to_llll()
    if (x->output_id3v2_tags) which += ID3TT_ID3V2;
    if (x->output_lyrics3_tags) which += ID3TT_LYRICS3;
    if (x->output_lyrics3v2_tags) which += ID3TT_LYRICS3V2;
    if (x->output_musicmatch_tags) which += ID3TT_MUSICMATCH;
    
    myTag.Link(filename->s_name, which);
    
    return tag_to_llll(x, myTag, filename->s_name);

}


t_symbol *gensym_n(const char *buf, long num_chars)
{
    char *temp = bach_newptr((num_chars + 1) * sizeof(char));
    snprintf(temp, num_chars, "%s", buf);
    temp[num_chars] = 0;
    t_symbol *out = gensym(temp);
    bach_freeptr(temp);
    return out;
}

t_llll *tag_to_llll(t_readtags *x, const ID3_Tag &myTag, const char *filename)
{
    t_llll *ll = llll_get();
    
    if (x->output_id3v1_tags) {
        id3tag_t myv1tag;
        FILE *fp;
        fp = fopen(filename, "rb");
        if (NULL == fp) {
            object_error((t_object *)x, "Can't open file %s for ID3V1 tags read.", filename);
            goto id3v2;
        }
        
        /* This simple detection code has a 1 in 16777216
         * chance of misrecognizing or deleting the last 128
         * bytes of your mp3 if it isn't tagged. ID3 ain't
         * world peace, live with it.
         */
        if (!id3_readtag(fp, &myv1tag)) {
            //            std::cout << "id3v1 tag info for " << sFileName << ":" << std::endl;
            t_llll *title_ll = llll_get(), *artist_ll = llll_get(), *album_ll = llll_get(), *year_ll = llll_get(), *genre_ll = llll_get(), *comment_ll = llll_get();
            
            // title
            llll_appendsym(title_ll, gensym("title"));
            if (x->add_version) llll_appendlong(title_ll, 1);
            if (x->add_descriptions) llll_appendsym(title_ll, gensym("Title"));
            llll_appendsym(title_ll, gensym_n(myv1tag.songname, 30));

            llll_appendsym(artist_ll, gensym("artist"));
            if (x->add_version) llll_appendlong(artist_ll, 1);
            if (x->add_descriptions) llll_appendsym(artist_ll, gensym("Artist"));
            llll_appendsym(artist_ll, gensym_n(myv1tag.artist, 30));

            llll_appendsym(album_ll, gensym("album"));
            if (x->add_version) llll_appendlong(album_ll, 1);
            if (x->add_descriptions) llll_appendsym(album_ll, gensym("Album"));
            llll_appendsym(album_ll, gensym_n(myv1tag.album, 30));

            llll_appendsym(year_ll, gensym("year"));
            if (x->add_version) llll_appendlong(year_ll, 1);
            if (x->add_descriptions) llll_appendsym(year_ll, gensym("Year"));
            llll_appendlong(year_ll, atol(myv1tag.year));

            llll_appendllll(ll, title_ll);
            llll_appendllll(ll, artist_ll);
            llll_appendllll(ll, album_ll);
            llll_appendllll(ll, year_ll);
            
            llll_appendsym(genre_ll, gensym("genre"));
            if (x->add_version) llll_appendlong(genre_ll, 1);
            if (x->add_descriptions) llll_appendsym(genre_ll, gensym("Genre"));
            llll_appendlong(genre_ll, myv1tag.style);
            llll_appendsym(genre_ll, myv1tag.style < GetGenreCount() ? gensym(GetGenreFromNum(myv1tag.style)) : gensym("Unknown"));

            llll_appendllll(ll, genre_ll);

            char		strbuf[31];
            if (myv1tag.note.v11.marker == '\0') {
                /* use v1.1 symantics */
                if (myv1tag.note.v11.note[0] != '\0') {
                    strncpy(strbuf, myv1tag.note.v11.note, 28);
                    strbuf[28] = '\0';
                    llll_appendsym(comment_ll, gensym("comment"));
                    if (x->add_version) llll_appendlong(comment_ll, 1);
                    if (x->add_descriptions) llll_appendsym(comment_ll, gensym("Comment"));
                    llll_appendsym(comment_ll, gensym(strbuf));
                }
                llll_appendllll(ll, comment_ll);
                if (myv1tag.note.v11.track != 0) {
                    llll_appendllll(ll, symbol_and_long_to_llll(gensym("track"), myv1tag.note.v11.track));
                }
            } else {
                if (myv1tag.note.v10.note[0] != '\0') {
                    strncpy(strbuf, myv1tag.note.v10.note, 30);
                    strbuf[30] = '\0';
                    llll_appendsym(comment_ll, gensym("comment"));
                    if (x->add_version) llll_appendlong(comment_ll, 1);
                    if (x->add_descriptions) llll_appendsym(comment_ll, gensym("Comment"));
                    llll_appendsym(comment_ll, gensym(strbuf));
                }
                llll_appendllll(ll, comment_ll);
            }
        }
        fclose(fp);
    }

id3v2:

    if (x->output_id3v2_tags) {
        ID3_Tag::ConstIterator* iter = myTag.CreateIterator();
        const ID3_Frame* frame = NULL;
        
        while (NULL != (frame = iter->GetNext()))
        {
            t_llll *subll = llll_get();
            const char* desc = frame->GetDescription();
            if (!desc) desc = "";
            

            llll_appendsym(subll, gensym(frame->GetTextID()));
            if (x->add_version)
                llll_appendlong(subll, 2);
            if (x->add_descriptions)
                llll_appendsym(subll, gensym(desc));
            //        cout << "=== " << frame->GetTextID() << " (" << desc << "): ";
            ID3_FrameID eFrameID = frame->GetID();
            switch (eFrameID)
            {
                case ID3FID_ALBUM:
                case ID3FID_COMPOSER:
                case ID3FID_CONTENTTYPE:
                case ID3FID_COPYRIGHT:
                case ID3FID_DATE:
                case ID3FID_PLAYLISTDELAY:
                case ID3FID_ENCODEDBY:
                case ID3FID_LYRICIST:
                case ID3FID_FILETYPE:
                case ID3FID_TIME:
                case ID3FID_CONTENTGROUP:
                case ID3FID_TITLE:
                case ID3FID_SUBTITLE:
                case ID3FID_INITIALKEY:
                case ID3FID_LANGUAGE:
                case ID3FID_SONGLEN:
                case ID3FID_MEDIATYPE:
                case ID3FID_ORIGALBUM:
                case ID3FID_ORIGFILENAME:
                case ID3FID_ORIGLYRICIST:
                case ID3FID_ORIGARTIST:
                case ID3FID_FILEOWNER:
                case ID3FID_LEADARTIST:
                case ID3FID_BAND:
                case ID3FID_CONDUCTOR:
                case ID3FID_MIXARTIST:
                case ID3FID_PARTINSET:
                case ID3FID_PUBLISHER:
                case ID3FID_TRACKNUM:
                case ID3FID_RECORDINGDATES:
                case ID3FID_NETRADIOSTATION:
                case ID3FID_NETRADIOOWNER:
                case ID3FID_SIZE:
                case ID3FID_ISRC:
                case ID3FID_ENCODERSETTINGS:
                {
                    char *sText = ID3_GetString(frame, ID3FN_TEXT);
                    llll_appendsym(subll, gensym(sText));
                    //                cout << sText << endl;
                    delete [] sText;
                    break;
                }
                case ID3FID_BPM:
                case ID3FID_ORIGYEAR:
                case ID3FID_YEAR:
                {
                    char *sText = ID3_GetString(frame, ID3FN_TEXT);
                    llll_appendlong(subll, atol(sText));
                    //                cout << sText << endl;
                    delete [] sText;
                    break;
                }
                case ID3FID_USERTEXT:
                {
                    char
                    *sText = ID3_GetString(frame, ID3FN_TEXT),
                    *sDesc = ID3_GetString(frame, ID3FN_DESCRIPTION);
                    llll_appendsym(subll, gensym(sText));
                    llll_appendsym(subll, gensym(sDesc));
                    //                cout << "(" << sDesc << "): " << sText << endl;
                    delete [] sText;
                    delete [] sDesc;
                    break;
                }
                case ID3FID_COMMENT:
                case ID3FID_UNSYNCEDLYRICS:
                {
                    char
                    *sText = ID3_GetString(frame, ID3FN_TEXT),
                    *sDesc = ID3_GetString(frame, ID3FN_DESCRIPTION),
                    *sLang = ID3_GetString(frame, ID3FN_LANGUAGE);
                    llll_appendsym(subll, gensym(sText));
                    llll_appendsym(subll, gensym(sDesc));
                    llll_appendsym(subll, gensym(sLang));
                    //                cout << "(" << sDesc << ")[" << sLang << "]: "  << sText << endl;
                    delete [] sText;
                    delete [] sDesc;
                    delete [] sLang;
                    break;
                }
                case ID3FID_WWWAUDIOFILE:
                case ID3FID_WWWARTIST:
                case ID3FID_WWWAUDIOSOURCE:
                case ID3FID_WWWCOMMERCIALINFO:
                case ID3FID_WWWCOPYRIGHT:
                case ID3FID_WWWPUBLISHER:
                case ID3FID_WWWPAYMENT:
                case ID3FID_WWWRADIOPAGE:
                {
                    char *sURL = ID3_GetString(frame, ID3FN_URL);
                    llll_appendsym(subll, gensym(sURL));
                    //                cout << sURL << endl;
                    delete [] sURL;
                    break;
                }
                case ID3FID_WWWUSER:
                {
                    char
                    *sURL = ID3_GetString(frame, ID3FN_URL),
                    *sDesc = ID3_GetString(frame, ID3FN_DESCRIPTION);
                    llll_appendsym(subll, gensym(sURL));
                    llll_appendsym(subll, gensym(sDesc));
                    //                cout << "(" << sDesc << "): " << sURL << endl;
                    delete [] sURL;
                    delete [] sDesc;
                    break;
                }
                case ID3FID_INVOLVEDPEOPLE:
                {
                    size_t nItems = frame->GetField(ID3FN_TEXT)->GetNumTextItems();
                    for (size_t nIndex = 0; nIndex < nItems; nIndex++)
                    {
                        char *sPeople = ID3_GetString(frame, ID3FN_TEXT, nIndex);
                        llll_appendsym(subll, gensym(sPeople));
                        //                    cout << sPeople;
                        delete [] sPeople;
                        //                    if (nIndex + 1 < nItems)
                        //                        cout << ", ";
                    }
                    //                cout << endl;
                    break;
                }
                case ID3FID_PICTURE:
                {
                    char
                    *sMimeType = ID3_GetString(frame, ID3FN_MIMETYPE),
                    *sDesc     = ID3_GetString(frame, ID3FN_DESCRIPTION),
                    *sFormat   = ID3_GetString(frame, ID3FN_IMAGEFORMAT);
                    size_t
                    nPicType   = frame->GetField(ID3FN_PICTURETYPE)->Get(),
                    nDataSize  = frame->GetField(ID3FN_DATA)->Size();
                    //                cout << "(" << sDesc << ")[" << sFormat << ", " << nPicType << "]: " << sMimeType << ", " << nDataSize<< " bytes" << endl;
                    llll_appendsym(subll, gensym(sMimeType));
                    llll_appendsym(subll, gensym(sDesc));
                    llll_appendsym(subll, gensym(sFormat));
                    delete [] sMimeType;
                    delete [] sDesc;
                    delete [] sFormat;
                    break;
                }
                case ID3FID_GENERALOBJECT:
                {
                    char
                    *sMimeType = ID3_GetString(frame, ID3FN_MIMETYPE),
                    *sDesc = ID3_GetString(frame, ID3FN_DESCRIPTION),
                    *sFileName = ID3_GetString(frame, ID3FN_FILENAME);
                    size_t
                    nDataSize = frame->GetField(ID3FN_DATA)->Size();
                    llll_appendsym(subll, gensym(sMimeType));
                    llll_appendsym(subll, gensym(sDesc));
                    llll_appendsym(subll, gensym(sFileName));
                    //                cout << "(" << sDesc << ")[" << sFileName << "]: " << sMimeType << ", " << nDataSize << " bytes" << endl;
                    delete [] sMimeType;
                    delete [] sDesc;
                    delete [] sFileName;
                    break;
                }
                case ID3FID_UNIQUEFILEID:
                {
                    char *sOwner = ID3_GetString(frame, ID3FN_OWNER);
                    size_t nDataSize = frame->GetField(ID3FN_DATA)->Size();
                    const uchar *sRawData = frame->GetField(ID3FN_DATA)->GetRawBinary();
                    
                    llll_appendsym(subll, gensym(sOwner));
                    
                    if (x->ufid_are_numbers) {
                        t_uint64 multichars_val = 0;
                        long i;
                        for (i = 0; i < nDataSize; i++)
                            multichars_val |= (sRawData[i] << (8 * i));
                        llll_appendlong(subll, multichars_val);
                    } else
                        llll_appendsym(subll, gensym((const char *)sRawData));
                    
//                    llll_appendlong(subll, nDataSize);
                    
                    //                cout << sOwner << ", " << nDataSize << " bytes" << endl;
                    delete [] sOwner;
                    break;
                }
                case ID3FID_PRIVATE:
                {
                    char *sOwner = ID3_GetString(frame, ID3FN_OWNER);
                    size_t nDataSize = frame->GetField(ID3FN_DATA)->Size();
                    const uchar *sRawData = frame->GetField(ID3FN_DATA)->GetRawBinary();
                    
                    llll_appendsym(subll, gensym(sOwner));
                    
                    llll_appendsym(subll, gensym((const char *)sRawData));
                    
//                    llll_appendlong(subll, nDataSize);
                    
                    //                cout << sOwner << ", " << nDataSize << " bytes" << endl;
                    delete [] sOwner;
                    break;
                }
                case ID3FID_PLAYCOUNTER:
                {
                    size_t nCounter = frame->GetField(ID3FN_COUNTER)->Get();
                    llll_appendlong(subll, nCounter);
                    //                cout << nCounter << endl;
                    break;
                }
                case ID3FID_POPULARIMETER:
                {
                    char *sEmail = ID3_GetString(frame, ID3FN_EMAIL);
                    size_t
                    nCounter = frame->GetField(ID3FN_COUNTER)->Get(),
                    nRating = frame->GetField(ID3FN_RATING)->Get();
                    llll_appendsym(subll, gensym(sEmail));
                    llll_appendlong(subll, nCounter);
                    llll_appendlong(subll, nRating);
                    //                cout << sEmail << ", counter=" << nCounter << " rating=" << nRating << endl;
                    delete [] sEmail;
                    break;
                }
                case ID3FID_CRYPTOREG:
                case ID3FID_GROUPINGREG:
                {
                    char *sOwner = ID3_GetString(frame, ID3FN_OWNER);
                    size_t
                    nSymbol = frame->GetField(ID3FN_ID)->Get(),
                    nDataSize = frame->GetField(ID3FN_DATA)->Size();
                    llll_appendsym(subll, gensym(sOwner));
                    llll_appendlong(subll, nSymbol);
                    llll_appendlong(subll, nDataSize);
                    //                cout << "(" << nSymbol << "): " << sOwner << ", " << nDataSize << " bytes" << endl;
                    break;
                }
                case ID3FID_SYNCEDLYRICS:
                {
                    // TO DO
                    /*                char
                     *sDesc = ID3_GetString(frame, ID3FN_DESCRIPTION),
                     *sLang = ID3_GetString(frame, ID3FN_LANGUAGE);
                     size_t
                     nTimestamp = frame->GetField(ID3FN_TIMESTAMPFORMAT)->Get(),
                     nRating = frame->GetField(ID3FN_CONTENTTYPE)->Get();
                     const char* format = (2 == nTimestamp) ? "ms" : "frames";
                     cout << "(" << sDesc << ")[" << sLang << "]: ";
                     switch (nRating)
                     {
                     case ID3CT_OTHER:    cout << "Other"; break;
                     case ID3CT_LYRICS:   cout << "Lyrics"; break;
                     case ID3CT_TEXTTRANSCRIPTION:     cout << "Text transcription"; break;
                     case ID3CT_MOVEMENT: cout << "Movement/part name"; break;
                     case ID3CT_EVENTS:   cout << "Events"; break;
                     case ID3CT_CHORD:    cout << "Chord"; break;
                     case ID3CT_TRIVIA:   cout << "Trivia/'pop up' information"; break;
                     }
                     cout << endl;
                     ID3_Field* fld = frame->GetField(ID3FN_DATA);
                     if (fld)
                     {
                     ID3_MemoryReader mr(fld->GetRawBinary(), fld->BinSize());
                     while (!mr.atEnd())
                     {
                     cout << io::readString(mr).c_str();
                     cout << " [" << io::readBENumber(mr, sizeof(uint32)) << " " 
                     << format << "] ";
                     }
                     }
                     cout << endl;
                     delete [] sDesc;
                     delete [] sLang; */
                    break;
                }
                case ID3FID_AUDIOCRYPTO:
                case ID3FID_EQUALIZATION:
                case ID3FID_EVENTTIMING:
                case ID3FID_CDID:
                case ID3FID_MPEGLOOKUP:
                case ID3FID_OWNERSHIP:
                case ID3FID_POSITIONSYNC:
                case ID3FID_BUFFERSIZE:
                case ID3FID_VOLUMEADJ:
                case ID3FID_REVERB:
                case ID3FID_SYNCEDTEMPO:
                case ID3FID_METACRYPTO:
                {
                    llll_appendsym(subll, gensym("unimplemented"));
                    //                cout << " (unimplemented)" << endl;
                    break;
                }
                default:
                {
                    llll_appendsym(subll, gensym("frame"));
                    //                cout << " frame" << endl;
                    break;
                }
            }
            llll_appendllll(ll, subll);
        }
        delete iter;
        
    }
    
    return ll;
}



