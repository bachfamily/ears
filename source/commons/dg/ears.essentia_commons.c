#include "ears.essentia_commons.h"
#include "ears.windows.h"

using namespace essentia;
using namespace standard;

std::vector<std::vector<Real>> NOWHERE_vecvec;
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


t_ears_essentia_analysis_params earsbufobj_get_essentia_analysis_params(t_earsbufobj *e_ob, t_buffer_obj *buf)
{
    t_ears_essentia_analysis_params params;
    params.duration_samps = ears_buffer_get_size_samps((t_object *)e_ob, buf);
    if (atom_gettype(&e_ob->a_numframes) == A_LONG && atom_getlong(&e_ob->a_numframes) >= 2) {
        params.hopsize_samps = (params.duration_samps * 1.) / (atom_getlong(&e_ob->a_numframes) - 1);
        params.framesize_samps = round(params.hopsize_samps * e_ob->a_overlap);
    } else {
        params.framesize_samps = earsbufobj_time_to_samps(e_ob, e_ob->a_winsize, buf, false, true);
        params.hopsize_samps = (Real)earsbufobj_time_to_fsamps((t_earsbufobj *)e_ob, e_ob->a_hopsize, buf, false, true);
    }
    params.framesize_samps = 2 * (params.framesize_samps / 2);     // ensure framesize is even
    if (params.hopsize_samps <= 0) {
        params.hopsize_samps = 1024;
        object_error((t_object *)e_ob, "Negative hop size ignored; a default value of 1024 samples will be used instead.");
    } else if (params.hopsize_samps < 1) {
        object_warn((t_object *)e_ob, "Hop size is smaller than one sample. The number of output frames may differ from what was expected.");
    }
    params.windowType = e_ob->a_wintype->s_name;
    params.lastFrameToEndOfFile = e_ob->a_lastframetoendoffile;
    params.startFromZero = e_ob->a_winstartfromzero;
    return params;
}

std::vector<Real> llll_to_vector_real(t_llll *ll)
{
    std::vector<Real> res;
    for (t_llllelem *el = ll->l_head; el; el = el->l_next)
        res.push_back(hatom_getdouble(&el->l_hatom));
    return res;
}


