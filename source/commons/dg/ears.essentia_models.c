#include "ears.essentia_models.h"

t_llll *llll_from_vector(std::vector<Real> vec)
{
    t_llll *ll = llll_get();
    for (long i = 0; i < vec.size(); i++) {
        llll_appenddouble(ll, vec[i]);
    }
    return ll;
}

std::vector<Real> vector_from_llll(t_llll *ll)
{
    std::vector<Real> vec;
    for (t_llllelem *el = ll->l_head; el; el = el->l_next)
        vec.push_back(hatom_getdouble(&el->l_hatom));
    return vec;
}


t_ears_err ears_buffer_setchannel(t_object *ob, t_buffer_obj *source, long dest_channel, std::vector<float> &samps)
{
    if (!source)
        return EARS_ERR_NO_BUFFER;
    
    t_ears_err err = EARS_ERR_NONE;
    float *sample = buffer_locksamples(source);
    
    if (!sample) {
        err = EARS_ERR_CANT_READ;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
    } else {
        
        t_atom_long    source_channelcount = buffer_getchannelcount(source);
        t_atom_long    source_framecount   = buffer_getframecount(source);
        
        long lim = MIN(source_framecount, samps.size());
        long i = 0;
        for (i = 0; i < lim; i++)
            sample[i * source_channelcount + dest_channel] = samps[i];
        
        // zeroing out other samps
        for (; i < source_framecount; i++)
            sample[i * source_channelcount + dest_channel] = 0;
        
        buffer_unlocksamples(source);
    }
    
    return err;
}



t_ears_err ears_buffer_sumchannel(t_object *ob, t_buffer_obj *source, long dest_channel, std::vector<float> &samps)
{
    if (!source)
        return EARS_ERR_NO_BUFFER;
    
    t_ears_err err = EARS_ERR_NONE;
    float *sample = buffer_locksamples(source);
    
    if (!sample) {
        err = EARS_ERR_CANT_READ;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
    } else {
        
        t_atom_long    source_channelcount = buffer_getchannelcount(source);
        t_atom_long    source_framecount   = buffer_getframecount(source);
        
        long lim = MIN(source_framecount, samps.size());
        long i = 0;
        for (i = 0; i < lim; i++)
            sample[i * source_channelcount + dest_channel] += samps[i];
        
        buffer_unlocksamples(source);
    }
    
    return err;
}



