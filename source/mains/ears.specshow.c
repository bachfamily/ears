/**
	@file
	ears.specshow.c
 
	@name
	ears.specshow~
 
	@realname
	ears.specshow~
 
    @type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	Display spectrograms
 
	@description
	Display a spectrogram obtained via STFT or CQT transform.
 
	@discussion
 
	@category
	ears display
 
	@keywords
	buffer, specshow, spectrogram, display, stft, cqt
 
	@seealso
	waveform~
	
	@owner
	Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "jpatcher_api.h"
#include "jgraphics.h"
#include "llllobj.h"
#include "llll_commons_ext.h"
#include "bach_math_utilities.h"
#include "ears.object.h"



typedef struct _buf_specshow {
    t_jbox      n_obj;

    t_jsurface  *n_surface;
    
    long         n_must_recreate_surface;
    t_buffer_obj *n_last_buffer;
    double       n_framespersecond;
    double       n_length_ms;
    
    long        n_autoscale;
    
    t_jrgba     n_mincolor;
    t_jrgba     n_maxcolor;
    double      n_minvalue;
    double      n_maxvalue;
    long        n_maxnumbins;
    
    double      n_display_start;
    double      n_display_end;

    t_systhread_mutex    n_mutex;

    void        *n_proxy1;            // proxy inlet
    void        *n_proxy2;            // proxy inlet
    long        n_proxy_inletnum;    // # of inlet currently in use

} t_buf_specshow;



// Pspecshowotypes
void            *buf_specshow_new(t_symbol *s, long argc, t_atom *argv);
void			buf_specshow_free(t_buf_specshow *x);
void			buf_specshow_bang(t_buf_specshow *x);
void			buf_specshow_anything(t_buf_specshow *x, t_symbol *msg, long ac, t_atom *av);
void            buf_specshow_int(t_buf_specshow *x, t_atom_long num);
void            buf_specshow_float(t_buf_specshow *x, t_atom_float num);

void buf_specshow_assist(t_buf_specshow *x, void *b, long m, long a, char *s);
void buf_specshow_inletinfo(t_buf_specshow *x, void *b, long a, char *t);
void buf_specshow_paint(t_buf_specshow *x, t_object *patcherview);
t_max_err buf_specshow_notify(t_buf_specshow *x, t_symbol *s, t_symbol *msg, void *sender, void *data);

void buf_specshow_create_surface(t_buf_specshow *x, t_buffer_obj *buf);

// Globals and Statics
static t_class	*s_buf_specshow_class = NULL;


/**********************************************************************/
// Class Definition and Life Cycle

