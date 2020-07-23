#include "ears.spectral.h"

const double PAULSTRETCH_MIN_STRETCH_FACTOR = 0.00001;

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



t_ears_err ears_buffer_paulstretch_envelope(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, t_llll *stretchenv, long winsize_samps, char spectral)
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
        t_ears_envelope_iterator eei = ears_envelope_iterator_create(stretchenv, 0., false);
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