t_ears_err ears_model_sine_analysis(t_object *ob, std::vector<Real> samples, double sr,
                                    t_llll **frequencies_ll, t_llll **magnitudes_ll, t_llll **phases_ll,
                                    t_ears_essentia_analysis_params *params,
                                    e_ears_angleunit out_angleunit,
                                    e_ears_ampunit out_ampunit,
                                    e_ears_frequnit out_frequnit,
                                    double freqDevOffset,
                                    double freqDevSlope,
                                    double magnitudeThreshold,
                                    double minFrequency,
                                    double maxFrequency,
                                    long maxPeaks,
                                    long maxnSines,
                                    const char *orderBy
                                    )
{
    t_ears_err err = EARS_ERR_NONE;
    
    // we approximate the hopsize to the nearest sample. Otherwise the overlap add gives troubles...
    long hopsize_samps = (long)params->hopsize_samps;
    int framesize_samps = (int)params->framesize_samps;
    
    std::vector<essentia::Real> framedata;
    std::vector<essentia::Real> wframedata;
    std::vector<std::complex<essentia::Real>> fftdata;
    std::vector<essentia::Real> magnitudes, frequencies, phases;

    *frequencies_ll = llll_get();
    *magnitudes_ll = llll_get();
    *phases_ll = llll_get();
    
    essentia::standard::Algorithm *frameCutter, *windower, *fft, *sine;
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
        
        fft = essentia::standard::AlgorithmFactory::create("FFT",
                                                           "size", framesize_samps);
        
        sine = essentia::standard::AlgorithmFactory::create("SineModelAnal",
                                                            "sampleRate", sr,
                                                            "freqDevOffset", freqDevOffset,
                                                            "freqDevSlope", freqDevSlope,
                                                            "magnitudeThreshold", magnitudeThreshold,
                                                            "minFrequency", minFrequency,
                                                            "maxFrequency", maxFrequency,
                                                            "maxPeaks", (int)maxPeaks,
                                                            "maxnSines", (int)maxnSines,
                                                            "orderBy", orderBy);

        frameCutter->input("signal").set(samples);
        frameCutter->output("frame").set(framedata);
        windower->input("frame").set(framedata);
        windower->output("frame").set(wframedata);
        fft->input("frame").set(wframedata);
        fft->output("fft").set(fftdata);
        sine->input("fft").set(fftdata);
        sine->output("frequencies").set(frequencies);
        sine->output("magnitudes").set(magnitudes);
        sine->output("phases").set(phases);
    } catch (essentia::EssentiaException e) {
        object_error(ob, e.what());
        return EARS_ERR_ESSENTIA;
    }
    
    try {
        frameCutter->reset();
        long actual_framecount = 0;
        while (true) {
            frameCutter->compute(); // new frame
            if (!framedata.size()) {
                break;
            }
            
            windower->compute();
            fft->compute();
            sine->compute();
            
            ears_convert_ampunit(magnitudes, NULL, EARS_AMPUNIT_DECIBEL, out_ampunit);
            ears_convert_frequnit(frequencies, NULL, EARS_FREQUNIT_HERTZ, out_frequnit);
            ears_convert_angleunit(phases, NULL, EARS_ANGLEUNIT_RADIANS, out_angleunit);
            
            llll_appendllll(*frequencies_ll, llll_from_vector(frequencies));
            llll_appendllll(*magnitudes_ll, llll_from_vector(magnitudes));
            llll_appendllll(*phases_ll, llll_from_vector(phases));

            actual_framecount++;
        }
    }
    catch (essentia::EssentiaException e)
    {
        object_error(ob, e.what());
    }
    
    delete frameCutter;
    delete windower;
    delete fft;
    delete sine;
    
    return err;
}