t_ears_err ears_vector_stft(t_object *ob, std::vector<Real> samples, double sr, t_buffer_obj *dest1, t_buffer_obj *dest2, long polar, long fullspectrum, t_ears_essentia_analysis_params *params, e_ears_angleunit angleunit)
{
    // we approximate the hopsize to the nearest sample. Otherwise the overlap add gives troubles...
    long hopsize_samps = (long)params->hopsize_samps;

    t_ears_err err = EARS_ERR_NONE;
    long outframecount_ceil = ceil(1 + ((samples.size() * 1.) / hopsize_samps));
    long numbins = fullspectrum ? params->framesize_samps : params->framesize_samps/2 + 1;
    
    double new_sr = sr/(hopsize_samps); // sr of the windowed signal
    ears_buffer_set_sr(ob, dest1, new_sr);
    ears_buffer_set_sr(ob, dest2, new_sr);
    ears_buffer_set_size_and_numchannels(ob, dest1, outframecount_ceil, numbins);
    ears_buffer_set_size_and_numchannels(ob, dest2, outframecount_ceil, numbins);

    t_ears_spectralbuf_metadata data;
    
    t_llll *bins = ears_ezarithmser(0, sr/params->framesize_samps, numbins);
    ears_spectralbuf_metadata_fill(&data, sr, sr/params->framesize_samps, 0, EARS_FREQUNIT_HERTZ, gensym("stft"), bins, false); // TODO
    ears_spectralbuf_metadata_set(ob, dest1, &data);
    ears_spectralbuf_metadata_set(ob, dest2, &data);
    llll_free(bins);
    //    ears_buffer_set_hidden_sr(ob, dest1, sr);
//    ears_buffer_set_hidden_sr(ob, dest2, sr);

    std::vector<essentia::Real> framedata;
    std::vector<essentia::Real> wframedata;
    std::vector<std::complex<essentia::Real>> fftdata;
    
    essentia::standard::Algorithm *frameCutter, *windower, *fft;
    try {
        frameCutter = essentia::standard::AlgorithmFactory::create("FrameCutter",
                                                                   "frameSize", params->framesize_samps,
                                                                   "hopSize", (Real)hopsize_samps,
                                                                   "startFromZero", params->startFromZero,
                                                                   "lastFrameToEndOfFile", params->lastFrameToEndOfFile);
        // windowing algorithm:
        windower = essentia::standard::AlgorithmFactory::create("Windowing",
                                                                "zeroPadding", 0,
                                                                "size", params->framesize_samps,
                                                                "type", params->windowType);
        
        // fft algorithm:
        fft = essentia::standard::AlgorithmFactory::create("FFT",
                                                           "size", params->framesize_samps);
        
        
        frameCutter->input("signal").set(samples);
        frameCutter->output("frame").set(framedata);
        windower->input("frame").set(framedata);
        windower->output("frame").set(wframedata);
        fft->input("frame").set(wframedata);
        fft->output("fft").set(fftdata);
    } catch (essentia::EssentiaException e) {  object_error(ob, e.what());  return EARS_ERR_ESSENTIA;   }

    
    float *dest_sample1 = buffer_locksamples(dest1);
    float *dest_sample2 = buffer_locksamples(dest2);
    
    if (!dest_sample1 || !dest_sample2) {
        err = EARS_ERR_CANT_WRITE;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
    } else {
        t_atom_long    channelcount = buffer_getchannelcount(dest1);   // number of floats in a frame
        t_atom_long    framecount   = buffer_getframecount(dest1);     // number of floats long the buffer is for a single channel
        
        if (channelcount != numbins || outframecount_ceil != framecount) {
            object_error(ob, "Cannot write buffer");
        } else {
            
            try {
                frameCutter->reset();
                long actual_framecount = 0;
                while (true) {
                    frameCutter->compute(); // new frame
                    if (!framedata.size()) {
                        break;
                    }
                    
                    if (actual_framecount >= outframecount_ceil) {
                        object_bug(ob, "Error: too few windows allocated.");
                        break;
                    }
                    
                    windower->compute();
                    fft->compute();
                    
                    if (fullspectrum) {
                        long halfspectrum_bins = fftdata.size();
                        for (long i = halfspectrum_bins-2; i >= 1; i--) {
                            fftdata.push_back(std::conj(fftdata[i]));
                        }
                    }
                    
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
            }
            catch (essentia::EssentiaException e)
            {
                object_error(ob, e.what());
            }
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






// framesize_samps is inferred, the one contained in #params is ignored
t_ears_err ears_specbuffer_istft(t_object *ob, long num_input_buffers, t_buffer_obj **source1, t_buffer_obj **source2, t_buffer_obj *dest, long polar, long fullspectrum, t_ears_essentia_analysis_params *params, e_ears_angleunit angleunit, double force_sr)
{
    t_ears_err err = EARS_ERR_NONE;

    if (num_input_buffers == 0) {
        // nothing to do
        ears_buffer_set_size_and_numchannels(ob, dest, 0, 1);
        return err;
    }
    
    // we approximate the hopsize to the nearest sample. Otherwise the overlap add gives troubles...
//    int hopsize_samps = (int)params->hopsize_samps;
//    double sr_original = ears_buffer_get_hidden_sr(ob, source1[0]);
    double sr_original = ears_spectralbuf_get_original_audio_sr(ob, source1[0]);

    int hopsize_samps = ears_ms_to_samps(1000./ears_buffer_get_sr(ob, source1[0]), force_sr ? force_sr : sr_original);
    
    double audio_sr = force_sr ? force_sr : sr_original;

    long numinputframes = ears_buffer_get_size_samps(ob, source1[0]);
    long numinputchannels = ears_buffer_get_numchannels(ob, source1[0]);
    
    for (long b = 1; b < num_input_buffers; b++) {
        if (ears_buffer_get_size_samps(ob, source1[b]) != numinputframes) {
            object_error(ob, "All input buffers must have the same number of samples");
            return EARS_ERR_GENERIC;
        }
        if (ears_buffer_get_numchannels(ob, source1[b]) != numinputchannels) {
            object_error(ob, "All input buffers must have the same number of channels");
            return EARS_ERR_GENERIC;
        }
    }
    
    long numoutputchannels = num_input_buffers;
    int framesize_samps = fullspectrum ? numinputchannels : 2 * (numinputchannels - 1);
    long outframecount = numinputframes * hopsize_samps + (framesize_samps - hopsize_samps);
    
    ears_buffer_set_sr(ob, dest, audio_sr);
    ears_buffer_set_size_and_numchannels(ob, dest, outframecount, numoutputchannels);
    
    std::vector<std::complex<essentia::Real>> framedata;
    std::vector<essentia::Real> ifftframe;
    std::vector<essentia::Real> wifftframe;
    std::vector<essentia::Real> audio;

    essentia::standard::Algorithm *ifft, *windower, *overlapAdd;
    try {
        // ifft algorithm
        ifft = essentia::standard::AlgorithmFactory::create("IFFT",
                                                            "normalize", true,
                                                            "size", framesize_samps);
        // windowing algorithm:
        windower = essentia::standard::AlgorithmFactory::create("Windowing",
                                                                "zeroPadding", 0,
                                                                "size", framesize_samps,
                                                                "type", params->windowType,
                                                                "normalized", false,
                                                                "zeroPhase", false);
        
        overlapAdd = essentia::standard::AlgorithmFactory::create("OverlapAdd",
                                                                  "frameSize", framesize_samps,
                                                                  "hopSize", hopsize_samps);
    } catch (essentia::EssentiaException e) {  object_error(ob, e.what());  return EARS_ERR_ESSENTIA;   }
    
    
    ifft->input("fft").set(framedata);
    ifft->output("frame").set(ifftframe);
    windower->input("frame").set(ifftframe);
    windower->output("frame").set(wifftframe);
    overlapAdd->input("signal").set(wifftframe);
    overlapAdd->output("signal").set(audio);
    
    
    
    float *dest_sample = buffer_locksamples(dest);
    float **source1_sample = (float **)bach_newptr(num_input_buffers * sizeof(float *));
    float **source2_sample = (float **)bach_newptr(num_input_buffers * sizeof(float *));

    bool all_phases_defined = true;
    bool all_mags_defined = true;
    bool startFromZero = params->startFromZero;
    
    for (long b = 0; b < num_input_buffers; b++) {
        source1_sample[b] = buffer_locksamples(source1[b]);
        source2_sample[b] = source2[b] ? buffer_locksamples(source2[b]) : NULL; // no phases?
        if (!source2[b])
            all_phases_defined = false; //TO DO check source1_sample != NULL
        if (!source1[b])
            all_mags_defined = false;
    }
    
    if (!all_phases_defined && params->numGriffinLimIterations <= 0) {
        object_warn(ob, "Some phases are not defined: you may want to set a number of iterations for a Griffin-Lim reconstruction.");
/*        for (long b = 0; b < num_input_buffers; b++) {
            if (source2[b]) {
                object_warn(ob, "Warning: phases are only defined for some channels. The other ones will undergo phase reconstruction.");
                break;
            }
        } */
    }
    
    if (!all_mags_defined) {
        err = EARS_ERR_CANT_READ;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
    } else if (!dest_sample) {
        err = EARS_ERR_CANT_WRITE;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
    } else {
        try {
            for (long c = 0; c < numoutputchannels; c++) {
                if (source2[c] || params->numGriffinLimIterations <= 0) {
                    std::vector<essentia::Real> allaudio;
                    overlapAdd->reset();
                    for (long i = 0; i < numinputframes; i++) {
                        framedata.clear();
                        if (!source2[c]) {
                            for (long b = 0; b < numinputchannels; b++) {
                                framedata.push_back(std::complex<essentia::Real>(source1_sample[c][i*numinputchannels + b], 0.));
                            }
                        } else {
                            if (polar) {
                                for (long b = 0; b < numinputchannels; b++) {
                                    framedata.push_back(std::polar(source1_sample[c][i*numinputchannels + b],  (essentia::Real)ears_angle_to_radians(source2_sample[c][i*numinputchannels + b], angleunit)));
                                }
                            } else {
                                for (long b = 0; b < numinputchannels; b++) {
                                    framedata.push_back(std::complex<essentia::Real>(source1_sample[c][i*numinputchannels + b], source2_sample[c][i*numinputchannels + b]));
                                }
                            }
                        }
                        
                        ifft->compute();
                        windower->compute();
                        overlapAdd->compute();
                        
                        if (!audio.size()) {
                            break;
                        }
                        
                        
                        if ((!startFromZero) && i == 0) {
                            // skip first half window (should we also skip the last half window?)
                            for (long s = framesize_samps/2; s < audio.size(); s++) {
                                allaudio.push_back(audio[s]);
                            }
                        } else {
                            allaudio.insert(allaudio.end(), audio.begin(), audio.end());
                        }
                    }
                    
                    if (allaudio.size() > outframecount) {
                        object_bug(ob, "Error: too few samples allocated.");
                        break;
                    }
                    
                    long i = 0;
                    long limit = MIN(allaudio.size(), outframecount);
                    for (; i < limit; i++)
                        dest_sample[i * numoutputchannels + c] = allaudio[i];
                    for (; i < outframecount; i++) {
                        dest_sample[i * numoutputchannels + c] = 0;
                    }
                } else {
                    // Reconstruct via Griffin-Lim
                    t_buffer_obj *phases = (t_buffer_obj *)object_new_typed(CLASS_BOX, gensym("buffer~"), 0, NULL);
                    t_buffer_obj *amps = (t_buffer_obj *)object_new_typed(CLASS_BOX, gensym("buffer~"), 0, NULL);
                    t_buffer_obj *tempout = (t_buffer_obj *)object_new_typed(CLASS_BOX, gensym("buffer~"), 0, NULL);

                    ears_buffer_copy_format(ob, dest, tempout);
                    ears_buffer_set_size_and_numchannels(ob, tempout, outframecount, 1);
                    
                    // initialize audio
                    float *tempout_sample = buffer_locksamples(tempout);
                    if (tempout_sample) {
                        t_atom_long    channelcount = buffer_getchannelcount(tempout); // must be 1
                        t_atom_long    framecount   = buffer_getframecount(tempout); // must be outframecount
                        
                        for (long j = 0; j < framecount*channelcount; j++)
                            tempout_sample[j] = random_double_in_range(-1, 1);
                        buffer_unlocksamples(tempout);
                    }
                    
                    for (int n = 0; n < params->numGriffinLimIterations; n++) {
                        // reconstruction spectrogram
                        std::vector<Real> samples = ears_buffer_get_sample_vector(ob, tempout, 0);
                        ears_vector_stft(ob, samples, audio_sr, amps, phases, polar, fullspectrum, params, angleunit);

                        // Discard magnitude part of the reconstruction and use the supplied magnitude spectrogram instead
                        ears_specbuffer_istft(ob, 1, &source1[c], &phases, tempout, polar, fullspectrum, params, angleunit, audio_sr);
                    }

                    tempout_sample = buffer_locksamples(tempout);
                    if (!tempout_sample) {
                        err = EARS_ERR_CANT_WRITE;
                        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
                    } else {
                        t_atom_long    channelcount = buffer_getchannelcount(tempout);
                        t_atom_long    framecount   = buffer_getframecount(tempout);
                        
                        long i = 0;
                        long limit = MIN(framecount, outframecount);
                        for (; i < limit; i++) {
                            dest_sample[i * numoutputchannels + c] = tempout_sample[i * channelcount];
                        }
                        for (; i < outframecount; i++) {
                            dest_sample[i * numoutputchannels + c] = 0;
                        }
                        buffer_unlocksamples(tempout);
                    }
                    
                    object_free(amps);
                    object_free(phases);
                    object_free(tempout);
                }
            }
        }
        catch (essentia::EssentiaException e)
        {
            object_error(ob, e.what());
        }

        buffer_setdirty(dest);
        buffer_unlocksamples(dest);
        
    }
    
    for (long b = 0; b < num_input_buffers; b++) {
        buffer_unlocksamples(source1[b]);
        if (source2[b])
            buffer_unlocksamples(source2[b]);
    }
    
    bach_freeptr(source1_sample);
    bach_freeptr(source2_sample);
    delete ifft;
    delete overlapAdd;
    delete windower;
    
    return err;
}







t_ears_err ears_vector_cqt(t_object *ob, std::vector<Real> samples, double sr, t_buffer_obj *dest1, t_buffer_obj *dest2, long polar, t_ears_essentia_analysis_params *params, e_ears_angleunit angleunit)
{
    double Q = params->scale / (pow(2, (1/(double)params->binsPerOctave))-1);
    int framesize_samps = essentia::nextPowerTwo((int)ceil(Q * sr / params->minFrequency));

    // we approximate the hopsize to the nearest sample. Otherwise the overlap add gives troubles...
    long hopsize_samps = (long)params->hopsize_samps;
    
    t_ears_err err = EARS_ERR_NONE;
    long outframecount_ceil = ceil(1 + ((samples.size() * 1.) / hopsize_samps));
    long numbins = params->numberBins;
    
    double new_sr = sr/(hopsize_samps); // sr of the windowed signal
    ears_buffer_set_sr(ob, dest1, new_sr);
    ears_buffer_set_sr(ob, dest2, new_sr);
    ears_buffer_set_size_and_numchannels(ob, dest1, outframecount_ceil, numbins);
    ears_buffer_set_size_and_numchannels(ob, dest2, outframecount_ceil, numbins);
    
    t_ears_spectralbuf_metadata data;
    t_llll *bins = ears_ezarithmser(ears_hz_to_cents(params->minFrequency, EARS_MIDDLE_A_TUNING), 1200./params->binsPerOctave, numbins);
    ears_spectralbuf_metadata_fill(&data, sr, 1200./params->binsPerOctave,
                                   ears_hz_to_cents(params->minFrequency, EARS_MIDDLE_A_TUNING),
                                   EARS_FREQUNIT_CENTS, gensym("cqt"),
                                   bins, false);
    ears_spectralbuf_metadata_set(ob, dest1, &data);
    ears_spectralbuf_metadata_set(ob, dest2, &data);
    llll_free(bins);
//    ears_buffer_set_hidden_sr(ob, dest1, sr);
//    ears_buffer_set_hidden_sr(ob, dest2, sr);

    std::vector<essentia::Real> framedata;
    std::vector<essentia::Real> wframedata;
    std::vector<std::complex<essentia::Real>> cqtdata;
    
    essentia::standard::Algorithm *frameCutter, *windower, *cqt;
    try {
        frameCutter = essentia::standard::AlgorithmFactory::create("FrameCutter",
                                                                                                  "frameSize", framesize_samps,
                                                                                                  "hopSize", (Real)hopsize_samps,
                                                                                                  "startFromZero", params->startFromZero,
                                                                                                  "lastFrameToEndOfFile", params->lastFrameToEndOfFile);

        windower = essentia::standard::AlgorithmFactory::create("Windowing",
                                                                                               "zeroPadding", 0,
                                                                                               "size", framesize_samps,
                                                                                               "type", params->windowType);
        
        cqt = essentia::standard::AlgorithmFactory::create("ConstantQ",
                                                                                          "binsPerOctave", params->binsPerOctave,
                                                                                          "minFrequency", params->minFrequency,
                                                                                          "minimumKernelSize", params->minimumKernelSize,
                                                                                          "numberBins", params->numberBins,
                                                                                          "sampleRate", sr,
                                                                                          "threshold", params->threshold,
                                                                                          "windowType", params->windowType
                                                                                          );
        
        frameCutter->input("signal").set(samples);
        frameCutter->output("frame").set(framedata);
        windower->input("frame").set(framedata);
        windower->output("frame").set(wframedata);
        cqt->input("frame").set(wframedata);
        cqt->output("constantq").set(cqtdata);
    } catch (essentia::EssentiaException e) {  object_error(ob, e.what());  return EARS_ERR_ESSENTIA;   }

    
    float *dest_sample1 = buffer_locksamples(dest1);
    float *dest_sample2 = buffer_locksamples(dest2);
    
    if (!dest_sample1 || !dest_sample2) {
        err = EARS_ERR_CANT_WRITE;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
    } else {
        try {
            frameCutter->reset();
            long actual_framecount = 0;
            while (true) {
                frameCutter->compute(); // new frame
                if (!framedata.size()) {
                    break;
                }
                
                if (actual_framecount >= outframecount_ceil) {
                    object_bug(ob, "Error: too few windows allocated.");
                    break;
                }
                
                windower->compute();
                cqt->compute();
                
                
                if (polar) {
                    for (long b = 0; b < numbins; b++) {
                        dest_sample1[actual_framecount*numbins + b] = std::abs(cqtdata[b]);
                        dest_sample2[actual_framecount*numbins + b] = ears_radians_to_angle(std::arg(cqtdata[b]), angleunit);
                    }
                } else {
                    for (long b = 0; b < numbins; b++) {
                        dest_sample1[actual_framecount*numbins + b] = cqtdata[b].real();
                        dest_sample2[actual_framecount*numbins + b] = cqtdata[b].imag();
                    }
                }
                
                actual_framecount++;
            }
        }
        catch (essentia::EssentiaException e)
        {
            object_error(ob, e.what());
        }
        
        buffer_setdirty(dest1);
        buffer_setdirty(dest2);
        buffer_unlocksamples(dest1);
        buffer_unlocksamples(dest2);
        
    }
    
    
    delete frameCutter;
    delete windower;
    delete cqt;
    
    return err;
}


// peaks of a spectrogram-buffer (channels are bins)
t_llll *ears_specbuffer_peaks(t_object *ob, t_buffer_obj *mags, t_buffer_obj *phases, bool interpolate, int maxPeaks, double minPeakDistance, t_symbol *orderBy, double threshold, e_ears_timeunit timeunit, e_ears_angleunit angleunit, t_ears_err *err)
{
    t_llll *out = llll_get();
    essentia::standard::Algorithm *peaks;
    std::vector<essentia::Real> bins, positions, amplitudes;

    *err = EARS_ERR_NONE;
    
    double spectrogram_sr = ears_buffer_get_sr(ob, mags);
    long spectrogram_numbins = ears_buffer_get_numchannels(ob, mags);
    t_ears_spectralbuf_metadata *data = ears_spectralbuf_metadata_get(ob, mags);

    double minPeakDistance_rel = data ? ((minPeakDistance/data->binsize)/spectrogram_numbins) : minPeakDistance;

    try {
        peaks = essentia::standard::AlgorithmFactory::create("PeakDetection",
                                                             "interpolate", interpolate,
                                                             "maxPeaks", maxPeaks,
                                                             "minPeakDistance", minPeakDistance_rel,
                                                             "orderBy", orderBy->s_name,
                                                             "threshold", (Real)threshold
                                                             );
        
        peaks->input("array").set(bins);
        peaks->output("positions").set(positions);
        peaks->output("amplitudes").set(amplitudes);
    } catch (essentia::EssentiaException e) {  object_error(ob, e.what());  *err = EARS_ERR_ESSENTIA; return out;   }
    
    
    float *mags_sample = buffer_locksamples(mags);
    float *phases_sample = phases ? buffer_locksamples(phases) : NULL;

    if (!mags_sample) {
        *err = EARS_ERR_CANT_READ;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
    } else {
        try {
            t_atom_long    channelcount = buffer_getchannelcount(mags);
            t_atom_long    framecount   = buffer_getframecount(mags);
            for (long f = 0; f < framecount; f++) {
                t_llll *framepeaks = llll_get();
                double t = ears_convert_timeunit(f / spectrogram_sr, mags, EARS_TIMEUNIT_SECONDS, timeunit);
                llll_appenddouble(framepeaks, t);
                long f_times_channelcount = f*channelcount;
                bins.clear();
                for (long c = 0; c < channelcount; c++)
                    bins.push_back(mags_sample[f_times_channelcount + c]);
                
                peaks->compute();
                
                long limit = MIN(positions.size(), amplitudes.size());
                for (long i = 0; i < limit; i++) {
                    t_llll *thispeak = llll_get();
                    double bin = positions[i] * (spectrogram_numbins - 1);
                    
                    // position
                    if (data)
                        llll_appenddouble(thispeak, data->offset + bin * data->binsize);
                    else
                        llll_appenddouble(thispeak, bin+1); // 1-based bin
                    
                    // amplitude
                    llll_appenddouble(thispeak, amplitudes[i]);
                    
                    // phase
                    if (phases_sample) {
                        double ph = 0;
                        if (bin <= 0)
                            ph = phases_sample[f*channelcount];
                        else if (bin >= channelcount - 1)
                            ph = phases_sample[f*channelcount + channelcount - 1];
                        else {
                            // linearly interpolating phases
                            long fl = floor(bin);
                            double diff = bin - fl;
                            double phlow = ears_angle_to_radians(phases_sample[f*channelcount + fl], angleunit);
                            double phhigh = ears_angle_to_radians(phases_sample[f*channelcount + fl+1], angleunit);
                            if (phhigh > phlow) {
                                while (phhigh - phlow > PI) {
                                    phhigh -= TWOPI;
                                }
                            } else if (phlow > phhigh) {
                                while (phlow - phhigh > PI) {
                                    phhigh += TWOPI;
                                }
                            }
                            ph = (1 - diff) * phlow + diff * phhigh;
                        }
                        llll_appenddouble(thispeak, ph);
                    }
                    
                    llll_appendllll(framepeaks, thispeak);
                }
                
                llll_appendllll(out, framepeaks);
            }
        }
        catch (essentia::EssentiaException e)
        {
            object_error(ob, e.what());
        }
        buffer_unlocksamples(mags);
        if (phases)
            buffer_unlocksamples(phases);
    }
    
    return out;
}


void earsbufobj_essentia_convert_timeunit(t_earsbufobj *e_ob, std::vector<Real> &vec, t_buffer_obj *buf, e_ears_timeunit to)
{
    for (long i = 0; i < vec.size(); i++)
        vec[i] = ears_convert_timeunit(vec[i], buf, (e_ears_timeunit)e_ob->l_timeunit, to);
}

void earsbufobj_essentia_convert_timeunit(t_earsbufobj *e_ob, double &val, t_buffer_obj *buf, e_ears_timeunit to)
{
    val = ears_convert_timeunit(val, buf, (e_ears_timeunit)e_ob->l_timeunit, to);
}

void earsbufobj_essentia_convert_antimeunit(t_earsbufobj *e_ob, double &val, t_buffer_obj *buf, e_ears_timeunit to)
{
    val = ears_convert_timeunit(val, buf, (e_ears_timeunit)e_ob->l_antimeunit, to);
}


void earsbufobj_essentia_convert_frequnit(t_earsbufobj *e_ob, std::vector<Real> &vec, t_buffer_obj *buf, e_ears_frequnit to)
{
    for (long i = 0; i < vec.size(); i++)
        vec[i] = ears_convert_frequnit(vec[i], buf, (e_ears_frequnit)e_ob->l_frequnit, to);
}

void earsbufobj_essentia_convert_frequnit(t_earsbufobj *e_ob, double &val, t_buffer_obj *buf, e_ears_frequnit to)
{
    val = ears_convert_frequnit(val, buf, (e_ears_frequnit)e_ob->l_frequnit, to);
}

void earsbufobj_essentia_convert_ampunit(t_earsbufobj *e_ob, std::vector<Real> &vec, t_buffer_obj *buf, e_ears_ampunit to)
{
    for (long i = 0; i < vec.size(); i++)
        vec[i] = ears_convert_ampunit(vec[i], buf, (e_ears_ampunit)e_ob->l_ampunit, to);
}

void earsbufobj_essentia_convert_ampunit(t_earsbufobj *e_ob, double &val, t_buffer_obj *buf, e_ears_ampunit to)
{
    val = ears_convert_ampunit(val, buf, (e_ears_ampunit)e_ob->l_ampunit, to);
}


void earsbufobj_essentia_convert_pitchunit(t_earsbufobj *e_ob, double &val, t_buffer_obj *buf, e_ears_pitchunit to)
{
    val = ears_convert_pitchunit(val, buf, (e_ears_pitchunit)e_ob->l_pitchunit, to);
}

void set_input(t_ears_essentia_extractors_library *lib, long extractor_idx, e_ears_essentia_extractor_input_type type, const char *label)
{
    lib->extractors[extractor_idx].num_inputs = 1;
    lib->extractors[extractor_idx].input_type = type;
    lib->extractors[extractor_idx].essentia_input_label[0] = label;

}


void set_input2(t_ears_essentia_extractors_library *lib, long extractor_idx, e_ears_essentia_extractor_input_type type, const char *label1, const char *label2)
{
    lib->extractors[extractor_idx].num_inputs = 2;
    lib->extractors[extractor_idx].input_type = type;
    lib->extractors[extractor_idx].essentia_input_label[0] = label1;
    lib->extractors[extractor_idx].essentia_input_label[1] = label2;
}

void set_input3(t_ears_essentia_extractors_library *lib, long extractor_idx, e_ears_essentia_extractor_input_type type, const char *label1, const char *label2, const char *label3)
{
    lib->extractors[extractor_idx].num_inputs = 3;
    lib->extractors[extractor_idx].input_type = type;
    lib->extractors[extractor_idx].essentia_input_label[0] = label1;
    lib->extractors[extractor_idx].essentia_input_label[1] = label2;
    lib->extractors[extractor_idx].essentia_input_label[2] = label3;
}


void set_essentia_outputs(t_ears_essentia_extractors_library *lib, long extractor_idx, const char *types, ...)
{
    va_list ap;
    va_start(ap, types);
    int essentia_num_outputs = MIN(strlen(types), EARS_ESSENTIA_EXTRACTOR_MAX_OUTPUTS);
    for (int i = 0; i < essentia_num_outputs; i++) {
        const char *label = va_arg(ap, const char *);
        lib->extractors[extractor_idx].essentia_output_label[i] = label;
        lib->extractors[extractor_idx].output_desc[i] = label;
    }
    va_end(ap);
    lib->extractors[extractor_idx].num_outputs = essentia_num_outputs;
    lib->extractors[extractor_idx].essentia_num_outputs = essentia_num_outputs;
    for (long o = 0; o < essentia_num_outputs; o++)
        lib->extractors[extractor_idx].output_map[o] = 0;
    for (long s = 0; s < essentia_num_outputs; s++) {
        lib->extractors[extractor_idx].essentia_output_type[s] = types[s];
        lib->extractors[extractor_idx].output_type[s] = types[s];
    }
    
    
}

void set_custom_outputs(t_ears_essentia_extractors_library *lib, long extractor_idx, const char *types, ...)
{
    va_list ap;
    va_start(ap, types);
    int num_essentia_outputs = MIN(strlen(types), EARS_ESSENTIA_EXTRACTOR_MAX_OUTPUTS);
    for (int i = 0; i < num_essentia_outputs; i++) {
        const char *label = va_arg(ap, const char *);
        lib->extractors[extractor_idx].output_desc[i] = label;
    }
    va_end(ap);
    lib->extractors[extractor_idx].essentia_num_outputs = num_essentia_outputs;
    lib->extractors[extractor_idx].num_outputs = 0;
    for (long o = 0; o < num_essentia_outputs; o++)
        lib->extractors[extractor_idx].output_map[o] = -1;
    for (long s = 0; s < num_essentia_outputs; s++) {
        lib->extractors[extractor_idx].output_type[s] = types[s];
        if (types[s] != 'x') {
            lib->extractors[extractor_idx].output_map[lib->extractors[extractor_idx].num_outputs] = s;
            lib->extractors[extractor_idx].num_outputs++;
        }
    }
}

void warn_if(t_object *ob, bool condition, const char *warn_msg)
{
    if (condition) {
        object_warn(ob, warn_msg);
    }
}

void set_spec_metadata(t_ears_essentia_extractors_library *lib, long extractor_idx, double binsize, double offset, t_symbol *type, e_ears_frequnit frequnit, t_llll *bins, bool also_free_llll)
{
    lib->extractors[extractor_idx].has_spec_metadata = true;
    lib->extractors[extractor_idx].specdata.binsize = binsize;
    lib->extractors[extractor_idx].specdata.offset = offset;
    lib->extractors[extractor_idx].specdata.type = type;
    lib->extractors[extractor_idx].specdata.frequnit = frequnit;
    if (also_free_llll)
        llll_free(lib->extractors[extractor_idx].specdata.bins);
    lib->extractors[extractor_idx].specdata.bins = bins ? llll_clone(bins) : llll_get();
}

const char *get_threshold_desc(long i)
{
    switch (i) {
        case 0: return "threshold_0";
        case 1: return "threshold_1";
        case 2: return "threshold_2";
        case 3: return "threshold_3";
        case 4: return "threshold_4";
        case 5: return "threshold_5";
        case 6: return "threshold_6";
        case 7: return "threshold_7";
        case 8: return "threshold_8";
        case 9: return "threshold_9";
        case 10: return "threshold_10";
        default: "threshold_0";
    }
}

t_ears_err ears_essentia_extractors_library_build(t_earsbufobj *e_ob, long num_features, long *features, long *temporalmodes, double sr, t_llll **args, t_ears_essentia_extractors_library *lib, t_ears_essentia_analysis_params *params)
{
    t_ears_err err = EARS_ERR_NONE;
    long spectrumsize = 1 + (params->framesize_samps/2);
    std::vector<Real> EMPTY_VECTOR_REAL;
    
    try
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
        
        // fft magnitude algorithm:
        lib->alg_Spectrum = AlgorithmFactory::create("Spectrum",
                                                     "size", params->framesize_samps);
        
        // envelope algorithm:
        lib->alg_Envelope = AlgorithmFactory::create("Envelope",
                                                     "attackTime", ears_samps_to_ms(params->envelope_attack_time_samps, sr),
                                                     "releaseTime", ears_samps_to_ms(params->envelope_release_time_samps, sr),
                                                     "sampleRate", sr);
        
        // cartesian to polar conversion:
        lib->alg_Car2pol = AlgorithmFactory::create("CartesianToPolar");

        
        lib->alg_SpectrumCentralMoments = AlgorithmFactory::create("CentralMoments",
                                                                   "range", sr/2.);

        lib->alg_EnvelopeCentralMoments = AlgorithmFactory::create("CentralMoments",
                                                                   "range", 1.);

        lib->alg_Loudness = AlgorithmFactory::create("Loudness");
        
        lib->alg_EqualLoudness = ((sr == 8000 || sr == 32000 || sr == 44100 || sr == 48000) ? AlgorithmFactory::create("Loudness", "sampleRate", sr) : NULL);


        lib->alg_RMS = AlgorithmFactory::create("RMS");

        lib->alg_SpectralPeaks = AlgorithmFactory::create("SpectralPeaks",
                                                          "sampleRate", sr,
                                                          "magnitudeThreshold", params->PEAKS_magnitudeThreshold,
                                                          "maxFrequency", params->PEAKS_maxFrequency,
                                                          "minFrequency", params->PEAKS_minFrequency,
                                                          "maxPeaks", params->PEAKS_maxPeaks,
                                                          "orderBy", params->PEAKS_orderBy
                                                          );

        // Compute chords
        // TODO review these parameters to improve chords detection. Keeping old code for now
        lib->alg_HPCP = AlgorithmFactory::create("HPCP",
                                   "sampleRate", sr,
                                   "size", params->HPCP_size,
                                   "referenceFrequency", params->HPCP_referenceFrequency,
                                   "harmonics", params->HPCP_harmonics,
                                   "bandPreset", params->HPCP_bandPreset,
                                   "minFrequency", params->HPCP_minFrequency,
                                   "maxFrequency", params->HPCP_maxFrequency,
                                   "maxShifted", params->HPCP_maxShifted,
                                   "normalized", params->HPCP_normalized,
                                   "bandSplitFrequency", params->HPCP_bandSplitFrequency,
                                   "weightType", params->HPCP_weightType,
                                   "nonLinear", params->HPCP_nonLinear,
                                   "windowSize", params->HPCP_windowSize);
        

        // Pitch YINFFT
        lib->alg_YinFFT = AlgorithmFactory::create("PitchYinFFT",
                                                 "frameSize", params->framesize_samps,
                                                 "interpolate", true,
                                                 "maxFrequency", params->YIN_maxFrequency,
                                                 "minFrequency", params->YIN_minFrequency,
                                                 "sampleRate", sr,
                                                 "tolerance", params->YIN_tolerance);
        
        
        
        lib->extractors = (t_ears_essentia_extractor *)bach_newptrclear(num_features * sizeof(t_ears_essentia_extractor));
        for (long i = 0; i < num_features; i++) {
            lib->extractors[i].feature = (e_ears_feature)features[i];
            lib->extractors[i].temporalmode = (e_ears_essentia_temporalmode)temporalmodes[i];
            for (long o = 0; o < EARS_ESSENTIA_EXTRACTOR_MAX_OUTPUTS; o++) {
                lib->extractors[i].essentia_output_timeunit[o] = EARS_TIMEUNIT_UNKNOWN;
                lib->extractors[i].essentia_output_ampunit[o] = EARS_AMPUNIT_UNKNOWN;
                lib->extractors[i].essentia_output_frequnit[o] = EARS_FREQUNIT_UNKNOWN;
                lib->extractors[i].essentia_output_pitchunit[o] = EARS_PITCHUNIT_UNKNOWN;
            }
            lib->extractors[i].output_timeunit = (e_ears_timeunit)e_ob->l_timeunit;
            lib->extractors[i].output_ampunit = (e_ears_ampunit)e_ob->l_ampunit;
            lib->extractors[i].output_frequnit = (e_ears_frequnit)e_ob->l_frequnit;
            lib->extractors[i].output_pitchunit = (e_ears_pitchunit)e_ob->l_pitchunit;
            lib->extractors[i].has_spec_metadata = false;
            lib->extractors[i].specdata.original_audio_signal_sr = 0;
            lib->extractors[i].specdata.binsize = 0;
            lib->extractors[i].specdata.offset = 0;
            lib->extractors[i].specdata.type = gensym("timeseries");
            lib->extractors[i].specdata.frequnit = EARS_FREQUNIT_UNKNOWN;
            lib->extractors[i].essentia_num_outputs = 1;


            
            if (args[i]) {
                t_symbol *timeunit_sym = NULL, *ampunit_sym = NULL;
                llll_parseattrs((t_object *)e_ob, args[i], LLLL_PA_DONTWARNFORWRONGKEYS, "ss", gensym("timeunit"), &timeunit_sym, gensym("ampunit"), &ampunit_sym);
                if (timeunit_sym)
                    lib->extractors[i].output_timeunit = ears_timeunit_from_symbol(timeunit_sym);
                if (ampunit_sym)
                    lib->extractors[i].output_ampunit = ears_ampunit_from_symbol(ampunit_sym);
            }
            
            switch (features[i]) {
                    
                case EARS_FEATURE_SPECTRUM:
                    lib->extractors[i].algorithm = AlgorithmFactory::create("UnaryOperator", "type", "identity");
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRUM, "array");
                    set_essentia_outputs(lib, i, "v", "array");
                    set_custom_outputs(lib, i, "v", "spectrum");
                    break;
                    
//////////////////////////////////////////////////////////////////////////////////////////
///// *****************************  ENVELOPES *****************************************//
//////////////////////////////////////////////////////////////////////////////////////////

                case EARS_FEATURE_ENVELOPE:
                    lib->extractors[i].algorithm = AlgorithmFactory::create("UnaryOperator", "type", "identity");
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_ENVELOPE, "array");
                    set_essentia_outputs(lib, i, "v", "array");
                    set_custom_outputs(lib, i, "v", "envelope");
                    break;
                    
                case EARS_FEATURE_LOGATTACKTIME:
                {
                    double startAttackThreshold = 0.2, stopAttackThreshold = 0.9;
                    llll_parseattrs((t_object *)e_ob, args[i], LLLL_PA_DONTWARNFORWRONGKEYS, "dd",
                                    gensym("startattackthreshold"), &startAttackThreshold,
                                    gensym("stopattackthreshold"), &stopAttackThreshold);
                    lib->extractors[i].algorithm = AlgorithmFactory::create("LogAttackTime",
                                                                            "sampleRate", sr,
                                                                            "startAttackThreshold", startAttackThreshold,
                                                                            "stopAttackThreshold", stopAttackThreshold
                                                                            );
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_ENVELOPE, "signal");
                    set_essentia_outputs(lib, i, "fff", "logAttackTime", "attackStart", "attackStop");
                    set_custom_outputs(lib, i, "fff", "log-10 attack time", "attack start", "attack stop");
                    lib->extractors[i].essentia_output_timeunit[0] = EARS_TIMEUNIT_SECONDS; // Log is handled manually while converting units
                }
                    break;

                case EARS_FEATURE_ENVMAXTIME:
                    lib->extractors[i].algorithm = AlgorithmFactory::create("MaxToTotal");
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_ENVELOPE, "envelope");
                    set_essentia_outputs(lib, i, "f", "maxToTotal");
                    set_custom_outputs(lib, i, "f", "maximum time");
                    lib->extractors[i].essentia_output_timeunit[0] = EARS_TIMEUNIT_DURATION_RATIO;
                    break;
                    
                case EARS_FEATURE_ENVMINTIME:
                    lib->extractors[i].algorithm = AlgorithmFactory::create("MinToTotal");
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_ENVELOPE, "envelope");
                    set_essentia_outputs(lib, i, "f", "minToTotal");
                    set_custom_outputs(lib, i, "f", "minimum time");
                    lib->extractors[i].essentia_output_timeunit[0] = EARS_TIMEUNIT_DURATION_RATIO;
                    break;
                    
                case EARS_FEATURE_STRONGDECAY:
                    lib->extractors[i].algorithm = AlgorithmFactory::create("StrongDecay",
                                                                            "sampleRate", sr);
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_ENVELOPE, "signal");
                    set_essentia_outputs(lib, i, "f", "strongDecay");
                    set_custom_outputs(lib, i, "f", "strong decay");
                    break;

                    
                    
//////////////////////////////////////////////////////////////////////////////////////////
///// *****************************  FILTERS   *****************************************//
//////////////////////////////////////////////////////////////////////////////////////////

                    // TODO ?
                    
//////////////////////////////////////////////////////////////////////////////////////////
///// *****************************  STANDARD  *****************************************//
//////////////////////////////////////////////////////////////////////////////////////////

                    // TODO : AutoCorrelation
                    
                    // TODO : DCT / IDCT

                case EARS_FEATURE_DERIVATIVE:
                    lib->extractors[i].algorithm = AlgorithmFactory::create("Derivative");
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_AUDIO, "signal");
                    set_essentia_outputs(lib, i, "s", "signal");
                    set_custom_outputs(lib, i, "s", "signal");
                    break;

                case EARS_FEATURE_MIN:
                    lib->extractors[i].algorithm = AlgorithmFactory::create("MinMax",
                                                                            "type", "min");
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_AUDIO, "array");
                    set_essentia_outputs(lib, i, "fi", "real", "int");
                    set_custom_outputs(lib, i, "ff", "minimum", "minimum time");
                    lib->extractors[i].essentia_output_timeunit[1] = EARS_TIMEUNIT_SAMPS;
                    break;

                case EARS_FEATURE_MAX:
                    lib->extractors[i].algorithm = AlgorithmFactory::create("MinMax",
                                                                            "type", "max");
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_AUDIO, "array");
                    set_essentia_outputs(lib, i, "fi", "real", "int");
                    set_custom_outputs(lib, i, "ff", "maximum", "maximum time");
                    lib->extractors[i].essentia_output_timeunit[1] = EARS_TIMEUNIT_SAMPS;
                    break;
                    
                    
                    
                // TODO: PeakDetection?

                case EARS_FEATURE_WELCH:
                {
                    double averagingFrames = 10;
                    t_symbol *scaling = gensym("density");
                    llll_parseattrs((t_object *)e_ob, args[i], LLLL_PA_DONTWARNFORWRONGKEYS, "is",
                                    gensym("averagingFrames"), &averagingFrames,
                                    gensym("scaling"), &scaling);
                    lib->extractors[i].algorithm = AlgorithmFactory::create("Welch",
                                                                            "averagingFrames", averagingFrames,
                                                                            "fftSize", params->framesize_samps,
                                                                            "frameSize", params->framesize_samps,
                                                                            "windowType", params->windowType,
                                                                            "scaling", scaling->s_name);
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_FRAME, "frame");
                    set_essentia_outputs(lib, i, "f", "psd");
                    set_custom_outputs(lib, i, "f", "power spectral density");
                    lib->extractors[i].essentia_output_ampunit[0] = EARS_AMPUNIT_DECIBEL;
                }
                    break;

    
                    

                    
                    
                    
                // FlatnessDB removed, Flatness kept

                    
                    
                case EARS_FEATURE_ZEROCROSSINGRATE:
                    lib->extractors[i].algorithm = AlgorithmFactory::create("ZeroCrossingRate");
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_AUDIO, "signal");
                    set_essentia_outputs(lib, i, "f", "zeroCrossingRate");
                    set_custom_outputs(lib, i, "f", "zero-crossing rate");
                    break;

                    
                case EARS_FEATURE_ENERGY:
                    lib->extractors[i].algorithm = AlgorithmFactory::create("Energy");
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRUM, "spectrum");
                    set_essentia_outputs(lib, i, "f", "energy");
                    set_custom_outputs(lib, i, "f", "energy");
                    break;
                    
                    
                case EARS_FEATURE_ENERGYBAND:
                {
                    double startCutoffFrequency = 0, stopCutoffFrequency = 100;
                    llll_parseattrs((t_object *)e_ob, args[i], LLLL_PA_DONTWARNFORWRONGKEYS, "dd",
                                    gensym("startcutofffrequency"), &startCutoffFrequency,
                                    gensym("stopcutofffrequency"), &stopCutoffFrequency);
                    earsbufobj_essentia_convert_frequnit(e_ob, startCutoffFrequency, NULL, EARS_FREQUNIT_HERTZ);
                    earsbufobj_essentia_convert_frequnit(e_ob, stopCutoffFrequency, NULL, EARS_FREQUNIT_HERTZ);
                    lib->extractors[i].algorithm = AlgorithmFactory::create("EnergyBand",
                                                                            "sampleRate", sr,
                                                                            "startCutoffFrequency", startCutoffFrequency,
                                                                            "stopCutoffFrequency", stopCutoffFrequency);
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRUM, "spectrum");
                    set_essentia_outputs(lib, i, "f", "energyBand");
                    set_custom_outputs(lib, i, "f", "energy band");
                }
                    break;
                    
                case EARS_FEATURE_ENERGYBANDRATIO:
                {
                    double startCutoffFrequency = 0, stopCutoffFrequency = 100;
                    llll_parseattrs((t_object *)e_ob, args[i], LLLL_PA_DONTWARNFORWRONGKEYS, "dd",
                                    gensym("startcutofffrequency"), &startCutoffFrequency,
                                    gensym("stopcutofffrequency"), &stopCutoffFrequency);
                    earsbufobj_essentia_convert_frequnit(e_ob, startCutoffFrequency, NULL, EARS_FREQUNIT_HERTZ);
                    earsbufobj_essentia_convert_frequnit(e_ob, stopCutoffFrequency, NULL, EARS_FREQUNIT_HERTZ);
                    lib->extractors[i].algorithm = AlgorithmFactory::create("EnergyBand",
                                                                            "sampleRate", sr,
                                                                            "startCutoffFrequency", startCutoffFrequency,
                                                                            "stopCutoffFrequency", stopCutoffFrequency);
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRUM, "spectrum");
                    set_essentia_outputs(lib, i, "f", "energyBandRatio");
                    set_custom_outputs(lib, i, "f", "energy band ratio");
                }
                    break;
                    
                    
                case EARS_FEATURE_MFCC:
                {
                    long numberBands = 40, numberCoefficients = 13, liftering = 0, dctType = 2;
                    double lowFrequencyBound = 0, highFrequencyBound = 11000;
                    llll_parseattrs((t_object *)e_ob, args[i], LLLL_PA_DONTWARNFORWRONGKEYS, "iiiddi",
                                    gensym("numberbands"), &numberBands,
                                    gensym("numbercoefficients"), &numberCoefficients,
                                    gensym("liftering"), &liftering,
                                    gensym("lowfrequencybound"), &lowFrequencyBound,
                                    gensym("highfrequencybound"), &highFrequencyBound,
                                    gensym("dcttype"), &dctType
                                    );
                    earsbufobj_essentia_convert_frequnit(e_ob, lowFrequencyBound, NULL, EARS_FREQUNIT_HERTZ);
                    earsbufobj_essentia_convert_frequnit(e_ob, highFrequencyBound, NULL, EARS_FREQUNIT_HERTZ);
                    lib->extractors[i].algorithm = AlgorithmFactory::create("MFCC",
                                                                            "sampleRate", sr,
                                                                            "numberBands", (int)numberBands,
                                                                            "numberCoefficients", (int)numberCoefficients,
                                                                            "liftering", (int)liftering,
                                                                            "inputSize", 1 + params->framesize_samps/2,
                                                                            "lowFrequencyBound", lowFrequencyBound,
                                                                            "highFrequencyBound", highFrequencyBound,
                                                                            "dctType", (int)dctType);
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRUM, "spectrum");
                    set_essentia_outputs(lib, i, "vv", "bands", "mfcc");
                    set_custom_outputs(lib, i, "vv", "energies in mel bands", "mel-frequency cepstrum coefficients");
                    set_spec_metadata(lib, i, 0, 0, gensym("mfcc"), EARS_FREQUNIT_UNKNOWN, NULL, true); // TODO
                }
                    break;
                    
                    
                    
                case EARS_FEATURE_BFCC:
                {
                    long dctType = 2, numberCoefficients = 13, liftering = 0, numberBands = 40;
                    t_symbol *logType = gensym("dbamp"), *normalize = gensym("unit_sum"), *type = gensym("power"), *weighting = gensym("warping");
                    double lowFrequencyBound = 0.;
                    llll_parseattrs((t_object *)e_ob, args[i], LLLL_PA_DONTWARNFORWRONGKEYS, "iiiissssd",
                                    gensym("dctType"), &dctType,
                                    gensym("numbercoefficients"), &numberCoefficients,
                                    gensym("liftering"), &liftering,
                                    gensym("numberBands"), &numberBands,
                                    gensym("logType"), &logType,
                                    gensym("normalize"), &normalize,
                                    gensym("type"), &type,
                                    gensym("weighting"), &weighting,
                                    gensym("lowFrequencyBound"), &lowFrequencyBound
                                    );
                    earsbufobj_essentia_convert_frequnit(e_ob, lowFrequencyBound, NULL, EARS_FREQUNIT_HERTZ);
                    lib->extractors[i].algorithm = AlgorithmFactory::create("BFCC",
                                                                            "sampleRate", sr,
                                                                            "numberCoefficients", (int)numberCoefficients,
                                                                            "inputSize", (int)spectrumsize,
                                                                            "liftering", (int)liftering,
                                                                            "logType", logType->s_name,
                                                                            "lowFrequencyBound", lowFrequencyBound,
                                                                            "normalize", normalize->s_name,
                                                                            "numberBands", (int)numberBands,
                                                                            "type", type->s_name,
                                                                            "weighting", weighting->s_name);
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRUM, "spectrum");
                    set_essentia_outputs(lib, i, "vv", "bands", "bfcc");
                    set_custom_outputs(lib, i, "vv", "energies in bark bands", "bark-frequency cepstrum coefficients");
                    set_spec_metadata(lib, i, 0, 0, gensym("bfcc"), EARS_FREQUNIT_UNKNOWN, NULL, true); // TODO
                }
                    break;
                    
                    
                case EARS_FEATURE_GFCC:
                {
                    long dctType = 2, numberCoefficients = 13, liftering = 0, numberBands = 40;
                    t_symbol *logType = gensym("dbamp"), *type = gensym("power");
                    double highFrequencyBound = 22050, lowFrequencyBound = 40., silenceThreshold = 0.0000000001;
                    llll_parseattrs((t_object *)e_ob, args[i], LLLL_PA_DONTWARNFORWRONGKEYS, "iiiisssddd",
                                    gensym("dctType"), &dctType,
                                    gensym("numbercoefficients"), &numberCoefficients,
                                    gensym("liftering"), &liftering,
                                    gensym("numberBands"), &numberBands,
                                    gensym("logType"), &logType,
                                    gensym("type"), &type,
                                    gensym("highFrequencyBound"), &highFrequencyBound,
                                    gensym("lowFrequencyBound"), &lowFrequencyBound,
                                    gensym("silenceThreshold"), &silenceThreshold
                                    );
                    earsbufobj_essentia_convert_frequnit(e_ob, lowFrequencyBound, NULL, EARS_FREQUNIT_HERTZ);
                    earsbufobj_essentia_convert_frequnit(e_ob, highFrequencyBound, NULL, EARS_FREQUNIT_HERTZ);
                    lib->extractors[i].algorithm = AlgorithmFactory::create("BFCC",
                                                                            "dctType", (int)dctType,
                                                                            "highFrequencyBound", highFrequencyBound,
                                                                            "sampleRate", sr,
                                                                            "numberCoefficients", (int)numberCoefficients,
                                                                            "inputSize", (int)spectrumsize,
                                                                            "logType", logType->s_name,
                                                                            "lowFrequencyBound", lowFrequencyBound,
                                                                            "numberBands", (int)numberBands,
                                                                            "type", type->s_name,
                                                                            "silenceThreshold", silenceThreshold);
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRUM, "spectrum");
                    set_essentia_outputs(lib, i, "vv", "bands", "gfcc");
                    set_custom_outputs(lib, i, "vv", "energies in ERB bands", "gammatone-frequency cepstrum coefficients");
                    set_spec_metadata(lib, i, 0, 0, gensym("gfcc"), EARS_FREQUNIT_UNKNOWN, NULL, true); // TODO
                }
                    break;
                    
                    
                case EARS_FEATURE_BARKBANDS:
                {
                    long numberBands = 13;
                    llll_parseattrs((t_object *)e_ob, args[i], LLLL_PA_DONTWARNFORWRONGKEYS, "i", gensym("numberbands"), &numberBands);
                    lib->extractors[i].algorithm = AlgorithmFactory::create("BarkBands",
                                                                            "sampleRate", sr,
                                                                            "numberBands", (int)numberBands);
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRUM, "spectrum");
                    set_essentia_outputs(lib, i, "v", "bands");
                    set_custom_outputs(lib, i, "v", "energies in bark bands");
                    set_spec_metadata(lib, i, 0, 0, gensym("barkbands"), EARS_FREQUNIT_UNKNOWN, NULL, true); // TODO
                }
                    break;
                    
                    
                    
                case EARS_FEATURE_ERBBANDS:
                {
                    long numberBands = 13;
                    llll_parseattrs((t_object *)e_ob, args[i], LLLL_PA_DONTWARNFORWRONGKEYS, "i", gensym("numberbands"), &numberBands);
                    lib->extractors[i].algorithm = AlgorithmFactory::create("ERBBands",
                                                                            "sampleRate", sr,
                                                                            "highFrequencyBound", sr/2.,
                                                                            "inputSize", 1 + params->framesize_samps/2,
                                                                            "numberBands", (int)numberBands);
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRUM, "spectrum");
                    set_essentia_outputs(lib, i, "v", "bands");
                    set_custom_outputs(lib, i, "v", "energies in ERB bands");
                    set_spec_metadata(lib, i, 0, 0, gensym("erbbands"), EARS_FREQUNIT_UNKNOWN, NULL, true); // TODO
                }
                    break;
                    
                    
                    
                case EARS_FEATURE_FREQUENCYBANDS:
                {
                    t_llll *frequencyBands_llll = NULL;
                    llll_parseattrs((t_object *)e_ob, args[i], LLLL_PA_DONTWARNFORWRONGKEYS, "l",
                                    gensym("frequencybands"), &frequencyBands_llll);
                    if (!frequencyBands_llll)
                        frequencyBands_llll = llll_from_text_buf("0 50 100 150 200 300 400 510 630 770 920 1080 1270 1480 1720 2000 2320 2700 3150 3700 4400 5300 6400 7700 9500 12000 15500 20500 27000");
                    std::vector<Real> frequencyBands = llll_to_vector_real(frequencyBands_llll);
                    earsbufobj_essentia_convert_frequnit(e_ob, frequencyBands, NULL, EARS_FREQUNIT_HERTZ);
                    lib->extractors[i].algorithm = AlgorithmFactory::create("FrequencyBands",
                                                                            "sampleRate", sr,
                                                                            "frequencyBands", frequencyBands);
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRUM, "spectrum");
                    set_essentia_outputs(lib, i, "v", "bands");
                    set_custom_outputs(lib, i, "v", "energies in frequency bands");
                    set_spec_metadata(lib, i, 0, 0, gensym("freqbands"), EARS_FREQUNIT_HERTZ, frequencyBands_llll, true);
                    llll_free(frequencyBands_llll);
                }
                    break;

                    
                    // TODO: FlatnessDB
                case EARS_FEATURE_FLUX:
                {
                    long halfRectify = false;
                    t_symbol *norm = gensym("L2");
                    llll_parseattrs((t_object *)e_ob, args[i], LLLL_PA_DONTWARNFORWRONGKEYS, "is",
                                    gensym("halfRectify"), &halfRectify,
                                    gensym("norm"), norm);
                    lib->extractors[i].algorithm = AlgorithmFactory::create("Flux",
                                                                            "halfRectify", (bool)halfRectify,
                                                                            "norm", norm);
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRUM, "spectrum");
                    set_essentia_outputs(lib, i, "f", "flux");
                    set_custom_outputs(lib, i, "f", "flux");
                }
                    break;

                case EARS_FEATURE_HFC:
                {
                    t_symbol *type = gensym("Masri");
                    llll_parseattrs((t_object *)e_ob, args[i], LLLL_PA_DONTWARNFORWRONGKEYS, "s",
                                    gensym("type"), &type);
                    lib->extractors[i].algorithm = AlgorithmFactory::create("HFC",
                                                                            "sampleRate", sr,
                                                                            "type", type->s_name);
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRUM, "spectrum");
                    set_essentia_outputs(lib, i, "f", "hfc");
                    set_custom_outputs(lib, i, "f", "high-frequency coefficient");
                }
                    break;
                    
                case EARS_FEATURE_LPC:
                {
                    long order = 10;
                    t_symbol *type = gensym("regular");
                    llll_parseattrs((t_object *)e_ob, args[i], LLLL_PA_DONTWARNFORWRONGKEYS, "is",
                                    gensym("order"), &order, gensym("type"), &type);
                    lib->extractors[i].algorithm = AlgorithmFactory::create("LPC",
                                                                            "sampleRate", sr,
                                                                            "order", (int)order,
                                                                            "type", type->s_name);
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRUM, "spectrum");
                    set_essentia_outputs(lib, i, "vv", "lpc", "reflection");
                    set_custom_outputs(lib, i, "vv", "LPC coefficients", "reflection coefficients");
                }
                    break;
                
                    // TODO: lpc reflection?

                case EARS_FEATURE_MAXMAGFREQ:
                {
                    lib->extractors[i].algorithm = AlgorithmFactory::create("MaxMagFreq",
                                                                            "sampleRate", sr);
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRUM, "spectrum");
                    set_essentia_outputs(lib, i, "f", "maxMagFreq");
                    set_custom_outputs(lib, i, "f", "frequency with the largest magnitude");
                    lib->extractors[i].essentia_output_frequnit[0] = EARS_FREQUNIT_HERTZ;
                }
                    break;
                    
                    // TODO: log spectrum?
                    
                    // TODO: panning?
                    
                    
                case EARS_FEATURE_POWERSPECTRUM:
                {
                    lib->extractors[i].algorithm = AlgorithmFactory::create("PowerSpectrum",
                                                                            "size", params->framesize_samps);
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRUM, "spectrum");
                    set_essentia_outputs(lib, i, "s", "powerSpectrum");
                    set_custom_outputs(lib, i, "s", "power spectrum");
                }
                    break;

                case EARS_FEATURE_ROLLOFF:
                {
                    double cutoff = 0.85;
                    llll_parseattrs((t_object *)e_ob, args[i], LLLL_PA_DONTWARNFORWRONGKEYS, "d",
                                    gensym("cutoff"), &cutoff);
                    lib->extractors[i].algorithm = AlgorithmFactory::create("RollOff",
                                                                            "sampleRate", sr,
                                                                            "cutoff", cutoff);
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRUM, "spectrum");
                    set_essentia_outputs(lib, i, "f", "rollOff");
                    set_custom_outputs(lib, i, "f", "roll-off frequency");
                    lib->extractors[i].essentia_output_frequnit[0] = EARS_FREQUNIT_HERTZ;
                }
                    break;

                case EARS_FEATURE_TIMEDOMAINSPECTRALCENTROID:
                {
                    lib->extractors[i].algorithm = AlgorithmFactory::create("SpectralCentroidTime",
                                                                            "sampleRate", sr);
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_AUDIO, "array");
                    set_essentia_outputs(lib, i, "f", "centroid");
                    set_custom_outputs(lib, i, "f", "spectral centroid");
                    lib->extractors[i].essentia_output_frequnit[0] = EARS_FREQUNIT_HERTZ;
                }
                    break;

                case EARS_FEATURE_SPECTRALCOMPLEXITY:
                {
                    double magnitudeThreshold = 0.005;
                    llll_parseattrs((t_object *)e_ob, args[i], LLLL_PA_DONTWARNFORWRONGKEYS, "d",
                                    gensym("magnitudethreshold"), &magnitudeThreshold);
                    lib->extractors[i].algorithm = AlgorithmFactory::create("SpectralCentroidTime",
                                                                            "sampleRate", sr,
                                                                            "magnitudeThreshold", (Real)magnitudeThreshold);
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRUM, "spectrum");
                    set_essentia_outputs(lib, i, "f", "spectralComplexity");
                    set_custom_outputs(lib, i, "f", "spectral complexity");
                }
                    break;

                    
                case EARS_FEATURE_SPECTRALCONTRAST:
                {
                    double highFrequencyBound = 11000, lowFrequencyBound = 20, neighbourRatio = 0.4, staticDistribution = 0.15;
                    long numberBands = 6;
                    llll_parseattrs((t_object *)e_ob, args[i], LLLL_PA_DONTWARNFORWRONGKEYS, "dddid",
                                    gensym("highfrequencybound"), &highFrequencyBound,
                                    gensym("lowfrequencybound"), &lowFrequencyBound,
                                    gensym("neighbourratio"), &neighbourRatio,
                                    gensym("numberbands"), &numberBands,
                                    gensym("staticdistribution"), &staticDistribution);
                    earsbufobj_essentia_convert_frequnit(e_ob, highFrequencyBound, NULL, EARS_FREQUNIT_HERTZ);
                    earsbufobj_essentia_convert_frequnit(e_ob, lowFrequencyBound, NULL, EARS_FREQUNIT_HERTZ);
                    lib->extractors[i].algorithm = AlgorithmFactory::create("SpectralCentroidTime",
                                                                            "sampleRate", sr,
                                                                            "frameSize", params->framesize_samps,
                                                                            "highFrequencyBound", (Real)highFrequencyBound,
                                                                            "lowFrequencyBound", (Real)lowFrequencyBound,
                                                                            "neighbourRatio", (Real)neighbourRatio,
                                                                            "numberBands", (int)numberBands,
                                                                            "staticDistribution", (Real)staticDistribution
                                                                            );
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRUM, "spectrum");
                    set_essentia_outputs(lib, i, "vv", "spectralContrast", "spectralValley");
                    set_custom_outputs(lib, i, "vv", "spectral contrast coefficients", "magnitudes of the valleys");
                }
                    break;

                
                    
                case EARS_FEATURE_STRONGPEAK:
                {
                    lib->extractors[i].algorithm = AlgorithmFactory::create("StrongPeak");
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRUM, "spectrum");
                    set_essentia_outputs(lib, i, "f", "strongPeak");
                    set_custom_outputs(lib, i, "f", "strong peak");
                }
                    break;
                    
                    
                    // TODO: SpectralWhitening (INPUT_PEAKS?)
                    
                case EARS_FEATURE_TRIANGULARBANDS:
                {
                    t_llll *frequencyBands_llll = NULL;
                    long log = 1;
                    t_symbol *normalize = gensym("unit_sum"), *type = gensym("power"), *weighting = gensym("linear");
                    llll_parseattrs((t_object *)e_ob, args[i], LLLL_PA_DONTWARNFORWRONGKEYS, "lisss",
                                    gensym("frequencybands"), &frequencyBands_llll,
                                    gensym("log"), &log,
                                    gensym("normalize"), &normalize,
                                    gensym("type"), &type,
                                    gensym("weighting"), &weighting);
                    if (!frequencyBands_llll)
                        frequencyBands_llll = llll_from_text_buf("21.533203125 43.06640625 64.599609375 86.1328125 107.666015625 129.19921875 150.732421875 172.265625 193.798828125 215.33203125 236.865234375 258.3984375 279.931640625 301.46484375 322.998046875 344.53125 366.064453125 387.59765625 409.130859375 430.6640625 452.197265625 473.73046875 495.263671875 516.796875 538.330078125 559.86328125 581.396484375 602.9296875 624.462890625 645.99609375 667.529296875 689.0625 710.595703125 732.12890625 753.662109375 775.1953125 796.728515625 839.794921875 861.328125 882.861328125 904.39453125 925.927734375 968.994140625 990.52734375 1012.06054688 1055.12695312 1076.66015625 1098.19335938 1141.25976562 1184.32617188 1205.859375 1248.92578125 1270.45898438 1313.52539062 1356.59179688 1399.65820312 1442.72460938 1485.79101562 1528.85742188 1571.92382812 1614.99023438 1658.05664062 1701.12304688 1765.72265625 1808.7890625 1873.38867188 1916.45507812 1981.0546875 2024.12109375 2088.72070312 2153.3203125 2217.91992188 2282.51953125 2347.11914062 2411.71875 2497.8515625 2562.45117188 2627.05078125 2713.18359375 2799.31640625 2885.44921875 2950.04882812 3036.18164062 3143.84765625 3229.98046875 3316.11328125 3423.77929688 3509.91210938 3617.578125 3725.24414062 3832.91015625 3940.57617188 4069.77539062 4177.44140625 4306.640625 4435.83984375 4565.0390625 4694.23828125 4844.97070312 4974.16992188 5124.90234375 5275.63476562 5426.3671875 5577.09960938 5749.36523438 5921.63085938 6093.89648438 6266.16210938 6459.9609375 6653.75976562 6847.55859375 7041.35742188 7256.68945312 7450.48828125 7687.35351562 7902.68554688 8139.55078125 8376.41601562 8613.28125 8871.6796875 9130.078125 9388.4765625 9668.40820312 9948.33984375 10249.8046875 10551.2695312 10852.734375 11175.7324219 11498.7304688 11843.2617188 12187.7929688 12553.8574219 12919.921875 13285.9863281 13673.5839844 14082.7148438 14491.8457031 14922.5097656 15353.1738281 15805.3710938 16257.5683594");
                    std::vector<Real> frequencyBands = llll_to_vector_real(frequencyBands_llll);
                    earsbufobj_essentia_convert_frequnit(e_ob, frequencyBands, NULL, EARS_FREQUNIT_HERTZ);
                    lib->extractors[i].algorithm = AlgorithmFactory::create("TriangularBands",
                                                                            "sampleRate", sr,
                                                                            "inputSize", (int)spectrumsize,
                                                                            "log", (bool)log,
                                                                            "frequencyBands", frequencyBands,
                                                                            "normalize", normalize->s_name,
                                                                            "type", type->s_name,
                                                                            "weighting", weighting->s_name
                                                                            );
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRUM, "spectrum");
                    set_essentia_outputs(lib, i, "v", "bands");
                    set_custom_outputs(lib, i, "v", "energy in triangular bands");
                    llll_free(frequencyBands_llll);
                    set_spec_metadata(lib, i, 0, 0, gensym("tribands"), EARS_FREQUNIT_HERTZ, frequencyBands_llll, true);
                }
                    break;
                    
                    
                case EARS_FEATURE_TRIANGULARBARKBANDS:
                {
                    long log = 1, numberBands = 24;
                    double highFrequencyBound = 22050, lowFrequencyBound = 0;
                    t_symbol *normalize = gensym("unit_sum"), *type = gensym("power"), *weighting = gensym("linear");
                    llll_parseattrs((t_object *)e_ob, args[i], LLLL_PA_DONTWARNFORWRONGKEYS, "ddiisss",
                                    gensym("highFrequencyBound"), &highFrequencyBound,
                                    gensym("lowFrequencyBound"), &lowFrequencyBound,
                                    gensym("numberBands"), &numberBands,
                                    gensym("log"), &log,
                                    gensym("normalize"), &normalize,
                                    gensym("type"), &type,
                                    gensym("weighting"), &weighting);
                    earsbufobj_essentia_convert_frequnit(e_ob, highFrequencyBound, NULL, EARS_FREQUNIT_HERTZ);
                    earsbufobj_essentia_convert_frequnit(e_ob, lowFrequencyBound, NULL, EARS_FREQUNIT_HERTZ);
                    lib->extractors[i].algorithm = AlgorithmFactory::create("TriangularBands",
                                                                            "sampleRate", sr,
                                                                            "inputSize", (int)spectrumsize,
                                                                            "highFrequencyBound", highFrequencyBound,
                                                                            "lowFrequencyBound", lowFrequencyBound,
                                                                            "log", (bool)log,
                                                                            "numberBands", (int)numberBands,
                                                                            "normalize", normalize->s_name,
                                                                            "type", type->s_name,
                                                                            "weighting", weighting->s_name
                                                                            );
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRUM, "spectrum");
                    set_essentia_outputs(lib, i, "v", "bands");
                    set_custom_outputs(lib, i, "v", "energy in triangular bark bands");
                    set_spec_metadata(lib, i, 0, 0, gensym("tribarkbands"), EARS_FREQUNIT_HERTZ, NULL, true); //TODO
                }
                    break;
                    
                    
