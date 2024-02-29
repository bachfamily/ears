
/**
	@file
	ears.pshow.c
 
	@name
	ears.pshow~
 
	@realname
	ears.pshow~
 
    @type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	Display partials
 
	@description
	Displays partial tracking
 
	@discussion
 
	@category
	ears display
 
	@keywords
	buffer, pshow, partials, display, peak, partial tracking
 
	@seealso
	waveform~
	
	@owner
	Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "jpatcher_api.h"
#include "jgraphics.h"
#include "foundation/llllobj.h"
#include "foundation/llll_commons_ext.h"
#include "math/bach_math_utilities.h"
#include "ears.object.h"



typedef struct _buf_pshow {
    t_jbox      n_obj;

    t_llll      *n_partials;
    
    long        n_autoscale_min;
    long        n_autoscale_max;
    double        n_colorcurve;
    
    double       n_length_ms;
    char        n_autolength;

    t_jrgba     n_bgcolor;
    
    t_jrgba     n_mincolor;
    t_jrgba     n_maxcolor;
    double      n_minvalue;
    double      n_maxvalue;
    long        n_maxnumbins;
    double      n_maxfreq;

    char        n_frequnit;
    char         n_ampunit;
    t_symbol    *n_type;
    double      n_freq_offset;

    char        n_displayinseconds;
    
    // attributes
    double      n_display_start_ms;
    double      n_display_end_ms;
    double      n_last_patcherzoom;
    
    double      n_peak_radius;
    double      n_partials_line_width;
    
    // actual values
    double      n_actual_display_start_ms;
    double      n_actual_display_end_ms;

    // grids
    double      n_grid_time_step_ms;
    t_jrgba     n_grid_time_color;
    t_atom      n_grid_freq_step;
    t_jrgba     n_grid_freq_color;
    t_jrgba     n_label_time_color;
    t_jrgba     n_label_freq_color;

    t_systhread_mutex    n_mutex;

    void        *n_proxy1;            // proxy inlet
    void        *n_proxy2;            // proxy inlet
    long        n_proxy_inletnum;    // # of inlet currently in use

} t_buf_pshow;



// Ppshowotypes
void            *buf_pshow_new(t_symbol *s, long argc, t_atom *argv);
void			buf_pshow_free(t_buf_pshow *x);
void			buf_pshow_bang(t_buf_pshow *x);
void			buf_pshow_anything(t_buf_pshow *x, t_symbol *msg, long ac, t_atom *av);
void            buf_pshow_int(t_buf_pshow *x, t_atom_long num);
void            buf_pshow_float(t_buf_pshow *x, t_atom_float num);

void buf_pshow_assist(t_buf_pshow *x, void *b, long m, long a, char *s);
void buf_pshow_inletinfo(t_buf_pshow *x, void *b, long a, char *t);
void buf_pshow_paint(t_buf_pshow *x, t_object *patcherview);

t_max_err buf_pshow_setattr_buffername(t_buf_pshow *x, void *attr, long argc, t_atom *argv);

void buf_pshow_create_surface(t_buf_pshow *x, t_buffer_obj *buf, t_rect *rect);

// Globals and Statics
static t_class	*s_buf_pshow_class = NULL;


/**********************************************************************/
// Class Definition and Life Cycle

t_max_err buf_pshow_setattr_autoscalemin(t_buf_pshow *x, void *attr, long argc, t_atom *argv)
{
    if (argc && argv) {
        if (is_atom_number(argv)) {
            x->n_autoscale_min = atom_getlong(argv);
            object_attr_setdisabled((t_object *)x, gensym("minvalue"), x->n_autoscale_min == 1);
        }
    }
    return MAX_ERR_NONE;
}