t_ears_err ears_model_sine_synthesis(t_object *ob, double sr,
                                    t_llll *frequencies_ll, t_llll *magnitudes_ll, t_llll *phases_ll,
                                    t_buffer_obj *outbuf,
                                    t_ears_essentia_analysis_params *params,
                                    e_ears_angleunit in_angleunit,
                                    e_ears_ampunit in_ampunit,
                                    e_ears_frequnit in_frequnit,
                                    long channelidx
                                    )
{
    t_ears_err err = EARS_ERR_NONE;
    
    // we approximate the hopsize to the nearest sample. Otherwise the overlap add gives troubles...
    long hopsize_samps = (long)params->hopsize_samps;
    int framesize_samps = (int)params->framesize_samps;
    
    std::vector<essentia::Real> magnitudes, frequencies, phases;
    std::vector<std::complex<essentia::Real>> sfftframe;
    std::vector<essentia::Real> ifftframe;
    std::vector<essentia::Real> wifftframe;
    std::vector<essentia::Real> audioOutput;

    essentia::standard::Algorithm *sinemodelsynth, *ifft, *overlapAdd;
    try {

        
        sinemodelsynth  = essentia::standard::AlgorithmFactory::create("SineModelSynth",
                                                                         "sampleRate", sr,
                                                                         "fftSize", framesize_samps,
                                                                         "hopSize", (int)hopsize_samps);

        ifft     = essentia::standard::AlgorithmFactory::create("IFFT",
                                                                          "size", framesize_samps);
        
        overlapAdd = essentia::standard::AlgorithmFactory::create("OverlapAdd",
                                                                            "frameSize", framesize_samps,
                                                                            "hopSize", (int)hopsize_samps);


        sinemodelsynth->input("magnitudes").set(magnitudes);
        sinemodelsynth->input("frequencies").set(frequencies);
        sinemodelsynth->input("phases").set(phases);
        sinemodelsynth->output("fft").set(sfftframe);
        
        // Synthesis
        ifft->input("fft").set(sfftframe);
        ifft->output("frame").set(ifftframe);

        overlapAdd->input("signal").set(ifftframe);
        overlapAdd->output("signal").set(audioOutput);
        
    } catch (essentia::EssentiaException e) {
        object_error(ob, e.what());
        return EARS_ERR_ESSENTIA;
    }
    
    try {
        std::vector<essentia::Real> allaudio;

        long counter = 0;
        t_llllelem *frequencies_el = frequencies_ll->l_head;
        t_llllelem *magnitudes_el = magnitudes_ll->l_head;
        t_llllelem *phases_el = phases_ll->l_head;

        for ( ; frequencies_el && magnitudes_el && phases_el;
             frequencies_el = frequencies_el->l_next,
             magnitudes_el = magnitudes_el->l_next,
             phases_el = phases_el->l_next) {
            
            if (hatom_gettype(&frequencies_el->l_hatom) != H_LLLL ||
                hatom_gettype(&magnitudes_el->l_hatom) != H_LLLL ||
                hatom_gettype(&phases_el->l_hatom) != H_LLLL) {
                err = EARS_ERR_GENERIC;
                break;
            }
            
            t_llll *freqframe_llll = hatom_getllll(&frequencies_el->l_hatom);
            t_llll *magframe_llll = hatom_getllll(&magnitudes_el->l_hatom);
            t_llll *phframe_llll = hatom_getllll(&phases_el->l_hatom);

            frequencies = vector_from_llll(freqframe_llll);
            magnitudes = vector_from_llll(magframe_llll);
            phases = vector_from_llll(phframe_llll);
            
            ears_convert_ampunit(magnitudes, NULL, in_ampunit, EARS_AMPUNIT_DECIBEL);
            ears_convert_frequnit(frequencies, NULL, in_frequnit, EARS_FREQUNIT_HERTZ);
            ears_convert_angleunit(phases, NULL, in_angleunit, EARS_ANGLEUNIT_RADIANS);
            

            // Sine model synthesis
            sinemodelsynth->compute();
            
            ifft->compute();
            overlapAdd->compute();
            
            if (counter == 0) {
                // skip first half window (should we also skip the last half window?)
                for (long s = params->framesize_samps/2; s < audioOutput.size(); s++) {
                    allaudio.push_back(audioOutput[s]);
                }
            } else {
                allaudio.insert(allaudio.end(), audioOutput.begin(), audioOutput.end());
            }

            counter++;
        }
        
        // putting audio into channel
        if (channelidx == 0)
            ears_buffer_set_size_samps(ob, outbuf, allaudio.size());
        
        ears_buffer_setchannel(ob, outbuf, channelidx, allaudio);
    }
    catch (essentia::EssentiaException e)
    {
        object_error(ob, e.what());
    }
    
    delete sinemodelsynth;
    delete ifft;
    delete overlapAdd;
    
    return err;
}










