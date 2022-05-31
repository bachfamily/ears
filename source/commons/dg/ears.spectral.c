#include "ears.spectral.h"
#include <numeric>

const double PAULSTRETCH_MIN_STRETCH_FACTOR = 0.00001;

long ears_get_window(float *win, const char *type, long numframes)
{
    double H = (numframes-1.)/2.;
    double halfF = PI / (numframes-1.);
    double F = 2 * PI / (numframes-1.);
    double twoF = 2 * F;
    double threeF = 3 * F;
    long err = 0;

    if (strcmp(type, "rect") == 0 || strcmp(type, "rectangle") == 0 || strcmp(type, "rectangular") == 0 || strcmp(type, "square") == 0) {
        for (long i = 0; i < numframes; i++)
            win[i] = 1.;
    } else if (strcmp(type, "tri") == 0 || strcmp(type, "triangle") == 0 || strcmp(type, "triangular") == 0) {
        for (long i = 0; i < numframes; i++)
            win[i] = 1 - fabs((i - H)/H); // denominator could be +1 or +2
    } else if (strcmp(type, "sine") == 0 || strcmp(type, "sin") == 0 || strcmp(type, "cos") == 0 || strcmp(type, "cosine") == 0) {
        for (long i = 0; i < numframes; i++)
            win[i] = sin(halfF * i);
    } else if (strcmp(type, "hann") == 0) {
        double a0 = 0.5;
        double a1 = 1. - a0;
        for (long i = 0; i < numframes; i++) 
            win[i] = a0  - a1 * cos(F*i);
    } else if (strcmp(type, "sqrthann") == 0) {
        double a0 = 0.5;
        double a1 = 1. - a0;
        for (long i = 0; i < numframes; i++)
            win[i] = sqrt(a0  - a1 * cos(F*i));
    } else if (strcmp(type, "sqrthamming") == 0) {
        double a0 = 25./46.;
        double a1 = 1. - a0;
        for (long i = 0; i < numframes; i++)
            win[i] = sqrt(a0  - a1 * cos(F*i));
    } else if (strcmp(type, "hamming") == 0) {
        double a0 = 25./46.;
        double a1 = 1. - a0;
        for (long i = 0; i < numframes; i++)
            win[i] = a0  - a1 * cos(F*i);
    } else if (strcmp(type, "blackman") == 0) {
        double a0 = 0.42, a1 = 0.5, a2 = 0.08;
        for (long i = 0; i < numframes; i++) {
            win[i] = a0 - a1 * cos(F*i) + a2 * cos(twoF * i);
        }
    } else if (strcmp(type, "nuttall") == 0) {
        double a0 = 0.355768, a1 = 0.487396, a2 = 0.144232, a3 = 0.012604;
        for (long i = 0; i < numframes; i++) {
            win[i] = a0 - a1 * cos(F*i) + a2 * cos(twoF * i) - a3 * cos(threeF * i);
        }
    } else if (strcmp(type, "blackmannuttall") == 0) {
        double a0 = 0.3635819, a1 = 0.4891775, a2 = 0.1365995, a3 = 0.0106411;
        for (long i = 0; i < numframes; i++) {
            win[i] = a0 - a1 * cos(F*i) + a2 * cos(twoF * i) - a3 * cos(threeF * i);
        }
    } else if (strcmp(type, "blackmanharris") == 0) {
        double a0 = 0.35875, a1 = 0.48829, a2 = 0.14128, a3 = 0.01168;
        for (long i = 0; i < numframes; i++) {
            win[i] = a0 - a1 * cos(F*i) + a2 * cos(twoF * i) - a3 * cos(threeF * i);
        }
    } else if (strcmp(type, "gaussian") == 0) {
        const double sigma = 0.4;
        double temp;
        double sigmaH = sigma * H;
        for (long i = 0; i < numframes; i++) {
            temp = (i - H)/sigmaH;
            win[i] = exp(-0.5 * temp * temp);
        }
    } else {
        for (long i = 0; i < numframes; i++)
            win[i] = 1.;
        err = 1;
    }
    return err;
}



// also supports inplace operations
t_ears_err ears_buffer_apply_window(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, t_symbol *window_type)
{
    if (!source || !dest)
        return EARS_ERR_NO_BUFFER;
    
    if (!window_type)
        return EARS_ERR_GENERIC;

    t_ears_err err = EARS_ERR_NONE;
    
    if (source != dest) {
        ears_buffer_copy_format(ob, source, dest);
    }
    
    float *orig_sample = buffer_locksamples(source);
    
    if (!orig_sample) {
        err = EARS_ERR_CANT_READ;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
    } else {
        t_atom_long    channelcount = buffer_getchannelcount(source);        // number of floats in a frame
        t_atom_long    framecount   = buffer_getframecount(source);            // number of floats long the buffer is for a single channel
        
        float *win = (float *)bach_newptr(framecount * sizeof(float));
        if (ears_get_window(win, window_type->s_name, framecount)) {
            err = EARS_ERR_GENERIC;
            object_error((t_object *)ob, "Unknown window type.");
        }
        
        if (source == dest) { // inplace operation!
            for (long i = 0; i < framecount; i++)
                for (long c = 0; c < channelcount; c++)
                    orig_sample[i*channelcount + c] *= win[i];
            buffer_setdirty(source);
        } else {
            ears_buffer_set_size_samps(ob, dest, framecount);
            float *dest_sample = buffer_locksamples(dest);
            if (!dest_sample) {
                err = EARS_ERR_CANT_WRITE;
                object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
            } else {
                for (long i = 0; i < framecount; i++)
                    for (long c = 0; c < channelcount; c++)
                        dest_sample[i*channelcount + c] = orig_sample[i*channelcount + c] * win[i];
            }
            
            buffer_setdirty(dest);
            buffer_unlocksamples(dest);
        }
        
        bach_freeptr(win);
        buffer_unlocksamples(source);
    }
    
    return err;
}

void test_kiss_fft()
{
    long fftsize = 2048;
    kiss_fft_cpx *A = (kiss_fft_cpx *)bach_newptrclear(fftsize * sizeof(kiss_fft_cpx));
    kiss_fft_cpx *B = (kiss_fft_cpx *)bach_newptr(fftsize * sizeof(kiss_fft_cpx));
    kiss_fft_cfg C = kiss_fft_alloc(fftsize, 0, NULL, NULL);
    for (long i = 0; i < fftsize; i++) {
        A[i].r = sin(i/1000.);
        A[i].i = 0.;
    }
    bach_fft_kiss(C, fftsize, 0, A, B, false);
    
    fftsize += 0;
}



t_ears_err ears_buffer_fft(t_object *ob, t_buffer_obj *source1, t_buffer_obj *source2, t_buffer_obj *dest1, t_buffer_obj *dest2, long polar_input, long polar_output, long inverse, long fullspectrum, e_ears_angleunit angleunit, long unitary)
{
    t_ears_err err = EARS_ERR_NONE;
    float *orig_sample1 = buffer_locksamples(source1);
    float *orig_sample2 = source2 ? buffer_locksamples(source2) : NULL;
    float *orig_sample1_wk = NULL;
    float *orig_sample2_wk = NULL;

    if (!orig_sample1) {
        err = EARS_ERR_CANT_READ;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
    } else {
        t_atom_long    channelcount = buffer_getchannelcount(source1);
        t_atom_long    framecount   = buffer_getframecount(source1);
        t_atom_long    outframecount;
        
        if (!inverse)
            outframecount = (fullspectrum ? framecount : (framecount/2 + 1));
        else
            outframecount = (fullspectrum ? framecount : 2*(framecount - 1));

        if (source1 == dest1) { // inplace operation!
            orig_sample1_wk = (float *)bach_newptr(channelcount * framecount * sizeof(float));
            sysmem_copyptr(orig_sample1, orig_sample1_wk, channelcount * framecount * sizeof(float));
            buffer_unlocksamples(source1);
        } else {
            orig_sample1_wk = orig_sample1;
            ears_buffer_copy_format(ob, source1, dest1, true); // won't change frame count and channel count here
        }

        if (source2 == dest2) { // inplace operation!
            orig_sample2_wk = (float *)bach_newptr(channelcount * framecount * sizeof(float));
            sysmem_copyptr(orig_sample2, orig_sample2_wk, channelcount * framecount * sizeof(float));
            buffer_unlocksamples(source2);
        } else {
            orig_sample2_wk = orig_sample2;
            ears_buffer_copy_format(ob, source1, dest2, true);  // won't change frame count and channel count here
        }
        
        ears_buffer_set_size_and_numchannels(ob, dest1, outframecount, channelcount);
        ears_buffer_set_size_and_numchannels(ob, dest2, outframecount, channelcount);
        
        float *dest_sample1 = buffer_locksamples(dest1);
        float *dest_sample2 = buffer_locksamples(dest2);

        if (!dest_sample1 || !dest_sample2) {
            err = EARS_ERR_CANT_WRITE;
            object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
        } else {
            long fftsize = (inverse && !fullspectrum) ? 2*(framecount-1) : framecount;
            
            // TO DO optimize kiss fft for half spectrum (don't know how to)
            
            kiss_fft_cpx *fin = (kiss_fft_cpx *)bach_newptrclear(fftsize * sizeof(kiss_fft_cpx));
            kiss_fft_cpx *fout = (kiss_fft_cpx *)bach_newptr(fftsize * sizeof(kiss_fft_cpx));
            kiss_fft_cfg cfg = kiss_fft_alloc(fftsize, inverse, NULL, NULL);

            for (long c = 0; c < channelcount; c++) {
                if (polar_input) {
                    float amp, phase;
                    for (long j = 0; j < framecount; j++) {
                        amp = orig_sample1_wk[j*channelcount + c];
                        phase = orig_sample2_wk ? ears_angle_to_radians(orig_sample2_wk[j*channelcount + c], angleunit) : 0;
                        fin[j].r = amp * cos(phase);
                        fin[j].i = amp * sin(phase);
                    }
                } else {
                    for (long j = 0; j < framecount; j++) {
                        fin[j].r = orig_sample1_wk[j*channelcount + c];
                        fin[j].i = orig_sample2_wk ? orig_sample2_wk[j*channelcount + c] : 0;
                    }
                }
                if (inverse && !fullspectrum) {
                    for (long j = framecount; j < fftsize; j++) {
                        fin[j] = cpx_conjugate(fin[fftsize-j]);
                    }
                }

                bach_fft_kiss(cfg, fftsize, inverse, fin, fout, unitary);
                
                if (polar_output) {
                    for (long j = 0; j < outframecount; j++) {
                        dest_sample1[j*channelcount + c] = get_cpx_ampli(fout[j]);
                        dest_sample2[j*channelcount + c] = ears_radians_to_angle(get_cpx_phase(fout[j]), angleunit);
                    }
                } else {
                    for (long j = 0; j < outframecount; j++) {
                        dest_sample1[j*channelcount + c] = fout[j].r;
                        dest_sample2[j*channelcount + c] = fout[j].i;
                    }
                }

            }
            
            free(cfg);
            bach_freeptr(fin);
            bach_freeptr(fout);

            buffer_setdirty(dest1);
            buffer_setdirty(dest2);
            buffer_unlocksamples(dest1);
            buffer_unlocksamples(dest2);
        }
        
        if (source1 == dest1) // inplace operation!
            bach_freeptr(orig_sample1_wk);
        else
            buffer_unlocksamples(source1);

        if (source2 == dest2) // inplace operation!
            bach_freeptr(orig_sample2_wk);
        else
            buffer_unlocksamples(source2);
    }
    
    return err;
}