t_max_err buf_pshow_setattr_autoscalemax(t_buf_pshow *x, void *attr, long argc, t_atom *argv)
{
    if (argc && argv) {
        if (is_atom_number(argv)) {
            x->n_autoscale_max = atom_getlong(argv);
            object_attr_setdisabled((t_object *)x, gensym("maxvalue"), x->n_autoscale_min == 1);
        }
    }
    return MAX_ERR_NONE;
}
void ext_main(void *r)
{
    t_class *c;
    
    common_symbols_init();
    llllobj_common_symbols_init();
    
    if (llllobj_check_version(bach_get_current_llll_version()) || llllobj_test()) {
        ears_error_bachcheck();
        return;
    }

    c = class_new("ears.pshow~", (method)buf_pshow_new, (method)buf_pshow_free, sizeof(t_buf_pshow), 0L, A_GIMME, 0);
    
    c->c_flags |= CLASS_FLAG_NEWDICTIONARY;
    jbox_initclass(c, JBOX_FIXWIDTH | JBOX_FONTATTR);
    
    // @method symbol/llll @digest Display spectrogram
    // @description A symbol is considered as a buffer containing a spectrogram, i.e.
    // one bin for channel and one window for sample. The spectrogram is displayed

    class_addmethod(c, (method)buf_pshow_anything, "anything",    A_GIMME, 0);
    class_addmethod(c, (method)buf_pshow_float, "float",    A_FLOAT, 0);
    class_addmethod(c, (method)buf_pshow_int, "int",    A_LONG, 0);
    class_addmethod(c, (method)buf_pshow_paint,     "paint",    A_CANT, 0);
    class_addmethod(c, (method)buf_pshow_assist,    "assist",        A_CANT, 0);

    // attributes

    CLASS_ATTR_RGBA(c, "bgcolor", 0, t_buf_pshow, n_bgcolor);
    CLASS_ATTR_DEFAULT_SAVE_PAINT(c, "bgcolor", 0, "1. 1. 1. 1.0");
    CLASS_ATTR_STYLE_LABEL(c, "bgcolor", 0, "rgba", "Background Color");
    CLASS_ATTR_BASIC(c, "bgcolor", 0);
    CLASS_ATTR_CATEGORY(c, "bgcolor", 0, "Appearance");
    // @description Sets the background color.

    
    CLASS_ATTR_RGBA(c, "mincolor", 0, t_buf_pshow, n_mincolor);
    CLASS_ATTR_DEFAULT_SAVE_PAINT(c, "mincolor", 0, "1. 1. 1. 1.0");
    CLASS_ATTR_STYLE_LABEL(c, "mincolor", 0, "rgba", "Lowest Color");
    CLASS_ATTR_BASIC(c, "mincolor", 0);
    CLASS_ATTR_CATEGORY(c, "mincolor", 0, "Appearance");
    // @description Sets the color corresponding to the minimum value.
    
    CLASS_ATTR_RGBA(c, "maxcolor", 0, t_buf_pshow, n_maxcolor);
    CLASS_ATTR_DEFAULT_SAVE_PAINT(c, "maxcolor", 0, "0. 0. 0. 1.0");
    CLASS_ATTR_STYLE_LABEL(c, "maxcolor", 0, "rgba", "Highest Color");
    CLASS_ATTR_BASIC(c, "maxcolor", 0);
    CLASS_ATTR_CATEGORY(c, "maxcolor", 0, "Appearance");
    // @description Sets the color corresponding to the maximum value.
    
    CLASS_ATTR_DOUBLE(c, "minvalue", 0, t_buf_pshow, n_minvalue);
    CLASS_ATTR_DEFAULT_SAVE_PAINT(c, "minvalue", 0, "0.");
    CLASS_ATTR_STYLE_LABEL(c, "minvalue", 0, "text", "Lowest Value");
    CLASS_ATTR_BASIC(c, "minvalue", 0);
    CLASS_ATTR_CATEGORY(c, "minvalue", 0, "Appearance");
    // @description Sets the value corresponding to <m>mincolor</m> (only if <m>autoscale</m> is off).
    
    CLASS_ATTR_DOUBLE(c, "maxvalue", 0, t_buf_pshow, n_maxvalue);
    CLASS_ATTR_DEFAULT_SAVE_PAINT(c, "maxvalue", 0, "1.");
    CLASS_ATTR_STYLE_LABEL(c, "maxvalue", 0, "text", "Highest Value");
    CLASS_ATTR_BASIC(c, "maxvalue", 0);
    CLASS_ATTR_CATEGORY(c, "maxvalue", 0, "Appearance");
    // @description Sets the value corresponding to <m>maxcolor</m> (only if <m>autoscale</m> is off).
    
    CLASS_ATTR_LONG(c, "maxnumbins", 0, t_buf_pshow, n_maxnumbins);
    CLASS_ATTR_DEFAULT_SAVE_PAINT(c, "maxnumbins", 0, "0");
    CLASS_ATTR_STYLE_LABEL(c, "maxnumbins", 0, "text", "Maximum Number Of Bins Displayed");
    CLASS_ATTR_CATEGORY(c, "maxnumbins", 0, "Appearance");
    // @description Sets the maximum number of bins displayed (leave 0 for all).

    CLASS_ATTR_DOUBLE(c, "maxfreq", 0, t_buf_pshow, n_maxfreq);
    CLASS_ATTR_DEFAULT_SAVE_PAINT(c, "maxfreq", 0, "12000");
    CLASS_ATTR_STYLE_LABEL(c, "maxfreq", 0, "text", "Maximum Frequency");
    CLASS_ATTR_CATEGORY(c, "maxfreq", 0, "Appearance");
    // @description Sets the maximum represented frequency (leave 0 for all).

    CLASS_ATTR_CHAR(c, "sec", 0, t_buf_pshow, n_displayinseconds);
    CLASS_ATTR_DEFAULT_SAVE_PAINT(c, "sec", 0, "0");
    CLASS_ATTR_STYLE_LABEL(c, "sec", 0, "onoff", "Display In Seconds");
    CLASS_ATTR_CATEGORY(c, "sec", 0, "Appearance");
    // @description Toggle the time display in seconds instead of milliseconds.

    
    CLASS_ATTR_DOUBLE(c, "peaksize", 0, t_buf_pshow, n_peak_radius);
    CLASS_ATTR_DEFAULT_SAVE_PAINT(c, "peaksize", 0, "1");
    CLASS_ATTR_STYLE_LABEL(c, "peaksize", 0, "text", "Peak Radius");
    CLASS_ATTR_CATEGORY(c, "peaksize", 0, "Appearance");
    // @description Sets the peak radius for display.

    CLASS_ATTR_DOUBLE(c, "linewidth", 0, t_buf_pshow, n_partials_line_width);
    CLASS_ATTR_DEFAULT_SAVE_PAINT(c, "linewidth", 0, "1");
    CLASS_ATTR_STYLE_LABEL(c, "linewidth", 0, "text", "Line Width");
    CLASS_ATTR_CATEGORY(c, "linewidth", 0, "Appearance");
    // @description Sets the line width for partials display.

    

    CLASS_ATTR_SYM(c, "spectype", 0, t_buf_pshow, n_type);
    CLASS_ATTR_DEFAULT_SAVE_PAINT(c, "spectype", 0, "stft");
    CLASS_ATTR_STYLE_LABEL(c, "spectype", 0, "text", "Spectral Analysis Type");
    CLASS_ATTR_CATEGORY(c, "spectype", 0, "Spectral Analysis");
    // @description Sets the spectral analysis type ("stft", "cqt" or "tempogram").

    CLASS_ATTR_DOUBLE(c, "freqoffset", 0, t_buf_pshow, n_freq_offset);
    CLASS_ATTR_DEFAULT_SAVE_PAINT(c, "freqoffset", 0, "0");
    CLASS_ATTR_STYLE_LABEL(c, "freqoffset", 0, "text", "Frequency Offset");
    CLASS_ATTR_CATEGORY(c, "freqoffset", 0, "Spectral Analysis");
    // @description Sets the offset for the frequency display.
    
    CLASS_ATTR_CHAR(c, "frequnit", 0, t_buf_pshow, n_frequnit);
    CLASS_ATTR_STYLE_LABEL(c,"frequnit",0,"enumindex","Frequency Values Are In");
    CLASS_ATTR_DEFAULT_SAVE_PAINT(c, "frequnit", 0, "0");
    CLASS_ATTR_ENUMINDEX(c,"frequnit", 0, "Hertz BPM Cents MIDI");
    CLASS_ATTR_CATEGORY(c, "frequnit", 0, "Units");
    // @description Sets the unit for pitch values: Hertz (default), BPM, Cents, MIDI

    CLASS_ATTR_CHAR(c, "ampunit", 0, t_buf_pshow, n_ampunit);
    CLASS_ATTR_STYLE_LABEL(c,"ampunit",0,"enumindex","Amplitude Values Are In");
    CLASS_ATTR_ENUMINDEX(c,"ampunit", 0, "Linear Decibel");
    CLASS_ATTR_DEFAULT_SAVE_PAINT(c, "ampunit", 0, "0");
    CLASS_ATTR_CATEGORY(c, "ampunit", 0, "Units");
    // @description Sets the unit for amplitudes: Linear (default) or Decibel (0dB corresponding to 1. in the linear scale).

    
    
    CLASS_ATTR_CHAR(c, "autolength", 0, t_buf_pshow, n_autolength);
    CLASS_ATTR_DEFAULT_SAVE_PAINT(c, "autolength", 0, "1");
    CLASS_ATTR_STYLE_LABEL(c, "autolength", 0, "onoff", "Automatically Adjust Length");
    CLASS_ATTR_BASIC(c, "autolength", 0);
    CLASS_ATTR_CATEGORY(c, "autolength", 0, "Settings");
    // @description Toggles the ability to adjust displayed length automatically according
    // to introduced partials.


    
    CLASS_ATTR_LONG(c, "autoscalemin", 0, t_buf_pshow, n_autoscale_min);
    CLASS_ATTR_DEFAULT_SAVE_PAINT(c, "autoscalemin", 0, "0");
    CLASS_ATTR_STYLE_LABEL(c, "autoscalemin", 0, "onoff", "Automatically Rescale Minimum");
    CLASS_ATTR_ACCESSORS(c, "autoscalemin", NULL, buf_pshow_setattr_autoscalemin);
    CLASS_ATTR_BASIC(c, "autoscalemin", 0);
    CLASS_ATTR_CATEGORY(c, "autoscalemax", 0, "Settings");
    // @description Toggles the ability to obtain <m>minvalue</m> automatically
    // from the minimum and maximum values in the input buffer.

    CLASS_ATTR_LONG(c, "autoscalemax", 0, t_buf_pshow, n_autoscale_max);
    CLASS_ATTR_DEFAULT_SAVE_PAINT(c, "autoscalemax", 0, "1");
    CLASS_ATTR_STYLE_LABEL(c, "autoscalemax", 0, "onoff", "Automatically Rescale Maximum");
    CLASS_ATTR_ACCESSORS(c, "autoscalemax", NULL, buf_pshow_setattr_autoscalemax);
    CLASS_ATTR_BASIC(c, "autoscalemax", 0);
    CLASS_ATTR_CATEGORY(c, "autoscalemax", 0, "Settings");
    // @description Toggles the ability to obtain <m>maxvalue</m> automatically
    // from the minimum and maximum values in the input buffer.

    
    CLASS_ATTR_DOUBLE(c, "colorcurve", 0, t_buf_pshow, n_colorcurve);
    CLASS_ATTR_DEFAULT_SAVE_PAINT(c, "colorcurve", 0, "0");
    CLASS_ATTR_STYLE_LABEL(c, "colorcurve", 0, "text", "Color Range Curve");
    CLASS_ATTR_BASIC(c, "colorcurve", 0);
    CLASS_ATTR_CATEGORY(c, "colorcurve", 0, "Settings");
    // @description Sets the curvature value for the color mapping.

    
    CLASS_ATTR_RGBA(c, "timegridcolor", 0, t_buf_pshow, n_grid_time_color);
    CLASS_ATTR_DEFAULT_SAVE_PAINT(c, "timegridcolor", 0, "0.678 0.756862745098039 0.764705882352941 0.7");
    CLASS_ATTR_STYLE_LABEL(c, "timegridcolor", 0, "rgba", "Time Grid Color");
    CLASS_ATTR_CATEGORY(c, "timegridcolor", 0, "Grid");
    // @description Sets the color of the time grid
    
    
    CLASS_ATTR_RGBA(c, "timelabelcolor", 0, t_buf_pshow, n_label_time_color);
    CLASS_ATTR_DEFAULT_SAVE_PAINT(c, "timelabelcolor", 0, "0.678 0.756862745098039 0.764705882352941 0.7");
    CLASS_ATTR_STYLE_LABEL(c, "timelabelcolor", 0, "rgba", "Time Grid Label Color");
    CLASS_ATTR_CATEGORY(c, "timelabelcolor", 0, "Grid");
    // @description Sets the color of the time grid labels.
    
    CLASS_ATTR_DOUBLE(c, "timegrid", 0, t_buf_pshow, n_grid_time_step_ms);
    CLASS_ATTR_DEFAULT_SAVE_PAINT(c, "timegrid", 0, "1000.");
    CLASS_ATTR_STYLE_LABEL(c, "timegrid", 0, "text", "Time Grid Step");
    CLASS_ATTR_CATEGORY(c, "timegrid", 0, "Grid");
    // @description Sets the time grid step in milliseconds; use 0 to turn grid off.

    CLASS_ATTR_RGBA(c, "freqgridcolor", 0, t_buf_pshow, n_grid_freq_color);
    CLASS_ATTR_DEFAULT_SAVE_PAINT(c, "freqgridcolor", 0, "0.678 0.756862745098039 0.764705882352941 0.7");
    CLASS_ATTR_STYLE_LABEL(c, "freqgridcolor", 0, "rgba", "Frequency Grid Color");
    CLASS_ATTR_CATEGORY(c, "freqgridcolor", 0, "Grid");
    // @description Sets the color of the vertical grid.

    CLASS_ATTR_RGBA(c, "freqlabelcolor", 0, t_buf_pshow, n_label_freq_color);
    CLASS_ATTR_DEFAULT_SAVE_PAINT(c, "freqlabelcolor", 0, "0.678 0.756862745098039 0.764705882352941 0.7");
    CLASS_ATTR_STYLE_LABEL(c, "freqlabelcolor", 0, "rgba", "Frequency Grid Label Color");
    CLASS_ATTR_CATEGORY(c, "freqlabelcolor", 0, "Grid");
    // @description Sets the color of the vertical grid labels.
    
    CLASS_ATTR_ATOM(c, "freqgrid", 0, t_buf_pshow, n_grid_freq_step);
    CLASS_ATTR_DEFAULT_SAVE_PAINT(c, "freqgrid", 0, "auto");
    CLASS_ATTR_STYLE_LABEL(c, "freqgrid", 0, "text", "Frequency Grid Step");
    CLASS_ATTR_CATEGORY(c, "freqgrid", 0, "Grid");
    // @description Sets the vertical grid step, in the unit obtained by the
    // incoming spectral buffer. Use 0 to turn grid off. Use "auto"
    // to infer from incoming data

    
    CLASS_ATTR_DEFAULT(c,"patching_rect",0, "0. 0. 256. 128.");
    
    class_register(CLASS_BOX, c);
    s_buf_pshow_class = c;
}