t_ears_err ears_model_SPR_analysis(t_object *ob, std::vector<Real> samples, double sr,
                                   t_llll **frequencies_ll, t_llll **magnitudes_ll, t_llll **phases_ll,
                                   std::vector<std::vector<essentia::Real>> &framewiseResiduals,
                                   long channelidx,
                                   t_ears_essentia_analysis_params *params,
                                   e_ears_angleunit out_angleunit,
                                   e_ears_ampunit out_ampunit,
                                   e_ears_frequnit out_frequnit,
                                   double freqDevOffset,
                                   double freqDevSlope,
                                   double magnitudeThreshold,
                                   double minFrequency,
                                   double maxFrequency,
                                   long maxPeaks,
                                   long maxnSines,
                                   const char *orderBy
                                   )
{
    t_ears_err err = EARS_ERR_NONE;
    
    // we approximate the hopsize to the nearest sample. Otherwise the overlap add gives troubles...
    long hopsize_samps = (long)params->hopsize_samps;
    int framesize_samps = (int)params->framesize_samps;
    
    std::vector<essentia::Real> framedata;
    std::vector<essentia::Real> wframedata;
    std::vector<essentia::Real> magnitudes, frequencies, phases;
    std::vector<essentia::Real> residual;
    std::vector<essentia::Real> residualAudio;

    framewiseResiduals.clear();
    
    *frequencies_ll = llll_get();
    *magnitudes_ll = llll_get();
    *phases_ll = llll_get();
    
    essentia::standard::Algorithm *frameCutter, *sprAnal, *overlapAdd;
    try {
        frameCutter = essentia::standard::AlgorithmFactory::create("FrameCutter",
                                                                   "frameSize", framesize_samps,
                                                                   "hopSize", (Real)hopsize_samps,
                                                                   "startFromZero", params->startFromZero,
                                                                   "lastFrameToEndOfFile", params->lastFrameToEndOfFile);
        
        sprAnal = essentia::standard::AlgorithmFactory::create("SprModelAnal",
                                                            "sampleRate", sr,
                                                            "freqDevOffset", (int)freqDevOffset,
                                                            "freqDevSlope", freqDevSlope,
                                                            "magnitudeThreshold", magnitudeThreshold,
                                                            "minFrequency", minFrequency,
                                                            "maxFrequency", maxFrequency,
                                                            "maxPeaks", (int)maxPeaks,
                                                            "maxnSines", (int)maxnSines,
                                                            "orderBy", orderBy);

        overlapAdd = essentia::standard::AlgorithmFactory::create("OverlapAdd",
                                                                  "frameSize", framesize_samps,
                                                                  "hopSize", (int)hopsize_samps);

        frameCutter->input("signal").set(samples);
        frameCutter->output("frame").set(framedata);
        sprAnal->input("frame").set(framedata);
        sprAnal->output("frequencies").set(frequencies);
        sprAnal->output("magnitudes").set(magnitudes);
        sprAnal->output("phases").set(phases);
        sprAnal->output("res").set(residual);
        
        overlapAdd->input("signal").set(residual);
        overlapAdd->output("signal").set(residualAudio);

    } catch (essentia::EssentiaException e) {
        object_error(ob, e.what());
        return EARS_ERR_ESSENTIA;
    }
    
    try {
        frameCutter->reset();
        long counter = 0, numsamps = 0;
        while (true) {
            frameCutter->compute(); // new frame
            if (!framedata.size()) {
                break;
            }
            
            sprAnal->compute();
            
            ears_convert_ampunit(magnitudes, NULL, EARS_AMPUNIT_DECIBEL, out_ampunit);
            ears_convert_frequnit(frequencies, NULL, EARS_FREQUNIT_HERTZ, out_frequnit);
            ears_convert_angleunit(phases, NULL, EARS_ANGLEUNIT_RADIANS, out_angleunit);
            
            llll_appendllll(*frequencies_ll, llll_from_vector(frequencies));
            llll_appendllll(*magnitudes_ll, llll_from_vector(magnitudes));
            llll_appendllll(*phases_ll, llll_from_vector(phases));
            
            framewiseResiduals.push_back(residual);
            
            counter++;
        }
    }
    catch (essentia::EssentiaException e)
    {
        object_error(ob, e.what());
    }
    
    delete frameCutter;
    delete sprAnal;
    delete overlapAdd;
    
    return err;
}


