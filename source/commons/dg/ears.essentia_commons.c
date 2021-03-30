#include "ears.essentia_commons.h"

using namespace essentia;
using namespace standard;

std::vector<Real> NOWHERE_vec;
Real NOWHERE_real;
int NOWHERE_int;

bool essentia_has_been_initialized = false;

void *ears_essentia_quit(t_symbol *s, short argc, t_atom *argv)
{
    essentia::shutdown();
    return NULL;
}

void ears_essentia_init()
{
    if (!essentia_has_been_initialized) {
        essentia::init();
        quittask_install((method)ears_essentia_quit, NULL);
        essentia_has_been_initialized = true;
    }
}



std::vector<Real> llll_to_vector_real(t_llll *ll)
{
    std::vector<Real> res;
    for (t_llllelem *el = ll->l_head; el; el = el->l_next)
        res.push_back(hatom_getdouble(&el->l_hatom));
    return res;
}




t_ears_err ears_vector_stft(t_object *ob, std::vector<Real> samples, t_buffer_obj *dest1, t_buffer_obj *dest2, long polar, long fullspectrum, t_ears_essentia_analysis_params *params, e_ears_angleunit angleunit)
{
    t_ears_err err = EARS_ERR_NONE;
    long outframecount_ceil = ceil(1 + ((samples.size() * 1.) / params->hopsize_samps));
    long numbins = fullspectrum ? params->framesize_samps : params->framesize_samps/2 + 1;
    
    ears_buffer_set_size_and_numchannels(ob, dest1, outframecount_ceil, numbins);
    ears_buffer_set_size_and_numchannels(ob, dest2, outframecount_ceil, numbins);
    
    std::vector<essentia::Real> framedata;
    std::vector<essentia::Real> wframedata;
    std::vector<std::complex<essentia::Real>> fftdata;
    
    
    essentia::standard::Algorithm *frameCutter = essentia::standard::AlgorithmFactory::create("FrameCutter",
                                                                                              "frameSize", params->framesize_samps,
                                                                                              "hopSize", (Real)params->hopsize_samps,
                                                                                              "startFromZero", params->startFromZero,
                                                                                              "lastFrameToEndOfFile", params->lastFrameToEndOfFile);
    // windowing algorithm:
    essentia::standard::Algorithm *windower = essentia::standard::AlgorithmFactory::create("Windowing",
                                                                                           "zeroPadding", 0,
                                                                                           "size", params->framesize_samps,
                                                                                           "type", params->windowType);
    
    // fft algorithm:
    essentia::standard::Algorithm *fft = essentia::standard::AlgorithmFactory::create("FFT",
                                                                                      "size", params->framesize_samps);
    
    
    frameCutter->input("signal").set(samples);
    frameCutter->output("frame").set(framedata);
    windower->input("frame").set(framedata);
    windower->output("frame").set(wframedata);
    fft->input("frame").set(wframedata);
    fft->output("fft").set(fftdata);
    
    
    float *dest_sample1 = buffer_locksamples(dest1);
    float *dest_sample2 = buffer_locksamples(dest2);
    
    if (!dest_sample1 || !dest_sample2) {
        err = EARS_ERR_CANT_WRITE;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
    } else {
        
        frameCutter->reset();
        long actual_framecount = 0;
        while (true) {
            frameCutter->compute(); // new frame
            if (!framedata.size()) {
                break;
            }
            
            windower->compute();
            fft->compute();
            
            if (polar) {
                for (long b = 0; b < numbins; b++) {
                    dest_sample1[actual_framecount*numbins + b] = std::abs(fftdata[b]);
                    dest_sample2[actual_framecount*numbins + b] = ears_radians_to_angle(std::arg(fftdata[b]), angleunit);
                }
            } else {
                for (long b = 0; b < numbins; b++) {
                    dest_sample1[actual_framecount*numbins + b] = fftdata[b].real();
                    dest_sample2[actual_framecount*numbins + b] = fftdata[b].imag();
                }
            }
            
            actual_framecount++;
        }
        
        buffer_setdirty(dest1);
        buffer_setdirty(dest2);
        buffer_unlocksamples(dest1);
        buffer_unlocksamples(dest2);
        
    }
    
    
    delete frameCutter;
    delete windower;
    delete fft;
    
    return err;
}







