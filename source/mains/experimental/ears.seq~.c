/**
 @file
 dada.seq~ 
 
 @ingroup	dada	
 */

#include "foundation/llllobj.h" // you must include this.
#include "ext.h"
#include "ext_obex.h"
#include "ext_common.h" // contains CLAMP macro
#include "z_dsp.h"
#include "ext_buffer.h"
#include "ext_globalsymbol.h"
#include "dada.commons.h"
#include "dada.geometry.h"
#include "dada.paint.h"
#include "dada.interface.h"
#include "dada.undo.h"
#include "dada.popupmenu.h"


#define DADA_SEQ_MAX_NUM_AUDIO_OUTLETS 100

#define DADA_SEQ_BUFFER_SAMPLES_PER_MS_VIEW0 0.1
#define DADA_SEQ_BUFFER_SAMPLES_PER_MS_VIEW1 1
#define DADA_SEQ_BUFFER_SAMPLES_PER_MS_VIEW2 10
#define DADA_SEQ_BUFFER_SAMPLES_PER_MS_VIEW3 20

#define DADA_SEQ_DEFAULT_PIXELS_PER_SECOND 1000
#define DADA_SEQ_CONST_MAX_TRACKS 100

#define DADA_SEQ_FREE_BUFFER_WHEN_COUNT_IS_ZERO true

#define DADA_SEQ_SELECTION_THRESH 2

#define DECLARE_SEQ_ATTR(r_ob, man, forced_position, name, displayed_label, owner_type, struct_name, struct_member, attr_type, attr_size, display_type, preprocess_flags, postprocess_flags) \
{ \
	DECLARE_BACH_ATTR(man, forced_position, name, displayed_label, owner_type, struct_name, struct_member, attr_type, attr_size, display_type, preprocess_flags, postprocess_flags); \
	bach_attribute_add_functions(get_bach_attribute(man, owner_type, name), (bach_getter_fn)dada_default_get_bach_attr, (bach_setter_fn)dada_default_set_bach_attr, NULL, NULL, (bach_inactive_fn)dada_default_attr_inactive); \
}

typedef enum _seq_item_types
{
	DADAITEM_TYPE_SEQ_BUFFER	=	80,
	DADAITEM_TYPE_SEQ_REGION	=	81,
	DADAITEM_TYPE_SEQ_TRACK		=	82,
} e_seq_item_types;


typedef struct _seq_buffer_view {
	double			sample_dist_ms;
	double			subsampling;
	double			num_samples;
	long			num_samples_ceil;
	float			*samples;
} t_seq_buffer_view;


typedef struct _seq_buffer {
	t_dadaitem			r_it;

	t_buffer_obj		*buffer;				//< Actually created buffer with object_new()
	t_buffer_ref		*buffer_reference;		//< Buffer reference

	t_symbol			*filename;
	t_symbol			*filename_fullpath;
	t_symbol			*buffername;
	
	double				duration_ms;
	long				duration_samps;

	double				sr;			//< Sampling rate

	t_seq_buffer_view	views[4];	//< View "i" has 1 sample each DADA_SEQ_BUFFER_SAMPLES_PER_MS_VIEW"i" ms

	long				reference_count;		//< Number of regions using this buffer
	
	t_llllelem			*elem;	///< Element in the seq buffer linked list
} t_seq_buffer;


typedef enum _seq_fade_types 
{
	FADE_IN_OR_OUT	=	0x00,
	FADE_CROSS		=	0x01,
} e_seq_fade_types;


typedef enum _seq_fade_style 
{
	FADE_LINEAR			=	0x00,
	FADE_EQUAL_POWER	=	0x01,
} e_seq_fade_style;


typedef struct _seq_fade {
//	e_seq_fade_types	fade_type;	//< Unsupported for now
//	e_seq_fade_style	fade_style;
	double				duration_ms;
	double				duration_samps;
	double				slope;
} t_seq_fade;


typedef struct _seq_region {
	t_dadaitem		r_it;

	t_seq_buffer	*buffer;			///< Associated buffer
	t_symbol		*filename;			///< Filename
	t_jrgba			color;				///< Color
	
	double			onset_ms;			///< Onset of the region in milliseconds;
	long			onset_samps;		///< Onset of the regions in samples;
	
	double			bufferstart_ms;		///< Starting point in the buffer in milliseconds;
	long			bufferstart_samps;	///< Starting point in the buffer in samples;

	double			dur_ms;				///< Duration of the region in milliseconds;
	long			dur_samps;			///< Duration of the region in samples;

	double			global_gain;
	t_seq_fade		fade_in;
	t_seq_fade		fade_out;
	
	long			mousedown_portion;	///< 0 if region, -1 if start, 1 if end, -2 if fade start, 2 if fade end 
	
	t_llllelem		*elem;				///< Element in the track regions llll
	t_llllelem		*all_elem;			///< Element in the all-regions llll, for sequencing
	
	t_note			*note;				///< Symbolics: a note associated with the region, if any
} t_seq_region;



typedef struct _seq_track {
	t_dadaitem		r_it;
	
	long		number;
	long		channelmap[4];
	
	long		num_regions;
	t_llll		*regions;		//< As H_OBJs
	
	t_symbol	*name;
	
	t_slot		slot[CONST_MAX_SLOTS];	///< The array containing the slot content (= automation) for the track (for each slot)

	t_llllelem		*elem;				///< Element in the track llll
} t_seq_track;


typedef struct _seq {
	t_dadaobj_pxjbox	b_ob;
	
	long				num_audio_outlets;		// Total number of audio outlets. This is static and need to be set as argument at the object instantiation
	
	long				num_buffers;
	t_llll				*buffers;				// Buffers as H_OBJs

	long				num_tracks;
	t_llll				*tracks;				// Tracks as H_OBJs: these tracks are ALL allocated at the beginning. 

	t_llll				*all_regions;			// All regions as H_OBJs: these tracks are ALL allocated at the beginning. 

	// Settings
	long				num_channels;			///< Number of channels (1 <= num_channels <= 4)
	double				sr;						///< Current Sampling rate

	double				track_height;
	double				track_header_width; 
	
	double				screen_ms_start;
	double				screen_ms_end;
	
	char				show_focus;
	char				has_focus;

	// Colors
	t_jrgba				bg_color;
	t_jrgba				fg_color;
	t_jrgba				border_color;
	double				border_size;
	t_jrgba				region_color;
	t_jrgba				play_color;
	t_jrgba				selection_color;
	
	// Sequencing
	char				playing;			///< Playing flag (1 when playing "internally", and not via an incoming signal, 0 otherwise) - always updated only in the perform routine
	long				*play_samps;		///< Array with current samples to be played (indices) - always updated only in the perform routine
	long				play_samps_size;	///< Size of play_samps array
	long				playhead_samps;		///< Position of the playhead for play start in samples
	long				playhead_ms;		///< Position of the playhead for play start in milliseconds
	long				todo_in_perform;	///< Sets the operation (in any thread) to be performed in the perform64 method (e.g. start playing or stopping)
	
	t_llll				*sequenced_regions;	///< Currently sequenced regions as H_OBJs: these tracks are ALL allocated at the beginning. 
	t_llllelem			*next_region_to_sequence_elem; //< From the #all_regions llll
	long				next_region_to_sequence_samps;	

//	void				*m_clock;			///< A clock, to notify stuff outside the perform method (e.g. playhead which needs to be painted)
	double				play_redraw_ms;		///< Temporal distances between clock ticks
	
	// Interface
	long					curr_change_track_num;	// utility, internal
	char					just_duplicated;
	
} t_seq;


t_seq *seq_new(t_symbol *s, short ac, t_atom *av);
void seq_free(t_seq *x);

void seq_perform64(t_seq *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam);
void seq_dsp64(t_seq *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags);
t_max_err seq_notify(t_seq *x, t_symbol *s, t_symbol *msg, void *sender, void *data);
t_max_err seq_dadanotify(t_dadaobj *r_ob, t_symbol *s, t_symbol *modified, t_dadaitem *it, void *data);
void seq_setattr_filename(t_seq *x, void *obj, t_bach_attribute *attr, long ac, t_atom *av);

	
void seq_assist(t_seq *x, void *b, long m, long a, char *s);
long seq_acceptsdrag_unlocked(t_seq *x, t_object *drag, t_object *view);

void seq_bang(t_seq *x);
void invalidate_static_layer_and_repaint(t_seq *x);
void seq_paint(t_seq *x, t_object *patcherview);
void seq_focusgained(t_seq *x, t_object *patcherview);
void seq_focuslost(t_seq *x, t_object *patcherview);
void seq_mousedown(t_seq *x, t_object *patcherview, t_pt pt, long modifiers);
void seq_mousemove(t_seq *x, t_object *patcherview, t_pt pt, long modifiers);
void seq_mouseup(t_seq *x, t_object *patcherview, t_pt pt, long modifiers);
void seq_mousedrag(t_seq *x, t_object *patcherview, t_pt pt, long modifiers);
long seq_key(t_seq *x, t_object *patcherview, long keycode, long modifiers, long textcharacter);
long seq_keyup(t_seq *x, t_object *patcherview, long keycode, long modifiers, long textcharacter);
void seq_mousewheel(t_seq *x, t_object *view, t_pt pt, long modifiers, double x_inc, double y_inc);
void seq_mousedoubleclick(t_seq *x, t_object *patcherview, t_pt pt, long modifiers);
t_dadaitem *seq_pixel_to_dadaitem(t_dadaobj *r_ob, t_pt pt, t_object *patcherview, long modifiers, t_pt *coordinates, double selection_pad, t_dadaitem_identifier *identifier);
char seq_preselect_items_in_rect(t_dadaobj *r_ob, t_object *view, t_rect rect);

void seq_anything(t_seq *x, t_symbol *msg, long ac, t_atom *av);

// buffers
t_seq_buffer *buffer_new(t_seq *x, t_symbol *buffername);
double buffer_ms_to_samps(t_seq_buffer *buf, double ms);
double buffer_samps_to_ms(t_seq_buffer *buf, double samps);


// tracks
void init_all_tracks(t_seq *x);
t_seq_track *track_new(t_seq *x, t_symbol *track_name, long number);
t_seq_track *track_nth(t_seq *x, long num);


// regions
t_seq_track	*region_get_parent(t_seq *x, t_seq_region *reg);
t_seq_region *region_nth(t_seq *x, t_seq_track *track, long num);
t_seq_region *region_new(t_seq *x);
t_seq_region *region_create_and_insert(t_seq *x, long track_number, double onset_ms, t_symbol *filename, 
									   double filestart_ms, double dur_ms, double gain, 
									   double fadein_ms, double fadein_slope, double fadeout_ms, double fadeout_slope);
t_llllelem *region_insert(t_seq *x, t_seq_track *track, t_seq_region *region, t_llllelem **play_elem);
void region_delete(t_seq *x, t_seq_region *reg);
void region_delete_all(t_seq *x, long track_num);
void region_delete_selected(t_seq *x);
void region_move_deltams(t_seq *x, t_seq_region *region, double delta_ms);
void region_clone(t_seq *x, t_seq_region *region, char transfer_selection);
void region_change_track(t_seq *x, t_seq_region *region, long new_track);
void region_change_length_deltams(t_seq *x, t_seq_region *region, double delta_ms);
void region_change_offset_deltams(t_seq *x, t_seq_region *region, double delta_ms);
void region_change_fade_deltams(t_seq *x, t_seq_region *region, double delta_ms, char which_fade);
void region_change_fade_deltaslope(t_seq *x, t_seq_region *region, double delta_slope, char which_fade);
t_rect region_get_rect(t_seq *x, t_seq_region *region, t_object *patcherview, double *fadein_x, double *fadeout_x);
void region_change_filename(t_seq *x, t_seq_region *reg, t_symbol *newfilename);
void region_postprocess(t_seq *x);
void track_postprocess(t_seq *x);

// conversions
double ms_to_samps(t_seq *x, double ms);
double samps_to_ms(t_seq *x, double samps);

// display
double ms_to_pix(t_seq *x, t_object *view, double ms);
double pix_to_ms(t_seq *x, t_object *view, double pix);
double deltams_to_deltapix(t_seq *x, double deltams);
double deltapix_to_deltams(t_seq *x, double deltapix);
double samps_to_pix(t_seq *x, t_object *view, double samps);
double pix_to_samps(t_seq *x, t_object *view, double pix);
void track_to_y_pos(t_seq *x, long track_num, double *top_y, double *bottom_y, double *center_y);
long y_pos_to_track(t_seq *x, double y);

// interface & undo
t_llll *seq_get_state(t_seq *x);
void seq_set_state(t_seq *x, t_llll *ll);
void seq_clear(t_seq *x);
void seq_undo_postprocess(t_seq *x);


static t_class *seq_class;