t_ears_err ears_model_SPR_synthesis(t_object *ob, double sr,
                                    t_llll *frequencies_ll, t_llll *magnitudes_ll, t_llll *phases_ll,
                                    t_buffer_obj *residual,
                                    t_buffer_obj *outbuf,
                                    t_ears_essentia_analysis_params *params,
                                    e_ears_angleunit in_angleunit,
                                    e_ears_ampunit in_ampunit,
                                    e_ears_frequnit in_frequnit,
                                    long channelidx
                                    )
{
    t_ears_err err = EARS_ERR_NONE;
    
    // we approximate the hopsize to the nearest sample. Otherwise the overlap add gives troubles...
    long hopsize_samps = (long)params->hopsize_samps;
    int framesize_samps = (int)params->framesize_samps;
    
    std::vector<essentia::Real> magnitudes, frequencies, phases;
    std::vector<std::complex<essentia::Real>> sfftframe;
    std::vector<essentia::Real> ifftframe;
    std::vector<essentia::Real> wifftframe;
    std::vector<essentia::Real> audioOutput;
    
    essentia::standard::Algorithm *sinemodelsynth, *ifft, *overlapAdd;
    try {
        
        sinemodelsynth  = essentia::standard::AlgorithmFactory::create("SineModelSynth",
                                                                       "sampleRate", sr,
                                                                       "fftSize", framesize_samps,
                                                                       "hopSize", (int)hopsize_samps);
        
        ifft     = essentia::standard::AlgorithmFactory::create("IFFT",
                                                                "size", framesize_samps);
        
        overlapAdd = essentia::standard::AlgorithmFactory::create("OverlapAdd",
                                                                  "frameSize", framesize_samps,
                                                                  "hopSize", (int)hopsize_samps);
        
        
        sinemodelsynth->input("magnitudes").set(magnitudes);
        sinemodelsynth->input("frequencies").set(frequencies);
        sinemodelsynth->input("phases").set(phases);
        sinemodelsynth->output("fft").set(sfftframe);
        
        // Synthesis
        ifft->input("fft").set(sfftframe);
        ifft->output("frame").set(ifftframe);
        
        overlapAdd->input("signal").set(ifftframe);
        overlapAdd->output("signal").set(audioOutput);
        
    } catch (essentia::EssentiaException e) {
        object_error(ob, e.what());
        return EARS_ERR_ESSENTIA;
    }
    
    try {
        std::vector<essentia::Real> allaudio;
        
        long counter = 0;
        t_llllelem *frequencies_el = frequencies_ll->l_head;
        t_llllelem *magnitudes_el = magnitudes_ll->l_head;
        t_llllelem *phases_el = phases_ll->l_head;
        
        for ( ; frequencies_el && magnitudes_el && phases_el;
             frequencies_el = frequencies_el->l_next,
             magnitudes_el = magnitudes_el->l_next,
             phases_el = phases_el->l_next) {
            
            if (hatom_gettype(&frequencies_el->l_hatom) != H_LLLL ||
                hatom_gettype(&magnitudes_el->l_hatom) != H_LLLL ||
                hatom_gettype(&phases_el->l_hatom) != H_LLLL) {
                err = EARS_ERR_GENERIC;
                break;
            }
            
            t_llll *freqframe_llll = hatom_getllll(&frequencies_el->l_hatom);
            t_llll *magframe_llll = hatom_getllll(&magnitudes_el->l_hatom);
            t_llll *phframe_llll = hatom_getllll(&phases_el->l_hatom);
            
            frequencies = vector_from_llll(freqframe_llll);
            magnitudes = vector_from_llll(magframe_llll);
            phases = vector_from_llll(phframe_llll);
            
            ears_convert_ampunit(magnitudes, NULL, in_ampunit, EARS_AMPUNIT_DECIBEL);
            ears_convert_frequnit(frequencies, NULL, in_frequnit, EARS_FREQUNIT_HERTZ);
            ears_convert_angleunit(phases, NULL, in_angleunit, EARS_ANGLEUNIT_RADIANS);
            
            
            // Sine model synthesis
            sinemodelsynth->compute();
            
            ifft->compute();
            overlapAdd->compute();
            
            if (counter == 0) {
                // skip first half window (should we also skip the last half window?)
                for (long s = params->framesize_samps/2; s < audioOutput.size(); s++) {
                    allaudio.push_back(audioOutput[s]);
                }
            } else {
                allaudio.insert(allaudio.end(), audioOutput.begin(), audioOutput.end());
            }
            
            counter++;
        }
        
        ears_buffer_sumchannel(ob, outbuf, channelidx, allaudio);
    }
    catch (essentia::EssentiaException e)
    {
        object_error(ob, e.what());
    }
    
    delete sinemodelsynth;
    delete ifft;
    delete overlapAdd;
    
    return err;
}