void ears_essentia_extractors_library_build(t_earsbufobj *e_ob, long num_features, long *features, long *temporalmodes, double sr, t_llll **args, t_ears_essentia_extractors_library *lib, t_ears_essentia_analysis_params *params)
{
    // slice audio into frames:
    lib->alg_FrameCutter = AlgorithmFactory::create("FrameCutter",
                                                    "frameSize", params->framesize_samps,
                                                    "hopSize", (Real)params->hopsize_samps,
                                                    "startFromZero", params->startFromZero,
                                                    "lastFrameToEndOfFile", params->lastFrameToEndOfFile);
    // windowing algorithm:
    lib->alg_Windower = AlgorithmFactory::create("Windowing",
                                                 "zeroPadding", 0,
                                                 "size", params->framesize_samps,
                                                 "type", params->windowType);
    
    // fft algorithm:
    lib->alg_Spectrum = AlgorithmFactory::create("Spectrum",
                                                 "size", params->framesize_samps);

    // envelope algorithm:
    lib->alg_Envelope = AlgorithmFactory::create("Envelope",
                                                 "attackTime", ears_samps_to_ms(params->envelope_attack_time_samps, sr),
                                                 "releaseTime", ears_samps_to_ms(params->envelope_release_time_samps, sr),
                                                 "sampleRate", sr);

    // cartesian to polar conversion:
    lib->alg_Car2pol = AlgorithmFactory::create("CartesianToPolar");
    
    
    lib->extractors = (t_ears_essentia_extractor *)bach_newptrclear(num_features * sizeof(t_ears_essentia_extractor));
    for (long i = 0; i < num_features; i++) {
        lib->extractors[i].feature = (e_ears_essentia_feature)features[i];
        lib->extractors[i].temporalmode = (e_ears_essentia_temporalmode)temporalmodes[i];
        lib->extractors[i].essentia_output_timeunit = EARSBUFOBJ_TIMEUNIT_DURATION_RATIO;
        lib->extractors[i].essentia_output_ampunit = EARSBUFOBJ_AMPUNIT_LINEAR;
        lib->extractors[i].output_timeunit = (e_ears_timeunit)e_ob->l_timeunit;
        lib->extractors[i].output_ampunit = (e_ears_ampunit)e_ob->l_ampunit;
        
        for (long d = 0; d < EARS_ESSENTIA_EXTRACTOR_MAX_OUTPUT_LABELS; d++) {
            lib->extractors[i].outputlabels_dummy[d] = NULL;
            lib->extractors[i].output_dummy_type[d] = 'v';
        }

        
        if (args[i]) {
            t_symbol *timeunit_sym = NULL, *ampunit_sym = NULL;
            llll_parseattrs((t_object *)e_ob, args[i], LLLL_PA_DONTWARNFORWRONGKEYS, "ss", gensym("timeunit"), &timeunit_sym, gensym("ampunit"), &ampunit_sym);
            if (timeunit_sym)
                lib->extractors[i].output_timeunit = ears_timeunit_from_symbol(timeunit_sym);
            if (ampunit_sym)
                lib->extractors[i].output_ampunit = ears_ampunit_from_symbol(ampunit_sym);
        }

        switch (features[i]) {
            case EARS_ESSENTIA_FEATURE_ENVELOPE:
                lib->extractors[i].algorithm = AlgorithmFactory::create("UnaryOperator", "type", "identity");
                lib->extractors[i].input_type = EARS_ESSENTIA_EXTRACTOR_INPUT_ENVELOPE;
                lib->extractors[i].output_type = EARS_ESSENTIA_EXTRACTOR_OUTPUT_SIGNAL;
                lib->extractors[i].num_fields = 1; // not important
                lib->extractors[i].inputlabel = "array";
                lib->extractors[i].outputlabel = "array";
                break;
                
            case EARS_ESSENTIA_FEATURE_LOGATTACKTIME:
                lib->extractors[i].algorithm = AlgorithmFactory::create("LogAttackTime",
                                                                        "sampleRate", sr);
                lib->extractors[i].input_type = EARS_ESSENTIA_EXTRACTOR_INPUT_ENVELOPE;
                lib->extractors[i].output_type = EARS_ESSENTIA_EXTRACTOR_OUTPUT_VALUES;
                lib->extractors[i].num_fields = 1;
                lib->extractors[i].inputlabel = "signal";
                lib->extractors[i].outputlabel = "logAttackTime";
                break;

            case EARS_ESSENTIA_FEATURE_MAXTOTOTAL:
                lib->extractors[i].algorithm = AlgorithmFactory::create("MaxToTotal");
                lib->extractors[i].input_type = EARS_ESSENTIA_EXTRACTOR_INPUT_ENVELOPE;
                lib->extractors[i].output_type = EARS_ESSENTIA_EXTRACTOR_OUTPUT_TIMEVALUES;
                lib->extractors[i].num_fields = 1;
                lib->extractors[i].inputlabel = "envelope";
                lib->extractors[i].outputlabel = "maxToTotal";
                lib->extractors[i].essentia_output_timeunit = EARSBUFOBJ_TIMEUNIT_DURATION_RATIO;
                break;

            case EARS_ESSENTIA_FEATURE_MINTOTOTAL:
                lib->extractors[i].algorithm = AlgorithmFactory::create("MinToTotal");
                lib->extractors[i].input_type = EARS_ESSENTIA_EXTRACTOR_INPUT_ENVELOPE;
                lib->extractors[i].output_type = EARS_ESSENTIA_EXTRACTOR_OUTPUT_TIMEVALUES;
                lib->extractors[i].num_fields = 1;
                lib->extractors[i].inputlabel = "envelope";
                lib->extractors[i].outputlabel = "minToTotal";
                break;

            case EARS_ESSENTIA_FEATURE_STRONGDECAY:
                lib->extractors[i].algorithm = AlgorithmFactory::create("StrongDecay",
                                                                        "sampleRate", sr);
                lib->extractors[i].input_type = EARS_ESSENTIA_EXTRACTOR_INPUT_ENVELOPE;
                lib->extractors[i].output_type = EARS_ESSENTIA_EXTRACTOR_OUTPUT_VALUES;
                lib->extractors[i].num_fields = 1;
                lib->extractors[i].inputlabel = "signal";
                lib->extractors[i].outputlabel = "strongDecay";
                break;

                
            case EARS_ESSENTIA_FEATURE_TCTOTOTAL:
                lib->extractors[i].algorithm = AlgorithmFactory::create("TCToTotal");
                lib->extractors[i].input_type = EARS_ESSENTIA_EXTRACTOR_INPUT_ENVELOPE;
                lib->extractors[i].output_type = EARS_ESSENTIA_EXTRACTOR_OUTPUT_TIMEVALUES;
                lib->extractors[i].essentia_output_timeunit = EARSBUFOBJ_TIMEUNIT_DURATION_RATIO;
                lib->extractors[i].num_fields = 1;
                lib->extractors[i].inputlabel = "envelope";
                lib->extractors[i].outputlabel = "TCToTotal";
                break;

                
            case EARS_ESSENTIA_FEATURE_TEMPORALCENTROID:
                lib->extractors[i].algorithm = AlgorithmFactory::create("Centroid");
                lib->extractors[i].input_type = EARS_ESSENTIA_EXTRACTOR_INPUT_ENVELOPE;
                lib->extractors[i].output_type = EARS_ESSENTIA_EXTRACTOR_OUTPUT_TIMEVALUES;
                lib->extractors[i].essentia_output_timeunit = EARSBUFOBJ_TIMEUNIT_SECONDS;
                lib->extractors[i].num_fields = 1;
                lib->extractors[i].inputlabel = "array";
                lib->extractors[i].outputlabel = "centroid";
                break;
                

                
            case EARS_ESSENTIA_FEATURE_DURATION:
                lib->extractors[i].algorithm = AlgorithmFactory::create("Duration",
                                                                        "sampleRate", sr);
                lib->extractors[i].input_type = EARS_ESSENTIA_EXTRACTOR_INPUT_AUDIO;
                lib->extractors[i].output_type = EARS_ESSENTIA_EXTRACTOR_OUTPUT_TIMEVALUES;
                lib->extractors[i].essentia_output_timeunit = EARSBUFOBJ_TIMEUNIT_DURATION_RATIO;
                lib->extractors[i].num_fields = 1;
                lib->extractors[i].inputlabel = "signal";
                lib->extractors[i].outputlabel = "duration";
                break;
                

                
                

                
            case EARS_ESSENTIA_FEATURE_SPECTRALMOMENTS:
                lib->extractors[i].algorithm = AlgorithmFactory::create("RawMoments",
                                                                        "range", sr/2.);
                lib->extractors[i].input_type = EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRUM;
                lib->extractors[i].output_type = EARS_ESSENTIA_EXTRACTOR_OUTPUT_VALUES;
                lib->extractors[i].num_fields = 6;
                lib->extractors[i].inputlabel = "array";
                lib->extractors[i].outputlabel = "rawMoments";
                break;

                
            case EARS_ESSENTIA_FEATURE_SPECTRALCENTROIDTIME:
                lib->extractors[i].algorithm = AlgorithmFactory::create("SpectralCentroidTime",
                                                                   "sampleRate", sr);
                lib->extractors[i].input_type = EARS_ESSENTIA_EXTRACTOR_INPUT_AUDIO;
                lib->extractors[i].output_type = EARS_ESSENTIA_EXTRACTOR_OUTPUT_FREQVALUES;
                lib->extractors[i].num_fields = 1;
                lib->extractors[i].inputlabel = "array";
                lib->extractors[i].outputlabel = "centroid";
                break;
                
                
                
            case EARS_ESSENTIA_FEATURE_SPECTRALFLATNESS:
                lib->extractors[i].algorithm = AlgorithmFactory::create("FlatnessDB");
                lib->extractors[i].input_type = EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRUM;
                lib->extractors[i].output_type = EARS_ESSENTIA_EXTRACTOR_OUTPUT_VALUES;
                lib->extractors[i].num_fields = 1;
                lib->extractors[i].inputlabel = "array";
                lib->extractors[i].outputlabel = "flatnessDB";
                break;

            case EARS_ESSENTIA_FEATURE_FLUX:
                lib->extractors[i].algorithm = AlgorithmFactory::create("Flux");
                lib->extractors[i].input_type = EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRUM;
                lib->extractors[i].output_type = EARS_ESSENTIA_EXTRACTOR_OUTPUT_VALUES;
                lib->extractors[i].num_fields = 1;
                lib->extractors[i].inputlabel = "spectrum";
                lib->extractors[i].outputlabel = "flux";
                break;


                
            case EARS_ESSENTIA_FEATURE_ZEROCROSSINGRATE:
                lib->extractors[i].algorithm = AlgorithmFactory::create("ZeroCrossingRate");
                lib->extractors[i].input_type = EARS_ESSENTIA_EXTRACTOR_INPUT_AUDIO;
                lib->extractors[i].output_type = EARS_ESSENTIA_EXTRACTOR_OUTPUT_VALUES;
                lib->extractors[i].num_fields = 1;
                lib->extractors[i].inputlabel = "signal";
                lib->extractors[i].outputlabel = "zeroCrossingRate";
                break;

            case EARS_ESSENTIA_FEATURE_ENERGYBAND:
            {
                double startCutoffFrequency = 0, stopCutoffFrequency = 100;
                llll_parseattrs((t_object *)e_ob, args[i], LLLL_PA_DONTWARNFORWRONGKEYS, "ff",
                                gensym("startCutoffFrequency"), &startCutoffFrequency,
                                gensym("stopCutoffFrequency"), &stopCutoffFrequency);
                lib->extractors[i].algorithm = AlgorithmFactory::create("EnergyBand",
                                                                        "sampleRate", sr,
                                                                        "startCutoffFrequency", startCutoffFrequency,
                                                                        "stopCutoffFrequency", stopCutoffFrequency);
                lib->extractors[i].input_type = EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRUM;
                lib->extractors[i].output_type = EARS_ESSENTIA_EXTRACTOR_OUTPUT_VALUES;
                lib->extractors[i].num_fields = 1;
                lib->extractors[i].inputlabel = "spectrum";
                lib->extractors[i].outputlabel = "energyBand";
            }
                break;

            case EARS_ESSENTIA_FEATURE_ENERGYBANDRATIO:
            {
                double startCutoffFrequency = 0, stopCutoffFrequency = 100;
                llll_parseattrs((t_object *)e_ob, args[i], LLLL_PA_DONTWARNFORWRONGKEYS, "ff",
                                gensym("startCutoffFrequency"), &startCutoffFrequency,
                                gensym("stopCutoffFrequency"), &stopCutoffFrequency);
                lib->extractors[i].algorithm = AlgorithmFactory::create("EnergyBand",
                                                                        "sampleRate", sr,
                                                                        "startCutoffFrequency", startCutoffFrequency,
                                                                        "stopCutoffFrequency", stopCutoffFrequency);
                lib->extractors[i].input_type = EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRUM;
                lib->extractors[i].output_type = EARS_ESSENTIA_EXTRACTOR_OUTPUT_VALUES;
                lib->extractors[i].num_fields = 1;
                lib->extractors[i].inputlabel = "spectrum";
                lib->extractors[i].outputlabel = "energyBand";
            }
                break;

                
            case EARS_ESSENTIA_FEATURE_MFCC:
            {
                long numberBands = 40, numberCoefficients = 13, liftering = 0, dctType = 2;
                double lowFrequencyBound = 0, highFrequencyBound = 11000;
                llll_parseattrs((t_object *)e_ob, args[i], LLLL_PA_DONTWARNFORWRONGKEYS, "iiiddi",
                                gensym("numberBands"), &numberBands,
                                gensym("numberCoefficients"), &numberCoefficients,
                                gensym("liftering"), &liftering,
                                gensym("lowFrequencyBound"), &lowFrequencyBound,
                                gensym("highFrequencyBound"), &highFrequencyBound,
                                gensym("dctType"), &dctType
                                );
                lib->extractors[i].algorithm = AlgorithmFactory::create("MFCC",
                                                                        "sampleRate", sr,
                                                                        "numberBands", (int)numberBands,
                                                                        "numberCoefficients", (int)numberCoefficients,
                                                                        "liftering", (int)liftering,
                                                                        "inputSize", 1 + params->framesize_samps/2,
                                                                        "lowFrequencyBound", lowFrequencyBound,
                                                                        "highFrequencyBound", highFrequencyBound,
                                                                        "dctType", (int)dctType);
                lib->extractors[i].input_type = EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRUM;
                lib->extractors[i].output_type = EARS_ESSENTIA_EXTRACTOR_OUTPUT_VALUES;
                lib->extractors[i].outputlabels_dummy[0] = "bands";
                lib->extractors[i].output_dummy_type[0] = 'v';
                lib->extractors[i].num_fields = numberCoefficients;
                lib->extractors[i].inputlabel = "spectrum";
                lib->extractors[i].outputlabel = "mfcc";
            }
                break;

                
                
            case EARS_ESSENTIA_FEATURE_BFCC:
            {
                long numberCoefficients = 13;
                llll_parseattrs((t_object *)e_ob, args[i], LLLL_PA_DONTWARNFORWRONGKEYS, "i", gensym("numberCoefficients"), &numberCoefficients);
                lib->extractors[i].algorithm = AlgorithmFactory::create("BFCC",
                                                                        "numberCoefficients", (int)numberCoefficients);
                lib->extractors[i].input_type = EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRUM;
                lib->extractors[i].output_type = EARS_ESSENTIA_EXTRACTOR_OUTPUT_VALUES;
                lib->extractors[i].outputlabels_dummy[0] = "bands";
                lib->extractors[i].output_dummy_type[0] = 'v';
                lib->extractors[i].num_fields = numberCoefficients;
                lib->extractors[i].inputlabel = "spectrum";
                lib->extractors[i].outputlabel = "bfcc";
            }
                break;

                
                
            case EARS_ESSENTIA_FEATURE_BARKBANDS:
            {
                long numberBands = 13;
                llll_parseattrs((t_object *)e_ob, args[i], LLLL_PA_DONTWARNFORWRONGKEYS, "i", gensym("numberBands"), &numberBands);
                lib->extractors[i].algorithm = AlgorithmFactory::create("BarkBands",
                                                                        "sampleRate", sr,
                                                                        "numberBands", (int)numberBands);
                lib->extractors[i].input_type = EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRUM;
                lib->extractors[i].output_type = EARS_ESSENTIA_EXTRACTOR_OUTPUT_VALUES;
                lib->extractors[i].num_fields = numberBands;
                lib->extractors[i].inputlabel = "spectrum";
                lib->extractors[i].outputlabel = "bands";
            }
                break;

                
                
            case EARS_ESSENTIA_FEATURE_ERBBANDS:
            {
                long numberBands = 13;
                llll_parseattrs((t_object *)e_ob, args[i], LLLL_PA_DONTWARNFORWRONGKEYS, "i", gensym("numberBands"), &numberBands);
                lib->extractors[i].algorithm = AlgorithmFactory::create("ERBBands",
                                                                        "sampleRate", sr,
                                                                        "highFrequencyBound", sr/2.,
                                                                        "inputSize", 1 + params->framesize_samps/2,
                                                                        "numberBands", (int)numberBands);
                lib->extractors[i].input_type = EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRUM;
                lib->extractors[i].output_type = EARS_ESSENTIA_EXTRACTOR_OUTPUT_VALUES;
                lib->extractors[i].num_fields = numberBands;
                lib->extractors[i].inputlabel = "spectrum";
                lib->extractors[i].outputlabel = "bands";
            }
                break;
                
                
                
            case EARS_ESSENTIA_FEATURE_FREQUENCYBANDS:
            {
                t_llll *frequencyBands_llll = NULL;
                llll_parseattrs((t_object *)e_ob, args[i], LLLL_PA_DONTWARNFORWRONGKEYS, "l",
                                gensym("frequencyBands"), &frequencyBands_llll);
                if (!frequencyBands_llll)
                    frequencyBands_llll = llll_from_text_buf("0 50 100 150 200 300 400 510 630 770 920 1080 1270 1480 1720 2000 2320 2700 3150 3700 4400 5300 6400 7700 9500 12000 15500 20500 27000");
                std::vector<Real> frequencyBands = llll_to_vector_real(frequencyBands_llll);
                lib->extractors[i].algorithm = AlgorithmFactory::create("FrequencyBands",
                                                                        "sampleRate", sr,
                                                                        "frequencyBands", frequencyBands);
                lib->extractors[i].input_type = EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRUM;
                lib->extractors[i].output_type = EARS_ESSENTIA_EXTRACTOR_OUTPUT_VALUES;
                lib->extractors[i].num_fields = 1;
                lib->extractors[i].inputlabel = "spectrum";
                lib->extractors[i].outputlabel = "bands";
                llll_free(frequencyBands_llll);
            }
                break;
                
            default:
                lib->extractors[i].algorithm = NULL;
                lib->extractors[i].input_type = EARS_ESSENTIA_EXTRACTOR_INPUT_AUDIO;
                lib->extractors[i].output_type = EARS_ESSENTIA_EXTRACTOR_OUTPUT_VALUES;
                lib->extractors[i].num_fields = 0;
                lib->extractors[i].inputlabel = "none";
                lib->extractors[i].outputlabel = "none";
                break;
        }
    }
    
    lib->num_extractors = num_features;
}