t_ears_err ears_buffer_stft(t_object *ob, t_buffer_obj *source1, t_buffer_obj *source2, long channel, t_buffer_obj *dest1, t_buffer_obj *dest2,
                            long framesize_samps, double hopsize_samps, const char *wintype,
                            long polar_input, long polar_output, long fullspectrum, e_ears_angleunit angleunit, long left_aligned_windows, long unitary)
{
    
    t_ears_err err = EARS_ERR_NONE;
    float *orig_sample1 = buffer_locksamples(source1);
    float *orig_sample2 = source2 ? buffer_locksamples(source2) : NULL;
    float *orig_sample1_wk = NULL;
    float *orig_sample2_wk = NULL;

    double audiosr = ears_buffer_get_sr(ob, source1);
    if (source2 && ears_buffer_get_sr(ob, source2) != audiosr) {
        if (orig_sample1)
            buffer_unlocksamples(source1);
        if (orig_sample2)
            buffer_unlocksamples(source2);
        object_error(ob, "Sample rate must be the same for the two inputs.");
        return EARS_ERR_GENERIC;
    }
    
    double new_sr = audiosr/(hopsize_samps); // sr of the windowed signal

    if (channel > ears_buffer_get_numchannels(ob, source1)) {
        if (orig_sample1)
            buffer_unlocksamples(source1);
        if (orig_sample2)
            buffer_unlocksamples(source2);
        object_error(ob, "Invalid channel number.");
        return EARS_ERR_GENERIC;
    }

    if (!orig_sample1) {
        if (orig_sample2)
            buffer_unlocksamples(source2);
        err = EARS_ERR_CANT_READ;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
    } else {
        t_atom_long    channelcount = buffer_getchannelcount(source1);
        t_atom_long    samplecount   = buffer_getframecount(source1);
        t_atom_long    numbins = 0;
        
        numbins = (fullspectrum ? framesize_samps : (framesize_samps/2 + 1));

        if (source1 == dest1) { // inplace operation!
            orig_sample1_wk = (float *)bach_newptr(channelcount * samplecount * sizeof(float));
            sysmem_copyptr(orig_sample1, orig_sample1_wk, channelcount * samplecount * sizeof(float));
            buffer_unlocksamples(source1);
        } else {
            orig_sample1_wk = orig_sample1;
            ears_buffer_copy_format(ob, source1, dest1, true); // won't change frame count and channel count here
        }

        if (source2 == dest2) { // inplace operation!
            orig_sample2_wk = (float *)bach_newptr(channelcount * samplecount * sizeof(float));
            sysmem_copyptr(orig_sample2, orig_sample2_wk, channelcount * samplecount * sizeof(float));
            buffer_unlocksamples(source2);
        } else {
            orig_sample2_wk = orig_sample2;
            ears_buffer_copy_format(ob, source1, dest2, true);  // won't change frame count and channel count here
        }
        
        ears_buffer_set_sr(ob, dest1, new_sr);
        ears_buffer_set_sr(ob, dest2, new_sr);
        
        t_ears_spectralbuf_metadata data;
        t_llll *bins = NULL;
        double binsize = 0;
        if (fullspectrum) {
            bins = ears_ezarithmser(0, audiosr, numbins + 1);
            llll_destroyelem(bins->l_tail);
            binsize = audiosr/numbins;
        } else {
            bins = ears_ezarithmser(0, audiosr/2., numbins);
            binsize = (audiosr/2.)/(numbins-1.);
        }
        ears_spectralbuf_metadata_fill(&data, audiosr, binsize, 0, EARS_FREQUNIT_HERTZ, gensym("stft"), bins, false);
        ears_spectralbuf_metadata_set(ob, dest1, &data);
        ears_spectralbuf_metadata_set(ob, dest2, &data);
        llll_free(bins);

        long numframes = 1+(long)(ceil(((double)(samplecount - 1)) / hopsize_samps));
        ears_buffer_set_size_and_numchannels(ob, dest1, numframes, numbins);
        ears_buffer_set_size_and_numchannels(ob, dest2, numframes, numbins);

        float *dest_sample1 = buffer_locksamples(dest1);
        float *dest_sample2 = buffer_locksamples(dest2);

        if (!dest_sample1 || !dest_sample2) {
            err = EARS_ERR_CANT_WRITE;
            object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
        } else {
            long fftsize = framesize_samps;
            // TO DO optimize kiss fft for half spectrum (don't know how to)
            
            // Get window
            float *window = (float *)bach_newptrclear(framesize_samps * sizeof(float));
            ears_get_window(window, wintype, framesize_samps);

            kiss_fft_cpx *fin = (kiss_fft_cpx *)bach_newptrclear(samplecount * sizeof(kiss_fft_cpx));

            if (polar_input) {
                float amp, phase;
                for (long j = 0; j < samplecount; j++) {
                    if (channel < 0) { // downmix
                        amp = phase = 0;
                        for (long c = 0; c < channelcount; c++) {
                            amp += orig_sample1_wk[j*channelcount + c];
                            phase += orig_sample2_wk ? ears_angle_to_radians(orig_sample2_wk[j*channelcount + c], angleunit) : 0;
                        }
                        amp /= channelcount;
                        phase /= channelcount;
                    } else {
                        amp = orig_sample1_wk[j*channelcount + channel];
                        phase = orig_sample2_wk ? ears_angle_to_radians(orig_sample2_wk[j*channelcount + channel], angleunit) : 0;
                    }
                    fin[j].r = amp * cos(phase);
                    fin[j].i = amp * sin(phase);
                }
            } else {
                for (long j = 0; j < samplecount; j++) {
                    if (channel < 0) { // downmix
                        fin[j].r = fin[j].i = 0;
                        for (long c = 0; c < channelcount; c++) {
                            fin[j].r += orig_sample1_wk[j*channelcount + c];
                            fin[j].i += orig_sample2_wk ? orig_sample2_wk[j*channelcount + c] : 0;
                        }
                        fin[j].r /= channelcount;
                        fin[j].i /= channelcount;
                    } else {
                        fin[j].r = orig_sample1_wk[j*channelcount + channel];
                        fin[j].i = orig_sample2_wk ? orig_sample2_wk[j*channelcount + channel] : 0;
                    }
                }
            }
            
            // windowing
            kiss_fft_cpx *win = (kiss_fft_cpx *)bach_newptrclear(fftsize * sizeof(kiss_fft_cpx));
            kiss_fft_cpx *wout = (kiss_fft_cpx *)bach_newptr(fftsize * sizeof(kiss_fft_cpx));
            kiss_fft_cfg cfg = kiss_fft_alloc(fftsize, 0, NULL, NULL);

            for (long f = 0; f < numframes; f++) {
                long central_sample = (long)(hopsize_samps * f);
                long start_sample = left_aligned_windows ? central_sample : central_sample - framesize_samps/2;
                
                // extract window
                for (long j = 0; j < framesize_samps; j++) {
                    long jj = start_sample + j;
                    if (jj < 0 || jj >= samplecount)
                        win[j].r = win[j].i = 0;
                    else {
                        win[j].r = fin[jj].r * window[j]; // windowed
                        win[j].i = fin[jj].i * window[j];
                    }
                }
                
                bach_fft_kiss(cfg, fftsize, 0, win, wout, unitary);

                if (polar_output) {
                    for (long b = 0; b < numbins; b++) {
                        dest_sample1[f*numbins + b] = get_cpx_ampli(wout[b]);
                        dest_sample2[f*numbins + b] = dest_sample1[f*numbins + b] == 0 ? 0 : ears_radians_to_angle(get_cpx_phase(wout[b]), angleunit);
                    }
                } else {
                    for (long b = 0; b < numbins; b++) {
                        dest_sample1[f*numbins + b] = wout[b].r;
                        dest_sample2[f*numbins + b] = wout[b].i;
                    }
                }

            }
            
            free(cfg);

            bach_freeptr(window);
            bach_freeptr(win);
            bach_freeptr(wout);
            bach_freeptr(fin);

            buffer_setdirty(dest1);
            buffer_setdirty(dest2);
            buffer_unlocksamples(dest1);
            buffer_unlocksamples(dest2);
        }
        
        if (source1 == dest1) // inplace operation!
            bach_freeptr(orig_sample1_wk);
        else
            buffer_unlocksamples(source1);

        if (source2 == dest2) // inplace operation!
            bach_freeptr(orig_sample2_wk);
        else
            buffer_unlocksamples(source2);
    }
    
    return err;
}




t_ears_err ears_buffer_istft(t_object *ob, long num_input_buffers, t_buffer_obj **source1, t_buffer_obj **source2, t_buffer_obj *dest1, t_buffer_obj *dest2, const char *wintype, long polar_input, long polar_output, long fullspectrum, e_ears_angleunit angleunit, double force_sr, long left_aligned_windows, long unitary)
{
    
    t_ears_err err = EARS_ERR_NONE;

    if (num_input_buffers == 0) {
        // nothing to do
        ears_buffer_set_size_and_numchannels(ob, dest1, 0, 1);
        if (dest2)
            ears_buffer_set_size_and_numchannels(ob, dest2, 0, 1);
        return err;
    }

    double sr_original = ears_spectralbuf_get_original_audio_sr(ob, source1[0]);
    double hopsize_samps = ears_ms_to_fsamps(1000./ears_buffer_get_sr(ob, source1[0]), force_sr ? force_sr : sr_original);
    double audio_sr = force_sr ? force_sr : sr_original;

    long numinputframes = ears_buffer_get_size_samps(ob, source1[0]);
    long numinputbins = ears_buffer_get_numchannels(ob, source1[0]);
    
    for (long b = 1; b < num_input_buffers; b++) {
        if (ears_buffer_get_size_samps(ob, source1[b]) != numinputframes) {
            object_error(ob, "All input buffers must have the same number of samples");
            return EARS_ERR_GENERIC;
        }
        if (ears_buffer_get_numchannels(ob, source1[b]) != numinputbins) {
            object_error(ob, "All input buffers must have the same number of channels");
            return EARS_ERR_GENERIC;
        }
    }

    long numoutputchannels = num_input_buffers;
    int framesize_samps = fullspectrum ? numinputbins : 2 * (numinputbins - 1);
    long outframecount = numinputframes * hopsize_samps + MAX(0, framesize_samps - hopsize_samps);
    
    if (framesize_samps <= 0) {
        object_error(ob, "Negative number of frames!");
        return EARS_ERR_GENERIC;
    }

    ears_buffer_set_sr(ob, dest1, audio_sr);
    if (dest2)
        ears_buffer_set_sr(ob, dest2, audio_sr);
    ears_buffer_set_size_and_numchannels(ob, dest1, outframecount, numoutputchannels);
    if (dest2)
        ears_buffer_set_size_and_numchannels(ob, dest2, outframecount, numoutputchannels);

    ears_buffer_clear(ob, dest1);
    if (dest2)
        ears_buffer_clear(ob, dest2);

    float *dest1_sample = buffer_locksamples(dest1);
    float *dest2_sample = dest2 ? buffer_locksamples(dest2) : NULL;
    float **source1_sample = (float **)bach_newptr(num_input_buffers * sizeof(float *));
    float **source2_sample = (float **)bach_newptr(num_input_buffers * sizeof(float *));

    bool all_phases_defined = true;
    bool all_mags_defined = true;
    
    for (long b = 0; b < num_input_buffers; b++) {
        source1_sample[b] = source1[b] ? buffer_locksamples(source1[b]) : NULL;
        source2_sample[b] = source2[b] ? buffer_locksamples(source2[b]) : NULL;
        if (!source2[b])
            all_phases_defined = false; //TO DO check source1_sample != NULL
        if (!source1[b])
            all_mags_defined = false;
    }
    
    if (!all_mags_defined || ! all_phases_defined) {
        err = EARS_ERR_CANT_READ;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
    } else if (!dest1_sample) {
        err = EARS_ERR_CANT_WRITE;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
    } else {
        // Get window
        float *window = (float *)bach_newptrclear(framesize_samps * sizeof(float));
        if (wintype)
            ears_get_window(window, wintype, framesize_samps);

        
        numoutputchannels = MIN(numoutputchannels, buffer_getchannelcount(dest1));
        outframecount = MIN(outframecount, buffer_getframecount(dest1));
        
        memset(dest1_sample, 0, outframecount * numoutputchannels * sizeof(float));
        if (dest2_sample)
            memset(dest1_sample, 0, outframecount * numoutputchannels * sizeof(float));
        
        for (long c = 0; c < numoutputchannels; c++) {
            long fftsize = framesize_samps;
            
            kiss_fft_cpx *win = (kiss_fft_cpx *)bach_newptrclear(fftsize * sizeof(kiss_fft_cpx));
            kiss_fft_cpx *wout = (kiss_fft_cpx *)bach_newptr(fftsize * sizeof(kiss_fft_cpx));
            kiss_fft_cfg cfg = kiss_fft_alloc(fftsize, 1, NULL, NULL);
            
            double onset = left_aligned_windows ? 0 : -framesize_samps/2;
            for (long f = 0; f < numinputframes; f++, onset += hopsize_samps) {
                // input formatting
                long b = 0;
                for (; b < numinputbins; b++) {
                    if (polar_input) {
                        float amp = source1_sample[c][f*numinputbins + b];
                        float phase = source2_sample[c] ? source2_sample[c][f*numinputbins + b] : 0.;
                        win[b].r = amp * cos(phase);
                        win[b].i = amp * sin(phase);
                    } else {
                        win[b].r = source1_sample[c][f*numinputbins + b];
                        win[b].i = source2_sample[c] ? source2_sample[c][f*numinputbins + b] : 0.;
                    }
                }
                for (; b < fftsize; b++) {
                    // mirroring remaining stuff
                    if (fftsize-b >= 0 && fftsize-b < numinputbins) {
                        win[b].r = win[fftsize-b].r;
                        win[b].i = -win[fftsize-b].i;
                    } else {
                        win[b].r = win[b].i = 0.;
                    }
                }
                
                // inverse
                bach_fft_kiss(cfg, fftsize, 1, win, wout, unitary);
                
                // overlap add
                long start_sample = (long)onset;
                for (long i = 0; i < fftsize; i++) {
                    long ii = start_sample + i;
                    if (ii >= 0 && ii < outframecount) {
                        dest1_sample[ii * numoutputchannels + c] += wout[i].r * window[i];
                        if (dest2_sample)
                            dest2_sample[ii * numoutputchannels + c] += wout[i].i * window[i];
                    }
                }
            }
            

            if (polar_output) {
                long outframecount = buffer_getframecount(dest1);
                for (long ii = 0; ii < outframecount; ii++) {
                    if (!dest2_sample) {
                        dest1_sample[ii * numoutputchannels + c] = fabs(dest1_sample[ii * numoutputchannels + c] );
                    } else {
                        double xx = dest1_sample[ii * numoutputchannels + c];
                        double yy = dest2_sample[ii * numoutputchannels + c];
                        dest1_sample[ii * numoutputchannels + c] = sqrt(xx * xx + yy * yy);
                        dest2_sample[ii * numoutputchannels + c] = atan2(yy, xx);
                    }
                }
            }

        }
        
        buffer_setdirty(dest1);
        if (dest2_sample) {
            buffer_setdirty(dest2);
        }
        
        bach_freeptr(window);
    }

    buffer_unlocksamples(dest1);
    if (dest2_sample) {
        buffer_unlocksamples(dest2);
    }

    for (long b = 0; b < num_input_buffers; b++) {
        buffer_unlocksamples(source1[b]);
        if (source2_sample[b])
            buffer_unlocksamples(source2[b]);
    }
    

    bach_freeptr(source1_sample);
    bach_freeptr(source2_sample);
    
    return err;
}





