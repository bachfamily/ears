/**
	@file
	ears.vamp.c
 
	@name
	ears.vamp~
 
	@realname
	ears.vamp~
 
	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	Vamp plugins host
 
	@description
	Extract audio features via Vamp plugins
 
	@discussion
 
	@category
	ears analysis
 
	@keywords
	buffer, feature, features, descriptor, vamp, plugin
 
	@seealso
	ears.essentia~
	
	@owner
	Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "foundation/llllobj.h"
#include "foundation/llll_commons_ext.h"
#include "math/bach_math_utilities.h"
#include "ears.object.h"

#include "ears.vamp_commons.h"

using namespace std;

using Vamp::Plugin;
using Vamp::PluginHostAdapter;
using Vamp::RealTime;
using Vamp::HostExt::PluginLoader;
using Vamp::HostExt::PluginWrapper;
using Vamp::HostExt::PluginInputDomainAdapter;

#define HOST_VERSION "1.5"
#define EARS_MAX_VAMP_PLUGINS 128

enum Verbosity {
    PluginIds,
    PluginOutputIds,
    PluginInformation,
    PluginInformationDetailed
};

typedef struct _buf_vamp_plugin {
    t_symbol*       n_soname; // library name (soname in language of vamp, I guess)
    t_symbol*       n_identifier; // plugin identifier
    t_symbol*       n_output;
    long            n_outputIndex;
    e_ears_analysis_temporalmode           n_temporalmode;
} t_buf_vamp_plugin;

typedef struct _buf_vamp {
    t_earsbufobj       e_ob;
    
    char               n_autoframehopsize;
    
    t_buf_vamp_plugin  n_plugins[EARS_MAX_VAMP_PLUGINS];
    long               n_numplugins;
    t_llll             *n_pluginparameters[EARS_MAX_VAMP_PLUGINS];

    char               n_summarization;

} t_buf_vamp;



// Prototypes
t_buf_vamp*         buf_vamp_new(t_symbol *s, short argc, t_atom *argv);
void			buf_vamp_free(t_buf_vamp *x);
void			buf_vamp_bang(t_buf_vamp *x);
void			buf_vamp_anything(t_buf_vamp *x, t_symbol *msg, long ac, t_atom *av);
void            buf_vamp_getlist(t_buf_vamp *x, t_symbol *msg, long ac, t_atom *av);
int             buf_vamp_add_plugin(t_buf_vamp *x, t_symbol *pluginname);

void buf_vamp_assist(t_buf_vamp *x, void *b, long m, long a, char *s);
void buf_vamp_inletinfo(t_buf_vamp *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;


EARSBUFOBJ_ADD_IO_METHODS(vamp)

/**********************************************************************/
// Class Definition and Life Cycle
t_max_err buf_vamp_setattr_auto(t_buf_vamp *x, void *attr, long argc, t_atom *argv)
{
    if (argc && argv) {
        if (is_atom_number(argv)) {
            x->n_autoframehopsize = (atom_getlong(argv) != 0 ? 1 : 0);
            object_attr_setdisabled((t_object *)x, gensym("framesize"), x->n_autoframehopsize);
            object_attr_setdisabled((t_object *)x, gensym("hopsize"), x->n_autoframehopsize);
            object_attr_setdisabled((t_object *)x, gensym("overlap"), x->n_autoframehopsize);
            object_attr_setdisabled((t_object *)x, gensym("numframes"), x->n_autoframehopsize);
        }
    }
    return MAX_ERR_NONE;
}


