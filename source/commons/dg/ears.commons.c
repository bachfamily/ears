#include "ext.h"
#include "ext_obex.h"
//#include "llllobj.h"
#include "llll_commons_ext.h"
#include "bach_math_utilities.h"
#include "ears.object.h"
#include <lame/lame.h> // only used to export mp3s



t_atom_long ears_buffer_get_size_samps(t_object *ob, t_buffer_obj *buf)
{
    return buffer_getframecount(buf);
/*
    t_buffer_info info;
    buffer_getinfo(buf, &info);
    return info.b_frames; */
}


double ears_buffer_get_size_ms(t_object *ob, t_buffer_obj *buf)
{
    return ears_samps_to_ms(buffer_getframecount(buf), buffer_getsamplerate(buf));
/*
    t_buffer_info info;
    buffer_getinfo(buf, &info);
    return ears_samps_to_ms(info.b_frames, info.b_sr);
 */
}


t_atom_float ears_buffer_get_sr(t_object *ob, t_buffer_obj *buf)
{
    return buffer_getsamplerate(buf);
/*    t_buffer_info info;
    buffer_getinfo(buf, &info);
    return info.b_sr; */
}

t_atom_long ears_buffer_get_numchannels(t_object *ob, t_buffer_obj *buf)
{
    return buffer_getchannelcount(buf);
/*    t_buffer_info info;
    buffer_getinfo(buf, &info);
    return info.b_nchans;
 */
}

t_ears_err ears_buffer_set_size(t_object *ob, t_buffer_obj *buf, long num_frames)
{
    t_atom a;
    atom_setlong(&a, num_frames);
    typedmess(buf, gensym("sizeinsamps"), 1, &a);
    return EARS_ERR_NONE;
}

t_ears_err ears_buffer_set_sr(t_object *ob, t_buffer_obj *buf, double sr)
{
    t_atom a;
    atom_setfloat(&a, sr);
    typedmess(buf, gensym("sr"), 1, &a);
    return EARS_ERR_NONE;
}





t_ears_err ears_buffer_crop_ms_inplace_maxapi(t_object *ob, t_buffer_obj *buf, double ms_start, long ms_end)
{
    // Using MAX Api, This is not so flexible, because it is quirky when ms_end is > buffer size, and also it doesn't deal with partial cropping (only start or end defined)
    t_atom a[2];
    atom_setfloat(a, ms_start);
    atom_setfloat(a+1, ms_end);
    typedmess(buf, gensym("crop"), 2, a);
    return EARS_ERR_NONE;
}


t_ears_err ears_buffer_normalize_inplace(t_object *ob, t_buffer_obj *buf, double level)
{
    t_atom a;
    atom_setfloat(&a, level);
    typedmess(buf, gensym("normalize"), 1, &a);
    return EARS_ERR_NONE;
}



t_ears_err ears_buffer_set_numchannels(t_object *ob, t_buffer_obj *buf, long numchannels)
{
    t_atom a[2];
    long num_frames = ears_buffer_get_size_samps(ob, buf);
    atom_setlong(a, num_frames);
    atom_setlong(a+1, numchannels);
    typedmess(buf, gensym("sizeinsamps"), 2, a);
 
    return EARS_ERR_NONE;
}


// ONE NEEDS TO BE SURE that this function is called when the samples of buf ARE NOT LOCKED!!!!
t_ears_err ears_buffer_set_size_and_numchannels(t_object *ob, t_buffer_obj *buf, long num_frames, long numchannels)
{
    t_atom a[2];
    atom_setlong(a, num_frames);
    atom_setlong(a+1, numchannels);
    typedmess(buf, gensym("sizeinsamps"), 2, a);
    
    return EARS_ERR_NONE;
}


t_ears_err ears_buffer_copy_format(t_object *ob, t_buffer_obj *orig, t_buffer_obj *dest)
{
    if (!orig || !dest) {
        object_error((t_object *)ob, EARS_ERROR_BUF_NO_BUFFER);
        return EARS_ERR_NO_BUFFER;
    }
    
    t_atom_long	orig_channelcount = buffer_getchannelcount(orig);		// number of floats in a frame
    double orig_sr = buffer_getsamplerate(orig);          // sample rate of the buffer in samples per second

    t_atom_long	dest_channelcount = buffer_getchannelcount(dest);	
    double dest_sr = buffer_getsamplerate(dest);

    
    if (dest_sr != orig_sr || dest_channelcount != orig_channelcount) {
        ears_buffer_set_sr(ob, dest, orig_sr);
        ears_buffer_set_numchannels(ob, dest, orig_channelcount);
    }
    
//    dest_channelcount = buffer_getchannelcount(dest);
//    dest_sr = buffer_getsamplerate(dest);

    return EARS_ERR_NONE;
}



// reverse buffer
t_ears_err ears_buffer_rev_inplace(t_object *ob, t_buffer_obj *buf)
{
    t_ears_err err = EARS_ERR_NONE;
    float *sample = buffer_locksamples(buf);
    
    if (!sample) {
        err = EARS_ERR_CANT_READ;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
    } else {
        t_atom_long	channelcount = buffer_getchannelcount(buf);		// number of floats in a frame
        t_atom_long	framecount   = buffer_getframecount(buf);			// number of floats long the buffer is for a single channel
        
        if (framecount > 0) {
            float *temp = (float *)bach_newptr(channelcount * framecount * sizeof(float));
            sysmem_copyptr(sample, temp, channelcount * framecount * sizeof(float));
            
            
            for (long i = 0; i < framecount; i++)
                for (long c = 0; c < channelcount; c++)
                    sample[i*channelcount + c] = temp[(framecount - i - 1)*channelcount + c];
            
            bach_freeptr(temp);
            buffer_setdirty(buf);
        }
        
        buffer_unlocksamples(buf);
    }
    return err;
}



// rotate buffer
t_ears_err ears_buffer_rot_inplace(t_object *ob, t_buffer_obj *buf, long shift_in_samps)
{
    t_ears_err err = EARS_ERR_NONE;
    float *sample = buffer_locksamples(buf);
    
    if (!sample) {
        err = EARS_ERR_CANT_READ;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
    } else {
        t_atom_long    channelcount = buffer_getchannelcount(buf);        // number of floats in a frame
        t_atom_long    framecount   = buffer_getframecount(buf);            // number of floats long the buffer is for a single channel
        
        shift_in_samps = positive_mod(shift_in_samps, framecount);
        
        if (framecount > 0) {
            float *temp = (float *)bach_newptr(channelcount * framecount * sizeof(float));
            sysmem_copyptr(sample, temp, channelcount * framecount * sizeof(float));
            
            sysmem_copyptr(temp+shift_in_samps*channelcount, sample, channelcount * (framecount - shift_in_samps) * sizeof(float));
            sysmem_copyptr(temp, sample + channelcount * (framecount - shift_in_samps), channelcount * shift_in_samps * sizeof(float));

            bach_freeptr(temp);
            buffer_setdirty(buf);
        }
        
        buffer_unlocksamples(buf);
    }
    return err;
}



// fill buffer with single value
t_ears_err ears_buffer_fill_inplace(t_object *ob, t_buffer_obj *buf, float val)
{
    t_ears_err err = EARS_ERR_NONE;
    float *sample = buffer_locksamples(buf);
    
    if (!sample) {
        err = EARS_ERR_CANT_READ;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
    } else {
        t_atom_long    channelcount = buffer_getchannelcount(buf);        // number of floats in a frame
        t_atom_long    framecount   = buffer_getframecount(buf);            // number of floats long the buffer is for a single channel
        
        if (framecount > 0) {
            for (long i = 0; i < framecount; i++)
                for (long c = 0; c < channelcount; c++)
                    sample[i*channelcount + c] = val;
            
            buffer_setdirty(buf);
        }
        
        buffer_unlocksamples(buf);
    }
    return err;
}





// Could be improved
// beware: <in> should be allocated with num_in_frames * num_channels floats, and <out> might be allocated with num_out_frames * num_channels float
long ears_resample(float *in, long num_in_frames, float **out, long num_out_frames, double factor, double fmax, double sr, double window_width, long num_channels)
{
    if (num_out_frames <= 0)
        num_out_frames = (long)ceil(num_in_frames * factor);
    if (out && !*out)
        *out = (float *)bach_newptr(num_out_frames * num_channels * sizeof(float));
    for (long ch = 0; ch < num_channels; ch++) {
        for (long s = 0; s < num_out_frames; s++) {
            long i, j;
            double x = s / factor;
            double r_w, r_a, r_snc;
            double r_g = 2 * fmax / sr; // Calc gain correction factor
            double r_y = 0;
            for (i = -window_width/2; i < window_width/2; i++) { // For 1 window width
                j = (long)(x + i);          // Calc input sample index
                //rem calculate von Hann Window. Scale and calculate Sinc
                r_w     = 0.5 - 0.5 * cos(TWOPI*(0.5 + (j - x)/window_width));
                r_a     = TWOPI*(j - x)*fmax/sr;
                r_snc   = (r_a != 0 ? r_snc = sin(r_a)/r_a : 1); ///<< sin(r_a) is incredibly slow. Do we have other options?
                if (j >= 0 && j < num_in_frames)
                    r_y   = r_y + r_g * r_w * r_snc * in[num_channels * j + ch];
            }
            (*out)[num_channels * s + ch] = r_y;                  // Return new filtered sample
        }
    }
    return num_out_frames * num_channels;
}

// resampling without converting sr
t_ears_err ears_buffer_resample(t_object *ob, t_buffer_obj *buf, double resampling_factor, long window_width)
{
    t_ears_err err = EARS_ERR_NONE;
    double curr_sr = buffer_getsamplerate(buf);
    double factor = resampling_factor;
    double sr = curr_sr * factor;
    
    double fmax = sr / 2.;
    
    float *sample = buffer_locksamples(buf);
    
    if (!sample) {
        err = EARS_ERR_CANT_READ;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
    } else {
        t_atom_long	channelcount = buffer_getchannelcount(buf);		// number of floats in a frame
        t_atom_long	framecount   = buffer_getframecount(buf);			// number of floats long the buffer is for a single channel
        
        long new_framecount = (long)ceil(framecount * factor);
        float *temp = (float *)bach_newptr(channelcount * framecount * sizeof(float));
        sysmem_copyptr(sample, temp, channelcount * framecount * sizeof(float));
        
        buffer_unlocksamples(buf);
        ears_buffer_set_size(ob, buf, new_framecount);
        sample = buffer_locksamples(buf);
        
        new_framecount = buffer_getframecount(buf);

        if (!sample) {
            err = EARS_ERR_CANT_WRITE;
            object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
        } else {
            for (long c = 0; c < channelcount; c++) {
                ears_resample(temp, framecount, &sample, new_framecount, factor, fmax, sr, window_width, channelcount);
            }
            buffer_setdirty(buf);
            buffer_unlocksamples(buf);
        }
        bach_freeptr(temp);
    }
    
    return err;
}

// resamples
t_ears_err ears_buffer_convert_sr(t_object *ob, t_buffer_obj *buf, double sr)
{
    t_ears_err err = EARS_ERR_NONE;
    double curr_sr = buffer_getsamplerate(buf);
    double factor = sr/curr_sr;
    
    long window_width = 11;
    double fmax = sr / 2.;
    
    float *sample = buffer_locksamples(buf);
    
    if (!sample) {
        err = EARS_ERR_CANT_READ;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
    } else {
        t_atom_long	channelcount = buffer_getchannelcount(buf);		// number of floats in a frame
        t_atom_long	framecount   = buffer_getframecount(buf);			// number of floats long the buffer is for a single channel
        
        long new_framecount = (long)ceil(framecount * factor);
        float *temp = (float *)bach_newptr(channelcount * framecount * sizeof(float));
        sysmem_copyptr(sample, temp, channelcount * framecount * sizeof(float));
        
        buffer_unlocksamples(buf);
        ears_buffer_set_size(ob, buf, new_framecount);
        ears_buffer_set_sr(ob, buf, sr);
        sample = buffer_locksamples(buf);
        new_framecount   = buffer_getframecount(buf);

        if (!sample) {
            err = EARS_ERR_CANT_WRITE;
            object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
        } else {
            ears_resample(temp, framecount, &sample, new_framecount, factor, fmax, sr, window_width, channelcount);
            buffer_setdirty(buf);
            buffer_unlocksamples(buf);
        }
        bach_freeptr(temp);
    }
    return err;
}


// keeps the buffer content while changing the nuber of channels
// REPAN mode is not supported by this function
t_ears_err ears_buffer_set_numchannels_preserve(t_object *ob, t_buffer_obj *buf, long new_numchannels, e_ears_channel_convert_modes mode)
{
    t_ears_err err = EARS_ERR_NONE;
    float *sample = buffer_locksamples(buf);
    
    if (!sample) {
        err = EARS_ERR_CANT_READ;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
    } else {
        t_atom_long	channelcount = buffer_getchannelcount(buf);		// number of floats in a frame
        t_atom_long	framecount   = buffer_getframecount(buf);			// number of floats long the buffer is for a single channel
        
        float *temp = (float *)bach_newptr(channelcount * framecount * sizeof(float));
        sysmem_copyptr(sample, temp, channelcount * framecount * sizeof(float));
        
        buffer_unlocksamples(buf);
        ears_buffer_set_numchannels(ob, buf, new_numchannels);
        sample = buffer_locksamples(buf);
        
        if (!sample) {
            err = EARS_ERR_CANT_WRITE;
            object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
        } else {
            switch (mode) {
                case EARS_CHANNELCONVERTMODE_CLEAR:
                    break;
                    
                case EARS_CHANNELCONVERTMODE_KEEP:
                    for (long i = 0; i < framecount; i++)
                        for (long c = 0; c < MIN(channelcount, new_numchannels); c++)
                            sample[i*new_numchannels + c] = temp[i*channelcount + c];
                    break;

                case EARS_CHANNELCONVERTMODE_PAD:
                    for (long c = 0; c < new_numchannels; c++) {
                        long c_wk = (c < channelcount ? c : channelcount - 1);
                        for (long i = 0; i < framecount; i++)
                            sample[i*new_numchannels + c] = temp[i*channelcount + c_wk];
                    }
                    break;

                case EARS_CHANNELCONVERTMODE_CYCLE:
                    for (long c = 0; c < new_numchannels; c++) {
                        long c_mod = c % channelcount;
                        for (long i = 0; i < framecount; i++)
                            sample[i*new_numchannels + c] = temp[i*channelcount + c_mod];
                    }
                    break;

                case EARS_CHANNELCONVERTMODE_PALINDROME:
                    for (long c = 0; c < new_numchannels; c++) {
                        long c_mod = c % (2 * channelcount);
                        if (c_mod >= channelcount)
                            c_mod = 2 * channelcount - c_mod - 1;
                        for (long i = 0; i < framecount; i++)
                            sample[i*new_numchannels + c] = temp[i*channelcount + c_mod];
                    }
                    break;
                    
                default:
                    break;
            }
            buffer_setdirty(buf);
            buffer_unlocksamples(buf);
        }
        bach_freeptr(temp);
    }
    return err;
}