t_ears_err ears_griffin_lim(t_object *ob, t_buffer_obj *amplitudes, t_buffer_obj *dest, long fullspectrum, e_ears_angleunit angleunit, long framesize_samps, double hopsize_samps, double audio_sr, long outframecount, long left_aligned_windows, long unitary, long numGriffinLimIterations)
{
    t_ears_err err = EARS_ERR_NONE;
    
    // Reconstruct via Griffin-Lim
    t_buffer_obj *phases = ears_buffer_make(NULL);
    t_buffer_obj *amps = ears_buffer_make(NULL);

    ears_buffer_set_sr(ob, dest, audio_sr);
    ears_buffer_set_size_and_numchannels(ob, dest, outframecount, 1);
    
    // initialize audio
    float *tempout_sample = buffer_locksamples(dest);
    if (tempout_sample) {
        t_atom_long    channelcount = buffer_getchannelcount(dest); // must be 1
        t_atom_long    framecount   = buffer_getframecount(dest); // must be outframecount
        
        for (long j = 0; j < framecount*channelcount; j++)
            tempout_sample[j] = random_double_in_range(-1, 1);
        buffer_unlocksamples(dest);
    }
    
    for (int n = 0; n < numGriffinLimIterations; n++) {
        // reconstruction spectrogram
        ears_buffer_stft(ob, dest, NULL, 0, amps, phases, framesize_samps, hopsize_samps, "sqrthann", false, true, fullspectrum, angleunit, left_aligned_windows, unitary);
        
        // Discard magnitude part of the reconstruction and use the supplied magnitude spectrogram instead
        ears_buffer_istft(ob, 1, &amplitudes, &phases, dest, NULL, "sqrthann", true, false, fullspectrum, angleunit, audio_sr, left_aligned_windows, unitary);
    }
    
    ears_buffer_free(amps);
    ears_buffer_free(phases);
    return err;
}


t_ears_err ears_griffin_lim_2(t_object *ob, t_buffer_obj *amplitudes, t_buffer_obj *dest, long fullspectrum, e_ears_angleunit angleunit, const char *wintype, long framesize_samps, double hopsize_samps, double audio_sr, long outframecount, long left_aligned_windows, long unitary, long numGriffinLimIterations)
{
    t_ears_err err = EARS_ERR_NONE;
    
    // Reconstruct via Griffin-Lim
    t_buffer_obj *estimate_phases = ears_buffer_make(NULL);
    t_buffer_obj *estimate_amps = ears_buffer_make(NULL);
    
    double momentum = 0.9;
    long bincount = ears_buffer_get_numchannels(ob, amplitudes);
    long wincount = ears_buffer_get_size_samps(ob, amplitudes);
    
    ears_buffer_set_sr(ob, dest, audio_sr);
    ears_buffer_set_size_and_numchannels(ob, dest, outframecount, 1);
    ears_buffer_copy_format(ob, amplitudes, estimate_phases);
    ears_buffer_set_size_and_numchannels(ob, estimate_phases, wincount, bincount);
    ears_buffer_copy_format(ob, amplitudes, estimate_amps);
    ears_buffer_set_size_and_numchannels(ob, estimate_amps, wincount, bincount);
    
    float *previous_real = (float *)bach_newptrclear((wincount + 10) * bincount * sizeof(float));
    float *previous_imag = (float *)bach_newptrclear((wincount + 10) * bincount * sizeof(float));

    // initialize audio
    float *estimate_phases_sample = buffer_locksamples(estimate_phases);
    if (estimate_phases_sample) {
        t_atom_long    channelcount = buffer_getchannelcount(estimate_phases); // must be 1
        t_atom_long    framecount   = buffer_getframecount(estimate_phases); // must be outframecount
        
        for (long j = 0; j < framecount*channelcount; j++)
            estimate_phases_sample[j] = random_double_in_range(0, TWOPI); // initial random phase
//                            tempout_sample[j] = random_double_in_range(-1, 1); // white noise?
        buffer_unlocksamples(estimate_phases);
    }
    
    for (int n = 0; n < numGriffinLimIterations; n++) {
        // Discard magnitude part of the reconstruction and use the supplied magnitude spectrogram instead
        ears_buffer_istft(ob, 1, &amplitudes, &estimate_phases, dest, NULL, wintype, true, false, fullspectrum, angleunit, audio_sr, left_aligned_windows, unitary);

        // reconstruction spectrogram
        ears_buffer_stft(ob, dest, NULL, 0, estimate_amps, estimate_phases, framesize_samps, hopsize_samps, "sqrthann", false, true, fullspectrum, angleunit, left_aligned_windows, unitary);
        
        // apply momentum
        float *estimate_amps_sample = buffer_locksamples(estimate_amps);
        float *estimate_phases_sample = buffer_locksamples(estimate_phases);
        if (estimate_phases_sample && estimate_amps_sample) {
            t_atom_long    amps_channelcount = buffer_getchannelcount(estimate_amps); // must be bincount
            t_atom_long    amps_framecount   = buffer_getframecount(estimate_amps); // must be outframecount
            t_atom_long    phases_channelcount = buffer_getchannelcount(estimate_phases); // must be bincount
            t_atom_long    phases_framecount   = buffer_getframecount(estimate_phases); // must be outframecount

            if (amps_channelcount != bincount || phases_channelcount != bincount || amps_framecount != phases_framecount || amps_framecount > wincount + 10) {
                object_error(ob, "Error!");
                buffer_unlocksamples(estimate_amps);
                buffer_unlocksamples(estimate_phases);
                break;
            }
            
            for (long j = 0; j < amps_framecount * bincount; j++) {
                // convert into polar coordinates
                double real = estimate_amps_sample[j] * cos(estimate_phases_sample[j]);
                double imag = estimate_amps_sample[j] * sin(estimate_phases_sample[j]);
                double newreal = real - (momentum/(1+momentum)) * previous_real[j];
                double newimag = imag - (momentum/(1+momentum)) * previous_imag[j];
                estimate_amps_sample[j] = sqrt(newreal*newreal + newimag * newimag);
                estimate_phases_sample[j] = atan2(newimag, newreal);
                previous_real[j] = real;
                previous_imag[j] = imag;

            }
            buffer_unlocksamples(estimate_amps);
            buffer_unlocksamples(estimate_phases);
        }
    }

    ears_buffer_istft(ob, 1, &amplitudes, &estimate_phases, dest, NULL, "sqrthann", true, false, fullspectrum, angleunit, audio_sr, left_aligned_windows, unitary);

    bach_freeptr(previous_real);
    bach_freeptr(previous_imag);

    ears_buffer_free(estimate_amps);
    ears_buffer_free(estimate_phases);
    
    return EARS_ERR_NONE;
}





