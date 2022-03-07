#include "ears.vamp_commons.h"
#include <vamp-hostsdk/host-c.h>

using namespace std;

using Vamp::Plugin;
using Vamp::PluginHostAdapter;
using Vamp::RealTime;
using Vamp::HostExt::PluginLoader;
using Vamp::HostExt::PluginWrapper;
using Vamp::HostExt::PluginInputDomainAdapter;


long ears_vamp_plugin_from_name()
{
    
}


t_llll *ears_vamp_enumerate_libraries(t_object *x)
{
    t_llll *out_ll = llll_get();
    int i;
    int libcount = vhGetLibraryCount();
    
//    printf("Vamp plugin libraries found:\n");
    for (i = 0; i < libcount; ++i) {
//        printf("%d: %s\n", i, vhGetLibraryName(i));
        llll_appendsym(out_ll, gensym(vhGetLibraryName(i)));
    }
    
    /*
    printf("Going to try loading qm-vamp-plugins...\n");
    int libindex = vhGetLibraryIndex("qm-vamp-plugins");
    vhLibrary lib = vhLoadLibrary(libindex);
    if (!lib) {
        object_error((t_object *)x, "Error while enumerating VAMP libraries.");
        return out_ll;
    }
    vhUnloadLibrary(lib);
     */
    
    return out_ll;
}

t_llll *ears_vamp_enumerate_plugins(t_object *x, bool enumerate_outlets)
{
    PluginLoader *loader = PluginLoader::getInstance();
    
    vector<PluginLoader::PluginKey> plugins = loader->listPlugins();
    typedef multimap<string, PluginLoader::PluginKey> LibraryMap;
    LibraryMap libraryMap;
    
    for (size_t i = 0; i < plugins.size(); ++i) {
        string path = loader->getLibraryPathForPlugin(plugins[i]);
        libraryMap.insert(LibraryMap::value_type(path, plugins[i]));
    }
    
    string prevPath = "";
    int index = 0;
    
    t_llll *out_ll = llll_get();
    t_llll *library_ll = llll_get();
    string library = "";
    for (LibraryMap::iterator i = libraryMap.begin(); i != libraryMap.end(); ++i) {

        string path = i->first;
        PluginLoader::PluginKey key = i->second;
        
        PluginLoader::PluginCategoryHierarchy category = loader->getPluginCategory(key);
        
        if (path != prevPath) {
            prevPath = path;
            index = 0;
            string::size_type ki = i->second.find(':');
            library = i->second.substr(0, ki);
            if (library_ll->l_size > 0) {
                llll_appendllll(out_ll, library_ll);
                library_ll = llll_get();
            }
        }
        
        Plugin *plugin = loader->loadPlugin(key, 48000);
        if (plugin) {
            
            char c = char('A' + index);
            if (c > 'Z') c = char('a' + (index - 26));

            char pluginfullidentifier[2048];
            snprintf_zero(pluginfullidentifier, 2048, "%s:%s", library.c_str(), plugin->getIdentifier().c_str());
            
            if (enumerate_outlets) {
                char outletfullidentifier[2048];

                t_llll *plugin_ll = llll_get();
                Plugin::OutputList outputs = plugin->getOutputDescriptors();
                for (size_t j = 0; j < outputs.size(); ++j) {
                    Plugin::OutputDescriptor &od(outputs[j]);
                    snprintf_zero(outletfullidentifier, 2048, "%s:%s", pluginfullidentifier, od.identifier.c_str());
                    llll_appendsym(plugin_ll, gensym(outletfullidentifier));
                }
                llll_appendllll(library_ll, plugin_ll);
            } else {
                llll_appendsym(library_ll, gensym(pluginfullidentifier));
            }
        }
    }
    
    if (library_ll->l_size > 0) {
        llll_appendllll(out_ll, library_ll);
    } else {
        llll_free(library_ll);
    }

    return out_ll;
}