t_ears_err ears_buffer_convert_numchannels(t_object *ob, t_buffer_obj *buf, long numchannels, e_ears_channel_convert_modes channelmode)
{
    t_ears_err err = EARS_ERR_NONE;
    long num_orig_channels = buffer_getchannelcount(buf);

    if (numchannels < num_orig_channels) {
        switch (channelmode) {
            case EARS_CHANNELCONVERTMODE_PAN:
                ears_buffer_pan1d(ob, buf, buf, numchannels, 0.5, EARS_PAN_MODE_LINEAR, EARS_PAN_LAW_COSINE, 1., true);
                break;
                
            default:
                ears_buffer_set_numchannels_preserve(ob, buf, numchannels, channelmode);
                break;
        }
    } else if (numchannels > num_orig_channels) {

        switch (channelmode) {
            case EARS_CHANNELCONVERTMODE_PAN:
                ears_buffer_pan1d(ob, buf, buf, numchannels, 0.5, EARS_PAN_MODE_LINEAR, EARS_PAN_LAW_COSINE, 1., false);
                break;
                
            default:
                ears_buffer_set_numchannels_preserve(ob, buf, numchannels, channelmode);
                break;
        }
    }
    
    
    return err;
}


// like ears_buffer_set_size() but keeps the samples
t_ears_err ears_buffer_convert_size(t_object *ob, t_buffer_obj *buf, long sizeinsamps)
{
    t_ears_err err = EARS_ERR_NONE;
    float *sample = buffer_locksamples(buf);
    
    if (!sample) {
        err = EARS_ERR_CANT_READ;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
    } else {
        t_atom_long	channelcount = buffer_getchannelcount(buf);		// number of floats in a frame
        t_atom_long	framecount   = buffer_getframecount(buf);			// number of floats long the buffer is for a single channel
        
        float *temp = (float *)bach_newptr(channelcount * framecount * sizeof(float));
        sysmem_copyptr(sample, temp, channelcount * framecount * sizeof(float));
        
        buffer_unlocksamples(buf);
        ears_buffer_set_size(ob, buf, sizeinsamps);
        sample = buffer_locksamples(buf);
        
        if (!sample) {
            err = EARS_ERR_CANT_WRITE;
            object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
        } else {
            t_atom_long	new_framecount = buffer_getframecount(buf);			// should be always equal to sizeinsamps!
            sysmem_copyptr(temp, sample, channelcount * MIN(new_framecount, framecount) * sizeof(float));
            buffer_setdirty(buf);
            buffer_unlocksamples(buf);
        }
        bach_freeptr(temp);
    }
    return err;
}



// Different from ears_buffer_copy_format() because it resamples and also downmixes/upmixes
t_ears_err ears_buffer_convert_format(t_object *ob, t_buffer_obj *orig, t_buffer_obj *dest, e_ears_channel_convert_modes channelmode)
{
    if (!orig || !dest) {
        object_error((t_object *)ob, EARS_ERROR_BUF_NO_BUFFER);
        return EARS_ERR_NO_BUFFER;
    }
    
    t_atom_long	orig_channelcount = buffer_getchannelcount(orig);		// number of floats in a frame
    double orig_sr = buffer_getsamplerate(orig);          // sample rate of the buffer in samples per second
    
    t_atom_long	dest_channelcount = buffer_getchannelcount(dest);
    double dest_sr = buffer_getsamplerate(dest);
    
    if (dest_sr != orig_sr)
        ears_buffer_convert_sr(ob, dest, orig_sr);
    
    if (dest_channelcount != orig_channelcount)
        ears_buffer_convert_numchannels(ob, dest, orig_channelcount, channelmode);
    
//    dest_channelcount = buffer_getchannelcount(dest);
//    dest_sr = buffer_getsamplerate(dest);
    
    return EARS_ERR_NONE;
}


// N.B. to sample is EXCLUDED!!!! The taken interval is [start_sample end_sample[
// end_sample is then the FIRST sample of the region after the crop
t_ears_err ears_buffer_crop(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, long start_sample, long end_sample)
{
    if (source == dest)
        ears_buffer_crop_inplace(ob, source, start_sample, end_sample);
    
    t_ears_err err = EARS_ERR_NONE;
    float *orig_sample = buffer_locksamples(source);
    
    if (!orig_sample) {
        if (!buffer_getframecount(source)) {
            object_warn((t_object *)ob, "Source buffer is empty!");
            ears_buffer_set_size(ob, dest, 0);
        } else {
            err = EARS_ERR_CANT_READ;
            object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
        }
    } else {
        t_atom_long	channelcount = buffer_getchannelcount(source);		// number of floats in a frame
        t_atom_long	framecount   = buffer_getframecount(source);			// number of floats long the buffer is for a single channel
        
        if (start_sample < 0) start_sample += framecount;
        if (start_sample < 0) start_sample = 0;
        CLIP_ASSIGN(start_sample, 0, framecount);
        if (end_sample < 0) end_sample += framecount;
        if (end_sample < 0) end_sample = 0;
        CLIP_ASSIGN(end_sample, 0, framecount);

        if (start_sample == framecount) {
            object_warn((t_object *)ob, "Starting crop point comes after the buffer end: empty buffer output.");
            ears_buffer_set_size(ob, dest, 0);
            
        } else if (end_sample <= start_sample) {
            object_warn((t_object *)ob, "Ending crop point precedes or coincides with starting crop point: empty buffer output.");
            ears_buffer_set_size(ob, dest, 0);

        } else {
            
            long new_dest_frames = end_sample - start_sample;
            ears_buffer_set_size_and_numchannels(ob, dest, new_dest_frames, channelcount);
            t_atom_long dest_channelcount = buffer_getchannelcount(dest);
            float *dest_sample = buffer_locksamples(dest);
            
            if (!dest_sample) {
                err = EARS_ERR_CANT_WRITE;
                object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
            } else {
                sysmem_copyptr(orig_sample + start_sample * channelcount, dest_sample, new_dest_frames * dest_channelcount * sizeof(float));
                buffer_setdirty(dest);
                buffer_unlocksamples(dest);
            }
        }
        buffer_unlocksamples(source);
    }
    
    return err;
}


t_ears_err ears_buffer_crop_ms(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, double start_ms, double end_ms)
{
    float msr = buffer_getmillisamplerate(source);
    long start_sample = round(start_ms * msr);
    long end_sample = round(end_ms * msr);
    
    return ears_buffer_crop(ob, source, dest, start_sample, end_sample);
}



// N.B. to sample is EXCLUDED!!!! The taken interval is [start_sample end_sample[
// end_sample is then the FIRST sample of the region after the crop
t_ears_err ears_buffer_crop_inplace(t_object *ob, t_buffer_obj *buf, long start_sample, long end_sample)
{
    t_ears_err err = EARS_ERR_NONE;
    float *sample = buffer_locksamples(buf);
    
    if (!sample) {
        if (!buffer_getframecount(buf)) {
            object_warn((t_object *)ob, "Source buffer is empty!");
            ears_buffer_set_size(ob, buf, 0);
        } else {
            err = EARS_ERR_CANT_READ;
            object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
        }
    } else {
        t_atom_long	channelcount = buffer_getchannelcount(buf);		// number of floats in a frame
        t_atom_long	framecount   = buffer_getframecount(buf);			// number of floats long the buffer is for a single channel
        
        if (start_sample < 0) start_sample = 0;
        CLIP_ASSIGN(start_sample, 0, framecount);
        if (end_sample < 0) end_sample = framecount;
        CLIP_ASSIGN(end_sample, 0, framecount);
        
        if (start_sample == framecount) {
            object_warn((t_object *)ob, "Starting crop point comes after the buffer end: empty buffer output.");
            ears_buffer_set_size(ob, buf, 0);
            
        } else if (end_sample <= start_sample) {
            object_warn((t_object *)ob, "Ending crop point precedes or coincides with starting crop point: empty buffer output.");
            ears_buffer_set_size(ob, buf, 0);
            
        } else {
            
            long new_dest_frames = end_sample - start_sample;
            float *temp = (float *)bach_newptr(channelcount * new_dest_frames * sizeof(float));
            sysmem_copyptr(sample + start_sample * channelcount, temp, channelcount * new_dest_frames * sizeof(float));
            
            buffer_unlocksamples(buf);
            ears_buffer_set_size(ob, buf, new_dest_frames);
            sample = buffer_locksamples(buf);
            
            if (!sample) {
                err = EARS_ERR_CANT_WRITE;
                object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
            } else {
                sysmem_copyptr(temp, sample, channelcount * new_dest_frames * sizeof(float));
                buffer_setdirty(buf);
            }
            
            bach_freeptr(temp);
        }
        buffer_unlocksamples(buf);
    }
    return err;
}


t_ears_err ears_buffer_crop_ms_inplace(t_object *ob, t_buffer_obj *buf, double start_ms, double end_ms)
{
    float msr = buffer_getmillisamplerate(buf);
    long start_sample = round(start_ms * msr);
    long end_sample = round(end_ms * msr);
    
    return ears_buffer_crop_inplace(ob, buf, start_sample, end_sample);
}




t_ears_err ears_buffer_clone(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest)
{
    if (!source || !dest)
        return EARS_ERR_NO_BUFFER;
    
    if (source == dest) // nothing to clone
        return EARS_ERR_NONE;
    
    // we do not use the "duplicate" message because we do not have control on it.
    // For instance, "duplicate" doesn't copy the buffer format.
    
    t_ears_err err = EARS_ERR_NONE;
    float *orig_sample = buffer_locksamples(source);
    
    if (!orig_sample) {
        err = EARS_ERR_CANT_READ;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
    } else {
        t_atom_long	channelcount = buffer_getchannelcount(source);		// number of floats in a frame
        t_atom_long	framecount   = buffer_getframecount(source);			// number of floats long the buffer is for a single channel

        ears_buffer_copy_format(ob, source, dest);
        ears_buffer_set_size(ob, dest, framecount);

        float *dest_sample = buffer_locksamples(dest);
        
        if (!dest_sample) {
            err = EARS_ERR_CANT_WRITE;
            object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
        } else {
            sysmem_copyptr(orig_sample, dest_sample, framecount * channelcount * sizeof(float));
            buffer_setdirty(dest);
            buffer_unlocksamples(dest);
        }
        buffer_unlocksamples(source);
    }
    
    return err;
}


// split sample is the first sample of the right portion
t_ears_err ears_buffer_slice(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest_left, t_buffer_obj *dest_right, long split_sample)
{
    t_ears_err err = EARS_ERR_NONE;
    float *orig_sample = buffer_locksamples(source);
    
    if (!orig_sample) {
        if (!buffer_getframecount(source)) {
            object_warn((t_object *)ob, "Source buffer is empty!");
            ears_buffer_set_size(ob, dest_left, 0);
            ears_buffer_set_size(ob, dest_right, 0);
        } else {
            err = EARS_ERR_CANT_READ;
            object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
        }
    } else {
        t_atom_long    channelcount = buffer_getchannelcount(source);        // number of floats in a frame
        t_atom_long    framecount   = buffer_getframecount(source);            // number of floats long the buffer is for a single channel
        
        if (split_sample < 0) split_sample += framecount;
        if (split_sample < 0) split_sample = 0;
        CLIP_ASSIGN(split_sample, 0, framecount);
        
        if (split_sample == framecount) {
            ears_buffer_clone(ob, source, dest_left);
            ears_buffer_set_size(ob, dest_right, 0);

        } else if (split_sample == 0) {
            ears_buffer_clone(ob, source, dest_right);
            ears_buffer_set_size(ob, dest_left, 0);

        } else {
            ears_buffer_clone(ob, source, dest_left);
            ears_buffer_clone(ob, source, dest_right);

            ears_buffer_set_size_and_numchannels(ob, dest_left, split_sample, channelcount);
            ears_buffer_set_size_and_numchannels(ob, dest_right, framecount - split_sample, channelcount);
            t_atom_long dest_left_channelcount = buffer_getchannelcount(dest_left);
            t_atom_long dest_right_channelcount = buffer_getchannelcount(dest_right);
            
            float *dest_left_sample = buffer_locksamples(dest_left);
            if (!dest_left_sample) {
                err = EARS_ERR_CANT_WRITE;
                object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
            } else {
                sysmem_copyptr(orig_sample, dest_left_sample, split_sample * dest_left_channelcount * sizeof(float));
                buffer_setdirty(dest_left);
                buffer_unlocksamples(dest_left);
            }

            float *dest_right_sample = buffer_locksamples(dest_right);
            if (!dest_left_sample) {
                err = EARS_ERR_CANT_WRITE;
                object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
            } else {
                sysmem_copyptr(orig_sample + split_sample * channelcount, dest_right_sample, (framecount - split_sample) * dest_right_channelcount * sizeof(float));
                buffer_setdirty(dest_right);
                buffer_unlocksamples(dest_right);
            }
            
        }
        buffer_unlocksamples(source);
    }
    
    return err;
}