t_ears_err ears_buffer_griffinlim(t_object *ob, long num_buffers, t_buffer_obj **orig_amps, t_buffer_obj **reconstructed_phases, t_buffer_obj **phases_mask, t_buffer_obj *dest_signal, const char *analysiswintype, const char *synthesiswintype, long fullspectrum, long left_aligned_windows, long unitary, long num_iterations, bool randomize_phases)
{
    
    t_ears_err err = EARS_ERR_NONE;

    if (num_buffers == 0) {
        // nothing to do
        return EARS_ERR_NO_BUFFER;
    }

    double sr_original = ears_spectralbuf_get_original_audio_sr(ob, orig_amps[0]);
    double hopsize_samps = ears_ms_to_fsamps(1000./ears_buffer_get_sr(ob, orig_amps[0]), sr_original);
    double audio_sr = sr_original;

    long numinputframes = ears_buffer_get_size_samps(ob, orig_amps[0]);
    long numinputbins = ears_buffer_get_numchannels(ob, orig_amps[0]);
    
    for (long b = 1; b < num_buffers; b++) {
        if (ears_buffer_get_size_samps(ob, orig_amps[b]) != numinputframes) {
            object_error(ob, "All input buffers must have the same number of samples");
            return EARS_ERR_GENERIC;
        }
        if (ears_buffer_get_numchannels(ob, orig_amps[b]) != numinputbins) {
            object_error(ob, "All input buffers must have the same number of channels");
            return EARS_ERR_GENERIC;
        }
    }
    
    int framesize_samps = fullspectrum ? numinputbins : 2 * (numinputbins - 1);
    
    if (framesize_samps <= 0) {
        object_error(ob, "Negative number of frames!");
        return EARS_ERR_GENERIC;
    }

    for (long i = 0; i < num_buffers; i++) {
        t_buffer_obj *candidate_phases_i = ears_buffer_make(NULL);
        
/*        ears_buffer_copy_format(ob, orig_amps[i], candidate_phases_i);
        ears_buffer_set_size_and_numchannels(ob, candidate_phases_i, numinputframes, numinputbins);
        ears_buffer_clear(ob, candidate_phases_i);
  */
        // Reconstruct via Griffin-Lim
        long orig_num_frames = ears_buffer_get_size_samps(ob, orig_amps[i]);
        t_buffer_obj *temp_amps_i = ears_buffer_make(NULL);
        t_buffer_obj *temp = ears_buffer_make(NULL);
        ears_buffer_clone(ob, orig_amps[i], candidate_phases_i);
        ears_buffer_clone(ob, orig_amps[i], temp_amps_i);
        
        // initialize phases randomly
        if (randomize_phases)
            ears_buffer_random_fill_inplace(ob, candidate_phases_i, 0, TWOPI);
        else
            ears_buffer_clone(ob, reconstructed_phases[i], candidate_phases_i);

        for (int n = 0; n < num_iterations; n++) {
            // apply mask, if needed
            if (phases_mask)
                ears_buffer_apply_mask(ob, candidate_phases_i, reconstructed_phases[i], phases_mask[i]);

            // inverse stft
            ears_buffer_istft(ob, 1, &orig_amps[i], &candidate_phases_i, temp, NULL, synthesiswintype, true, false, fullspectrum, EARS_ANGLEUNIT_RADIANS, audio_sr, left_aligned_windows, unitary);
            
            ears_buffer_stft(ob, temp, NULL, 0, temp_amps_i, candidate_phases_i, framesize_samps, hopsize_samps, analysiswintype, false, true, fullspectrum, EARS_ANGLEUNIT_RADIANS, left_aligned_windows, unitary);
            
            // remove final frames that may have been added by istft/stft combo.
            ears_buffer_crop_inplace(ob, temp_amps_i, 0, orig_num_frames);
            ears_buffer_crop_inplace(ob, candidate_phases_i, 0, orig_num_frames);
        }
        
        if (phases_mask)
            ears_buffer_apply_mask(ob, candidate_phases_i, reconstructed_phases[i], phases_mask[i]);

        ears_buffer_clone(ob, candidate_phases_i, reconstructed_phases[i]);
        
        if (dest_signal) {
            ears_buffer_istft(ob, 1, &orig_amps[i], &candidate_phases_i, temp, NULL, synthesiswintype, true, false, fullspectrum, EARS_ANGLEUNIT_RADIANS, audio_sr, left_aligned_windows, unitary);
            
            if (i == 0) {
                ears_buffer_copy_format(ob, temp, dest_signal);
                ears_buffer_set_size_and_numchannels(ob, dest_signal, ears_buffer_get_size_samps(ob, temp), num_buffers);
            }
            ears_buffer_copychannel(ob, temp, 0, dest_signal, i);
        }
        
        ears_buffer_free(temp_amps_i);
        ears_buffer_free(temp);
    }
    
    return err;
}






/// Paulstretch functions



long optimize_windowsize(long n)
{
    long orig_n=n;
    while (true) {
        n=orig_n;
        while ((n%2)==0)
            n/=2;
        while ((n%3)==0)
            n/=3;
        while ((n%5)==0)
            n/=5;
        
        if (n<2)
            break;
        
        orig_n++;
    }
    return orig_n;
}


t_ears_err ears_buffer_paulstretch(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, double stretchfactor, long framesize_samps, char spectral, bool precise_output_time)
{
    
    if (!source || !dest)
        return EARS_ERR_NO_BUFFER;
    
    t_ears_err err = EARS_ERR_NONE;
    float *orig_sample = buffer_locksamples(source);
    float *orig_sample_wk = NULL;
    double sr = ears_buffer_get_sr(ob, source);
    
    if (!orig_sample) {
        err = EARS_ERR_CANT_READ;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
    } else {
        t_atom_long    channelcount = buffer_getchannelcount(source);        // number of floats in a frame
        t_atom_long    framecount   = buffer_getframecount(source);            // number of floats long the buffer is for a single channel
        
        orig_sample_wk = (float *)bach_newptr(channelcount * framecount * sizeof(float));
        sysmem_copyptr(orig_sample, orig_sample_wk, channelcount * framecount * sizeof(float));
        buffer_unlocksamples(source);
        
        // checking stretch factor
        if (stretchfactor < PAULSTRETCH_MIN_STRETCH_FACTOR) {
            stretchfactor = PAULSTRETCH_MIN_STRETCH_FACTOR;
            object_warn(ob, "Stretch factor cannot be less than %f", PAULSTRETCH_MIN_STRETCH_FACTOR);
        }

        // make sure that windowsize is even and larger than 16
        if (framesize_samps<16)
            framesize_samps=16;
        framesize_samps=optimize_windowsize(framesize_samps);
        framesize_samps=(long)(framesize_samps/2)*2;
        long half_framesize_samps=(long)(framesize_samps/2);
        
        // correct the end of the smp by adding a little fade out
        long end_size = MAX(16, (long)(sr*0.05));
        for (long c = 0; c < channelcount; c++)
            for (long i = MAX(0, framecount-end_size); i < framecount; i++)
                orig_sample_wk[i*channelcount + c] *= rescale(i, framecount-end_size, framecount-1, 1., 0.);
        
        // compute the displacement inside the input file
        double start_pos = 0.;
        double displace_pos = 0;
        t_atom_long outframecount = 0;
        if (precise_output_time) {
            outframecount = long(round(stretchfactor * framecount));
            displace_pos = (double)framecount / (((double)(outframecount - framesize_samps))/half_framesize_samps + 1);
        } else {
            displace_pos = (framesize_samps*0.5)/stretchfactor; //Dx = framesize / 2*f
            outframecount = ((long)(ceil(framecount/displace_pos)-1))* half_framesize_samps + framesize_samps;
        }
        
        if (source == dest) { // inplace operation!
            ears_buffer_set_size_samps(ob, source, outframecount);
        } else {
            ears_buffer_copy_format_and_set_size_samps(ob, source, dest, outframecount);
        }
        
        // create window function
        float *window = (float *)bach_newptr(framesize_samps * sizeof(float));
        for (long i = 0; i < framesize_samps; i++)   // this window is not in the original paulstretch algorithm
            window[i] = sqrt(0.5 * (1 - cos(TWOPI * i / (framesize_samps - 1)))); // square root of hann should ensure perfect reconstruction
//            window[i] = pow(1 - pow(rescale(i, 0, framesize_samps - 1, -1., 1.), 2.), 1.25); // < this was the previously used window
        
        float *dest_sample = buffer_locksamples(dest);
        
        if (!dest_sample) {
            err = EARS_ERR_CANT_WRITE;
            object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
        } else {
            t_atom_long    dest_channelcount = buffer_getchannelcount(dest);
            t_atom_long    dest_framecount   = buffer_getframecount(dest);
            long dest_sample_size = dest_framecount * dest_channelcount;

            // fft buffers
            long nfft = framesize_samps;
            kiss_fft_cpx *fin = (kiss_fft_cpx *)bach_newptrclear(framesize_samps * sizeof(kiss_fft_cpx));
            kiss_fft_cpx *fout = (kiss_fft_cpx *)bach_newptr(framesize_samps * sizeof(kiss_fft_cpx));
            kiss_fft_cfg cfg = kiss_fft_alloc(nfft, false, NULL, NULL);
            kiss_fft_cfg cfginv = kiss_fft_alloc(nfft, true, NULL, NULL);

            // clearing destination buffer
            for (long c = 0; c < dest_channelcount; c++)
                for (long i = 0; i < dest_framecount; i++)
                    dest_sample[i * dest_channelcount + c] = 0;
            
            long n = 0;
            while (true) {
                // get the windowed buffer
                long istart_pos=(long)(floor(start_pos));
                for (long c = 0; c < channelcount; c++) {
                    for (long i = 0; i < framesize_samps; i++)
                        fin[i].r = (istart_pos+i >= framecount ? 0. : orig_sample_wk[(istart_pos+i)*channelcount + c]) * window[i];
                    
                    if (spectral) {
                        // performing FFT
                        bach_fft_kiss(cfg, nfft, false, fin, fout, false);
                        
                        // randomizing the phase
                        for (long i = 0; i < framesize_samps; i++)
                            fout[i] = polar_to_cpx(get_cpx_ampli(fout[i]), random_double_in_range(0., TWOPI));
                        
                        // performing inverse FFT
                        bach_fft_kiss(cfginv, nfft, true, fout, fin, false);
                        
                        // applying window again
                        for (long i = 0; i < framesize_samps; i++)
                            fin[i].r *= window[i];
                    }
                    
                    // then overlap-adding the window
                    for (long i = 0; i < framesize_samps; i++) {
                        long ii = (i + (n * half_framesize_samps)) * dest_channelcount + c;
                        if (ii < dest_sample_size)
                            dest_sample[ii] += fin[i].r;
                    }
                }
                
                start_pos += displace_pos;
                n++;
                if (start_pos >= framecount)
                    break;

            }
            
            free(cfg);
            free(cfginv);
            bach_freeptr(fin);
            bach_freeptr(fout);
            buffer_setdirty(dest);
            buffer_unlocksamples(dest);
        }
        
        bach_freeptr(orig_sample_wk);
        bach_freeptr(window);
    }
    
    return err;
}