void C74_EXPORT ext_main(void* moduleRef)
{
	common_symbols_init();
	llllobj_common_symbols_init();
	
	if (llllobj_check_version(BACH_LLLL_VERSION) || llllobj_test()) {
		error("bach: bad installation");
		return 1;
	}
	
	t_class *c = class_new("dada.seq~", (method)seq_new, (method)seq_free, sizeof(t_seq), 0L, A_GIMME, 0);
	
	c->c_flags |= CLASS_FLAG_NEWDICTIONARY;
//	jbox_initclass(c, 0);
	jbox_initclass(c, JBOX_TEXTFIELD | JBOX_FIXWIDTH);	// include textfield and Fonts attributes
	class_dspinitjbox(c);
	
	class_addmethod(c, (method)	seq_dsp64, "dsp64", A_CANT, 0);
	class_addmethod(c, (method)	seq_assist, "assist", A_CANT, 0);
	class_addmethod(c, (method)	seq_notify, "bachnotify", A_CANT, 0);
	class_addmethod(c, (method)	seq_paint,	"paint",	A_CANT, 0);
	class_addmethod(c, (method)	seq_bang,	"bang", 0);
	class_addmethod(c, (method) seq_focusgained, "focusgained", A_CANT, 0);
	class_addmethod(c, (method) seq_focuslost, "focuslost", A_CANT, 0); 
	class_addmethod(c, (method) seq_acceptsdrag_unlocked, "acceptsdrag_unlocked", A_CANT, 0);
	class_addmethod(c, (method) seq_acceptsdrag_unlocked, "acceptsdrag_locked", A_CANT, 0);
	class_addmethod(c, (method) seq_mousedown, "mousedown", A_CANT, 0);
	class_addmethod(c, (method) seq_mousedrag, "mousedrag", A_CANT, 0);
	class_addmethod(c, (method) seq_mouseup, "mouseup", A_CANT, 0);
  	class_addmethod(c, (method) seq_key, "key", A_CANT, 0);
  	class_addmethod(c, (method) seq_keyup, "keyup", A_CANT, 0);
	class_addmethod(c, (method) seq_mousemove, "mousemove", A_CANT, 0);
//	class_addmethod(c, (method) seq_mouseenter, "mouseenter", A_CANT, 0);
//	class_addmethod(c, (method) seq_mouseleave, "mouseleave", A_CANT, 0);
	class_addmethod(c, (method) seq_mousewheel, "mousewheel", A_CANT, 0); 
	class_addmethod(c, (method) seq_mousedoubleclick,	"mousedoubleclick", A_CANT, 0);


	class_addmethod(c, (method)	seq_anything, "anything", A_GIMME, 0);
	class_addmethod(c, (method)	seq_anything, "addregion", A_GIMME, 0);
	class_addmethod(c, (method)	seq_anything, "clear", A_GIMME, 0);
	class_addmethod(c, (method)	seq_anything, "list", A_GIMME, 0);
	class_addmethod(c, (method)	seq_anything, "play", A_GIMME, 0);
	class_addmethod(c, (method)	seq_anything, "stop", A_GIMME, 0);
	class_addmethod(c, (method)	seq_anything, "dump", A_GIMME, 0);


	llllobj_class_add_out_attr(c, LLLL_OBJ_UIMSP); 

// THIS LINE MAKES A MESS?	
//	dadaobj_class_init(c, LLLL_OBJ_UIMSP, DADAOBJ_ZOOM | DADAOBJ_SPLITXYZOOM | DADAOBJ_UNDO | DADAOBJ_EMBED | DADAOBJ_MOUSEHOVER);
	dadaobj_class_init(c, LLLL_OBJ_UIMSP, DADAOBJ_ZOOM | DADAOBJ_SPLITXYZOOM | DADAOBJ_CENTEROFFSET | DADAOBJ_EMBED);

	CLASS_STICKY_ATTR(c,"category",0,"Settings");

	CLASS_ATTR_LONG(c,"chans",0, t_seq, num_channels);
	CLASS_ATTR_FILTER_CLIP(c, "chans", 1, 4);
	CLASS_ATTR_STYLE_LABEL(c,"chans",0,"text","Number Of Channels");
	CLASS_ATTR_DEFAULT_SAVE_PAINT(c,"chans",0,"1");
	
/*	CLASS_ATTR_DOUBLE(c,"zoom",0, t_seq, zoom_x_perc);
	CLASS_ATTR_FILTER_CLIP(c, "zoom", 1, 100000);
	CLASS_ATTR_STYLE_LABEL(c,"zoom",0,"text","Horizontal Zoom %");
	CLASS_ATTR_DEFAULT_SAVE_PAINT(c,"zoom",0,"100"); 

	CLASS_ATTR_DOUBLE(c,"vzoom",0, t_seq, zoom_y_perc);
	CLASS_ATTR_FILTER_CLIP(c, "vzoom", 1, 100000);
	CLASS_ATTR_STYLE_LABEL(c,"vzoom",0,"text","Global Zoom %");
	CLASS_ATTR_DEFAULT_SAVE_PAINT(c,"vzoom",0,"100"); */

	CLASS_ATTR_DOUBLE(c,"playstep",0, t_seq, play_redraw_ms);
	CLASS_ATTR_FILTER_MIN(c, "playstep", 1);
	CLASS_ATTR_STYLE_LABEL(c,"playstep",0,"text","Playhead Display Step");
	CLASS_ATTR_DEFAULT_SAVE_PAINT(c,"playstep",0,"50");
	
	CLASS_STICKY_ATTR_CLEAR(c, "category");
	
	CLASS_STICKY_ATTR(c,"category",0,"Appearance");
	
	CLASS_ATTR_DOUBLE(c,"trackheight",0, t_seq, track_height);
	CLASS_ATTR_FILTER_CLIP(c, "trackheight", 10, 500);
	CLASS_ATTR_STYLE_LABEL(c,"trackheight",0,"text","Track Height");
	CLASS_ATTR_DEFAULT_SAVE_PAINT(c,"trackheight",0,"30");
	
	CLASS_ATTR_DOUBLE(c,"trackheader",0, t_seq, track_header_width);
	CLASS_ATTR_FILTER_MIN(c, "trackheader", 0);
	CLASS_ATTR_STYLE_LABEL(c,"trackheader",0,"text","Track Header Width");
	CLASS_ATTR_DEFAULT_SAVE_PAINT(c,"trackheader",0,"90");

	CLASS_ATTR_CHAR(c,"showfocus",0, t_seq, show_focus);
	CLASS_ATTR_STYLE_LABEL(c,"showfocus",0,"onoff","Show Focus");
	CLASS_ATTR_DEFAULT_SAVE_PAINT(c,"showfocus",0,"1");
	// @description Show that the object has the focus by increasing the width of the border.

	CLASS_STICKY_ATTR_CLEAR(c, "category");
	
	CLASS_STICKY_ATTR(c,"category",0,"Color");

	CLASS_ATTR_RGBA(c, "bgcolor", 0, t_seq, bg_color);
	CLASS_ATTR_DEFAULT_SAVE_PAINT(c, "bgcolor", 0, "0.9 0.9 0.9 1.");
	CLASS_ATTR_STYLE_LABEL(c, "bgcolor", 0, "rgba", "Background Color");
	CLASS_ATTR_BASIC(c, "bgcolor",0);
	// @description Background color

	
	CLASS_ATTR_RGBA(c, "fgcolor", 0, t_seq, fg_color);
	CLASS_ATTR_DEFAULT_SAVE_PAINT(c, "fgcolor", 0, "0.75 0.75 0.75 1.");
	CLASS_ATTR_STYLE_LABEL(c, "fgcolor", 0, "rgba", "Foreground Color");
	CLASS_ATTR_BASIC(c, "fgcolor",0);
	// @description Foreground color

	CLASS_ATTR_RGBA(c, "regioncolor", 0, t_seq, region_color);
	CLASS_ATTR_DEFAULT_SAVE_PAINT(c, "regioncolor", 0, "0.4 0. 0. 1.");
	CLASS_ATTR_STYLE_LABEL(c, "regioncolor", 0, "rgba", "Default Region Color");
	CLASS_ATTR_BASIC(c, "regioncolor",0);
	// @description Region color

	CLASS_ATTR_RGBA(c, "playcolor", 0, t_seq, play_color);
	CLASS_ATTR_DEFAULT_SAVE_PAINT(c, "playcolor", 0, "0.34 0.87 0.20 1.");
	CLASS_ATTR_STYLE_LABEL(c, "playcolor", 0, "rgba", "Play Color");
	// @description Play color
	
	CLASS_ATTR_RGBA(c, "selectioncolor", 0, t_seq, selection_color);
	CLASS_ATTR_DEFAULT_SAVE_PAINT(c, "selectioncolor", 0, "0.8 0. 0.8 1.");
	CLASS_ATTR_STYLE_LABEL(c, "selectioncolor", 0, "rgba", "Selection Color");
	// @description Selection color
	
	CLASS_ATTR_RGBA(c, "bordercolor", 0, t_seq, border_color);
	CLASS_ATTR_DEFAULT_SAVE_PAINT(c, "bordercolor", 0, "0.75 0.75 0.75 1.");
	CLASS_ATTR_STYLE_LABEL(c, "bordercolor", 0, "rgba", "Border Color");
	// @description Border color

	CLASS_ATTR_DOUBLE(c,"bordersize",0, t_seq, border_size);
	CLASS_ATTR_FILTER_MIN(c, "bordersize", 0);
	CLASS_ATTR_STYLE_LABEL(c,"bordersize",0,"text","Border Size");
	CLASS_ATTR_DEFAULT_SAVE_PAINT(c,"bordersize",0,"1");
	// @description Border size in pixels
	
	CLASS_STICKY_ATTR_CLEAR(c, "category");
	
	CLASS_ATTR_DEFAULT(c,"patching_rect",0, "0. 0. 700. 140.");

	class_register(CLASS_BOX, c);
	seq_class = c;
	
}

void seq_task(t_seq *x)
{
	jbox_redraw((t_jbox *)x);
	clock_fdelay(x->b_ob.d_ob.m_play.m_clock, x->b_ob.d_ob.m_play.play_step_ms);
}


void update_next_region_to_sequence(t_seq *x)
{
	// updating x->next_region_to_sequence_elem
	if (x->next_region_to_sequence_elem) {
		x->next_region_to_sequence_elem = x->next_region_to_sequence_elem->l_next;

		if (x->next_region_to_sequence_elem)
			x->next_region_to_sequence_samps = ((t_seq_region *)hatom_getobj(&x->next_region_to_sequence_elem->l_hatom))->onset_samps;
		else
			x->next_region_to_sequence_samps  = -1;

	} else {
	
		t_llllelem *elem = x->all_regions->l_head;
		while (elem) {
			long onset_samps = ((t_seq_region *)hatom_getobj(&elem->l_hatom))->onset_samps;
			if (onset_samps >= x->playhead_samps) {
				x->next_region_to_sequence_elem = elem;
				x->next_region_to_sequence_samps = onset_samps;
				break;
			}
		}
	}
	
//	if (x->next_region_to_sequence_elem)
//		llll_appendobj(x->sequenced_regions, hatom_getobj(&x->next_region_to_sequence_elem->l_hatom), 0, WHITENULL_llll);
}

void clear_region_to_sequence(t_seq *x)
{
	x->next_region_to_sequence_elem = NULL;
	x->next_region_to_sequence_samps = -1;
}


void check_played_regions(t_seq *x, long sampleframes)
{
	long start_samp = *x->play_samps;
	long end_samp = start_samp + sampleframes; 
	
	// check if new regions begin
	while (x->next_region_to_sequence_elem && end_samp > x->next_region_to_sequence_samps) {
		llll_appendobj(x->sequenced_regions, hatom_getobj(&x->next_region_to_sequence_elem->l_hatom), 0, WHITENULL_llll);
		update_next_region_to_sequence(x);
	}
	
	// check if playing is over
	if (!x->sequenced_regions->l_head && !x->next_region_to_sequence_elem) {
		x->playing = false;
		clock_unset(x->b_ob.d_ob.m_play.m_clock);
		jbox_redraw((t_jbox *)x);
	}
	
	// check if played regions have ended (but only once in a while?)
	t_llllelem *elem = x->sequenced_regions->l_head, *nextelem;
	while (elem) {
		nextelem = elem->l_next;
		t_seq_region *reg = (t_seq_region *)hatom_getobj(&elem->l_hatom);
		if (start_samp > reg->onset_samps + reg->dur_samps) // region no longer played
			llll_destroyelem(elem);
		elem = nextelem;
	}
}


// This method is called each time the perform method start. 
// This is useful for thread safety, if we need to perform some operations on the stuff used in the perform method
void start_perform64(t_seq *x, long sampleframes)
{
	if (x->todo_in_perform) { 
		switch (x->todo_in_perform) {
			case 1: // = START PLAYING
			{
				// resetting cursor
				long *cursor = x->play_samps;
				long i, start = x->playhead_samps;
				long max = MIN(sampleframes, x->play_samps_size);
				for (i = 0; i < max; i++)
					*cursor++ = start + i;
				
				update_next_region_to_sequence(x);
				
				// setting play to true
				x->playing = true;
				clock_fdelay(x->b_ob.d_ob.m_play.m_clock, x->b_ob.d_ob.m_play.play_step_ms);
			}
				break;
				
			case 2: // = STOP PLAYING
			{
				llll_clear(x->sequenced_regions);
				clear_region_to_sequence(x);

				// setting play to false
				x->playing = false;
				clock_unset(x->b_ob.d_ob.m_play.m_clock);
			}
				break;

			default:
				break;
		}
		x->todo_in_perform = 0;
	}
}


void check_regions_error(t_seq *x)
{
	post("EEEEEE – Error!");
}