// extract multiple regions from a buffer
// all ** must be allocated of size <num_regions>
t_ears_err ears_buffer_split(t_object *ob, t_buffer_obj *source, t_buffer_obj **dest, long *start_samples, long *end_samples, long num_regions)
{
    t_ears_err err = EARS_ERR_NONE;
    if (num_regions > 0) {
        float *orig_sample = buffer_locksamples(source);
        
        if (!orig_sample) {
            if (!buffer_getframecount(source)) {
                object_warn((t_object *)ob, "Source buffer is empty!");
                for (long i = 0; i < num_regions; i++)
                    ears_buffer_set_size(ob, dest[i], 0);
            } else {
                err = EARS_ERR_CANT_READ;
                object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
            }
        } else {
            t_atom_long    channelcount = buffer_getchannelcount(source);        // number of floats in a frame
            t_atom_long    framecount   = buffer_getframecount(source);            // number of floats long the buffer is for a single channel
            
            for (long i = 0; i < num_regions; i++) {
                t_buffer_obj *this_dest = dest[i];
                long this_start = start_samples[i];
                long this_end = end_samples[i];
                if (this_start < 0) this_start += framecount;
                if (this_start < 0) this_start = 0;
                CLIP_ASSIGN(this_start, 0, framecount);
                if (this_end < 0) this_end += framecount;
                if (this_end < 0) this_end = 0;
                CLIP_ASSIGN(this_end, 0, framecount);
                if (this_end < this_start) {
                    long temp = this_start; this_start = this_end; this_end = temp;
                }

                if (this_end == this_start) {
                    ears_buffer_set_size(ob, this_dest, 0);
                    
                } else {
                    ears_buffer_copy_format(ob, source, this_dest);
                    ears_buffer_set_size_and_numchannels(ob, this_dest, this_end - this_start, channelcount);
                    t_atom_long dest_channelcount = buffer_getchannelcount(this_dest);
                    
                    float *dest_sample = buffer_locksamples(this_dest);
                    if (!dest_sample) {
                        err = EARS_ERR_CANT_WRITE;
                        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
                    } else {
                        sysmem_copyptr(orig_sample + this_start * dest_channelcount, dest_sample, (this_end - this_start) * dest_channelcount * sizeof(float));
                        buffer_setdirty(this_dest);
                        buffer_unlocksamples(this_dest);
                    }
                }
            }
            
            buffer_unlocksamples(source);
        }
    } else {
        err = EARS_ERR_GENERIC;
        object_error((t_object *)ob, EARS_ERROR_BUF_NO_SEGMENTS);
    }
    
    return err;
}





t_ears_err ears_buffer_copychannel(t_object *ob, t_buffer_obj *source, long source_channel, t_buffer_obj *dest, long dest_channel)
{
    if (!source || !dest)
        return EARS_ERR_NO_BUFFER;
    
    t_ears_err err = EARS_ERR_NONE;
    float *source_sample = buffer_locksamples(source);
    
    if (!source_sample) {
        err = EARS_ERR_CANT_READ;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
    } else {
        t_atom_long	source_channelcount = buffer_getchannelcount(source);
        t_atom_long	source_framecount   = buffer_getframecount(source);
        
        float *dest_sample = buffer_locksamples(dest);
        if (!dest_sample) {
            err = EARS_ERR_CANT_READ;
            object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
        } else {
            t_atom_long	dest_channelcount = buffer_getchannelcount(dest);
            t_atom_long	dest_framecount   = buffer_getframecount(dest);

            for (long i = 0; i < source_framecount && i < dest_framecount; i++) {
                dest_sample[i * dest_channelcount + dest_channel] = source_sample[i * source_channelcount + source_channel];
            }
            buffer_unlocksamples(dest);
        }
        buffer_unlocksamples(source);
    }
    
    return err;
}


t_ears_err ears_buffer_setempty(t_object *ob, t_buffer_obj *buf, long num_channels)
{
    ears_buffer_set_size_and_numchannels(ob, buf, 0, num_channels);
    ears_buffer_set_sr(ob, buf, EARS_DEFAULT_SR);
    return EARS_ERR_NONE;
}


t_ears_err ears_buffer_extractchannels(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, long num_channels, long *channels)
{
    t_ears_err this_err, err = EARS_ERR_NONE;
    long frames = ears_buffer_get_size_samps(ob, source);
    ears_buffer_set_size_and_numchannels(ob, dest, frames, num_channels);
    for (long i = 0; i < num_channels; i++)
        if ((this_err = ears_buffer_copychannel(ob, source, channels[i], dest, i)) != EARS_ERR_NONE)
            err = this_err;
    return err;
}


t_ears_err ears_buffer_extractchannels_from_llll(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, t_llll *channels)
{
    long num_source_channels = ears_buffer_get_numchannels(ob, source);
    t_llll *temp = llll_clone(channels);
    llll_develop_ranges_and_parse_negative_indices_inplace(&temp, num_source_channels, true);
    
    long num_channels = temp->l_size;
    if (num_channels == 0)
        return ears_buffer_extractchannels(ob, source, dest, 0, NULL);
    else {
        long i = 0;
        long *channels = (long *)bach_newptr(num_channels * sizeof(long));
        for (t_llllelem *el = temp->l_head; el; el = el->l_next, i++)
            channels[i] = hatom_getlong(&el->l_hatom);
        t_max_err err = ears_buffer_extractchannels(ob, source, dest, num_channels, channels);
        bach_freeptr(channels);
        return err;
    }
    
    llll_free(temp);
}



t_ears_err ears_buffer_pack(t_object *ob, long num_sources, t_buffer_obj **source, t_buffer_obj *dest)
{
    t_ears_err err = EARS_ERR_NONE;
    
    if (!dest)
        return EARS_ERR_NO_BUFFER;
    
    if (num_sources == 0) {
        // empty buffer, 1 channel
        ears_buffer_setempty(ob, dest, 1);
        return EARS_ERR_NONE;
    }
    
    long num_channels = 0;
    long max_num_frames = 0;
    for (long i = 0; i < num_sources; i++) {
        num_channels += ears_buffer_get_numchannels(ob, source[i]);
        max_num_frames = MAX(max_num_frames, ears_buffer_get_size_samps(ob, source[i]));
    }
    
    ears_buffer_set_size_and_numchannels(ob, dest, max_num_frames, num_channels);
    
    long channel_cur = 0;
    for (long i = 0; i < num_sources; i++) {
        long this_num_channels = ears_buffer_get_numchannels(ob, source[i]);
        for (long c = 0; c < this_num_channels; c++) {
            t_ears_err this_err = ears_buffer_copychannel(ob, source[i], c, dest, channel_cur);
            if (err == EARS_ERR_NONE)
                err = this_err;
            channel_cur++;
        }
    }
    
    return err;
}


t_ears_err ears_buffer_pack_from_llll(t_object *ob, t_llll *sources_ll, t_buffer_obj *dest)
{
    long num_sources = sources_ll->l_size;
    
    if (num_sources == 0)
        return ears_buffer_pack(ob, 0, NULL, dest);
    else {
        long i = 0;
        t_buffer_obj **sources = (t_buffer_obj **)bach_newptr(num_sources * sizeof(t_buffer_obj *));
        for (t_llllelem *el = sources_ll->l_head; el; el = el->l_next, i++)
            sources[i] = (t_buffer_obj *)hatom_getobj(&el->l_hatom);
        t_max_err err = ears_buffer_pack(ob, num_sources, sources, dest);
        bach_freeptr(sources);
        return err;
    }
}

t_ears_err ears_buffer_lace(t_object *ob, t_buffer_obj *left, t_buffer_obj *right, t_buffer_obj *dest)
{
    t_ears_err err = EARS_ERR_NONE;
    
    if (!dest)
        return EARS_ERR_NO_BUFFER;
    
    long num_channels_left = ears_buffer_get_numchannels(ob, left);
    long num_channels_right = ears_buffer_get_numchannels(ob, right);
    long num_channels = num_channels_left + num_channels_right;
    long max_num_frames = MAX(ears_buffer_get_size_samps(ob, left), ears_buffer_get_size_samps(ob, right));
    
    ears_buffer_set_size_and_numchannels(ob, dest, max_num_frames, num_channels);
    
    long left_i = 0, right_i = 0;
    for (long i = 0; i < num_channels; i++) {
        if (i % 2 == 0 && left_i < num_channels_left) {
            t_ears_err this_err = ears_buffer_copychannel(ob, left, left_i++, dest, i);
            if (err == EARS_ERR_NONE)
                err = this_err;
        } else if (right_i < num_channels_right) {
            t_ears_err this_err = ears_buffer_copychannel(ob, right, right_i++, dest, i);
            if (err == EARS_ERR_NONE)
                err = this_err;
        }
    }
    
    return err;
}


t_ears_err ears_buffer_get_minmax(t_object *ob, t_buffer_obj *source, double *ampmin, double *ampmax)
{
    t_ears_err err = EARS_ERR_NONE;
    double min = 0, max = 0;

    float *sample = buffer_locksamples(source);
    
    if (!sample) {
        err = EARS_ERR_CANT_READ;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
    } else {
        t_atom_long	channelcount = buffer_getchannelcount(source);		// number of floats in a frame
        t_atom_long	framecount   = buffer_getframecount(source);			// number of floats long the buffer is for a single channel
        
        if (framecount == 0) {
            err = EARS_ERR_NO_BUFFER;
        } else {
            max = min = sample[0];
            
            for (long i = 0; i < channelcount * framecount; i++) {
                if (sample[i] > max)
                    max = sample[i];
                if (sample[i] < min)
                    min = sample[i];
            }
        }
    }
    
    buffer_unlocksamples(source);
    
    if (ampmin)
        *ampmin = min;
    if (ampmax)
        *ampmax = max;
    return err;
}


t_ears_err ears_buffer_get_maxabs(t_object *ob, t_buffer_obj *source, double *maxabs)
{
    double min = 0, max = 0;
    t_ears_err err = ears_buffer_get_minmax(ob, source, &min, &max);
    min = fabs(min);
    max = fabs(max);
    if (maxabs)
        *maxabs = MAX(min, max);
    return err;
}

t_ears_err ears_buffer_get_rms(t_object *ob, t_buffer_obj *source, double *rms)
{
    t_ears_err err = EARS_ERR_NONE;
    double tot = 0;
    
    float *sample = buffer_locksamples(source);
    
    if (!sample) {
        err = EARS_ERR_CANT_READ;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
    } else {
        t_atom_long	channelcount = buffer_getchannelcount(source);		// number of floats in a frame
        t_atom_long	framecount   = buffer_getframecount(source);			// number of floats long the buffer is for a single channel
        
        if (framecount == 0) {
            err = EARS_ERR_NO_BUFFER;
            object_error((t_object *)ob, EARS_ERROR_BUF_EMPTY_BUFFER);
        } else {
            for (long i = 0; i < channelcount * framecount; i++)
                tot += sample[i] * sample[i];
            
            tot = sqrt(tot/(channelcount * framecount));
        }
    }
    
    buffer_unlocksamples(source);
    
    if (rms)
        *rms = tot;
    
    return err;
}


// also supports inplace operations
t_ears_err ears_buffer_gain(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, double gain_factor, char use_decibels)
{
    if (!source || !dest)
        return EARS_ERR_NO_BUFFER;
    
    t_ears_err err = EARS_ERR_NONE;
    float *orig_sample = buffer_locksamples(source);
    float *orig_sample_wk = NULL;
    
    if (!orig_sample) {
        err = EARS_ERR_CANT_READ;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
    } else {
        double factor = use_decibels ? ears_db_to_linear(gain_factor) : gain_factor;
        
        t_atom_long	channelcount = buffer_getchannelcount(source);		// number of floats in a frame
        t_atom_long	framecount   = buffer_getframecount(source);			// number of floats long the buffer is for a single channel
        
        if (source == dest) { // inplace operation!
            orig_sample_wk = (float *)bach_newptr(channelcount * framecount * sizeof(float));
            sysmem_copyptr(orig_sample, orig_sample_wk, channelcount * framecount * sizeof(float));
            buffer_unlocksamples(source);
        } else {
            orig_sample_wk = orig_sample;
            ears_buffer_copy_format(ob, source, dest);
            ears_buffer_set_size(ob, dest, framecount);
        }
        
        float *dest_sample = buffer_locksamples(dest);
        
        if (!dest_sample) {
            err = EARS_ERR_CANT_WRITE;
            object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
        } else {

            for (long i = 0; i < channelcount * framecount; i++)
                dest_sample[i] = orig_sample_wk[i] * factor;
            
            buffer_setdirty(dest);
            buffer_unlocksamples(dest);
        }

        if (source == dest) // inplace operation!
            bach_freeptr(orig_sample_wk);
        else
            buffer_unlocksamples(source);
    }
    
    return err;
}


double min_circular_dist(double v1, double v2, double modulo)
{
    v1 = positive_fmod(v1, modulo);
    v2 = positive_fmod(v2, modulo);
    double m1 = MIN(v1, v2);
    double m2 = MAX(v1, v2);
    return MIN(fabs(m1 - m2), fabs(m1 + modulo - m2));
}


// pan must be between 0 and 1
float ears_get_pan_factor(long channel, long num_channels, double pan, e_ears_pan_modes pan_mode, e_ears_pan_laws pan_law)
{
    if (num_channels <= 1)
        return 1.;
    
    switch (pan_mode) {
        case EARS_PAN_MODE_LINEAR:
            pan *= num_channels - 1;
            
            switch (pan_law) {
                case EARS_PAN_LAW_NEAREST_NEIGHBOR:
                    return (channel == (long)round(pan));
                    
                case EARS_PAN_LAW_COSINE:
                    if (pan > channel + 1)
                        return 0.;
                    if (pan < channel - 1)
                        return 0.;
                    return cos(fabs(pan - channel) * PIOVERTWO);
            }
            break;
            
        default:
            pan *= num_channels;
            pan = fmod(pan, num_channels);
            
            switch (pan_law) {
                case EARS_PAN_LAW_NEAREST_NEIGHBOR:
                    return (channel == (long)round(pan));
                    
                case EARS_PAN_LAW_COSINE:
                {
                    double mindist = min_circular_dist(pan, channel, num_channels);
                    if (mindist < 1)
                        return cos(mindist * PIOVERTWO);
                    else
                        return 0.;
                }
            }
            break;
    }
}


// pan goes between 0 and 1
double multichannel_pan(double pan, e_ears_pan_modes pan_mode, long channelcount, long channel, double aperture)
{
    double out_pan = pan;
    
    if (channelcount <= 1)
        return out_pan; // that's the trivial case
    
    double ccm1d2 = (channelcount - 1)/2.;
    double delta_channel = (channel / ccm1d2) - 1; // this goes from -1 to 1 and is exactly 0 for the middle channels.
                                                                   // it is -1 for the first channel and 1 for the last
    switch (pan_mode) {
        case EARS_PAN_MODE_CIRCULAR:
            out_pan = positive_fmod(pan + delta_channel * (ccm1d2 / channelcount) * aperture, 1.);
            break;
            
        case EARS_PAN_MODE_LINEAR:
        {
            double temp = MAX(0, MIN(pan, 1. - pan));
            out_pan = pan + delta_channel * temp * aperture;
        }
            break;
            
        default:
            break;
    }
    
    return out_pan;
}