//////////////////////////////////////////////////////////////////////////////////////////
///// *****************************   RHYTHM   *****************************************//
//////////////////////////////////////////////////////////////////////////////////////////

                case EARS_FEATURE_BEATTRACKERDEGARA:
                {
                    long maxTempo = 40, minTempo = 208;
                    llll_parseattrs((t_object *)e_ob, args[i], LLLL_PA_DONTWARNFORWRONGKEYS, "ii",
                                    gensym("maxTempo"), &maxTempo,
                                    gensym("minTempo"), &minTempo);
                    lib->extractors[i].algorithm = AlgorithmFactory::create("BeatTrackerDegara",
                                                                            "maxTempo", (int)maxTempo,
                                                                            "minTempo", (int)minTempo);
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_AUDIO, "signal");
                    set_essentia_outputs(lib, i, "v", "ticks");
                    set_custom_outputs(lib, i, "v", "estimated beat locations");
                }
                    break;

                case EARS_FEATURE_BEATTRACKERMULTIFEATURE:
                {
                    long maxTempo = 40, minTempo = 208;
                    llll_parseattrs((t_object *)e_ob, args[i], LLLL_PA_DONTWARNFORWRONGKEYS, "ii",
                                    gensym("maxTempo"), &maxTempo,
                                    gensym("minTempo"), &minTempo);
                    lib->extractors[i].algorithm = AlgorithmFactory::create("BeatTrackerDegara",
                                                                            "maxTempo", (int)maxTempo,
                                                                            "minTempo", (int)minTempo);
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_AUDIO, "signal");
                    set_essentia_outputs(lib, i, "vv", "ticks", "confidence");
                    set_custom_outputs(lib, i, "vv", "estimated beat locations", "confidence");
                }
                    break;
                    
                //TODO BpmHistogram
                    
                    
                    
                case EARS_FEATURE_BEATSLOUDNESS:
                {
                    t_llll *beats_llll = NULL, *frequencyBands_llll = NULL;
                    double beatDuration = 50, beatWindowDuration = 100;
                    t_symbol *normalize = gensym("unit_sum"), *type = gensym("power"), *weighting = gensym("linear");
                    llll_parseattrs((t_object *)e_ob, args[i], LLLL_PA_DONTWARNFORWRONGKEYS, "lisss",
                                    gensym("beats"), &beats_llll,
                                    gensym("frequencyBands"), &frequencyBands_llll,
                                    gensym("normalize"), &normalize,
                                    gensym("type"), &type,
                                    gensym("weighting"), &weighting);
                    if (!beats_llll)
                        beats_llll = llll_get();
                    if (!frequencyBands_llll)
                        frequencyBands_llll = llll_from_text_buf("20 150 400 3200 7000 22000");
                    std::vector<Real> frequencyBands = llll_to_vector_real(frequencyBands_llll);
                    std::vector<Real> beats = llll_to_vector_real(beats_llll);
                    earsbufobj_essentia_convert_frequnit(e_ob, frequencyBands, NULL, EARS_FREQUNIT_HERTZ);
                    earsbufobj_essentia_convert_timeunit(e_ob, beats, NULL, EARS_TIMEUNIT_SECONDS);
                    earsbufobj_essentia_convert_timeunit(e_ob, beatDuration, NULL, EARS_TIMEUNIT_SECONDS);
                    earsbufobj_essentia_convert_timeunit(e_ob, beatWindowDuration, NULL, EARS_TIMEUNIT_SECONDS);
                    lib->extractors[i].algorithm = AlgorithmFactory::create("TriangularBands",
                                                                            "sampleRate", sr,
                                                                            "beatDuration", beatDuration,
                                                                            "beatWindowDuration", beatWindowDuration,
                                                                            "beats", beats,
                                                                            "frequencyBands", frequencyBands
                                                                            );
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRUM, "spectrum");
                    set_essentia_outputs(lib, i, "vw", "loudness", "loudnessBandRatio");
                    set_custom_outputs(lib, i, "vw", "beat loudness", "beat loudness band ratio");
                    llll_free(frequencyBands_llll);
                    llll_free(beats_llll);
                }
                    break;
                    
                    // TODO: BpmRubato
                    
                case EARS_FEATURE_DANCEABILITY:
                {
                    double maxTau = 8800, minTau = 310, tauMultiplier = 1.1;
                    llll_parseattrs((t_object *)e_ob, args[i], LLLL_PA_DONTWARNFORWRONGKEYS, "ddd",
                                    gensym("maxtau"), &maxTau,
                                    gensym("mintau"), &minTau,
                                    gensym("taumultiplier"), &tauMultiplier);
                    earsbufobj_essentia_convert_timeunit(e_ob, maxTau, NULL, EARS_TIMEUNIT_MS);
                    earsbufobj_essentia_convert_timeunit(e_ob, minTau, NULL, EARS_TIMEUNIT_MS);
                    lib->extractors[i].algorithm = AlgorithmFactory::create("Danceability",
                                                                            "sampleRate", sr,
                                                                            "maxTau", maxTau,
                                                                            "minTau", minTau,
                                                                            "tauMultiplier", tauMultiplier
                                                                            );
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_AUDIO, "signal");
                    set_essentia_outputs(lib, i, "f", "danceability");
                    set_custom_outputs(lib, i, "f", "danceability");
                }
                    break;
                    
                    
                    // TODO: HarmonicBpm
                    // TODO: LoopBpmConfidence
                    
                case EARS_FEATURE_LOOPBPMESTIMATOR: // PROBLEMATIC
                {
                    double confidenceThreshold = 0.95;
                    llll_parseattrs((t_object *)e_ob, args[i], LLLL_PA_DONTWARNFORWRONGKEYS, "d",
                                    gensym("confidenceThreshold"), &confidenceThreshold);
                    lib->extractors[i].algorithm = AlgorithmFactory::create("LoopBpmEstimator",
                                                                            "confidenceThreshold", confidenceThreshold
                                                                            );
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_AUDIO, "signal");
                    set_essentia_outputs(lib, i, "f", "bpm");
                    set_custom_outputs(lib, i, "f", "estimated BPM");
                }
                    break;
                    
                    // TODO: Meter, NoveltyCurve, NoveltyCurveFixedBpmEstimator
                    
                    
                case EARS_FEATURE_ONSETDETECTION:
                {
                    t_symbol *method = gensym("hfc");
                    llll_parseattrs((t_object *)e_ob, args[i], LLLL_PA_DONTWARNFORWRONGKEYS, "s",
                                    gensym("method"), &method);
                    lib->extractors[i].algorithm = AlgorithmFactory::create("LoopBpmEstimator",
                                                                            "method", method->s_name,
                                                                            "sampleRate", sr
                                                                            );
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRUM, "spectrum");
                    set_essentia_outputs(lib, i, "f", "onsetDetection");
                    set_custom_outputs(lib, i, "f", "onset probability");
                    // TODO: phase input
                }
                    break;
                    
                    // TODO: all the rest of rhythm section
                    
                    
