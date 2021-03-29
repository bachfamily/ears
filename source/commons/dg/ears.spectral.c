#include "ears.spectral.h"

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
            ears_buffer_copy_format(ob, source1, dest1);
        }

        if (source2 == dest2) { // inplace operation!
            orig_sample2_wk = (float *)bach_newptr(channelcount * framecount * sizeof(float));
            sysmem_copyptr(orig_sample2, orig_sample2_wk, channelcount * framecount * sizeof(float));
            buffer_unlocksamples(source2);
        } else {
            orig_sample2_wk = orig_sample2;
            ears_buffer_copy_format(ob, source1, dest2);
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


t_ears_err ears_buffer_paulstretch(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, double stretchfactor, long winsize_samps, char spectral)
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
        if (winsize_samps<16)
            winsize_samps=16;
        winsize_samps=optimize_windowsize(winsize_samps);
        winsize_samps=(long)(winsize_samps/2)*2;
        long half_winsize_samps=(long)(winsize_samps/2);
        
        // correct the end of the smp by adding a little fade out
        long end_size = MAX(16, (long)(sr*0.05));
        for (long c = 0; c < channelcount; c++)
            for (long i = MAX(0, framecount-end_size); i < framecount; i++)
                orig_sample_wk[i*channelcount + c] *= rescale(i, framecount-end_size, framecount-1, 1., 0.);
        
        // compute the displacement inside the input file
        double start_pos = 0.;
        double displace_pos = (winsize_samps*0.5)/stretchfactor;
        
        t_atom_long outframecount = ((long)(ceil(framecount/displace_pos)-1))* half_winsize_samps + winsize_samps;
        
        if (source == dest) { // inplace operation!
            ears_buffer_set_size(ob, source, outframecount);
        } else {
            ears_buffer_copy_format(ob, source, dest);
            ears_buffer_set_size(ob, dest, outframecount);
        }
        
        // create window function
        float *window = (float *)bach_newptr(winsize_samps * sizeof(float));
        for (long i = 0; i < winsize_samps; i++)                                // this window is not in the original paulstretch algorithm
            window[i] = sqrt(0.5 * (1 - cos(TWOPI * i / (winsize_samps - 1)))); // square root of hann should ensure perfect reconstruction
//            window[i] = pow(1 - pow(rescale(i, 0, winsize_samps - 1, -1., 1.), 2.), 1.25); // < this was the previously used window
        
        float *dest_sample = buffer_locksamples(dest);
        
        if (!dest_sample) {
            err = EARS_ERR_CANT_WRITE;
            object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
        } else {
            // fft buffers
            long nfft = winsize_samps;
            kiss_fft_cpx *fin = (kiss_fft_cpx *)bach_newptrclear(winsize_samps * sizeof(kiss_fft_cpx));
            kiss_fft_cpx *fout = (kiss_fft_cpx *)bach_newptr(winsize_samps * sizeof(kiss_fft_cpx));
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
                    for (long i = 0; i < winsize_samps; i++)
                        fin[i].r = (istart_pos+i >= framecount ? 0. : orig_sample_wk[(istart_pos+i)*channelcount + c]) * window[i];
                    
                    if (spectral) {
                        // performing FFT
                        bach_fft_kiss(cfg, nfft, false, fin, fout);
                        
                        // randomizing the phase
                        for (long i = 0; i < winsize_samps; i++)
                            fout[i] = polar_to_cpx(get_cpx_ampli(fout[i]), random_double_in_range(0., TWOPI));
                        
                        // performing inverse FFT
                        bach_fft_kiss(cfginv, nfft, true, fout, fin);
                        
                        // applying window again
                        for (long i = 0; i < winsize_samps; i++)
                            fin[i].r *= window[i];
                    }
                    
                    // then overlap-adding the window
                    for (long i = 0; i < winsize_samps; i++)
                        dest_sample[(i + (n * half_winsize_samps)) * channelcount + c] += fin[i].r;
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



t_ears_err ears_buffer_paulstretch_envelope(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, t_llll *stretchenv, long winsize_samps, char spectral, e_slope_mapping slopemapping)
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
        if (winsize_samps<16)
            winsize_samps=16;
        winsize_samps=optimize_windowsize(winsize_samps);
        winsize_samps=(long)(winsize_samps/2)*2;
        long half_winsize_samps=(long)(winsize_samps/2);
        
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
        t_atom_long outframecount = half_winsize_samps;
        while (true) {
            outframecount += half_winsize_samps;

            double stretchfactor = ears_envelope_iterator_walk_interp(&eei, start_pos, framecount);
            // checking stretch factor
            if (stretchfactor < PAULSTRETCH_MIN_STRETCH_FACTOR) {
                stretchfactor = PAULSTRETCH_MIN_STRETCH_FACTOR;
                if (!have_warned) {
                    object_warn(ob, "Stretch factor cannot be less than %f", PAULSTRETCH_MIN_STRETCH_FACTOR);
                    have_warned = true;
                }
            }
            start_pos += (winsize_samps*0.5)/stretchfactor;
            n++;
            if (start_pos >= framecount)
                break;
        }
        
        start_pos = 0.;
        ears_envelope_iterator_reset(&eei);
        
        if (source == dest) { // inplace operation!
            ears_buffer_set_size(ob, source, outframecount);
        } else {
            ears_buffer_copy_format(ob, source, dest);
            ears_buffer_set_size(ob, dest, outframecount);
        }
        
        // create window function
        float *window = (float *)bach_newptr(winsize_samps * sizeof(float));
        for (long i = 0; i < winsize_samps; i++)
            window[i] = pow(1 - pow(rescale(i, 0, winsize_samps - 1, -1., 1.), 2.), 1.25);
        
        float *dest_sample = buffer_locksamples(dest);
        
        if (!dest_sample) {
            err = EARS_ERR_CANT_WRITE;
            object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
        } else {
            // fft buffers
            long nfft = winsize_samps;
            kiss_fft_cpx *fin = (kiss_fft_cpx *)bach_newptrclear(winsize_samps * sizeof(kiss_fft_cpx));
            kiss_fft_cpx *fout = (kiss_fft_cpx *)bach_newptr(winsize_samps * sizeof(kiss_fft_cpx));
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
                    for (long i = 0; i < winsize_samps; i++)
                        fin[i].r = (istart_pos+i >= framecount ? 0. : orig_sample_wk[(istart_pos+i)*channelcount + c]) * window[i];
                    
                    if (spectral) {
                        // performing FFT
                        bach_fft_kiss(cfg, nfft, false, fin, fout);
                        
                        // randomizing the phase
                        for (long i = 0; i < winsize_samps; i++)
                            fout[i] = polar_to_cpx(get_cpx_ampli(fout[i]), random_double_in_range(0., TWOPI));
                        
                        // performing inverse FFT
                        bach_fft_kiss(cfginv, nfft, true, fout, fin);
                        
                        // applying window again
                        for (long i = 0; i < winsize_samps; i++)
                            fin[i].r *= window[i];
                    }
                    
                    // then overlap-adding the window
                    for (long i = 0; i < winsize_samps; i++)
                        dest_sample[(i + (n * half_winsize_samps)) * channelcount + c] += fin[i].r;
                }
                
                double stretchfactor = ears_envelope_iterator_walk_interp(&eei, start_pos, framecount);
                if (stretchfactor < PAULSTRETCH_MIN_STRETCH_FACTOR)
                    stretchfactor = PAULSTRETCH_MIN_STRETCH_FACTOR;
                start_pos += (winsize_samps*0.5)/stretchfactor;
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