// pan goes between 0 and 1
// Should also work "inplace" when source = dest (NOT TESTED)
t_ears_err ears_buffer_pan1d(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, long num_out_channels, double pan, e_ears_pan_modes pan_mode, e_ears_pan_laws pan_law, double multichannel_pan_aperture, char compensate_gain_for_multichannel_to_avoid_clipping)
{
    if (!source || !dest)
        return EARS_ERR_NO_BUFFER;
    
    t_ears_err err = EARS_ERR_NONE;
    float *orig_sample = buffer_locksamples(source);
    float *orig_sample_wk = NULL;
    if (!orig_sample) {
        err = EARS_ERR_CANT_READ;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
    } else {
        t_atom_long	channelcount = buffer_getchannelcount(source);
        t_atom_long	framecount   = buffer_getframecount(source);
        
        
        if (source == dest) { // inplace operation!
            orig_sample_wk = (float *)bach_newptr(channelcount * framecount * sizeof(float));
            sysmem_copyptr(orig_sample, orig_sample_wk, channelcount * framecount * sizeof(float));
            buffer_unlocksamples(source);
        } else {
            orig_sample_wk = orig_sample;
            ears_buffer_copy_format(ob, source, dest);
        }
        
        ears_buffer_set_size_and_numchannels(ob, dest, framecount, num_out_channels);
        
        float *dest_sample = buffer_locksamples(dest);
        
        if (!dest_sample) {
            err = EARS_ERR_CANT_WRITE;
            object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
        } else {
            
            if (channelcount == 1) { // panning a mono source
                
                double pan_factor[EARS_MAX_NUM_CHANNELS]; // panning factors for each channel, and for each
                for (long c = 0; c < num_out_channels; c++)
                    pan_factor[c] = ears_get_pan_factor(c, num_out_channels, pan, pan_mode, pan_law);
                
                for (long i = 0; i < framecount; i++)
                    for (long c = 0; c < num_out_channels; c++)
                        dest_sample[i*num_out_channels + c] = orig_sample_wk[i] * pan_factor[c];
            } else {
                if (multichannel_pan_aperture == 0) {
                    double pan_factor[EARS_MAX_NUM_CHANNELS]; // panning factors for each channel
                    for (long c = 0; c < num_out_channels; c++) {
                        pan_factor[c] = ears_get_pan_factor(c, num_out_channels, pan, pan_mode, pan_law);
                        if (compensate_gain_for_multichannel_to_avoid_clipping)
                            pan_factor[c] /= channelcount;
                    }

                    for (long i = 0; i < framecount; i++) {
                        // mixdown to mono before panning
                        double s = 0;
                        for (long c = 0; c < channelcount; c++)
                            s += orig_sample_wk[i*channelcount + c];
                        
                        for (long c = 0; c < num_out_channels; c++)
                            dest_sample[i*num_out_channels + c] = s * pan_factor[c];
                    }
                } else {
                    double *pan_factor = (double *)bach_newptr(num_out_channels * channelcount * sizeof(double));
                    double adjusted_pan[EARS_MAX_NUM_CHANNELS]; // panning position for each channel
                    for (long d = 0; d < channelcount; d++)
                        adjusted_pan[d] = multichannel_pan(pan, pan_mode, channelcount, d, multichannel_pan_aperture);
                    for (long c = 0; c < num_out_channels; c++)
                        for (long d = 0; d < channelcount; d++) { // spreading each input channel to a different position
                            pan_factor[c * channelcount + d] = ears_get_pan_factor(c, num_out_channels, adjusted_pan[d], pan_mode, pan_law) / (compensate_gain_for_multichannel_to_avoid_clipping ? channelcount : 1);
                        }

                    
                    memset(dest_sample, 0, framecount * num_out_channels * sizeof(float));
                    
                    for (long i = 0; i < framecount; i++) {
                        for (long c = 0; c < num_out_channels; c++)
                            for (long d = 0; d < channelcount; d++)
                                dest_sample[i * num_out_channels + c] += orig_sample_wk[i * channelcount + d] * pan_factor[c * channelcount + d];
                    }
                    bach_freeptr(pan_factor);
                }
            }
            
            buffer_setdirty(dest);
            buffer_unlocksamples(dest);
        }
        
        if (source == dest) // inplace operation!
            bach_freeptr(orig_sample_wk);
        else
            buffer_unlocksamples(source);
    }
    
    return err;
}



t_ears_err ears_buffer_pan1d_envelope(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, long num_out_channels, t_llll *env, e_ears_pan_modes pan_mode, e_ears_pan_laws pan_law, double multichannel_pan_aperture, char compensate_gain_for_multichannel_to_avoid_clipping)
{
    
    
    if (!source || !dest)
        return EARS_ERR_NO_BUFFER;
    
    t_ears_err err = EARS_ERR_NONE;
    float *orig_sample = buffer_locksamples(source);
    
    if (!orig_sample) {
        err = EARS_ERR_CANT_READ;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
    } else {
        t_atom_long	channelcount = buffer_getchannelcount(source);
        t_atom_long	framecount   = buffer_getframecount(source);
        
        ears_buffer_copy_format(ob, source, dest);
        ears_buffer_set_size_and_numchannels(ob, dest, framecount, num_out_channels);
        
        float *dest_sample = buffer_locksamples(dest);
        
        if (!dest_sample) {
            err = EARS_ERR_CANT_WRITE;
            object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
        } else {
            
            t_ears_envelope_iterator eei = ears_envelope_iterator_create(env, 0., false);
            
            if (channelcount == 1) { // panning a mono source
                for (long i = 0; i < framecount; i++) {
                    double pan = ears_envelope_iterator_walk_interp(&eei, i, framecount);
                    for (long c = 0; c < num_out_channels; c++)
                        dest_sample[i*num_out_channels + c] = orig_sample[i] * ears_get_pan_factor(c, num_out_channels, pan, pan_mode, pan_law);
                }
            } else {
                if (multichannel_pan_aperture == 0) {
                    
                    for (long i = 0; i < framecount; i++) {
                        // mixdown to mono before panning
                        double s = 0;
                        for (long c = 0; c < channelcount; c++)
                            s += orig_sample[i*channelcount + c];
                        if (compensate_gain_for_multichannel_to_avoid_clipping)
                            s /= channelcount;
                        
                        double pan = ears_envelope_iterator_walk_interp(&eei, i, framecount);
                        for (long c = 0; c < num_out_channels; c++)
                            dest_sample[i*num_out_channels + c] = s * ears_get_pan_factor(c, num_out_channels, pan, pan_mode, pan_law);
                    }
                } else {
                    double adjusted_pan[EARS_MAX_NUM_CHANNELS]; // panning position for each channel
                    for (long i = 0; i < framecount; i++) {
                        double pan = ears_envelope_iterator_walk_interp(&eei, i, framecount);
                        for (long d = 0; d < channelcount; d++)
                            adjusted_pan[d] = multichannel_pan(pan, pan_mode, channelcount, d, multichannel_pan_aperture);
                        for (long c = 0; c < num_out_channels; c++)
                            for (long d = 0; d < channelcount; d++)
                                dest_sample[i * num_out_channels + c] += orig_sample[i * channelcount + d] * ears_get_pan_factor(c, num_out_channels, adjusted_pan[d], pan_mode, pan_law) / (compensate_gain_for_multichannel_to_avoid_clipping ? channelcount : 1);
                    }
                }
            }
            
            buffer_setdirty(dest);
            buffer_unlocksamples(dest);
        }
        buffer_unlocksamples(source);
    }
    
    return err;
}



t_ears_err ears_buffer_pan1d_buffer(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, long num_out_channels, t_buffer_obj *pan, e_ears_pan_modes pan_mode, e_ears_pan_laws pan_law, double multichannel_pan_aperture, char compensate_gain_for_multichannel_to_avoid_clipping)
{
    if (!source || !pan || !dest)
        return EARS_ERR_NO_BUFFER;
    
    t_ears_err err = EARS_ERR_NONE;
    float *source_sample = buffer_locksamples(source);
    
    if (!source_sample) {
        err = EARS_ERR_CANT_READ;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
    } else {
        t_atom_long    source_channelcount = buffer_getchannelcount(source);
        t_atom_long    source_framecount   = buffer_getframecount(source);
        
        ears_buffer_copy_format(ob, source, dest);
        ears_buffer_set_size_and_numchannels(ob, dest, source_framecount, num_out_channels);

//        t_atom_long    dest_channelcount = buffer_getchannelcount(dest);
//        t_atom_long    dest_framecount   = buffer_getframecount(dest);
 
        float *dest_sample = buffer_locksamples(dest);
        
        if (!dest_sample) {
            err = EARS_ERR_CANT_WRITE;
            object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
        } else {
            float *pan_sample = buffer_locksamples(pan);
            
            if (!pan_sample) {
                err = EARS_ERR_CANT_WRITE;
                object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
            } else {
                
                t_atom_long	pan_channelcount = buffer_getchannelcount(pan);
                t_atom_long	pan_framecount   = buffer_getframecount(pan);
                
                if (pan_channelcount >= 1) {
                    long max_sample = MIN(source_framecount, pan_framecount);
                    
                    if (source_channelcount == 1) { // panning a mono source
                        for (long i = 0; i < max_sample; i++) {
                            double pan = pan_sample[i];
                            for (long c = 0; c < num_out_channels; c++)
                                dest_sample[i*num_out_channels + c] = source_sample[i] * ears_get_pan_factor(c, num_out_channels, pan, pan_mode, pan_law);
                        }
                    } else {
                        if (multichannel_pan_aperture == 0) {
                            
                            for (long i = 0; i < max_sample; i++) {
                                // mixdown to mono before panning
                                double s = 0;
                                for (long c = 0; c < source_channelcount; c++)
                                    s += source_sample[i*source_channelcount + c];
                                if (compensate_gain_for_multichannel_to_avoid_clipping)
                                    s /= source_channelcount;
                                
                                double pan = pan_sample[i];
                                for (long c = 0; c < num_out_channels; c++)
                                    dest_sample[i*num_out_channels + c] = s * ears_get_pan_factor(c, num_out_channels, pan, pan_mode, pan_law);
                            }
                        } else {
                            double adjusted_pan[EARS_MAX_NUM_CHANNELS]; // panning position for each channel
                            for (long i = 0; i < max_sample; i++) {
                                double pan = pan_sample[i];
                                for (long d = 0; d < source_channelcount; d++)
                                    adjusted_pan[d] = multichannel_pan(pan, pan_mode, source_channelcount, d, multichannel_pan_aperture);
                                for (long c = 0; c < num_out_channels; c++)
                                    for (long d = 0; d < source_channelcount; d++)
                                        dest_sample[i * num_out_channels + c] += source_sample[i * source_channelcount + d] * ears_get_pan_factor(c, num_out_channels, adjusted_pan[d], pan_mode, pan_law) / (compensate_gain_for_multichannel_to_avoid_clipping ? source_channelcount : 1);
                            }
                        }
                    }
                }
                
                buffer_unlocksamples(pan);
            }
            buffer_setdirty(dest);
            buffer_unlocksamples(dest);
        }
        buffer_unlocksamples(source);
    }
    
    return err;
}









t_ears_err ears_buffer_multiply_inplace(t_object *ob, t_buffer_obj *buf, t_buffer_obj *factor)
{
    if (!buf || !factor)
        return EARS_ERR_NO_BUFFER;
    
    t_ears_err err = EARS_ERR_NONE;
    float *buf_sample = buffer_locksamples(buf);
    
    if (!buf_sample) {
        err = EARS_ERR_CANT_READ;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
    } else {
        t_atom_long	channelcount = buffer_getchannelcount(buf);
        t_atom_long	framecount   = buffer_getframecount(buf);
        
        float *factor_sample = buffer_locksamples(factor);
        
        if (!factor_sample) {
            err = EARS_ERR_CANT_WRITE;
            object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
        } else {
            
            t_atom_long	factor_channelcount = buffer_getchannelcount(factor);
            t_atom_long	factor_framecount   = buffer_getframecount(factor);
            
            for (long i = 0; i < framecount; i++) {
                if (i < factor_framecount) {
                    for (long c = 0; c < channelcount; c++) {
                        buf_sample[i*channelcount + c] *= factor_sample[i*factor_channelcount + MIN(c, factor_channelcount - 1)];
                    }
                } else {
                    for (long c = 0; c < channelcount; c++)
                        buf_sample[i*channelcount + c] = 0;
                }
            }
            
            buffer_unlocksamples(factor);
        }
        buffer_setdirty(buf);
        buffer_unlocksamples(buf);
    }
    
    return err;
}


t_ears_err ears_buffer_sum_inplace(t_object *ob, t_buffer_obj *buf, t_buffer_obj *addend)
{
    if (!buf || !addend)
        return EARS_ERR_NO_BUFFER;
    
    t_ears_err err = EARS_ERR_NONE;
    float *buf_sample = buffer_locksamples(buf);
    
    if (!buf_sample) {
        err = EARS_ERR_CANT_READ;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
    } else {
        t_atom_long	channelcount = buffer_getchannelcount(buf);
        t_atom_long	framecount   = buffer_getframecount(buf);
        
        float *addend_sample = buffer_locksamples(addend);
        
        if (!addend_sample) {
            err = EARS_ERR_CANT_WRITE;
            object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
        } else {
            
            t_atom_long	addend_channelcount = buffer_getchannelcount(buf);
            t_atom_long	addend_framecount   = buffer_getframecount(buf);
            
            for (long i = 0; i < framecount; i++) {
                if (i < addend_framecount) {
                    for (long c = 0; c < channelcount; c++) {
                        buf_sample[i*channelcount + c] += addend_sample[i*addend_channelcount + MIN(c, addend_channelcount - 1)];
                    }
                }
            }
            
            buffer_unlocksamples(addend);
        }
        buffer_setdirty(buf);
        buffer_unlocksamples(buf);
    }
    
    return err;
}