void ears_essentia_extractors_configure(t_object *x, long num_extractors, t_ears_essentia_extractor *extractors)
{
//    _centralMoments->configure("range", halfSampleRate);

}

void ears_essentia_extractors_library_free(t_ears_essentia_extractors_library *lib)
{
    delete lib->alg_Car2pol;
    delete lib->alg_Spectrum;
    delete lib->alg_Envelope;
    delete lib->alg_Windower;
    delete lib->alg_FrameCutter;
    for (long i = 0; i < lib->num_extractors; i++) {
        if (lib->extractors[i].result)
            llll_free(lib->extractors[i].result);
        if (lib->extractors[i].algorithm)
            delete lib->extractors[i].algorithm;
    }
    bach_freeptr(lib->extractors);
}

Real vector_average(std::vector<Real> v)
{
    double sum = 0;
    for (long i = 0; i < v.size(); i++)
        sum += v[i];
    return sum/v.size();
}


std::vector<Real> vector_interp_with_hopsize(std::vector<Real> v, Real hopsize)
{
    std::vector<Real> res;
    for (Real o = 0; o < v.size(); o += hopsize) {
        long o_floor = (long)o;
        long o_ceil = o_floor + 1;
        Real t = o - o_floor;
        if (o_ceil < v.size()) { // linear interpolation
            res.push_back((1 - t) * v[o_floor] + t + v[o_ceil]);
        } else
            res.push_back(v[o_floor]);
    }
    return res;
}