//////////////////////////////////////////////////////////////////////////////////////////
///// ***************************** STATISTICS *****************************************//
//////////////////////////////////////////////////////////////////////////////////////////

                case EARS_FEATURE_TEMPORALCENTRALMOMENTS:
                    lib->extractors[i].algorithm = AlgorithmFactory::create("UnaryOperator", "type", "identity");
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_ENVELOPECENTRALMOMENTS, "array");
                    set_essentia_outputs(lib, i, "v", "array");
                    set_custom_outputs(lib, i, "v", "temporal central moments");
                    // To do: time unit????
                    break;
                    
                case EARS_FEATURE_SPECTRALCENTRALMOMENTS:
                    lib->extractors[i].algorithm = AlgorithmFactory::create("UnaryOperator", "type", "identity");
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRUMCENTRALMOMENTS, "array");
                    set_essentia_outputs(lib, i, "v", "array");
                    set_custom_outputs(lib, i, "v", "spectral central moments");
                    // To do: freq unit????
                    break;
                
                    
                case EARS_FEATURE_TEMPORALRAWMOMENTS:
                    lib->extractors[i].algorithm = AlgorithmFactory::create("RawMoments");
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRUM, "array");
                    set_essentia_outputs(lib, i, "v", "rawMoments");
                    set_custom_outputs(lib, i, "v", "temporal raw moments");
                    // To do: time unit????
                    break;
                    
                case EARS_FEATURE_SPECTRALRAWMOMENTS:
                    lib->extractors[i].algorithm = AlgorithmFactory::create("RawMoments",
                                                                            "range", sr/2.);
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRUM, "array");
                    set_essentia_outputs(lib, i, "v", "rawMoments");
                    set_custom_outputs(lib, i, "v", "spectral raw moments");
                    // To do: freq unit????
                    break;
                    
                    
                case EARS_FEATURE_TEMPORALCENTROID:
                    lib->extractors[i].algorithm = AlgorithmFactory::create("Centroid");
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_ENVELOPE, "array");
                    set_essentia_outputs(lib, i, "f", "centroid");
                    set_custom_outputs(lib, i, "f", "temporal centroid");
                    lib->extractors[i].essentia_output_timeunit[0] = EARS_TIMEUNIT_DURATION_RATIO;
                    break;
                    
                    
                case EARS_FEATURE_SPECTRALCENTROID:
                    lib->extractors[i].algorithm = AlgorithmFactory::create("Centroid",
                                                                            "range", sr/2);
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRUM, "array");
                    set_essentia_outputs(lib, i, "f", "centroid");
                    set_custom_outputs(lib, i, "f", "spectral centroid");
                    lib->extractors[i].essentia_output_frequnit[0] = EARS_FREQUNIT_HERTZ;
                    break;

                case EARS_FEATURE_TEMPORALCREST:
                    lib->extractors[i].algorithm = AlgorithmFactory::create("Crest");
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_ENVELOPE, "array");
                    set_essentia_outputs(lib, i, "f", "crest");
                    set_custom_outputs(lib, i, "f", "temporal crest");
                    break;
                    
                case EARS_FEATURE_SPECTRALCREST:
                    lib->extractors[i].algorithm = AlgorithmFactory::create("Crest");
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRUM, "array");
                    set_essentia_outputs(lib, i, "f", "crest");
                    set_custom_outputs(lib, i, "f", "spectral crest");
                    break;

                case EARS_FEATURE_TEMPORALDECREASE:
                    lib->extractors[i].algorithm = AlgorithmFactory::create("Decrease");
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_ENVELOPE, "array");
                    set_essentia_outputs(lib, i, "f", "decrease");
                    set_custom_outputs(lib, i, "f", "temporal decrease");
                    break;
                    
                case EARS_FEATURE_SPECTRALDECREASE:
                    lib->extractors[i].algorithm = AlgorithmFactory::create("Decrease",
                                                                            "range", sr/2);
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRUM, "array");
                    set_essentia_outputs(lib, i, "f", "decrease");
                    set_custom_outputs(lib, i, "f", "spectral decrease");
                    break;

                case EARS_FEATURE_TEMPORALDISTRIBUTIONSHAPE:
                    lib->extractors[i].algorithm = AlgorithmFactory::create("DistributionShape");
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_ENVELOPECENTRALMOMENTS, "centralMoments");
                    set_essentia_outputs(lib, i, "fff", "spread", "skewness", "kurtosis");
                    set_custom_outputs(lib, i, "fff", "temporal spread", "temporal skewness", "temporal kurtosis");
                    lib->extractors[i].essentia_output_timeunit[0] = EARS_TIMEUNIT_DURATION_RATIO;
                    lib->extractors[i].essentia_output_timeunit[1] = EARS_TIMEUNIT_DURATION_RATIO;
                    lib->extractors[i].essentia_output_timeunit[2] = EARS_TIMEUNIT_DURATION_RATIO;
                    // TODO check units
                    break;

                case EARS_FEATURE_SPECTRALDISTRIBUTIONSHAPE:
                    lib->extractors[i].algorithm = AlgorithmFactory::create("DistributionShape");
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRUMCENTRALMOMENTS, "centralMoments");
                    set_essentia_outputs(lib, i, "fff", "spread", "skewness", "kurtosis");
                    set_custom_outputs(lib, i, "fff", "spectral spread", "spectral skewness", "spectral kurtosis");
                    break;

                case EARS_FEATURE_TEMPORALSPREAD:
                    lib->extractors[i].algorithm = AlgorithmFactory::create("DistributionShape");
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_ENVELOPECENTRALMOMENTS, "centralMoments");
                    set_essentia_outputs(lib, i, "fff", "spread", "skewness", "kurtosis");
                    set_custom_outputs(lib, i, "fxx", "temporal spread", "temporal skewness", "temporal kurtosis");
                    lib->extractors[i].essentia_output_timeunit[0] = EARS_TIMEUNIT_DURATION_RATIO;
                    lib->extractors[i].essentia_output_timeunit[1] = EARS_TIMEUNIT_DURATION_RATIO;
                    lib->extractors[i].essentia_output_timeunit[2] = EARS_TIMEUNIT_DURATION_RATIO;
                    break;
                    
                case EARS_FEATURE_SPECTRALSPREAD:
                    lib->extractors[i].algorithm = AlgorithmFactory::create("DistributionShape");
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRUMCENTRALMOMENTS, "centralMoments");
                    set_essentia_outputs(lib, i, "fff", "spread", "skewness", "kurtosis");
                    set_custom_outputs(lib, i, "fxx", "spectral spread", "spectral skewness", "spectral kurtosis");
                    break;

                case EARS_FEATURE_TEMPORALSKEWNESS:
                    lib->extractors[i].algorithm = AlgorithmFactory::create("DistributionShape");
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_ENVELOPECENTRALMOMENTS, "centralMoments");
                    set_essentia_outputs(lib, i, "fff", "spread", "skewness", "kurtosis");
                    set_custom_outputs(lib, i, "xfx", "temporal spread", "temporal skewness", "temporal kurtosis");
                    lib->extractors[i].essentia_output_timeunit[0] = EARS_TIMEUNIT_DURATION_RATIO;
                    lib->extractors[i].essentia_output_timeunit[1] = EARS_TIMEUNIT_DURATION_RATIO;
                    lib->extractors[i].essentia_output_timeunit[2] = EARS_TIMEUNIT_DURATION_RATIO;
                    break;
                    
                case EARS_FEATURE_SPECTRALSKEWNESS:
                    lib->extractors[i].algorithm = AlgorithmFactory::create("DistributionShape");
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRUMCENTRALMOMENTS, "centralMoments");
                    set_essentia_outputs(lib, i, "fff", "spread", "skewness", "kurtosis");
                    set_custom_outputs(lib, i, "xfx", "spectral spread", "spectral skewness", "spectral kurtosis");
                    break;

                case EARS_FEATURE_TEMPORALKURTOSIS:
                    lib->extractors[i].algorithm = AlgorithmFactory::create("DistributionShape");
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_ENVELOPECENTRALMOMENTS, "centralMoments");
                    set_essentia_outputs(lib, i, "fff", "spread", "skewness", "kurtosis");
                    set_custom_outputs(lib, i, "xxf", "temporal spread", "temporal skewness", "temporal kurtosis");
                    lib->extractors[i].essentia_output_timeunit[0] = EARS_TIMEUNIT_DURATION_RATIO;
                    lib->extractors[i].essentia_output_timeunit[1] = EARS_TIMEUNIT_DURATION_RATIO;
                    lib->extractors[i].essentia_output_timeunit[2] = EARS_TIMEUNIT_DURATION_RATIO;
                    break;
                    
                case EARS_FEATURE_SPECTRALKURTOSIS:
                    lib->extractors[i].algorithm = AlgorithmFactory::create("DistributionShape");
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRUMCENTRALMOMENTS, "centralMoments");
                    set_essentia_outputs(lib, i, "fff", "spread", "skewness", "kurtosis");
                    set_custom_outputs(lib, i, "xxf", "spectral spread", "spectral skewness", "spectral kurtosis");
                    break;

                case EARS_FEATURE_SPECTRALENERGY:
                    lib->extractors[i].algorithm = AlgorithmFactory::create("Energy");
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRUM, "array");
                    set_essentia_outputs(lib, i, "f", "energy");
                    set_custom_outputs(lib, i, "f", "spectral energy");
                    break;

                case EARS_FEATURE_SPECTRALENTROPY:
                    lib->extractors[i].algorithm = AlgorithmFactory::create("Entropy");
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRUM, "array");
                    set_essentia_outputs(lib, i, "f", "entropy");
                    set_custom_outputs(lib, i, "f", "spectral entropy");
                    break;

                case EARS_FEATURE_TEMPORALFLATNESS:
                    lib->extractors[i].algorithm = AlgorithmFactory::create("Flatness");
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_AUDIO, "array");
                    set_essentia_outputs(lib, i, "f", "flatness");
                    set_custom_outputs(lib, i, "f", "temporal flatness");
                    break;
                    
                case EARS_FEATURE_SPECTRALFLATNESS:
                    lib->extractors[i].algorithm = AlgorithmFactory::create("Flatness");
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRUM, "array");
                    set_essentia_outputs(lib, i, "f", "flatness");
                    set_custom_outputs(lib, i, "f", "spectral flatness");
                    break;
                    
                case EARS_FEATURE_SPECTRALGEOMETRICMEAN:
                    lib->extractors[i].algorithm = AlgorithmFactory::create("GeometricMean");
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRUM, "array");
                    set_essentia_outputs(lib, i, "f", "geometricMean");
                    set_custom_outputs(lib, i, "f", "geometric mean");
                    break;

                case EARS_FEATURE_INSTANTPOWER:
                    lib->extractors[i].algorithm = AlgorithmFactory::create("InstantPower");
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_AUDIO, "array");
                    set_essentia_outputs(lib, i, "f", "power");
                    set_custom_outputs(lib, i, "f", "power");
                    break;

                case EARS_FEATURE_SPECTRALMEAN:
                    lib->extractors[i].algorithm = AlgorithmFactory::create("Mean");
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRUM, "array");
                    set_essentia_outputs(lib, i, "f", "mean");
                    set_custom_outputs(lib, i, "f", "mean");
                    break;
                    
                case EARS_FEATURE_SPECTRALMEDIAN:
                    lib->extractors[i].algorithm = AlgorithmFactory::create("Median");
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRUM, "array");
                    set_essentia_outputs(lib, i, "f", "median");
                    set_custom_outputs(lib, i, "f", "median");
                    break;

                case EARS_FEATURE_SPECTRALRMS:
                    lib->extractors[i].algorithm = AlgorithmFactory::create("RMS");
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRUM, "array");
                    set_essentia_outputs(lib, i, "f", "rms");
                    set_custom_outputs(lib, i, "f", "rms");
                    lib->extractors[i].essentia_output_ampunit[0] = EARS_AMPUNIT_LINEAR;
                    break;

                case EARS_FEATURE_TEMPORALVARIANCE:
                    lib->extractors[i].algorithm = AlgorithmFactory::create("Variance");
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_ENVELOPE, "array");
                    set_essentia_outputs(lib, i, "f", "variance");
                    set_custom_outputs(lib, i, "f", "variance");
                    // TODO: UNITS
                    break;

                case EARS_FEATURE_SPECTRALVARIANCE:
                    lib->extractors[i].algorithm = AlgorithmFactory::create("Variance");
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRUM, "array");
                    set_essentia_outputs(lib, i, "f", "variance");
                    set_custom_outputs(lib, i, "f", "variance");
                    // TODO: UNITS
                    break;

                    
                    