void C74_EXPORT ext_main(void* moduleRef)
{
    common_symbols_init();
    llllobj_common_symbols_init();
    
    if (llllobj_check_version(bach_get_current_llll_version()) || llllobj_test()) {
        ears_error_bachcheck();
        return 1;
    }
    
    t_class *c;
    
    CLASS_NEW_CHECK_SIZE(c, "ears.vamp~",
                         (method)buf_vamp_new,
                         (method)buf_vamp_free,
                         sizeof(t_buf_vamp),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    // @method list/llll @digest Store buffers
    // @description A list or llll with buffer names will be considered as the names of the buffers to be stored and output
    // (also according on the <m>naming</m> attribute).
    EARSBUFOBJ_DECLARE_COMMON_METHODS_HANDLETHREAD(vamp) // TO DO: should we NOT defer this?

    // @method getlist @digest Get plugin list
    // @description The <m>getlist</m> message provides the list of identifiers for the currently installed plugins,
    // organized in sublist by library.
    // A <b>getlist outputs</b> message additionally provides the list of outlets for each plugin, with one
    // additional level of parentheses. <br />
    // A <b>getlist parameter</b> message, followed by the key identifier of a plugin
    // (<m>libraryid</m>:<m>pluginid</m>, as in the object box arguments)
    // provides the list of parameters for the plugin, along with their default values, ranges and additional information. <br />
    // A <b>getlist libraries</b> message provides the list of libraries. <br />
    // A <b>getlist detailed</b> message provides full details about functions, parameters and outputs of each plugins. <br />
    class_addmethod(c, (method)buf_vamp_getlist,  "getlist",      A_GIMME, 0); \

    llllobj_class_add_out_attr(c, LLLL_OBJ_VANILLA);
    
    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_blocking_attr(c);
    earsbufobj_class_add_naming_attr(c);
    earsbufobj_class_add_timeunit_attr(c);
    earsbufobj_class_add_antimeunit_attr(c);

    earsbufobj_class_add_framesize_attr(c);
    earsbufobj_class_add_hopsize_attr(c);
    earsbufobj_class_add_numframes_attr(c);
    earsbufobj_class_add_overlap_attr(c);

    CLASS_ATTR_CHAR(c, "auto", 0, t_buf_vamp, n_autoframehopsize);
    CLASS_ATTR_STYLE_LABEL(c,"auto",0,"onoff","Automatically Assign Analysis Parameters");
    CLASS_ATTR_BASIC(c, "auto", 0);
    CLASS_ATTR_FILTER_CLIP(c, "auto", 0, 1);
    CLASS_ATTR_CATEGORY(c, "auto", 0, "Analysis");
    CLASS_ATTR_ACCESSORS(c, "auto", NULL, buf_vamp_setattr_auto);
    // @description Toggles the ability to automatically assign analysis parameters according to the preferred values
    // of each plugin
    
    
    CLASS_ATTR_CHAR(c, "summary", 0, t_buf_vamp, n_summarization);
    CLASS_ATTR_STYLE_LABEL(c,"summary",0,"enumindex","Summarization Mode");
    CLASS_ATTR_ENUMINDEX(c, "summary", 0, "First Last Middle Mean Median Mode");
    CLASS_ATTR_BASIC(c, "summary", 0);
    CLASS_ATTR_FILTER_CLIP(c, "summary", 0, 5);
    CLASS_ATTR_CATEGORY(c, "summary", 0, "Summarization");
    // @description Sets the summarization mode. Available modes are:
    // <b>First</b>: take first frame; <br />
    // <b>Last</b>: last last frame; <br />
    // <b>Middle</b>: take middle frame; <br />
    // <b>Mean</b>: average through frames; <br />
    // <b>Median</b>: median through frames (for use with single-valued essentia); <br />
    // <b>Mode</b>: mode through frames (for use with discrete essentia). <br />
    
    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
    return 0;
}

void buf_vamp_assist(t_buf_vamp *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        if (a == 0)
            sprintf(s, "symbol/list/llll: Incoming Buffer Names"); // @in 0 @type symbol/list/llll @digest Incoming buffer names
        else
            sprintf(s, "list/llll: Parameters for %s:%s:%s", x->n_plugins[a-1].n_soname->s_name, x->n_plugins[a-1].n_identifier->s_name, x->n_plugins[a-1].n_output->s_name);
            // @in 1 @loop 1 @type llll @digest Parameters for each of the features (one inlet per feature).
    } else {
        char *type = NULL;
        llllobj_get_llll_outlet_type_as_string((t_object *) x, LLLL_OBJ_VANILLA, a, &type);
        if (a < x->n_numplugins) {
            if (x->n_plugins[a].n_temporalmode == EARS_ANALYSIS_TEMPORALMODE_WHOLE)
                sprintf(s, "llll (%s): %s:%s:%s (global)", type, x->n_plugins[a].n_soname->s_name, x->n_plugins[a].n_identifier->s_name, x->n_plugins[a].n_output->s_name);
            else if (x->n_plugins[a].n_temporalmode == EARS_ANALYSIS_TEMPORALMODE_TIMESERIES)
                sprintf(s, "llll (%s): %s:%s:%s (time series)", type, x->n_plugins[a].n_soname->s_name, x->n_plugins[a].n_identifier->s_name, x->n_plugins[a].n_output->s_name);
            else if (x->n_plugins[a].n_temporalmode == EARS_ANALYSIS_TEMPORALMODE_LABELLEDTIMESERIES)
                sprintf(s, "llll (%s): %s:%s:%s (time-tagged time series)", type, x->n_plugins[a].n_soname->s_name, x->n_plugins[a].n_identifier->s_name, x->n_plugins[a].n_output->s_name);
            else if (x->n_plugins[a].n_temporalmode == EARS_ANALYSIS_TEMPORALMODE_BUFFER)
                sprintf(s, "buffer: %s:%s:%s", x->n_plugins[a].n_soname->s_name, x->n_plugins[a].n_identifier->s_name, x->n_plugins[a].n_output->s_name);
            // @out 0 @loop 1 @type llll/buffer @digest Feature for the corresponding key
            // @description Feature for one of the introduced key; some essentia require multiple outlets (see outlet assistance).
        } else {
            sprintf(s, "llll (%s): Notifications", type);
            // @out 1 @type llll @digest Notifications
            // @description Receive notifications from <m>getlist</m> messages.
        }
    }
}