void check_regions(t_seq *x)
{
	t_llllelem *tr_elem, *reg_elem;
	long count = 0, count_regions = 0;
	for (tr_elem = x->tracks->l_head; tr_elem && count < x->num_tracks; tr_elem = tr_elem->l_next, count++) {
		t_seq_track *tr = (t_seq_track *)hatom_getobj(&tr_elem->l_hatom);
		if (tr->elem != tr_elem)
			check_regions_error(x);
		for (reg_elem = tr->regions->l_head; reg_elem; reg_elem = reg_elem->l_next) {
			t_seq_region *reg = (t_seq_region *)hatom_getobj(&reg_elem->l_hatom);
			if (reg->elem != reg_elem)
				check_regions_error(x);
			if (!reg->elem)
				check_regions_error(x);
			if (!reg->elem->l_parent)
				check_regions_error(x);
			t_seq_track *track_supposed = (t_seq_track *)reg->elem->l_parent->l_thing.w_obj;
			if (track_supposed != tr)
				check_regions_error(x);
			post("Track %ld, region %ld", count, count_regions);
			count_regions++;
		}
	}
	
	if (count_regions != (long)x->all_regions->l_size) 
		check_regions_error(x);
	
	for (reg_elem = x->all_regions->l_head; reg_elem; reg_elem = reg_elem->l_next) {
		t_seq_region *reg = (t_seq_region *)hatom_getobj(&reg_elem->l_hatom);
		if (reg->all_elem != reg_elem)
			check_regions_error(x);
	}
}


void seq_perform64(t_seq *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, 
					  long sampleframes, long flags, void *userparam)
{
	start_perform64(x, sampleframes);
	
	t_llllelem *elem;
	long num_channels = x->num_channels;
	long chan, i, n = sampleframes;
	
	num_channels = MIN(num_channels, numouts);
	
	for (i = 0; i < num_channels; i++) {
		t_double *out = outs[i];
		while (n--)
			*out++ = 0.0;
	}
	
	if (x->playing) {

		check_played_regions(x, sampleframes);
		
		for (elem = x->sequenced_regions->l_head; elem; elem = elem->l_next) {
			t_seq_region *reg = (t_seq_region *)hatom_getobj(&elem->l_hatom);
			t_buffer_obj *buffer = buffer_ref_getobject(reg->buffer->buffer_reference);
			long reg_onset_samps = reg->onset_samps;
			long offset = reg->bufferstart_samps;
			long reg_dur_samps = reg->dur_samps;
			long frames, nc, max_nc;

			// fades
			long reg_fadein_samps = reg->fade_in.duration_samps, reg_fadeout_samps = reg->fade_out.duration_samps;
			double reg_fadein_slope = reg->fade_in.slope, reg_fadeout_slope = reg->fade_out.slope; 
//			long reg_fadein_type = reg->fade_in.fade_type, reg_fadeout_type = reg->fade_out.fade_type;

			t_float		*tab = buffer_locksamples(buffer);
			if (!tab)
				continue;
			
			frames = buffer_getframecount(buffer);
			nc = buffer_getchannelcount(buffer);
			max_nc = MIN(nc, num_channels);
			
			// cycle on each channel
			for (chan = 0; chan < max_nc; chan++) {
				long out_chan = region_get_parent(x, reg)->channelmap[chan];
				if (out_chan >= 0 && out_chan < num_channels) {
					t_double	*out = outs[out_chan];
					long		index;
					long *cursor = x->play_samps;
					
					n = sampleframes;
					while (n--) {
						index = *cursor++ - reg_onset_samps + offset; 
						
						if (index >= 0 && index < frames) { 
							// index lies inside buffer samples
							double contribution = tab[index * nc + out_chan];
							
							// handling fades
							if (reg_fadein_samps && index < reg_fadein_samps) 
								contribution *= rescale_with_slope(index, 0, reg_fadein_samps, 0., 1., reg_fadein_slope, false);
							if (reg_fadeout_samps && index > reg_dur_samps - reg_fadeout_samps) 
								contribution *= rescale_with_slope(index, reg_dur_samps - reg_fadeout_samps, reg_dur_samps, 1., 0., reg_fadeout_slope, false);
							
							*out += contribution;
						}
						out++;
					}
				}
			}
			buffer_unlocksamples(buffer);
		}
		
		// Increase samples cursor
		long *cursor = x->play_samps;
		long start = *cursor;
		long max = MIN(sampleframes, x->play_samps_size);
		for (i = 0; i < max; i++)
			*cursor++ += sampleframes;
	}
}


void seq_dsp64(t_seq *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
	x->sr = samplerate;
	if (!x->play_samps)
		bach_freeptr(x->play_samps);
	x->play_samps = (long *)bach_newptr(maxvectorsize * sizeof(long));
	x->play_samps_size = maxvectorsize;
	
    dsp_add64(dsp64, (t_object*)x, (t_perfroutine64)seq_perform64, 0, NULL);
}


void seq_anything(t_seq *x, t_symbol *msg, long ac, t_atom *av)
{
	dadaobj_anything(dadaobj_cast(x), msg, ac, av);

	t_llll *parsed = llllobj_parse_llll((t_object *) x, LLLL_OBJ_UIMSP, msg, ac, av, LLLL_PARSE_CLONE);
	if (parsed && parsed->l_head && hatom_gettype(&parsed->l_head->l_hatom) == H_SYM) {
		t_symbol *router = hatom_getsym(&parsed->l_head->l_hatom);
		long playing = x->playing;	// we query it outside the audio thread, conscious that we don't want to put heavy thread protection, 
									// and that'll be a small glitch in the worst case scenario 
		llll_destroyelem(parsed->l_head);
		
		if (router == _llllobj_sym_play && !playing) {
			// To avoid conflicts with the audio thread, we do this in the perform routine
			x->todo_in_perform = 1; // = play
		} else if (router == _llllobj_sym_stop && playing) { 
			// To avoid conflicts with the audio thread, we do this in the perform routine
			x->todo_in_perform = 2; // = stop
		} else if (router == gensym("addregion") && parsed->l_head && hatom_gettype(&parsed->l_head->l_hatom) == H_SYM) {
			t_symbol *filename = hatom_getsym(&parsed->l_head->l_hatom);
			double onset = 0, offset = 0, duration = -1, gain = 0, fadeindur = 0, fadeinslope = 0, fadeoutdur = 0, fadeoutslope = 0;
			t_atom_long track = 1;
			llll_destroyelem(parsed->l_head);
			llll_parseargs((t_object *)x, parsed, "iddd", gensym("track"), &track, gensym("onset"), 
						   &onset, gensym("offset"), &offset, gensym("duration"), &duration, gensym("gain"), &gain,
						   gensym("fadeindur"), &fadeindur, gensym("fadeinslope"), &fadeinslope, gensym("fadeoutdur"), &fadeoutdur,
						   gensym("fadeoutslope"), &fadeoutslope);
			region_create_and_insert(x, CLAMP(track - 1, 0, x->num_tracks), onset, filename, offset, duration, gain, fadeindur, fadeinslope, fadeoutdur, fadeoutslope);
			invalidate_static_layer_and_repaint(x);
		} else if (router == _sym_clear) {
			region_delete_all(x, parsed->l_head && hatom_gettype(&parsed->l_head->l_hatom) == H_LONG ? hatom_getlong(&parsed->l_head->l_hatom) - 1 : -1);
			invalidate_static_layer_and_repaint(x);
		} else if (router == _sym_dump) {
			t_llll *ll = seq_get_state(x);
			llllobj_outlet_llll((t_object *)x, LLLL_OBJ_UIMSP, 0, ll);
			llll_free(ll);
		} else {
			seq_set_state(x, parsed);
		}
	} else if (parsed) {
		seq_set_state(x, parsed);
	}
	llll_free(parsed);
}

void seq_assist(t_seq *x, void *b, long m, long a, char *s)
{
	if (m == ASSIST_INLET)
		sprintf(s,"(signal/anything) Index, Entire Content or Messages");
	else {
		switch (a) {	
			case 0:	sprintf(s,"Entire Content");	break;
			case 1:	sprintf(s,"Playout");	break;
			default:
				if (a == x->num_audio_outlets + 2)
					sprintf(s, "bang When Changed");
				else
					sprintf(s, "Signal Outlet %ld", a-1);
				break;
		}
	}
}
	 
t_seq *seq_new(t_symbol *s, short ac, t_atom *av)
{
	t_seq *x = NULL;
 	t_dictionary *d=NULL;
	t_object *textfield;
	long i, boxflags;

	long true_ac = attr_args_offset(ac, av);

	if (!(d=object_dictionaryarg(ac, av)))
		return NULL;
	
	if (x = (t_seq *)object_alloc(seq_class)) {
		boxflags = 0 
		| JBOX_DRAWFIRSTIN 
		| JBOX_NODRAWBOX
		| JBOX_DRAWINLAST
		//		| JBOX_TRANSPARENT	
		//		| JBOX_NOGROW
		//		| JBOX_GROWY
		| JBOX_GROWBOTH
		| JBOX_HILITE
		//		| JBOX_BACKGROUND
		//		| JBOX_DRAWBACKGROUND
		//		| JBOX_NOFLOATINSPECTOR
//		| JBOX_TEXTFIELD
		//		| JBOX_MOUSEDRAGDELTA
		;
		
		// default values of stuff
		x->sr = 44100.;
		x->play_redraw_ms = 50;
		x->num_channels = 1;
		x->b_ob.d_ob.m_zoom.zoom = build_pt(1., 1.);
		x->b_ob.d_ob.m_zoom.zoom_perc = x->b_ob.d_ob.m_zoom.zoom_y_perc = 100.;
		x->bg_color = get_grey(1);
		x->fg_color = get_grey(0.25);
		x->border_color = get_grey(0.25);
		x->region_color = build_jrgba(0.4, 0, 0, 1);
		x->play_color = build_jrgba(0.34, 0.87, 0.20, 1.);
		x->selection_color = build_jrgba(0.8, 0., 0.8, 1.);
		x->border_size = 1.;
		x->track_height = 1;
		x->track_header_width = 90;
		x->show_focus = 1;
		x->has_focus = 0;
		x->b_ob.d_ob.m_interface.send_bang_upon_undo = true;
		x->b_ob.d_ob.m_undo.max_num_steps = 50;
		x->b_ob.d_ob.m_undo.max_num_steps = 50;
		x->b_ob.d_ob.m_zoom.zoom_perc = 100;
		x->b_ob.d_ob.m_zoom.zoom = build_pt(1, 1);
		x->b_ob.d_ob.m_zoom.zoom_static_additional = build_pt(1, 1);
		x->b_ob.d_ob.m_zoom.allow_zoom = 1;
		x->b_ob.d_ob.allow_mouse_hover = 1;
		x->b_ob.d_ob.m_zoom.allow_center_shifting = 1;
		
		
/*		textfield = jbox_get_textfield((t_object *) x); 
		if (textfield) {
			textfield_set_noactivate(textfield, 1);
			textfield_set_editonclick(textfield, 0);			// set it to 0 if you don't want user to edit it in lock mode
			textfield_set_textmargins(textfield, 3, 3, 3, 3);	// margin on each side
		}
*/		
		jbox_new((t_jbox *)x, boxflags, ac, av);
		x->b_ob.r_ob.l_ob.z_box.b_firstin = (t_object *)x;
		dsp_setupjbox((t_pxjbox *)x, 1); // number of signal inlets?

		
		// Static number of audio outlets
		long i;
		char outlets[DADA_SEQ_MAX_NUM_AUDIO_OUTLETS + 10];
		x->num_audio_outlets = 1;
		if (true_ac && atom_gettype(av) == A_LONG)  {
			post("Foo: %d", atom_getlong(av));
			x->num_audio_outlets = CLAMP(atom_getlong(av), 0, DADA_SEQ_MAX_NUM_AUDIO_OUTLETS);
		}
		outlets[0] = 'b'; // last bang
		outlets[1] = '4'; // playout
		outlets[2] = '4'; // leftmost outlet
		for (i = 0; i < x->num_audio_outlets; i++)
			outlets[i+3] = 's';
		outlets[i+3] = 0; // that's all	
		
		
		attr_args_process(x, ac, av); // this must be called before llllobj_obj_setup
		
		dadaobj_pxjbox_setup((t_dadaobj_pxjbox *)x, DADAOBJ_ZOOMX | DADAOBJ_CENTEROFFSETX | DADAOBJ_UNDO | DADAOBJ_MOUSEHOVER | DADAOBJ_SELECTION | DADAOBJ_CHANGEDBANG | DADAOBJ_INSPECTOR, build_pt(0.1, 30), 2, 3, "vnafc", 2, outlets);
		dadaobj_addfunctions(dadaobj_cast(x), (dada_mousemove_fn)seq_mousemove, (method)seq_task, (method)seq_undo_postprocess, NULL, NULL, (pixel_to_dadaitem_fn)seq_pixel_to_dadaitem, (preselect_items_in_rectangle_fn)seq_preselect_items_in_rect,
							 (dadanotify_fn)seq_dadanotify);

		dadaobj_dadaitem_class_alloc(dadaobj_cast(x), (e_dadaitem_types)DADAITEM_TYPE_SEQ_REGION, gensym("region"), gensym("Region"), DADA_ITEM_ALLOC_DONT, 0, true, sizeof(t_seq_region), calcoffset(t_seq, all_regions), 1000000000, 
									 DADA_FUNC_v_oX, NULL, NULL, DADA_FUNC_X_o, NULL, NULL, (method)region_postprocess, NULL, NULL, true);
		dadaobj_dadaitem_class_add_single_set_get_func(dadaobj_cast(x), (e_dadaitem_types)DADAITEM_TYPE_SEQ_REGION, 
													   DADA_FUNC_v_diX, (method)dadaitem_set_from_llll, NULL, DADA_FUNC_X_di, (method)dadaitem_get_as_llll, NULL);

		dadaobj_dadaitem_class_alloc(dadaobj_cast(x), (e_dadaitem_types)DADAITEM_TYPE_SEQ_TRACK, gensym("track"), gensym("Track"), DADA_ITEM_ALLOC_DONT, 0, true, sizeof(t_seq_track), 0, 1000000000, 
									 DADA_FUNC_v_oX, NULL, NULL, DADA_FUNC_X_o, NULL, NULL, (method)track_postprocess, NULL, NULL, true);
		dadaobj_dadaitem_class_add_single_set_get_func(dadaobj_cast(x), (e_dadaitem_types)DADAITEM_TYPE_SEQ_TRACK, 
													   DADA_FUNC_v_diX, (method)dadaitem_set_from_llll, NULL, DADA_FUNC_X_di, (method)dadaitem_get_as_llll, NULL);
		
		// buffers
		x->num_buffers = 0;
		x->buffers = llll_get();
		
		// tracks
		x->tracks = llll_get();
		init_all_tracks(x);
		x->num_tracks = 5;	// we start with 5 tracks
		
		x->all_regions = llll_get();
		x->sequenced_regions = llll_get();
		
//		for (i = 0; i < x->num_channels; i++)
//			outlet_new((t_object *)x, "signal");

		t_bach_attr_manager *man = x->b_ob.d_ob.m_inspector.attr_manager;

		
		DECLARE_DADA_ATTR(dadaobj_cast(x), man, -1, gensym("filename"), (char *)"Filename", (e_dadaitem_types)DADAITEM_TYPE_SEQ_REGION, t_seq_region, filename, k_BACH_ATTR_SYM, 1, k_BACH_ATTR_DISPLAY_TEXT, 0, 0);
		bach_attribute_add_functions(get_dada_attribute(dadaobj_cast(x), (e_dadaitem_types)DADAITEM_TYPE_SEQ_REGION, gensym("filename")), NULL, (bach_setter_fn)seq_setattr_filename, NULL, NULL, NULL);
		DECLARE_DADA_ATTR(dadaobj_cast(x), man, -1, gensym("color"), (char *)"Color", (e_dadaitem_types)DADAITEM_TYPE_SEQ_REGION, t_seq_region, color, k_BACH_ATTR_COLOR, 1, k_BACH_ATTR_DISPLAY_COLOR, 0, 0);
		DECLARE_DADA_ATTR(dadaobj_cast(x), man, -1, gensym("onset"), (char *)"Onset", (e_dadaitem_types)DADAITEM_TYPE_SEQ_REGION, t_seq_region, onset_ms, k_BACH_ATTR_DOUBLE, 1, k_BACH_ATTR_DISPLAY_TEXT, 0, 0);
		DECLARE_DADA_ATTR(dadaobj_cast(x), man, -1, gensym("duration"), (char *)"Duration", (e_dadaitem_types)DADAITEM_TYPE_SEQ_REGION, t_seq_region, dur_ms, k_BACH_ATTR_DOUBLE, 1, k_BACH_ATTR_DISPLAY_TEXT, 0, 0);
		DECLARE_DADA_ATTR(dadaobj_cast(x), man, -1, gensym("offset"), (char *)"Soundfile Offset", (e_dadaitem_types)DADAITEM_TYPE_SEQ_REGION, t_seq_region, bufferstart_ms, k_BACH_ATTR_DOUBLE, 1, k_BACH_ATTR_DISPLAY_TEXT, 0, 0);
		DECLARE_DADA_ATTR(dadaobj_cast(x), man, -1, gensym("gain"), (char *)"Global Gain", (e_dadaitem_types)DADAITEM_TYPE_SEQ_REGION, t_seq_region, global_gain, k_BACH_ATTR_DOUBLE, 1, k_BACH_ATTR_DISPLAY_TEXT, 0, 0);
		DECLARE_DADA_ATTR_SUBSTRUCTURE(dadaobj_cast(x), man, -1, gensym("fadeindur"), (char *)"Fade In Duration", (e_dadaitem_types)DADAITEM_TYPE_SEQ_REGION, t_seq_region, fade_in, t_seq_fade, duration_ms,  k_BACH_ATTR_DOUBLE, 1, k_BACH_ATTR_DISPLAY_TEXT, 0, 0);
		DECLARE_DADA_ATTR_SUBSTRUCTURE(dadaobj_cast(x), man, -1, gensym("fadeinslope"), (char *)"Fade In Slope", (e_dadaitem_types)DADAITEM_TYPE_SEQ_REGION, t_seq_region, fade_in, t_seq_fade, slope,  k_BACH_ATTR_DOUBLE, 1, k_BACH_ATTR_DISPLAY_TEXT, 0, 0);
		DECLARE_DADA_ATTR_SUBSTRUCTURE(dadaobj_cast(x), man, -1, gensym("fadeoutdur"), (char *)"Fade Out Duration", (e_dadaitem_types)DADAITEM_TYPE_SEQ_REGION, t_seq_region, fade_out, t_seq_fade, duration_ms,  k_BACH_ATTR_DOUBLE, 1, k_BACH_ATTR_DISPLAY_TEXT, 0, 0);
		DECLARE_DADA_ATTR_SUBSTRUCTURE(dadaobj_cast(x), man, -1, gensym("fadeoutslope"), (char *)"Fade Out Slope", (e_dadaitem_types)DADAITEM_TYPE_SEQ_REGION, t_seq_region, fade_out, t_seq_fade, slope,  k_BACH_ATTR_DOUBLE, 1, k_BACH_ATTR_DISPLAY_TEXT, 0, 0);

		DECLARE_DADA_ATTR(dadaobj_cast(x), man, -1, gensym("name"), (char *)"Name", (e_dadaitem_types)DADAITEM_TYPE_SEQ_TRACK, t_seq_track, name, k_BACH_ATTR_SYM, 1, k_BACH_ATTR_DISPLAY_TEXT, 0, 0);
		DECLARE_DADA_ATTR(dadaobj_cast(x), man, -1, gensym("chanmap"), (char *)"Channel Map", (e_dadaitem_types)DADAITEM_TYPE_SEQ_TRACK, t_seq_track, channelmap, k_BACH_ATTR_LONG, 4, k_BACH_ATTR_DISPLAY_TEXT, 0, 0);
		ADD_BACH_ATTR_VARSIZE_FIELD(man, gensym("chanmap"), (e_dadaitem_types)DADAITEM_TYPE_SEQ_TRACK, t_seq, num_audio_outlets);

		
		jbox_ready((t_jbox *)x);
	}
	
	return (x);
}