void buf_pshow_assist(t_buf_pshow *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        sprintf(s, "symbol/llll: Buffer Containing spectrogram");
        // @in 0 @type symbol/llll @digest Buffer containing spectrogram
    }
}

void buf_pshow_inletinfo(t_buf_pshow *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}




void *buf_pshow_new(t_symbol *s, long argc, t_atom *argv)
{
    t_buf_pshow *x = NULL;
    t_dictionary *d = NULL;
    long boxflags;
    
    if (!(d = object_dictionaryarg(argc,argv)))
        return NULL;
    
    x = (t_buf_pshow *)object_alloc(s_buf_pshow_class);
    if (x) {
        x->n_display_start_ms = 0;
        x->n_display_end_ms = -1;
        x->n_autoscale_max = 1;
        x->n_autoscale_min = 0;

        x->n_mincolor = get_grey(1.);
        x->n_maxcolor = get_grey(0.);
        x->n_minvalue = 0.;
        x->n_maxvalue = 0.1;
        x->n_colorcurve = 0.;
        
        x->n_frequnit = EARS_FREQUNIT_HERTZ;
        x->n_ampunit = EARS_AMPUNIT_LINEAR;
        x->n_type = gensym("stft");
        
        x->n_partials = llll_get();
        
        boxflags = 0
        | JBOX_DRAWFIRSTIN
        | JBOX_NODRAWBOX
        | JBOX_DRAWINLAST
        | JBOX_TRANSPARENT
        //        | JBOX_NOGROW
        //        | JBOX_GROWY
        | JBOX_GROWBOTH
        //        | JBOX_HILITE
        //        | JBOX_BACKGROUND
        | JBOX_DRAWBACKGROUND
        //        | JBOX_NOFLOATINSPECTOR
        //        | JBOX_TEXTFIELD
        //        | JBOX_MOUSEDRAGDELTA
        //        | JBOX_TEXTFIELD
        ;
        
        systhread_mutex_new_debug(&x->n_mutex, 0);
        jbox_new((t_jbox *)x, boxflags, argc, argv);
        x->n_obj.b_firstin = (t_object *)x;
        x->n_proxy2 = proxy_new(x, 2, &x->n_proxy_inletnum);
        x->n_proxy1 = proxy_new(x, 1, &x->n_proxy_inletnum);
        jbox_set_fontsize((t_object *)x, 10);
        attr_dictionary_process(x,d);
        jbox_ready((t_jbox *)x);
    }
    return x;
}