void buf_vamp_inletinfo(t_buf_vamp *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}

int buf_vamp_add_plugin(t_buf_vamp *x, t_symbol *pluginname)
{
    e_ears_analysis_temporalmode tm = EARS_ANALYSIS_TEMPORALMODE_WHOLE;
    long pluginindex = x->n_numplugins;
    
    // convert to lower case
    char buf[2048];
    long len = strlen(pluginname->s_name);
    for (long i = 0; i <= len; i++) { // including trailing 0
        buf[i] = tolower(pluginname->s_name[i]);
    }
    
    t_symbol *s = gensym(buf);
    if (ears_symbol_ends_with(s, "...", false)) {
        snprintf_zero(buf, 2048, "%s", s->s_name);
        buf[strlen(s->s_name)-3] = 0;
        tm = EARS_ANALYSIS_TEMPORALMODE_TIMESERIES;
    } else if (ears_symbol_ends_with(s, ":::", false)) {
        snprintf_zero(buf, 2048, "%s", s->s_name);
        buf[strlen(s->s_name)-3] = 0;
        s = gensym(buf);
        tm = EARS_ANALYSIS_TEMPORALMODE_LABELLEDTIMESERIES;
    } else if (ears_symbol_ends_with(s, "~", false)) {
        snprintf_zero(buf, 2048, "%s", s->s_name);
        buf[strlen(s->s_name)-1] = 0;
        tm = EARS_ANALYSIS_TEMPORALMODE_BUFFER;
    } else {
        tm = EARS_ANALYSIS_TEMPORALMODE_WHOLE;
    }
    
    string soname = buf;
    string plugid, output;
    
    string::size_type sep = soname.find(':');
    
    if (sep != string::npos) {
        plugid = soname.substr(sep + 1);
        soname = soname.substr(0, sep);
        
        sep = plugid.find(':');
        if (sep != string::npos) {
            output = plugid.substr(sep + 1);
            plugid = plugid.substr(0, sep);
        }
    }
    
    if (plugid == "") {
        object_error((t_object *)x, "Empty plugin defined");
        return 1;
    }
    
    /*
    if (output != "" && outputNo != -1) {
        usage(name);
    }
     */
    
    x->n_plugins[pluginindex].n_soname = gensym(soname.c_str());
    x->n_plugins[pluginindex].n_identifier = gensym(plugid.c_str());
    x->n_plugins[pluginindex].n_output = gensym(output.c_str());
    x->n_plugins[pluginindex].n_temporalmode = tm;

    // check if plugin exists
    PluginLoader *loader = PluginLoader::getInstance();
    PluginLoader::PluginKey key = loader->composePluginKey(soname, plugid);
    Plugin *plugin = loader->loadPlugin(key, 48000, PluginLoader::ADAPT_ALL_SAFE);
    if (!plugin) {
        char errormsg[2048];
        snprintf_zero(errormsg, 2048, "Failed to load plugin '%s' from library '%s'.",
                      x->n_plugins[pluginindex].n_identifier ? x->n_plugins[pluginindex].n_identifier->s_name : "",
                      x->n_plugins[pluginindex].n_soname ? x->n_plugins[pluginindex].n_soname->s_name : "");
        object_error((t_object *)x, errormsg);
        return 1;
    }
    
    // check if output exists
    Plugin::OutputList outputs = plugin->getOutputDescriptors();
    if (output == "") {
        if (outputs.size() > 0) {
            x->n_plugins[pluginindex].n_outputIndex = 0;
            x->n_plugins[pluginindex].n_output = gensym(outputs[0].identifier.c_str());
        } else {
            char errormsg[2048];
            snprintf_zero(errormsg, 2048, "Plugin '%s' from library '%s' has no outputs.",
                          x->n_plugins[pluginindex].n_identifier ? x->n_plugins[pluginindex].n_identifier->s_name : "",
                          x->n_plugins[pluginindex].n_soname ? x->n_plugins[pluginindex].n_soname->s_name : "");
            object_error((t_object *)x, errormsg);
            return 1;
        }
    } else {
        bool found = false;
        for (size_t oi = 0; oi < outputs.size(); ++oi) {
            if (outputs[oi].identifier == output) {
                x->n_plugins[pluginindex].n_outputIndex = oi;
                found = true;
                break;
            }
        }
        if (!found) {
            char errormsg[2048];
            snprintf_zero(errormsg, 2048, "Output '%s' not found for plugin '%s' from library '%s'.", output.c_str(),
                          x->n_plugins[pluginindex].n_identifier ? x->n_plugins[pluginindex].n_identifier->s_name : "",
                          x->n_plugins[pluginindex].n_soname ? x->n_plugins[pluginindex].n_soname->s_name : "");
            object_error((t_object *)x, errormsg);
            return 1;
        }
    }
    
    // check if output is OK
    if (tm == EARS_ANALYSIS_TEMPORALMODE_BUFFER && outputs[x->n_plugins[pluginindex].n_outputIndex].sampleType == Plugin::OutputDescriptor::VariableSampleRate) {
        char errormsg[2048];
        snprintf_zero(errormsg, 2048, "Output '%s' of plugin '%s' from library '%s' has a variable sample rate and hence does not support the buffer temporal mode.", output.c_str(),
                      x->n_plugins[pluginindex].n_identifier ? x->n_plugins[pluginindex].n_identifier->s_name : "",
                      x->n_plugins[pluginindex].n_soname ? x->n_plugins[pluginindex].n_soname->s_name : "");
        object_error((t_object *)x, errormsg);
        return 1;
    }

    
    x->n_numplugins ++;
    delete plugin;
    return 0;
}