t_llll *ears_buffer_to_llll(t_object *ob, t_buffer_obj *buf)
{
    t_llll *out = llll_get();
    
    if (!buf)
        return out;
    
    float *sample = buffer_locksamples(buf);
    
    if (!sample) {
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
        return out;
        
    } else {
        
        t_atom_long	channelcount = buffer_getchannelcount(buf);		// number of floats in a frame
        t_atom_long	framecount   = buffer_getframecount(buf);			// number of floats long the buffer is for a single channel
        
        t_llll **chan = (t_llll **)bach_newptr(MAX(1, channelcount) * sizeof(t_llll *));
        for (long c = 0; c < channelcount; c++)
            chan[c] = llll_get();
        for (long i = 0; i < framecount; i++)
            for (long c = 0; c < channelcount; c++)
                llll_appenddouble(chan[c], sample[channelcount * i + c]);

        for (long i = 0; i < channelcount; i++)
            llll_appendllll(out, chan[i]);
        bach_freeptr(chan);
    }
    
    buffer_unlocksamples(buf);
    return out;
}


t_atom_long ears_buffer_channel_to_array(t_object *ob, t_buffer_obj *buf, long channel, float **outsamples)
{
    if (!buf)
        return -1;
    
    float *sample = buffer_locksamples(buf);
    t_atom_long    framecount = 0;
    
    if (!sample) {
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
        return -1;
        
    } else {
        
        t_atom_long    channelcount = buffer_getchannelcount(buf);        // number of floats in a frame
        framecount   = buffer_getframecount(buf);            // number of floats long the buffer is for a single channel
        
        if (!(*outsamples))
            *outsamples = (float *)bach_newptr(framecount * sizeof(float));

        for (long i = 0; i < framecount; i++)
            (*outsamples)[i] = sample[channelcount * i + channel];
    }
    
    buffer_unlocksamples(buf);
    return framecount;
}

t_atom_long ears_buffer_channel_to_double_array(t_object *ob, t_buffer_obj *buf, long channel, double **outsamples)
{
    if (!buf)
        return -1;
    
    float *sample = buffer_locksamples(buf);
    t_atom_long    framecount = 0;
    
    if (!sample) {
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
        return -1;
        
    } else {
        
        t_atom_long    channelcount = buffer_getchannelcount(buf);        // number of floats in a frame
        framecount   = buffer_getframecount(buf);            // number of floats long the buffer is for a single channel
        
        if (!(*outsamples))
            *outsamples = (double *)bach_newptr(framecount * sizeof(double));
        
        for (long i = 0; i < framecount; i++)
            (*outsamples)[i] = sample[channelcount * i + channel];
    }
    
    buffer_unlocksamples(buf);
    return framecount;
}



t_ears_err ears_buffer_from_llll(t_object *ob, t_buffer_obj *buf, t_llll *ll, char reformat)
{
    if (!buf)
        return EARS_ERR_NO_BUFFER;
    
    // Get max number of samples
    long max_num_samples = 0;
    for (t_llllelem *el = ll->l_head; el; el = el->l_next)
        if (hatom_gettype(&el->l_hatom) == H_LLLL)
            max_num_samples = MAX(max_num_samples, hatom_getllll(&el->l_hatom)->l_size);
    
    // Get number of channels
    long num_channels = ll->l_size;
    
    if (reformat)
        ears_buffer_set_size_and_numchannels(ob, buf, max_num_samples, num_channels);
    
    float *sample = buffer_locksamples(buf);
    
    if (!sample) {
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
        return EARS_ERR_CANT_READ;
        
    } else {
        
        t_atom_long	channelcount = buffer_getchannelcount(buf);
        t_atom_long	framecount   = buffer_getframecount(buf);
        
        // erasing samples
        memset(sample, 0, framecount * channelcount * sizeof(float));
        
        long c = 0;
        for (t_llllelem *el = ll->l_head; el && c < channelcount; el = el->l_next, c++) {
            if (hatom_gettype(&el->l_hatom) == H_LLLL) {
                long i = 0;
                for (t_llllelem *smp = hatom_getllll(&el->l_hatom)->l_head; smp && i < framecount; smp = smp->l_next, i++) {
                    sample[i*channelcount + c] = hatom_getdouble(&smp->l_hatom);
                }
            }
        }
        
        buffer_setdirty(buf);
    }
    
    buffer_unlocksamples(buf);
    return EARS_ERR_NONE;
}


t_pts elem_to_pts(t_llllelem *incoming_el)
{
    t_pts out;
    out.x = out.y = out.slope = 0;
    
    if (hatom_gettype(&incoming_el->l_hatom) == H_LLLL) {
        t_llllelem *elem = hatom_getllll(&incoming_el->l_hatom)->l_head;
        
        if (elem) {
            out.x = hatom_getdouble(&elem->l_hatom);
            elem = elem->l_next;
            if (elem) {
                out.y = hatom_getdouble(&elem->l_hatom);
                elem = elem->l_next;
                if (elem)
                    out.slope = CLAMP(hatom_getdouble(&elem->l_hatom), -1., 1.);
            }
        }
    }
    return out;
}



t_ears_envelope_iterator ears_envelope_iterator_create(t_llll *envelope, double default_val, char use_decibels)
{
    t_ears_envelope_iterator eei;
    eei.env = envelope;
    eei.default_val = default_val;
    eei.left_el = NULL;
    eei.right_el = envelope ? envelope->l_head : NULL;
    if (eei.right_el)
        eei.left_pts = eei.right_pts = elem_to_pts(eei.right_el);
    eei.use_decibels = use_decibels;
    return eei;
}

t_ears_envelope_iterator ears_envelope_iterator_create_from_llllelem(t_llllelem *envelope, double default_val, char use_decibels)
{
    t_ears_envelope_iterator eei;
    if (!envelope) {
        eei.env = NULL; // no envelope, fixed value
        eei.right_el = NULL;
        eei.default_val = default_val;
    } else if (hatom_gettype(&envelope->l_hatom) == H_LLLL) {
        eei.env = hatom_getllll(&envelope->l_hatom);
        eei.right_el = eei.env ? eei.env->l_head : NULL;
        eei.default_val = 0.;
    } else {
        eei.env = NULL; // no envelope, fixed value
        eei.right_el = NULL;
        eei.default_val = hatom_getdouble(&envelope->l_hatom);
    }
    eei.use_decibels = use_decibels;
    eei.left_el = NULL;
    if (eei.right_el)
        eei.left_pts = eei.right_pts = elem_to_pts(eei.right_el);
    return eei;
}

void ears_envelope_iterator_reset(t_ears_envelope_iterator *eei)
{
    eei->left_el = NULL;
    eei->right_el = eei->env ? eei->env->l_head : NULL;
    if (eei->right_el)
        eei->left_pts = eei->right_pts = elem_to_pts(eei->right_el);
}


double ears_envelope_iterator_walk_interp(t_ears_envelope_iterator *eei, long sample_num, long tot_num_samples)
{
    if (!eei->env)
        return eei->default_val;
    
    // If the envelope is a simple gain value, return that, there's nowhere to walk!
    if (eei->env->l_depth == 1 && eei->env->l_size == 1)
        return hatom_getdouble(&eei->env->l_head->l_hatom);
    
    // X position is in samples! It has been converted by earsbufobj_llllelem_to_linear_and_samples
    double pos_x = ((double)sample_num); ///(tot_num_samples-1);
    while (eei->right_el && pos_x >= eei->right_pts.x) {
        eei->left_el = eei->right_el;
        eei->right_el = eei->right_el->l_next;
        eei->left_pts = eei->right_pts;
        if (eei->right_el)
            eei->right_pts = elem_to_pts(eei->right_el);
    }
    
    double amp = 0.;
    if (eei->right_el && eei->left_el)
        amp = rescale_with_slope(pos_x, eei->left_pts.x, eei->right_pts.x, eei->left_pts.y, eei->right_pts.y, eei->right_pts.slope);
    else if (eei->left_el)
        amp = eei->left_pts.y;
    else if (eei->right_el)
        amp = eei->right_pts.y;
    
    return eei->use_decibels ? ears_db_to_linear(amp) : amp;
}


t_ears_err ears_buffer_gain_envelope(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, t_llll *env, char use_decibels)
{
    if (!source || !dest)
        return EARS_ERR_NO_BUFFER;
    
    t_ears_err err = EARS_ERR_NONE;
    float *orig_sample = buffer_locksamples(source);
    float *orig_sample_wk = NULL;
    
    if (!orig_sample) {
        err = EARS_ERR_CANT_READ;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
    } else {
        
        t_atom_long	channelcount = buffer_getchannelcount(source);		// number of floats in a frame
        t_atom_long	framecount   = buffer_getframecount(source);			// number of floats long the buffer is for a single channel
        
        if (source == dest) { // inplace operation!
            orig_sample_wk = (float *)bach_newptr(channelcount * framecount * sizeof(float));
            sysmem_copyptr(orig_sample, orig_sample_wk, channelcount * framecount * sizeof(float));
            buffer_unlocksamples(source);
        } else {
            orig_sample_wk = orig_sample;
            ears_buffer_copy_format(ob, source, dest);
            ears_buffer_set_size(ob, dest, framecount);
        }
        
        float *dest_sample = buffer_locksamples(dest);
        
        if (!dest_sample) {
            err = EARS_ERR_CANT_WRITE;
            object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
        } else {
            
            t_ears_envelope_iterator eei = ears_envelope_iterator_create(env, 0., use_decibels);
            for (long i = 0; i < framecount; i++) {
                double factor = ears_envelope_iterator_walk_interp(&eei, i, framecount);
                
                for (long c = 0; c < channelcount; c++) {
                    long idx = channelcount * i + c;
                    dest_sample[idx] = orig_sample[idx] * factor;
                }
            }
            
            buffer_setdirty(dest);
            buffer_unlocksamples(dest);
        }
        
        if (source == dest)  // inplace operation!
            bach_freeptr(orig_sample_wk);
        else
            buffer_unlocksamples(source);

    }
    
    return err;
}





t_ears_err ears_buffer_normalize(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, double linear_amp_level, double mix)
{
    double maxabs = 0;
    t_ears_err err = ears_buffer_get_maxabs(ob, source, &maxabs);
    if (err == EARS_ERR_EMPTY_BUFFER)
        object_warn(ob, EARS_ERROR_BUF_EMPTY_BUFFER);

    if (err != EARS_ERR_NONE)
        return err;
    else if (maxabs == 0) {
        object_error((t_object *)ob, EARS_ERROR_BUF_ZERO_AMP);
        if (source != dest)
            ears_buffer_clone(ob, source, dest);
        return EARS_ERR_ZERO_AMP;
    } else {
        double factor = mix * linear_amp_level/maxabs + (1-mix);
        return ears_buffer_gain(ob, source, dest, factor, false);
    }
}


t_ears_err ears_buffer_normalize_rms(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, double linear_amp_level, double mix)
{
    double rms = 0;
    t_ears_err err = ears_buffer_get_rms(ob, source, &rms);
    
    if (err != EARS_ERR_NONE)
        return err;
    if (rms == 0) {
        object_error((t_object *)ob, EARS_ERROR_BUF_ZERO_AMP);
        if (source != dest)
            ears_buffer_clone(ob, source, dest);
        return EARS_ERR_ZERO_AMP;
    } else {
        double factor = mix * linear_amp_level/rms + (1-mix);
        return ears_buffer_gain(ob, source, dest, factor, false);
    }
}







// position ranges from 0 = fade is at 0 factor, to 1 = fade is at 1 factor (original gain)
float ears_get_fade_factor(char in_or_out, e_ears_fade_types fade, double position, double curve)
{
    switch (fade) {
        case EARS_FADE_LINEAR:
            return position;
            break;
        case EARS_FADE_EQUALPOWER:
            return sin(PIOVERTWO * position);
            break;
        case EARS_FADE_CURVE: // could find a faster formula, this is CPU consuming
            CLIP_ASSIGN(curve, -1., 1.);
            return rescale_with_slope(position, 0, 1., 0., 1., curve * in_or_out);
            break;
        case EARS_FADE_SCURVE:  // could find a faster formula, this is CPU consuming
            CLIP_ASSIGN(curve, -1., 1.);
            if (position == 0.5)
                return 0.5;
            else if (position < 0.5)
                return rescale_with_slope(position, 0., 0.5, 0., 0.5, curve * in_or_out);
            else
                return 1. - rescale_with_slope(1. - position, 0., 0.5, 0., 0.5, curve * in_or_out);
            break;
        default:
            return 1.;
            break;
    }
}

float ears_get_fade_in_factor(e_ears_fade_types fade, double position, double curve)
{
    return ears_get_fade_factor(1, fade, position, curve);
}

float ears_get_fade_out_factor(e_ears_fade_types fade, double position, double curve)
{
    return ears_get_fade_factor(-1, fade, position, curve);
}


t_ears_err ears_buffer_fade(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, long fade_in_samples, long fade_out_samples, e_ears_fade_types fade_in_type, e_ears_fade_types fade_out_type, double fade_in_curve, double fade_out_curve)
{
    if (source == dest)
        return ears_buffer_fade_inplace(ob, source, fade_in_samples, fade_out_samples, fade_in_type, fade_out_type, fade_in_curve, fade_out_curve);
    
    t_ears_err err = EARS_ERR_NONE;
    float *orig_sample = buffer_locksamples(source);
    
    if (!orig_sample) {
        err = EARS_ERR_CANT_READ;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
    } else {
        t_atom_long	channelcount = buffer_getchannelcount(source);
        t_atom_long	framecount   = buffer_getframecount(source);
        
        long actual_fade_in_samples = CLAMP(fade_in_samples, 0, framecount);
        long actual_fade_out_samples = CLAMP(fade_out_samples, 0, framecount);
        
        ears_buffer_set_size_and_numchannels(ob, dest, framecount, channelcount);
        float *dest_sample = buffer_locksamples(dest);
        
        t_atom_long dest_channelcount = buffer_getchannelcount(dest);
        
        if (!dest_sample) {
            err = EARS_ERR_CANT_WRITE;
            object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
        } else {
            long j, c;

            // copying
            sysmem_copyptr(orig_sample, dest_sample, framecount * dest_channelcount * sizeof(float));
            
            // applying fade in
            for (j = 0; j < actual_fade_in_samples; j++) {
                for (c = 0; c < dest_channelcount; c++)
                    dest_sample[j*dest_channelcount + c] *= ears_get_fade_in_factor(fade_in_type, ((float)j)/fade_in_samples, fade_in_curve);
            }

            // applying fade out
            for (j = framecount - actual_fade_out_samples; j < framecount; j++) {
                for (c = 0; c < dest_channelcount; c++)
                    dest_sample[j*dest_channelcount + c] *= ears_get_fade_out_factor(fade_out_type, (framecount - (float)j)/fade_out_samples, fade_out_curve);
            }
            
            buffer_setdirty(dest);
            buffer_unlocksamples(dest);
        }
        buffer_unlocksamples(source);
    }
    
    return err;
}

