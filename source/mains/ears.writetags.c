/**
	@file
	ears.writetags.c
 
	@name 
	ears.writetags
 
	@realname
	ears.writetags
 
	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest 
	Set metadata tags
 
	@description
	Writes id3 and id3v2 metadata tags to mp3 files
 
	@discussion
    The source code uses the id3lib library to handle mp3 metadata.
 
	@category
	ears metadata
 
	@keywords
	id3, tag, id3v2, id3tool, metadata, mp3, set, write
 
	@seealso
	
	
	@owner
	Daniele Ghisi
*/

#include "ext.h"
#include "ext_obex.h"
#include "llllobj.h"
#include "llll_commons_ext.h"
#include "bach_math_utilities.h"

#include "id3add.h" // includes all the remaining id3 stuff
#include "id3.h"

typedef struct _writetags {
    t_llllobj_object	d_obj;
    
    char                add_descriptions;
    char                verbose_errors;
    
    char                set_id3v1_tags;
    char                set_id3v2_tags;
    
    char                sync_v1_v2_tags;
    char                protect;
    
    t_llll              *curr_filenames;
    t_systhread_mutex   c_mutex;
    
    char        ufid_are_numbers; // treat unique file ids "UFID" fields as containing number (this will not fill up Max symbol table)

    
    void				*n_proxy[3];
    long				n_in;
} t_writetags;



// Prototypes
t_writetags*      writetags_new(t_symbol *s, short argc, t_atom *argv);
void			writetags_free(t_writetags *x);
void			writetags_bang(t_writetags *x);
void			writetags_anything(t_writetags *x, t_symbol *msg, long ac, t_atom *av);
void			writetags_int(t_writetags *x, t_atom_long num);

void writetags_assist(t_writetags *x, void *b, long m, long a, char *s);
void writetags_inletinfo(t_writetags *x, void *b, long a, char *t);