void buf_pshow_free(t_buf_pshow *x)
{
    systhread_mutex_free_debug(x->n_mutex);
    llll_free(x->n_partials);
    jbox_free((t_jbox *)x);
}


void buf_pshow_float(t_buf_pshow *x, t_atom_float num)
{
    t_atom argv[1];
    atom_setfloat(argv, num);
    buf_pshow_anything(x, _sym_list, 1, argv);
}

void buf_pshow_int(t_buf_pshow *x, t_atom_long num)
{
    t_atom argv[1];
    atom_setlong(argv, num);
    buf_pshow_anything(x, _sym_list, 1, argv);
}


void buf_pshow_anything(t_buf_pshow *x, t_symbol *msg, long ac, t_atom *av)
{
    long inlet = proxy_getinlet((t_object *)x);

    t_llll *parsed = NULL;
    if (msg == _llllobj_sym_bach_llll) {
        if (ac > 0 && atom_gettype(av) == A_LONG)
            parsed = llll_retrieve_from_phonenumber_and_retain(atom_getlong(av));
    } else if (msg == _sym_list) {
        parsed = llll_parse(ac, av);
    } else {
        parsed = llll_get();
        llll_appendsym(parsed, msg);
    }
    if (!parsed) return;
    
    if (parsed && parsed->l_head) {
        if (inlet == 0) { // partials
            systhread_mutex_lock(x->n_mutex);
            llll_free(x->n_partials);
            x->n_partials = llll_clone(parsed);
            if (x->n_autolength) {
                // automatically find length
                double maxonset = 0;
                for (t_llllelem *partial = x->n_partials->l_head; partial; partial = partial->l_next) {
                    t_llll *partial_ll = hatom_getllll(&partial->l_hatom);
                    if (partial_ll) {
                        for (t_llllelem *peak = partial_ll->l_head; peak; peak = peak->l_next) {
                            t_llll *peak_ll = hatom_getllll(&peak->l_hatom);
                            if (peak_ll && peak_ll->l_head && is_hatom_number(&peak_ll->l_head->l_hatom)) { // onset, frequency, amplitude, phase
                                maxonset = MAX(maxonset, hatom_getdouble(&peak_ll->l_head->l_hatom));
                            }
                        }
                    }
                }
                x->n_length_ms = maxonset;
            }
            systhread_mutex_unlock(x->n_mutex);
            jbox_redraw((t_jbox *)x);
        } else if (inlet == 1) { // start display
            if (parsed && parsed->l_head && is_hatom_number(&parsed->l_head->l_hatom)) {
                x->n_display_start_ms = hatom_getdouble(&parsed->l_head->l_hatom);
                jbox_redraw((t_jbox *)x);
            }
        } else if (inlet == 2) { // end display
            if (parsed && parsed->l_head && is_hatom_number(&parsed->l_head->l_hatom)) {
                x->n_display_end_ms = hatom_getdouble(&parsed->l_head->l_hatom);
                jbox_redraw((t_jbox *)x);
            }
        }
    }
    llll_free(parsed);
}