t_ears_err ears_buffer_fade_ms(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, long fade_in_ms, long fade_out_ms, e_ears_fade_types fade_in_type, e_ears_fade_types fade_out_type, double fade_in_curve, double fade_out_curve)
{
    if (source == dest)
        return ears_buffer_fade_ms_inplace(ob, source, fade_in_ms, fade_out_ms, fade_in_type, fade_out_type, fade_in_curve, fade_out_curve);

    
    float msr = buffer_getmillisamplerate(source);
    long fade_in_samps = round(fade_in_ms * msr);
    long fade_out_samps = round(fade_out_ms * msr);
    
    return ears_buffer_fade(ob, source, dest, fade_in_samps, fade_out_samps, fade_in_type, fade_out_type, fade_in_curve, fade_out_curve);
}


t_ears_err ears_buffer_fade_inplace(t_object *ob, t_buffer_obj *buf, long fade_in_samples, long fade_out_samples, e_ears_fade_types fade_in_type, e_ears_fade_types fade_out_type, double fade_in_curve, double fade_out_curve)
{
    t_ears_err err = EARS_ERR_NONE;
    float *sample = buffer_locksamples(buf);
    
    if (!sample) {
        err = EARS_ERR_CANT_READ;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
    } else {
        t_atom_long	channelcount = buffer_getchannelcount(buf);
        t_atom_long	framecount   = buffer_getframecount(buf);
        
        long actual_fade_in_samples = CLAMP(fade_in_samples, 0, framecount);
        long actual_fade_out_samples = CLAMP(fade_out_samples, 0, framecount);

        long j, c;
        
        // applying fade in
        for (j = 0; j < actual_fade_in_samples; j++) {
            for (c = 0; c < channelcount; c++)
                sample[j*channelcount + c] *= ears_get_fade_in_factor(fade_in_type, ((float)j)/fade_in_samples, fade_in_curve);
        }
        
        // applying fade out
        for (j = framecount - actual_fade_out_samples; j < framecount; j++) {
            for (c = 0; c < channelcount; c++)
                sample[j*channelcount + c] *= ears_get_fade_out_factor(fade_out_type, (framecount - (float)j)/fade_out_samples, fade_out_curve);
        }
        
        buffer_setdirty(buf);
        buffer_unlocksamples(buf);
    }
    
    return err;
}

t_ears_err ears_buffer_fade_ms_inplace(t_object *ob, t_buffer_obj *buf, long fade_in_ms, long fade_out_ms, e_ears_fade_types fade_in_type, e_ears_fade_types fade_out_type, double fade_in_curve, double fade_out_curve)
{
    float msr = buffer_getmillisamplerate(buf);
    long fade_in_samps = round(fade_in_ms * msr);
    long fade_out_samps = round(fade_out_ms * msr);
    
    return ears_buffer_fade_inplace(ob, buf, fade_in_samps, fade_out_samps, fade_in_type, fade_out_type, fade_in_curve, fade_out_curve);
}


t_ears_err ears_buffer_mix(t_object *ob, t_buffer_obj **source, long num_sources, t_buffer_obj *dest, t_llll *gains, long *offset_samps, e_ears_normalization_modes normalization_mode)
{
    t_ears_err err = EARS_ERR_NONE;
    
    if (num_sources == 0) {
        ears_buffer_set_size(ob, dest, 0);
        return EARS_ERR_NONE;
    }
    
    
    long i, num_locked = 0;
    float **samples = (float **)bach_newptr(num_sources * sizeof(float *));
    long *num_samples = (long *)bach_newptr(num_sources * sizeof(long));
    long *num_channels = (long *)bach_newptr(num_sources * sizeof(long));

    t_atom_long	channelcount = 0;
    double total_length = 0;
    float *dest_sample = NULL;
    
    channelcount = buffer_getchannelcount(source[0]);		// number of floats in a frame
    for (i = 0, num_locked = 0; i < num_sources; i++, num_locked++) {
        samples[i] = buffer_locksamples(source[i]);
        if (!samples[i]) {
            err = EARS_ERR_CANT_WRITE;
            object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
            goto end;
        }
    }
    
    
    for (i = 0; i < num_sources; i++) {
        num_samples[i] = buffer_getframecount(source[i]);
        num_channels[i] = buffer_getchannelcount(source[i]);
    }

    // Getting max num samples
    total_length = num_samples[0] + offset_samps[0];
    for (i = 1; i < num_sources; i++)
        if (num_samples[i] + offset_samps[i] > total_length)
            total_length = num_samples[i] + offset_samps[i];

    ears_buffer_set_size_and_numchannels(ob, dest, total_length, channelcount);

    dest_sample = buffer_locksamples(dest);
    if (!dest_sample) {
        err = EARS_ERR_CANT_WRITE;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
    } else {
        // erasing samples
        memset(dest_sample, 0, total_length * channelcount * sizeof(float));
        
        // writing samples
        long j, c;
        t_llllelem *elem;
        for (i = 0, elem = gains ? gains->l_head : NULL; i < num_sources; i++, elem = (elem && elem->l_next) ? elem->l_next : elem) {
            
            long this_onset_samps = offset_samps[i] > 0 ? offset_samps[i] : 0;
            
            t_ears_envelope_iterator eei = ears_envelope_iterator_create_from_llllelem(elem, 1., false);
            for (j = 0; j < num_samples[i]; j++) {
                double this_gain = ears_envelope_iterator_walk_interp(&eei, j, num_samples[i]);
                for (c = 0; c < num_channels[i] && c < channelcount; c++)
                    dest_sample[(j + this_onset_samps) * channelcount + c] += samples[i][j * num_channels[i] + c] * this_gain;
            }
        }
        
        buffer_setdirty(dest);
        buffer_unlocksamples(dest);
    }
    
end:
    for (i = 0; i < num_locked; i++)
        buffer_unlocksamples(source[i]);
    
    bach_freeptr(samples);
    bach_freeptr(num_samples);
    bach_freeptr(num_channels);

    // Finally, we normalize if needed
    switch (normalization_mode) {
        case EARS_NORMALIZE_DO:
            ears_buffer_normalize_inplace(ob, dest, 1.);
            break;
            
        case EARS_NORMALIZE_OVERLOAD_PROTECTION_ONLY:
        {
            double maxabs = 0.;
            t_ears_err err = ears_buffer_get_maxabs(ob, dest, &maxabs);
            if (err == EARS_ERR_EMPTY_BUFFER)
                object_warn(ob, EARS_ERROR_BUF_EMPTY_BUFFER);
            if (err == EARS_ERR_NONE && maxabs > 1.) {
                object_warn(ob, "Mixdown peak is %.3f, output buffer will be normalized due to overload protection.", maxabs);
                ears_buffer_normalize_inplace(ob, dest, 1.);
            }
        }
            break;
            
        case EARS_NORMALIZE_DONT:
        default:
            break;
    }
    
    
    return err;
}


t_ears_err ears_buffer_mix_from_llll(t_object *ob, t_llll *sources_ll, t_buffer_obj *dest, t_llll *gains, t_llll *offset_samps_ll, e_ears_normalization_modes normalization_mode)
{
    long num_sources = sources_ll->l_size;
    
    if (num_sources == 0)
        return ears_buffer_pack(ob, 0, NULL, dest);
    else {
        long i = 0;
        long *offset_samps = (long *)bach_newptrclear(num_sources * sizeof(long));
        t_buffer_obj **sources = (t_buffer_obj **)bach_newptr(num_sources * sizeof(t_buffer_obj *));

        for (t_llllelem *el = sources_ll->l_head; el; el = el->l_next, i++)
            sources[i] = (t_buffer_obj *)hatom_getobj(&el->l_hatom);
        
        i = 0;
        for (t_llllelem *el = offset_samps_ll->l_head; el; el = el->l_next, i++)
            offset_samps[i] = hatom_getlong(&el->l_hatom);

        t_max_err err = ears_buffer_mix(ob, sources, num_sources, dest, gains, offset_samps, normalization_mode);
        bach_freeptr(sources);
        bach_freeptr(offset_samps);
        return err;
    }
}



t_ears_err ears_buffer_concat(t_object *ob, t_buffer_obj **source, long num_sources, t_buffer_obj *dest,
                              long *xfade_left_samples, long *xfade_right_samples, char also_fade_boundaries,
                              e_ears_fade_types fade_type, double fade_curve)
{
    t_ears_err err = EARS_ERR_NONE;

    if (num_sources == 0) {
        ears_buffer_set_size(ob, dest, 0);
        return EARS_ERR_NONE;
    } else if (num_sources == 1) {
        if (also_fade_boundaries)
            return ears_buffer_fade(ob, source[0], dest, xfade_right_samples[0], xfade_left_samples[0], fade_type, fade_type, fade_curve, fade_curve);
        else
            return ears_buffer_clone(ob, source[0], dest);
    }
    
    // here we have at least 2 buffers to concatenate
        
    long i, num_locked = 0;
    float **samples = (float **)bach_newptr(num_sources * sizeof(float *));
    long *num_samples = (long *)bach_newptr(num_sources * sizeof(long));
    long *num_channels = (long *)bach_newptr(num_sources * sizeof(long));

    // position in samples of beginning, end, end of fade in and beginning of fade out, for each buffer
    long *sample_start = (long *)bach_newptr(num_sources * sizeof(long));
    long *sample_fadein_end = (long *)bach_newptr(num_sources * sizeof(long)); // this is the FIRST sample of the non-fade region
    long *sample_fadeout_start = (long *)bach_newptr(num_sources * sizeof(long));
    long *sample_end = (long *)bach_newptr(num_sources * sizeof(long)); // this is the FIRST sample of the region AFTER the buffer,
                                                                        // so that sample_end-sample_start = actual samples in the buffer
    t_atom_long	channelcount = 0;
    double total_length = 0;
    float *dest_sample = NULL;
    
    for (i = 0, num_locked = 0; i < num_sources; i++, num_locked++) {
        samples[i] = buffer_locksamples(source[i]);
        if (!samples[i]) {
            err = EARS_ERR_CANT_WRITE;
            object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
            goto end;
        }
    }
    
    for (i = 0; i < num_sources; i++) {
        num_samples[i] = buffer_getframecount(source[i]);
        num_channels[i] = buffer_getchannelcount(source[i]);
    }
    
    /// Calculating total length in samples of dest buffer
    channelcount = buffer_getchannelcount(source[0]);		// number of floats in a frame
    for (i = 0; i < num_sources; i++) {
        if (i == 0) {
            sample_start[i] = 0;
            sample_end[i] = sample_start[i] + num_samples[i];
            sample_fadein_end[i] = also_fade_boundaries ? sample_start[i] + MIN(xfade_right_samples[i]/2, num_samples[i]/2) : 0;
            sample_fadeout_start[i] = sample_end[i] - MIN(MIN(xfade_left_samples[i]/2, num_samples[i]/2), num_samples[i+1]/2);
        } else if (i == num_sources - 1) {
            sample_start[i] = sample_fadeout_start[i-1];
            sample_end[i] = sample_start[i] + num_samples[i];
            sample_fadein_end[i] = sample_start[i] + MIN(MIN(xfade_right_samples[i]/2, num_samples[i]/2), num_samples[i-1]/2);
            sample_fadeout_start[i] = also_fade_boundaries ? sample_end[i] - MIN(xfade_left_samples[i]/2, num_samples[i]/2) : sample_end[i];
        } else {
            sample_start[i] = sample_fadeout_start[i-1];
            sample_end[i] = sample_start[i] + num_samples[i];
            sample_fadein_end[i] = sample_start[i] + MIN(MIN(xfade_right_samples[i]/2, num_samples[i]/2), num_samples[i-1]/2);
            sample_fadeout_start[i] = sample_end[i] - MIN(MIN(xfade_left_samples[i]/2, num_samples[i]/2), num_samples[i+1]/2);
        }
    }

    total_length = sample_end[num_sources-1]; // global length
//    ears_buffer_set_size(ob, dest, total_length);
    ears_buffer_set_size_and_numchannels(ob, dest, total_length, channelcount);
    
    dest_sample = buffer_locksamples(dest);
    if (!dest_sample) {
        err = EARS_ERR_CANT_WRITE;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
    } else {
        // erasing samples
        memset(dest_sample, 0, total_length * channelcount * sizeof(float));
        
        // writing samples
        long j, c;
        for (i = 0; i < num_sources; i++) {
            
            // fade in
            for (j = sample_start[i]; j < sample_fadein_end[i]; j++) {
                long l = j - sample_start[i];
                for (c = 0; c < channelcount && c < num_channels[i] && l < num_samples[i]; c++) {
                    dest_sample[j * channelcount + c] += samples[i][l * num_channels[i] + c] * ears_get_fade_in_factor(fade_type, ((float)l)/(sample_fadein_end[i] - sample_start[i]), fade_curve);
                }
            }
            
            
            // "sustain"
            for ( ; j < sample_fadeout_start[i]; j++) {
                // copying samples
                // TO DO: can be done via sysmem_copyptr()
                long l = j - sample_start[i];
                for (c = 0; c < channelcount && c < num_channels[i] && l < num_samples[i]; c++) {
                    dest_sample[j * channelcount + c] += samples[i][l * num_channels[i] + c];
                }
            }
            
            // fade out
            for ( ; j < sample_end[i]; j++) {
                long l = j - sample_start[i];
                for (c = 0; c < channelcount && c < num_channels[i] && l < num_samples[i]; c++) {
                    dest_sample[j * channelcount + c] += samples[i][l * num_channels[i] + c] * ears_get_fade_out_factor(fade_type, (sample_end[i] - ((float)j))/(sample_end[i] - sample_fadeout_start[i]), fade_curve);
                }
            }
        }
        
        buffer_setdirty(dest);
        buffer_unlocksamples(dest);
    }
    
end:
    for (i = 0; i < num_locked; i++)
        buffer_unlocksamples(source[i]);
    
    bach_freeptr(samples);
    bach_freeptr(num_samples);
    bach_freeptr(num_channels);
    bach_freeptr(sample_fadein_end);
    bach_freeptr(sample_end);
    
    return err;
}