t_ears_err ears_buffer_paulstretch_envelope(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, t_llll *stretchenv, long framesize_samps, char spectral, e_slope_mapping slopemapping, e_ears_timeunit factor_timeunit)
{
    if (!source || !dest)
        return EARS_ERR_NO_BUFFER;
    
    t_ears_err err = EARS_ERR_NONE;
    float *orig_sample = buffer_locksamples(source);
    float *orig_sample_wk = NULL;
    double sr = ears_buffer_get_sr(ob, source);
    
    if (!orig_sample) {
        err = EARS_ERR_CANT_READ;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
    } else {
        t_atom_long    channelcount = buffer_getchannelcount(source);        // number of floats in a frame
        t_atom_long    framecount   = buffer_getframecount(source);            // number of floats long the buffer is for a single channel
        
        orig_sample_wk = (float *)bach_newptr(channelcount * framecount * sizeof(float));
        sysmem_copyptr(orig_sample, orig_sample_wk, channelcount * framecount * sizeof(float));
        buffer_unlocksamples(source);
        
        //  make sure that windowsize is even and larger than 16
        if (framesize_samps<16)
            framesize_samps=16;
        framesize_samps=optimize_windowsize(framesize_samps);
        framesize_samps=(long)(framesize_samps/2)*2;
        long half_framesize_samps=(long)(framesize_samps/2);
        
        // correct the end of the smp by adding a little fade out
        long end_size = MAX(16, (long)(sr*0.05));
        for (long c = 0; c < channelcount; c++)
            for (long i = framecount-end_size; i < framecount; i++)
                orig_sample_wk[i*channelcount + c] *= rescale(i, framecount-end_size, framecount-1, 1., 0.);
        
        // compute output frame count
        t_ears_envelope_iterator eei = ears_envelope_iterator_create(stretchenv, 0., false, slopemapping);
        long n = 0;
        double start_pos = 0.;
        char have_warned = false;
        t_atom_long outframecount = half_framesize_samps;
        while (true) {
            outframecount += half_framesize_samps;

            double stretchfactor = ears_convert_timeunit(ears_envelope_iterator_walk_interp(&eei, start_pos, framecount), source, factor_timeunit, EARS_TIMEUNIT_DURATION_RATIO);
            // checking stretch factor
            if (stretchfactor < PAULSTRETCH_MIN_STRETCH_FACTOR) {
                stretchfactor = PAULSTRETCH_MIN_STRETCH_FACTOR;
                if (!have_warned) {
                    object_warn(ob, "Stretch factor cannot be less than %f", PAULSTRETCH_MIN_STRETCH_FACTOR);
                    have_warned = true;
                }
            }
            start_pos += (framesize_samps*0.5)/stretchfactor;
            n++;
            if (start_pos >= framecount)
                break;
        }
        
        start_pos = 0.;
        ears_envelope_iterator_reset(&eei);
        
        if (source == dest) { // inplace operation!
            ears_buffer_set_size_samps(ob, source, outframecount);
        } else {
            ears_buffer_copy_format_and_set_size_samps(ob, source, dest, outframecount);
        }
        
        // create window function
        float *window = (float *)bach_newptr(framesize_samps * sizeof(float));
        for (long i = 0; i < framesize_samps; i++)
            window[i] = pow(1 - pow(rescale(i, 0, framesize_samps - 1, -1., 1.), 2.), 1.25);
        
        float *dest_sample = buffer_locksamples(dest);
        
        if (!dest_sample) {
            err = EARS_ERR_CANT_WRITE;
            object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
        } else {
            // fft buffers
            long nfft = framesize_samps;
            kiss_fft_cpx *fin = (kiss_fft_cpx *)bach_newptrclear(framesize_samps * sizeof(kiss_fft_cpx));
            kiss_fft_cpx *fout = (kiss_fft_cpx *)bach_newptr(framesize_samps * sizeof(kiss_fft_cpx));
            kiss_fft_cfg cfg = kiss_fft_alloc(nfft, false, NULL, NULL);
            kiss_fft_cfg cfginv = kiss_fft_alloc(nfft, true, NULL, NULL);
            
            // clearing destination buffer
            for (long c = 0; c < channelcount; c++)
                for (long i = 0; i < outframecount; i++)
                    dest_sample[i * channelcount + c] = 0;
            
            long n = 0;
            while (true) {
                // get the windowed buffer
                long istart_pos=(long)(floor(start_pos));
                for (long c = 0; c < channelcount; c++) {
                    for (long i = 0; i < framesize_samps; i++)
                        fin[i].r = (istart_pos+i >= framecount ? 0. : orig_sample_wk[(istart_pos+i)*channelcount + c]) * window[i];
                    
                    if (spectral) {
                        // performing FFT
                        bach_fft_kiss(cfg, nfft, false, fin, fout, false);
                        
                        // randomizing the phase
                        for (long i = 0; i < framesize_samps; i++)
                            fout[i] = polar_to_cpx(get_cpx_ampli(fout[i]), random_double_in_range(0., TWOPI));
                        
                        // performing inverse FFT
                        bach_fft_kiss(cfginv, nfft, true, fout, fin, false);
                        
                        // applying window again
                        for (long i = 0; i < framesize_samps; i++)
                            fin[i].r *= window[i];
                    }
                    
                    // then overlap-adding the window
                    for (long i = 0; i < framesize_samps; i++)
                        dest_sample[(i + (n * half_framesize_samps)) * channelcount + c] += fin[i].r;
                }
                
                double stretchfactor = ears_convert_timeunit(ears_envelope_iterator_walk_interp(&eei, start_pos, framecount), source, factor_timeunit, EARS_TIMEUNIT_DURATION_RATIO);
//                double stretchfactor = ears_envelope_iterator_walk_interp(&eei, start_pos, framecount);
                
                if (stretchfactor < PAULSTRETCH_MIN_STRETCH_FACTOR)
                    stretchfactor = PAULSTRETCH_MIN_STRETCH_FACTOR;
                start_pos += (framesize_samps*0.5)/stretchfactor;
                n++;
                if (start_pos >= framecount)
                    break;
                
            }
            
            free(cfg);
            free(cfginv);
            bach_freeptr(fin);
            bach_freeptr(fout);
            buffer_setdirty(dest);
            buffer_unlocksamples(dest);
        }
        
        bach_freeptr(orig_sample_wk);
        bach_freeptr(window);
    }
    
    return err;
}


long random_long_in_range(long min_number, long max_number)
{
    if (min_number == max_number)
        return min_number;
    
    return rand() % (max_number + 1 - min_number) + min_number;
}

t_ears_err ears_buffer_paulfreeze(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, long onset_samps, long framesize_samps, long jitter_samps, long duration_samps, char spectral)
{
    
    if (!source || !dest)
        return EARS_ERR_NO_BUFFER;
    
    t_ears_err err = EARS_ERR_NONE;
    float *orig_sample = buffer_locksamples(source);
    float *orig_sample_wk = NULL;
    double sr = ears_buffer_get_sr(ob, source);
    
    if (!orig_sample) {
        err = EARS_ERR_CANT_READ;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
    } else {
        t_atom_long    channelcount = buffer_getchannelcount(source);        // number of floats in a frame
        t_atom_long    framecount   = buffer_getframecount(source);            // number of floats long the buffer is for a single channel
        
        orig_sample_wk = (float *)bach_newptr(channelcount * framecount * sizeof(float));
        sysmem_copyptr(orig_sample, orig_sample_wk, channelcount * framecount * sizeof(float));
        buffer_unlocksamples(source);
        
        // make sure that windowsize is even and larger than 16
        if (framesize_samps<16)
            framesize_samps=16;
        framesize_samps=optimize_windowsize(framesize_samps);
        framesize_samps=(long)(framesize_samps/2)*2;
        long half_framesize_samps=(long)(framesize_samps/2);
        
        // correct the end of the smp by adding a little fade out
        long end_size = MAX(16, (long)(sr*0.05));
        for (long c = 0; c < channelcount; c++)
            for (long i = MAX(0, framecount-end_size); i < framecount; i++)
                orig_sample_wk[i*channelcount + c] *= rescale(i, framecount-end_size, framecount-1, 1., 0.);
        
        t_atom_long outframecount = duration_samps;
        
        if (source == dest) { // inplace operation!
            ears_buffer_set_size_samps(ob, source, outframecount);
        } else {
            ears_buffer_copy_format_and_set_size_samps(ob, source, dest, outframecount);
        }
        
        // create window function
        float *window = (float *)bach_newptr(framesize_samps * sizeof(float));
        for (long i = 0; i < framesize_samps; i++)   // this window is not in the original paulstretch algorithm
            window[i] = sqrt(0.5 * (1 - cos(TWOPI * i / (framesize_samps - 1)))); // square root of hann should ensure perfect reconstruction
        //            window[i] = pow(1 - pow(rescale(i, 0, framesize_samps - 1, -1., 1.), 2.), 1.25); // < this was the previously used window
        
        float *dest_sample = buffer_locksamples(dest);
        long dest_sample_size = outframecount * channelcount;
        
        if (!dest_sample) {
            err = EARS_ERR_CANT_WRITE;
            object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
        } else {
            // fft buffers
            long nfft = framesize_samps;
            kiss_fft_cpx *fin = (kiss_fft_cpx *)bach_newptrclear(framesize_samps * sizeof(kiss_fft_cpx));
            kiss_fft_cpx *fout = (kiss_fft_cpx *)bach_newptr(framesize_samps * sizeof(kiss_fft_cpx));
            kiss_fft_cfg cfg = kiss_fft_alloc(nfft, false, NULL, NULL);
            kiss_fft_cfg cfginv = kiss_fft_alloc(nfft, true, NULL, NULL);
            
            // clearing destination buffer
            for (long c = 0; c < channelcount; c++)
                for (long i = 0; i < outframecount; i++)
                    dest_sample[i * channelcount + c] = 0;
            
            long n = 0;
            while (true) {
                bool must_break = false;
                
                // get the windowed buffer
                long istart_pos= onset_samps + random_long_in_range(0, jitter_samps);
                for (long c = 0; c < channelcount; c++) {
                    for (long i = 0; i < framesize_samps; i++)
                        fin[i].r = (istart_pos+i >= framecount ? 0. : orig_sample_wk[(istart_pos+i)*channelcount + c]) * window[i];
                    
                    if (spectral) {
                        // performing FFT
                        bach_fft_kiss(cfg, nfft, false, fin, fout, false);
                        
                        // randomizing the phase
                        for (long i = 0; i < framesize_samps; i++)
                            fout[i] = polar_to_cpx(get_cpx_ampli(fout[i]), random_double_in_range(0., TWOPI));
                        
                        // performing inverse FFT
                        bach_fft_kiss(cfginv, nfft, true, fout, fin, false);
                        
                        // applying window again
                        for (long i = 0; i < framesize_samps; i++)
                            fin[i].r *= window[i];
                    }
                    
                    // then overlap-adding the window
                    for (long i = 0; i < framesize_samps; i++) {
                        long ii = (i + (n * half_framesize_samps)) * channelcount + c;
                        if (ii < dest_sample_size)
                            dest_sample[ii] += fin[i].r;
                        else
                            must_break = true;
                    }
                }
                
                n++;
                if (must_break)
                    break;
                
            }
            
            free(cfg);
            free(cfginv);
            bach_freeptr(fin);
            bach_freeptr(fout);
            buffer_setdirty(dest);
            buffer_unlocksamples(dest);
        }
        
        bach_freeptr(orig_sample_wk);
        bach_freeptr(window);
    }
    
    return err;
}




long array_argmin(double *val, long size)
{
    if (size == 0)
        return -1;
    
    double minval = val[0];
    long minarg = 0;
    for (long i = 1; i < size; i++) {
        if (val[i] < minval) {
            minval = val[i];
            minarg = i;
        }
    }
    return minarg;
}

long argmin3(double val1, double val2, double val3, long idx1, long idx2, long idx3)
{
    if (val2 <= val1 && val2 <= val3)
        return idx2;

    if (val1 <= val2 && val1 <= val3)
        return idx1;
    
    if (val3 <= val1 && val3 <= val2)
        return idx3;
    
    return idx1;
}


double unwrapped_phase_avg(double phase1, double phase2)
{
    while (phase2 > phase1 + PI)
        phase2 -= TWOPI;
    while (phase2 < phase1 - PI)
        phase2 += TWOPI;
    return phase1*0.5 + phase2*0.5;
}