t_buf_vamp *buf_vamp_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_vamp *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_vamp*)object_alloc_debug(s_tag_class);
    if (x) {
        earsbufobj_init((t_earsbufobj *)x, EARSBUFOBJ_FLAG_SUPPORTS_COPY_NAMES);
        
        x->n_summarization = EARS_ANALYSIS_SUMMARIZATION_MEDIAN;

        x->n_autoframehopsize = true;
        object_attr_setdisabled((t_object *)x, gensym("framesize"), x->n_autoframehopsize);
        object_attr_setdisabled((t_object *)x, gensym("hopsize"), x->n_autoframehopsize);
        object_attr_setdisabled((t_object *)x, gensym("overlap"), x->n_autoframehopsize);
        object_attr_setdisabled((t_object *)x, gensym("numframes"), x->n_autoframehopsize);

        
        // @arg 0 @name plugin_identifiers @optional 1 @type symbol/list
        // @digest List of plugin identifiers in the form: <b>libraryid:pluginid[:output]</b>.
        // The output is optional; if no output is given the first default one is used.
        // Plugins must be located in the VAMP default folder.
        // The ids are the textual identifiers. For instance, a valid argument is
        // <b>bbc-vamp-plugins:bbc-energy</b> or <b>bbc-vamp-plugins:bbc-energy:rmsenergy</b>
        // with explicit specification of an outlet.

        t_llll *args = llll_parse(true_ac, argv);
        
        char errormsg[2048];
        for (t_llllelem *el = args->l_head; el; el = el->l_next) {
            if (hatom_gettype(&el->l_hatom) != H_SYM) {
                llll_free(args);
                snprintf_zero(errormsg, 2048, "Plugin name at position %d is not a symbol!");
                object_error((t_object *)x, errormsg);
                object_free(x);
                return NULL;
            } else if (buf_vamp_add_plugin(x, hatom_getsym(&el->l_hatom))) {
                llll_free(args);
                snprintf_zero(errormsg, 2048, "Error in plugin syntax at position %d", x->n_numplugins+1);
                object_error((t_object *)x, errormsg);
                object_free(x);
                return NULL;
            }
        }
        
        attr_args_process(x, argc, argv);
        

        char outlets[LLLL_MAX_OUTLETS];
        for (long i = 0; i < x->n_numplugins; i++) {
            outlets[x->n_numplugins-i] = (x->n_plugins[i].n_temporalmode == EARS_ANALYSIS_TEMPORALMODE_BUFFER ? 'E' : '4');
        }
        outlets[0] = '4';
        outlets[x->n_numplugins+1] = 0;
       
        char intypes[LLLL_MAX_INLETS];
        intypes[0] = 'E';
        for (long i = 0; i < x->n_numplugins && i < LLLL_MAX_INLETS - 2; i++) {
            intypes[i+1] = '4';
        }
        intypes[CLAMP(x->n_numplugins + 1, 0, LLLL_MAX_INLETS-1)] = 0;

        earsbufobj_setup((t_earsbufobj *)x, intypes, outlets, NULL);

        llll_free(args);
        
    }
    return x;
}