//////////////////////////////////////////////////////////////////////////////////////////
///// *****************************    TONAL   *****************************************//
//////////////////////////////////////////////////////////////////////////////////////////
                
                case EARS_FEATURE_CHORDSDETECTION:
                {
                    double windowSize = 2000;
                    llll_parseattrs((t_object *)e_ob, args[i], LLLL_PA_DONTWARNFORWRONGKEYS, "s",
                                    gensym("windowsize"), &windowSize);
                    earsbufobj_essentia_convert_timeunit(e_ob, windowSize, NULL, EARS_TIMEUNIT_SECONDS);
                    lib->extractors[i].algorithm = AlgorithmFactory::create("ChordsDetection",
                                                                            "hopSize", (int)params->hopsize_samps,
                                                                            "sampleRate", sr,
                                                                            "windowSize", (int)windowSize);
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_PITCHCLASSPROFILEBATCH, "pcp");
                    set_essentia_outputs(lib, i, "bv", "chords", "strength");
                    set_custom_outputs(lib, i, "bv", "chords", "strength");
                    // TODO ASSERT
                    // The algorithm assumes that the sequence of the input HPCP frames has been computed with framesize = 2*hopsize
                }
                    break;

                    
                    // TODO: ChordsDetectionBeats
                    
                    // TODO: Chromagram (see if it has dependencies)

                case EARS_FEATURE_DISSONANCE:
                {
                    lib->extractors[i].algorithm = AlgorithmFactory::create("Dissonance");
                    set_input2(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRALPEAKS, "frequencies", "magnitudes");
                    set_essentia_outputs(lib, i, "f", "dissonance");
                    set_custom_outputs(lib, i, "f", "dissonance");
                }
                    break;
                    
                case EARS_FEATURE_HPCP:
                {
                    lib->extractors[i].algorithm = AlgorithmFactory::create("HPCP",
                                                                            "sampleRate", sr,
                                                                            "size", params->HPCP_size,
                                                                            "referenceFrequency", params->HPCP_referenceFrequency,
                                                                            "harmonics", params->HPCP_harmonics,
                                                                            "bandPreset", params->HPCP_bandPreset,
                                                                            "minFrequency", params->HPCP_minFrequency,
                                                                            "maxFrequency", params->HPCP_maxFrequency,
                                                                            "maxShifted", params->HPCP_maxShifted,
                                                                            "normalized", params->HPCP_normalized,
                                                                            "bandSplitFrequency", params->HPCP_bandSplitFrequency,
                                                                            "weightType", params->HPCP_weightType,
                                                                            "nonLinear", params->HPCP_nonLinear,
                                                                            "windowSize", params->HPCP_windowSize);
                    set_input2(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRALPEAKS, "frequencies", "magnitudes");
                    set_essentia_outputs(lib, i, "v", "hpcp");
                    set_custom_outputs(lib, i, "v", "hpcp");
                }
                    break;
                

                case EARS_FEATURE_HARMONICPEAKS:
                {
                    int maxHarmonics = 20;
                    double tolerance = 2000;
                    llll_parseattrs((t_object *)e_ob, args[i], LLLL_PA_DONTWARNFORWRONGKEYS, "id",
                                    gensym("maxharmonics"), &maxHarmonics,
                                    gensym("tolerance"), &tolerance);
                    lib->extractors[i].algorithm = AlgorithmFactory::create("HarmonicPeaks",
                                                                            "maxHarmonics", maxHarmonics,
                                                                            "tolerance", tolerance);
                    set_input3(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRALPEAKSANDFZERO, "frequencies", "magnitudes", "pitch");
                    set_essentia_outputs(lib, i, "v", "harmonicFrequencies", "harmonicMagnitudes");
                    set_custom_outputs(lib, i, "v", "harmonic frequencies", "harmonicMagnitudes");
                    lib->extractors[i].essentia_output_frequnit[0] = EARS_FREQUNIT_HERTZ;
                    lib->extractors[i].essentia_output_ampunit[1] = EARS_AMPUNIT_LINEAR;
                }
                    break;

                case EARS_FEATURE_HIGHRESOLUTIONFEATURES:
                {
                    int maxPeaks = 24;
                    llll_parseattrs((t_object *)e_ob, args[i], LLLL_PA_DONTWARNFORWRONGKEYS, "i",
                                    gensym("maxpeaks"), &maxPeaks);
                    lib->extractors[i].algorithm = AlgorithmFactory::create("HighResolutionFeatures",
                                                                            "maxPeaks", maxPeaks);
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_PITCHCLASSPROFILE, "pcp");
                    set_essentia_outputs(lib, i, "fff", "equalTemperedDeviation", "nonTemperedEnergyRatio", "nonTemperedPeaksEnergyRatio");
                    set_custom_outputs(lib, i, "fff", "equal tempered deviation", "non-tempered energy ratio", "non-tempered peaks energy ratio");
                    lib->extractors[i].essentia_output_frequnit[0] = EARS_FREQUNIT_HERTZ;
                    lib->extractors[i].essentia_output_ampunit[1] = EARS_AMPUNIT_LINEAR;
                    warn_if((t_object *)e_ob, params->verbose && params->HPCP_size < 120,
                            "The HighResolutionFeatures module works best with a HPCP size of at least 120.");
                    warn_if((t_object *)e_ob, params->verbose && params->HPCP_size % 12 != 0,
                            "The HighResolutionFeatures module needs a multiple of 12 as HPCP size.");
                }
                    break;
                    
                    
                case EARS_FEATURE_INHARMONICITY:
                {
                    lib->extractors[i].algorithm = AlgorithmFactory::create("Inharmonicity");
                    set_input2(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRALPEAKS, "frequencies", "magnitudes");
                    set_essentia_outputs(lib, i, "f", "inharmonicity");
                    set_custom_outputs(lib, i, "f", "inharmonicity");
                }
                    break;
                    
                    
                    
                case EARS_FEATURE_KEY:
                {
                    long numHarmonics = 4, pcpSize = 36;
                    t_symbol *profileType = gensym("bgate");
                    double slope = 0.6;
                    long useMajMin = 0, usePolyphony = 1, useThreeChords = 1;
                    llll_parseattrs((t_object *)e_ob, args[i], LLLL_PA_DONTWARNFORWRONGKEYS, "iisdiii",
                                    gensym("numharmonics"), &numHarmonics,
                                    gensym("pcpsize"), &pcpSize,
                                    gensym("profiletype"), &profileType,
                                    gensym("slope"), &slope,
                                    gensym("usemajmin"), &useMajMin,
                                    gensym("usepolyphony"), &usePolyphony,
                                    gensym("usethreechords"), &useThreeChords);
                    lib->extractors[i].algorithm = AlgorithmFactory::create("Key",
                                                                            "numHarmonics", (int)numHarmonics,
                                                                            "pcpSize", (int)pcpSize,
                                                                            "profileType", profileType->s_name,
                                                                            "slope", slope,
                                                                            "useMajMin", (bool)useMajMin,
                                                                            "usePolyphony", (bool)usePolyphony,
                                                                            "useThreeChords", (bool)useThreeChords
                                                                            );
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_PITCHCLASSPROFILE, "pcp");
                    set_essentia_outputs(lib, i, "ccff", "key", "scale", "strength", "firstToSecondRelativeStrength");
                    set_custom_outputs(lib, i, "ccff", "key", "scale", "strength", "first-to-second relative strength");
                }
                    break;
                    
               
                    
                case EARS_FEATURE_KEYEXTRACTOR:
                {
                    long averageDetuningCorrection = 1, hpcpSize = 12, maximumSpectralPeaks = 60;
                    double frameSize = 4096, hopSize = 4096, maxFrequency = 3500, minFrequency = 25, pcpThreshold = 0.2, spectralPeaksThreshold = 0.0001;
                    t_symbol *profileType = gensym("bgate"), *weightType = gensym("cosine"), *windowType = gensym("hann");
                    earsbufobj_essentia_convert_frequnit(e_ob, maxFrequency, NULL, EARS_FREQUNIT_HERTZ);
                    earsbufobj_essentia_convert_frequnit(e_ob, minFrequency, NULL, EARS_FREQUNIT_HERTZ);
                    earsbufobj_essentia_convert_antimeunit(e_ob, frameSize, NULL, EARS_TIMEUNIT_SAMPS);
                    earsbufobj_essentia_convert_antimeunit(e_ob, hopSize, NULL, EARS_TIMEUNIT_SAMPS);
                    llll_parseattrs((t_object *)e_ob, args[i], LLLL_PA_DONTWARNFORWRONGKEYS, "iddiiddddsss",
                                    gensym("averagedetuningcorrection"), &averageDetuningCorrection,
                                    gensym("framesize"), &frameSize,
                                    gensym("hopsize"), &hopSize,
                                    gensym("hpcpsize"), &hpcpSize,
                                    gensym("maximumspectralpeaks"), &maximumSpectralPeaks,
                                    gensym("maxfrequency"), &maxFrequency,
                                    gensym("minfrequency"), &minFrequency,
                                    gensym("pcpthreshold"), &pcpThreshold,
                                    gensym("spectralpeaksthreshold"), &spectralPeaksThreshold,
                                    gensym("profiletype"), &profileType,
                                    gensym("weighttype"), &weightType,
                                    gensym("windowtype"), &windowType
                                    );
                    lib->extractors[i].algorithm = AlgorithmFactory::create("KeyExtractor",
                                                                            "sampleRate", sr,
                                                                            "averageDetuningCorrection", (bool)averageDetuningCorrection,
                                                                            "frameSize", (int)frameSize,
                                                                            "hopSize", (int)hopSize,
                                                                            "hpcpSize", (int)hpcpSize,
                                                                            "maximumSpectralPeaks", (int)maximumSpectralPeaks,
                                                                            "maxFrequency", maxFrequency,
                                                                            "minFrequency", minFrequency,
                                                                            "pcpThreshold", pcpThreshold,
                                                                            "spectralPeaksThreshold", spectralPeaksThreshold,
                                                                            "profileType", profileType->s_name,
                                                                            "weightType", weightType->s_name,
                                                                            "windowType", windowType->s_name
                                                                            );
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_AUDIO, "audio");
                    set_essentia_outputs(lib, i, "ccf", "key", "scale", "strength");
                    set_custom_outputs(lib, i, "ccf", "key", "scale", "strength");
                }
                    break;
                
                    // missing: NNLSChroma
                    
                    
                case EARS_FEATURE_ODDTOEVENHARMONICENERGYRATIO:
                {
                    lib->extractors[i].algorithm = AlgorithmFactory::create("OddToEvenHarmonicEnergyRatio");
                    set_input2(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRALPEAKS, "frequencies", "magnitudes");
                    set_essentia_outputs(lib, i, "f", "oddToEvenHarmonicEnergyRatio");
                    set_custom_outputs(lib, i, "f", "odd-to-even harmonic energy ratio");
                }
                    break;

                    
                case EARS_FEATURE_PITCHSALIENCE:
                {
                    double highBoundary = 5000, lowBoundary = 100;
                    earsbufobj_essentia_convert_frequnit(e_ob, highBoundary, NULL, EARS_FREQUNIT_HERTZ);
                    earsbufobj_essentia_convert_frequnit(e_ob, lowBoundary, NULL, EARS_FREQUNIT_HERTZ);
                    llll_parseattrs((t_object *)e_ob, args[i], LLLL_PA_DONTWARNFORWRONGKEYS, "dd",
                                    gensym("highboundary"), &highBoundary,
                                    gensym("lowboundary"), &lowBoundary);
                    lib->extractors[i].algorithm = AlgorithmFactory::create("PitchSalience",
                                                                            "lowBoundary", lowBoundary,
                                                                            "highBoundary", highBoundary,
                                                                            "sampleRate", sr);
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRUM, "spectrum");
                    set_essentia_outputs(lib, i, "f", "oddToEvenHarmonicEnergyRatio");
                    set_custom_outputs(lib, i, "f", "odd-to-even harmonic energy ratio");
                }
                    break;

                    
                case EARS_FEATURE_SPECTRUMCQ:
                {
                    lib->extractors[i].algorithm = AlgorithmFactory::create("SpectrumCQ",
                                                                            "binsPerOctave", params->binsPerOctave,
                                                                            "minFrequency", params->minFrequency,
                                                                            "minimumKernelSize", params->minimumKernelSize,
                                                                            "numberBins", params->numberBins,
                                                                            "sampleRate", sr,
                                                                            "scale", params->scale,
                                                                            "threshold", params->threshold,
                                                                            "windowType", params->windowType,
                                                                            "zeroPhase", true
                                                                            );
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_FRAME, "spectrumCQ");
                    set_essentia_outputs(lib, i, "f", "spectrumCQ");
                    set_custom_outputs(lib, i, "f", "constant-q spectrum");
                    t_llll *bins = ears_ezarithmser(ears_hz_to_cents(params->minFrequency, EARS_MIDDLE_A_TUNING), 1200./params->binsPerOctave, params->numberBins);
                    set_spec_metadata(lib, i, 1200./params->binsPerOctave, ears_hz_to_cents(params->minFrequency, EARS_MIDDLE_A_TUNING), gensym("cqt"), EARS_FREQUNIT_CENTS, bins, true);
                    llll_free(bins);
                }
                    break;
                    
                    
                    // will drop: TonalExtractor (combination of others)
                    // missing: TonicIndianArtMusic
                    
                case EARS_FEATURE_TRISTIMULUS:
                {
                    lib->extractors[i].algorithm = AlgorithmFactory::create("Tristimulus");
                    set_input2(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRALPEAKS, "frequencies", "magnitudes");
                    set_essentia_outputs(lib, i, "v", "tristimulus");
                    set_custom_outputs(lib, i, "v", "tristimulus");
                }
                    break;

                    
                case EARS_FEATURE_TUNINGFREQUENCY:
                {
                    long resolution = 1;
                    llll_parseattrs((t_object *)e_ob, args[i], LLLL_PA_DONTWARNFORWRONGKEYS, "i",
                                    gensym("resolution"), &resolution);
                    lib->extractors[i].algorithm = AlgorithmFactory::create("TuningFrequency",
                                                                            "resolution", (int)resolution);
                    set_input2(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRALPEAKS, "frequencies", "magnitudes");
                    set_essentia_outputs(lib, i, "ff", "tuningFrequency", "tuningCents");
                    set_custom_outputs(lib, i, "ff", "tuning frequency", "pitch cents difference with A=440");
                    lib->extractors[i].essentia_output_frequnit[0] = EARS_FREQUNIT_HERTZ;
                    lib->extractors[i].essentia_output_pitchunit[1] = EARS_PITCHUNIT_CENTS;
                }
                    break;
                    

///////////////////////////////////////////////////////////////////////////////////////////
///// ***************************** FINGERPRINTING **************************************//
///////////////////////////////////////////////////////////////////////////////////////////



                case EARS_FEATURE_CHROMAPRINTER:
                {
                    double maxLength = 0;
                    llll_parseattrs((t_object *)e_ob, args[i], LLLL_PA_DONTWARNFORWRONGKEYS, "d",
                                    gensym("maxlength"), &maxLength);
                    earsbufobj_essentia_convert_timeunit(e_ob, maxLength, NULL, EARS_TIMEUNIT_SECONDS);
                    lib->extractors[i].algorithm = AlgorithmFactory::create("Chromaprinter",
                                                                            "maxLength", maxLength,
                                                                            "sampleRate", sr);
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_AUDIO, "signal");
                    set_essentia_outputs(lib, i, "c", "fingerprint");
                    set_custom_outputs(lib, i, "c", "fingerprint");
                }
                    break;
                    

///////////////////////////////////////////////////////////////////////////////////////////
///// ***************************** AUDIO PROBLEMS **************************************//
///////////////////////////////////////////////////////////////////////////////////////////

                    // TODO ALL
                    
                    // Todo TruePeakDetector

       
                    
////////////////////////////////////////////////////////////////////////////////////////////////
///// ***************************** DURATION/SILENCE **************************************/////
////////////////////////////////////////////////////////////////////////////////////////////////

                case EARS_FEATURE_DURATION:
                    lib->extractors[i].algorithm = AlgorithmFactory::create("Duration",
                                                                            "sampleRate", sr);
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_AUDIO, "signal");
                    set_essentia_outputs(lib, i, "f", "duration");
                    set_custom_outputs(lib, i, "f", "duration");
                    lib->extractors[i].essentia_output_timeunit[0] = EARS_TIMEUNIT_SECONDS;
                    break;
                    
                case EARS_FEATURE_EFFECTIVEDURATION:
                {
                    double thresholdRatio = 0.4;
                    llll_parseattrs((t_object *)e_ob, args[i], LLLL_PA_DONTWARNFORWRONGKEYS, "d",
                                    gensym("thresholdratio"), &thresholdRatio);
                    lib->extractors[i].algorithm = AlgorithmFactory::create("EffectiveDuration",
                                                                            "sampleRate", sr,
                                                                            "thresholdRatio", thresholdRatio);
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_AUDIO, "signal");
                    set_essentia_outputs(lib, i, "f", "effectiveDuration");
                    set_custom_outputs(lib, i, "f", "effective duration");
                    lib->extractors[i].essentia_output_timeunit[0] = EARS_TIMEUNIT_SECONDS;
                }
                    break;
                    
                    //TODO: FadeDetection
                    
                case EARS_FEATURE_SILENCERATE:
                {
                    t_llll *thresholds_llll = NULL;
                    llll_parseattrs((t_object *)e_ob, args[i], LLLL_PA_DONTWARNFORWRONGKEYS, "l",
                                    gensym("thresholds_llll"), &thresholds_llll);
                    std::vector<Real> thresholds;
                    if (!thresholds_llll) {
                        thresholds_llll = llll_from_text_buf("-20 -30 -60");
                        thresholds = llll_to_vector_real(thresholds_llll);
                    } else {
                        thresholds = llll_to_vector_real(thresholds_llll);
                        earsbufobj_essentia_convert_ampunit(e_ob, thresholds, NULL, EARS_AMPUNIT_DECIBEL);
                    }
                    lib->extractors[i].algorithm = AlgorithmFactory::create("EffectiveDuration",
                                                                            "sampleRate", sr,
                                                                            "thresholds", thresholds);
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_FRAME, "frame");
                    long num_outputs = thresholds.size();
                    for (int o = 0; o < num_outputs; o++) {
                        lib->extractors[i].essentia_output_label[o] = get_threshold_desc(o);
                        lib->extractors[i].output_desc[o] = get_threshold_desc(o);
                    }
                    lib->extractors[i].num_outputs = num_outputs;
                    lib->extractors[i].essentia_num_outputs = num_outputs;
                    for (long o = 0; o < num_outputs; o++)
                        lib->extractors[i].output_map[o] = 0;
                    for (long o = 0; o < num_outputs; o++) {
                        lib->extractors[i].essentia_output_type[o] = 'f';
                        lib->extractors[i].output_type[o] = 'f';
                    }
                }
                    break;
                    
                // TODO StartStopSilence
                    