// phase_handling = 0: none; 1: compensate; 2: griffin-lim
t_ears_err ears_buffer_spectral_seam_carve(t_object *ob, long num_channels, t_buffer_obj **amplitudes, t_buffer_obj **phases, t_buffer_obj **out_amplitudes, t_buffer_obj **out_phases, t_buffer_obj *energy_map, t_buffer_obj *seam_path, long delta_num_frames, double framesize_samps, double hopsize_samps, long energy_mode, updateprogress_fn update_progress, long phase_handling, double weighting_amount, double weighting_numframes_stdev,
   // only used for griffin-lim;
   long fullspectrum, long winleftalign, long unitary, long num_griffin_lim_iter, long griffin_lim_invalidate_width, bool griffin_lim_vertical, bool griffin_lim_randomize, const char *analysiswintype, const char *synthesiswintype)
{
    bool verbose = false;

    if (num_channels == 0)
        return EARS_ERR_NO_BUFFER;
    
    for (long i = 0; i < num_channels; i++) {
        if (!amplitudes[i] || !phases[i])
            return EARS_ERR_NO_BUFFER;
    }
    
    long num_frames = ears_buffer_get_size_samps(ob, amplitudes[0]);
    
    if (delta_num_frames + num_frames <= 0) {
        object_warn(ob, "No more frames left; empty output buffer!");
        for (long i = 0; i < num_channels; i++) {
            ears_buffer_clear(ob, amplitudes[i]);
            ears_buffer_clear(ob, phases[i]);
        }
        if (seam_path)
            ears_buffer_clear(ob, seam_path);
        if (energy_map)
            ears_buffer_clear(ob, energy_map);
        return EARS_ERR_GENERIC;
    }
    
    long orig_num_frames = num_frames;
    long orig_delta_num_frames = delta_num_frames;
    for (long i = 1; i < num_channels; i++) {
        if (ears_buffer_get_size_samps(ob, amplitudes[i]) != num_frames) {
            object_error(ob, "Size mismatch in frame sizes across channels");
            return EARS_ERR_SIZE_MISMATCH;
        }
    }

    long num_bins = ears_buffer_get_numchannels(ob, amplitudes[0]);
    for (long i = 1; i < num_channels; i++) {
        if (ears_buffer_get_numchannels(ob, amplitudes[i]) != num_bins) {
            object_error(ob, "Size mismatch in number of bins across channels");
            return EARS_ERR_SIZE_MISMATCH;
        }
    }
    
    long num_out_frames = num_frames + delta_num_frames;
    long num_alloc_frames = MAX(num_frames, num_out_frames);
    
    if (num_out_frames < 0) {
        object_error(ob, "Carving too many frames, empty buffer.");
        return EARS_ERR_GENERIC;
    }
    
    t_ears_err err = EARS_ERR_NONE;
    double *energymap = (double *)bach_newptr(num_bins * num_alloc_frames * sizeof(double));
    double *energymapcumul = (double *)bach_newptr(num_bins * num_alloc_frames * sizeof(double));
//    double energymap[num_bins][num_alloc_frames]; // @ANDREA, I KNOW...  BUT LET ME DO THIS LIKE THIS, I NEED TO EXPLORE! THEN I'll CHANGE IT; I PROMISE ;-)
    
    double *weights = (double *)bach_newptr(num_bins * num_alloc_frames * sizeof(double));
    for (long i = 0; i < num_bins * num_alloc_frames; i++)
        weights[i] = 1.;

    t_buffer_obj *mask_buf = NULL;
    float *glmask = (float *)bach_newptr(num_bins * num_alloc_frames * sizeof(float));
    for (long i = 0; i < num_bins * num_alloc_frames; i++)
        glmask[i] = 1.;

    // 1) lock all the samples, cloning them
    long num_amps_locked = 0, num_phases_locked = 0;
    float **amps_samples = (float **)bach_newptr(num_channels * sizeof(float *));
    float **phases_samples = (float **)bach_newptr(num_channels * sizeof(float *));
    float *carvingpath_samps = NULL, *energymapout_samps = NULL;
    for (long c = 0; c < num_channels; c++) {
        float *temp = buffer_locksamples(amplitudes[c]);
        if (!temp) {
            err = EARS_ERR_CANT_WRITE;
            object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
            goto end;
        }
        amps_samples[c] = (float *)bach_newptr(num_alloc_frames * num_bins * sizeof(float));
        bach_copyptr(temp, amps_samples[c], num_alloc_frames * num_bins * sizeof(float));
        buffer_unlocksamples(amplitudes[c]);
        num_amps_locked++;
        
        temp = buffer_locksamples(phases[c]);
        if (!temp) {
            err = EARS_ERR_CANT_WRITE;
            object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
            goto end;
        }
        phases_samples[c] = (float *)bach_newptr(num_alloc_frames * num_bins * sizeof(float));
        bach_copyptr(temp, phases_samples[c], num_alloc_frames * num_bins * sizeof(float));
        buffer_unlocksamples(phases[c]);
        num_phases_locked++;
    }
    
    // prepare carving path buffer to contain data
    if (seam_path) {
        ears_buffer_copy_format((t_object *)ob, amplitudes[0], seam_path, true);
        ears_buffer_clear((t_object *)ob, seam_path);
        ears_buffer_set_size_and_numchannels((t_object *)ob, seam_path, num_alloc_frames, num_bins);
        carvingpath_samps = buffer_locksamples(seam_path);
        if (!carvingpath_samps || buffer_getframecount(seam_path) != num_alloc_frames || buffer_getchannelcount(seam_path) != num_bins) {
            err = EARS_ERR_CANT_WRITE;
            object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
            goto end;
        }
    }
    
    // prepare energy map buffer to contain data
    if (energy_map) {
        ears_buffer_copy_format((t_object *)ob, amplitudes[0], energy_map, true);
        ears_buffer_clear((t_object *)ob, energy_map);
        ears_buffer_set_size_and_numchannels((t_object *)ob, energy_map, num_frames, num_bins);
        energymapout_samps = buffer_locksamples(energy_map);
        if (!energymapout_samps || buffer_getframecount(energy_map) != num_frames || buffer_getchannelcount(energy_map) != num_bins) {
            err = EARS_ERR_CANT_WRITE;
            object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
            goto end;
        }
    }

    
    // 1) get energy map
    for (long b = 0; b < num_bins; b++) {
        for (long f = 0; f < num_frames; f++) {
            energymap[f*num_bins + b] = 0;
            switch (energy_mode) {
                case EARS_SEAM_CARVE_MODE_MAGNITUDE:
                    for (long c = 0; c < num_channels; c++)
                        energymap[f*num_bins + b] += amps_samples[c][f*num_bins + b] * weights[f * num_bins + b];
                    break;
                    
                case EARS_SEAM_CARVE_MODE_GRADIENT_MAGNITUDE:
                default:
                    for (long c = 0; c < num_channels; c++)
                        energymap[f*num_bins + b] += (fabs(amps_samples[c][(f == num_frames - 1 ? f-1 : f+1)*num_bins + b]-amps_samples[c][f*num_bins + b]) + fabs(amps_samples[c][f*num_bins + (b == num_bins - 1 ? b-1 : b+1)]-amps_samples[c][f*num_bins + b])) * weights[f * num_bins + b];
                    break;
                    
            }
            energymap[f*num_bins + b] /= num_channels; // not really needed though
        }
    }
    
    // 2) compute cumulative energies
    for (long f = 0; f < num_frames; f++) {
        energymapcumul[f*num_bins + 0] = energymap[f*num_bins + 0];
    }
    for (long b = 1; b < num_bins; b++) {
        for (long f = 0; f < num_frames; f++) {
            energymapcumul[f*num_bins + b] = energymap[f*num_bins + b] + MIN(MIN(energymapcumul[(f>0 ? f-1 : f) * num_bins + b-1], energymapcumul[f*num_bins + b-1]), energymapcumul[(f<num_frames-1 ? f+1 : f)*num_bins + b-1]);
        }
    }
    
    // write energymap to output, but only for the first iteration
    if (energymapout_samps) {
        for (long b = 0; b < num_bins; b++) {
            for (long f = 0; f < num_frames; f++) {
                energymapout_samps[f * num_bins + b] = energymapcumul[f * num_bins + b];
            }
        }
    }
    
    while (true) {
        
        if (verbose) {
            post("weights");
            t_llll *ll4 = double_array_to_llll(weights, num_bins * num_frames);
            llll_print(ll4);
            llll_free(ll4);
            
            post("energymap");
            t_llll *ll1 = double_array_to_llll(energymap, num_bins * num_frames);
            llll_print(ll1);
            llll_free(ll1);
            
            post("cumulativeenergies");
            t_llll *ll = double_array_to_llll(energymapcumul, num_bins * num_frames);
            llll_print(ll);
            llll_free(ll);
        }

        
        if (update_progress) {
            (update_progress)(ob, 1.-fabs((double)delta_num_frames)/fabs(orig_delta_num_frames));
        }
        
        if (delta_num_frames == 0) // we're done
            break;
        
        // 3) find carving
        long carve[num_bins];
        // find minimum at num_bins-1
        double curr_min = energymapcumul[0*num_bins + num_bins - 1];
        long curr_min_arg = 0;
        for (long f = 1; f < num_frames; f++) {
            if (energymapcumul[f*num_bins + num_bins - 1] < curr_min) {
                curr_min = energymapcumul[f*num_bins + num_bins - 1];
                curr_min_arg = f;
            }
        }

        bool ONLY_STRAIGHT_CUTS = false; // < this is sometimes set to true just to test
        carve[num_bins - 1] = curr_min_arg;
        for (long b = num_bins - 2; b >= 0; b--) {
            if (ONLY_STRAIGHT_CUTS) {
                carve[b] = carve[num_bins-1];
            } else {
                long cb = carve[b+1];
                double v1 = energymapcumul[cb*num_bins + b];
                double v2 = energymapcumul[(cb > 0 ? cb-1 : cb)*num_bins + b];
                double v3 = energymapcumul[(cb < num_frames-1 ? cb+1 : cb)*num_bins + b];
                carve[b] = argmin3(v1, v2, v3, cb, cb > 0 ? cb-1 : cb, cb < num_frames-1 ? cb+1 : cb);
            }
        }
        
        
        if (verbose) {
            t_llll *ll = llll_get();
            for (long b = 0; b < num_bins; b++)
                llll_appendlong(ll, carve[b]);
            post("carved indices:");
            llll_print(ll);
        }

        // writing carving data to output buffer
        if (carvingpath_samps) {
            for (long b = 0; b < num_bins; b++) {
                long thisframe = carve[b] - carvingpath_samps[carve[b] * num_bins + b];
                if (thisframe < 0 || thisframe >= num_alloc_frames) {
                    object_error((t_object *)ob, "Internal error!");
                }
                if (delta_num_frames > 0) {
                    for (long f = thisframe+1; f < num_alloc_frames; f++)
                        carvingpath_samps[f * num_bins + b] += 1;
                } else {
                    for (long f = thisframe; f < num_alloc_frames; f++)
                        carvingpath_samps[f * num_bins + b] -= 1;
                }
            }
        }
            
        // 4) updating weights
        if (weighting_amount != 0) {
            for (long b = 0; b < num_bins; b++) {
                for (long f = MAX(0, (long)(carve[b]-5*weighting_numframes_stdev)); f < MIN(num_frames, (long)(carve[b]+5*weighting_numframes_stdev)); f++) {
                    weights[f * num_bins + b] += weighting_amount * exp(-((f-carve[b])*(f - carve[b]))/(2 * weighting_numframes_stdev*weighting_numframes_stdev));
                    //            weights[f] += 1./(weighting_numframes_stdev * abs(f - pivot_upper_f));
                }
            }
        }
        
        // 5) apply seam carving
        if (delta_num_frames > 0) {
            // adding a seam
            for (long i = 0; i < num_bins; i++) {
                for (long c = 0; c < num_channels; c++) {
                    long pivot_f = carve[i];
                    double phase_shift = (phase_handling == 1) * (pivot_f < num_frames - 1 ? phases_samples[c][(pivot_f + 1)*num_bins + i] - phases_samples[c][pivot_f*num_bins + i] : 0);
                    for (long f = num_frames; f >= pivot_f; f--) {
                        amps_samples[c][(f+1)*num_bins + i] = amps_samples[c][f*num_bins + i];
                        phases_samples[c][(f+1)*num_bins + i] = positive_fmod(phases_samples[c][f*num_bins + i] + phase_shift, TWOPI); // shifting phases to account for time translation
                        if (c == num_channels - 1) {
                            glmask[(f+1)*num_bins + i] = glmask[f*num_bins + i];
                            weights[(f+1)*num_bins + i] = weights[f*num_bins + i];
                            energymap[(f+1)*num_bins + i] = energymap[f*num_bins + i];
                            energymapcumul[(f+1)*num_bins + i] = energymapcumul[f*num_bins + i];
                        }
                    }
                    if (pivot_f > 0) {
                        amps_samples[c][pivot_f*num_bins + i] = (amps_samples[c][(pivot_f+1)*num_bins + i] + amps_samples[c][(pivot_f-1)*num_bins + i])/2.;
                        phases_samples[c][pivot_f*num_bins + i] = random_double_in_range(0, TWOPI); // unwrapped_phase_avg(phases_samples[c][(pivot_f+1)*num_bins + i], phases_samples[c][(pivot_f-1)*num_bins + i]); //
                        if (c == num_channels - 1) {
                            weights[pivot_f*num_bins + i] = 0.5 * (weights[(pivot_f+1)*num_bins + i] + weights[(pivot_f-1)*num_bins + i]);
                            energymap[pivot_f*num_bins + i] = 0.5 * (energymap[(pivot_f+1)*num_bins + i] + energymap[(pivot_f-1)*num_bins + i]);
                            energymapcumul[pivot_f*num_bins + i] = 0.5 * (energymapcumul[(pivot_f+1)*num_bins + i] + energymapcumul[(pivot_f-1)*num_bins + i]);
                        }
                    } else if (pivot_f == 0) {
                        amps_samples[c][pivot_f*num_bins + i] = amps_samples[c][(pivot_f+1)*num_bins + i] ;
                        phases_samples[c][pivot_f*num_bins + i] = random_double_in_range(0, TWOPI);
                        if (c == num_channels - 1) {
                            weights[pivot_f*num_bins + i] = weights[(pivot_f+1)*num_bins + i];
                            energymap[pivot_f*num_bins + i] = energymap[(pivot_f+1)*num_bins + i];
                            energymapcumul[pivot_f*num_bins + i] = energymapcumul[(pivot_f+1)*num_bins + i];
                        }
                    }
                    if (c == num_channels - 1 && phase_handling == 2) {
                        long gl_start_f = MAX(0, pivot_f-griffin_lim_invalidate_width);
                        long gl_end_f = MIN(num_frames,pivot_f+griffin_lim_invalidate_width+1);
                        if (griffin_lim_vertical) {
                            for (long j = 0; j < num_bins; j++) {
                                for (long f = gl_start_f; f < gl_end_f; f++) {
                                    glmask[f*num_bins + j] = 0;
                                }
                            }
                        } else {
                            for (long f = gl_start_f; f < gl_end_f; f++) {
                                glmask[f*num_bins + i] = 0;
                            }
                        }
                    }
                }
            }

            num_frames++;
            delta_num_frames--;
        } else {
            // deleting a seam
            for (long i = 0; i < num_bins; i++) {
                for (long c = 0; c < num_channels; c++) {
                    long pivot_f = carve[i];
                    double phase_shift = (phase_handling == 1) * (pivot_f < num_frames - 1 ? phases_samples[c][(pivot_f + 1)*num_bins + i] - phases_samples[c][pivot_f*num_bins + i] : 0);
                    for (long f = pivot_f; f < num_frames - 1; f++) {
                        amps_samples[c][f*num_bins + i] = amps_samples[c][(f+1)*num_bins + i];
                        phases_samples[c][f*num_bins + i] = positive_fmod(phases_samples[c][(f+1)*num_bins + i] - phase_shift, TWOPI); // shifting phases to account for time translation
                        if (c == num_channels - 1) {
                            glmask[f*num_bins + i] = glmask[(f+1)*num_bins + i];
                            weights[f*num_bins + i] = weights[(f+1)*num_bins + i];
                            energymap[f*num_bins + i] = energymap[(f+1)*num_bins + i];
                            energymapcumul[f*num_bins + i] = energymapcumul[(f+1)*num_bins + i];
                        }
                    }
                    if (c == num_channels - 1 && phase_handling == 2) {
                        long gl_start_f = MAX(0, pivot_f-griffin_lim_invalidate_width);
                        long gl_end_f = MIN(num_frames,pivot_f+griffin_lim_invalidate_width);
                        if (griffin_lim_vertical) {
                            for (long j = 0; j < num_bins; j++) {
                                for (long f = gl_start_f; f < gl_end_f; f++) {
                                    glmask[f*num_bins + j] = 0;
                                }
                            }
                        } else {
                            for (long f = gl_start_f; f < gl_end_f; f++) {
                                glmask[f*num_bins + i] = 0;
                            }
                        }
                    }

                }
            }
            
            num_frames--;
            delta_num_frames++;
        }
        
        // recomputing invalidated region of energy map cumul
        long inv_f_start = carve[0];
        long inv_f_end = carve[0] - 1;
        for (long b = 1; b < num_bins; b++) {
            inv_f_start -= 1;
            inv_f_end += 1;
            
            inv_f_start = CLAMP(inv_f_start, 0, num_frames-1);
            inv_f_end = CLAMP(inv_f_end, 0, num_frames-1);
            for (long f = inv_f_start; f <= inv_f_end; f++) {
                energymapcumul[f*num_bins + b] = energymap[f*num_bins + b] + MIN(MIN(energymapcumul[(f>0 ? f-1 : f) * num_bins + b-1], energymapcumul[f*num_bins + b-1]), energymapcumul[(f<num_frames-1 ? f+1 : f)*num_bins + b-1]);
            }
            
            inv_f_start = MIN(inv_f_start, carve[b]);
            inv_f_end = MAX(inv_f_end, carve[b]-1);
        }

    }
    
    if (carvingpath_samps) {
        for (long b = 0; b < num_bins; b++) {
            long prev = 0;
            for (long f = 0; f < num_frames; f++) {
                long temp = carvingpath_samps[f * num_bins + b];
                if (temp != prev)
                    carvingpath_samps[f * num_bins + b] = 1;
                else
                    carvingpath_samps[f * num_bins + b] = 0;
                prev = temp;
            }
        }
    }
    
    
    bach_freeptr(energymap);
    bach_freeptr(energymapcumul);
    bach_freeptr(weights);

    if (phase_handling == 2) {
        mask_buf = ears_buffer_make(NULL);
        ears_buffer_set_size_and_numchannels(ob, mask_buf, num_out_frames, num_bins);
        float *temp = buffer_locksamples(mask_buf);
        if (!temp || ears_buffer_get_numchannels(ob, mask_buf) != num_bins || ears_buffer_get_size_samps(ob, mask_buf) != num_out_frames) {
            err = EARS_ERR_CANT_WRITE;
            object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
            buffer_unlocksamples(mask_buf);
            ears_buffer_free(mask_buf);
            mask_buf = NULL;
        } else {
            bach_copyptr(glmask, temp, num_out_frames * num_bins * sizeof(float));
            buffer_unlocksamples(mask_buf);
        }
    }

    
    for (long c = 0; c < num_channels; c++) {
        ears_buffer_copy_format_and_set_size_samps(ob, amplitudes[c], out_amplitudes[c], num_out_frames);
        ears_buffer_copy_format_and_set_size_samps(ob, phases[c], out_phases[c], num_out_frames);
        
        float *temp = buffer_locksamples(out_amplitudes[c]);
        if (!temp || ears_buffer_get_numchannels(ob, out_amplitudes[c]) != num_bins || ears_buffer_get_size_samps(ob, out_amplitudes[c]) != num_out_frames) {
            err = EARS_ERR_CANT_WRITE;
            object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
            goto end;
        }
        
        bach_copyptr(amps_samples[c], temp, num_out_frames * num_bins * sizeof(float));
        buffer_setdirty(out_amplitudes[c]);
        buffer_unlocksamples(out_amplitudes[c]);
        
        temp = buffer_locksamples(out_phases[c]);
        if (!temp || ears_buffer_get_numchannels(ob, out_phases[c]) != num_bins || ears_buffer_get_size_samps(ob, out_phases[c]) != num_out_frames) {
            err = EARS_ERR_CANT_WRITE;
            object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
            goto end;
        }
        bach_copyptr(phases_samples[c], temp, num_out_frames * num_bins * sizeof(float));
        buffer_setdirty(out_phases[c]);
        buffer_unlocksamples(out_phases[c]);
        
        if (mask_buf) {
            // griffin lim
            ears_buffer_griffinlim(ob, 1, &(out_amplitudes[c]), &(out_phases[c]), &mask_buf, NULL, analysiswintype, synthesiswintype, fullspectrum, winleftalign, unitary, num_griffin_lim_iter, griffin_lim_randomize);
        }
    }
    
    bach_freeptr(glmask);

    
    if (seam_path) {
        buffer_setdirty(seam_path);
        buffer_unlocksamples(seam_path);
        ears_buffer_set_size_samps(ob, seam_path, orig_num_frames);
    }
//    ears_buffer_set_size_samps_preserve(ob, seam_path, orig_num_frames);
    if (energy_map) {
        buffer_setdirty(energy_map);
        buffer_unlocksamples(energy_map);
    }

end:
    for (long c = 0; c < num_amps_locked; c++) {
        bach_freeptr(amps_samples[c]);
    }
    for (long c = 0; c < num_phases_locked; c++) {
        bach_freeptr(phases_samples[c]);
    }

    bach_freeptr(amps_samples);
    bach_freeptr(phases_samples);

    return err;
}