std::vector<Real> vector_of_vector_average(std::vector<std::vector<Real>> v)
{
    std::vector<Real> res;
    if (v.size() > 0) {
        long num_features = v[0].size();
        for (long f = 0; f < num_features; f++) {
            Real sum = 0;
            for (long j = 0; j < v.size(); j++) {
                sum += v[j][f];
            }
            res.push_back(sum/v.size());
        }
    }
    return res;
}

t_ears_err ears_essentia_fill_buffer_from_vector(t_object *x, t_buffer_obj *buf, std::vector<std::vector<Real>> values, double hop_size_samps, long dur_samps, long interp_order)
{
    t_ears_err err = EARS_ERR_NONE;
    ears_buffer_set_size_and_numchannels(x, buf, dur_samps, values.size() > 0 ? values[0].size() : 1);
    float *sample = buffer_locksamples(buf);
    
    if (!sample) {
        err = EARS_ERR_CANT_READ;
        object_error((t_object *)x, EARS_ERROR_BUF_CANT_READ);
    } else {

        t_atom_long    channelcount = buffer_getchannelcount(buf);        // number of floats in a frame
        t_atom_long    framecount   = buffer_getframecount(buf);            // number of floats long the buffer is for a single channel
        
        if (framecount > 0) {
            for (long c = 0; c < channelcount; c++) {
                long p = 0;
                double next_hop_position = hop_size_samps;
                if (interp_order == 0) {
                    for (long i = 0; i < framecount; i++) {
                        while (i > next_hop_position) {
                            next_hop_position += hop_size_samps;
                            if (p < values.size()-1)
                                p++;
                        }
                        sample[i*channelcount + c] = values[p][c];
                    }
                } else if (interp_order == 1 || values.size() < 3) { // linear
                    bool lastp = (p == values.size() - 1);
                    for (long i = 0; i < framecount; i++) {
                        while (i > next_hop_position) {
                            next_hop_position += hop_size_samps;
                            if (p < values.size()-1) {
                                p++;
                                lastp = (p == values.size() - 1);
                            }
                        }
                        sample[i*channelcount + c] = (lastp ? values[p][c] : rescale(i, next_hop_position - hop_size_samps, next_hop_position, values[p][c], values[p+1][c]));
                    }
                } else {
                    object_error(x, "Only 0-th and 1-st degree interpolation are implemented.");
                }
            }
            
            buffer_setdirty(buf);
        }
        
        buffer_unlocksamples(buf);
    }
    return err;
}