void seq_setattr_filename(t_seq *x, void *obj, t_bach_attribute *attr, long ac, t_atom *av){
	if (ac && av) {
		t_symbol *newfilename = atom_getsym(av);
		region_change_filename(x, (t_seq_region *)obj, newfilename);
	} 
}



t_max_err seq_dadanotify(t_dadaobj *r_ob, t_symbol *s, t_symbol *modified, t_dadaitem *it, void *data)
{
	t_seq *x = (t_seq *)r_ob->orig_obj;
	if (s == _sym_attr_modified){
		switch (it->type) {
			case DADAITEM_TYPE_SEQ_REGION:
			{
				t_seq_region *reg = (t_seq_region *)it;
				if (modified == gensym("fadeindur")) {
					reg->fade_in.duration_samps = (long)round(ms_to_samps(x, reg->fade_in.duration_ms));
				} else if (modified == gensym("fadeoutdur")) {
					reg->fade_out.duration_samps = (long)round(ms_to_samps(x, reg->fade_out.duration_ms));
				} else if (modified == gensym("onset")) {
					reg->onset_samps = (long)round(ms_to_samps(x, reg->onset_ms));
				} else if (modified == gensym("duration")) {
					reg->dur_samps = (long)round(ms_to_samps(x, reg->dur_ms));
				}
			}
				break;
			default:
				break;
		}
	}
	return MAX_ERR_NONE;
}

void seq_bang(t_seq *x)
{
	t_seq_region *reg = region_create_and_insert(x, (long)rand_range(0, 4), rand_range(0, 5000), gensym("test2.wav"), 0, -1, 0, 1000, 0, 500, 0);
	invalidate_static_layer_and_repaint(x);
}

void seq_free_buffers(t_seq *x)
{
	t_llllelem *elem;
	long i;
	for (elem = x->buffers->l_head; elem; elem = elem->l_next) {
		t_seq_buffer *buf = (t_seq_buffer *)hatom_getobj(&elem->l_hatom);
		for (i = 0; i < 4; i++)
			if (buf->views[i].samples)
				bach_freeptr(buf->views[i].samples);
	}
	bach_freeptr(x->buffers);
}

void seq_free(t_seq *x)
{
	seq_free_buffers(x);
	dadaobj_pxjbox_free((t_dadaobj_pxjbox *)x);
	dsp_freejbox((t_pxjbox *)x);

}


t_max_err seq_notify(t_seq *x, t_symbol *s, t_symbol *msg, void *sender, void *data)
{
	if (msg == _sym_attr_modified) {
		t_symbol *attr_name = (t_symbol *)object_method((t_object *)data, _sym_getname);
		if (attr_name == _llllobj_sym_zoom) {
//			x->zoom_x = x->b_ob.d_ob.zoom_perc / 100.;
			invalidate_static_layer_and_repaint(x);
		}
		if (attr_name == _llllobj_sym_vzoom) {
//			x->zoom_y = x->zoom_y_perc / 100.;
			invalidate_static_layer_and_repaint(x);
		}
	}
	return 0;
}




// ******** SET/GET STATE
void seq_clear(t_seq *x)
{
	region_delete_all(x, -1);
}


void seq_set_state(t_seq *x, t_llll *ll)
{
	t_llllelem *elem, *subelem;
	long track_num = 0;
	
	seq_clear(x);
	x->num_tracks = ll->l_size;
	
	for (elem = ll->l_head; elem; elem = elem->l_next, track_num++) {
		if (hatom_gettype(&elem->l_hatom) == H_LLLL) {
			t_llll *track_ll = hatom_getllll(&elem->l_hatom);
			for (subelem = track_ll->l_head; subelem; subelem = subelem->l_next) {
				if (hatom_gettype(&subelem->l_hatom) == H_LLLL) {
					check_regions(x);
					t_seq_region *region = region_create_and_insert(x, track_num, 0, NULL, 0, 0, 0, 0, 0, 0, 0);
					check_regions(x);
					dadaitem_set_from_llll(dadaobj_cast(x), (t_dadaitem *)region, hatom_getllll(&subelem->l_hatom), 0);
					check_regions(x);
				}
			}
		}
	}
	check_regions(x);
	invalidate_static_layer_and_repaint(x);
}


t_llll *seq_get_state(t_seq *x)
{
	t_llllelem *track, *region;
	t_llll *out_ll = llll_get();
	long track_count = 0;
	for (track = x->tracks->l_head; track && track_count < x->num_tracks; track = track->l_next, track_count++) {
		t_llll *t_ll = llll_get();
		t_seq_track *t = (t_seq_track *)hatom_getobj(&track->l_hatom);
		for (region = t->regions->l_head; region; region = region->l_next) {
			t_seq_region *r = (t_seq_region *)hatom_getobj(&region->l_hatom);
			llll_appendllll(t_ll, dadaitem_get_as_llll(dadaobj_cast(x), (t_dadaitem *)r), 0, WHITENULL_llll);
		}
		llll_appendllll(out_ll, t_ll, 0, WHITENULL_llll);
	}
	return out_ll;
}





// pixels_per_second is a zoom parameter
void paint_buffer(t_seq *x, t_jgraphics *g, t_pt init, double max_height, t_seq_buffer *buf, 
				  long ms_start, long ms_duration, double pixels_per_second, t_jrgba *color) 
{
	if (!buf)
		return;
	
	double pixel_dist_ms = 1000. / pixels_per_second;
	double sample_start = buffer_ms_to_samps(buf, ms_start), sample_end = buffer_ms_to_samps(buf, ms_start + ms_duration);

	// choose appropriate view zoom
	long view_idx = 3;
	if (pixel_dist_ms < DADA_SEQ_BUFFER_SAMPLES_PER_MS_VIEW1)
		view_idx = 0;
	else if (pixel_dist_ms < DADA_SEQ_BUFFER_SAMPLES_PER_MS_VIEW2)
		view_idx = 1;
	else if (pixel_dist_ms < DADA_SEQ_BUFFER_SAMPLES_PER_MS_VIEW3)
		view_idx = 2;
	
	double resample_ratio = pixel_dist_ms / buf->views[view_idx].sample_dist_ms;
	
	// actually paint the buffer
	long i;
	t_seq_buffer_view *view = &buf->views[view_idx];
	long num_samples_ceil = view->num_samples_ceil;
	float *samples = view->samples;
	double cur, start_cur = sample_start / view->subsampling, end_cur = MIN(sample_end / view->subsampling, num_samples_ceil);
	
	for (i = init.x, cur = start_cur; cur < end_cur; i++, cur += resample_ratio) {
		// resampling data
		long cur_floor = floor(cur);
//		double diff = cur - cur_floor;
		double val;
		if (cur_floor + 1 < num_samples_ceil)
//			val = samples[cur_floor] * (1 - diff) + samples[cur_floor + 1] * diff;
			val = MAX(samples[cur_floor], samples[cur_floor + 1]);
		else
			val = samples[cur_floor];
		val *= max_height;
		paint_line(g, *color, i, init.y - val, i, init.y + val, 1);
	}
	
	// TO DO: WE COULD PAINT ONLY UPPER HALF OF THE WAVE, and mirror the down half
}