void ext_main(void *r)
{
    t_class *c;
    
    common_symbols_init();
    llllobj_common_symbols_init();
    
    if (llllobj_check_version(bach_get_current_llll_version()) || llllobj_test()) {
        ears_error_bachcheck();
        return;
    }

    c = class_new("ears.specshow~", (method)buf_specshow_new, (method)buf_specshow_free, sizeof(t_buf_specshow), 0L, A_GIMME, 0);
    
    c->c_flags |= CLASS_FLAG_NEWDICTIONARY;
    jbox_initclass(c, JBOX_FIXWIDTH | JBOX_COLOR);
    
    // @method symbol/llll @digest Display spectrogram
    // @description A symbol is considered as a buffer containing a spectrogram, i.e.
    // one bin for channel and one window for sample. The spectrogram is displayed

    class_addmethod(c, (method)buf_specshow_anything, "anything",    A_GIMME, 0);
    class_addmethod(c, (method)buf_specshow_float, "float",    A_FLOAT, 0);
    class_addmethod(c, (method)buf_specshow_int, "int",    A_LONG, 0);
    class_addmethod(c, (method)buf_specshow_paint,     "paint",    A_CANT, 0);
    class_addmethod(c, (method)buf_specshow_assist,    "assist",        A_CANT, 0);
    class_addmethod(c, (method)buf_specshow_notify,        "notify",    A_CANT, 0);

    // attributes
    CLASS_ATTR_RGBA(c, "mincolor", 0, t_buf_specshow, n_mincolor);
    CLASS_ATTR_DEFAULT_SAVE_PAINT(c, "mincolor", 0, "1. 1. 1. 1.0");
    CLASS_ATTR_STYLE_LABEL(c, "mincolor", 0, "rgba", "Lowest Color");
    CLASS_ATTR_BASIC(c, "mincolor", 0);
    // @description Sets the color corresponding to the minimum value.
    
    CLASS_ATTR_RGBA(c, "maxcolor", 0, t_buf_specshow, n_maxcolor);
    CLASS_ATTR_DEFAULT_SAVE_PAINT(c, "maxcolor", 0, "0. 0. 0. 1.0");
    CLASS_ATTR_STYLE_LABEL(c, "maxcolor", 0, "rgba", "Highest Color");
    CLASS_ATTR_BASIC(c, "maxcolor", 0);
    // @description Sets the color corresponding to the maximum value.
    
    CLASS_ATTR_DOUBLE(c, "minvalue", 0, t_buf_specshow, n_minvalue);
    CLASS_ATTR_DEFAULT_SAVE_PAINT(c, "minvalue", 0, "0.");
    CLASS_ATTR_STYLE_LABEL(c, "minvalue", 0, "text", "Lowest Value");
    CLASS_ATTR_BASIC(c, "minvalue", 0);
    // @description Sets the value corresponding to <m>mincolor</m>.
    
    CLASS_ATTR_DOUBLE(c, "maxvalue", 0, t_buf_specshow, n_maxvalue);
    CLASS_ATTR_DEFAULT_SAVE_PAINT(c, "maxvalue", 0, "1.");
    CLASS_ATTR_STYLE_LABEL(c, "maxvalue", 0, "text", "Highest Value");
    CLASS_ATTR_BASIC(c, "maxvalue", 0);
    // @description Sets the value corresponding to <m>maxcolor</m>.
    
    CLASS_ATTR_LONG(c, "maxnumbins", 0, t_buf_specshow, n_maxnumbins);
    CLASS_ATTR_DEFAULT_SAVE_PAINT(c, "maxnumbins", 0, "0");
    CLASS_ATTR_STYLE_LABEL(c, "maxnumbins", 0, "text", "Maximum Number Of Bins Displayed");
    // @description Sets the maximum number of bins displayed (leave 0 for all).

    CLASS_ATTR_LONG(c, "autoscale", 0, t_buf_specshow, n_autoscale);
    CLASS_ATTR_DEFAULT_SAVE_PAINT(c, "autoscale", 0, "0");
    CLASS_ATTR_STYLE_LABEL(c, "autoscale", 0, "onoff", "Automatically Rescale Range");
    // @description Toggles the ability to obtain <m>minvalue</m> and <m>maxvalue</m> automatically
    // from the minimum and maximum values in the input buffer.

    CLASS_ATTR_DEFAULT(c,"patching_rect",0, "0. 0. 256. 128.");
    
    class_register(CLASS_BOX, c);
    s_buf_specshow_class = c;
}


void buf_specshow_assist(t_buf_specshow *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        sprintf(s, "symbol/llll: Buffer Containing spectrogram");
        // @in 0 @type symbol/llll @digest Buffer containing spectrogram
    }
}

void buf_specshow_inletinfo(t_buf_specshow *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}

t_max_err buf_specshow_notify(t_buf_specshow *x, t_symbol *s, t_symbol *msg, void *sender, void *data)
{
    if (msg == _sym_attr_modified) {
        x->n_must_recreate_surface = true;
        jbox_redraw((t_jbox *)x);
    }
    return MAX_ERR_NONE;
}


void *buf_specshow_new(t_symbol *s, long argc, t_atom *argv)
{
    t_buf_specshow *x = NULL;
    t_dictionary *d = NULL;
    long boxflags;
    
    if (!(d = object_dictionaryarg(argc,argv)))
        return NULL;
    
    x = (t_buf_specshow *)object_alloc(s_buf_specshow_class);
    if (x) {
        x->n_surface = NULL;
        x->n_last_buffer = NULL;
        x->n_display_start = 0;
        x->n_display_end = -1;
        x->n_autoscale = 0;
        
        x->n_mincolor = get_grey(1.);
        x->n_maxcolor = get_grey(0.);
        x->n_minvalue = 0.;
        x->n_maxvalue = 0.1;
        
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
        attr_dictionary_process(x,d);
        jbox_ready((t_jbox *)x);
    }
    return x;
}

void buf_specshow_free(t_buf_specshow *x)
{
    systhread_mutex_free_debug(x->n_mutex);
    if (x->n_surface)
        jgraphics_surface_destroy(x->n_surface);
    jbox_free((t_jbox *)x);
}


void buf_specshow_float(t_buf_specshow *x, t_atom_float num)
{
    t_atom argv[1];
    atom_setfloat(argv, num);
    buf_specshow_anything(x, _sym_list, 1, argv);
}

void buf_specshow_int(t_buf_specshow *x, t_atom_long num)
{
    t_atom argv[1];
    atom_setlong(argv, num);
    buf_specshow_anything(x, _sym_list, 1, argv);
}