t_symbol *default_filepath2buffername(t_symbol *filepath, long buffer_index)
{
    char temp[MAX_PATH_CHARS * 2];
    snprintf_zero(temp, MAX_PATH_CHARS * 2, "%s_%ld_earsbuf", filepath->s_name, buffer_index);
    return gensym(temp);
}



long ears_buffer_read_handle_mp3(t_object *ob, char *filename, double start_sample, double end_sample, t_buffer_obj *buf)
{
    long ears_err = EARS_ERR_NONE;
    
#ifdef EARS_FROMFILE_NATIVE_MP3_HANDLING
    mpg123_handle *mh;
    int err;
    int res = MPG123_OK;
    long init_samplerate = 44100; // TO DO: change this
    
    /* initializations */
    mh = mpg123_new(NULL, &err);
    
    mpg123_format_none(mh);
    mpg123_format(mh, init_samplerate, MPG123_MONO | MPG123_STEREO, MPG123_ENC_FLOAT_32);
    mpg123_param(mh, MPG123_FORCE_RATE, init_samplerate, 0.);
    
    
    /* open the file and get the decoding format */
    int channels, encoding;
    long rate;
    mpg123_open(mh, filename);
    mpg123_getformat(mh, &rate, &channels, &encoding);
    
    
    /* set the output format */
    mpg123_format_none(mh);
    mpg123_format(mh, rate, channels, MPG123_ENC_FLOAT_32);
    
    
    
    /* fixing boundaries */
    if (start_sample < 0)
        start_sample = 0;
    
    if (end_sample < 0) {
        mpg123_scan(mh);
        end_sample = mpg123_length(mh);
    }
    
    if (end_sample < start_sample) {
        long tmp = end_sample;
        end_sample = start_sample;
        start_sample = tmp;
    }
    
    
    
    long num_samples = end_sample - start_sample;
    size_t buffer_size = num_samples * channels * 4;
    unsigned char *buffer = (unsigned char*) bach_newptr(buffer_size * sizeof(unsigned char));
    
    
    mpg123_seek(mh, start_sample, SEEK_SET);
    
    size_t done;
    res = mpg123_read(mh, buffer, buffer_size, &done);
    
    if (done > 0) { // we have something to write
        ears_buffer_set_size_and_numchannels(ob, buf, num_samples, channels);
        ears_buffer_set_sr(ob, buf, rate);
        
        
        long numsamps = ears_buffer_get_size_samps(ob, buf);
        
        float *sample = buffer_locksamples(buf);
        
        if (!sample) {
            err = EARS_ERR_CANT_READ;
            object_error(ob, EARS_ERROR_BUF_CANT_READ);
        } else {
            
            /*            for (long i = 0; i < done/4; i++) {
             double this_value = (t_double) ((t_float *)buffer)[i];
             sample[i] = this_value;
             } */
            sysmem_copyptr(buffer, sample, buffer_size * sizeof(unsigned char));
            buffer_setdirty(buf);
            buffer_unlocksamples(buf);
            
        }
    }
    
    if (mh) {
        mpg123_close(mh);
        mpg123_delete(mh);
    }
    
    bach_freeptr(buffer);
    
#endif

    return ears_err;
}


// This is not used by the [ears.read] object, but rather from other objects.
// The <dest> buffers are meant to be used and freed.
t_ears_err ears_buffer_from_file(t_object *ob, t_buffer_obj **dest, t_symbol *file, double start_ms, double end_ms, double sr, long buffer_idx)
{
    t_ears_err err = EARS_ERR_NONE;
    t_symbol *filepath = ears_ezlocate_file(file, NULL);
    
    if (filepath) {
        // creating a buffer object!
        
        t_symbol *name = default_filepath2buffername(filepath, buffer_idx);
        t_atom a;
        atom_setsym(&a, name);
        
        *dest = (t_buffer_obj *)object_new_typed(CLASS_BOX, gensym("buffer~"), 1, &a);
        
        if (!*dest) {
            err = EARS_ERR_GENERIC;
        } else {
            
#ifdef EARS_FROMFILE_NATIVE_MP3_HANDLING
            if (ears_filename_ends_with(filepath, ".mp3", true)) {
                long startsamp = start_ms >= 0 ? ears_ms_to_samps(start_ms, sr) : -1;
                long endsamp = end_ms >= 0 ? ears_ms_to_samps(end_ms, sr) : -1;
                err = ears_buffer_read_handle_mp3(ob, filepath->s_name, startsamp, endsamp, *dest);
            } else {
#endif
                // trying to load file into input buffer
                atom_setsym(&a, filepath);
                typedmess(*dest, gensym("importreplace"), 1, &a);
                
                // possibly cropping to given portions
                if (start_ms > 0 || end_ms >= 0) {
                    ears_buffer_crop_ms_inplace(ob, *dest, start_ms, end_ms);
                }
                
#ifdef EARS_FROMFILE_NATIVE_MP3_HANDLING
            }
#endif
        }
        
    } else {
        // can't locate file!
        err = EARS_ERR_NO_FILE;
        object_error(ob, EARS_ERROR_BUF_NO_FILE_NAMED, filepath ? filepath->s_name : (file ? file->s_name : "???"));
    }
    
    return err;
}




t_ears_err ears_buffer_repeat(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, long new_numsamples)
{
    if (!source || !dest)
        return EARS_ERR_NO_BUFFER;
    
    t_ears_err err = EARS_ERR_NONE;
    float *orig_sample = buffer_locksamples(source);
    float *orig_sample_wk = NULL;
    
    if (!orig_sample) {
        err = EARS_ERR_CANT_READ;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
    } else {
        t_atom_long	channelcount = buffer_getchannelcount(source);		// number of floats in a frame
        t_atom_long	framecount   = buffer_getframecount(source);			// number of floats long the buffer is for a single channel
        
        if (source == dest) { // inplace operation!
            orig_sample_wk = (float *)bach_newptr(channelcount * framecount * sizeof(float));
            sysmem_copyptr(orig_sample, orig_sample_wk, channelcount * framecount * sizeof(float));
            buffer_unlocksamples(source);
        } else {
            orig_sample_wk = orig_sample;
            ears_buffer_copy_format(ob, source, dest);
        }

        ears_buffer_set_size(ob, dest, new_numsamples);
        
        float *dest_sample = buffer_locksamples(dest);
        
        if (!dest_sample) {
            err = EARS_ERR_CANT_WRITE;
            object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
        } else {
//            t_atom_long	dest_channelcount = buffer_getchannelcount(dest);
//            t_atom_long	dest_framecount   = buffer_getframecount(dest);

            
            for (long f = 0; f < new_numsamples; f += framecount)
                sysmem_copyptr(orig_sample_wk, dest_sample + f * channelcount, channelcount * MIN(framecount, new_numsamples - f) * sizeof(float));
            
            buffer_setdirty(dest);
            buffer_unlocksamples(dest);
        }
        
        if (source == dest) // inplace operation!
            bach_freeptr(orig_sample_wk);
        else
            buffer_unlocksamples(source);
    }
    
    return err;
}

t_ears_err ears_buffer_repeat_times(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, double num_times)
{
    return ears_buffer_repeat(ob, source, dest, (long)ceil(num_times * ears_buffer_get_size_samps(ob, source)));
}


t_ears_err ears_buffer_shift(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, long shift_samps)
{
    if (shift_samps < 0)
        return ears_buffer_crop(ob, source,dest, -shift_samps, -1);
    
    if (!source || !dest)
        return EARS_ERR_NO_BUFFER;
    
    t_ears_err err = EARS_ERR_NONE;
    float *orig_sample = buffer_locksamples(source);
    float *orig_sample_wk = NULL;
    
    if (!orig_sample) {
        err = EARS_ERR_CANT_READ;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
    } else {
        t_atom_long    channelcount = buffer_getchannelcount(source);        // number of floats in a frame
        t_atom_long    framecount   = buffer_getframecount(source);            // number of floats long the buffer is for a single channel
        
        if (source == dest) { // inplace operation!
            orig_sample_wk = (float *)bach_newptr(channelcount * framecount * sizeof(float));
            sysmem_copyptr(orig_sample, orig_sample_wk, channelcount * framecount * sizeof(float));
            buffer_unlocksamples(source);
        } else {
            orig_sample_wk = orig_sample;
            ears_buffer_copy_format(ob, source, dest);
        }
        
        ears_buffer_set_size(ob, dest, framecount + shift_samps);
        
        float *dest_sample = buffer_locksamples(dest);
        
        if (!dest_sample) {
            err = EARS_ERR_CANT_WRITE;
            object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
        } else {
            //            t_atom_long    dest_channelcount = buffer_getchannelcount(dest);
            //            t_atom_long    dest_framecount   = buffer_getframecount(dest);
            
            for (long i = 0; i < shift_samps * channelcount; i++)
                dest_sample[i] = 0;
            sysmem_copyptr(orig_sample_wk, dest_sample + shift_samps * channelcount, channelcount * framecount * sizeof(float));
            
            buffer_setdirty(dest);
            buffer_unlocksamples(dest);
        }
        
        if (source == dest) // inplace operation!
            bach_freeptr(orig_sample_wk);
        else
            buffer_unlocksamples(source);
    }
    
    return err;
}


t_ears_err ears_buffer_trim(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, double amp_thresh_linear, char trim_start, char trim_end)
{
    if (!source || !dest)
        return EARS_ERR_NO_BUFFER;
    
    if (amp_thresh_linear < 0) { // nothing to trim
        if (source != dest)
            return ears_buffer_clone(ob, source, dest);
    }
    
    t_ears_err err = EARS_ERR_NONE;
    float *orig_sample = buffer_locksamples(source);
    
    if (!orig_sample) {
        err = EARS_ERR_CANT_READ;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
    } else {
        t_atom_long    channelcount = buffer_getchannelcount(source);        // number of floats in a frame
        t_atom_long    framecount   = buffer_getframecount(source);            // number of floats long the buffer is for a single channel
        
        // finding
        long start_samp = 0;
        long end_samp_included = framecount-1;
        
        if (trim_start) {
            while (start_samp < framecount) {
                double max_amp = 0;
                for (long c = 0; c < channelcount; c++)
                    max_amp = MAX(max_amp, orig_sample[start_samp*channelcount + c]);
                if (max_amp > amp_thresh_linear)
                    break;
                start_samp ++;
            }
        }
        if (trim_end) {
            while (end_samp_included >= 0) {
                double max_amp = 0;
                for (long c = 0; c < channelcount; c++)
                    max_amp = MAX(max_amp, orig_sample[end_samp_included*channelcount + c]);
                if (max_amp > amp_thresh_linear)
                    break;
                end_samp_included --;
            }
        }

        buffer_unlocksamples(source);
        
        if (start_samp > 0 || end_samp_included < framecount - 1)
            ears_buffer_crop(ob, source, dest, start_samp, end_samp_included + 1); // crop wants as end sample the FIRST sample not to be taken
    }
    

    return err;
}




t_ears_err ears_buffer_onepole(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, double cutoff_freq, char highpass)
{
    double a0 = 1.;
    double b1 = 0.;
    
    double sr = ears_buffer_get_sr(ob, source);
   
    b1 = exp(-2.0 * PI * (cutoff_freq/sr));
    a0 = 1.0 - b1;
    
    if (!source || !dest)
        return EARS_ERR_NO_BUFFER;
    
    t_ears_err err = EARS_ERR_NONE;
    float *orig_sample = buffer_locksamples(source);
    float *orig_sample_wk = NULL;
    
    if (!orig_sample) {
        err = EARS_ERR_CANT_READ;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
    } else {
        t_atom_long	channelcount = buffer_getchannelcount(source);		// number of floats in a frame
        t_atom_long	framecount   = buffer_getframecount(source);			// number of floats long the buffer is for a single channel
        
        if (source == dest) { // inplace operation!
            orig_sample_wk = (float *)bach_newptr(channelcount * framecount * sizeof(float));
            sysmem_copyptr(orig_sample, orig_sample_wk, channelcount * framecount * sizeof(float));
            buffer_unlocksamples(source);
        } else {
            orig_sample_wk = orig_sample;
            ears_buffer_copy_format(ob, source, dest);
            ears_buffer_set_size(ob, dest, framecount);
        }
        
        float *dest_sample = buffer_locksamples(dest);
        
        if (!dest_sample) {
            err = EARS_ERR_CANT_WRITE;
            object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
        } else {
            
            long idx;
            float *prev = (float *)bach_newptr(channelcount * sizeof(float));
            for (long c = 0; c < channelcount; c++)
                prev[c] = 0.;
            
            for (long f = 0; f < framecount; f++) {
                for (long c = 0; c < channelcount; c++) {
                    idx = f * channelcount + c;
                    dest_sample[idx] = orig_sample_wk[idx] * a0 + prev[c] * b1;
                    prev[c] = dest_sample[idx];
                }
            }
            
            if (highpass) {
                for (long f = 0; f < framecount; f++) {
                    for (long c = 0; c < channelcount; c++) {
                        idx = f * channelcount + c;
                        dest_sample[idx] = orig_sample_wk[idx] - dest_sample[idx];
                    }
                }
            }

            bach_freeptr(prev);
            buffer_setdirty(dest);
            buffer_unlocksamples(dest);
        }
        
        if (source == dest) // inplace operation!
            bach_freeptr(orig_sample_wk);
        else
            buffer_unlocksamples(source);
    }
    
    return err;
}