/////////////////////////////////////////////////////////////////////////////////////////////////
///// ***************************** LOUDNESS/DYNAMICS **************************************/////
/////////////////////////////////////////////////////////////////////////////////////////////////

                case EARS_FEATURE_DYNAMICCOMPLEXITY:
                {
                    lib->extractors[i].algorithm = AlgorithmFactory::create("DynamicComplexity",
                                                                            "sampleRate", sr,
                                                                            "frameSize", ears_samps_to_ms(params->framesize_samps, sr)/1000.);
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_AUDIO, "signal");
                    set_essentia_outputs(lib, i, "ff", "dynamicComplexity", "loudness");
                    set_custom_outputs(lib, i, "ff", "dynamic complexity coefficient", "estimate of the loudness");
                    lib->extractors[i].essentia_output_ampunit[1] = EARS_AMPUNIT_DECIBEL;
                }
                    break;

                    // Intensity has: quality = outdated
                                                                            
                case EARS_FEATURE_LARM:
                {
                    double attackTime = 10, releaseTime = 1500, power = 1.5;
                    llll_parseattrs((t_object *)e_ob, args[i], LLLL_PA_DONTWARNFORWRONGKEYS, "ddd",
                                    gensym("attackTime"), &attackTime, gensym("releaseTime"), &releaseTime, gensym("power"), &power);
                    earsbufobj_essentia_convert_timeunit(e_ob, attackTime, NULL, EARS_TIMEUNIT_MS);
                    earsbufobj_essentia_convert_timeunit(e_ob, releaseTime, NULL, EARS_TIMEUNIT_MS);
                    lib->extractors[i].algorithm = AlgorithmFactory::create("Larm",
                                                                            "attackTime", attackTime,
                                                                            "releaseTime", releaseTime,
                                                                            "power", power,
                                                                            "sampleRate", sr);
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_AUDIO, "signal");
                    set_essentia_outputs(lib, i, "f", "larm");
                    set_custom_outputs(lib, i, "f", "LARM loudness estimate");
                    lib->extractors[i].essentia_output_ampunit[0] = EARS_AMPUNIT_DECIBEL;
                }
                    break;

                    
                    
                case EARS_FEATURE_LEQ:
                {
                    lib->extractors[i].algorithm = AlgorithmFactory::create("Leq");
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_AUDIO, "signal");
                    set_essentia_outputs(lib, i, "f", "leq");
                    set_custom_outputs(lib, i, "f", "equivalent sound level estimate");
                    lib->extractors[i].essentia_output_ampunit[0] = EARS_AMPUNIT_DECIBEL;
                }
                    break;

                    
                    
                case EARS_FEATURE_LOUDNESS:
                {
                    lib->extractors[i].algorithm = AlgorithmFactory::create("Loudness");
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_FRAME, "signal");
                    set_essentia_outputs(lib, i, "f", "loudness");
                    set_custom_outputs(lib, i, "f", "loudness");
                    lib->extractors[i].essentia_output_ampunit[0] = EARS_AMPUNIT_LINEAR;
                }
                    break;

                    
                case EARS_FEATURE_LOUDNESSVICKERS:
                {
                    lib->extractors[i].algorithm = AlgorithmFactory::create("LoudnessVickers",
                                                                            "sampleRate", sr);
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_FRAME, "signal");
                    set_essentia_outputs(lib, i, "f", "loudness");
                    set_custom_outputs(lib, i, "f", "loudness");
                    lib->extractors[i].essentia_output_ampunit[0] = EARS_AMPUNIT_DECIBEL;
                }
                    break;

                    
                // LoudnessEBUR128 missing for now, because it needs a different input format, rigourously STEREO (vector_stereosample)
                    
                case EARS_FEATURE_REPLAYGAIN:
                {
                    lib->extractors[i].algorithm = AlgorithmFactory::create("ReplayGain",
                                                                            "sampleRate", sr);
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_AUDIO, "signal");
                    set_essentia_outputs(lib, i, "f", "replayGain");
                    set_custom_outputs(lib, i, "f", "distance to the suitable average replay level (31dbB) defined by SMPTE");
                    warn_if((t_object *)e_ob, params->verbose && lib->extractors[i].temporalmode != EARS_ESSENTIA_TEMPORALMODE_WHOLE &&
                            ears_convert_timeunit(params->framesize_samps, NULL, EARS_TIMEUNIT_SAMPS, EARS_TIMEUNIT_MS) < 50,
                            "The ReplayGain module needs at least windows of 50ms; increase the 'winsize' attirbute.");
                    lib->extractors[i].essentia_output_ampunit[0] = EARS_AMPUNIT_DECIBEL;
                }
                    break;
                    
                    

/////////////////////////////////////////////////////////////////////////////////////
///// ***************************** PITCH **************************************/////
/////////////////////////////////////////////////////////////////////////////////////

                case EARS_FEATURE_MULTIPITCHKLAPURI:
                    {
                        double binResolution = 10, harmonicWeight = 0.8, frameSize = 2048, hopSize = 128;
                        double magnitudeCompression = 1, magnitudeThreshold = 40, maxFrequency = 1760, minFrequency = 80;
                        long numberHarmonics = 10;
                        double referenceFrequency = 55;
                        llll_parseattrs((t_object *)e_ob, args[i], LLLL_PA_DONTWARNFORWRONGKEYS, "ddddddddid",
                                        gensym("binresolution"), &binResolution,
                                        gensym("harmonicweight"), &harmonicWeight,
                                        gensym("framesize"), &frameSize,
                                        gensym("hopsize"), &hopSize,
                                        gensym("magnitudecompression"), &magnitudeCompression,
                                        gensym("magnitudethreshold"), &magnitudeThreshold,
                                        gensym("maxfrequency"), &maxFrequency,
                                        gensym("minfrequency"), &minFrequency,
                                        gensym("numberharmonics"), &numberHarmonics,
                                        gensym("referencefrequency"), &referenceFrequency);
                        earsbufobj_essentia_convert_frequnit(e_ob, maxFrequency, NULL, EARS_FREQUNIT_HERTZ);
                        earsbufobj_essentia_convert_frequnit(e_ob, minFrequency, NULL, EARS_FREQUNIT_HERTZ);
                        earsbufobj_essentia_convert_antimeunit(e_ob, frameSize, NULL, EARS_TIMEUNIT_SAMPS);
                        earsbufobj_essentia_convert_antimeunit(e_ob, hopSize, NULL, EARS_TIMEUNIT_SAMPS);
                        earsbufobj_essentia_convert_frequnit(e_ob, referenceFrequency, NULL, EARS_FREQUNIT_HERTZ);
                        lib->extractors[i].algorithm = AlgorithmFactory::create("MultiPitchKlapuri",
                                                                                "sampleRate", sr,
                                                                                "binResolution", binResolution,
                                                                                "harmonicWeight", harmonicWeight,
                                                                                "frameSize", (int)frameSize,
                                                                                "hopSize", (int)hopSize,
                                                                                "magnitudeCompression", magnitudeCompression,
                                                                                "magnitudeThreshold", magnitudeThreshold,
                                                                                "maxFrequency", maxFrequency,
                                                                                "minFrequency", minFrequency,
                                                                                "numberHarmonics", (int)numberHarmonics,
                                                                                "referenceFrequency", referenceFrequency
                                                                                );
                        set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_AUDIO, "signal");
                        set_essentia_outputs(lib, i, "w", "pitch");
                        set_custom_outputs(lib, i, "w", "pitch");
                        // TO DO: summarization
                        lib->extractors[i].essentia_output_pitchunit[0] = EARS_PITCHUNIT_HERTZ;
                    }
                    break;
                    
                case EARS_FEATURE_MULTIPITCHMELODIA:
                    {
                        double binResolution = 10, harmonicWeight = 0.8, frameSize = 2048, hopSize = 128;
                        double magnitudeCompression = 1, magnitudeThreshold = 40, maxFrequency = 20000, minFrequency = 40;
                        double minDuration = 10, peakDistributionThreshold = 0.9, peakFrameThreshold = 0.9, pitchContinuity = 27.5625, timeContinuity = 100;
                        long numberHarmonics = 20, filterIterations = 3, guessUnvoiced = 0;
                        double referenceFrequency = 55;
                        llll_parseattrs((t_object *)e_ob, args[i], LLLL_PA_DONTWARNFORWRONGKEYS, "diiddddddddiddddd",
                                        gensym("binresolution"), &binResolution,
                                        gensym("filteriterations"), &filterIterations,
                                        gensym("guessunvoiced"), &guessUnvoiced,
                                        gensym("harmonicweight"), &harmonicWeight,
                                        gensym("framesize"), &frameSize,
                                        gensym("hopsize"), &hopSize,
                                        gensym("magnitudecompression"), &magnitudeCompression,
                                        gensym("magnitudethreshold"), &magnitudeThreshold,
                                        gensym("minduration"), &minDuration,
                                        gensym("maxfrequency"), &maxFrequency,
                                        gensym("minfrequency"), &minFrequency,
                                        gensym("numberharmonics"), &numberHarmonics,
                                        gensym("peakdistributionthreshold"), &peakDistributionThreshold,
                                        gensym("peakframethreshold"), &peakFrameThreshold,
                                        gensym("pitchcontinuity"), &pitchContinuity,
                                        gensym("referencefrequency"), &referenceFrequency,
                                        gensym("timecontinuity"), &timeContinuity);
                        earsbufobj_essentia_convert_timeunit(e_ob, minDuration, NULL, EARS_TIMEUNIT_MS);
                        earsbufobj_essentia_convert_timeunit(e_ob, timeContinuity, NULL, EARS_TIMEUNIT_MS);
                        earsbufobj_essentia_convert_pitchunit(e_ob, pitchContinuity, NULL, EARS_PITCHUNIT_CENTS);
                        earsbufobj_essentia_convert_frequnit(e_ob, maxFrequency, NULL, EARS_FREQUNIT_HERTZ);
                        earsbufobj_essentia_convert_frequnit(e_ob, minFrequency, NULL, EARS_FREQUNIT_HERTZ);
                        earsbufobj_essentia_convert_antimeunit(e_ob, frameSize, NULL, EARS_TIMEUNIT_SAMPS);
                        earsbufobj_essentia_convert_antimeunit(e_ob, hopSize, NULL, EARS_TIMEUNIT_SAMPS);
                        earsbufobj_essentia_convert_frequnit(e_ob, referenceFrequency, NULL, EARS_FREQUNIT_HERTZ);
                        lib->extractors[i].algorithm = AlgorithmFactory::create("MultiPitchMelodia",
                                                                                "sampleRate", sr,
                                                                                "binResolution", binResolution,
                                                                                "filterIterations", (int)filterIterations,
                                                                                "guessUnvoiced", (bool)guessUnvoiced,
                                                                                "harmonicWeight", harmonicWeight,
                                                                                "frameSize", (int)frameSize,
                                                                                "hopSize", (int)hopSize,
                                                                                "magnitudeCompression", magnitudeCompression,
                                                                                "magnitudeThreshold", magnitudeThreshold,
                                                                                "maxFrequency", maxFrequency,
                                                                                "minFrequency", minFrequency,
                                                                                "minDuration", (int)minDuration,
                                                                                "numberHarmonics", (int)numberHarmonics,
                                                                                "peakDistributionThreshold", peakDistributionThreshold
                                                                                );
                        lib->extractors[i].algorithm->configure("peakFrameThreshold", peakFrameThreshold,
                                                                "pitchContinuity", pitchContinuity,
                                                                "referenceFrequency", referenceFrequency,
                                                                "timeContinuity", (int)timeContinuity);

                        set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_AUDIO, "signal");
                        set_essentia_outputs(lib, i, "w", "pitch");
                        set_custom_outputs(lib, i, "w", "pitch");
                        // TO DO: summarization
                        lib->extractors[i].essentia_output_pitchunit[0] = EARS_PITCHUNIT_HERTZ;
                    }
                    break;
                    
                    // PitchContourSegmentation?
                    
                    // MELODIA, ETC: TO DO
                    
                    
                    
                case EARS_FEATURE_PITCHSALIENCEFUNCTION:
                {
                    double binResolution = 10, harmonicWeight = 0.8, magnitudeCompression = 1., magnitudeThreshold = 40., referenceFrequency = 55;
                    long numberHarmonics = 20;
                    llll_parseattrs((t_object *)e_ob, args[i], LLLL_PA_DONTWARNFORWRONGKEYS, "dddddi",
                                    gensym("binresolution"), &binResolution,
                                    gensym("harmonicweight"), &harmonicWeight,
                                    gensym("magnitudecompression"), &magnitudeCompression,
                                    gensym("magnitudethreshold"), &magnitudeThreshold,
                                    gensym("referencefrequency"), &referenceFrequency,
                                    gensym("numberharmonics"), &numberHarmonics);
                    earsbufobj_essentia_convert_frequnit(e_ob, referenceFrequency, NULL, EARS_FREQUNIT_HERTZ);
                    lib->extractors[i].algorithm = AlgorithmFactory::create("PitchSalienceFunction",
                                                                            "binResolution", binResolution,
                                                                            "harmonicWeight", harmonicWeight,
                                                                            "magnitudeCompression", magnitudeCompression,
                                                                            "magnitudeThreshold", magnitudeThreshold,
                                                                            "referenceFrequency", referenceFrequency,
                                                                            "numberharmonics", (int)numberHarmonics);
                    set_input2(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRALPEAKS, "frequencies", "magnitudes");
                    set_essentia_outputs(lib, i, "v", "salienceFunction");
                    set_custom_outputs(lib, i, "v", "array of the quantized pitch salience values");
                }
                    break;
                    
                
                
                case EARS_FEATURE_PITCHYIN:
                {
                    double maxFrequency = 22050, minFrequency = 20, tolerance = 0.15;
                    long interpolate = 1;
                    llll_parseattrs((t_object *)e_ob, args[i], LLLL_PA_DONTWARNFORWRONGKEYS, "dddi",
                                    gensym("maxFrequency"), &maxFrequency,
                                    gensym("minFrequency"), &minFrequency,
                                    gensym("tolerance"), &tolerance,
                                    gensym("interpolate"), &interpolate);
                    earsbufobj_essentia_convert_frequnit(e_ob, maxFrequency, NULL, EARS_FREQUNIT_HERTZ);
                    earsbufobj_essentia_convert_frequnit(e_ob, minFrequency, NULL, EARS_FREQUNIT_HERTZ);
                    lib->extractors[i].algorithm = AlgorithmFactory::create("PitchYin",
                                                                            "frameSize", params->framesize_samps,
                                                                            "sampleRate", sr,
                                                                            "interpolate", (bool)interpolate,
                                                                            "maxFrequency", maxFrequency,
                                                                            "minFrequency", minFrequency,
                                                                            "tolerance", tolerance);
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_FRAME, "signal");
                    set_essentia_outputs(lib, i, "ff", "pitch", "pitchConfidence");
                    set_custom_outputs(lib, i, "ff", "pitch", "confidence");
                    lib->extractors[i].essentia_output_pitchunit[0] = EARS_PITCHUNIT_HERTZ;
                }
                    break;

                    
                case EARS_FEATURE_PITCHYINFFT:
                {
                    double maxFrequency = 22050, minFrequency = 20, tolerance = 0.15;
                    long interpolate = 1;
                    llll_parseattrs((t_object *)e_ob, args[i], LLLL_PA_DONTWARNFORWRONGKEYS, "dddi",
                                    gensym("maxFrequency"), &maxFrequency,
                                    gensym("minFrequency"), &minFrequency,
                                    gensym("tolerance"), &tolerance,
                                    gensym("interpolate"), &interpolate);
                    earsbufobj_essentia_convert_frequnit(e_ob, maxFrequency, NULL, EARS_FREQUNIT_HERTZ);
                    earsbufobj_essentia_convert_frequnit(e_ob, minFrequency, NULL, EARS_FREQUNIT_HERTZ);
                    lib->extractors[i].algorithm = AlgorithmFactory::create("PitchYinFFT",
                                                                            "frameSize", params->framesize_samps,
                                                                            "sampleRate", sr,
                                                                            "interpolate", (bool)interpolate,
                                                                            "maxFrequency", maxFrequency,
                                                                            "minFrequency", minFrequency,
                                                                            "tolerance", tolerance);
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRUM, "spectrum");
                    set_essentia_outputs(lib, i, "ff", "pitch", "pitchConfidence");
                    set_custom_outputs(lib, i, "ff", "pitch", "confidence");
                    lib->extractors[i].essentia_output_pitchunit[0] = EARS_PITCHUNIT_HERTZ;
                    // warn if window is not hann?
                }
                    break;
                
                    
                    
                case EARS_FEATURE_PITCHYINPROBABILISTIC:
                {
                    double lowRMSThreshold = 0.1;
                    long preciseTime = 0;
                    t_symbol *outputUnvoiced = gensym("negative");
                    llll_parseattrs((t_object *)e_ob, args[i], LLLL_PA_DONTWARNFORWRONGKEYS, "dis",
                                    gensym("lowrmsthreshold"), &lowRMSThreshold,
                                    gensym("precisetime"), &preciseTime,
                                    gensym("outputunvoiced"), &outputUnvoiced);
                    lib->extractors[i].algorithm = AlgorithmFactory::create("PitchYinProbabilistic",
                                                                            "frameSize", params->framesize_samps,
                                                                            "hopSize", (int)params->hopsize_samps,
                                                                            "sampleRate", sr,
                                                                            "lowRMSThreshold", lowRMSThreshold,
                                                                            "preciseTime", (bool)preciseTime,
                                                                            "outputUnvoiced", outputUnvoiced->s_name);
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_AUDIO, "signal");
                    set_essentia_outputs(lib, i, "vv", "pitch", "voicedProbabilities");
                    set_custom_outputs(lib, i, "vv", "pitch", "voiced probabilities");
                    lib->extractors[i].essentia_output_pitchunit[0] = EARS_PITCHUNIT_HERTZ;
                    // TO DO: set that this is batch but gives frames --- cannot be done frame-wise, but can be summarized
                }
                    break;
                
                default:
                    lib->extractors[i].algorithm = NULL;
                    set_input(lib, i, EARS_ESSENTIA_EXTRACTOR_INPUT_AUDIO, "none");
                    set_essentia_outputs(lib, i, "f", "none");
                    break;
            }
        }
        lib->num_extractors = num_features;
    } catch (essentia::EssentiaException e) {
        object_error((t_object *)e_ob, e.what());
        err = EARS_ERR_ESSENTIA;
        lib->num_extractors = 0;
    }

    
    return err;
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
    delete lib->alg_SpectrumCentralMoments;
    delete lib->alg_EnvelopeCentralMoments;
    delete lib->alg_EqualLoudness;
    delete lib->alg_Loudness;
    delete lib->alg_RMS;
    delete lib->alg_SpectralPeaks;
    delete lib->alg_HPCP;
    delete lib->alg_YinFFT;
    for (long i = 0; i < lib->num_extractors; i++) {
        for (long o = 0; o < EARS_ESSENTIA_EXTRACTOR_MAX_OUTPUTS; o++) {
            if (lib->extractors[i].specdata.bins)
                llll_free(lib->extractors[i].specdata.bins);
            if (lib->extractors[i].result[o])
                llll_free(lib->extractors[i].result[o]);
        }
        if (lib->extractors[i].algorithm)
            delete lib->extractors[i].algorithm;
    }
    bach_freeptr(lib->extractors);
}