t_ears_err ears_essentia_fill_buffer_from_vector_of_samples(t_object *x, t_buffer_obj *buf, std::vector<Real> values)
{
    t_ears_err err = EARS_ERR_NONE;
    ears_buffer_set_size_and_numchannels(x, buf, values.size(), 1);
    float *sample = buffer_locksamples(buf);
    
    if (!sample) {
        err = EARS_ERR_CANT_READ;
        object_error((t_object *)x, EARS_ERROR_BUF_CANT_READ);
    } else {
        
        t_atom_long    channelcount = buffer_getchannelcount(buf);        // number of floats in a frame
        t_atom_long    framecount   = buffer_getframecount(buf);            // number of floats long the buffer is for a single channel
        
        if (channelcount == 1 && framecount > 0) {
            for (long i = 0; i < framecount; i++)
                sample[i] = values[i];
            buffer_setdirty(buf);
        }
        
        buffer_unlocksamples(buf);
    }
    return err;
}


double ears_essentia_handle_units(double value, t_buffer_obj *buf, t_ears_essentia_extractor *extr)
{
    if (extr->output_type == EARS_ESSENTIA_EXTRACTOR_OUTPUT_TIMEVALUES) {
        if (extr->essentia_output_timeunit != extr->output_timeunit) {
            return ears_convert_timeunit(value, buf, extr->essentia_output_timeunit, extr->output_timeunit);
        }
    }
    return value;
}