void set_tags_to_file(t_writetags *x, const char *filename, t_llll *tags);
void clear_all_tags_from_file(t_writetags *x, const char *filename, char clearv1, char clearv2);
void clear_specific_tag(t_writetags *x, ID3_Tag &myTag, const char *code, char all_occurrences);
long code_to_id3v2_frameid(const char *code);
void tag_setfield(t_writetags *x, ID3_Tag &myTag, const char *code, t_llllelem *first_val, const char *filename);


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
	
	CLASS_NEW_CHECK_SIZE(c, "ears.writetags",
				  (method)writetags_new,
				  (method)writetags_free,
				  sizeof(t_writetags),
				  (method)NULL,
				  A_GIMME,
				  0L);
	

    // @method read @digest Function depends on inlet
    // @description An llll in the first inlet writes some ID3V1 or ID3V2 tags. Such llll is
    // expected to be a list of tags in the form
    // <b>(<m>tag</m> <m>content</m>), (<m>tag</m> <m>content</m>)...</b>, where each <m>tag</m>
    // is an ID3V1 or ID3V2 tag and <m>content</m> is its corresponding value to be set.
    // Tags for ID3V1 can be one of the following symbols: "artist", "title", "album", "year", "comment", "track";
    // tags for ID3V2 files are four-letter symbols: see http://id3.org/id3v2.3.0 to know more.<br />
    // An llll in the second inlet sets the file or the list of files to which the tags should be set.
    // @marg 0 @name file_name @optional 1 @type symbol
    class_addmethod(c, (method)writetags_int,				"int",			A_LONG, 0);
	class_addmethod(c, (method)writetags_anything,			"anything",			A_GIMME, 0);
	class_addmethod(c, (method)writetags_anything,			"list",			A_GIMME, 0);
    class_addmethod(c, (method)writetags_bang,              "bang",	 0);

    
    // @method clear @digest Remove all tags
    // @description A <m>clear</m> message removes all the ID3V1 and ID3V2 tags (if the <m>id3v1</m> and <m>id3v2</m>
    // attributes are on). An optional integer argument (1 or 2) limits the removal to the ID3V1 or ID3V2 tags respectively.
    // If further optional arguments are given, these are expected to be one or several of the four-letter symbols (see http://id3.org/id3v2.3.0 to know more)
    // corresponding to the fields to be cleared; if such arguments are given, any other field will be left untouched.
    // @marg 0 @name version @optional 1 @type int
    // @marg 1 @name fields @optional 1 @type llll
    class_addmethod(c, (method)writetags_anything,			"clear",			A_GIMME, 0);


    // @method clearfirst @digest Remove single tag
    // @description A <m>clearfirst</m> message removes only the first occurrence of a ID3V2 tag with a given four-letter signature.
    // For all other purposes use <m>clear</m>
    // @marg 0 @name version @optional 1 @type int
    class_addmethod(c, (method)writetags_anything,			"clearfirst",			A_GIMME, 0);
    

    class_addmethod(c, (method)writetags_assist,					"assist",				A_CANT,		0);
    class_addmethod(c, (method)writetags_inletinfo,					"inletinfo",			A_CANT,		0);

    llllobj_class_add_out_attr(c, LLLL_OBJ_VANILLA);
    
	CLASS_STICKY_ATTR(c,"category",0,"Settings");

    CLASS_ATTR_CHAR(c, "verbose",	0,	t_writetags, verbose_errors);
    CLASS_ATTR_STYLE_LABEL(c, "verbose", 0, "onoff", "Verbose Errors");
    // @description When set to 1, outputs errors in the Max window.

    CLASS_ATTR_CHAR(c, "id3v1",	0,	t_writetags, set_id3v1_tags);
    CLASS_ATTR_STYLE_LABEL(c, "id3v1", 0, "onoff", "Handle ID3V1 tags");
    // @description When set to 1, handles ID3V1 tags.
    
    CLASS_ATTR_CHAR(c, "id3v2",	0,	t_writetags, set_id3v2_tags);
    CLASS_ATTR_STYLE_LABEL(c, "id3v2", 0, "onoff", "Handle ID3V2 tags");
    // @description When set to 1, handles ID3V2 tags.

    CLASS_ATTR_CHAR(c, "sync",	0,	t_writetags, sync_v1_v2_tags);
    CLASS_ATTR_STYLE_LABEL(c, "sync", 0, "onoff", "Sync ID3V1 and ID3V2 tags");
    // @description When set to 1, it enhances synchronization between ID3V1 and ID3V2 tags.
    
    CLASS_ATTR_CHAR(c, "protect", 0,	t_writetags, protect);
    CLASS_ATTR_STYLE_LABEL(c, "protect", 0, "onoff", "Only tag MP3 files");
    // @description When set to 1, it protects against non-MP3 file tagging, i.e. it only allows tagging of files
    // ending with the "mp3" extension. This is handy to avoid unwanted tagging of non-mp3 files, which might
    // cause file corruption.
    
    CLASS_ATTR_CHAR(c, "numericufid",	0,	t_writetags, ufid_are_numbers);
    CLASS_ATTR_STYLE_LABEL(c, "numericufid", 0, "onoff", "Treat 'UFID' fields as numbers");
    // @description When set to 1, treats unique file ID fields ('UFID') as numeric. This will prevent
    // filling up Max symbol table if a lot of IDs are used.

    
    
    CLASS_STICKY_ATTR_CLEAR(c, "category");

	

	class_register(CLASS_BOX, c);
	s_tag_class = c;
	ps_event = gensym("event");
	return 0;
}

void writetags_assist(t_writetags *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        if (a == 0) // @in 0 @type llll @digest Tags
            sprintf(s, "llll: Tags in llll form");
            // @description The ID3V1 or ID3V2 field name followed by the associated tag value
            // For ID3V2 this is a 4-letter character combination, while for ID3V1
            // it is one of the following symbols: "artist", "year", "album", "title",
            // "comment", "track".
            // Multiple tags can be assigned by wrapping them in levels of parentheses.
            // Which tags are set and whether they are synchronized between versions is handled
            // via the various attributes.
        else if (a == 1) // @in 1 @type symbol @digest Filename(s)
            sprintf(s, "symbol: Filename(s)"); // @description The filename or filenames to which metadata should be set
    } else {
        if (a == 0) // @out 0 @type bang @digest bang when writing is ended
            sprintf(s, "bang when Done Writing");
    }
}