void buf_specshow_anything(t_buf_specshow *x, t_symbol *msg, long ac, t_atom *av)
{
    long inlet = proxy_getinlet((t_object *)x);

    t_llll *parsed = NULL;
    if (msg == _llllobj_sym_bach_llll || msg == _sym_list) {
        parsed = llll_parse(ac, av);
    } else {
        parsed = llll_get();
        llll_appendsym(parsed, msg);
    }
    if (!parsed) return;
    
    if (parsed && parsed->l_head) {
        if (inlet == 0) {
            if (hatom_gettype(&parsed->l_head->l_hatom) == H_SYM) {
                t_symbol *buffer = hatom_getsym(&parsed->l_head->l_hatom);
                if (ears_buffer_symbol_is_buffer(buffer)) {
                    x->n_last_buffer = ears_buffer_getobject(buffer);
                    x->n_must_recreate_surface = true;
                    jbox_redraw((t_jbox *)x);
                } else {
                    object_error((t_object *)x, "Input symbol does not correspond to a buffer!");
                }
            }
        } else if (inlet == 1) { // start display
            if (parsed && parsed->l_head && is_hatom_number(&parsed->l_head->l_hatom)) {
                x->n_display_start = hatom_getdouble(&parsed->l_head->l_hatom);
                jbox_redraw((t_jbox *)x);
            }
        } else if (inlet == 2) { // end display
            if (parsed && parsed->l_head && is_hatom_number(&parsed->l_head->l_hatom)) {
                x->n_display_end = hatom_getdouble(&parsed->l_head->l_hatom);
                jbox_redraw((t_jbox *)x);
            }
        }
    }
    llll_free(parsed);
}

t_jrgba buf_specshow_value_to_color(t_buf_specshow *x, double value)
{
    return color_interp(x->n_mincolor, x->n_maxcolor, CLAMP((value - x->n_minvalue)/(x->n_maxvalue-x->n_minvalue), 0, 1));
}

void buf_specshow_create_surface(t_buf_specshow *x, t_buffer_obj *buf)
{
    if (!buf)
        return;
    
    systhread_mutex_lock(x->n_mutex);
    
    x->n_framespersecond = ears_buffer_get_sr((t_object *)x, buf);
    x->n_length_ms = ears_buffer_get_size_ms((t_object *)x, buf);
    
    if (x->n_autoscale) {
        double amin, amax;
        if (ears_buffer_get_minmax((t_object *)x, buf, &amin, &amax) == EARS_ERR_NONE) {
            x->n_minvalue = amin;
            x->n_maxvalue = (amin == amax ? amin + 1 : amax);
        }
    }
    
    long numframes = ears_buffer_get_size_samps((t_object *)x, buf);
    long numchannels = ears_buffer_get_numchannels((t_object *)x, buf);
    long numbins = numchannels;
    if (x->n_maxnumbins > 0) {
        numbins = MIN(numbins, x->n_maxnumbins);
    }
    if (x->n_surface)
        jgraphics_surface_destroy(x->n_surface);
    x->n_surface = jgraphics_image_surface_create(JGRAPHICS_FORMAT_ARGB32, numframes, numbins);
    
    float *sample = buffer_locksamples(buf);
    
    if (!sample) {
        object_error((t_object *)x, EARS_ERROR_BUF_CANT_READ);
    } else {
        for (long f = 0; f < numframes; f++) {
            for (long c = 0; c < numbins; c++) {
                jgraphics_image_surface_set_pixel(x->n_surface, f, numbins - c - 1, buf_specshow_value_to_color(x, sample[f*numchannels + c]));
            }
        }
    }
    
    buffer_unlocksamples(buf);
    
    systhread_mutex_unlock(x->n_mutex);
}

void buf_specshow_paint(t_buf_specshow *x, t_object *patcherview)
{
    t_rect src, rect;
    t_jgraphics *g = (t_jgraphics *) patcherview_get_jgraphics(patcherview);        // obtain graphics context
    jbox_get_rect_for_view((t_object *)x, patcherview, &rect);
    
    t_rect fullrect = build_rect(0, 0, rect.width, rect.height);
    
    if (x->n_must_recreate_surface && x->n_last_buffer) {
        buf_specshow_create_surface(x, x->n_last_buffer);
        x->n_must_recreate_surface = false;
    }
    
    if (!x->n_surface) {
        t_jrgba white = get_grey(1.);
        paint_rect(g, &fullrect, &white, &white, 0, 0);
        return;
    } else {
        src.y = 0;
        src.height = jgraphics_image_surface_get_height(x->n_surface);
        if (x->n_display_start <= 0 && x->n_display_end <= 0) {
            src.x = 0;
            src.width = jgraphics_image_surface_get_width(x->n_surface);
        } else {
            double dstart = x->n_display_start <= 0 ? 0 : x->n_display_start;
            double dend = x->n_display_end <= 0 ? x->n_length_ms : x->n_display_end;
            src.x = x->n_framespersecond * dstart * 0.001;
            src.width = x->n_framespersecond * (dend - dstart) * 0.001;
        }

        jgraphics_image_surface_draw(g, x->n_surface, src, fullrect);
    }
}