void paint_region(t_seq *x, t_jgraphics *g, t_object *patcherview, t_seq_region *region, long track_num, t_jfont *jf_region_name)
{
	double fade_in_x, fade_out_x; 
	t_rect region_rect = region_get_rect(x, region, patcherview, &fade_in_x, &fade_out_x);
	double track_top_y = region_rect.y, track_bottom_y = region_rect.y + region_rect.height, track_middle_y = (track_top_y + track_bottom_y)/2.; 
	long selected = dadaitem_is_preselected_xor_selected((t_dadaitem *)region);
	t_jrgba region_color = selected ? x->selection_color : region->color;
	t_jrgba region_bg_color = change_alpha(selected ? x->selection_color : region->color, (selected ? x->selection_color.alpha : region->color.alpha) * 0.25);
	double waveform_max_height = MAX(0, region_rect.height / 2. - 2);

	paint_rect(g, &region_rect, NULL, &region_bg_color, 0, 0);
	paint_buffer(x, g, build_pt(region_rect.x, track_middle_y), waveform_max_height, region->buffer, 
				 region->bufferstart_ms, region->dur_ms, DADA_SEQ_DEFAULT_PIXELS_PER_SECOND * x->b_ob.d_ob.m_zoom.zoom.x, &x->region_color);
	paint_rect(g, &region_rect, &region_color, NULL, 2, 0);
	
	// paint fades
	if (region->fade_in.duration_samps > 0) {
		paint_curve(g, region_color, region_rect.x, track_bottom_y, fade_in_x, track_top_y, region->fade_in.slope, 1);
		paint_line(g,region_color , fade_in_x, track_top_y, fade_in_x, track_bottom_y, 1);
	}
	if (region->fade_out.duration_samps > 0) {
		paint_curve(g, region_color, fade_out_x, track_top_y, region_rect.x + region_rect.width, track_bottom_y, region->fade_out.slope, 1);
		paint_line(g,region_color , fade_out_x, track_top_y, fade_out_x, track_bottom_y, 1);
	}
	
//	region->rect = region_rect;
	
	if (jf_region_name) 
		write_text(g, jf_region_name, x->fg_color, region->filename->s_name, region_rect.x + 2, region_rect.y + 2, region_rect.width - 4, region_rect.height - 4, JGRAPHICS_TEXT_JUSTIFICATION_TOP + JGRAPHICS_TEXT_JUSTIFICATION_RIGHT, true, true);
}

void paint_static_stuff(t_seq *x, t_object *view, t_rect *rect, t_jfont *jf_track_names, t_jfont *jf_region_names)
{
	t_jgraphics *g = jbox_start_layer((t_object *)x, view, gensym("static_layer1"), rect->width, rect->height);
	
	if (g){
		t_llllelem *tr, *re;
		double track_header_width = x->track_header_width; 

		// painting tracks
		double cur_y = 0.5;
		long track_num = 0;
		double pad_for_track_name = 5;
		double track_height = x->track_height * x->b_ob.d_ob.m_zoom.zoom.y;

		// painting track regions
		for (tr = x->tracks->l_head, track_num = 0; tr && track_num < x->num_tracks; tr = tr->l_next, track_num++) {
			t_seq_track *track = (t_seq_track *)hatom_getobj(&tr->l_hatom);
			for (re = track->regions->l_head; re; re = re->l_next) {
				t_seq_region *region = (t_seq_region *)hatom_getobj(&re->l_hatom);
				paint_region(x, g, view, region, track_num, jf_region_names);
			}
			
		}
		
		// painting track header
		paint_rectangle(g, DADA_GREY_10, DADA_GREY_10, 0, 0, track_header_width, rect->height, 0);
//		paint_line(g, x->fg_color, track_header_width, 0, track_header_width, rect->height, 2);

		for (cur_y = 0.5, tr = x->tracks->l_head, track_num = 0; tr && track_num < x->num_tracks; tr = tr->l_next, track_num++) {
			t_seq_track *track = (t_seq_track *)hatom_getobj(&tr->l_hatom);
			long selected = dadaitem_is_preselected_xor_selected((t_dadaitem *)track);

			if (selected)
				paint_rectangle(g, DADA_GREY_10, x->selection_color, 0, cur_y, track_header_width, track_height, 0);

			write_text(g, jf_track_names, DADA_GREY_90, track->name->s_name, 0 + pad_for_track_name, cur_y, track_header_width - 2 * pad_for_track_name, track_height, JGRAPHICS_TEXT_JUSTIFICATION_VCENTERED + JGRAPHICS_TEXT_JUSTIFICATION_LEFT, true, true);
			
			cur_y += track_height;
			paint_line(g, DADA_GREY_50, 0, cur_y, rect->width, cur_y, 0.5);
		}
			
		jbox_end_layer((t_object *)x, view, gensym("static_layer1"));
	}
	
	jbox_paint_layer((t_object *)x, view, gensym("static_layer1"), 0., 0.);	// position of the layer
}

void invalidate_static_layer_and_repaint(t_seq *x)
{
	jbox_invalidate_layer((t_object *)x, NULL, gensym("static_layer1"));
	jbox_redraw((t_jbox *) x);
}

void check_center(t_seq *x, t_object *patcherview, t_rect rect)
{
	double max_coord = -delta_pix_to_delta_coord(dadaobj_cast(x), build_pt(rect.width / 2. - x->track_header_width, 0.)).x;
	if (x->b_ob.d_ob.m_zoom.center_offset.x > max_coord)
		x->b_ob.d_ob.m_zoom.center_offset.x = max_coord;
}

void seq_paint(t_seq *x, t_object *patcherview)
{
	t_rect rect, rect_00;
	t_jgraphics *g = (t_jgraphics*) patcherview_get_jgraphics(patcherview);		// obtain graphics context
	jbox_get_rect_for_view((t_object *)x, patcherview, &rect);
	
	t_jfont *jf_track_names = jfont_create_debug("Arial", JGRAPHICS_FONT_SLANT_NORMAL, JGRAPHICS_FONT_WEIGHT_BOLD, 12);
	t_jfont *jf_region_names = jfont_create_debug("Arial", JGRAPHICS_FONT_SLANT_NORMAL, JGRAPHICS_FONT_WEIGHT_BOLD, 9);

	check_center(x, patcherview, rect);
	
	rect_00 = build_rect(0, 0, rect.width, rect.height);
	
	paint_background((t_object *)x, g, &rect, &x->bg_color, 0);
	
	paint_static_stuff(x, patcherview, &rect, jf_track_names, jf_region_names);
	
	if (x->b_ob.d_ob.m_interface.mouse_is_down && x->b_ob.d_ob.m_tools.curr_tool == DADA_TOOL_SELECT) {
		t_jrgba border = change_alpha(x->selection_color, 0.5);
		t_jrgba inner = change_alpha(x->selection_color, 0.1);
		dadaobj_paint_selection_rectangle(dadaobj_cast(x), g, &border, &inner);
	}
	
	if (x->playing) {	// ok, we are reading the "playing" field outside the audio thread
						// however we're not willing to put heavy thread protections for such a small glitch
		double pix = samps_to_pix(x, patcherview, *x->play_samps);
		paint_playhead(g, x->play_color, pix, 10, rect.height - 10, 1., 3 * x->b_ob.d_ob.m_zoom.zoom.y);
	}
	
	paint_border((t_object *)x, g, &rect, &x->border_color, x->border_size * (x->has_focus ? 2.5 : 1), 0);
	
	jfont_destroy(jf_track_names);
	jfont_destroy(jf_region_names);
}



///// MOUSE AND KEYBOARD INTERFACE


void seq_focusgained(t_seq *x, t_object *patcherview) {
	x->has_focus = true;
	jbox_redraw((t_jbox *)x);
}

void seq_focuslost(t_seq *x, t_object *patcherview) {
	x->has_focus = false;
	jbox_redraw((t_jbox *)x);
}

long seq_acceptsdrag_unlocked(t_seq *x, t_object *drag, t_object *view)
{
	if (jdrag_matchdragrole(drag, gensym("audiofile"), 0)) {
		jdrag_box_add(drag, (t_object *)x, gensym("addregion"));
		return true;
	}
	return false;
}


t_dadaitem *seq_pixel_to_dadaitem(t_dadaobj *r_ob, t_pt pt, t_object *patcherview, long modifiers, t_pt *coordinates, double selection_pad, t_dadaitem_identifier *identifier)
{
	t_seq *x = (t_seq *)r_ob->orig_obj;

	if (pt.x < x->track_header_width) {
		// tracks
		long track = y_pos_to_track(x, pt.y);
		if (track >= 0 && track < x->num_tracks)
			return (t_dadaitem *)track_nth(x, track);
	} else {
		// regions?
		t_llllelem *elem;
		for (elem = x->all_regions->l_head; elem; elem = elem->l_next) {
			t_seq_region *reg = (t_seq_region *)hatom_getobj(&elem->l_hatom);
			double fade_in_x, fade_out_x;
			t_rect reg_rect = region_get_rect(x, reg, patcherview, &fade_in_x, &fade_out_x);
			if (is_pt_in_rectangle(pt, build_rect(reg_rect.x - DADA_SEQ_SELECTION_THRESH, reg_rect.y, 2 * DADA_SEQ_SELECTION_THRESH, reg_rect.height))) {
				reg->mousedown_portion = (x->b_ob.d_ob.m_tools.curr_tool == DADA_TOOL_FADE ? -2 : -1);
				return (t_dadaitem *)reg;
			}
			
			if (is_pt_in_rectangle(pt, build_rect(reg_rect.x + reg_rect.width - DADA_SEQ_SELECTION_THRESH, reg_rect.y, 2 * DADA_SEQ_SELECTION_THRESH, reg_rect.height))) {
				reg->mousedown_portion = (x->b_ob.d_ob.m_tools.curr_tool == DADA_TOOL_FADE ? 2 : 1);
				return (t_dadaitem *)reg;
			}
			
			if (reg->fade_in.duration_ms > 0) {
				if (is_pt_in_rectangle(pt, build_rect(fade_in_x - DADA_SEQ_SELECTION_THRESH, reg_rect.y, 2 * DADA_SEQ_SELECTION_THRESH, reg_rect.height))) {
					reg->mousedown_portion = -2;
					return (t_dadaitem *)reg;
				}
				if (x->b_ob.d_ob.m_tools.curr_tool == DADA_TOOL_CURVE && is_pt_in_rectangle(pt, build_rect(reg_rect.x, reg_rect.y, fade_in_x - reg_rect.x, reg_rect.height))) {
					reg->mousedown_portion = -3;
					return (t_dadaitem *)reg;
				}
			}
			
			if (reg->fade_out.duration_ms > 0) {
				if (is_pt_in_rectangle(pt, build_rect(fade_out_x - DADA_SEQ_SELECTION_THRESH, reg_rect.y, 2 * DADA_SEQ_SELECTION_THRESH, reg_rect.height))) {
					reg->mousedown_portion = 2;
					return (t_dadaitem *)reg;
				}
				if (x->b_ob.d_ob.m_tools.curr_tool == DADA_TOOL_CURVE && is_pt_in_rectangle(pt, build_rect(fade_out_x, reg_rect.y, reg_rect.x + reg_rect.width - fade_out_x, reg_rect.height))) {
					reg->mousedown_portion = 3;
					return (t_dadaitem *)reg;
				}
			}
			
			if (is_pt_in_rectangle(pt, reg_rect)) {
				reg->mousedown_portion = 0;
				return (t_dadaitem *)reg;
			}
		}
	}
	
	return NULL;
}


void seq_mousedown(t_seq *x, t_object *patcherview, t_pt pt, long modifiers){

	if (dadaobj_mousedown(dadaobj_cast(x), patcherview, pt, modifiers))
		invalidate_static_layer_and_repaint(x);
	
	x->curr_change_track_num = 0;
	
	if (modifiers & ePopupMenu) {
		
	} else {
		llll_format_modifiers(&modifiers, NULL);
		
		t_dadaitem *it = x->b_ob.d_ob.m_interface.mousedown_item;
		
		if (!(modifiers & eShiftKey || (it && it->selected))) {
			dadaobj_selection_clear_selection(dadaobj_cast(x));
			dadaobj_selection_clear_preselection(dadaobj_cast(x));
			invalidate_static_layer_and_repaint(x);
		}
		
		if (it) {
			switch (it->type) {
				case DADAITEM_TYPE_SEQ_REGION:
					if (modifiers == eCommandKey)  {
						region_delete(x, (t_seq_region *)it);
						invalidate_static_layer_and_repaint(x);
					}
					break;
				case DADAITEM_TYPE_SEQ_TRACK:
					break;
				default:
					break;
			}
		}
	}
	
	jbox_redraw((t_jbox *)x);
	
}

void seq_mouseup(t_seq *x, t_object *patcherview, t_pt pt, long modifiers){
	
	x->just_duplicated = false;

	check_regions(x);

	if (dadaobj_mouseup(dadaobj_cast(x), patcherview, pt, modifiers)) {
		check_regions(x);
		return;
	}
}


char seq_find_selected_region_leftmost_onset_fn(t_dadaobj *r_ob, t_dadaitem *item, void *data)
{
	if (item->type == (e_dadaitem_types)DADAITEM_TYPE_SEQ_REGION) {
		t_seq_region *region = (t_seq_region *)item;
		if (region->onset_ms < *((double *)data))
			*((double *)data) = region->onset_ms;
	}
	return 0;
}


char seq_move_region_fn(t_dadaobj *r_ob, t_dadaitem *item, void *data)
{
	if (item->type == (e_dadaitem_types)DADAITEM_TYPE_SEQ_REGION) {
		t_seq_region *region = (t_seq_region *)item;
		region_move_deltams((t_seq *)r_ob->orig_obj, region, *((double *)data));
	}
	return 0;
}

char seq_change_region_track_diff_fn(t_dadaobj *r_ob, t_dadaitem *item, void *data)
{
	t_seq *x = (t_seq *)r_ob->orig_obj;
	if (item->type == (e_dadaitem_types)DADAITEM_TYPE_SEQ_REGION) {
		t_seq_region *region = (t_seq_region *)item;
		t_seq_track *parent = region_get_parent(x, region);
		if (parent) {
			long new_track = parent->number + *((long *)data);
			new_track = CLAMP(new_track, 0, x->num_tracks - 1);
			region_change_track(x, region, new_track);
		} else {
			post("ERROR!");
		}
	}
	return 0;
}