t_ears_err ears_buffer_biquad(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, double a0, double a1, double a2, double b1, double b2)
{
    
    if (!source || !dest)
        return EARS_ERR_NO_BUFFER;
    
    t_ears_err err = EARS_ERR_NONE;
    float *orig_sample = buffer_locksamples(source);
    float *orig_sample_wk = NULL;
    
    if (!orig_sample) {
        err = EARS_ERR_CANT_READ;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
    } else {
        t_atom_long    channelcount = buffer_getchannelcount(source);        // number of floats in a frame
        t_atom_long    framecount   = buffer_getframecount(source);            // number of floats long the buffer is for a single channel
        
        if (source == dest) { // inplace operation!
            orig_sample_wk = (float *)bach_newptr(channelcount * framecount * sizeof(float));
            sysmem_copyptr(orig_sample, orig_sample_wk, channelcount * framecount * sizeof(float));
            buffer_unlocksamples(source);
        } else {
            orig_sample_wk = orig_sample;
            ears_buffer_copy_format(ob, source, dest);
            ears_buffer_set_size(ob, dest, framecount);
        }
        
        float *dest_sample = buffer_locksamples(dest);
        
        if (!dest_sample) {
            err = EARS_ERR_CANT_WRITE;
            object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
        } else {
            
            long idx;
            float *prev_fb_1 = (float *)bach_newptr(channelcount * sizeof(float));
            float *prev_fb_2 = (float *)bach_newptr(channelcount * sizeof(float));
            for (long c = 0; c < channelcount; c++)
                prev_fb_1[c] = prev_fb_2[c] = 0.;
            
            for (long f = 0; f < framecount; f++) {
                for (long c = 0; c < channelcount; c++) {
                    idx = f * channelcount + c;
                    dest_sample[idx] = orig_sample_wk[idx] * a0 + (f >= 1 ? orig_sample_wk[idx - channelcount] * a1 : 0) + (f >= 2 ? orig_sample_wk[idx - 2 * channelcount] * a2 : 0) - prev_fb_1[c] * b1 - prev_fb_2[c] * b2;
                    prev_fb_2[c] = prev_fb_1[c];
                    prev_fb_1[c] = dest_sample[idx];
                }
            }
            
            bach_freeptr(prev_fb_1);
            bach_freeptr(prev_fb_2);
            buffer_setdirty(dest);
            buffer_unlocksamples(dest);
        }
        
        if (source == dest) // inplace operation!
            bach_freeptr(orig_sample_wk);
        else
            buffer_unlocksamples(source);
    }
    
    return err;
}



t_ears_err ears_buffer_expr(t_object *ob, t_lexpr *expr, t_buffer_obj **arguments, long num_arguments, t_buffer_obj *dest, e_ears_normalization_modes normalization_mode)
{
    t_ears_err err = EARS_ERR_NONE;
    
    if (num_arguments == 0) {
        ears_buffer_set_size(ob, dest, 0);
        return EARS_ERR_NONE;
    }
    
    
    long i, num_locked = 0;
    float **samples = (float **)bach_newptr(num_arguments * sizeof(float *));
    long *num_samples = (long *)bach_newptr(num_arguments * sizeof(long));
    long *num_channels = (long *)bach_newptr(num_arguments * sizeof(long));
    
    t_atom_long    channelcount = 0;
    double total_length = 0;
    float *dest_sample = NULL;
    
    channelcount = buffer_getchannelcount(arguments[0]);        // number of floats in a frame
    for (i = 0, num_locked = 0; i < num_arguments; i++, num_locked++) {
        samples[i] = buffer_locksamples(arguments[i]);
        if (!samples[i]) {
            err = EARS_ERR_CANT_WRITE;
            object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
            goto end;
        }
    }
    
    
    /// All buffers have been locked. Now we need to harmonize numchannels and sampsize.
    /// As a rule: we take the property of the first one.
    num_samples[0] = buffer_getframecount(arguments[0]);
    num_channels[0] = channelcount;
    for (i = 1; i < num_arguments; i++) {
        num_channels[i] = ears_buffer_get_numchannels(ob, arguments[i]); // it happens that copy_format doesn't work in changing the number of channels
        num_samples[i] = ears_buffer_get_size_samps(ob, arguments[i]);
    }
    
    // Getting max num samples and max num channels
    total_length = num_samples[0];
    for (i = 1; i < num_arguments; i++) {
        if (num_samples[i] > total_length)
            total_length = num_samples[i];
        if (num_channels[i] > channelcount)
            channelcount = num_channels[i];
    }
    
    ears_buffer_set_size_and_numchannels(ob, dest, total_length, channelcount);
    
    dest_sample = buffer_locksamples(dest);
    if (!dest_sample) {
        err = EARS_ERR_CANT_WRITE;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
    } else {
        // erasing samples
        memset(dest_sample, 0, total_length * channelcount * sizeof(float));
        
        t_hatom vars[LEXPR_MAX_VARS];
        t_hatom stack[L_MAX_TOKENS];
        hatom_setdouble(stack, 0);
        
        // writing samples
        if (expr) {
            long j, c;
            for (j = 0; j < total_length; j++) {
                for (c = 0; c < channelcount; c++) {
                    // setting variables
                    for (long i = 0; i < num_arguments && i < LEXPR_MAX_VARS; i++)
                        if (c < num_channels[i] && j < num_samples[i])
                            hatom_setdouble(vars+i, samples[i][j * num_channels[i] + c]);
                        else
                            hatom_setdouble(vars+i, 0.);
                    
                    // used to be like this:
                    // TO DO: optimize this stuff, should not be like this
                    //                    t_hatom *res = lexpr_eval(expr, vars);
//                    bach_freeptr(res);

                    lexpr_eval_upon(expr, vars, stack); // optimized version of lexpr_eval for many evaluations
                    dest_sample[j * channelcount + c] = hatom_getdouble(stack);
                }
            }
        }
        
        buffer_setdirty(dest);
        buffer_unlocksamples(dest);
    }
    
end:
    for (i = 0; i < num_locked; i++)
        buffer_unlocksamples(arguments[i]);
    
    bach_freeptr(samples);
    bach_freeptr(num_samples);
    bach_freeptr(num_channels);
    
    // Finally, we normalize if needed
    switch (normalization_mode) {
        case EARS_NORMALIZE_DO:
            ears_buffer_normalize_inplace(ob, dest, 1.);
            break;
            
        case EARS_NORMALIZE_OVERLOAD_PROTECTION_ONLY:
        {
            double maxabs = 0.;
            t_ears_err err = ears_buffer_get_maxabs(ob, dest, &maxabs);
            if (err == EARS_ERR_EMPTY_BUFFER)
                object_warn(ob, EARS_ERROR_BUF_EMPTY_BUFFER);
            if (err == EARS_ERR_NONE && maxabs > 1.) {
                object_warn(ob, "Output peak is %.3f, output buffer will be normalized due to overload protection.", maxabs);
                ears_buffer_normalize_inplace(ob, dest, 1.);
            }
        }
            break;
            
        case EARS_NORMALIZE_DONT:
        default:
            break;
    }
    
    
    return err;
}





void ears_writeaiff(t_object *buf, t_symbol *filename)
{
    t_atom a;
    atom_setsym(&a, filename);
    typedmess(buf, gensym("writeaiff"), 1, &a);
}

void ears_writewave(t_object *buf, t_symbol *filename)
{
    t_atom a;
    atom_setsym(&a, filename);
    typedmess(buf, gensym("writewave"), 1, &a);
}

void ears_writeflac(t_object *buf, t_symbol *filename)
{
    t_atom a;
    atom_setsym(&a, filename);
    typedmess(buf, gensym("writeflac"), 1, &a);
}

void ears_writeraw(t_object *buf, t_symbol *filename)
{
    t_atom a;
    atom_setsym(&a, filename);
    typedmess(buf, gensym("writeraw"), 1, &a);
}



void ears_writemp3(t_object *buf, t_symbol *filename)
{
    int write;
    t_symbol *resolved_path = filename;
    path_absolutepath(&resolved_path, filename, NULL, 0);
    
    char conformed_path[MAX_PATH_CHARS];
    path_nameconform(resolved_path->s_name, conformed_path, PATH_STYLE_MAX, PATH_TYPE_BOOT);
    FILE *mp3 = fopen(conformed_path, "wb");

    
    if (!mp3) {
        error("Cannot open file %s for write", resolved_path->s_name);
        return;
    }
    
    const int PCM_SIZE = 8192;
    const int MP3_SIZE = 8192;
    
    float pcm_buffer_l[PCM_SIZE];
    float pcm_buffer_r[PCM_SIZE];
    unsigned char mp3_buffer[MP3_SIZE];
    int sr = ears_buffer_get_sr(NULL, buf);
    
    lame_t lame = lame_init();
    lame_set_in_samplerate(lame, sr);
    lame_set_VBR(lame, vbr_default);
    lame_init_params(lame);
    
    float *sample = buffer_locksamples(buf);
    if (!sample) {
        error("Can't read buffer!");
    } else {
        t_atom_long    channelcount = buffer_getchannelcount(buf);        // number of floats in a frame
        t_atom_long    framecount   = buffer_getframecount(buf);            // number of floats long the buffer is for a single channel
        
        long i = 0;
        while (i < framecount) {
            long nsamples = MIN(PCM_SIZE, framecount - i);
            for (long j = 0; j < nsamples; j++) {
                pcm_buffer_l[j] = sample[(i+j) * channelcount];
                pcm_buffer_r[j] = channelcount > 1 ? sample[(i+j) * channelcount + 1] : sample[(i+j) * channelcount];
            }
            write = lame_encode_buffer_ieee_float(lame, pcm_buffer_l, pcm_buffer_r, nsamples, mp3_buffer, MP3_SIZE);
            fwrite(mp3_buffer, write, 1, mp3);
            
            i += nsamples;
        }
        write = lame_encode_flush(lame, mp3_buffer, MP3_SIZE);
        buffer_unlocksamples(buf);
    }

    lame_close(lame);
    fclose(mp3);
}

const char *get_filename_ext(const char *filename)
{
    const char *dot = strrchr(filename, '.');
    if (!dot || dot == filename) return "";
    return dot + 1;
}

void ears_write_buffer(t_object *buf, t_symbol *filename, t_object *culprit)
{
    const char *ext = get_filename_ext(filename->s_name);
    if (!strcmp(ext, "aif") || !strcmp(ext, "aiff"))
        ears_writeaiff(buf, filename);
    else if (!strcmp(ext, "wav") || !strcmp(ext, "wave"))
        ears_writewave(buf, filename);
    else if (!strcmp(ext, "flac"))
        ears_writeflac(buf, filename);
    else if (!strcmp(ext, "data"))
        ears_writeraw(buf, filename);
    else if (!strcmp(ext, "mp3"))
        ears_writemp3(buf, filename);
    else {
        object_error(culprit, "Could not determine file tipe from extension.");
        object_error(culprit, "       Please use one of the following extensions: aif(f), wav(e), mp3, flac or data.");
    }
}


void ears_envelope_get_max_x(t_llllelem *el, t_atom *a_max)
{
    atom_setlong(a_max, 0);
    if (el && hatom_gettype(&el->l_hatom) == H_LLLL) {
        t_llll *ll = hatom_getllll(&el->l_hatom);
        if (ll->l_tail && hatom_gettype(&ll->l_tail->l_hatom) == H_LLLL) {
            t_llll *subll = hatom_getllll(&ll->l_tail->l_hatom);
            if (subll && subll->l_head) {
                if (hatom_gettype(&subll->l_head->l_hatom) == H_LONG)
                    atom_setlong(a_max, hatom_getlong(&subll->l_head->l_hatom));
                else
                    atom_setfloat(a_max, hatom_getdouble(&subll->l_head->l_hatom));
            }
        }
    }
}



t_ears_err ears_buffer_get_split_points_samps_silence(t_object *ob, t_buffer_obj *buf, double thresh_linear, double min_silence_samps, t_llll **samp_start, t_llll **samp_end, char keep_silence)
{
    if (!buf) {
        *samp_start = llll_get();
        *samp_end = llll_get();
        return EARS_ERR_NO_BUFFER;
    }
    
    t_ears_err err = EARS_ERR_NONE;
    float *sample = buffer_locksamples(buf);
    
    if (!sample) {
        err = EARS_ERR_CANT_READ;
        *samp_start = llll_get();
        *samp_end = llll_get();
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
    } else {
        char in_silence = true;
        long curr_num_silence_samps = 0;
        char reverse = (keep_silence == 2);
        *samp_start = llll_get();
        *samp_end = llll_get();
        
        t_atom_long    channelcount = buffer_getchannelcount(buf);
        t_atom_long    framecount   = buffer_getframecount(buf);
        
        if (reverse) {
            char testing_silence = false;
            in_silence = false;
            for (long f = 0; f < framecount; f++) {
                float max_amp = 0;
                for (long c = 0; c < channelcount; c++)
                    max_amp = MAX(max_amp, fabs(sample[f * channelcount + c]));
                
                if (!in_silence && max_amp < thresh_linear) {
                    curr_num_silence_samps++;
                    in_silence = true;
                    testing_silence = true;
                } else if (in_silence) {
                    if (max_amp >= thresh_linear) {
                        if (!testing_silence)
                            llll_appendlong(*samp_end, f);
                        in_silence = false;
                        curr_num_silence_samps = 0;
                    } else {
                        curr_num_silence_samps++;
                        if (testing_silence && curr_num_silence_samps >= min_silence_samps) {
                            testing_silence = false;
                            llll_appendlong(*samp_start, f - curr_num_silence_samps + 1);
                        }
                    }
                }
            }
            
            while ((*samp_end)->l_size < (*samp_start)->l_size)
                llll_appendlong(*samp_end, framecount);

        } else {
            char in_silence = true;
            for (long f = 0; f < framecount; f++) {
                float max_amp = 0;
                for (long c = 0; c < channelcount; c++)
                    max_amp = MAX(max_amp, fabs(sample[f * channelcount + c]));
                
                if (!in_silence && max_amp < thresh_linear) {
                    curr_num_silence_samps++;
                    if (curr_num_silence_samps >= min_silence_samps) {
                        llll_appendlong(*samp_end, f - curr_num_silence_samps + 1);
                        in_silence = true;
                    }
                } else if (in_silence) {
                    if (max_amp >= thresh_linear) {
                        llll_appendlong(*samp_start, f);
                        in_silence = false;
                        curr_num_silence_samps = 0;
                    } else {
                        curr_num_silence_samps++;
                    }
                }
            }
            
            while ((*samp_end)->l_size < (*samp_start)->l_size)
                llll_appendlong(*samp_end, framecount);
        }
        
        llll_print(*samp_start);
        llll_print(*samp_end);
        
        if (keep_silence == 1) {
            t_llllelem *s, *e;
            for (s = (*samp_start)->l_head->l_next, e = (*samp_end)->l_head; e; s = s ? s->l_next : NULL, e=e->l_next) {
                if (!s)
                    hatom_setlong(&e->l_hatom, framecount);
                else
                    hatom_setlong(&e->l_hatom, hatom_getlong(&s->l_hatom));
            }
        }

        buffer_unlocksamples(buf);
    }
    
    return err;
}





double ears_ratio_to_cents(double ratio)
{
    return 1200 * log2(ratio);
}

double ears_cents_to_ratio(double cents)
{
    return pow(2, cents/1200.);
}