t_llll *ears_vamp_enumerate_plugins_detailed(t_object *x)
{
    PluginLoader *loader = PluginLoader::getInstance();
    
    vector<PluginLoader::PluginKey> plugins = loader->listPlugins();
    typedef multimap<string, PluginLoader::PluginKey>
    LibraryMap;
    LibraryMap libraryMap;
    
    for (size_t i = 0; i < plugins.size(); ++i) {
        string path = loader->getLibraryPathForPlugin(plugins[i]);
        libraryMap.insert(LibraryMap::value_type(path, plugins[i]));
    }
    
    string prevPath = "";
    int index = 0;
    
    t_llll *out_ll = llll_get();
    
    t_symbol *library = gensym("");
    for (LibraryMap::iterator i = libraryMap.begin();
         i != libraryMap.end(); ++i) {
        
        string path = i->first;
        PluginLoader::PluginKey key = i->second;
        
        PluginLoader::PluginCategoryHierarchy category = loader->getPluginCategory(key);
        
        if (path != prevPath) {
            prevPath = path;
            index = 0;
            string::size_type ki = i->second.find(':');
            library = gensym(i->second.substr(0, ki).c_str());
        }
        
        Plugin *plugin = loader->loadPlugin(key, 48000);
        if (plugin) {
            
            t_llll *plugin_ll = llll_get();
            
            char c = char('A' + index);
            if (c > 'Z') c = char('a' + (index - 26));
            
            PluginLoader::PluginCategoryHierarchy category =
            loader->getPluginCategory(key);
            string catstr;
            if (!category.empty()) {
                for (size_t ci = 0; ci < category.size(); ++ci) {
                    if (ci > 0) catstr += " > ";
                    catstr += category[ci];
                }
            }
            
            llll_appendllll(plugin_ll, symbol_and_symbol_to_llll(gensym("name"), plugin->getName().c_str() ? gensym(plugin->getName().c_str()) : gensym("")));
            
            // category
            t_llll *category_ll = llll_get();
            llll_appendsym(category_ll, gensym("category"));
            if (!category.empty()) {
                for (size_t j = 0; j < category.size(); ++j) {
                    llll_appendsym(category_ll, gensym(category[j].c_str()));
                }
            }
            llll_appendllll(plugin_ll, category_ll);
            
            llll_appendllll(plugin_ll, symbol_and_symbol_to_llll(gensym("library"), library));
            //            llll_appendllll(plugin_ll, symbol_and_symbol_to_llll(gensym("identifier"), key ? gensym(key.c_str()) : gensym("")));
            llll_appendllll(plugin_ll, symbol_and_symbol_to_llll(gensym("identifier"), plugin->getIdentifier().c_str() ? gensym(plugin->getIdentifier().c_str()) : gensym("")));
            llll_appendllll(plugin_ll, symbol_and_long_to_llll(gensym("pluginversion"), plugin->getPluginVersion()));
            llll_appendllll(plugin_ll, symbol_and_long_to_llll(gensym("vampapiversion"), plugin->getVampApiVersion()));
            llll_appendllll(plugin_ll, symbol_and_symbol_to_llll(gensym("maker"), plugin->getMaker().c_str() ? gensym(plugin->getMaker().c_str()) : gensym("")));
            llll_appendllll(plugin_ll, symbol_and_symbol_to_llll(gensym("copyright"), plugin->getCopyright().c_str() ? gensym(plugin->getCopyright().c_str()) : gensym("")));
            llll_appendllll(plugin_ll, symbol_and_symbol_to_llll(gensym("description"), plugin->getDescription().c_str() ? gensym(plugin->getDescription().c_str()) : gensym("")));
            llll_appendllll(plugin_ll, symbol_and_symbol_to_llll(gensym("inputdomain"), (plugin->getInputDomain() == Vamp::Plugin::TimeDomain ?
                                                                                         gensym("time") : gensym("frequency"))));
            llll_appendllll(plugin_ll, symbol_and_long_to_llll(gensym("preferredstepsize"), plugin->getPreferredStepSize()));
            llll_appendllll(plugin_ll, symbol_and_long_to_llll(gensym("preferredblocksize"), plugin->getPreferredBlockSize()));
            llll_appendllll(plugin_ll, symbol_and_long_to_llll(gensym("minchannelcount"), plugin->getMinChannelCount()));
            llll_appendllll(plugin_ll, symbol_and_long_to_llll(gensym("maxchannelcount"), plugin->getMaxChannelCount()));
            
            t_llll *parameters_ll = llll_get();
            Plugin::ParameterList params = plugin->getParameterDescriptors();
            llll_appendsym(parameters_ll, gensym("parameters"));
            for (size_t j = 0; j < params.size(); ++j) {
                Plugin::ParameterDescriptor &pd(params[j]);
                t_llll *this_param_ll = llll_get();
                llll_appendllll(this_param_ll, symbol_and_long_to_llll(gensym("index"), j+1));
                llll_appendllll(this_param_ll, symbol_and_symbol_to_llll(gensym("name"), pd.name.c_str() ? gensym(pd.name.c_str()) : gensym("")));
                llll_appendllll(this_param_ll, symbol_and_symbol_to_llll(gensym("identifier"), pd.identifier.c_str() ? gensym(pd.identifier.c_str()) : gensym("")));
                llll_appendllll(this_param_ll, symbol_and_symbol_to_llll(gensym("description"), pd.description.c_str() ? gensym(pd.description.c_str()) : gensym("")));
                llll_appendllll(this_param_ll, symbol_and_symbol_to_llll(gensym("unit"), pd.unit.c_str() ? gensym(pd.unit.c_str()) : gensym("")));
                
                t_llll *range_ll = llll_get();
                llll_appendsym(range_ll, gensym("range"));
                llll_appenddouble(range_ll, pd.minValue);
                llll_appenddouble(range_ll, pd.maxValue);
                llll_appendllll(this_param_ll, range_ll);
                
                llll_appendllll(this_param_ll, symbol_and_double_to_llll(gensym("default"), pd.defaultValue));
                llll_appendllll(this_param_ll, symbol_and_long_to_llll(gensym("isquantized"), pd.isQuantized));
                llll_appendllll(this_param_ll, symbol_and_double_to_llll(gensym("quantizestep"), pd.isQuantized ? pd.quantizeStep : 0));
                
                t_llll *value_names_ll = llll_get();
                llll_appendsym(value_names_ll, gensym("valuenames"));
                if (!pd.valueNames.empty()) {
                    for (size_t k = 0; k < pd.valueNames.size(); ++k) {
                        llll_appendsym(value_names_ll, pd.valueNames[k].c_str() ? gensym(pd.valueNames[k].c_str()) : gensym(""));
                    }
                }
                llll_appendllll(this_param_ll, value_names_ll);
                
                llll_appendllll(parameters_ll, this_param_ll);
            }
            llll_appendllll(plugin_ll, parameters_ll);
            
            
            t_llll *outputs_ll = llll_get();
            Plugin::OutputList outputs = plugin->getOutputDescriptors();
            llll_appendsym(parameters_ll, gensym("outputs"));
            for (size_t j = 0; j < outputs.size(); ++j) {
                Plugin::OutputDescriptor &od(outputs[j]);
                t_llll *this_output_ll = llll_get();
                llll_appendllll(this_output_ll, symbol_and_long_to_llll(gensym("index"), j+1));
                llll_appendllll(this_output_ll, symbol_and_symbol_to_llll(gensym("name"), od.name.c_str() ? gensym(od.name.c_str()) : gensym("")));
                llll_appendllll(this_output_ll, symbol_and_symbol_to_llll(gensym("identifier"), od.identifier.c_str() ? gensym(od.identifier.c_str()) : gensym("")));
                llll_appendllll(this_output_ll, symbol_and_symbol_to_llll(gensym("description"), od.description.c_str() ? gensym(od.description.c_str()) : gensym("")));
                llll_appendllll(this_output_ll, symbol_and_symbol_to_llll(gensym("unit"), od.unit.c_str() ? gensym(od.unit.c_str()) : gensym("")));
                llll_appendllll(this_output_ll, symbol_and_long_to_llll(gensym("bincount"), od.hasFixedBinCount ? od.binCount : 0));
                
                t_llll *bin_names_ll = llll_get();
                llll_appendsym(bin_names_ll, gensym("binnames"));
                if (!od.binNames.empty()) {
                    bool have = false;
                    for (size_t k = 0; k < od.binNames.size(); ++k) {
                        if (od.binNames[k] != "") {
                            have = true; break;
                        }
                    }
                    if (have) {
                        for (size_t k = 0; k < od.binNames.size(); ++k) {
                            llll_appendsym(bin_names_ll, od.binNames[k].c_str() ? gensym(od.binNames[k].c_str()) : gensym(""));
                        }
                    }
                }
                llll_appendllll(this_output_ll, bin_names_ll);
                
                llll_appendllll(this_output_ll, symbol_and_long_to_llll(gensym("hasknownextents"), od.hasKnownExtents));
                
                t_llll *default_extents_ll = llll_get();
                llll_appendsym(default_extents_ll, gensym("defaultextents"));
                if (od.hasKnownExtents) {
                    llll_appenddouble(default_extents_ll, od.minValue);
                    llll_appenddouble(default_extents_ll, od.maxValue);
                }
                llll_appendllll(this_output_ll, default_extents_ll);
                
                llll_appendllll(this_output_ll, symbol_and_long_to_llll(gensym("isquantized"), od.isQuantized));
                llll_appendllll(this_output_ll, symbol_and_double_to_llll(gensym("quantizestep"), od.isQuantized ? od.quantizeStep : 0));
                
                llll_appendllll(this_output_ll, symbol_and_symbol_to_llll(gensym("sampletype"),
                                                                          (od.sampleType ==
                                                                           Plugin::OutputDescriptor::OneSamplePerStep ?
                                                                           gensym("onesampleperstep") :
                                                                           od.sampleType ==
                                                                           Plugin::OutputDescriptor::FixedSampleRate ?
                                                                           gensym("fixedsamplerate") :
                                                                           gensym("variablesamplerate"))
                                                                          ));
                
                llll_appendllll(this_output_ll, symbol_and_double_to_llll(gensym("defaultrate"), od.sampleRate));
                llll_appendllll(this_output_ll, symbol_and_long_to_llll(gensym("hasduration"), od.hasDuration));
                
                llll_appendllll(outputs_ll, this_output_ll);
            }
            llll_appendllll(plugin_ll, outputs_ll);
            
            /*
             if (outputs.size() > 1 || verbosity == PluginOutputIds) {
             for (size_t j = 0; j < outputs.size(); ++j) {
             if (verbosity == PluginInformation) {
             cout << "         (" << j << ") "
             << outputs[j].name << ", \""
             << outputs[j].identifier << "\"" << endl;
             if (outputs[j].description != "") {
             cout << "             - "
             << outputs[j].description << endl;
             }
             } else if (verbosity == PluginOutputIds) {
             cout << "vamp:" << key << ":" << outputs[j].identifier << endl;
             }
             }
             } */
            
            ++index;
            
            llll_appendllll(out_ll, plugin_ll);
            delete plugin;
        }
    }
    
    return out_ll;
}