Real vector_average(std::vector<Real> v, e_ears_essentia_summarization summarization, e_ears_essentia_summarizationweight weight, std::vector<Real> rms_weights, std::vector<Real> loudness_weights)
{
    switch (summarization) {
        case EARS_ESSENTIA_SUMMARIZATION_FIRST:
            return v.size() > 0 ? v[0] : 0;
            break;

        case EARS_ESSENTIA_SUMMARIZATION_LAST:
            return v.size() > 0 ? v[v.size()-1] : 0;
            break;

        case EARS_ESSENTIA_SUMMARIZATION_MIDDLE:
            return v.size() > 0 ? v[v.size()/2] : 0;
            break;

        case EARS_ESSENTIA_SUMMARIZATION_MEAN:
        {
            if (weight == EARS_ESSENTIA_SUMMARIZATIONWEIGHT_NONE) {
                double sum = 0;
                for (long i = 0; i < v.size(); i++)
                    sum += v[i];
                return sum/v.size();
            } else {
                Real *weights = (weight == EARS_ESSENTIA_SUMMARIZATIONWEIGHT_RMS ? &rms_weights[0] : &loudness_weights[0]);
                double sum = 0, tot_weights = 0;
                for (long i = 0; i < v.size(); i++) {
                    sum += v[i] * weights[i];
                    tot_weights += weights[i];
                }
                return tot_weights == 0 ? 0 : sum/tot_weights;
            }
        }
            break;

        default:
        {
            double sum = 0;
            for (long i = 0; i < v.size(); i++)
                sum += v[i];
            return sum/v.size();
        }
            break;
    }
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


// feature dimension = 1
std::vector<std::vector<Real>> vector_of_vector_average(std::vector<std::vector<std::vector<Real>>> v, e_ears_essentia_summarization summarization, e_ears_essentia_summarizationweight summarizationweight, std::vector<Real> rms_weights, std::vector<Real> loudness_weights)
{
    std::vector<std::vector<Real>> res;
    if (v.size() > 0) {
        switch (summarization) {
            case EARS_ESSENTIA_SUMMARIZATION_FIRST:
                res = v[0];
                break;
                
            case EARS_ESSENTIA_SUMMARIZATION_LAST:
                res = v[v.size()-1];
                break;
                
            case EARS_ESSENTIA_SUMMARIZATION_MIDDLE:
                res = v[v.size()/2];
                break;
                
            case EARS_ESSENTIA_SUMMARIZATION_MEAN:
                if (summarizationweight == EARS_ESSENTIA_SUMMARIZATIONWEIGHT_NONE) {
                    if (v.size() > 0 && v[0].size() > 0 && v[0][0].size() > 0) {
                        for (long f = 0; f < v[0].size(); f++) {
                            std::vector<Real> innermost;
                            for (long g = 0; g < v[0][f].size(); g++) {
                                Real sum = 0;
                                for (long j = 0; j < v.size(); j++) {
                                    sum += v[j][f][g];
                                }
                                innermost.push_back(sum/v.size());
                            }
                            res.push_back(innermost);
                        }
                    }
                } else {
                    Real *weights = (summarizationweight == EARS_ESSENTIA_SUMMARIZATIONWEIGHT_RMS ? &rms_weights[0] : &loudness_weights[0]);
                    if (v.size() > 0 && v[0].size() > 0 && v[0][0].size() > 0) {
                        for (long f = 0; f < v[0].size(); f++) {
                            std::vector<Real> innermost;
                            for (long g = 0; g < v[0][f].size(); g++) {
                                Real sum = 0;
                                double tot_weight = 0;
                                for (long j = 0; j < v.size(); j++) {
                                    sum += v[j][f][g] * weights[j];
                                    tot_weight += weights[j];
                                }
                                innermost.push_back(tot_weight == 0 ? 0 : sum/tot_weight);
                            }
                            res.push_back(innermost);
                        }
                    }
                }
                break;
                
            case EARS_ESSENTIA_SUMMARIZATION_MODE:
                if (v.size() > 0 && v[0].size() > 0 && v[0][0].size() > 0) {
                    std::vector<long> unique_ids;
                    std::vector<long> unique_ids_countequal;
                    for (long s = 0; s < v.size(); s++) {
                        long this_unique_id = 0, mul = 1;
                        for (long f = 0; f < v[s].size(); f++) {
                            for (long g = 0; g < v[s][f].size(); g++) { // v[0][f].size() will likely be 1
                                this_unique_id += (long)(mul * v[s][f][g]);
                                mul *= 256; // likely numbers of ASCII chars
                            }
                        }
                        unique_ids.push_back(this_unique_id);
                    }
                    
                    // slow, improve
                    long maxcount = 0;
                    long elem_pivot = -1;
                    for (long s = 0; s < v.size(); s++) {
                        double count = 0;
                        for (long t = 0; t < v.size(); t++) {
                            if (unique_ids[t] == unique_ids[s]) {
                                if (summarizationweight == EARS_ESSENTIA_SUMMARIZATIONWEIGHT_RMS)
                                    count += rms_weights[s];
                                else if (summarizationweight == EARS_ESSENTIA_SUMMARIZATIONWEIGHT_LOUDNESS)
                                    count += loudness_weights[s];
                                else
                                    count++;
                            }
                        }
                        if (count > maxcount) {
                            maxcount = count;
                            elem_pivot = s;
                        }
                    }
                    if (elem_pivot > -1)
                        res = v[elem_pivot];
                }
                break;

            default:
                break;
        }
    }
    return res;
}

/*
// channels then samples
std::vector<Real> vector_of_vector_average(std::vector<std::vector<std::vector<Real>>> v, int feature_dim, e_ears_essentia_summarization summarization, std::vector<Real> loudness_weights)
{
    std::vector<std::vector<Real>> res;
    if (feature_dim == 1) {
        if (v.size() > 0) {
            long num_features = v[0].size();
            switch (summarization) {
                case EARS_ESSENTIA_SUMMARIZATION_FIRST:
                    res = v[0];
                    break;

                case EARS_ESSENTIA_SUMMARIZATION_LAST:
                    res = v[v.size()-1];
                    break;

                case EARS_ESSENTIA_SUMMARIZATION_MIDDLE:
                    res = v[v.size()/2];
                    break;

                case EARS_ESSENTIA_SUMMARIZATION_MEAN:
                    if (v.size() > 0 && v[0].size() > 0 && v[0][0].size() > 0) {
                        for (long f = 0; f < v[0].size(); f++) {
                            std::vector<Real> innermost;
                            for (long g = 0; g < v[0][f].size(); g++) {
                                Real sum = 0;
                                for (long j = 0; j < v.size(); j++) {
                                    sum += v[j][f][g];
                                }
                                innermost.push_back(sum/v.size());
                            }
                            res.push_back(innermost);
                        }
                    }
                    break;

                case EARS_ESSENTIA_SUMMARIZATION_LOUDNESSWEIGHTEDMEAN:
                    if (v.size() > 0 && v[0].size() > 0 && v[0][0].size() > 0) {
                        for (long f = 0; f < v[0].size(); f++) {
                            std::vector<Real> innermost;
                            for (long g = 0; g < v[0][f].size(); g++) {
                                Real sum = 0;
                                double tot_weight = 0;
                                for (long j = 0; j < v.size(); j++) {
                                    sum += v[j][f][g] * loudness_weights[j];
                                    tot_weight += loudness_weights[j];
                                }
                                innermost.push_back(tot_weight == 0 ? 0 : sum/tot_weight);
                            }
                            res.push_back(innermost);
                        }
                    }
                    break;

                default:
                    break;
            }
        }
    } else if (feature_dim == 0) {
        if (v.size() > 0) {
            long num_features = v.size();
            switch (summarization) {
                case EARS_ESSENTIA_SUMMARIZATION_FIRST:
                    for (long f = 0; f < num_features; f++)
                        res.push_back(v[f].size() > 0 ? v[f][0] : 0);
                    break;
                    
                case EARS_ESSENTIA_SUMMARIZATION_LAST:
                    for (long f = 0; f < num_features; f++)
                        res.push_back(v[f].size() > 0 ? v[f][v[f].size()-1] : 0);
                    break;
                    
                case EARS_ESSENTIA_SUMMARIZATION_MIDDLE:
                    for (long f = 0; f < num_features; f++)
                        res.push_back(v[f].size() > 0 ? v[f][v[f].size()/2] : 0);
                    break;
                
                case EARS_ESSENTIA_SUMMARIZATION_MEAN:
                    for (long f = 0; f < num_features; f++) {
                        Real sum = 0;
                        for (long j = 0; j < v[f].size(); j++) {
                            sum += v[f][j];
                        }
                        res.push_back(sum/v[f].size());
                    }
                    break;
                    
                case EARS_ESSENTIA_SUMMARIZATION_LOUDNESSWEIGHTEDMEAN:
                    for (long f = 0; f < num_features; f++) {
                        Real sum = 0;
                        double tot_weight = 0;
                        for (long j = 0; j < v[f].size(); j++) {
                            sum += v[f][j] * loudness_weights[j];
                            tot_weight += loudness_weights[j];
                        }
                        res.push_back(tot_weight == 0 ? 0 : sum/tot_weight);
                    }
                    break;
                    
                default:
                    break;

            }
        }
    }
    return res;
}
*/

t_ears_err ears_essentia_fill_buffer_from_vector(t_object *x, t_buffer_obj *buf, std::vector<std::vector<std::vector<Real>>> values, double hop_size_samps, long dur_samps, long interp_mode, double audiosr, t_ears_spectralbuf_metadata *specmetadata)
{
    t_ears_err err = EARS_ERR_NONE;
    long innerdim1 = values.size() > 0 ? values[0].size() : 0;
    long innerdim2 = values.size() > 0 && values[0].size() > 0 ? values[0][0].size() : 0;
    long numchannels = values.size() > 0 ? innerdim1 * innerdim2 : 1;

    if (interp_mode == EARS_ESSENTIA_BUFFERINTERPMODE_DONT) {
        double new_sr = audiosr * ((values.size()*1.)/dur_samps);
        ears_buffer_set_size_and_numchannels(x, buf, values.size(), numchannels);
        ears_buffer_set_sr(x, buf, new_sr);
        float *sample = buffer_locksamples(buf);
        if (!sample) {
            err = EARS_ERR_CANT_READ;
            object_error((t_object *)x, EARS_ERROR_BUF_CANT_READ);
        } else {
            t_atom_long    channelcount = buffer_getchannelcount(buf);        // number of floats in a frame
            t_atom_long    framecount   = buffer_getframecount(buf);            // number of floats long the buffer is for a single channel
            
            for (long i = 0; i < framecount; i++) {
                for (long j = 0; j < innerdim1; j++) {
                    for (long k = 0; k < innerdim2; k++) {
                        sample[i*channelcount + innerdim2 * j + k] = values[i][j][k];
                    }
                }
            }
//                for (long c = 0; c < channelcount; c++) {
//                    sample[i*channelcount + c] = values[i][c];
//                }
            
            buffer_setdirty(buf);
            buffer_unlocksamples(buf);
        }
        
        t_ears_spectralbuf_metadata data;
        if (specmetadata) {
            ears_spectralbuf_metadata_fill(&data, audiosr, specmetadata->binsize, specmetadata->offset, specmetadata->frequnit, specmetadata->type, specmetadata->bins, false);
        } else {
            ears_spectralbuf_metadata_fill(&data, audiosr, 0, 0, EARS_FREQUNIT_UNKNOWN, gensym("timeseries"), NULL, false);
        }
        ears_spectralbuf_metadata_set(x, buf, &data);
        
    } else {
        
        if (ears_buffer_is_spectral(x, buf))
            ears_spectralbuf_metadata_remove(x, buf); // not spectral
        
        ears_buffer_set_size_and_numchannels(x, buf, dur_samps, numchannels);
        float *sample = buffer_locksamples(buf);
        
        if (!sample) {
            err = EARS_ERR_CANT_READ;
            object_error((t_object *)x, EARS_ERROR_BUF_CANT_READ);
        } else {
            
            t_atom_long    channelcount = buffer_getchannelcount(buf);        // number of floats in a frame
            t_atom_long    framecount   = buffer_getframecount(buf);            // number of floats long the buffer is for a single channel
            
            if (framecount > 0) {
                for (long j = 0; j < innerdim1; j++) {
                    for (long k = 0; k < innerdim2; k++) {
                        long p = 0;
                        long c = innerdim2 * j + k;
                        double next_hop_position = hop_size_samps;
                        if (interp_mode == EARS_ESSENTIA_BUFFERINTERPMODE_LOWEST) {
                            for (long i = 0; i < framecount; i++) {
                                while (i > next_hop_position) {
                                    next_hop_position += hop_size_samps;
                                    if (p < values.size()-1)
                                        p++;
                                }
                                sample[i*channelcount + c] = values[p][j][k];
                            }
                        } else if (interp_mode == EARS_ESSENTIA_BUFFERINTERPMODE_LINEAR) { // linear
                            bool lastp = (p == values.size() - 1);
                            for (long i = 0; i < framecount; i++) {
                                while (i > next_hop_position) {
                                    next_hop_position += hop_size_samps;
                                    if (p < values.size()-1) {
                                        p++;
                                        lastp = (p == values.size() - 1);
                                    }
                                }
                                sample[i*channelcount + c] = (lastp ? values[p][j][k] : rescale(i, next_hop_position - hop_size_samps, next_hop_position, values[p][j][k], values[p+1][j][k]));
                            }
                        } else {
                            object_error(x, "Unimplemented interpolation mode.");
                        }
                    }
                    
                }
                buffer_setdirty(buf);
            }
            
            buffer_unlocksamples(buf);
        }
    }
    return err;
        
}


t_ears_err ears_essentia_fill_buffer_from_samples(t_object *x, t_buffer_obj *buf, std::vector<Real> values)
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

// channels then samples
t_ears_err ears_essentia_fill_buffer_from_samples(t_object *x, t_buffer_obj *buf, std::vector<std::vector<Real>> values)
{
    t_ears_err err = EARS_ERR_NONE;
    long numchannels = values.size() > 0 ? values.size() : 1;
    ears_buffer_set_size_and_numchannels(x, buf, values.size(), numchannels);
    float *sample = buffer_locksamples(buf);
    
    if (!sample) {
        err = EARS_ERR_CANT_READ;
        object_error((t_object *)x, EARS_ERROR_BUF_CANT_READ);
    } else {
        
        t_atom_long    channelcount = buffer_getchannelcount(buf);        // number of floats in a frame
        t_atom_long    framecount   = buffer_getframecount(buf);            // number of floats long the buffer is for a single channel
        
        for (long c = 0; c < channelcount; c++) {
            for (long i = 0; i < framecount; i++)
                sample[i] = values[c][i];
            buffer_setdirty(buf);
        }
        
        buffer_unlocksamples(buf);
    }
    return err;
}

double ears_essentia_handle_units(double value, t_buffer_obj *buf, t_ears_essentia_extractor *extr, long output_num)
{
    if (extr->feature == EARS_FEATURE_LOGATTACKTIME) {
        value = pow(10, value);
    }

    if (extr->essentia_output_timeunit[output_num] != EARS_TIMEUNIT_UNKNOWN) {
        if (extr->essentia_output_timeunit[output_num] != extr->output_timeunit) {
            value = ears_convert_timeunit(value, buf, extr->essentia_output_timeunit[output_num], extr->output_timeunit);
        }
    } else if (extr->essentia_output_ampunit[output_num] != EARS_AMPUNIT_UNKNOWN) {
        if (extr->essentia_output_ampunit[output_num] != extr->output_ampunit) {
            value = ears_convert_ampunit(value, buf, extr->essentia_output_ampunit[output_num], extr->output_ampunit);
        }
    } else if (extr->essentia_output_frequnit[output_num] != EARS_FREQUNIT_UNKNOWN) {
        if (extr->essentia_output_frequnit[output_num] != extr->output_frequnit) {
            value = ears_convert_frequnit(value, buf, extr->essentia_output_frequnit[output_num], extr->output_frequnit);
        }
    } else if (extr->essentia_output_pitchunit[output_num] != EARS_PITCHUNIT_UNKNOWN) {
        if (extr->essentia_output_pitchunit[output_num] != extr->output_pitchunit) {
            value = ears_convert_pitchunit(value, buf, extr->essentia_output_pitchunit[output_num], extr->output_pitchunit);
        }
    }
    
    if (extr->feature == EARS_FEATURE_LOGATTACKTIME) {
        value = log10(value);
    }

    return value;
}

std::vector<Real> ears_essentia_handle_units(std::vector<Real> vec, t_buffer_obj *buf, t_ears_essentia_extractor *extr, long output_num)
{
    
    if (extr->essentia_output_timeunit[output_num] != EARS_TIMEUNIT_UNKNOWN) {
        if (extr->essentia_output_timeunit[output_num] != extr->output_timeunit) {
            std::vector<Real> outvec;
            for (long i = 0; i < vec.size(); i++)
                outvec.push_back(ears_essentia_handle_units(vec[i], buf, extr, output_num));
            return outvec;
        }
    } else if (extr->essentia_output_ampunit[output_num] != EARS_AMPUNIT_UNKNOWN) {
        if (extr->essentia_output_ampunit[output_num] != extr->output_ampunit) {
            std::vector<Real> outvec;
            for (long i = 0; i < vec.size(); i++)
                outvec.push_back(ears_essentia_handle_units(vec[i], buf, extr, output_num));
            return outvec;
        }
    } else if (extr->essentia_output_frequnit[output_num] != EARS_FREQUNIT_UNKNOWN) {
        if (extr->essentia_output_frequnit[output_num] != extr->output_frequnit) {
            std::vector<Real> outvec;
            for (long i = 0; i < vec.size(); i++)
                outvec.push_back(ears_essentia_handle_units(vec[i], buf, extr, output_num));
            return outvec;
        }
    } else if (extr->essentia_output_pitchunit[output_num] != EARS_PITCHUNIT_UNKNOWN) {
        if (extr->essentia_output_pitchunit[output_num] != extr->output_pitchunit) {
            std::vector<Real> outvec;
            for (long i = 0; i < vec.size(); i++)
                outvec.push_back(ears_essentia_handle_units(vec[i], buf, extr, output_num));
            return outvec;
        }
    }
    return vec;
}

std::vector<std::vector<Real>> ears_essentia_handle_units(std::vector<std::vector<Real>> vec, t_buffer_obj *buf, t_ears_essentia_extractor *extr, long output_num)
{
    if (extr->essentia_output_timeunit[output_num] != EARS_TIMEUNIT_UNKNOWN) {
        if (extr->essentia_output_timeunit[output_num] != extr->output_timeunit) {
            std::vector<std::vector<Real>> outvec;
            for (long i = 0; i < vec.size(); i++) {
                std::vector<Real> inner;
                for (long j = 0; j < vec[i].size(); j++)
                    inner.push_back(ears_essentia_handle_units(vec[i][j], buf, extr, output_num));
                outvec.push_back(inner);
            }
            return outvec;
        }
    } else if (extr->essentia_output_ampunit[output_num] != EARS_AMPUNIT_UNKNOWN) {
        if (extr->essentia_output_ampunit[output_num] != extr->output_ampunit) {
            std::vector<std::vector<Real>> outvec;
            for (long i = 0; i < vec.size(); i++) {
                std::vector<Real> inner;
                for (long j = 0; j < vec[i].size(); j++)
                    inner.push_back(ears_essentia_handle_units(vec[i][j], buf, extr, output_num));
                outvec.push_back(inner);
            }
            return outvec;
        }
    } else if (extr->essentia_output_frequnit[output_num] != EARS_FREQUNIT_UNKNOWN) {
        if (extr->essentia_output_frequnit[output_num] != extr->output_frequnit) {
            std::vector<std::vector<Real>> outvec;
            for (long i = 0; i < vec.size(); i++) {
                std::vector<Real> inner;
                for (long j = 0; j < vec[i].size(); j++)
                    inner.push_back(ears_essentia_handle_units(vec[i][j], buf, extr, output_num));
                outvec.push_back(inner);
            }
            return outvec;
        }
    } else if (extr->essentia_output_pitchunit[output_num] != EARS_PITCHUNIT_UNKNOWN) {
        if (extr->essentia_output_pitchunit[output_num] != extr->output_pitchunit) {
            std::vector<std::vector<Real>> outvec;
            for (long i = 0; i < vec.size(); i++) {
                std::vector<Real> inner;
                for (long j = 0; j < vec[i].size(); j++)
                    inner.push_back(ears_essentia_handle_units(vec[i][j], buf, extr, output_num));
                outvec.push_back(inner);
            }
            return outvec;
        }
    }
    return vec;
}

std::vector<std::vector<std::vector<Real>>> ears_essentia_handle_units(std::vector<std::vector<std::vector<Real>>> vec, t_buffer_obj *buf, t_ears_essentia_extractor *extr, long output_num)
{
    if (extr->essentia_output_timeunit[output_num] != EARS_TIMEUNIT_UNKNOWN) {
        if (extr->essentia_output_timeunit[output_num] != extr->output_timeunit) {
            std::vector<std::vector<std::vector<Real>>> outvec;
            for (long i = 0; i < vec.size(); i++) {
                std::vector<std::vector<Real>> inner;
                for (long j = 0; j < vec[i].size(); j++) {
                    std::vector<Real> innermost;
                    for (long k = 0; k < vec[i].size(); k++)
                        innermost.push_back(ears_essentia_handle_units(vec[i][j][k], buf, extr, output_num));
                    inner.push_back(innermost);
                }
                outvec.push_back(inner);
            }
            return outvec;
        }
    } else if (extr->essentia_output_ampunit[output_num] != EARS_AMPUNIT_UNKNOWN) {
        if (extr->essentia_output_ampunit[output_num] != extr->output_ampunit) {
            std::vector<std::vector<std::vector<Real>>> outvec;
            for (long i = 0; i < vec.size(); i++) {
                std::vector<std::vector<Real>> inner;
                for (long j = 0; j < vec[i].size(); j++) {
                    std::vector<Real> innermost;
                    for (long k = 0; k < vec[i].size(); k++)
                        innermost.push_back(ears_essentia_handle_units(vec[i][j][k], buf, extr, output_num));
                    inner.push_back(innermost);
                }
                outvec.push_back(inner);
            }
            return outvec;
        }
    } else if (extr->essentia_output_frequnit[output_num] != EARS_FREQUNIT_UNKNOWN) {
        if (extr->essentia_output_frequnit[output_num] != extr->output_frequnit) {
            std::vector<std::vector<std::vector<Real>>> outvec;
            for (long i = 0; i < vec.size(); i++) {
                std::vector<std::vector<Real>> inner;
                for (long j = 0; j < vec[i].size(); j++) {
                    std::vector<Real> innermost;
                    for (long k = 0; k < vec[i].size(); k++)
                        innermost.push_back(ears_essentia_handle_units(vec[i][j][k], buf, extr, output_num));
                    inner.push_back(innermost);
                }
                outvec.push_back(inner);
            }
            return outvec;
        }
    } else if (extr->essentia_output_pitchunit[output_num] != EARS_PITCHUNIT_UNKNOWN) {
        if (extr->essentia_output_pitchunit[output_num] != extr->output_pitchunit) {
            std::vector<std::vector<std::vector<Real>>> outvec;
            for (long i = 0; i < vec.size(); i++) {
                std::vector<std::vector<Real>> inner;
                for (long j = 0; j < vec[i].size(); j++) {
                    std::vector<Real> innermost;
                    for (long k = 0; k < vec[i].size(); k++)
                        innermost.push_back(ears_essentia_handle_units(vec[i][j][k], buf, extr, output_num));
                    inner.push_back(innermost);
                }
                outvec.push_back(inner);
            }
            return outvec;
        }
    }
    return vec;
}

t_symbol *symbol_from_ascii_vector_wrapped(std::vector<std::vector<Real>> ascii)
{
    char temp[ascii.size()+1];
    for (long i = 0; i < ascii.size(); i++)
        temp[i] = ascii[i].size() > 0 ? (unsigned char)ascii[i][0] : 0;
    temp[ascii.size()] = 0;
    return gensym(temp);
}

t_symbol *symbol_from_ascii_vector(std::vector<Real> ascii)
{
    char temp[ascii.size()+1];
    for (long i = 0; i < ascii.size(); i++)
        temp[i] = (unsigned char)ascii[i];
    temp[ascii.size()] = 0;
    return gensym(temp);
}

std::vector<Real> string_to_ascii_vector(std::string s)
{
    std::vector<Real> v;
    for (long i = 0; i < s.size(); i++)
        v.push_back((int)s.c_str()[i]);
    return v;
}

std::vector<std::vector<Real>> string_to_ascii_vector_wrapped(std::string s)
{
    std::vector<std::vector<Real>> v;
    for (long i = 0; i < s.size(); i++) {
        std::vector<Real> w;
        w.push_back((int)s.c_str()[i]);
        v.push_back(w);
    }
    return v;
}

t_ears_err ears_essentia_extractors_library_compute(t_earsbufobj *e_ob, t_buffer_obj *buf, t_ears_essentia_extractors_library *lib, t_ears_essentia_analysis_params *params, long buffer_output_interpolation_mode)
{
    t_ears_err err = EARS_ERR_NONE;
    t_object *ob = (t_object *)e_ob;
    double sr = ears_buffer_get_sr(ob, buf);
    std::vector<Real> data = ears_buffer_get_sample_vector_mono(ob, buf);
    std::vector<Real> dataeql;
    std::vector<Real> framedata;
    std::vector<Real> wframedata;
    std::vector<Real> specdata;
    std::vector<Real> envdata;
    std::vector<Real> envdata_cm;
    std::vector<Real> specdata_cm;
    std::vector<Real> peaksdatafreqs;
    std::vector<Real> peaksdatamags;
    std::vector<std::vector<Real>> hpcpdata;
    std::vector<Real> hpcpdata_frame;

    Real frameloudness, framerms;
    Real fzerodata;
    Real fzeroconfidencedata;
    
    try {
        lib->alg_FrameCutter->input("signal").set(data);
        lib->alg_FrameCutter->output("frame").set(framedata);
        lib->alg_Windower->input("frame").set(framedata);
        lib->alg_Windower->output("frame").set(wframedata);
        lib->alg_Spectrum->input("frame").set(wframedata);
        lib->alg_Spectrum->output("spectrum").set(specdata);
        
        lib->alg_Envelope->input("signal").set(data);
        lib->alg_Envelope->output("signal").set(envdata);

        lib->alg_EnvelopeCentralMoments->input("array").set(envdata);
        lib->alg_EnvelopeCentralMoments->output("centralMoments").set(envdata_cm);

        lib->alg_EnvelopeCentralMoments->configure("range", ears_convert_timeunit((data.size()-1)/sr, buf, EARS_TIMEUNIT_SECONDS, (e_ears_timeunit)e_ob->l_timeunit));
        
        lib->alg_SpectrumCentralMoments->input("array").set(specdata);
        lib->alg_SpectrumCentralMoments->output("centralMoments").set(specdata_cm);
        
        lib->alg_Loudness->input("signal").set(wframedata);
        lib->alg_Loudness->output("loudness").set(frameloudness);

        if (sr == 8000 || sr == 32000 || sr == 44100 || sr == 48000) {
            lib->alg_EqualLoudness->input("signal").set(data);
            lib->alg_EqualLoudness->output("signal").set(dataeql);
        } else {
            // EqualLoudness algorithm won't support those frequencies
            dataeql = data;
        }

        lib->alg_RMS->input("array").set(wframedata);
        lib->alg_RMS->output("rms").set(framerms);

        lib->alg_SpectralPeaks->input("spectrum").set(specdata);
        lib->alg_SpectralPeaks->output("frequencies").set(peaksdatafreqs);
        lib->alg_SpectralPeaks->output("magnitudes").set(peaksdatamags);

        lib->alg_HPCP->input("frequencies").set(peaksdatafreqs);
        lib->alg_HPCP->input("magnitudes").set(peaksdatamags);
        lib->alg_HPCP->output("hpcp").set(hpcpdata_frame);

        lib->alg_YinFFT->input("spectrum").set(specdata);
        lib->alg_YinFFT->output("pitch").set(fzerodata);
        lib->alg_YinFFT->output("pitchConfidence").set(fzeroconfidencedata);

    } catch (essentia::EssentiaException e) {  object_error(ob, e.what());  err = EARS_ERR_ESSENTIA;   }
    
    for (long i = 0; i < lib->num_extractors; i++) {
        e_ears_essentia_temporalmode temporalmode = lib->extractors[i].temporalmode;
        e_ears_essentia_extractor_input_type inputtype = lib->extractors[i].input_type;
        t_ears_spectralbuf_metadata *specmetadata = lib->extractors[i].has_spec_metadata ? &lib->extractors[i].specdata : NULL;

        for (long o = 0; o < EARS_ESSENTIA_EXTRACTOR_MAX_OUTPUTS; o++)
            if (lib->extractors[i].result)
                llll_free(lib->extractors[i].result[o]);
        for (long o = 0; o < lib->extractors[i].essentia_num_outputs; o++)
            lib->extractors[i].result[o] = llll_get();
        
        try {
            
            switch (lib->extractors[i].feature) {
                case EARS_FEATURE_TEMPORALDECREASE:
                case EARS_FEATURE_TEMPORALRAWMOMENTS:
                    lib->extractors[i].algorithm->configure("range", (data.size()-1)/sr);
                    break;
                    
                default:
                    break;
            }
            
            int res_int[EARS_ESSENTIA_EXTRACTOR_MAX_OUTPUTS];
            Real res_real[EARS_ESSENTIA_EXTRACTOR_MAX_OUTPUTS];
            std::string res_string[EARS_ESSENTIA_EXTRACTOR_MAX_OUTPUTS];
            std::vector<Real> res_vector[EARS_ESSENTIA_EXTRACTOR_MAX_OUTPUTS];
            std::vector<std::vector<Real>> res_vector_vector[EARS_ESSENTIA_EXTRACTOR_MAX_OUTPUTS];
            std::vector<std::string> res_stringvector[EARS_ESSENTIA_EXTRACTOR_MAX_OUTPUTS];
            
            for (long o = 0; o < lib->extractors[i].essentia_num_outputs; o++) {
                switch (lib->extractors[i].essentia_output_type[o]) {
                    case 'f':
                        lib->extractors[i].algorithm->output(lib->extractors[i].essentia_output_label[o]).set(res_real[o]);
                        break;
                    case 'i':
                        lib->extractors[i].algorithm->output(lib->extractors[i].essentia_output_label[o]).set(res_int[o]);
                        break;
                    case 'v':
                    case 's':
                        lib->extractors[i].algorithm->output(lib->extractors[i].essentia_output_label[o]).set(res_vector[o]);
                        break;
                    case 'w':
                        lib->extractors[i].algorithm->output(lib->extractors[i].essentia_output_label[o]).set(res_vector_vector[o]);
                        break;
                    case 'b':
                        lib->extractors[i].algorithm->output(lib->extractors[i].essentia_output_label[o]).set(res_stringvector[o]);
                        break;
                    case 'c':
                        lib->extractors[i].algorithm->output(lib->extractors[i].essentia_output_label[o]).set(res_string[o]);
                        break;
                    default:
                        break;
                }
            }

            bool need_framewise_iteration = (temporalmode == EARS_ESSENTIA_TEMPORALMODE_TIMESERIES ||
                                             (temporalmode == EARS_ESSENTIA_TEMPORALMODE_BUFFER && lib->extractors[i].essentia_output_type[0] != 's') ||
                                             inputtype == EARS_ESSENTIA_EXTRACTOR_INPUT_FRAME ||
                                             inputtype == EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRUM ||
                                             inputtype == EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRUMCENTRALMOMENTS ||
                                             inputtype == EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRALPEAKS ||
                                             inputtype == EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRALPEAKSANDFZERO ||
                                             inputtype == EARS_ESSENTIA_EXTRACTOR_INPUT_LOUDNESS ||
                                             inputtype == EARS_ESSENTIA_EXTRACTOR_INPUT_PITCHCLASSPROFILE ||
                                             inputtype == EARS_ESSENTIA_EXTRACTOR_INPUT_PITCHCLASSPROFILEBATCH );
            
            // resetting algorithms
            lib->alg_FrameCutter->reset();
            lib->alg_Windower->reset();
            lib->alg_Spectrum->reset();
            lib->alg_Car2pol->reset();
            lib->alg_Loudness->reset();
            lib->alg_EqualLoudness->reset();
            lib->alg_RMS->reset();
            lib->alg_EnvelopeCentralMoments->reset();
            lib->alg_SpectrumCentralMoments->reset();
            lib->alg_SpectralPeaks->reset();
            lib->alg_HPCP->reset();
            lib->alg_YinFFT->reset();
            lib->extractors[i].algorithm->reset();
            hpcpdata.clear();
            

            if (need_framewise_iteration) {
                //// FRAMEWISE ITERATION
                std::vector<std::vector<std::vector<Real>>> frame_features[EARS_ESSENTIA_EXTRACTOR_MAX_OUTPUTS];
                std::vector<Real> loudness_weights, rms_weights;
                bool need_loudness = ((temporalmode == EARS_ESSENTIA_TEMPORALMODE_WHOLE && params->summarization == EARS_ESSENTIA_SUMMARIZATION_MEAN && params->summarizationweight == EARS_ESSENTIA_SUMMARIZATIONWEIGHT_LOUDNESS) || (inputtype == EARS_ESSENTIA_EXTRACTOR_INPUT_LOUDNESS));
                bool need_rms = ((temporalmode == EARS_ESSENTIA_TEMPORALMODE_WHOLE && params->summarization == EARS_ESSENTIA_SUMMARIZATION_MEAN && params->summarizationweight == EARS_ESSENTIA_SUMMARIZATIONWEIGHT_RMS) || (inputtype == EARS_ESSENTIA_EXTRACTOR_INPUT_LOUDNESS));

                lib->alg_FrameCutter->reset();

                while (true) {
                    lib->alg_FrameCutter->compute();
                    if (!framedata.size()) {
                        if (inputtype == EARS_ESSENTIA_EXTRACTOR_INPUT_PITCHCLASSPROFILEBATCH) {
                            lib->extractors[i].algorithm->input(lib->extractors[i].essentia_input_label[0]).set(hpcpdata);
                            lib->extractors[i].algorithm->compute();
                            goto finally_compute;
                        }
                        break;
                    }
                    
                    switch (inputtype) {
                        case EARS_ESSENTIA_EXTRACTOR_INPUT_FRAME:
                            lib->alg_Windower->compute();
                            lib->extractors[i].algorithm->input(lib->extractors[i].essentia_input_label[0]).set(wframedata);
                            break;
                            
                        case EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRUM:
                        case EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRUMCENTRALMOMENTS:
                            lib->alg_Windower->compute();
                            lib->alg_Spectrum->compute();
                            if (inputtype == EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRUMCENTRALMOMENTS) {
                                lib->alg_SpectrumCentralMoments->compute();
                                lib->extractors[i].algorithm->input(lib->extractors[i].essentia_input_label[0]).set(specdata_cm);
                            } else {
                                lib->extractors[i].algorithm->input(lib->extractors[i].essentia_input_label[0]).set(specdata);
                            }
                            break;
                            
                        case EARS_ESSENTIA_EXTRACTOR_INPUT_ENVELOPE:
                        case EARS_ESSENTIA_EXTRACTOR_INPUT_ENVELOPECENTRALMOMENTS:
                            lib->alg_Envelope->compute();
                            if (inputtype == EARS_ESSENTIA_EXTRACTOR_INPUT_ENVELOPECENTRALMOMENTS) {
                                lib->alg_EnvelopeCentralMoments->compute();
                                lib->extractors[i].algorithm->input(lib->extractors[i].essentia_input_label[0]).set(envdata_cm);
                            } else {
                                lib->extractors[i].algorithm->input(lib->extractors[i].essentia_input_label[0]).set(envdata);
                            }
                            break;

                        case EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRALPEAKS:
                            lib->alg_Windower->compute();
                            lib->alg_Spectrum->compute();
                            lib->alg_SpectralPeaks->compute();
                            lib->extractors[i].algorithm->input(lib->extractors[i].essentia_input_label[0]).set(peaksdatafreqs);
                            lib->extractors[i].algorithm->input(lib->extractors[i].essentia_input_label[1]).set(peaksdatamags);
                            break;
 
                        case EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRALPEAKSANDFZERO:
                            lib->alg_Windower->compute();
                            lib->alg_Spectrum->compute();
                            lib->alg_SpectralPeaks->compute();
                            lib->alg_YinFFT->compute();
                            lib->extractors[i].algorithm->input(lib->extractors[i].essentia_input_label[0]).set(peaksdatafreqs);
                            lib->extractors[i].algorithm->input(lib->extractors[i].essentia_input_label[1]).set(peaksdatamags);
                            lib->extractors[i].algorithm->input(lib->extractors[i].essentia_input_label[2]).set(fzerodata);
                            break;

                        case EARS_ESSENTIA_EXTRACTOR_INPUT_PITCHCLASSPROFILE:
                            lib->alg_Windower->compute();
                            lib->alg_Spectrum->compute();
                            lib->alg_SpectralPeaks->compute();
                            lib->alg_HPCP->compute();
                            lib->extractors[i].algorithm->input(lib->extractors[i].essentia_input_label[0]).set(hpcpdata_frame);
                            break;

                        case EARS_ESSENTIA_EXTRACTOR_INPUT_PITCHCLASSPROFILEBATCH:
                            lib->alg_Windower->compute();
                            lib->alg_Spectrum->compute();
                            lib->alg_SpectralPeaks->compute();
                            lib->alg_HPCP->compute();
                            hpcpdata.push_back(hpcpdata_frame);
                            break;
                            
                        case EARS_ESSENTIA_EXTRACTOR_INPUT_LOUDNESS:
                            lib->extractors[i].algorithm->input(lib->extractors[i].essentia_input_label[0]).set(frameloudness);
                            break;
                            
                        case EARS_ESSENTIA_EXTRACTOR_INPUT_AUDIO:
                        default:
                            lib->extractors[i].algorithm->input(lib->extractors[i].essentia_input_label[0]).set(framedata);
                            break;
                    }
                    
                    if (need_loudness) {
                        lib->alg_Loudness->compute();
                        loudness_weights.push_back(frameloudness);
                    }

                    if (need_rms) {
                        lib->alg_RMS->compute();
                        rms_weights.push_back(framerms);
                    }
                    
                    if (inputtype != EARS_ESSENTIA_EXTRACTOR_INPUT_PITCHCLASSPROFILEBATCH) {
                        lib->extractors[i].algorithm->compute();
                        
                    finally_compute:
                        for (long o = 0; o < lib->extractors[i].essentia_num_outputs; o++) {
                            if (lib->extractors[i].output_type[o] == 'x')
                                continue;
                            
                            switch (lib->extractors[i].essentia_output_type[o]) {
                                case 'f':
                                {
                                    std::vector<Real> temp;
                                    std::vector<std::vector<Real>> vtemp;
                                    temp.push_back(res_real[o]);
                                    vtemp.push_back(temp);
                                    frame_features[o].push_back(vtemp);
                                }
                                    break;
                                case 'i':
                                {
                                    std::vector<Real> temp;
                                    std::vector<std::vector<Real>> vtemp;
                                    temp.push_back(res_int[o]);
                                    vtemp.push_back(temp);
                                    frame_features[o].push_back(vtemp);
                                }
                                    break;
                                case 'v':
                                case 's':
                                {
                                    std::vector<std::vector<Real>> vtemp;
                                    vtemp.push_back(res_vector[o]);
                                    frame_features[o].push_back(vtemp);
                                }
                                    break;
                                case 'w':
                                    frame_features[o].push_back(res_vector_vector[o]);
                                    break;
                                case 'c':
                                {
                                    frame_features[o].push_back(string_to_ascii_vector_wrapped(res_string[o]));
                                }
                                    break;
                                case 'b':
                                    if (inputtype == EARS_ESSENTIA_EXTRACTOR_INPUT_PITCHCLASSPROFILEBATCH) {
                                        for (long i = 0; i < res_stringvector[o].size(); i++)
                                            frame_features[o].push_back(string_to_ascii_vector_wrapped(res_stringvector[o][i]));
                                    } else {
                                        std::vector<std::vector<Real>> vtemp;
                                        for (long i = 0; i < res_stringvector[o].size(); i++)
                                            vtemp.push_back(string_to_ascii_vector(res_stringvector[o][i]));
                                        frame_features[o].push_back(vtemp);
                                    }
                                    break;
                                default:
                                    break;
                            }
                        }
                        
                        if (inputtype == EARS_ESSENTIA_EXTRACTOR_INPUT_PITCHCLASSPROFILEBATCH) {
                            break;
                        }
                    }
                }
                
                for (long o = 0; o < lib->extractors[i].essentia_num_outputs; o++) {
                    switch (temporalmode) {
                        case EARS_ESSENTIA_TEMPORALMODE_TIMESERIES:
                            for (long fr = 0; fr < frame_features[o].size(); fr++) {
                                if (lib->extractors[i].output_type[o] == 'b' || lib->extractors[i].output_type[o] == 'c') {
                                    llll_appendsym(lib->extractors[i].result[o], symbol_from_ascii_vector_wrapped(frame_features[o][fr]));
                                } else {
                                    t_llll *inner = llll_get();
                                    for (long f = 0; f < frame_features[o][fr].size(); f++) {
                                        t_llll *innermost = llll_get();
                                        for (long g = 0; g < frame_features[o][fr][f].size(); g++) {
                                            double val = ears_essentia_handle_units(frame_features[o][fr][f][g], buf, &lib->extractors[i], o);
                                            if (lib->extractors[i].output_type[o] == 'i')
                                                llll_appendlong(innermost, val);
                                            else
                                                llll_appenddouble(innermost, val);
                                        }
                                        llll_appendllll(inner, innermost);
                                    }
                                    llll_appendllll(lib->extractors[i].result[o], inner);
                                }
                            }
                            break;
                            
                        case EARS_ESSENTIA_TEMPORALMODE_BUFFER:
                            ears_essentia_fill_buffer_from_vector(ob, lib->extractors[i].result_buf[o], ears_essentia_handle_units(frame_features[o], buf, &lib->extractors[i], o), params->hopsize_samps, params->duration_samps, buffer_output_interpolation_mode, sr, specmetadata);
                            break;
                            
                        case EARS_ESSENTIA_TEMPORALMODE_WHOLE:
                        {
                            e_ears_essentia_summarization summ = params->summarization;
                            if (summ == EARS_ESSENTIA_SUMMARIZATION_MEAN && (lib->extractors[i].output_type[o] == 'b' || lib->extractors[i].output_type[o] == 'c'))
                                summ = EARS_ESSENTIA_SUMMARIZATION_MODE; // for symbols, MEAN -> MODE
                            std::vector<std::vector<Real>> avg = vector_of_vector_average(frame_features[o], summ, params->summarizationweight, rms_weights, loudness_weights);
                            if (lib->extractors[i].output_type[o] == 'b' || lib->extractors[i].output_type[o] == 'c') {
                                llll_appendsym(lib->extractors[i].result[o], symbol_from_ascii_vector_wrapped(avg));
                            } else {
                                for (long f = 0; f < avg.size(); f++) {
                                    t_llll *innermost = llll_get();
                                    for (long g = 0; g < avg[f].size(); g++) {
                                        double val = ears_essentia_handle_units(avg[f][g], buf, &lib->extractors[i], o);
                                        if (lib->extractors[i].output_type[o] == 'i')
                                            llll_appendlong(innermost, val);
                                        else
                                            llll_appenddouble(innermost, val);
                                    }
                                    llll_appendllll(lib->extractors[i].result[o], innermost);
                                }
                            }
                        }
                        default:
                            break;
                    }
                }
                
                
            } else {
                ////// GLOBAL COMPUTATION
                std::vector<std::vector<Real>> features[EARS_ESSENTIA_EXTRACTOR_MAX_OUTPUTS];
                
                switch (inputtype) {
                    case EARS_ESSENTIA_EXTRACTOR_INPUT_ENVELOPE:
                    case EARS_ESSENTIA_EXTRACTOR_INPUT_ENVELOPECENTRALMOMENTS:
                        lib->alg_Envelope->compute();
                        if (inputtype == EARS_ESSENTIA_EXTRACTOR_INPUT_ENVELOPECENTRALMOMENTS) {
                            lib->alg_EnvelopeCentralMoments->compute();
                            lib->extractors[i].algorithm->input(lib->extractors[i].essentia_input_label[0]).set(envdata_cm);
                        } else {
                            lib->extractors[i].algorithm->input(lib->extractors[i].essentia_input_label[0]).set(envdata);
                        }
                        break;
                        
                    case EARS_ESSENTIA_EXTRACTOR_INPUT_AUDIO:
                    default:
                        lib->extractors[i].algorithm->input(lib->extractors[i].essentia_input_label[0]).set(data);
                        break;
                }
                
                lib->extractors[i].algorithm->compute();
                
                for (long o = 0; o < lib->extractors[i].essentia_num_outputs; o++) {
                    if (lib->extractors[i].output_type[o] == 'x')
                        continue;
                    switch (lib->extractors[i].essentia_output_type[o]) {
                        case 'f':
                        {
                            std::vector<Real> temp;
                            temp.push_back(res_real[o]);
                            features[o].push_back(temp);
                        }
                            break;
                        case 'i':
                        {
                            std::vector<Real> temp;
                            temp.push_back(res_int[o]);
                            features[o].push_back(temp);
                        }
                            break;
                        case 'v':
                        case 's':
                            features[o].push_back(res_vector[o]);
                            break;
                        case 'w':
                            features[o] = res_vector_vector[o];
                            break;
                        default:
                            break;
                    }
                    
                    switch (temporalmode) {
                        case EARS_ESSENTIA_TEMPORALMODE_WHOLE:
                            for (long f = 0; f < features[o].size(); f++) {
                                t_llll *inner = llll_get();
                                for (long g = 0; g < features[o][f].size(); g++) {
                                    double val = ears_essentia_handle_units(features[o][f][g], buf, &lib->extractors[i], o);
                                    if (lib->extractors[i].output_type[o] == 'i')
                                        llll_appendlong(inner, val);
                                    else
                                        llll_appenddouble(inner, val);
                                }
                                llll_appendllll(lib->extractors[i].result[o], inner);
                            }
                            break;
                            
                        case EARS_ESSENTIA_TEMPORALMODE_BUFFER:
                            ears_essentia_fill_buffer_from_samples(ob, lib->extractors[i].result_buf[o],
                                                                   ears_essentia_handle_units(features[o], buf, &lib->extractors[i], o));
                            break;
                            
                        default:
                            break;
                    }
                }
                
            }
            
            
            // flattening lllls if needed
            long shape[3];
            for (long o = 0; o < lib->extractors[i].essentia_num_outputs; o++) {
                shape[0] = lib->extractors[i].result[o]->l_size;
                shape[1] = lib->extractors[i].result[o]->l_head && hatom_gettype(&lib->extractors[i].result[o]->l_head->l_hatom) == H_LLLL ? hatom_getllll(&lib->extractors[i].result[o]->l_head->l_hatom)->l_size : 0;
                shape[2] = shape[1] > 0 && hatom_getllll(&lib->extractors[i].result[o]->l_head->l_hatom)->l_head && hatom_gettype(&hatom_getllll(&lib->extractors[i].result[o]->l_head->l_hatom)->l_head->l_hatom) == H_LLLL ? hatom_getllll(&hatom_getllll(&lib->extractors[i].result[o]->l_head->l_hatom)->l_head->l_hatom)->l_size : 0;
                
                if (shape[2] == 1)
                    llll_flat(lib->extractors[i].result[o], 2, 2);
                if (shape[1] == 1)
                    llll_flat(lib->extractors[i].result[o], 1, 1);
                if (lib->extractors[i].temporalmode == EARS_ESSENTIA_TEMPORALMODE_WHOLE && shape[0] == 1) // otherwise shape[0] are the frames
                    llll_flat(lib->extractors[i].result[o], 1, 1);
//                    llll_splatter(lib->extractors[i].result[o]->l_head, LLLL_FREETHING_DONT);
            }
            
        } catch (essentia::EssentiaException e) {  object_error(ob, e.what());  err =  EARS_ERR_ESSENTIA;   }
    }
    
    return err;
}