char seq_change_region_length_fn(t_dadaobj *r_ob, t_dadaitem *item, void *data)
{
	if (item->type == (e_dadaitem_types)DADAITEM_TYPE_SEQ_REGION) {
		t_seq_region *region = (t_seq_region *)item;
		region_change_length_deltams((t_seq *)r_ob->orig_obj, region, *((double *)data));
	}
	return 0;
}


char seq_change_region_offset_fn(t_dadaobj *r_ob, t_dadaitem *item, void *data)
{
	if (item->type == (e_dadaitem_types)DADAITEM_TYPE_SEQ_REGION) {
		t_seq_region *region = (t_seq_region *)item;
		region_change_offset_deltams((t_seq *)r_ob->orig_obj, region, *((double *)data));
	}
	return 0;
}


char seq_change_fadein_pos_fn(t_dadaobj *r_ob, t_dadaitem *item, void *data)
{
	if (item->type == (e_dadaitem_types)DADAITEM_TYPE_SEQ_REGION) {
		t_seq_region *region = (t_seq_region *)item;
		region_change_fade_deltams((t_seq *)r_ob->orig_obj, region, *((double *)data), -1);
	}
	return 0;
}

char seq_change_fadeout_pos_fn(t_dadaobj *r_ob, t_dadaitem *item, void *data)
{
	if (item->type == (e_dadaitem_types)DADAITEM_TYPE_SEQ_REGION) {
		t_seq_region *region = (t_seq_region *)item;
		region_change_fade_deltams((t_seq *)r_ob->orig_obj, region, -*((double *)data), 1);
	}
	return 0;
}

char seq_change_fadein_slope_fn(t_dadaobj *r_ob, t_dadaitem *item, void *data)
{
	if (item->type == (e_dadaitem_types)DADAITEM_TYPE_SEQ_REGION) {
		t_seq_region *region = (t_seq_region *)item;
		region_change_fade_deltaslope((t_seq *)r_ob->orig_obj, region, *((double *)data), -1);
	}
	return 0;
}

char seq_change_fadeout_slope_fn(t_dadaobj *r_ob, t_dadaitem *item, void *data)
{
	if (item->type == (e_dadaitem_types)DADAITEM_TYPE_SEQ_REGION) {
		t_seq_region *region = (t_seq_region *)item;
		region_change_fade_deltaslope((t_seq *)r_ob->orig_obj, region, *((double *)data), 1);
	}
	return 0;
}

char seq_preselect_items_in_rect(t_dadaobj *r_ob, t_object *view, t_rect rect)
{
	t_llllelem *elem;
	t_seq *x = (t_seq *)r_ob->orig_obj;
	double x_min = MIN(r_ob->m_interface.mousedrag_pix.x, r_ob->m_interface.mousedown_pix.x);
	double x_max = MAX(r_ob->m_interface.mousedrag_pix.x, r_ob->m_interface.mousedown_pix.x);
	double y_min = MIN(r_ob->m_interface.mousedrag_pix.y, r_ob->m_interface.mousedown_pix.y);
	double y_max = MAX(r_ob->m_interface.mousedrag_pix.y, r_ob->m_interface.mousedown_pix.y);
	double start_ms = pix_to_ms(x, view, x_min), end_ms = pix_to_ms(x, view, x_max);
	long start_voice = CLAMP(y_pos_to_track(x, y_min), 0, x->num_tracks), end_voice = CLAMP(y_pos_to_track(x, y_max), 0, x->num_tracks);
	
	for (elem = x->all_regions->l_head; elem; elem = elem->l_next) {
		t_seq_region *reg = (t_seq_region *)hatom_getobj(&elem->l_hatom);
		t_seq_track *track = region_get_parent(x, reg);
		if (reg && track && 
			reg->onset_ms <= end_ms && start_ms <= reg->onset_ms + reg->dur_ms &&
			track->number >= start_voice && track->number <= end_voice) 
				dadaobj_selection_preselect_item(r_ob, (t_dadaitem *)reg, k_SELECTION_MODE_FORCE_SELECT);
		else if (reg->onset_ms > end_ms)
			break;
	}
	return 0;
}


void seq_mousedrag(t_seq *x, t_object *patcherview, t_pt pt, long modifiers){
	
	llll_format_modifiers(&modifiers, NULL);

	if (dadaobj_mousedrag(dadaobj_cast(x), patcherview, pt, modifiers)) {
		invalidate_static_layer_and_repaint(x);
		return;
	}
	
	if (x->b_ob.d_ob.m_interface.mousedown_item) {
		switch (x->b_ob.d_ob.m_interface.mousedown_item->type) {
			case DADAITEM_TYPE_SEQ_REGION:
			{
				t_seq_region *reg = (t_seq_region *)x->b_ob.d_ob.m_interface.mousedown_item;
				double onset_shift = deltapix_to_deltams(x, x->b_ob.d_ob.m_interface.mousedrag_delta_pix.x);
				post("reg->mousedown_portion: %ld", reg->mousedown_portion);
				switch (reg->mousedown_portion) {
					case -1: // start point dragged 
						dadaobj_selection_iterate(dadaobj_cast(x), (dadaitem_iterfn)seq_change_region_offset_fn, &onset_shift);
						break;
					case 1: // end point dragged 
						dadaobj_selection_iterate(dadaobj_cast(x), (dadaitem_iterfn)seq_change_region_length_fn, &onset_shift);
						break;
					case -2: // fadein point dragged  
						dadaobj_selection_iterate(dadaobj_cast(x), (dadaitem_iterfn)seq_change_fadein_pos_fn, &onset_shift);
						break;
					case 2: // fadeout point dragged 
						dadaobj_selection_iterate(dadaobj_cast(x), (dadaitem_iterfn)seq_change_fadeout_pos_fn, &onset_shift);
						break;
					case -3: // fadein region dragged  
					{
						double delta_slope = x->b_ob.d_ob.m_interface.mousedrag_delta_pix.y / 100.; 
						dadaobj_selection_iterate(dadaobj_cast(x), (dadaitem_iterfn)seq_change_fadein_slope_fn, &delta_slope);
					}
						break;
					case 3: // fadeout region dragged 
					{
						double delta_slope = -x->b_ob.d_ob.m_interface.mousedrag_delta_pix.y / 100.; 
						dadaobj_selection_iterate(dadaobj_cast(x), (dadaitem_iterfn)seq_change_fadeout_slope_fn, &delta_slope);
					}
						break;
					default: // body dragged
					{
						// cloning?
						if (modifiers == eAltKey && !x->just_duplicated) {
							t_llllelem *elem;
							t_llll *toprocess = llll_get();
							for (elem = x->b_ob.d_ob.m_interface. selection->l_head; elem; elem = elem->l_next) {
								t_dadaitem *item = (t_dadaitem *)hatom_getobj(&elem->l_hatom);
								if (item->type == DADAITEM_TYPE_SEQ_REGION)
									llll_appendobj(toprocess, item, 0, WHITENULL_llll);
							}
							for (elem = toprocess->l_head; elem; elem = elem->l_next) 
								region_clone(x, (t_seq_region *)hatom_getobj(&elem->l_hatom), true);

							x->just_duplicated = true;
							llll_free(toprocess);
						}
						
						// move onsets
						if (x->b_ob.d_ob.m_interface.main_drag_direction >= 0) {
							double leftmost_onset = 1000000000;
							dadaobj_selection_iterate(dadaobj_cast(x), (dadaitem_iterfn)seq_find_selected_region_leftmost_onset_fn, &leftmost_onset);
							if (leftmost_onset + onset_shift < 0)
								onset_shift -= (leftmost_onset + onset_shift);
							dadaobj_selection_iterate(dadaobj_cast(x), (dadaitem_iterfn)seq_move_region_fn, &onset_shift);
						}
						
						if (x->b_ob.d_ob.m_interface.main_drag_direction <= 0) {
							// change tracks
							long delta_tracks = round((x->b_ob.d_ob.m_interface.mousedrag_pix.y - x->b_ob.d_ob.m_interface.mousedown_pix.y) / (x->track_height * x->b_ob.d_ob.m_zoom.zoom.y) - x->curr_change_track_num);
							if (delta_tracks) {
								dadaobj_selection_iterate(dadaobj_cast(x), (dadaitem_iterfn)seq_change_region_track_diff_fn, &delta_tracks);
								x->curr_change_track_num += delta_tracks;
							}
						}
					}
						break;
				}
				invalidate_static_layer_and_repaint(x);
			}
				break;
			default:
				break;
		}
	}
}

void seq_mousemove(t_seq *x, t_object *patcherview, t_pt pt, long modifiers) {
	llll_format_modifiers(&modifiers, NULL);

	if (dadaobj_mousemove(dadaobj_cast(x), patcherview, pt, modifiers))
		return;

//	t_dadaitem *it = x->b_ob.d_ob.m_interface.mousemove_item = x->b_ob.d_ob.m_interface.mousedown_item = seq_pixel_to_dadaitem(x, pt, patcherview, modifiers);

	if (popup_menu_is_shown(dadaobj_cast(x))) {
		dada_set_cursor(dadaobj_cast(x), patcherview, BACH_CURSOR_DEFAULT);
	} else {
		t_dadaitem *item = x->b_ob.d_ob.m_interface.mousemove_item;
		if (item) {
			switch (item->type) {
				case DADAITEM_TYPE_SEQ_REGION:
					if (modifiers == eAltKey)
						dada_set_cursor(dadaobj_cast(x), patcherview, BACH_CURSOR_DUPLICATE);
					else if (modifiers == eCommandKey)
						dada_set_cursor(dadaobj_cast(x), patcherview, BACH_CURSOR_DELETE);
					else {
						switch (((t_seq_region *)item)->mousedown_portion) {
							case 1:
							case -1:
							case 2:
							case -2:
								dada_set_cursor(dadaobj_cast(x), patcherview, BACH_CURSOR_RESIZE_LEFTRIGHT);
								break;
							case 3:
							case -3:
								if (x->b_ob.d_ob.m_tools.curr_tool == DADA_TOOL_CURVE)
									dada_set_cursor(dadaobj_cast(x), patcherview, BACH_CURSOR_CURVE);
								else
									dada_set_cursor(dadaobj_cast(x), patcherview, BACH_CURSOR_POINTINGHAND);
								break;
							default:
								dada_set_cursor(dadaobj_cast(x), patcherview, BACH_CURSOR_POINTINGHAND);
								break;
						}
					}
					break;
				case DADAITEM_TYPE_SEQ_TRACK:
					dada_set_cursor(dadaobj_cast(x), patcherview, BACH_CURSOR_DEFAULT);
					break;
				default:
					dada_set_cursor(dadaobj_cast(x), patcherview, BACH_CURSOR_CROSSHAIR);
					break;
			}
		} else 
			dada_set_cursor(dadaobj_cast(x), patcherview, BACH_CURSOR_DEFAULT);
	}
}

void seq_mousedoubleclick(t_seq *x, t_object *patcherview, t_pt pt, long modifiers) 
{
	if (x->b_ob.d_ob.m_interface.mousedown_item && x->b_ob.d_ob.m_interface.mousedown_item->type == DADAITEM_TYPE_SEQ_REGION) 
		buffer_view(((t_seq_region *)x->b_ob.d_ob.m_interface.mousedown_item)->buffer->buffer);
}

void seq_mousewheel(t_seq *x, t_object *view, t_pt pt, long modifiers, double x_inc, double y_inc){
	llll_format_modifiers(&modifiers, NULL);  
	double old_zoom = x->b_ob.d_ob.m_zoom.zoom.x;
	t_pt old_center = x->b_ob.d_ob.m_zoom.center_offset;
	t_pt mousemove_coord = x->b_ob.d_ob.m_interface.mousemove_coord;
	
	if (dadaobj_mousewheel(dadaobj_cast(x), view, pt, modifiers, x_inc, y_inc)) {
		invalidate_static_layer_and_repaint(x);
		if (x->b_ob.d_ob.m_zoom.zoom.x != old_zoom) {
			// zoom has changed, let's preserve center position
			t_pt temp = pt_number_prod(mousemove_coord, x->b_ob.d_ob.m_zoom.zoom.x/old_zoom);
			t_pt new_center = pt_pt_diff(mousemove_coord, temp);
			double vals[2]; vals[0] = new_center.x; vals[1] = new_center.y;
			object_attr_setdouble_array(x, gensym("center"), 2, vals);
			//TODO
		}
		return;
	} 

/*	if (modifiers == eCommandKey || modifiers == eCommandKey + eShiftKey) {
		double new_zoom_perc = x->zoom_x_perc - 100 * y_inc * 3 * (modifiers & eShiftKey ? DADA_FINER_FROM_KEYBOARD : 1.);
		new_zoom_perc = CLAMP(new_zoom_perc, DADA_MIN_ZOOM_PERC, DADA_MAX_ZOOM_PERC);
		object_attr_setfloat((t_object *)x, gensym("zoom"), new_zoom_perc);
		
		invalidate_static_layer_and_repaint(x);
	} else {
		double delta = x_inc * 5;
		if (delta < -x->screen_ms_start)
			delta = -x->screen_ms_start;
		x->screen_ms_start += delta;
		x->screen_ms_end += delta;
		invalidate_static_layer_and_repaint(x);
	} */
}