long wrap_once_fn(void *data, t_hatom *a, const t_llll *address){
    if (is_hatom_number(a)) {
        t_llll *ll = llll_get();
        llll_appenddouble(ll, hatom_getdouble(a));
        hatom_change_to_llll_and_free(a, ll);
    }
    
    return 0;
}

t_ears_err ears_vamp_run_plugin(t_earsbufobj *e_ob, t_buffer_obj *buf, string soname, string identifier, int outputNo,
                        t_llll **out_features, e_ears_analysis_temporalmode temporalmode, e_ears_timeunit timeunit, e_ears_analysis_summarization summarization, long framesize_samps, long hopsize_samps, t_llll *params)
{
    t_ears_err err = EARS_ERR_NONE;
    bool useFrames = (e_ob->l_timeunit == EARS_TIMEUNIT_SAMPS);
    
    t_object *x = (t_object *)e_ob;
    t_llll *out_ll = llll_get();
    PluginLoader *loader = PluginLoader::getInstance();
    
    PluginLoader::PluginKey key = loader->composePluginKey(soname, identifier);
    
    double sr = ears_buffer_get_sr((t_object *)x, buf);
    
    Plugin *plugin = loader->loadPlugin(key, sr, PluginLoader::ADAPT_ALL_SAFE);
    if (!plugin) {
        char error_msg[2048];
        snprintf_zero(error_msg, 2048, "Failed to load plugin '%s' from library '%s'", identifier.c_str(), soname.c_str());
        object_error((t_object *)x, error_msg);
        return EARS_ERR_GENERIC;
    }
    
    //    cerr << "Running plugin: \"" << plugin->getIdentifier() << "\"..." << endl;
    
    // Note that the following would be much simpler if we used a
    // PluginBufferingAdapter as well -- i.e. if we had passed
    // PluginLoader::ADAPT_ALL to loader->loadPlugin() above, instead
    // of ADAPT_ALL_SAFE.  Then we could simply specify our own block
    // size, keep the step size equal to the block size, and ignore
    // the plugin's bleatings.  However, there are some issues with
    // using a PluginBufferingAdapter that make the results sometimes
    // technically different from (if effectively the same as) the
    // un-adapted plugin, so we aren't doing that here.  See the
    // PluginBufferingAdapter documentation for details.
    
    int blockSize = framesize_samps != 0 ? framesize_samps : plugin->getPreferredBlockSize();
    int stepSize = hopsize_samps != 0 ? hopsize_samps : plugin->getPreferredStepSize();
    
    if (plugin->getInputDomain() == Plugin::FrequencyDomain) {
        object_warn((t_object *)x, "Frequency domain plugins are currently unsupported.");
    }
    
    if (blockSize == 0) {
        blockSize = 1024;
    }
    if (stepSize == 0) {
        if (plugin->getInputDomain() == Plugin::FrequencyDomain) {
            stepSize = blockSize/2;
        } else {
            stepSize = blockSize;
        }
    } else if (stepSize > blockSize) {
        object_warn((t_object *)e_ob, "Step size is greater than block size. Modifying block size.");
//        cerr << "WARNING: stepSize " << stepSize << " > blockSize " << blockSize << ", resetting blockSize to ";
        if (plugin->getInputDomain() == Plugin::FrequencyDomain) {
            blockSize = stepSize * 2;
        } else {
            blockSize = stepSize;
        }
//        cerr << blockSize << endl;
    }
    long currentStep = 0;
    int finalStepsRemaining = max(1, (blockSize / stepSize) - 1); // at end of file, this many part-silent frames needed after we hit EOF
    
    int channels = ears_buffer_get_numchannels((t_object *)x, buf);
    
    //    float *filebuf = new float[blockSize * channels];
    float **plugbuf = new float*[channels];
    for (int c = 0; c < channels; ++c)
        plugbuf[c] = new float[blockSize + 2];
    
    //    cerr << "Using block size = " << blockSize << ", step size = " << stepSize << endl;
    
    // The channel queries here are for informational purposes only --
    // a PluginChannelAdapter is being used automatically behind the
    // scenes, and it will take case of any channel mismatch
    
    int minch = plugin->getMinChannelCount();
    int maxch = plugin->getMaxChannelCount();
    //    cerr << "Plugin accepts " << minch << " -> " << maxch << " channel(s)" << endl;
    //    cerr << "Sound file has " << channels << " (will mix/augment if necessary)" << endl;
    
    // no need to worry about different channels, plugins will mix/augment automatically.
/*    if (channels < minch || channels > maxch) {
        char error_msg[2048];
        snprintf_zero(error_msg, 2048, "Plugin '%s' from library '%s' accepts between %ld and %ld channels (input buffer has %ld).", identifier.c_str(), soname.c_str(), minch, maxch, channels);
        object_warn(x, error_msg);
        object_warn(x, "    Plugin will mix or augment channels if necessary.");
    }*/
    
    Plugin::OutputList outputs = plugin->getOutputDescriptors();
    Plugin::OutputDescriptor od;
    Plugin::FeatureSet features;
    
    int returnValue = 1;
    //    int progress = 0;
    
    RealTime rt;
    PluginWrapper *wrapper = 0;
    RealTime adjustment = RealTime::zeroTime;
    
    t_float *bufsamps = NULL;
    
    long num_samps = 0, cur = 0;
    
    if (outputs.empty()) {
        char error_msg[2048];
        snprintf_zero(error_msg, 2048, "Plugin '%s' from library '%s' has no outputs", identifier.c_str(), soname.c_str());
        object_error((t_object *)x, error_msg);
        err = EARS_ERR_GENERIC;
        goto done;
    }
    
    if (int(outputs.size()) <= outputNo) {
        char error_msg[2048];
        snprintf_zero(error_msg, 2048, "Output %ld requested, but plugin '%s' from library '%s' has only %ld output(s)", outputNo, identifier.c_str(), soname.c_str(), outputs.size());
        object_error((t_object *)x, error_msg);
        err = EARS_ERR_GENERIC;
        goto done;
    }
    
    od = outputs[outputNo];
    //    cerr << "Output is: \"" << od.identifier << "\"" << endl;
    
    if (!plugin->initialise(channels, stepSize, blockSize)) {
        char error_msg[2048];
        snprintf_zero(error_msg, 2048, "Error in plugin initialization for plugin '%s' from library '%s'.", identifier.c_str(), soname.c_str(), outputs.size());
        object_error((t_object *)x, error_msg);
        //        cerr << "ERROR: Plugin initialise (channels = " << channels
        //        << ", stepSize = " << stepSize << ", blockSize = "
        //        << blockSize << ") failed." << endl;
        err = EARS_ERR_GENERIC;
        goto done;
    }
    
    // parameters
    if (params) {
        for (t_llllelem *el = params->l_head; el; el = el->l_next) {
            if (hatom_gettype(&el->l_hatom) == H_LLLL) {
                t_llll *ll = hatom_getllll(&el->l_hatom);
                if (ll->l_size >= 2 && hatom_gettype(&ll->l_head->l_hatom) == H_SYM && is_hatom_number(&ll->l_head->l_next->l_hatom)) {
                    t_symbol *parameter = hatom_getsym(&ll->l_head->l_hatom);
                    double value = hatom_getdouble(&ll->l_head->l_next->l_hatom);
                    plugin->setParameter(parameter->s_name, value); // does this work?
                }
            }
        }
    }
    
    wrapper = dynamic_cast<PluginWrapper *>(plugin);
    if (wrapper) {
        // See documentation for
        // PluginInputDomainAdapter::getTimestampAdjustment
        PluginInputDomainAdapter *ida = wrapper->getWrapper<PluginInputDomainAdapter>();
        if (ida) adjustment = ida->getTimestampAdjustment();
    }
    
    bufsamps = buffer_locksamples(buf);
    
    if (!bufsamps) {
        object_error((t_object *)x, EARS_ERROR_BUF_CANT_READ);
        err = EARS_ERR_CANT_READ;
        goto done;
    }
    
    num_samps = ears_buffer_get_size_samps((t_object *)x, buf);
    cur = 0;
    
    // Here we iterate over the frames, avoiding asking the numframes in case it's streaming input.
    do {
        
        /*
         int count;
         if ((blockSize==stepSize) || (currentStep==0)) {
         // read a full fresh block
         if ((count = sf_readf_float(sndfile, filebuf, blockSize)) < 0) {
         cerr << "ERROR: sf_readf_float failed: " << sf_strerror(sndfile) << endl;
         break;
         }
         if (count != blockSize) --finalStepsRemaining;
         } else {
         //  otherwise shunt the existing data down and read the remainder.
         memmove(filebuf, filebuf + (stepSize * channels), overlapSize * channels * sizeof(float));
         if ((count = sf_readf_float(sndfile, filebuf + (overlapSize * channels), stepSize)) < 0) {
         cerr << "ERROR: sf_readf_float failed: " << sf_strerror(sndfile) << endl;
         break;
         }
         if (count != stepSize) {
         memset(filebuf + ((overlapSize + count) * channels), 0,
         (stepSize - count) * channels * sizeof(float));
         --finalStepsRemaining;
         }
         
         count += overlapSize;
         }
         */
        
        for (int c = 0; c < channels; ++c) {
            int j = 0;
            while (j + cur < num_samps && j < blockSize) {
                plugbuf[c][j] = bufsamps[(j + cur) * channels + c];
                //                plugbuf[c][j] = filebuf[j * channels + c];
                ++j;
            }
            while (j < blockSize) {
                plugbuf[c][j] = 0.0f;
                ++j;
            }
        }
        
        
        rt = RealTime::frame2RealTime(currentStep * stepSize, sr);
        
        features = plugin->process(plugbuf, rt);
        
        ears_vamp_append_features_to_llll(RealTime::realTime2Frame(rt + adjustment, sr), sr, od, outputNo, features, useFrames, out_ll, temporalmode, timeunit);
        
        cur += stepSize;

        ++currentStep;
        
    } while (cur < num_samps);
    
    buffer_unlocksamples(buf);
    
    
    rt = RealTime::frame2RealTime(currentStep * stepSize, sr);
    
    features = plugin->getRemainingFeatures();
    
    ears_vamp_append_features_to_llll(RealTime::realTime2Frame(rt + adjustment, sr), sr, od, outputNo, features, useFrames, out_ll, temporalmode, timeunit);
//    llll_appendllll(out_ll, getFeatures(RealTime::realTime2Frame(rt + adjustment, sr), sr, od, outputNo, features, useFrames));
    
    // summarization
    if (temporalmode == EARS_ANALYSIS_TEMPORALMODE_WHOLE) {
        bool wrapped = false;
        if (out_ll->l_depth == 1) {
            llll_funall(out_ll, (fun_fn) wrap_once_fn, NULL, 1, 1, FUNALL_ONLY_PROCESS_ATOMS);
            wrapped = true;
        }
        
        t_llll *trans_ll = llll_trans(out_ll, 0);
        for (t_llllelem *el = trans_ll->l_head; el; el = el->l_next) {
            if (hatom_gettype(&el->l_hatom) == H_LLLL) {
                // summarize el
                t_llll *ll = hatom_getllll(&el->l_hatom);
                double summary = 0;
                t_llllelem *summary_el = NULL;
                switch (summarization) {
                    case EARS_ANALYSIS_SUMMARIZATION_FIRST:
                        if (ll->l_head)
                            summary = hatom_getdouble(&ll->l_head->l_hatom);
                        break;
                    case EARS_ANALYSIS_SUMMARIZATION_LAST:
                        if (ll->l_head)
                            summary = hatom_getdouble(&ll->l_tail->l_hatom);
                        break;
                    case EARS_ANALYSIS_SUMMARIZATION_MIDDLE:
                        summary_el = llll_getindex(ll, ll->l_size/2, I_STANDARD);
                        if (summary_el)
                            summary = hatom_getdouble(&summary_el->l_hatom);
                        break;
                    case EARS_ANALYSIS_SUMMARIZATION_MEDIAN:
                    {
                        llll_inplacesort(ll);
                        summary_el = llll_getindex(ll, ll->l_size/2, I_STANDARD);
                        if (summary_el) {
                            if (ll->l_size % 2 == 0 && summary_el->l_prev) {
                                summary = 0.5 * (hatom_getdouble(&summary_el->l_prev->l_hatom) + hatom_getdouble(&summary_el->l_hatom));
                            } else {
                                summary = hatom_getdouble(&summary_el->l_hatom);
                            }
                        }
                    }
                        break;
                    case EARS_ANALYSIS_SUMMARIZATION_MEAN:
                    {
                        summary = llll_average_of_plain_double_llll(ll);
                    }
                        break;
                    case EARS_ANALYSIS_SUMMARIZATION_MODE:
                    {
                        // TO DO
                    }
                        break;
                    default:
                        break;
                }
                llll_clear(ll);
                llll_appenddouble(ll, summary);
            }
        }
                
        llll_flatten(trans_ll, -1, 0);
        *out_features = trans_ll;
    } else {
        *out_features = out_ll;
    }
    
    returnValue = 0;
    
done:
    for (int c = 0; c < channels; ++c)
        delete plugbuf[c];
    delete [] plugbuf;
    delete plugin;
    
    return 0;
}