std::vector<Real> ears_essentia_handle_units(std::vector<Real> vec, t_buffer_obj *buf, t_ears_essentia_extractor *extr)
{
    if (extr->output_type == EARS_ESSENTIA_EXTRACTOR_OUTPUT_TIMEVALUES) {
        if (extr->essentia_output_timeunit != extr->output_timeunit) {
            std::vector<Real> outvec;
            for (long i = 0; i < vec.size(); i++)
                outvec.push_back(ears_convert_timeunit(vec[i], buf, extr->essentia_output_timeunit, extr->output_timeunit));
            return outvec;
        }
    }
    return vec;
}

std::vector<std::vector<Real>> ears_essentia_handle_units(std::vector<std::vector<Real>> vec, t_buffer_obj *buf, t_ears_essentia_extractor *extr)
{
    if (extr->output_type == EARS_ESSENTIA_EXTRACTOR_OUTPUT_TIMEVALUES) {
        if (extr->essentia_output_timeunit != extr->output_timeunit) {
            std::vector<std::vector<Real>> outvec;
            for (long i = 0; i < vec.size(); i++) {
                std::vector<Real> inner;
                for (long j = 0; j < vec[i].size(); j++)
                    inner.push_back(ears_convert_timeunit(vec[i][j], buf, extr->essentia_output_timeunit, extr->output_timeunit));
                outvec.push_back(inner);
            }
            return outvec;
        }
    }
    return vec;
}

void ears_essentia_extractor_set_dummy_outputs(t_ears_essentia_extractors_library *lib, long i)
{
    for (long d = 0; d < EARS_ESSENTIA_EXTRACTOR_MAX_OUTPUT_LABELS; d++) {
        if (lib->extractors[i].outputlabels_dummy[d]) {
            if (lib->extractors[i].output_dummy_type[d] == 'f')
                lib->extractors[i].algorithm->output(lib->extractors[i].outputlabels_dummy[d]).set(NOWHERE_real);
            else if (lib->extractors[i].output_dummy_type[d] == 'i')
                lib->extractors[i].algorithm->output(lib->extractors[i].outputlabels_dummy[d]).set(NOWHERE_int);
            else
                lib->extractors[i].algorithm->output(lib->extractors[i].outputlabels_dummy[d]).set(NOWHERE_vec);
        }
    }
}