long seq_keyup(t_seq *x, t_object *patcherview, long keycode, long modifiers, long textcharacter){
	
	llll_format_modifiers(&modifiers, &keycode);

	if (dadaobj_keyup(dadaobj_cast(x), patcherview, keycode, modifiers, textcharacter))
		return 1;

	return 0;
}


long seq_key(t_seq *x, t_object *patcherview, long keycode, long modifiers, long textcharacter)
{
	llll_format_modifiers(&modifiers, &keycode);
	
	if (dadaobj_key(dadaobj_cast(x), patcherview, keycode, modifiers, textcharacter))
		return 1;
	
	switch (keycode) {
		case JKEY_BACKSPACE:	// play/stop
			region_delete_selected(x);
			invalidate_static_layer_and_repaint(x);
			return 1;
		case JKEY_SPACEBAR:	// play/stop
			if (x->playing)
				x->todo_in_perform = 2; // = stop
			else
				x->todo_in_perform = 1; // = play
			return 1;
		default:
			break;
	}
	
	return 0;
}


/// ******************************************** /// 
/// BUFFERS, TRACKS AND REGION INTERFACE
/// ******************************************** /// 

/// BUFFERS

t_seq_buffer *buffer_new(t_seq *x, t_symbol *buffername)
{
	t_seq_buffer *buf = (t_seq_buffer *)bach_newptrclear(sizeof(t_seq_buffer));

	dadaitem_init(dadaobj_cast(x), (t_dadaitem *)buf, (e_dadaitem_types)DADAITEM_TYPE_SEQ_BUFFER, 0, true, 0);

	t_atom av;
	atom_setsym(&av, buffername);
	buf->buffer = (t_object *) object_new_typed(CLASS_BOX, gensym("buffer~"), 1, &av);
	if (buf->buffer) {
		buf->buffer_reference = buffer_ref_new((t_object*)x, buffername);
		buf->sr = buffer_getsamplerate(buf->buffer);
	}
	
	buf->reference_count = 0;
	buf->buffername = buffername;
	x->num_buffers++;
	buf->elem = llll_appendobj(x->buffers, buf, 0, WHITENULL_llll);
	return buf;
}

void buffer_create_view(t_seq *x, t_seq_buffer *buffer, long view_idx, double sample_dist_ms)
{
	t_seq_buffer_view *view = &buffer->views[view_idx];
	
	if (view->samples)
		bach_freeptr(view->samples);
	
	t_atom_long num_frames = buffer_getframecount(buffer->buffer);
	double sr = buffer_getsamplerate(buffer->buffer);
	double duration_sec = num_frames / sr;
	double sample_dist_sec = sample_dist_ms / 1000.; 
	double num_view_samples = duration_sec / sample_dist_sec;
	long num_view_samples_ceil = ceil(num_view_samples);
	double subsampling = sr * sample_dist_sec;
	view->num_samples = num_view_samples;
	view->num_samples_ceil = num_view_samples_ceil;
	view->samples = (float *)bach_newptr(num_view_samples_ceil * sizeof(float));
	view->sample_dist_ms = sample_dist_ms;
	view->subsampling = subsampling;
	
	long i, j, k = 1;
	float *v = view->samples;
	t_float	*tab = buffer_locksamples(buffer->buffer);
	for (i = 0, j = 0; i < num_view_samples_ceil && j < num_frames; v++) {
		float abs_max = 0;
		double k_subs = k * subsampling;
		while (j < k_subs && j < num_frames) {
			if (*tab > abs_max)
				abs_max = *tab;
			else if (*tab < -abs_max)
				abs_max = -(*tab);
			tab++;
			j++;
		}
		k++;
		*v = abs_max;
		i++;
	} 
	buffer_unlocksamples(buffer->buffer);
	
}

void buffer_fill(t_seq *x, t_seq_buffer *buf, t_symbol *filename)
{
	t_atom av, rv;
	t_buffer_obj *buffer = buf->buffer;
	t_symbol *fp = dada_ezlocate_file(filename, NULL);
	atom_setsym(&av, fp);
	object_method_typed(buffer, gensym("import"), 1, &av, &rv);
	
	buf->filename = filename;
	buf->filename_fullpath = fp;
	buf->duration_samps = buffer_getframecount(buf->buffer);
	buf->duration_ms = buffer_samps_to_ms(buf, buf->duration_samps);
	
	buffer_create_view(x, buf, 0, DADA_SEQ_BUFFER_SAMPLES_PER_MS_VIEW0);
	buffer_create_view(x, buf, 1, DADA_SEQ_BUFFER_SAMPLES_PER_MS_VIEW1);
	buffer_create_view(x, buf, 2, DADA_SEQ_BUFFER_SAMPLES_PER_MS_VIEW2);
	buffer_create_view(x, buf, 3, DADA_SEQ_BUFFER_SAMPLES_PER_MS_VIEW3);
}


t_seq_buffer *buffer_find(t_seq *x, t_symbol *filename)
{
	// could be made faster, with hashtable
	t_symbol *fp = dada_ezlocate_file(filename, NULL);
	t_llllelem *elem;
	for (elem = x->buffers->l_head; elem; elem = elem->l_next) {
		t_seq_buffer *this_buffer = (t_seq_buffer *)hatom_getobj(&elem->l_hatom);
		if (this_buffer->filename_fullpath == fp) 
			return this_buffer;
	}
	return NULL;
}

t_symbol *buffer_gen_name(t_seq *x)
{
	char buf[100];
	snprintf_zero(buf, 100, "__dadaseq_buf_%d", x->num_buffers);
	return gensym(buf);
}

t_seq_buffer *buffer_find_or_create(t_seq *x, t_symbol *filename)
{
	t_seq_buffer *buf = buffer_find(x, filename);

	if (!buf) {
		buf = buffer_new(x, buffer_gen_name(x));
		buffer_fill(x, buf, filename);
	}
	
	return buf;
}

double ms_to_samps(t_seq *x, double ms)
{
	return (ms / 1000.) * x->sr;
}

double samps_to_ms(t_seq *x, double samps)
{
	return (samps / x->sr) * 1000.;
}


double buffer_ms_to_samps(t_seq_buffer *buf, double ms)
{
	return (ms / 1000.) * buf->sr;
}

double buffer_samps_to_ms(t_seq_buffer *buf, double samps)
{
	return (samps / buf->sr) * 1000.;
}

void buffer_delete(t_seq *x, t_seq_buffer *buf)
{
	long i;
	
	shashtable_chuck_thing(x->b_ob.d_ob.IDtable, buf->r_it.ID);

	for (i = 0; i < 4; i++)
		if (buf->views[i].samples)
			bach_freeptr(buf->views[i].samples);
	
	if (buf->reference_count) { // some regions still have the buffer as "their" buffer. Need to free them
		object_warn((t_object *)x, "Warning: some regions are still using file %s. These regions will be deleted.", buf->filename->s_name);
		t_llllelem *elem;
		for (elem = x->all_regions->l_head; elem; elem = elem->l_next) {
			t_seq_region *reg = (t_seq_region *)hatom_getobj(&elem->l_hatom);
			if (reg->buffer == buf) {
				buf->reference_count = 2; // to avoid region_delete() calling buffer_delete() once again
				region_delete(x, reg);
			}
		}
	}
	
	llll_destroyelem(buf->elem);
	
	bach_freeptr(buf);
}



/// TRACKS

// 0-based
t_seq_track *track_nth(t_seq *x, long num)
{
	t_llllelem *elem = llll_getindex(x->tracks, num+1, I_STANDARD);
	if (elem)
		return (t_seq_track *)hatom_getobj(&elem->l_hatom);
	return NULL;
}

t_seq_track *track_new(t_seq *x, t_symbol *track_name, long number) 
{
	t_seq_track *out = (t_seq_track *)bach_newptrclear(sizeof(t_seq_track));

	dadaitem_init(dadaobj_cast(x), (t_dadaitem *)out, (e_dadaitem_types)DADAITEM_TYPE_SEQ_TRACK, 0, true, 0);

	out->num_regions = 0;
	out->regions = llll_get();
	out->name = track_name;
	out->number = number;
	out->regions->l_thing.w_obj = out;
	out->elem = llll_appendobj(x->tracks, out, 0, WHITENULL_llll);
	
	long i;
	for (i = 0; i < 4; i++)
		out->channelmap[i] = i % x->num_audio_outlets;
	
	return out;
}

void init_all_tracks(t_seq *x)
{
	long i;
	for (i = 0; i < DADA_SEQ_CONST_MAX_TRACKS; i++) {
		char buf[100];
		snprintf_zero(buf, 20, "Track %d", i+1);
		t_symbol *track_name = gensym(buf);
		track_new(x, track_name, i);
	}
}


////// REGIONS

// 0-based
t_seq_region *region_nth(t_seq *x, t_seq_track *track, long num)
{
	t_llllelem *elem = llll_getindex(track->regions, num+1, I_STANDARD);
	if (elem)
		return (t_seq_region *)hatom_getobj(&elem->l_hatom);
	return NULL;
}

// 0-based
t_seq_region *get_nth_track_region(t_seq *x, long track_num, long region_num)
{
	t_seq_track *tr = track_nth(x, track_num);
	if (tr)
		return region_nth(x, tr, region_num); 
	return NULL;
}


t_llllelem *region_insert(t_seq *x, t_seq_track *track, t_seq_region *region, t_llllelem **all_elem)
{
	t_llllelem *elem;
	char inserted = false;
	for (elem = x->all_regions->l_head; elem; elem = elem->l_next) {
		t_seq_region *this_region = (t_seq_region *)hatom_getobj(&elem->l_hatom);
		if (this_region->onset_ms > region->onset_ms) {
			inserted = true;
			post("Inserted!");
			*all_elem = llll_insertobj_before(region, elem, 0, WHITENULL_llll);
			break;
		}
	}
	if (!inserted) {
		post("Inserted!");
		*all_elem = llll_appendobj(x->all_regions, region, 0, WHITENULL_llll);
	}
	
	for (elem = track->regions->l_head; elem; elem = elem->l_next) {
		t_seq_region *this_region = (t_seq_region *)hatom_getobj(&elem->l_hatom);
		if (this_region->onset_ms > region->onset_ms) {
			return llll_insertobj_before(region, elem, 0, WHITENULL_llll);
		}
	}
	return llll_appendobj(track->regions, region, 0, WHITENULL_llll);
}


t_seq_region *region_new(t_seq *x) 
{
	t_seq_region *out = (t_seq_region *)bach_newptrclear(sizeof(t_seq_region));
	
	dadaitem_init(dadaobj_cast(x), (t_dadaitem *)out, (e_dadaitem_types)DADAITEM_TYPE_SEQ_REGION, 0, true, 0);

	out->note = build_note(DADA_DEFAULT_PITCH, 0, DADA_DEFAULT_VELOCITY);

	return out;
}

t_seq_track	*region_get_parent(t_seq *x, t_seq_region *reg)
{
	t_seq_track *res = (t_seq_track *)reg->elem->l_parent->l_thing.w_obj;
	return res;
}

// use -1 as duration to say: whole file
t_seq_region *region_create_and_insert(t_seq *x, long track_number, double onset_ms, t_symbol *filename, 
									   double filestart_ms, double dur_ms, double gain, 
									   double fadein_ms, double fadein_slope, double fadeout_ms, double fadeout_slope)
{
	t_seq_track *track = track_nth(x, track_number);
	if (!track)
		return NULL;
	
	t_seq_region *reg = region_new(x);
	
	
	reg->filename = NULL;
	reg->buffer = NULL;
	reg->color = x->region_color;
	region_change_filename(x, reg, filename);
	
	check_regions(x);

	reg->r_it.coord.x =reg->onset_ms = onset_ms;
	reg->r_it.coord.y = -track_number;
	reg->onset_samps = round(ms_to_samps(x, onset_ms));
	reg->bufferstart_ms = filestart_ms;
	if (reg->buffer) {
		reg->bufferstart_samps = round(buffer_ms_to_samps(reg->buffer, filestart_ms));
		reg->dur_ms = (dur_ms >= 0 ? dur_ms : 1000 * buffer_getframecount(reg->buffer->buffer)/buffer_getsamplerate(reg->buffer->buffer));
	} else {
		reg->bufferstart_samps = reg->dur_ms = 0;
	}
	reg->dur_samps = (long)round(ms_to_samps(x, reg->dur_ms));
	
	check_regions(x);

	reg->global_gain = gain;
	
	reg->fade_in.duration_ms = fadein_ms;
	reg->fade_in.duration_samps = (long)round(ms_to_samps(x, fadein_ms));
	reg->fade_in.slope = fadein_slope;
	reg->fade_out.duration_ms = fadeout_ms;
	reg->fade_out.duration_samps = (long)round(ms_to_samps(x, fadeout_ms));
	reg->fade_out.slope = fadeout_slope;
	
	check_regions(x);
	reg->elem = region_insert(x, track, reg, &reg->all_elem);
	check_regions(x);
	return reg;
}

void region_delete(t_seq *x, t_seq_region *reg)
{
	if (reg->buffer) {
		reg->buffer->reference_count--;
		if (reg->buffer->reference_count <= 0 && DADA_SEQ_FREE_BUFFER_WHEN_COUNT_IS_ZERO)
			buffer_delete(x, reg->buffer);
	}
	shashtable_chuck_thing(x->b_ob.d_ob.IDtable, reg->r_it.ID);
	llll_destroyelem(reg->elem);
	llll_destroyelem(reg->all_elem);
	free_note(NULL, reg->note);
	bach_freeptr(reg);
}