static double toSeconds(const RealTime &time)
{
    return time.sec + double(time.nsec + 1) / 1000000000.0; // why +1 ? it was in the vamp stuff, let's keep it.
}


void ears_vamp_append_features_to_llll(int frame, int sr,
                                    const Plugin::OutputDescriptor &output, int outputNo,
                                    const Plugin::FeatureSet &features, bool useFrames,
                                    t_llll *features_ll, e_ears_analysis_temporalmode temporalmode, e_ears_timeunit timeunit)
{

    static int featureCount = -1;
    
    if (features.find(outputNo) == features.end())
        return;
    
    for (size_t i = 0; i < features.at(outputNo).size(); ++i) {
        const Plugin::Feature &f = features.at(outputNo).at(i);
        bool hasduration = false;
        double timestamp = 0, durationstamp = 0;
        
        if (temporalmode == EARS_ANALYSIS_TEMPORALMODE_LABELLEDTIMESERIES) {
            bool haveRt = false;
            RealTime rt;
            
            if (output.sampleType == Plugin::OutputDescriptor::VariableSampleRate) {
                rt = f.timestamp;
                haveRt = true;
            } else if (output.sampleType == Plugin::OutputDescriptor::FixedSampleRate) {
                if (f.hasTimestamp) {
                    int n = int(round(toSeconds(f.timestamp) * output.sampleRate));
                    haveRt = true;
                    rt = RealTime::fromSeconds(double(n) / output.sampleRate);
                }
/*                int n = featureCount + 1;
                if (f.hasTimestamp) {
                    n = int(round(toSeconds(f.timestamp) * output.sampleRate));
                }
                rt = RealTime::fromSeconds(double(n) / output.sampleRate);
                haveRt = true;
                featureCount = n; */
            }
            
            if (timeunit == EARS_TIMEUNIT_SAMPS) {
                
                int displayFrame = frame;
                
                if (haveRt) {
                    displayFrame = RealTime::realTime2Frame(rt, sr);
                }
                
                timestamp = rt.msec();
//                llll_appendlong(frametimes_ll, displayFrame);
                //            (out ? *out : cout) << displayFrame;
                
                if (f.hasDuration) {
                    displayFrame = RealTime::realTime2Frame(f.duration, sr);
                    durationstamp = displayFrame;
                    hasduration = true;
//                    llll_appendlong(frametimes_ll, displayFrame);
                    //                (out ? *out : cout) << "," << displayFrame;
                }
                
                //            (out ? *out : cout)  << ":";
                
            } else {
                
                if (!haveRt) {
                    rt = RealTime::frame2RealTime(frame, sr);
                }
                
                timestamp = rt.msec();
//                llll_appenddouble(frametimes_ll, rt.msec());
                //            (out ? *out : cout) << rt.toString();
                
                if (f.hasDuration) {
                    rt = f.duration;
                    durationstamp = rt.msec();
                    hasduration = true;
//                    llll_appenddouble(frametimes_ll, rt.msec());
                    //                (out ? *out : cout) << "," << rt.toString();
                }
                
                //            (out ? *out : cout) << ":";
            }
        }
        
        
        
        if (f.values.size() == 1) {
            if (temporalmode == EARS_ANALYSIS_TEMPORALMODE_LABELLEDTIMESERIES) {
                if (hasduration) {
                    t_llll *inner = llll_get();
                    llll_appendllll(inner, double_couple_to_llll(timestamp, durationstamp));
                    llll_appenddouble(inner, f.values[0]);
                    llll_appendllll(features_ll, inner);
                } else {
                    llll_appendllll(features_ll, double_couple_to_llll(timestamp, f.values[0]));
                }
            } else {
                llll_appenddouble(features_ll, f.values[0]);
            }
        } else {
            t_llll *this_frame_ll = llll_get();
            if (temporalmode == EARS_ANALYSIS_TEMPORALMODE_LABELLEDTIMESERIES) {
                if (hasduration) {
                    t_llll *inner = double_couple_to_llll(timestamp, durationstamp);
                    llll_appendllll(this_frame_ll, inner);
                } else {
                    llll_appenddouble(this_frame_ll, timestamp);
                }
            }
            for (unsigned int j = 0; j < f.values.size(); ++j) {
                llll_appenddouble(this_frame_ll, f.values[j]);
            }
            llll_appendllll(features_ll, this_frame_ll);
        }
    }
}