t_jrgba buf_pshow_value_to_color(t_buf_pshow *x, double value)
{
    if (x->n_colorcurve == 0)
        return color_interp(x->n_mincolor, x->n_maxcolor, CLAMP((value - x->n_minvalue)/(x->n_maxvalue-x->n_minvalue), 0, 1));
    else
        return color_interp(x->n_mincolor, x->n_maxcolor, CLAMP(rescale_with_slope(value, x->n_minvalue, x->n_maxvalue, 0, 1, x->n_colorcurve, k_SLOPE_MAPPING_BACH), 0, 1));
    
}


double onset_to_xpos(t_buf_pshow *x, t_rect *rect, double onset)
{
    return rect->width * (onset-x->n_actual_display_start_ms)/(x->n_actual_display_end_ms - x->n_actual_display_start_ms);
}

double freq_to_ypos(t_buf_pshow *x, t_rect *rect, double freq)
{
    return rect->height * (1 - freq/(x->n_maxfreq));
//    return rect->height * (1 - (freq-x->n_bin_offset)/(x->n_num_bins *x->n_bin_step));
}

const char *get_frequnit_abbr(t_buf_pshow *x)
{
    switch (x->n_frequnit) {
        case EARS_FREQUNIT_HERTZ:
            return "Hz";
            break;
        case EARS_FREQUNIT_CENTS:
            return "cents";
            break;
        case EARS_FREQUNIT_BPM:
            return "bpm";
            break;
        case EARS_FREQUNIT_MIDI:
            return "MIDI";
            break;
        default:
            return "";
            break;
    }
}