void writetags_inletinfo(t_writetags *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_writetags* writetags_new(t_symbol *s, short argc, t_atom *argv)
{
	t_writetags* x;
	
//	long true_ac = attr_args_offset(argc, argv);

	x = (t_writetags*)object_alloc_debug(s_tag_class);
	if (x) {
        x->curr_filenames = llll_get();
        x->set_id3v2_tags = true;
        x->set_id3v1_tags = true;
        x->sync_v1_v2_tags = true;
        x->verbose_errors = true;
        x->protect = true;
        
		llllobj_obj_setup((t_llllobj_object *)x, 2, "b");
		
        attr_args_process(x, argc, argv);

		systhread_mutex_new_debug(&x->c_mutex, 0);

        long i;
        for (i = 1; i > 0; i--)
            x->n_proxy[i] = proxy_new_debug((t_object *) x, i, &x->n_in);
	}
	return x;
}


void writetags_free(t_writetags *x)
{
    llll_free(x->curr_filenames);
    long i;
    for (i = 1; i > 0; i--)
        object_free_debug(x->n_proxy[i]);
	systhread_mutex_free_debug(x->c_mutex);
}


void writetags_int(t_writetags *x, t_atom_long num)
{
	t_atom argv[1]; 
	atom_setlong(argv, num);
	writetags_anything(x, _sym_list, 1, argv);
}


void writetags_bang(t_writetags *x)
{
    writetags_anything(x, _sym_bang, 0, NULL);
}



long clear_tags_from_file_fn(void *data, t_hatom *a, const t_llll *address)
{
    t_writetags *x = (t_writetags *) ((void **)data)[0];
    long whichtags = *((long *) ((void **)data)[1]);
    t_llll *fields = (t_llll *) ((void **)data)[2];
    char all_occurrences = *((char *) ((void **)data)[3]);
    
    if (hatom_gettype(a) == H_SYM) {
        t_symbol *s = hatom_getsym(a);
        if (s) {
            if (!fields || fields->l_size == 0)
                clear_all_tags_from_file(x, s->s_name, whichtags == 0 || whichtags == 1, whichtags == 0 || whichtags == 2);
            else {
                ID3_Tag myTag;
                
                char conformed_name[MAX_PATH_CHARS];
                path_nameconform(s->s_name, conformed_name, PATH_STYLE_MAX, PATH_TYPE_BOOT);
                
                myTag.Link(conformed_name, ID3TT_ID3V2);
                myTag.SetUnsync(x->sync_v1_v2_tags ? 1 : 0);
                
                t_llllelem *elem;
                for (elem = fields->l_head; elem; elem = elem->l_next) {
                    if (hatom_gettype(&elem->l_hatom) == H_SYM)
                        clear_specific_tag(x, myTag, hatom_getsym(&elem->l_hatom)->s_name, all_occurrences);
                }
                
                myTag.Update(ID3TT_ID3V2);
            }
        }
    }
    return 0;
}


char is_symbol_mp3_file(t_symbol *s)
{
    if (!s)
        return 0;
    
    long len = strlen(s->s_name);
    if (len >= 4 && strcmp_case_insensitive(s->s_name + len - 4, ".mp3") == 0)
        return 1;
    return 0;
}

long set_tags_to_file_fn(void *data, t_hatom *a, const t_llll *address)
{
    if (hatom_gettype(a) == H_SYM) {
        t_writetags *x = (t_writetags *) (((void **)data)[0]);
        t_llll *tags = (t_llll *) (((void **)data)[1]);
        t_symbol *s = hatom_getsym(a);
        if (s && (!x->protect || is_symbol_mp3_file(s)))
            set_tags_to_file(x, s->s_name, tags);
    }
    return 0;
}

void writetags_anything(t_writetags *x, t_symbol *msg, long ac, t_atom *av)
{
	long inlet = proxy_getinlet((t_object *) x);
	
    t_llll *parsed = llllobj_parse_llll((t_object *) x, LLLL_OBJ_VANILLA, msg, ac, av, LLLL_PARSE_CLONE);
    if (!parsed) return;
    
    if (inlet == 1) {
		// set filename(s)
        systhread_mutex_lock(x->c_mutex);
        llll_free(x->curr_filenames);
        x->curr_filenames = llll_clone(parsed);
        systhread_mutex_unlock(x->c_mutex);

	} else  if (inlet == 0) {
        if (parsed->l_head && hatom_gettype(&parsed->l_head->l_hatom) == H_SYM &&
            (hatom_getsym(&parsed->l_head->l_hatom) == _sym_clear || hatom_getsym(&parsed->l_head->l_hatom) == gensym("clearfirst"))) {
            // clear tag from files
            char all = (hatom_getsym(&parsed->l_head->l_hatom) == _sym_clear);
            
            llll_behead(parsed);

            void *data[4];
            long whichtags = 0;
            
            if (parsed->l_head && is_hatom_number(&parsed->l_head->l_hatom)) {
                whichtags = hatom_getlong(&parsed->l_head->l_hatom);
                llll_behead(parsed);
            }

            data[0] = x;
            data[1] = &whichtags;
            data[2] = parsed;
            data[3] = &all;
            
            systhread_mutex_lock(x->c_mutex);
            llll_funall(x->curr_filenames, (fun_fn) clear_tags_from_file_fn, data, 1, -1, FUNALL_ONLY_PROCESS_ATOMS);
            systhread_mutex_unlock(x->c_mutex);
            llllobj_outlet_bang((t_object *)x, LLLL_OBJ_VANILLA, 0);
        } else {
            // set tag to files
            void *data[2];
            data[0] = x;
            data[1] = parsed;
            systhread_mutex_lock(x->c_mutex);
            if (x->curr_filenames->l_size == 0) {
                if (x->verbose_errors)
                    object_warn((t_object *)x, "No files defined.");
            } else
                llll_funall(x->curr_filenames, (fun_fn) set_tags_to_file_fn, data, 1, -1, FUNALL_ONLY_PROCESS_ATOMS);
            systhread_mutex_unlock(x->c_mutex);
            llllobj_outlet_bang((t_object *)x, LLLL_OBJ_VANILLA, 0);
        }
    }

    llll_free(parsed);
}




///// LLLL WRAPPERS

flags_t get_which_tags(t_writetags *x)
{
    flags_t which = 0;
    if (x->set_id3v1_tags)
        which |= ID3TT_ID3V1;
    if (x->set_id3v2_tags)
        which |= ID3TT_ID3V2;
    return which;
}

void clear_all_tags_from_file(t_writetags *x, const char *filename, char clearv1, char clearv2)
{
    char conformed_name[MAX_PATH_CHARS];
    path_nameconform(filename, conformed_name, PATH_STYLE_MAX, PATH_TYPE_BOOT);

    FILE * fp;
    /* cludgy to check if we have the proper perms */
    fp = fopen(conformed_name, "r+");
    if (fp == NULL) { /* file didn't open */
        if (x->verbose_errors)
            object_error((t_object *)x, "Can't open file %s. Check permissions.", filename);
        return;
    }
    fclose(fp);
    
    ID3_Tag myTag;
    myTag.Link(conformed_name, ID3TT_ALL);
    
    luint nTags;
    if (x->set_id3v1_tags && clearv1) {
        nTags = myTag.Strip(ID3TT_ID3V1);
    }
    if (x->set_id3v2_tags && clearv2) {
        nTags = myTag.Strip(ID3TT_ID3V2);
    }
}

void set_tags_to_file(t_writetags *x, const char *filename, t_llll *tags)
{
    ID3_Tag myTag;
    
    char conformed_name[MAX_PATH_CHARS];
    path_nameconform(filename, conformed_name, PATH_STYLE_MAX, PATH_TYPE_BOOT);
    
    myTag.Link(conformed_name, ID3TT_ID3V2);
    myTag.SetUnsync(x->sync_v1_v2_tags ? 1 : 0);

    t_llllelem *elem;
    for (elem = tags->l_head; elem; elem = elem->l_next)
    {
        if (hatom_gettype(&elem->l_hatom) != H_LLLL)
            continue;
        
        t_llll *ll = hatom_getllll(&elem->l_hatom);
        if (!ll->l_head || hatom_gettype(&ll->l_head->l_hatom) != H_SYM)
            continue;
        
        tag_setfield(x, myTag, hatom_getsym(&ll->l_head->l_hatom)->s_name, ll->l_head->l_next, conformed_name);
    }
    
    myTag.Update(ID3TT_ID3V2);
}


/*
 { "artist",  required_argument, &iLongOpt, 'a' },
 { "album",   required_argument, &iLongOpt, 'A' },
 { "song",    required_argument, &iLongOpt, 't' },
 { "comment", required_argument, &iLongOpt, 'c' },
 { "genre",   required_argument, &iLongOpt, 'g' },
 { "year",    required_argument, &iLongOpt, 'y' },
 { "track",   required_argument, &iLongOpt, 'T' },
*/


void clear_specific_tag(t_writetags *x, ID3_Tag &myTag, const char *code, char all_occurrences)
{
    
    ID3_FrameID frame_id = (enum ID3_FrameID)string_to_frameid(code);
    if (frame_id == ID3FID_NOFRAME)
        frame_id = (enum ID3_FrameID)code_to_id3v2_frameid(code);
    
    if (!all_occurrences) {
        ID3_Frame *pFrame = myTag.Find(frame_id);
        if (pFrame != NULL)
            myTag.RemoveFrame(pFrame);
    } else {
        ID3_Frame *pFrame;
        while ((pFrame = myTag.Find(frame_id)))
            myTag.RemoveFrame(pFrame);
    }
}

void tag_setfield(t_writetags *x, ID3_Tag &myTag, const char *code, t_llllelem *first_val, const char *filename)
{
    t_symbol *s = NULL;
    t_llllelem *t_elem = NULL, *u_elem = NULL;
    if (first_val) {
        s = hatom_to_symbol(&first_val->l_hatom);
        t_elem = first_val->l_next;
        if (t_elem) u_elem = t_elem->l_next;
    }
    
    ID3_Frame *myFrame;
    myFrame = new ID3_Frame;
    if (NULL == myFrame)
    {
        if (x->verbose_errors)
            object_error((t_object *)x, "Out of memory!");
        return;
    }
    
    ID3_FrameID frame_id = (enum ID3_FrameID)string_to_frameid(code);
    if (frame_id == ID3FID_NOFRAME)
        frame_id = (enum ID3_FrameID)code_to_id3v2_frameid(code);
    myFrame->SetID(frame_id);
    
    
    /// id3v1 cases
    // We handle these cases separately, because id3lib handling of v2->v1 seems rather quirky and weird :-)
    if (x->set_id3v1_tags) {
        if (frame_id == ID3FID_COMMENT || frame_id == ID3FID_TRACKNUM || frame_id == ID3FID_CONTENTTYPE ||
            frame_id == ID3FID_ALBUM || frame_id == ID3FID_LEADARTIST ||
            frame_id == ID3FID_TITLE || frame_id == ID3FID_YEAR) {
            char *		newtitle = NULL;
            char *		newalbum = NULL;
            char *		newartist = NULL;
            char *		newnote = NULL;
            long		newyear = -1;
            long		newgenre = -1;
            long		newtrack = -1;
            id3tag_t	mytag;
            FILE		*fptr = NULL;
            int		newtag = 0;
            char		strbuf[31];
            int		opt_index = 0;
            int		ctr;

            
            fptr = fopen(filename, "rb");
            if (NULL == fptr) {
                object_error((t_object *)x, "Can't open file %s for read.", filename);
                goto id3v2;
            }
            if (id3_readtag(fptr, &mytag)) {
                newtag = 1;
                id3_cleartag(&mytag);
            } else {
                newtag = 0;
            }

            fclose(fptr);

            
            const char *s_str = s ? s->s_name : "";
            switch (frame_id) {
                case ID3FID_TITLE:
                    memset (mytag.songname, 0, 30);
                    strncpy (mytag.songname, s_str, 30);
                    break;
                    
                case ID3FID_LEADARTIST:
                    memset (mytag.artist, 0, 30);
                    strncpy (mytag.artist, s_str, 30);
                    break;
                    
                case ID3FID_ALBUM:
                    memset (mytag.album, 0, 30);
                    strncpy (mytag.album, s_str, 30);
                    break;
                    
                case ID3FID_YEAR:
                    sprintf (strbuf, "%4.4ld", atol(s_str));
                    memset (mytag.year, 0, 4);
                    strncpy (mytag.year, strbuf, 4);
                    break;
                    
                case ID3FID_TRACKNUM:
                    mytag.note.v11.marker = '\0';
                    mytag.note.v11.track = atoi(s_str);
                    break;
                    
                case ID3FID_COMMENT:
                    if (mytag.note.v11.marker == '\0'
                        && mytag.note.v11.track != 0) {
                        memset (mytag.note.v10.note, 0, 28);
                        strncpy (mytag.note.v11.note, s_str, 28);
                    } else {
                        memset (mytag.note.v10.note, 0, 30);
                        strncpy (mytag.note.v10.note, s_str, 30);
                    }
                    break;
                    
                case ID3FID_CONTENTTYPE:
                {
                    long newgenre = 0;
                    if (hatom_gettype(&first_val->l_hatom) == H_LONG)
                        newgenre = hatom_getlong(&first_val->l_hatom);
                    else {
                        for (ctr = 0; id3_styles[ctr].name != NULL; ctr++) {
                            if (!strcasecmp(s_str, id3_styles[ctr].name)) {
                                /* we found it! */
                                break;
                            }
                        }
                        if (id3_styles[ctr].name == NULL) {
                            object_warn((t_object *)x, "Couldn't find genre ", s_str);
                            goto id3v2;
                        }
                        newgenre = id3_styles[ctr].styleid;
                    }
                    mytag.style = newgenre;
                }
                    
                default:
                    break;
            }

            
            fptr = fopen(filename, "r+b");
            if (NULL == fptr) {
                object_error((t_object *)x, "Can't open file %s for write.", filename);
                goto id3v2;
            }
            if (newtag) {
                id3_appendtag(fptr, &mytag);
            } else {
                id3_replacetag(fptr, &mytag);
            }
            fclose(fptr);
            
        }
    }
    
id3v2:
    
    ID3_Frame *pFrame;
    pFrame = myTag.Find(frame_id);
    
    switch (frame_id)
    {
            //  strings
        case ID3FID_ALBUM:
        case ID3FID_BPM:
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
        case ID3FID_ORIGYEAR:
        case ID3FID_FILEOWNER:
        case ID3FID_LEADARTIST:
        case ID3FID_BAND:
        case ID3FID_CONDUCTOR:
        case ID3FID_MIXARTIST:
        case ID3FID_PARTINSET:
        case ID3FID_PUBLISHER:
        case ID3FID_RECORDINGDATES:
        case ID3FID_NETRADIOSTATION:
        case ID3FID_NETRADIOOWNER:
        case ID3FID_SIZE:
        case ID3FID_ISRC:
        case ID3FID_ENCODERSETTINGS:
        case ID3FID_YEAR:
        {
            if (pFrame != NULL)
            {
                myTag.RemoveFrame(pFrame);
            }
            if (s) {
                myFrame->Field(ID3FN_TEXT) = s->s_name;
                myTag.AttachFrame(myFrame);
            }
            break;
        }
        case ID3FID_TRACKNUM:
        {
            // check if there is a total track number and if we only have
            // the track number for this file.  In this case combine them.
            char *currentTrackNum, *newTrackNum = NULL;
            
            if (pFrame != NULL && s)
            {
                currentTrackNum = ID3_GetString(pFrame, ID3FN_TEXT);
                if (*currentTrackNum == '/')
                {
                    newTrackNum = (char *)malloc(strlen(currentTrackNum)
                                                 + strlen(s->s_name));
                    strcpy(newTrackNum, s->s_name);
                    strcat(newTrackNum, currentTrackNum);
                }
                else
                {
                    myTag.RemoveFrame(pFrame);
                }
            }
            
            if (s)
                myFrame->Field(ID3FN_TEXT) = s->s_name;
            myTag.AttachFrame(myFrame);
            
            free(newTrackNum);
            break;
        }
        case ID3FID_USERTEXT:
        {
            if (pFrame != NULL) {
                myTag.RemoveFrame(pFrame);
            }
            
            // split the string at the ':' remember if no : then leave
            // descrip empty
            if (s) {
                myFrame->Field(ID3FN_TEXT) = s->s_name;
                if (t_elem) {
                    t_symbol *t = hatom_getsym(&t_elem->l_hatom);
                    if (t)
                        myFrame->Field(ID3FN_DESCRIPTION) = t->s_name;
                }
            }
            if (strlen(ID3_GetString(myFrame, ID3FN_TEXT)) > 0) {
                myTag.AttachFrame(myFrame);
            }
            
            break;
        }
        case ID3FID_COMMENT:
        case ID3FID_UNSYNCEDLYRICS:
        {
            // split the string at the ':' remember if no : then leave
            // descrip/lang empty
            if (s) {
                myFrame->Field(ID3FN_TEXT) = s->s_name;
                if (t_elem) {
                    t_symbol *t = hatom_getsym(&t_elem->l_hatom);
                    if (t)
                    myFrame->Field(ID3FN_DESCRIPTION) = t->s_name;
                }
                if (u_elem) {
                    t_symbol *u = hatom_getsym(&u_elem->l_hatom);
                    if (u)
                    myFrame->Field(ID3FN_LANGUAGE) = u->s_name;
                }
            }
            // debug
            // std::cout << ID3_GetString(myFrame, ID3FN_DESCRIPTION) << std::endl
            // << ID3_GetString(myFrame, ID3FN_TEXT) << std::endl
            // << ID3_GetString(myFrame, ID3FN_LANGUAGE) << std::endl;
            
            
            // now try and find a comment/lyrics with the same descript
            // and lang as what we have
            ID3_Frame *pFirstFrame = NULL;
            do {
                // if pFrame is NULL, either there were no comments/lyrics
                // to begin with, or we removed them all in the process
                if (pFrame == NULL) break;
                
                if (pFirstFrame == NULL)
                {
                    pFirstFrame = pFrame;
                }
                
                char *tmp_desc = ID3_GetString(pFrame, ID3FN_DESCRIPTION);
                char *tmp_my_desc = ID3_GetString(myFrame, ID3FN_DESCRIPTION);
                char *tmp_lang = ID3_GetString(pFrame, ID3FN_LANGUAGE);
                char *tmp_my_lang = ID3_GetString(myFrame, ID3FN_LANGUAGE);
                if ((strcmp(tmp_desc, tmp_my_desc) == 0) &&
                    (strcmp(tmp_lang, tmp_my_lang) == 0))
                {
                    myTag.RemoveFrame(pFrame);
                    if (pFrame == pFirstFrame)
                    {
                        pFirstFrame = NULL;
                    }
                }
                delete [] tmp_desc;
                delete [] tmp_my_desc;
                delete [] tmp_lang;
                delete [] tmp_my_lang;
                
                // get the next frame until it wraps around
            } while ((pFrame = myTag.Find(frame_id)) != pFirstFrame);
            
            if (strlen(ID3_GetString(myFrame, ID3FN_TEXT)) > 0) {
                myTag.AttachFrame(myFrame);
            }
            
            
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
            if (pFrame != NULL && s) {
                char *sURL = ID3_GetString(pFrame, ID3FN_URL);
                if (strcmp(s->s_name, sURL) == 0)
                    myTag.RemoveFrame(pFrame);
            }
            
            if (s && strlen(s->s_name) > 0) {
                myFrame->Field(ID3FN_URL) = s->s_name;
                myTag.AttachFrame(myFrame);
            }
            
            break;
            
        }
        case ID3FID_WWWUSER:
        {
            // UNSUPPORTED???
            /*                char
             *sURL = ID3_GetString(myFrame, ID3FN_URL),
             *sDesc = ID3_GetString(myFrame, ID3FN_DESCRIPTION);
             std::cout << "(" << sDesc << "): " << sURL << std::endl;
             delete [] sURL;
             delete [] sDesc;*/
            break;
        }
        case ID3FID_INVOLVEDPEOPLE:
        {
            // This isn't the right way to do it---will only get first person
            /*            size_t nItems = myFrame->Field(ID3FN_TEXT).GetNumTextItems();
             for (size_t nIndex = 1; nIndex <= nItems; nIndex++)
             {
             char *sPeople = ID3_GetString(myFrame, ID3FN_TEXT, nIndex);
             std::cout << sPeople;
             delete [] sPeople;
             if (nIndex < nItems)
             {
             std::cout << ", ";
             }
             }
             std::cout << std::endl;
             */          break;
        }
        case ID3FID_PICTURE:
        {
            /*            char
             *sMimeType = ID3_GetString(myFrame, ID3FN_MIMETYPE),
             *sDesc     = ID3_GetString(myFrame, ID3FN_DESCRIPTION),
             *sFormat   = ID3_GetString(myFrame, ID3FN_IMAGEFORMAT);
             size_t
             nPicType   = myFrame->Field(ID3FN_PICTURETYPE).Get(),
             nDataSize  = myFrame->Field(ID3FN_DATA).Size();
             std::cout << "(" << sDesc << ")[" << sFormat << ", "
             << nPicType << "]: " << sMimeType << ", " << nDataSize
             << " bytes" << std::endl;
             delete [] sMimeType;
             delete [] sDesc;
             delete [] sFormat;
             */          break;
        }
        case ID3FID_GENERALOBJECT:
        {
            /*            char
             *sMimeType = ID3_GetString(myFrame, ID3FN_TEXT),
             *sDesc = ID3_GetString(myFrame, ID3FN_DESCRIPTION),
             *sFileName = ID3_GetString(myFrame, ID3FN_FILENAME);
             size_t
             nDataSize = myFrame->Field(ID3FN_DATA).Size();
             std::cout << "(" << sDesc << ")["
             << sFileName << "]: " << sMimeType << ", " << nDataSize
             << " bytes" << std::endl;
             delete [] sMimeType;
             delete [] sDesc;
             delete [] sFileName;
             */          break;
        }
        case ID3FID_UNIQUEFILEID:
        {
            
            if (s) {
                myFrame->Field(ID3FN_OWNER) = s->s_name; // owner
                if (t_elem) {
                    if (x->ufid_are_numbers && hatom_gettype(&t_elem->l_hatom) == H_LONG) {
                        long val = hatom_getlong(&t_elem->l_hatom);
                        char res[64];
                        long i;
/*                        for (i = 0; i < 4 && i < strlen(code); i++)
                            fourchars_code |= (code[i] << (8 * (4 - i - 1)));
  */
                        for (i = 0; i < 64 && val > 0; i++) {
                            res[i] = (char)val;
                            val = val >> 8;
                        }
                        
                        myFrame->GetField(ID3FN_DATA)->Set((uchar *)res, i);
//                        myFrame->GetField(ID3FN_DATA)->Set((uchar *)data, strlen((const char*)data));
                    } else {
                        t_symbol *t = hatom_to_symbol(&t_elem->l_hatom);
                        myFrame->GetField(ID3FN_DATA)->Set((uchar *)t->s_name, strlen((const char*)t->s_name));
                    }
                }
            }
            
            
            // now try and find a comment/lyrics with the same owner
            // as what we have
            ID3_Frame *pFirstFrame = NULL;
            do {
                // if pFrame is NULL, either there were no comments/lyrics
                // to begin with, or we removed them all in the process
                if (pFrame == NULL) break;
                
                if (pFirstFrame == NULL)
                {
                    pFirstFrame = pFrame;
                }
                
                char *tmp_owner = ID3_GetString(pFrame, ID3FN_OWNER);
                char *tmp_my_owner = ID3_GetString(myFrame, ID3FN_OWNER);
                if ((strcmp(tmp_owner, tmp_my_owner) == 0))
                {
                    myTag.RemoveFrame(pFrame);
                    if (pFrame == pFirstFrame)
                    {
                        pFirstFrame = NULL;
                    }
                }
                delete [] tmp_owner;
                delete [] tmp_my_owner;
                
                // get the next frame until it wraps around
            } while ((pFrame = myTag.Find(frame_id)) != pFirstFrame);
            
            if (strlen(ID3_GetString(myFrame, ID3FN_OWNER)) > 0) {
                myTag.AttachFrame(myFrame);
            }

/*
            if (pFrame != NULL)
            {
                char *sOwner = ID3_GetString(pFrame, ID3FN_TEXT);
                size_t nDataSize = pFrame->Field(ID3FN_DATA).Size();
                post("%s, %ld", sOwner, nDataSize);
                delete [] sOwner;
            }
 */
            break;
        }
        case ID3FID_PRIVATE:
        {
            if (s) {
                myFrame->Field(ID3FN_OWNER) = s->s_name; // owner
                if (t_elem) {
                    t_symbol *t = hatom_to_symbol(&t_elem->l_hatom);
                    myFrame->GetField(ID3FN_DATA)->Set((uchar *)t->s_name, strlen((const char*)t->s_name));
                }
            }
            
            
            // now try and find a private with the same owner
            // as what we have
            ID3_Frame *pFirstFrame = NULL;
            do {
                // if pFrame is NULL, either there were no comments/lyrics
                // to begin with, or we removed them all in the process
                if (pFrame == NULL) break;
                
                if (pFirstFrame == NULL)
                {
                    pFirstFrame = pFrame;
                }
                
                char *tmp_owner = ID3_GetString(pFrame, ID3FN_OWNER);
                char *tmp_my_owner = ID3_GetString(myFrame, ID3FN_OWNER);
                if ((strcmp(tmp_owner, tmp_my_owner) == 0))
                {
                    myTag.RemoveFrame(pFrame);
                    if (pFrame == pFirstFrame)
                    {
                        pFirstFrame = NULL;
                    }
                }
                delete [] tmp_owner;
                delete [] tmp_my_owner;
                
                // get the next frame until it wraps around
            } while ((pFrame = myTag.Find(frame_id)) != pFirstFrame);
            
            if (strlen(ID3_GetString(myFrame, ID3FN_OWNER)) > 0) {
                myTag.AttachFrame(myFrame);
            }
            
            break;
        }
        case ID3FID_PLAYCOUNTER:
        {
            /*            if (pFrame != NULL)
             {
             size_t nCounter = pFrame->Field(ID3FN_COUNTER).Get();
             std::cout << nCounter << std::endl;
             }
             */          break;
        }
        case ID3FID_POPULARIMETER:
        {
            /*            if (pFrame != NULL)
             {
             char *sEmail = ID3_GetString(pFrame, ID3FN_EMAIL);
             size_t
             nCounter = pFrame->Field(ID3FN_COUNTER).Get(),
             nRating = pFrame->Field(ID3FN_RATING).Get();
             std::cout << sEmail << ", counter="
             << nCounter << " rating=" << nRating;
             delete [] sEmail;
             }
             */          break;
        }
        case ID3FID_CRYPTOREG:
        case ID3FID_GROUPINGREG:
        {
            /*            char *sOwner = ID3_GetString(myFrame, ID3FN_OWNER);
             size_t
             nSymbol = myFrame->Field(ID3FN_ID).Get(),
             nDataSize = myFrame->Field(ID3FN_DATA).Size();
             std::cout << "(" << nSymbol << "): " << sOwner
             << ", " << nDataSize << " bytes";
             */          break;
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
        case ID3FID_SYNCEDLYRICS:
        case ID3FID_SYNCEDTEMPO:
        case ID3FID_METACRYPTO:
        {
            //                std::cout << " (unimplemented)" << std::endl;
            break;
        }
        default:
        {
            //                std::cout << " frame" << std::endl;
            break;
        }
    }
}