t_llll *ears_vamp_get_frametime_as_llll(int frame, int sr,
                    const Plugin::OutputDescriptor &output, int outputNo,
                    const Plugin::FeatureSet &features, bool useFrames)
{
    t_llll *out_ll = llll_get();
    
    static int featureCount = -1;
    
    if (features.find(outputNo) == features.end())
        return out_ll;
    
    for (size_t i = 0; i < features.at(outputNo).size(); ++i) {
        t_llll *feature_ll = llll_get();
        const Plugin::Feature &f = features.at(outputNo).at(i);
        
        bool haveRt = false;
        RealTime rt;
        
        if (output.sampleType == Plugin::OutputDescriptor::VariableSampleRate) {
            rt = f.timestamp;
            haveRt = true;
        } else if (output.sampleType == Plugin::OutputDescriptor::FixedSampleRate) {
            int n = featureCount + 1;
            if (f.hasTimestamp) {
                n = int(round(toSeconds(f.timestamp) * output.sampleRate));
            }
            rt = RealTime::fromSeconds(double(n) / output.sampleRate);
            haveRt = true;
            featureCount = n;
        }
        
        if (useFrames) {
            
            int displayFrame = frame;
            
            if (haveRt) {
                displayFrame = RealTime::realTime2Frame(rt, sr);
            }
            
            llll_appendlong(feature_ll, displayFrame);
            //            (out ? *out : cout) << displayFrame;
            
            if (f.hasDuration) {
                displayFrame = RealTime::realTime2Frame(f.duration, sr);
                llll_appendlong(feature_ll, displayFrame);
                //                (out ? *out : cout) << "," << displayFrame;
            }
            
            //            (out ? *out : cout)  << ":";
            
        } else {
            
            if (!haveRt) {
                rt = RealTime::frame2RealTime(frame, sr);
            }
            
            llll_appenddouble(feature_ll, rt.msec());
            //            (out ? *out : cout) << rt.toString();
            
            if (f.hasDuration) {
                rt = f.duration;
                llll_appenddouble(feature_ll, rt.msec());
                //                (out ? *out : cout) << "," << rt.toString();
            }
            
            //            (out ? *out : cout) << ":";
        }
        
        for (unsigned int j = 0; j < f.values.size(); ++j) {
            llll_appenddouble(feature_ll, f.values[j]);
            //            (out ? *out : cout) << " " << f.values[j];
        }
        //        (out ? *out : cout) << " " << f.label;
        
        //        (out ? *out : cout) << endl;
        llll_appendllll(out_ll, feature_ll);
    }
    
    return out_ll;
}

