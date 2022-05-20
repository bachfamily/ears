#include "ears.spectral.h"
#include <numeric>

const double PAULSTRETCH_MIN_STRETCH_FACTOR = 0.00001;


t_ears_err ears_buffer_fft(t_object *ob, t_buffer_obj *source1, t_buffer_obj *source2, t_buffer_obj *dest1, t_buffer_obj *dest2, long polar, long inverse, long fullspectrum, e_ears_angleunit angleunit)
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
                if (polar) {
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

                bach_fft_kiss(cfg, fftsize, inverse, fin, fout);
                
                if (polar) {
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
                        bach_fft_kiss(cfg, nfft, false, fin, fout);
                        
                        // randomizing the phase
                        for (long i = 0; i < framesize_samps; i++)
                            fout[i] = polar_to_cpx(get_cpx_ampli(fout[i]), random_double_in_range(0., TWOPI));
                        
                        // performing inverse FFT
                        bach_fft_kiss(cfginv, nfft, true, fout, fin);
                        
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
                        bach_fft_kiss(cfg, nfft, false, fin, fout);
                        
                        // randomizing the phase
                        for (long i = 0; i < framesize_samps; i++)
                            fout[i] = polar_to_cpx(get_cpx_ampli(fout[i]), random_double_in_range(0., TWOPI));
                        
                        // performing inverse FFT
                        bach_fft_kiss(cfginv, nfft, true, fout, fin);
                        
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
                        bach_fft_kiss(cfg, nfft, false, fin, fout);
                        
                        // randomizing the phase
                        for (long i = 0; i < framesize_samps; i++)
                            fout[i] = polar_to_cpx(get_cpx_ampli(fout[i]), random_double_in_range(0., TWOPI));
                        
                        // performing inverse FFT
                        bach_fft_kiss(cfginv, nfft, true, fout, fin);
                        
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

t_ears_err ears_buffer_spectral_seam_carve(t_object *ob, long num_channels, t_buffer_obj **amplitudes, t_buffer_obj **phases, t_buffer_obj **out_amplitudes, t_buffer_obj **out_phases, t_buffer_obj *energy_map, t_buffer_obj *seam_path, long delta_num_frames, double framesize_samps, double hopsize_samps, long energy_mode)
{
    
    if (num_channels == 0)
        return EARS_ERR_NO_BUFFER;
    
    for (long i = 0; i < num_channels; i++) {
        if (!amplitudes[i] || !phases[i])
            return EARS_ERR_NO_BUFFER;
    }
    
    long num_frames = ears_buffer_get_size_samps(ob, amplitudes[0]);
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
    ears_buffer_copy_format((t_object *)ob, amplitudes[0], seam_path, true);
    ears_buffer_clear((t_object *)ob, seam_path);
    ears_buffer_set_size_and_numchannels((t_object *)ob, seam_path, num_frames, num_bins);
    carvingpath_samps = buffer_locksamples(seam_path);
    if (!carvingpath_samps || buffer_getframecount(seam_path) != num_frames || buffer_getchannelcount(seam_path) != num_bins) {
        err = EARS_ERR_CANT_WRITE;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
        goto end;
    }
    ears_buffer_copy_format((t_object *)ob, amplitudes[0], energy_map, true);
    ears_buffer_clear((t_object *)ob, energy_map);
    ears_buffer_set_size_and_numchannels((t_object *)ob, energy_map, num_frames, num_bins);
    energymapout_samps = buffer_locksamples(energy_map);
    if (!energymapout_samps || buffer_getframecount(energy_map) != num_frames || buffer_getchannelcount(energy_map) != num_bins) {
        err = EARS_ERR_CANT_WRITE;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
        goto end;
    }

    while (delta_num_frames != 0) {
        // 1) get energy map
        for (long b = 0; b < num_bins; b++) {
            for (long f = 0; f < num_frames; f++) {
                energymap[f*num_bins + b] = 0;
                switch (energy_mode) {
                    case EARS_SEAM_CARVE_MODE_MAGNITUDE:
                        for (long c = 0; c < num_channels; c++)
                            energymap[f*num_bins + b] += amps_samples[c][f*num_bins + b];
                        break;
                        
                    case EARS_SEAM_CARVE_MODE_GRADIENT_MAGNITUDE:
                    default:
                        for (long c = 0; c < num_channels; c++)
                            energymap[f*num_bins + b] += fabs(amps_samples[c][(f == num_frames - 1 ? f-1 : f+1)*num_bins + b]-amps_samples[c][f*num_bins + b]) + fabs(amps_samples[c][f*num_bins + (b == num_bins - 1 ? b-1 : b+1)]-amps_samples[c][f*num_bins + b]);
                        break;
                        
                }
                energymap[f*num_bins + b] /= num_channels; // not really needed though
            }
        }
        
        // 2) compute cumulative energies
        for (long b = 1; b < num_bins; b++) {
            for (long f = 0; f < num_frames; f++) {
                energymap[f*num_bins + b] += MIN(MIN(energymap[(f>0 ? f-1 : f) * num_bins + b-1], energymap[f*num_bins + b-1]), energymap[(f<num_frames-1 ? f+1 : f)*num_bins + b-1]);
            }
        }
        
        // write energymap to output, but only for the first iteration
        if (firsttime) {
            for (long b = 0; b < num_bins; b++) {
                for (long f = 0; f < num_frames; f++) {
                    energymapout_samps[f * num_bins + b] = energymap[f * num_bins + b];
                }
            }
        }
        
        // 3) find carving
        long carve[num_bins];
        // find minimum at num_bins-1
        double curr_min = energymap[0*num_bins + num_bins - 1];
        long curr_min_arg = 0;
        for (long f = 1; f < num_frames; f++) {
            if (energymap[f*num_bins + num_bins - 1] < curr_min) {
                curr_min = energymap[f*num_bins + num_bins - 1] ;
                curr_min_arg = f;
            }
        }
        carve[num_bins - 1] = curr_min_arg;
        for (long b = num_bins - 2; b >= 0; b--) {
            long cb = carve[b+1];
            double v1 = energymap[cb*num_bins + b];
            double v2 = energymap[(cb > 0 ? cb-1 : cb)*num_bins + b];
            double v3 = energymap[(cb < num_frames-1 ? cb+1 : cb)*num_bins + b];
            carve[b] = argmin3(v1, v2, v3, cb, cb > 0 ? cb-1 : cb, cb < num_frames-1 ? cb+1 : cb);
        }
        
        // writing carving data to output buffer
        for (long b = 0; b < num_bins; b++) {
            long thisframe = carve[b];
            long toadd = 0;
            for (long f = 0; f < thisframe; f++)
                toadd += carvingpath_samps[thisframe * num_bins + b];
            thisframe += toadd;
            if (thisframe < 0 || thisframe >= num_frames) {
                object_error((t_object *)ob, "Error!");
            } else {
                carvingpath_samps[thisframe * num_bins + b] += (delta_num_frames > 0 ? 1 : -1);
            }
        }
        
        // 4) apply seam carving
        if (delta_num_frames > 0) {
            // adding a seam
            for (long i = 0; i < num_bins; i++) {
                double phase_shift = (TWOPI * ((double)i) * hopsize_samps / framesize_samps);
                for (long c = 0; c < num_channels; c++) {
                    long pivot_f = carve[i];
                    for (long f = num_frames; f >= pivot_f; f--) {
                        amps_samples[c][(f+1)*num_bins + i] = amps_samples[c][f*num_bins + i];
                        phases_samples[c][(f+1)*num_bins + i] = phases_samples[c][f*num_bins + i] + phase_shift; // shifting phases to account for time translation
                    }
                    if (pivot_f > 0) {
                        amps_samples[c][pivot_f*num_bins + i] = (amps_samples[c][(pivot_f+1)*num_bins + i] + amps_samples[c][(pivot_f-1)*num_bins + i])/2.;
//                        phases_samples[c][pivot_f] = // TODO: phases!
                    }
                }
            }
            
            num_frames++;
            delta_num_frames--;
        } else {
            // deleting a seam
            for (long i = 0; i < num_bins; i++) {
                double phase_shift = -(TWOPI * ((double)i) * hopsize_samps / framesize_samps);
                for (long c = 0; c < num_channels; c++) {
                    long pivot_f = carve[i];
                    for (long f = pivot_f; f < num_frames; f++) {
                        amps_samples[c][f*num_bins + i] = amps_samples[c][(f+1)*num_bins + i];
                        phases_samples[c][f*num_bins + i] = phases_samples[c][(f+1)*num_bins + i] + phase_shift; // shifting phases to account for time translation
                    }
                    if (pivot_f > 0) {
//                        amps_samples[c][pivot_f*num_bins + i] = (amps_samples[c][(pivot_f+1)*num_bins + i] + amps_samples[c][(pivot_f-1)*num_bins + i])/2.;
//                        phases_samples[c][pivot_f] = // TODO: phases!?!
                    }
                }
            }

            num_frames--;
            delta_num_frames++;
        }
        
        firsttime = false;
    }
    
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
    
    bach_freeptr(energymap);
    
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
    
    buffer_setdirty(seam_path);
    buffer_unlocksamples(seam_path);
    buffer_setdirty(energy_map);
    buffer_unlocksamples(energy_map);

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






