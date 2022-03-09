/**
	@file
	ears.vamp_commons.h
	Vamp bridge for ears
 
	by Daniele Ghisi
 */

#ifndef _EARS_BUF_VAMP_COMMONS_H_
#define _EARS_BUF_VAMP_COMMONS_H_

#include "ears.commons.h"

#include "vamp-hostsdk/PluginHostAdapter.h"
#include "vamp-hostsdk/PluginInputDomainAdapter.h"
#include "vamp-hostsdk/PluginLoader.h"

#include <iostream>
#include <fstream>
#include <set>
//#include <sndfile.h>

#include <cstring>
#include <cstdlib>

#include "system.h"

#include <cmath>

using namespace std;

using Vamp::Plugin;
using Vamp::PluginHostAdapter;
using Vamp::RealTime;
using Vamp::HostExt::PluginLoader;
using Vamp::HostExt::PluginWrapper;
using Vamp::HostExt::PluginInputDomainAdapter;

t_llll *ears_vamp_enumerate_plugins(t_object *x, bool enumerate_outlets);
t_llll *ears_vamp_enumerate_plugin_parameters(t_object *x, t_symbol *fullkey);
t_llll *ears_vamp_enumerate_plugins_detailed(t_object *x, t_symbol *fullkey); // use fullkey=NULL for all plugins
t_llll *ears_vamp_enumerate_libraries(t_object *x);
t_ears_err ears_vamp_run_plugin(t_earsbufobj *e_ob, t_buffer_obj *buf, string soname, string identifier, int outputNo,
                                t_llll **out_features, t_buffer_obj *featuresbuf, e_ears_analysis_temporalmode temporalmode, e_ears_timeunit timeunit, e_ears_analysis_summarization summarizationm, long framesize_samps, long hopsize_samps, t_llll *params);
void ears_vamp_append_features_to_llll(int frame, int sr,
                                       const Plugin::OutputDescriptor &output, int outputNo,
                                       const Plugin::FeatureSet &features, bool useFrames,
                                       t_llll *features_ll, t_llll *timetags_ll, e_ears_analysis_temporalmode temporalmode, e_ears_timeunit timeunit);

t_llll *getFeatures(int, int,
                    const Plugin::OutputDescriptor &, int,
                    const Plugin::FeatureSet &, bool frames);
void transformInput(float *, size_t);
void fft(unsigned int, bool, double *, double *, double *, double *);
void printPluginPath(bool verbose);
void printPluginCategoryList();




#endif // _EARS_BUF_VAMP_COMMONS_H_