void buf_pshow_paint(t_buf_pshow *x, t_object *patcherview)
{
    t_rect src, rect;
    t_jgraphics *g = (t_jgraphics *) patcherview_get_jgraphics(patcherview);        // obtain graphics context
    jbox_get_rect_for_view((t_object *)x, patcherview, &rect);
    
    double patcherzoom = patcherview_get_zoomfactor(patcherview);
    t_rect fullrect = build_rect(0, 0, rect.width, rect.height);
    
    // paint background
    paint_rectangle(g, x->n_bgcolor, x->n_bgcolor, 0, 0, rect.width, rect.height, 0);
    
    systhread_mutex_lock(x->n_mutex);
    
    double dstart = x->n_display_start_ms <= 0 ? 0 : x->n_display_start_ms;
    double dend = x->n_display_end_ms <= 0 ? x->n_length_ms : x->n_display_end_ms;
    x->n_actual_display_start_ms = dstart;
    x->n_actual_display_end_ms = dend;
    
    for (t_llllelem *partial = x->n_partials->l_head; partial; partial = partial->l_next) {
        t_llll *partial_ll = hatom_getllll(&partial->l_hatom);
        if (partial_ll) {
            double prev_amplitude = -10, prev_onset = -10, prev_freq = -10, prev_xpos = 0, prev_ypos = 0;
            t_jrgba prev_color = get_grey(0);
            for (t_llllelem *peak = partial_ll->l_head; peak; peak = peak->l_next) {
                t_llll *peak_ll = hatom_getllll(&peak->l_hatom);
                if (peak_ll && peak_ll->l_size >= 2) { // onset, frequency, [amplitude, phase]
                    double amplitude = 1; // default, if no amplitude inserted
                    double onset = 0;
                    double freq = 0;
                    if (peak_ll && peak_ll->l_head && is_hatom_number(&peak_ll->l_head->l_hatom)) {
                        onset = hatom_getdouble(&peak_ll->l_head->l_hatom);
                    }
                    if (peak_ll && peak_ll->l_head && peak_ll->l_head->l_next && is_hatom_number(&peak_ll->l_head->l_next->l_hatom)) {
                        freq = hatom_getdouble(&peak_ll->l_head->l_next->l_hatom);
                    }
                    if (peak_ll && peak_ll->l_head && peak_ll->l_head->l_next && peak_ll->l_head->l_next->l_next  && is_hatom_number(&peak_ll->l_head->l_next->l_next->l_hatom)) {
                        amplitude = hatom_getdouble(&peak_ll->l_head->l_next->l_next->l_hatom);
                    }
                    
                    t_jrgba color = buf_pshow_value_to_color(x, amplitude);
                    double xpos = onset_to_xpos(x, &rect, onset);
                    double ypos = freq_to_ypos(x, &rect, freq);
                    
                    if (prev_onset >= 0) {
                        paint_line(g, color, prev_xpos, prev_ypos, xpos, ypos, x->n_partials_line_width);
                    }
                    paint_circle_filled(g, color, xpos, ypos, x->n_peak_radius);

                    prev_xpos = xpos;
                    prev_ypos = ypos;
                    prev_color = color;
                    prev_amplitude = amplitude;
                    prev_freq = freq;
                    prev_onset = onset;
                }
            }
        }
    }
    
    // paint grids
    double length = x->n_length_ms;
    double grid_time_step = x->n_grid_time_step_ms;
    double grid_freq_step = 0;
    if (atom_gettype(&x->n_grid_freq_step) == A_SYM) {
        if (atom_getsym(&x->n_grid_freq_step) == _llllobj_sym_auto) {
            if (x->n_type == gensym("stft")) {
                grid_freq_step = 1000 * floor(1100./rect.height);
            } else if (x->n_type == gensym("cqt")) {
                grid_freq_step = 1200 * floor(400./rect.height);
            } else if (x->n_type == gensym("tempogram")) {
                grid_freq_step = 10 * floor(3000./rect.height);
//            } else if (x->n_type == gensym("stc")) { // to check
//                grid_freq_step = 256 * floor(1100./rect.height);
            }
        }
    } else {
        grid_freq_step = atom_getfloat(&x->n_grid_freq_step);
    }
    if (grid_time_step > 0 || grid_freq_step > 0) {
        t_jfont *jf_text = jfont_create_debug(jbox_get_fontname((t_object *)x)->s_name, (t_jgraphics_font_slant)jbox_get_font_slant((t_object *)x),
                                              (t_jgraphics_font_weight)jbox_get_font_weight((t_object *)x), jbox_get_fontsize((t_object *)x));
        char text[1024];
        double text_ends = -1;
        double tw, th;
        if (grid_time_step > 0) {
            for (double o = 0; o < length; o += grid_time_step) {
                if (o < dstart)
                    continue;
                if (o > dend)
                    break;
                double h = onset_to_xpos(x, &rect, o);
                paint_line(g, x->n_grid_time_color, h, 0, h, rect.height, 1);
                if (h > text_ends) {
                    if (x->n_displayinseconds) {
                        if (((int)o) % 1000 == 0)
                            snprintf_zero(text, 1024, " %d s", (int)o/1000);
                        else
                            snprintf_zero(text, 1024, " %.3 s", o/1000.);
                    } else
                        snprintf_zero(text, 1024, " %d ms", (int)o);
                    jfont_text_measure(jf_text, text, &tw, &th);
                    text_ends = h + tw;
                    write_text_standard(g, jf_text, x->n_label_time_color, text, h, 0, rect.width, rect.height);
                }
            }
        }

        if (grid_freq_step > 0) {
//            double numbins = x->n_num_bins;
            double offset = x->n_freq_offset;
//            double step = x->n_bin_step;
            double max = offset + x->n_maxfreq;
            const char *unit = get_frequnit_abbr(x);
            for (double b = offset; b < max; b += grid_freq_step) {
                double v = freq_to_ypos(x, &rect, b);
                paint_line(g, x->n_grid_freq_color, 0, v, rect.width, v, 1);
                if (v > 2 * jbox_get_fontsize((t_object *)x)) {
                    sprintf(text, " %d %s", (int)b, unit);
                    jfont_text_measure(jf_text, text, &tw, &th);
                    write_text_standard(g, jf_text, x->n_label_freq_color, text, 0, v - th, rect.width, rect.height);
                }
            }
        }
        
        jfont_destroy_debug(jf_text);
    }
    
    systhread_mutex_unlock(x->n_mutex);
    
    x->n_last_patcherzoom = patcherzoom;
}