void ears_essentia_extractors_library_compute(t_object *x, t_buffer_obj *buf, t_ears_essentia_extractors_library *lib, t_ears_essentia_analysis_params *params, long buffer_output_interpolation_order)
{
    std::vector<Real> data = ears_buffer_get_sample_vector_mono((t_object *)x, buf);
    std::vector<Real> framedata;
    std::vector<Real> wframedata;
    std::vector<Real> specdata;
    std::vector<Real> envdata;

    lib->alg_FrameCutter->input("signal").set(data);
    lib->alg_FrameCutter->output("frame").set(framedata);
    lib->alg_Windower->input("frame").set(framedata);
    lib->alg_Windower->output("frame").set(wframedata);
    lib->alg_Spectrum->input("frame").set(wframedata);
    lib->alg_Spectrum->output("spectrum").set(specdata);

    lib->alg_Envelope->input("signal").set(data);
    lib->alg_Envelope->output("signal").set(envdata);

    for (long i = 0; i < lib->num_extractors; i++) {
        t_llll *extr_out = llll_get();
        std::vector<std::vector<Real>> vector_out;
        e_ears_essentia_temporalmode temporalmode = lib->extractors[i].temporalmode;
        e_ears_essentia_extractor_input_type inputtype = lib->extractors[i].input_type;
        e_ears_essentia_extractor_output_type outputtype = lib->extractors[i].output_type;
        
//////////////// ALGORITHM OUTPUT IS A SIGNAL
        if (outputtype == EARS_ESSENTIA_EXTRACTOR_OUTPUT_SIGNAL) {
            
            std::vector<Real> values;
            if (inputtype == EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRUM) {
                // TO DO! WRONG!
                lib->alg_Windower->compute();
                lib->alg_Spectrum->compute();
                lib->extractors[i].algorithm->input(lib->extractors[i].inputlabel).set(specdata);
                lib->extractors[i].algorithm->output(lib->extractors[i].outputlabel).set(values);
            } else if (inputtype == EARS_ESSENTIA_EXTRACTOR_INPUT_ENVELOPE) {
                lib->alg_Envelope->compute();
                lib->extractors[i].algorithm->input(lib->extractors[i].inputlabel).set(envdata);
                lib->extractors[i].algorithm->output(lib->extractors[i].outputlabel).set(values);
            } else {
                lib->extractors[i].algorithm->input(lib->extractors[i].inputlabel).set(data);
                lib->extractors[i].algorithm->output(lib->extractors[i].outputlabel).set(values);
            }
            ears_essentia_extractor_set_dummy_outputs(lib, i);
            lib->extractors[i].algorithm->compute();
            
            switch (temporalmode) {
                case EARS_ESSENTIA_TEMPORALMODE_BUFFER:
                    ears_essentia_fill_buffer_from_vector_of_samples(x, lib->extractors[i].result_buf, ears_essentia_handle_units(values, buf, &lib->extractors[i]));
                    break;
                case EARS_ESSENTIA_TEMPORALMODE_WHOLE:
                    llll_appenddouble(extr_out, ears_essentia_handle_units(vector_average(values), buf, &lib->extractors[i]));
                    break;
                case EARS_ESSENTIA_TEMPORALMODE_TIMESERIES:
                {
                    std::vector<Real>timeseries = vector_interp_with_hopsize(values, params->hopsize_samps);
                    t_llll *extr_in = llll_get();
                    for (long j = 0; j < timeseries.size(); j++)
                        llll_appenddouble(extr_in, ears_essentia_handle_units(timeseries[j], buf, &lib->extractors[i]));
                    llll_appendllll(extr_out, extr_in);
                }
                    break;

                default:
                    break;
            }

/////////////// ALGORITHM OUTPUT IS NOT A SIGNAL
        } else {
            if (temporalmode == EARS_ESSENTIA_TEMPORALMODE_TIMESERIES || temporalmode == EARS_ESSENTIA_TEMPORALMODE_BUFFER) {
                t_llll *extr_in = llll_get();
                lib->alg_FrameCutter->reset();
                while (true) {
                    lib->alg_FrameCutter->compute(); // new frame
                    if (!framedata.size()) {
                        break;
                    }
                    
                    if (lib->extractors[i].num_fields == 1) {
                        Real this_res = 0;
                        if (inputtype == EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRUM) {
                            lib->alg_Windower->compute();
                            lib->alg_Spectrum->compute();
                            lib->extractors[i].algorithm->input(lib->extractors[i].inputlabel).set(specdata);
                            lib->extractors[i].algorithm->output(lib->extractors[i].outputlabel).set(this_res);
                        } else if (inputtype == EARS_ESSENTIA_EXTRACTOR_INPUT_ENVELOPE) {
                            lib->alg_Envelope->compute();
                            lib->extractors[i].algorithm->input(lib->extractors[i].inputlabel).set(envdata);
                            lib->extractors[i].algorithm->output(lib->extractors[i].outputlabel).set(this_res);
                        } else {
                            lib->extractors[i].algorithm->input(lib->extractors[i].inputlabel).set(framedata);
                            lib->extractors[i].algorithm->output(lib->extractors[i].outputlabel).set(this_res);
                        }
                        ears_essentia_extractor_set_dummy_outputs(lib, i);
                        lib->extractors[i].algorithm->compute();
                        if (temporalmode == EARS_ESSENTIA_TEMPORALMODE_TIMESERIES)
                            llll_appenddouble(extr_in, ears_essentia_handle_units(this_res, buf, &lib->extractors[i]));
                        else {
                            std::vector<Real> temp;
                            temp.push_back(this_res);
                            vector_out.push_back(temp);
                        }
                    } else if (lib->extractors[i].num_fields > 1){
                        std::vector<Real> this_res;
                        if (inputtype == EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRUM) {
                            lib->alg_Windower->compute();
                            lib->alg_Spectrum->compute();
                            lib->extractors[i].algorithm->input(lib->extractors[i].inputlabel).set(specdata);
                            lib->extractors[i].algorithm->output(lib->extractors[i].outputlabel).set(this_res);
                        } else if (inputtype == EARS_ESSENTIA_EXTRACTOR_INPUT_ENVELOPE) {
                            lib->alg_Envelope->compute();
                            lib->extractors[i].algorithm->input(lib->extractors[i].inputlabel).set(envdata);
                            lib->extractors[i].algorithm->output(lib->extractors[i].outputlabel).set(this_res);
                        } else {
                            lib->extractors[i].algorithm->input(lib->extractors[i].inputlabel).set(framedata);
                            lib->extractors[i].algorithm->output(lib->extractors[i].outputlabel).set(this_res);
                        }
                        ears_essentia_extractor_set_dummy_outputs(lib, i);
                        lib->extractors[i].algorithm->compute();
                        if (temporalmode == EARS_ESSENTIA_TEMPORALMODE_TIMESERIES) {
                            for (long j = 0; j < this_res.size(); j++)
                                llll_appenddouble(extr_in, ears_essentia_handle_units(this_res[j], buf, &lib->extractors[i]));
                        } else {
                            vector_out.push_back(this_res);
                        }
                    }
                }
                
                llll_appendllll(extr_out, extr_in);
                
                if (temporalmode == EARS_ESSENTIA_TEMPORALMODE_BUFFER) {
                    ears_essentia_fill_buffer_from_vector(x, lib->extractors[i].result_buf, ears_essentia_handle_units(vector_out, buf, &lib->extractors[i]), params->hopsize_samps, params->duration_samps, buffer_output_interpolation_order);
                }
                
                
            } else {
                // static
                
                if (lib->extractors[i].num_fields == 1) {
                    Real this_res;
                    if (inputtype == EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRUM) {
                        // still need to do an average over the windows
                        std::vector<Real> this_res_per_frame;
                        lib->alg_FrameCutter->reset();
                        while (true) {
                            lib->alg_FrameCutter->compute(); // new frame
                            if (!framedata.size()) {
                                break;
                            }
                            
                            Real this_res_single_frame = 0;
                            lib->alg_Windower->compute();
                            lib->alg_Spectrum->compute();
                            lib->extractors[i].algorithm->input(lib->extractors[i].inputlabel).set(specdata);
                            lib->extractors[i].algorithm->output(lib->extractors[i].outputlabel).set(this_res_single_frame);
                            ears_essentia_extractor_set_dummy_outputs(lib, i);
                            lib->extractors[i].algorithm->compute();
                            this_res_per_frame.push_back(this_res_single_frame);
                        }
                        
                        this_res = vector_average(this_res_per_frame);
                    } else if (inputtype == EARS_ESSENTIA_EXTRACTOR_INPUT_ENVELOPE) {
                        lib->alg_Envelope->compute();
                        lib->extractors[i].algorithm->input(lib->extractors[i].inputlabel).set(envdata);
                        lib->extractors[i].algorithm->output(lib->extractors[i].outputlabel).set(this_res);
                        ears_essentia_extractor_set_dummy_outputs(lib, i);
                        lib->extractors[i].algorithm->compute();
                    } else {
                        lib->extractors[i].algorithm->input(lib->extractors[i].inputlabel).set(data);
                        lib->extractors[i].algorithm->output(lib->extractors[i].outputlabel).set(this_res);
                        ears_essentia_extractor_set_dummy_outputs(lib, i);
                        lib->extractors[i].algorithm->compute();
                    }
                    
                    llll_appenddouble(extr_out, ears_essentia_handle_units(this_res, buf, &lib->extractors[i]));
                    
                } else if (lib->extractors[i].num_fields > 1){
                    
                    std::vector<Real> this_res;
                    if (inputtype == EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRUM) {
                        // still need to do an average over the windows
                        std::vector<std::vector<Real>> this_res_per_frame;
                        lib->alg_FrameCutter->reset();
                        while (true) {
                            lib->alg_FrameCutter->compute(); // new frame
                            if (!framedata.size()) {
                                break;
                            }
                            
                            std::vector<Real> this_res_single_frame;
                            lib->alg_Windower->compute();
                            lib->alg_Spectrum->compute();
                            lib->extractors[i].algorithm->input(lib->extractors[i].inputlabel).set(specdata);
                            lib->extractors[i].algorithm->output(lib->extractors[i].outputlabel).set(this_res_single_frame);
                            ears_essentia_extractor_set_dummy_outputs(lib, i);
                            lib->extractors[i].algorithm->compute();
                            this_res_per_frame.push_back(this_res_single_frame);
                        }
                        
                        this_res = vector_of_vector_average(this_res_per_frame);
                    } else if (inputtype == EARS_ESSENTIA_EXTRACTOR_INPUT_ENVELOPE) {
                        lib->extractors[i].algorithm->input(lib->extractors[i].inputlabel).set(data);
                        lib->extractors[i].algorithm->output(lib->extractors[i].outputlabel).set(this_res);
                        ears_essentia_extractor_set_dummy_outputs(lib, i);
                        lib->extractors[i].algorithm->compute();
                    } else {
                        lib->extractors[i].algorithm->input(lib->extractors[i].inputlabel).set(data);
                        lib->extractors[i].algorithm->output(lib->extractors[i].outputlabel).set(this_res);
                        ears_essentia_extractor_set_dummy_outputs(lib, i);
                        lib->extractors[i].algorithm->compute();
                    }
                    for (long j = 0; j < this_res.size(); j++)
                        llll_appenddouble(extr_out, ears_essentia_handle_units(this_res[j], buf, &lib->extractors[i]));
                }
            }
        }
        
        if (lib->extractors[i].result)
            llll_free(lib->extractors[i].result);
        lib->extractors[i].result = extr_out;
    }
}