t_llll *getFeatures(int frame, int sr,
            const Plugin::OutputDescriptor &output, int outputNo,
            const Plugin::FeatureSet &features, bool useFrames)
{
    t_llll *out_ll = llll_get();
    
    static int featureCount = -1;
    
    if (features.find(outputNo) == features.end())
        return out_ll;
    
    for (size_t i = 0; i < features.at(outputNo).size(); ++i) {
        t_llll *feature_ll = llll_get();
        const Plugin::Feature &f = features.at(outputNo).at(i);
        
        bool haveRt = false;
        RealTime rt;
        
        if (output.sampleType == Plugin::OutputDescriptor::VariableSampleRate) {
            rt = f.timestamp;
            haveRt = true;
        } else if (output.sampleType == Plugin::OutputDescriptor::FixedSampleRate) {
            int n = featureCount + 1;
            if (f.hasTimestamp) {
                n = int(round(toSeconds(f.timestamp) * output.sampleRate));
            }
            rt = RealTime::fromSeconds(double(n) / output.sampleRate);
            haveRt = true;
            featureCount = n;
        }
        
        if (useFrames) {
            
            int displayFrame = frame;
            
            if (haveRt) {
                displayFrame = RealTime::realTime2Frame(rt, sr);
            }
            
            llll_appendlong(feature_ll, displayFrame);
            //            (out ? *out : cout) << displayFrame;
            
            if (f.hasDuration) {
                displayFrame = RealTime::realTime2Frame(f.duration, sr);
                llll_appendlong(feature_ll, displayFrame);
                //                (out ? *out : cout) << "," << displayFrame;
            }
            
            //            (out ? *out : cout)  << ":";
            
        } else {
            
            if (!haveRt) {
                rt = RealTime::frame2RealTime(frame, sr);
            }
            
            llll_appenddouble(feature_ll, rt.msec());
            //            (out ? *out : cout) << rt.toString();
            
            if (f.hasDuration) {
                rt = f.duration;
                llll_appenddouble(feature_ll, rt.msec());
                //                (out ? *out : cout) << "," << rt.toString();
            }
            
            //            (out ? *out : cout) << ":";
        }
        
        for (unsigned int j = 0; j < f.values.size(); ++j) {
            llll_appenddouble(feature_ll, f.values[j]);
            //            (out ? *out : cout) << " " << f.values[j];
        }
        //        (out ? *out : cout) << " " << f.label;
        
        //        (out ? *out : cout) << endl;
        llll_appendllll(out_ll, feature_ll);
    }
    
    return out_ll;
}