void region_delete_all(t_seq *x, long track_num)
{
	if (track_num < 0) {
		t_llllelem *elem = x->all_regions->l_head;
		while (elem) {
			t_llllelem *next = elem->l_next;
			region_delete(x, (t_seq_region *)hatom_getobj(&elem->l_hatom));
			elem = next;
		}
	} else {
		t_seq_track *tr = track_nth(x, track_num);
		t_llllelem *elem = tr->regions->l_head;
		while (elem) {
			t_llllelem *next = elem->l_next;
			region_delete(x, (t_seq_region *)hatom_getobj(&elem->l_hatom));
			elem = next;
		}
	}
}

char region_delete_selected_fn(t_dadaobj *r_ob, t_dadaitem *item, void *data)
{
	region_delete((t_seq *)r_ob->orig_obj, (t_seq_region *)item);
	return 0;
}

void region_delete_selected(t_seq *x)
{
	dadaobj_selection_iterate(dadaobj_cast(x), region_delete_selected_fn, NULL);
}

void replace_elems(t_seq *x, t_llllelem *orig, t_llllelem *replacement)
{
	t_llllelem *elem;
	if (x->next_region_to_sequence_elem == orig)
		x->next_region_to_sequence_elem = replacement;
	for (elem = x->sequenced_regions->l_head; elem; elem = elem->l_next) {
		x->next_region_to_sequence_elem = replacement;
	}
}

void region_change_length_deltams(t_seq *x, t_seq_region *region, double delta_ms)
{
	double new_duration = region->dur_ms + delta_ms;
	if (new_duration < 0)
		new_duration = 0;
	if (region->buffer)
	if (new_duration > region->buffer->duration_ms)
		new_duration = region->buffer->duration_ms;
	region->dur_ms = new_duration;
	region->dur_samps = (long)round(ms_to_samps(x, region->dur_ms));
}




void region_move_deltams(t_seq *x, t_seq_region *region, double delta_ms)
{
	double new_onset_ms = region->onset_ms + delta_ms;
	if (new_onset_ms < 0)
		new_onset_ms = 0;
	region->r_it.coord.x = region->onset_ms = new_onset_ms;
	region->onset_samps = ms_to_samps(x, region->onset_ms);
	
	if (region->all_elem->l_prev && region->all_elem->l_prev->l_hatom.h_w.w_obj == region->all_elem->l_hatom.h_w.w_obj) {
		long foo = 8;
		foo++;
	}
	
	// checking position in llll
	if (delta_ms > 0) {
		dada_llll_check(region->elem->l_parent);
		while (region->elem->l_next && ((t_seq_region *)hatom_getobj(&region->elem->l_next->l_hatom))->onset_ms < new_onset_ms)
			llll_swapelems(region->elem, region->elem->l_next);
		dada_llll_check(region->elem->l_parent);

		dada_llll_check(x->all_regions);
		while (region->all_elem->l_next && ((t_seq_region *)hatom_getobj(&region->all_elem->l_next->l_hatom))->onset_ms < new_onset_ms)
			llll_swapelems(region->all_elem, region->all_elem->l_next);
		dada_llll_check(x->all_regions);
	} else if (delta_ms < 0) {
		dada_llll_check(region->elem->l_parent);
		while (region->elem->l_prev && ((t_seq_region *)hatom_getobj(&region->elem->l_prev->l_hatom))->onset_ms > new_onset_ms)
			llll_swapelems(region->elem->l_prev, region->elem);
		dada_llll_check(region->elem->l_parent);

		dada_llll_check(x->all_regions);
		while (region->all_elem->l_prev && ((t_seq_region *)hatom_getobj(&region->all_elem->l_prev->l_hatom))->onset_ms > new_onset_ms)
				llll_swapelems(region->all_elem->l_prev, region->all_elem);
		dada_llll_check(x->all_regions);
	} 
	
	if (region->all_elem->l_prev && region->all_elem->l_prev->l_hatom.h_w.w_obj == region->all_elem->l_hatom.h_w.w_obj) {
		long foo = 8;
		foo++;
	}
}

void region_change_offset_deltams(t_seq *x, t_seq_region *region, double delta_ms)
{
	double new_duration = region->dur_ms - delta_ms;
	if (new_duration < 0)
		delta_ms = region->dur_ms;
	if (new_duration > region->buffer->duration_ms) 
		delta_ms = region->dur_ms - region->buffer->duration_ms;

	double offset = region->bufferstart_ms + delta_ms;
	if (offset < 0)
		offset = 0;
	if (offset > region->buffer->duration_ms)
		offset = region->buffer->duration_ms;
	region->bufferstart_ms = offset;
	region->bufferstart_samps = buffer_ms_to_samps(region->buffer, region->bufferstart_ms);
	
	region_move_deltams(x, region, delta_ms);
	region_change_length_deltams(x, region, -delta_ms);
}


void region_change_track(t_seq *x, t_seq_region *region, long new_track)
{
	t_seq_track *tr = region_get_parent(x, region);
	if (new_track != tr->number) {
		t_seq_track *new_tr = track_nth(x, new_track);
		if (new_tr) {
			t_llllelem *elem = region->elem, *prev = region->elem->l_prev, *next = region->elem->l_next;
			if (prev)
				prev->l_next = next;
			else
				elem->l_parent->l_head = next;
			if (next)
				next->l_prev = prev;
			else
				elem->l_parent->l_tail = prev;
			elem->l_parent->l_size--;
			
			dada_llll_check(elem->l_parent);
			
			long inserted = false;
			for (elem = new_tr->regions->l_head; elem; elem = elem->l_next) {
				t_seq_region *this_region = (t_seq_region *)hatom_getobj(&elem->l_hatom);
				if (this_region->onset_ms > region->onset_ms) {
					inserted = true;
					llll_insert_before(region->elem, elem, WHITENULL_llll);	// region->elem has been re-adopted
				}
			}
			if (!inserted) 
				llll_append(new_tr->regions, region->elem, WHITENULL_llll);
			
			dada_llll_check(new_tr->regions);

			new_tr->r_it.coord.y = -new_track;

/*			t_llllelem *orig = region->elem;
			llll_destroyelem(region->elem);
			llll_destroyelem(region->all_elem);
			region->elem = region_insert(x, new_tr, region, &region->all_elem); */
		}
	}
}

void region_clone(t_seq *x, t_seq_region *region, char transfer_selection)
{
	long track_num = region_get_parent(x, region)->number;
	t_seq_region *cloned = region_create_and_insert(x, track_num, region->onset_ms, region->filename, region->bufferstart_ms, region->dur_ms, region->global_gain, 
													region->fade_in.duration_ms, region->fade_in.slope, region->fade_out.duration_ms, region->fade_out.slope);
	if (transfer_selection) {
		dadaobj_selection_select_item(dadaobj_cast(x), (t_dadaitem *)cloned, k_SELECTION_MODE_FORCE_SELECT);
		dadaobj_selection_select_item(dadaobj_cast(x), (t_dadaitem *)region, k_SELECTION_MODE_FORCE_UNSELECT);
		if (x->b_ob.d_ob.m_interface.mousedown_item == (t_dadaitem *)region)
			x->b_ob.d_ob.m_interface.mousedown_item = (t_dadaitem *)cloned;
	}
}

void region_change_fade_deltams(t_seq *x, t_seq_region *region, double delta_ms, char which_fade)
{
	double new_fade = (which_fade < 0 ? region->fade_in.duration_ms : region->fade_out.duration_ms) + delta_ms;
	if (new_fade < 0)
		new_fade = 0;
	if (new_fade > region->dur_ms)
		new_fade = region->dur_ms;
	
	if (which_fade < 0) {
		region->fade_in.duration_ms = new_fade;
		region->fade_in.duration_samps = buffer_ms_to_samps(region->buffer, new_fade);
	} else {
		region->fade_out.duration_ms = new_fade;
		region->fade_out.duration_samps = buffer_ms_to_samps(region->buffer, new_fade);
	}
}


void region_change_fade_deltaslope(t_seq *x, t_seq_region *region, double delta_slope, char which_fade)
{
	double new_slope = (which_fade < 0 ? region->fade_in.slope : region->fade_out.slope) + delta_slope;
	new_slope = CLAMP(new_slope, -1., 1.);
	
	if (which_fade < 0) {
		region->fade_in.slope = new_slope;
	} else {
		region->fade_out.slope = new_slope;
	}
}

t_rect region_get_rect(t_seq *x, t_seq_region *region, t_object *patcherview, double *fadein_x, double *fadeout_x)
{
	double x_start = ms_to_pix(x, patcherview, region->onset_ms), x_end = ms_to_pix(x, patcherview, region->onset_ms + region->dur_ms);
	double track_top_y, track_bottom_y, track_middle_y;
	long track_num = region_get_parent(x, region)->number;
	track_to_y_pos(x, track_num, &track_top_y, &track_bottom_y, &track_middle_y);
	
	t_rect region_rect = build_rect(x_start, track_top_y, x_end - x_start, track_bottom_y - track_top_y);

	if (fadein_x) {
		if (region->fade_in.duration_samps > 0) 
			*fadein_x = ms_to_pix(x, patcherview, region->onset_ms + region->fade_in.duration_ms);
		else 
			*fadein_x = x_start;
	}

	if (fadeout_x) {
		if (region->fade_out.duration_samps > 0) 
			*fadeout_x = ms_to_pix(x, patcherview, region->onset_ms + region->dur_ms - region->fade_out.duration_ms);
		else 
			*fadeout_x = x_end;
	}
	
	return region_rect;
}


void region_change_filename(t_seq *x, t_seq_region *reg, t_symbol *newfilename)
{
	if (reg->filename != newfilename) {
		char filename_no_path[MAX_FILENAME_CHARS];
		short path;

		if (reg->buffer) {
			reg->buffer->reference_count--;
			if (reg->buffer->reference_count <= 0 && DADA_SEQ_FREE_BUFFER_WHEN_COUNT_IS_ZERO)
				buffer_delete(x, reg->buffer);
		}
		
		if (!newfilename) {
			reg->filename = NULL;
			reg->buffer = NULL;
		} else {
			if (path_frompathname(newfilename->s_name, &path, filename_no_path))
				reg->filename = newfilename;
			else
				reg->filename = gensym(filename_no_path);
			reg->buffer = buffer_find_or_create(x, newfilename);
			reg->buffer->reference_count++;
		}
	}
}


void region_postprocess(t_seq *x)
{
	check_regions(x);
	invalidate_static_layer_and_repaint(x);
}



t_llll *region_get_as_llll(t_seq *x, t_seq_region *region)
{
	return dadaitem_get_as_llll(dadaobj_cast(x), (t_dadaitem *)region);
}



void track_postprocess(t_seq *x)
{
	check_regions(x);
	invalidate_static_layer_and_repaint(x);
}



///// DISPLAY FUNCTION

double samps_to_pix(t_seq *x, t_object *view, double samps)
{
	return ms_to_pix(x, view, samps_to_ms(x, samps));
}

double pix_to_samps(t_seq *x, t_object *view, double pix)
{
	return ms_to_samps(x, pix_to_ms(x, view, pix));
}

double ms_to_pix(t_seq *x, t_object *view, double ms)
{
	return coord_to_pix_from_view(dadaobj_cast(x), view, build_pt(ms, 0)).x;
//	return x->track_header_width + (ms - x->screen_ms_start) * x->b_ob.d_ob.zoom.x * DADA_SEQ_DEFAULT_PIXELS_PER_SECOND / 1000.;
}

double pix_to_ms(t_seq *x, t_object *view, double pix)
{
	return pix_to_coord_from_view(dadaobj_cast(x), view, build_pt(pix, 0)).x;
//	return x->screen_ms_start + (1000 * (pix - x->track_header_width) / DADA_SEQ_DEFAULT_PIXELS_PER_SECOND) / x->b_ob.d_ob.zoom.x;
}

double deltapix_to_deltams(t_seq *x, double deltapix)
{
	return delta_pix_to_delta_coord(dadaobj_cast(x), build_pt(deltapix, 0)).x;
//	return (1000 * deltapix / DADA_SEQ_DEFAULT_PIXELS_PER_SECOND) / x->b_ob.d_ob.zoom.x;
}

double deltams_to_deltapix(t_seq *x, double deltams)
{
	return delta_coord_to_delta_pix(dadaobj_cast(x), build_pt(deltams, 0)).x;
//	return deltams * x->b_ob.d_ob.zoom.x * DADA_SEQ_DEFAULT_PIXELS_PER_SECOND / 1000.;
}

void track_to_y_pos(t_seq *x, long track_num, double *top_y, double *bottom_y, double *center_y)
{
	double ty = x->track_height * x->b_ob.d_ob.m_zoom.zoom.y * track_num;
	double by = ty + x->track_height * x->b_ob.d_ob.m_zoom.zoom.y;
	double cy = 0.5 * (ty + by);
	if (top_y) *top_y = ty;
	if (bottom_y) *bottom_y = by;
	if (center_y) *center_y = cy;
}
						
long y_pos_to_track(t_seq *x, double y)
{
	long res = floor(y / (x->track_height * x->b_ob.d_ob.m_zoom.zoom.y));
	if (res >= 0 && res < x->num_tracks)
		return res;
	else if (res > 0)
		return 100000;
	return -100000;
}

////// UNDO HANDLING


void seq_undo_postprocess(t_seq *x)
{
	jbox_invalidate_layer((t_object *)x, NULL, gensym("static_layer1"));
	jbox_redraw((t_jbox *)x);
}

void seq_undo_step_push_regions(t_seq *x, t_symbol *operation)
{
//	undo_add_interface_step((t_object *)x, &x->m_undo, &x->m_interface, DADA_FUNC_v_oX, (method)seq_set_regions, NULL, DADA_FUNC_X_o, (method)seq_get_regions, NULL, operation);
}