/*
t_ears_err ears_buffer_spectral_seam_carve_orig(t_object *ob, long num_channels, t_buffer_obj **amplitudes, t_buffer_obj **phases, t_buffer_obj **out_amplitudes, t_buffer_obj **out_phases, t_buffer_obj *energy_map, t_buffer_obj *seam_path, long delta_num_frames, double framesize_samps, double hopsize_samps, long energy_mode, updateprogress_fn update_progress, double compensate_phases, double weighting_amount, double weighting_numframes_stdev)
{
    bool verbose = false;

    if (num_channels == 0)
        return EARS_ERR_NO_BUFFER;
    
    for (long i = 0; i < num_channels; i++) {
        if (!amplitudes[i] || !phases[i])
            return EARS_ERR_NO_BUFFER;
    }
    
    long num_frames = ears_buffer_get_size_samps(ob, amplitudes[0]);
    
    if (delta_num_frames + num_frames <= 0) {
        object_warn(ob, "No more frames left; empty output buffer!");
        for (long i = 0; i < num_channels; i++) {
            ears_buffer_clear(ob, amplitudes[i]);
            ears_buffer_clear(ob, phases[i]);
        }
        if (seam_path)
            ears_buffer_clear(ob, seam_path);
        if (energy_map)
            ears_buffer_clear(ob, energy_map);
        return EARS_ERR_GENERIC;
    }
    
    long orig_num_frames = num_frames;
    long orig_delta_num_frames = delta_num_frames;
    for (long i = 1; i < num_channels; i++) {
        if (ears_buffer_get_size_samps(ob, amplitudes[i]) != num_frames) {
            object_error(ob, "Size mismatch in frame sizes across channels");
            return EARS_ERR_SIZE_MISMATCH;
        }
    }

    long num_bins = ears_buffer_get_numchannels(ob, amplitudes[0]);
    for (long i = 1; i < num_channels; i++) {
        if (ears_buffer_get_numchannels(ob, amplitudes[i]) != num_bins) {
            object_error(ob, "Size mismatch in number of bins across channels");
            return EARS_ERR_SIZE_MISMATCH;
        }
    }
    
    long num_out_frames = num_frames + delta_num_frames;
    long num_alloc_frames = MAX(num_frames, num_out_frames);
    
    if (num_out_frames < 0) {
        object_error(ob, "Carving too many frames, empty buffer.");
        return EARS_ERR_GENERIC;
    }
    
    t_ears_err err = EARS_ERR_NONE;
    double *energymap = (double *)bach_newptr(num_bins * num_alloc_frames * sizeof(double));
//    double energymap[num_bins][num_alloc_frames]; // @ANDREA, I KNOW...  BUT LET ME DO THIS LIKE THIS, I NEED TO EXPLORE! THEN I'll CHANGE IT; I PROMISE ;-)
    
    double *weights = (double *)bach_newptr(num_bins * num_alloc_frames * sizeof(double));
    for (long i = 0; i < num_bins * num_alloc_frames; i++)
        weights[i] = 1.;
    
    // 1) lock all the samples, cloning them
    long num_amps_locked = 0, num_phases_locked = 0;
    float **amps_samples = (float **)bach_newptr(num_channels * sizeof(float *));
    float **phases_samples = (float **)bach_newptr(num_channels * sizeof(float *));
    float *carvingpath_samps = NULL, *energymapout_samps = NULL;
    bool firsttime = true;

    for (long c = 0; c < num_channels; c++) {
        float *temp = buffer_locksamples(amplitudes[c]);
        if (!temp) {
            err = EARS_ERR_CANT_WRITE;
            object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
            goto end;
        }
        amps_samples[c] = (float *)bach_newptr(num_alloc_frames * num_bins * sizeof(float));
        bach_copyptr(temp, amps_samples[c], num_alloc_frames * num_bins * sizeof(float));
        buffer_unlocksamples(amplitudes[c]);
        num_amps_locked++;
        
        temp = buffer_locksamples(phases[c]);
        if (!temp) {
            err = EARS_ERR_CANT_WRITE;
            object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
            goto end;
        }
        phases_samples[c] = (float *)bach_newptr(num_alloc_frames * num_bins * sizeof(float));
        bach_copyptr(temp, phases_samples[c], num_alloc_frames * num_bins * sizeof(float));
        buffer_unlocksamples(phases[c]);
        num_phases_locked++;
    }
    
    // prepare carving path buffer to contain data
    if (seam_path) {
        ears_buffer_copy_format((t_object *)ob, amplitudes[0], seam_path, true);
        ears_buffer_clear((t_object *)ob, seam_path);
        ears_buffer_set_size_and_numchannels((t_object *)ob, seam_path, num_alloc_frames, num_bins);
        carvingpath_samps = buffer_locksamples(seam_path);
        if (!carvingpath_samps || buffer_getframecount(seam_path) != num_alloc_frames || buffer_getchannelcount(seam_path) != num_bins) {
            err = EARS_ERR_CANT_WRITE;
            object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
            goto end;
        }
    }
    
    // prepare energy map buffer to contain data
    if (energy_map) {
        ears_buffer_copy_format((t_object *)ob, amplitudes[0], energy_map, true);
        ears_buffer_clear((t_object *)ob, energy_map);
        ears_buffer_set_size_and_numchannels((t_object *)ob, energy_map, num_frames, num_bins);
        energymapout_samps = buffer_locksamples(energy_map);
        if (!energymapout_samps || buffer_getframecount(energy_map) != num_frames || buffer_getchannelcount(energy_map) != num_bins) {
            err = EARS_ERR_CANT_WRITE;
            object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
            goto end;
        }
    }

    while (true) {
        if (verbose) {
            post("weights");
            t_llll *ll4 = double_array_to_llll(weights, num_bins * num_frames);
            llll_print(ll4);
            llll_free(ll4);
        }

        // 1) get energy map
        for (long b = 0; b < num_bins; b++) {
            for (long f = 0; f < num_frames; f++) {
                energymap[f*num_bins + b] = 0;
                switch (energy_mode) {
                    case EARS_SEAM_CARVE_MODE_MAGNITUDE:
                        for (long c = 0; c < num_channels; c++)
                            energymap[f*num_bins + b] += amps_samples[c][f*num_bins + b] * weights[f * num_bins + b];
                        break;
                        
                    case EARS_SEAM_CARVE_MODE_GRADIENT_MAGNITUDE:
                    default:
                        for (long c = 0; c < num_channels; c++)
                            energymap[f*num_bins + b] += (fabs(amps_samples[c][(f == num_frames - 1 ? f-1 : f+1)*num_bins + b]-amps_samples[c][f*num_bins + b]) + fabs(amps_samples[c][f*num_bins + (b == num_bins - 1 ? b-1 : b+1)]-amps_samples[c][f*num_bins + b])) * weights[f * num_bins + b];
                        break;
                        
                }
                energymap[f*num_bins + b] /= num_channels; // not really needed though
            }
        }
        
        if (verbose) {
            post("energymap");
            t_llll *ll1 = double_array_to_llll(energymap, num_bins * num_frames);
            llll_print(ll1);
            llll_free(ll1);
        }
        

        // 2) compute cumulative energies
        for (long b = 1; b < num_bins; b++) {
            for (long f = 0; f < num_frames; f++) {
                energymap[f*num_bins + b] += MIN(MIN(energymap[(f>0 ? f-1 : f) * num_bins + b-1], energymap[f*num_bins + b-1]), energymap[(f<num_frames-1 ? f+1 : f)*num_bins + b-1]);
            }
        }
        
        if (verbose) {
            post("cumulativeenergies");
            t_llll *ll = double_array_to_llll(energymap, num_bins * num_frames);
            llll_print(ll);
            llll_free(ll);
        }
        
        // write energymap to output, but only for the first iteration
        if (firsttime && energymapout_samps) {
            for (long b = 0; b < num_bins; b++) {
                for (long f = 0; f < num_frames; f++) {
                    energymapout_samps[f * num_bins + b] = energymap[f * num_bins + b];
                }
            }
        }
        
        if (update_progress) {
            (update_progress)(ob, 1.-fabs((double)delta_num_frames)/fabs(orig_delta_num_frames));
        }
        
        if (delta_num_frames == 0) // we're done
            break;
        
        // weight by uniformity
        //
        
        // 3) find carving
        long carve[num_bins];
        // find minimum at num_bins-1
        double curr_min = energymap[0*num_bins + num_bins - 1];
        long curr_min_arg = 0;
        for (long f = 1; f < num_frames; f++) {
            if (energymap[f*num_bins + num_bins - 1] < curr_min) {
                curr_min = energymap[f*num_bins + num_bins - 1];
                curr_min_arg = f;
            }
        }
        

        bool ONLY_STRAIGHT_CUTS = false; // < this is sometimes set to true just to test
        carve[num_bins - 1] = curr_min_arg;
        for (long b = num_bins - 2; b >= 0; b--) {
            if (ONLY_STRAIGHT_CUTS) {
                carve[b] = carve[num_bins-1];
            } else {
                long cb = carve[b+1];
                double v1 = energymap[cb*num_bins + b];
                double v2 = energymap[(cb > 0 ? cb-1 : cb)*num_bins + b];
                double v3 = energymap[(cb < num_frames-1 ? cb+1 : cb)*num_bins + b];
                carve[b] = argmin3(v1, v2, v3, cb, cb > 0 ? cb-1 : cb, cb < num_frames-1 ? cb+1 : cb);
            }
        }
        
        if (verbose) {
            t_llll *ll = llll_get();
            for (long b = 0; b < num_bins; b++)
                llll_appendlong(ll, carve[b]);
            post("carved indices:");
            llll_print(ll);
        }

        
        // writing carving data to output buffer
        if (carvingpath_samps) {
            for (long b = 0; b < num_bins; b++) {
                long thisframe = carve[b] - carvingpath_samps[carve[b] * num_bins + b];
                if (thisframe < 0 || thisframe >= num_alloc_frames) {
                    object_error((t_object *)ob, "Internal error!");
                }
                if (delta_num_frames > 0) {
                    for (long f = thisframe+1; f < num_alloc_frames; f++)
                        carvingpath_samps[f * num_bins + b] += 1;
                } else {
                    for (long f = thisframe; f < num_alloc_frames; f++)
                        carvingpath_samps[f * num_bins + b] -= 1;
                }
            }
        }
            
        // 4) updating weights
        if (weighting_amount != 0) {
            for (long b = 0; b < num_bins; b++) {
                for (long f = 0; f < num_frames; f++) {
                    weights[f * num_bins + b] += weighting_amount * exp(-((f-carve[b])*(f - carve[b]))/(2 * weighting_numframes_stdev*weighting_numframes_stdev));
                    //            weights[f] += 1./(weighting_numframes_stdev * abs(f - pivot_upper_f));
                }
            }
        }
        
        // 5) apply seam carving
        if (delta_num_frames > 0) {
            // adding a seam
            for (long i = 0; i < num_bins; i++) {
                for (long c = 0; c < num_channels; c++) {
                    long pivot_f = carve[i];
                    double phase_shift = compensate_phases * (pivot_f < num_frames - 1 ? phases_samples[c][(pivot_f + 1)*num_bins + i] - phases_samples[c][pivot_f*num_bins + i] : 0);
                    for (long f = num_frames; f >= pivot_f; f--) {
                        amps_samples[c][(f+1)*num_bins + i] = amps_samples[c][f*num_bins + i];
                        phases_samples[c][(f+1)*num_bins + i] = positive_fmod(phases_samples[c][f*num_bins + i] + phase_shift, TWOPI); // shifting phases to account for time translation
                        weights[(f+1)*num_bins + i] = weights[f*num_bins + i];
                    }
                    if (pivot_f > 0) {
                        amps_samples[c][pivot_f*num_bins + i] = (amps_samples[c][(pivot_f+1)*num_bins + i] + amps_samples[c][(pivot_f-1)*num_bins + i])/2.;
                        phases_samples[c][pivot_f*num_bins + i] = random_double_in_range(0, TWOPI); // unwrapped_phase_avg(phases_samples[c][(pivot_f+1)*num_bins + i], phases_samples[c][(pivot_f-1)*num_bins + i]); //
                        weights[pivot_f*num_bins + i] = 0.5*(weights[(pivot_f+1)*num_bins + i] + weights[(pivot_f-1)*num_bins + i]);
//                        phases_samples[c][pivot_f] = // TODO: phases!
                    } else if (pivot_f == 0) {
                        amps_samples[c][pivot_f*num_bins + i] = amps_samples[c][(pivot_f+1)*num_bins + i] ;
                        phases_samples[c][pivot_f*num_bins + i] = random_double_in_range(0, TWOPI);
                        weights[pivot_f*num_bins + i] = weights[(pivot_f+1)*num_bins + i];
                    }
                }
            }

            num_frames++;
            delta_num_frames--;
        } else {
            // deleting a seam
            // now this is not ideal: one should work with phase differences, but it's a matter of optimization, and we have worse issues now...
            for (long i = 0; i < num_bins; i++) {
                // TODO: ISSUE WITH PHASE SHIFT...
                for (long c = 0; c < num_channels; c++) {
                    long pivot_f = carve[i];
                    // temp is a parameter just to try from outside whether the phase shift improves things or makes them worse...
                    double phase_shift = compensate_phases * (pivot_f < num_frames - 1 ? phases_samples[c][(pivot_f + 1)*num_bins + i] - phases_samples[c][pivot_f*num_bins + i] : 0);
                    for (long f = pivot_f; f < num_frames - 1; f++) {
                        amps_samples[c][f*num_bins + i] = amps_samples[c][(f+1)*num_bins + i];
                        phases_samples[c][f*num_bins + i] = positive_fmod(phases_samples[c][(f+1)*num_bins + i] - phase_shift, TWOPI); // shifting phases to account for time translation
                        weights[f*num_bins + i] = weights[(f+1)*num_bins + i];
                    }
                }
            }
            
            num_frames--;
            delta_num_frames++;

        }

        firsttime = false;
    }
    
    if (carvingpath_samps) {
        for (long b = 0; b < num_bins; b++) {
            long prev = 0;
            for (long f = 0; f < num_frames; f++) {
                long temp = carvingpath_samps[f * num_bins + b];
                if (temp != prev)
                    carvingpath_samps[f * num_bins + b] = 1;
                else
                    carvingpath_samps[f * num_bins + b] = 0;
                prev = temp;
            }
        }
    }
    
    bach_freeptr(energymap);
    bach_freeptr(weights);
    
    for (long c = 0; c < num_channels; c++) {
        ears_buffer_copy_format_and_set_size_samps(ob, amplitudes[c], out_amplitudes[c], num_out_frames);
        ears_buffer_copy_format_and_set_size_samps(ob, phases[c], out_phases[c], num_out_frames);
        
        float *temp = buffer_locksamples(out_amplitudes[c]);
        if (!temp || ears_buffer_get_numchannels(ob, out_amplitudes[c]) != num_bins || ears_buffer_get_size_samps(ob, out_amplitudes[c]) != num_out_frames) {
            err = EARS_ERR_CANT_WRITE;
            object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
            goto end;
        }
        
        bach_copyptr(amps_samples[c], temp, num_out_frames * num_bins * sizeof(float));
        buffer_setdirty(out_amplitudes[c]);
        buffer_unlocksamples(out_amplitudes[c]);
        
        temp = buffer_locksamples(out_phases[c]);
        if (!temp || ears_buffer_get_numchannels(ob, out_phases[c]) != num_bins || ears_buffer_get_size_samps(ob, out_phases[c]) != num_out_frames) {
            err = EARS_ERR_CANT_WRITE;
            object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
            goto end;
        }
        bach_copyptr(phases_samples[c], temp, num_out_frames * num_bins * sizeof(float));
        buffer_setdirty(out_phases[c]);
        buffer_unlocksamples(out_phases[c]);
    }
    
    if (seam_path) {
        buffer_setdirty(seam_path);
        buffer_unlocksamples(seam_path);
        ears_buffer_set_size_samps(ob, seam_path, orig_num_frames);
    }
//    ears_buffer_set_size_samps_preserve(ob, seam_path, orig_num_frames);
    if (energy_map) {
        buffer_setdirty(energy_map);
        buffer_unlocksamples(energy_map);
    }

end:
    for (long c = 0; c < num_amps_locked; c++) {
        bach_freeptr(amps_samples[c]);
    }
    for (long c = 0; c < num_phases_locked; c++) {
        bach_freeptr(phases_samples[c]);
    }

    bach_freeptr(amps_samples);
    bach_freeptr(phases_samples);

    return err;
}






*/