void buf_vamp_free(t_buf_vamp *x)
{
    for (long l = 0; l < x->n_numplugins; l++)
        if (x->n_pluginparameters[l])
            llll_free(x->n_pluginparameters[l]);
    earsbufobj_free((t_earsbufobj *)x);
}


void buf_vamp_bang(t_buf_vamp *x)
{
    long num_buffers = earsbufobj_get_instore_size((t_earsbufobj *)x, 0);
    long num_plugins = x->n_numplugins;
    
    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_buffers, true);
    
    for (long i = 0; i < num_plugins; i++) {
        if (x->n_plugins[i].n_temporalmode == EARS_ANALYSIS_TEMPORALMODE_BUFFER)
            earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_OUT, earsbufobj_outlet_to_bufstore((t_earsbufobj *)x, i), num_buffers, true);
    }
    
    earsbufobj_mutex_lock((t_earsbufobj *)x);
    earsbufobj_init_progress((t_earsbufobj *)x, num_buffers);

    t_llll **res = (t_llll **)bach_newptr(num_plugins * sizeof(t_llll *));
    t_buffer_obj **res_buf = (t_buffer_obj **)bach_newptr(num_plugins * sizeof(t_buffer_obj *));
    for (long i = 0; i < num_plugins; i++)
        res[i] = llll_get();
    
    for (long count = 0; count < num_buffers; count++) {
        t_buffer_obj *buf = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, count);
        if (buf) {
            for (long p = 0; p < x->n_numplugins; p++) {
                t_llll *out_features = NULL;
                t_buffer_obj *featuresbuf = (x->n_plugins[p].n_temporalmode == EARS_ANALYSIS_TEMPORALMODE_BUFFER) ? earsbufobj_get_stored_buffer_obj((t_earsbufobj *)x, EARSBUFOBJ_OUT, earsbufobj_outlet_to_bufstore((t_earsbufobj *)x, p), count) : NULL;
                t_ears_err err = ears_vamp_run_plugin((t_earsbufobj *)x, buf, x->n_plugins[p].n_soname->s_name, x->n_plugins[p].n_identifier->s_name, x->n_plugins[p].n_outputIndex, &out_features, featuresbuf, x->n_plugins[p].n_temporalmode, (e_ears_timeunit)x->e_ob.l_timeunit, (e_ears_analysis_summarization) x->n_summarization, x->n_autoframehopsize ? 0 : ears_convert_timeunit(x->e_ob.a_framesize, buf, (e_ears_timeunit)x->e_ob.l_antimeunit, EARS_TIMEUNIT_SAMPS), x->n_autoframehopsize ? 0 : ears_convert_timeunit(x->e_ob.a_hopsize, buf, (e_ears_timeunit)x->e_ob.l_antimeunit, EARS_TIMEUNIT_SAMPS), x->n_pluginparameters[p]);
                if (err != EARS_ERR_NONE)
                    object_error((t_object *)x, "Error while processing plugins.");
                if (out_features) {
                    if (x->n_plugins[p].n_temporalmode == EARS_ANALYSIS_TEMPORALMODE_WHOLE && out_features->l_depth == 1 && out_features->l_size == 1)
                        llll_chain(res[p], out_features);
                    else if (x->n_plugins[p].n_temporalmode == EARS_ANALYSIS_TEMPORALMODE_BUFFER)
                        llll_free(out_features);
                    else
                        llll_appendllll(res[p], out_features);
                }
            }
        }
        if (earsbufobj_iter_progress((t_earsbufobj *)x, count, num_buffers)) break;
    }
    
    earsbufobj_mutex_unlock((t_earsbufobj *)x);

    for (long o = num_plugins - 1; o >= 0; o--) {
        if (x->n_plugins[o].n_temporalmode == EARS_ANALYSIS_TEMPORALMODE_BUFFER)
            earsbufobj_outlet_buffer((t_earsbufobj *)x, o);
        else
            earsbufobj_outlet_llll((t_earsbufobj *)x, o, res[o]);
    }
    
    for (long i = 0; i < num_plugins; i++)
        llll_free(res[i]);
    
    bach_freeptr(res);
    bach_freeptr(res_buf);

    
//    earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
}

void buf_vamp_getlist(t_buf_vamp *x, t_symbol *msg, long ac, t_atom *av)
{
    t_llll *parsed = earsbufobj_parse_gimme((t_earsbufobj *) x, LLLL_OBJ_VANILLA, NULL, ac, av);
    t_llll *info = NULL;
    
    if (parsed && parsed->l_head && hatom_getsym(&parsed->l_head->l_hatom) == gensym("libraries")) {
        info = ears_vamp_enumerate_libraries((t_object *)x);
    } else if (parsed && parsed->l_head && hatom_getsym(&parsed->l_head->l_hatom) == gensym("detailed")){
        if (parsed->l_head->l_next && hatom_gettype(&parsed->l_head->l_next->l_hatom) == H_SYM) {
            info = ears_vamp_enumerate_plugins_detailed((t_object *)x, hatom_getsym(&parsed->l_head->l_next->l_hatom));
        } else {
            info = ears_vamp_enumerate_plugins_detailed((t_object *)x, NULL);
        }
    } else if (parsed && parsed->l_head && hatom_getsym(&parsed->l_head->l_hatom) == gensym("outputs")){
        info = ears_vamp_enumerate_plugins((t_object *)x, true);
    } else if (parsed && parsed->l_head && hatom_getsym(&parsed->l_head->l_hatom) == gensym("parameters")){
        if (parsed->l_head->l_next && hatom_gettype(&parsed->l_head->l_next->l_hatom) == H_SYM) {
            info = ears_vamp_enumerate_plugin_parameters((t_object *)x, hatom_getsym(&parsed->l_head->l_next->l_hatom));
        } else {
            object_error((t_object *)x, "No plugin identifier given.");
        }
    } else {
        info = ears_vamp_enumerate_plugins((t_object *)x, false);
    }
    
    earsbufobj_outlet_llll((t_earsbufobj *)x, 0, info);
    llll_free(info);
    
    llll_free(parsed);
}

void buf_vamp_anything(t_buf_vamp *x, t_symbol *msg, long ac, t_atom *av)
{
    long inlet = earsbufobj_proxy_getinlet((t_earsbufobj *) x);

    t_llll *parsed = earsbufobj_parse_gimme((t_earsbufobj *) x, LLLL_OBJ_VANILLA, msg, ac, av);
    if (!parsed) return;
    
    if (parsed && parsed->l_head) {
        if (inlet == 0) {
            long num_bufs = llll_get_num_symbols_root(parsed);
            earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_bufs, true);
            
            earsbufobj_store_buffer_list((t_earsbufobj *)x, parsed, 0);
            
            buf_vamp_bang(x);
        } else {
            // change params
            
            earsbufobj_mutex_lock((t_earsbufobj *)x);
            
            if (x->n_pluginparameters[inlet-1]) {
                llll_free(x->n_pluginparameters[inlet-1]);
            }
            
            x->n_pluginparameters[inlet-1] = llll_clone(parsed);
            
            earsbufobj_mutex_unlock((t_earsbufobj *)x);
        }
    }
    llll_free(parsed);
}

