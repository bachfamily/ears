#include "ext.h"
#include "ext_obex.h"
#include "llll_commons_ext.h"
#include "bach_math_utilities.h"
#include "ears.object.h"
#include "ears.windows.h"
#include "ears.mp3.h"
#include "ears.wavpack.h"
#include <vector>

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

t_symbol *ears_buffer_get_sampleformat(t_object *ob, t_buffer_obj *buf)
{
    return object_attr_getsym(buf, gensym("format"));
}

t_ears_err ears_buffer_set_sampleformat(t_object *ob, t_buffer_obj *buf, t_symbol *sampleformat)
{
    object_attr_setsym(buf, gensym("format"), sampleformat);
    return EARS_ERR_NONE;
}



t_ears_err ears_buffer_set_size_samps(t_object *ob, t_buffer_obj *buf, long num_frames)
{
    if (num_frames != ears_buffer_get_size_samps(ob, buf)) {
        t_atom a;
        atom_setlong(&a, num_frames);
        typedmess(buf, gensym("sizeinsamps"), 1, &a);
    }
    return EARS_ERR_NONE;
}

t_ears_err ears_buffer_set_size_samps_preserve(t_object *ob, t_buffer_obj *buf, long num_frames)
{
    long curr_num_frames = ears_buffer_get_size_samps(ob, buf);
    if (num_frames < curr_num_frames) {
        ears_buffer_crop_inplace(ob, buf, 0, num_frames);
    } else if (num_frames > curr_num_frames) {
        t_atom_long channelcount = ears_buffer_get_numchannels(ob, buf);
        float *temp = (float *)bach_newptr(curr_num_frames * channelcount * sizeof(float));
        float *sample = buffer_locksamples(buf);
        sysmem_copyptr(sample, temp, curr_num_frames * channelcount * sizeof(float));
        buffer_unlocksamples(buf);
        ears_buffer_set_size_samps(ob, buf, num_frames);
        sample = buffer_locksamples(buf);
        sysmem_copyptr(temp, sample, curr_num_frames * channelcount * sizeof(float));
        buffer_unlocksamples(buf);
        bach_freeptr(temp);
    }
    return EARS_ERR_NONE;
}

t_ears_err ears_buffer_set_sr(t_object *ob, t_buffer_obj *buf, double sr)
{
    if (sr != ears_buffer_get_sr(ob, buf)) {
        t_atom a;
        atom_setfloat(&a, sr);
        typedmess(buf, gensym("sr"), 1, &a);
    }
    return EARS_ERR_NONE;
}

t_ears_err ears_buffer_clear(t_object *ob, t_buffer_obj *buf)
{
    typedmess(buf, gensym("clear"), 0, NULL);
    return EARS_ERR_NONE;
}

t_symbol *ears_buffer_get_name(t_object *ob, t_buffer_obj *buf)
{
    t_buffer_info info;
    info.b_name = NULL;
    buffer_getinfo(buf, &info);
    return info.b_name;
}




bool ears_buffers_have_the_same_sr(t_object *ob, long num_buffers, t_buffer_obj **buffer)
{
    if (num_buffers <= 1)
        return true;
    
    double sr = -1;
    for (long i = 0; i < num_buffers; i++) {
        if (!buffer[i])
            continue;
        double this_sr = ears_buffer_get_sr(ob, buffer[i]);
        if (sr < 0) sr = this_sr;
        if (this_sr != sr)
            return false;
    }
    return true;
}

double ears_buffers_get_lowest_sr(t_object *ob, long num_buffers, t_buffer_obj **buffer)
{
    if (num_buffers < 1)
        return 0;
    
    double sr = DBL_MAX;
    for (long i = 0; i < num_buffers; i++) {
        if (!buffer[i])
            continue;
        double this_sr = ears_buffer_get_sr(ob, buffer[i]);
        if (this_sr < sr)
            sr = this_sr;
    }
    return sr == DBL_MAX ? 0 : sr;
}

double ears_buffers_get_highest_sr(t_object *ob, long num_buffers, t_buffer_obj **buffer)
{
    if (num_buffers < 1)
        return 0;
    
    double sr = DBL_MIN;
    for (long i = 0; i < num_buffers; i++) {
        if (!buffer[i])
            continue;
        double this_sr = ears_buffer_get_sr(ob, buffer[i]);
        if (this_sr > sr)
            sr = this_sr;
    }
    return sr == DBL_MIN ? 0 : sr;
}

bool is_in_vec_double(double num, std::vector<double> &vec)
{
    for (long i = 0; i < vec.size(); i++)
        if (num == vec[i])
            return true;
    return false;
}

double ears_buffers_get_mostcommon_sr(t_object *ob, long num_buffers, t_buffer_obj **buffer)
{
    if (num_buffers < 1)
        return 0;
    
    double most_common_sr = 0;
    long most_common_count = 0;
    std::vector<double> processed_sr;
    for (long i = 0; i < num_buffers; i++) {
        if (!buffer[i])
            continue;
        
        double this_sr = ears_buffer_get_sr(ob, buffer[i]);
        if (is_in_vec_double(this_sr, processed_sr))
            continue;
        processed_sr.push_back(this_sr);
        long count = 1;
        for (long j = i+1; j < num_buffers; j++) {
            if (!buffer[j])
                continue;
            if (ears_buffer_get_sr(ob, buffer[j]) == this_sr)
                count++;
        }
        if (count > most_common_count) {
            most_common_sr = this_sr;
            most_common_count = count;
        }
    }
    return most_common_sr;
}


double ears_buffers_get_collective_sr(t_object *ob, long numbuffers, t_buffer_obj **buffer, e_ears_resamplingpolicy resamplingpolicy)
{
    switch (resamplingpolicy) {
        case EARS_RESAMPLINGPOLICY_TOLOWESTSR:
            return ears_buffers_get_lowest_sr(ob, numbuffers, buffer);
            break;
            
        case EARS_RESAMPLINGPOLICY_TOHIGHESTSR:
            return ears_buffers_get_highest_sr(ob, numbuffers, buffer);
            break;

        case EARS_RESAMPLINGPOLICY_TOMOSTCOMMONSR:
            return ears_buffers_get_mostcommon_sr(ob, numbuffers, buffer);
            break;

        case EARS_RESAMPLINGPOLICY_TOCURRENTMAXSR:
            return sys_getsr();
            break;
            
        default:
            return 0;
            break;
    }
}


// THESE 3 ARE TOO DANGEROUS
/*
long get_hidden_sr_offset()
{
    return (sizeof(t_object) + 9*sizeof(long) + 3*sizeof(float)+ 2*sizeof(float*) + sizeof(t_symbol*)+2*sizeof(short)+sizeof(void*) + 2 * sizeof(double) + 7*sizeof(short))/sizeof(short);
}

t_ears_err ears_buffer_set_hidden_sr(t_object *ob, t_buffer_obj *buf, double sr)
{
    long offset = get_hidden_sr_offset();

    unsigned int sr_int = sr;
    unsigned short s1 = sr_int % (1 << 16);
    unsigned short s2 = sr_int >> 16;
    
    *(((unsigned short *)buf)+offset) = (unsigned short)s1;
    *(((unsigned short *)buf)+offset+1) = (unsigned short)s2;
    return EARS_ERR_NONE;
}


t_atom_float ears_buffer_get_hidden_sr(t_object *ob, t_buffer_obj *buf)
{
    long offset = get_hidden_sr_offset();
    unsigned short s1 = *(((unsigned short *)buf)+offset);
    unsigned short s2 = *(((unsigned short *)buf)+offset+1);

    return (s2 << 16) + s1;
}
*/


// THESE ONES ARE BETTER

// turns a spectralbuf into an ordinary buffer
t_ears_err ears_spectralbuf_metadata_remove(t_object *ob, t_buffer_obj *buf)
{
    t_ears_spectralbuf_metadata *data = ears_spectralbuf_metadata_get(ob, buf);
    if (data) {
        llll_free(data->bins);
        data->bins = NULL;
        if (hashtab_delete(ears_hashtab_spectrograms_get(), ears_buffer_get_name(ob, buf)) == MAX_ERR_NONE)
            return EARS_ERR_NONE;
    }
    return EARS_ERR_GENERIC;
}

t_ears_err ears_spectralbuf_metadata_set(t_object *ob, t_buffer_obj *buf, t_ears_spectralbuf_metadata *data)
{
    t_symbol *buffer_name = ears_buffer_get_name(ob, buf);
    if (buffer_name) {
        t_ears_spectralbuf_metadata *storeddata = (t_ears_spectralbuf_metadata *)sysmem_newptrclear(sizeof(t_ears_spectralbuf_metadata));
        storeddata->original_audio_signal_sr = data->original_audio_signal_sr;
        storeddata->binsize = data->binsize;
        storeddata->binoffset = data->binoffset;
        storeddata->binunit = data->binunit;
        storeddata->type = data->type;
        llll_free(storeddata->bins);
        storeddata->bins = llll_clone(data->bins);
        if (ears_hashtab_spectrograms_store(buffer_name, storeddata) == 0) {
            ears_buffer_set_sampleformat(ob, buf, _sym_float32); // also sets sample format as float32
            return EARS_ERR_NONE;
        }
        return EARS_ERR_GENERIC;
    }
    
    return EARS_ERR_GENERIC;
}

bool ears_buffer_is_spectral(t_object *ob, t_buffer_obj *buf)
{
    if (ears_spectralbuf_metadata_get(ob, buf) == NULL)
        return false;
    return true;
}

// When you use this function, you should NOT FREE the obtained metadata, you don't own them
t_ears_spectralbuf_metadata *ears_spectralbuf_metadata_get(t_object *ob, t_buffer_obj *buf)
{
    t_symbol *buffer_name = ears_buffer_get_name(ob, buf);
    return (t_ears_spectralbuf_metadata *)ears_hashtab_spectrograms_retrieve(buffer_name);
}

double ears_spectralbuf_get_original_audio_sr(t_object *ob, t_buffer_obj *buf)
{
    t_ears_spectralbuf_metadata *data = ears_spectralbuf_metadata_get(ob, buf);
    return data ? data->original_audio_signal_sr : ears_buffer_get_sr(ob, buf);
}


double ears_spectralbuf_get_binsize(t_object *ob, t_buffer_obj *buf)
{
    t_ears_spectralbuf_metadata *data = ears_spectralbuf_metadata_get(ob, buf);
    return data ? data->binsize : 0;
}


double ears_spectralbuf_get_binoffset(t_object *ob, t_buffer_obj *buf)
{
    t_ears_spectralbuf_metadata *data = ears_spectralbuf_metadata_get(ob, buf);
    return data ? data->binoffset : 0;
}


t_symbol *ears_spectralbuf_get_spectype(t_object *ob, t_buffer_obj *buf)
{
    t_ears_spectralbuf_metadata *data = ears_spectralbuf_metadata_get(ob, buf);
    return data ? data->type : _llllobj_sym_none;
}

e_ears_frequnit ears_spectralbuf_get_binunit(t_object *ob, t_buffer_obj *buf)
{
    t_ears_spectralbuf_metadata *data = ears_spectralbuf_metadata_get(ob, buf);
    return data ? data->binunit : EARS_FREQUNIT_UNKNOWN;
}

t_llll* ears_spectralbuf_get_bins(t_object *ob, t_buffer_obj *buf)
{
    t_ears_spectralbuf_metadata *data = ears_spectralbuf_metadata_get(ob, buf);
    if (data)
        return data->bins;
    else
        return NULL;
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


// ONE NEEDS TO MAKE SURE that this function is called when the samples of buf ARE NOT LOCKED!!!!
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
    
    // is spectral?
    if (ears_buffer_is_spectral(ob, orig)) {
        ears_spectralbuf_metadata_set(ob, dest, ears_spectralbuf_metadata_get(ob, orig));
    } else {
        if (ears_spectralbuf_metadata_get(ob, dest))
            ears_spectralbuf_metadata_remove(ob, dest);
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




double ears_interp_circular_bandlimited(float *in, long num_in_frames, double index, double window_width)
{
    long i, j;
    double r_w, r_a, r_snc;
    double r_g = 1;
    double r_y = 0;
    for (i = -window_width/2; i <= window_width/2; i++) { // For 1 window width
        j = (long)(index + i);          // Calc input sample index
        //rem calculate von Hann Window. Scale and calculate Sinc
        r_w     = 0.5 - 0.5 * cos(TWOPI*(0.5 + (j - index)/window_width));
        r_a     = TWOPI*(j - index)*0.5;
        r_snc   = (r_a != 0 ? sin(r_a)/r_a : 1); ///<< sin(r_a) is slow. Do we have other options?
        j = positive_mod(j, num_in_frames);
        r_y   = r_y + r_g * r_w * r_snc * in[j];
    }
    return r_y;                  // Return new filtered sample
}

double ears_interp_bandlimited(float *in, long num_in_frames, double index, double window_width, long step)
{
    long i, j;
    double r_w, r_a, r_snc;
    double r_g = 1;
    double r_y = 0;
    long index_floor = floor(index);
    double diff = index-index_floor;
    for (i = -window_width/2; i <= window_width/2; i++) { // For 1 window width
//        j = (long)floor(index + i);          // Calc input sample index
        j = index_floor + i;
        if (j >= 0 && j < num_in_frames) {
            //rem calculate von Hann Window. Scale and calculate Sinc
//            r_w     = 0.5 - 0.5 * cos(TWOPI*(0.5 + (j - index)/window_width));
//            r_a     = TWOPI*(j - index)*0.5;
            r_w     = 0.5 - 0.5 * cos(TWOPI*(0.5 + (i - diff)/window_width));
            r_a     = TWOPI*(i - diff)*0.5;
            r_snc   = (r_a != 0 ? sin(r_a)/r_a : 1); ///<< sin(r_a) is slow. Do we have other options?
            r_y   = r_y + r_g * r_w * r_snc * in[j*step];
        }
    }
    return r_y;                  // Return new filtered sample
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
            for (i = -window_width/2; i <= window_width/2; i++) { // For 1 window width // Was: i < window_width/2.
                j = (long)(x + i);          // Calc input sample index
                //rem calculate von Hann Window. Scale and calculate Sinc
                r_w     = 0.5 - 0.5 * cos(TWOPI*(0.5 + (j - x)/window_width));
                r_a     = TWOPI*(j - x)*fmax/sr;
                r_snc   = (r_a != 0 ? sin(r_a)/r_a : 1); ///<< sin(r_a) is slow. Do we have other options?
                if (j >= 0 && j < num_in_frames)
                    r_y   = r_y + r_g * r_w * r_snc * in[num_channels * j + ch];
            }
            (*out)[num_channels * s + ch] = r_y;                  // Return new filtered sample
        }
    }
    return num_out_frames * num_channels;
}

// Could be improved
// beware: <in> should be allocated with num_in_frames * num_channels floats, and <out> might be allocated with num_out_frames * num_channels float
long ears_resample_env(float *in, long num_in_frames, float **out, long num_out_frames, t_ears_envelope_iterator *factor_env, double fmax, double sr, double window_width, long num_channels)
{
    double maxfactor = ears_envelope_iterator_get_max_y(factor_env);
    if (num_out_frames <= 0)
        num_out_frames = (long)ceil(num_in_frames * maxfactor);
    if (out && !*out)
        *out = (float *)bach_newptr(num_out_frames * num_channels * sizeof(float));
    for (long ch = 0; ch < num_channels; ch++) {
        ears_envelope_iterator_reset(factor_env);
        
        long pivot_sample = 0;
        double factor = ears_envelope_iterator_walk_interp(factor_env, pivot_sample, num_in_frames);
        double x = 0;
        for (long s = 0; s < num_out_frames; s++) {
            long i, j;
            double r_w, r_a, r_snc;
            double r_g = 2 * fmax / sr; // Calc gain correction factor
            double r_y = 0;
            for (i = -window_width/2; i < window_width/2; i++) { // For 1 window width
//                x = s / factor;
                j = (long)(x + i);          // Calc input sample index
                //rem calculate von Hann Window. Scale and calculate Sinc
                r_w     = 0.5 - 0.5 * cos(TWOPI*(0.5 + (j - x)/window_width));
                r_a     = TWOPI*(j - x)*fmax/sr;
                r_snc   = (r_a != 0 ? r_snc = sin(r_a)/r_a : 1); ///<< sin(r_a) is slow. Do we have other options?
                if (j >= 0 && j < num_in_frames)
                    r_y   = r_y + r_g * r_w * r_snc * in[num_channels * j + ch];
            }
            (*out)[num_channels * s + ch] = r_y;                  // Return new filtered sample
            x += 1 / factor;
            if ((long)x > pivot_sample) {
                factor = ears_envelope_iterator_walk_interp(factor_env, (long)x, num_in_frames);
                pivot_sample = (long)x;
            }
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
        ears_buffer_set_size_samps(ob, buf, new_framecount);
        sample = buffer_locksamples(buf);
        
        new_framecount = buffer_getframecount(buf);

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


// resampling without converting sr
t_ears_err ears_buffer_resample_envelope(t_object *ob, t_buffer_obj *buf, t_llll *resampling_factor, long window_width, e_slope_mapping slopemapping)
{
    t_ears_err err = EARS_ERR_NONE;
    t_ears_envelope_iterator eei = ears_envelope_iterator_create(resampling_factor, 1., false, slopemapping);
    double curr_sr = buffer_getsamplerate(buf);
    double maxfactor = ears_envelope_iterator_get_max_y(&eei);
    double sr = curr_sr * maxfactor;
    
    double fmax = sr / 2.;
    
    float *sample = buffer_locksamples(buf);
    
    if (!sample) {
        err = EARS_ERR_CANT_READ;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
    } else {
        t_atom_long    channelcount = buffer_getchannelcount(buf);        // number of floats in a frame
        t_atom_long    framecount   = buffer_getframecount(buf);            // number of floats long the buffer is for a single channel
        
        long new_framecount = (long)ceil(framecount * maxfactor);
        float *temp = (float *)bach_newptr(channelcount * framecount * sizeof(float));
        sysmem_copyptr(sample, temp, channelcount * framecount * sizeof(float));
        
        buffer_unlocksamples(buf);
        ears_buffer_set_size_samps(ob, buf, new_framecount);
        sample = buffer_locksamples(buf);
        
        new_framecount = buffer_getframecount(buf);
        
        if (!sample) {
            err = EARS_ERR_CANT_WRITE;
            object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
        } else {
            ears_envelope_iterator_reset(&eei);
            ears_resample_env(temp, framecount, &sample, new_framecount, &eei, fmax, sr, window_width, channelcount);
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
        ears_buffer_set_size_samps(ob, buf, new_framecount);
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


// keeps the buffer content while changing the number of channels
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

            memset(sample, 0, framecount * new_numchannels * sizeof(float));
            
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
                    if (new_numchannels > channelcount) { // upmixing
                        for (long c = 0; c < new_numchannels; c++) {
                            long c_mod = c % channelcount;
                            for (long i = 0; i < framecount; i++)
                                sample[i*new_numchannels + c] = temp[i*channelcount + c_mod];
                        }
                    } else { // downmixing
                        for (long c = 0; c < channelcount; c++) {
                            long c_mod = c % new_numchannels;
                            for (long i = 0; i < framecount; i++)
                                sample[i*new_numchannels + c_mod] += temp[i*channelcount + c];
                        }
                    }
                    break;
                    
                case EARS_CHANNELCONVERTMODE_PALINDROME:
                    if (new_numchannels > channelcount) { // upmixing
                        for (long c = 0; c < new_numchannels; c++) {
                            long c_mod = c % (2 * channelcount);
                            if (c_mod >= channelcount)
                                c_mod = 2 * channelcount - c_mod - 1;
                            for (long i = 0; i < framecount; i++)
                                sample[i*new_numchannels + c] = temp[i*channelcount + c_mod];
                        }
                    } else {
                        for (long c = 0; c < channelcount; c++) {
                            long c_mod = c % (2 * new_numchannels);
                            if (c_mod >= new_numchannels)
                                c_mod = 2 * new_numchannels - c_mod - 1;
                            for (long i = 0; i < framecount; i++)
                                sample[i*new_numchannels + c_mod] += temp[i*channelcount + c];
                        }
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

t_ears_err ears_buffer_convert_numchannels(t_object *ob, t_buffer_obj *buf, long numchannels, e_ears_channel_convert_modes channelmode_upmix, e_ears_channel_convert_modes channelmode_downmix)
{
    t_ears_err err = EARS_ERR_NONE;
    long num_orig_channels = buffer_getchannelcount(buf);

    if (numchannels < num_orig_channels) {
        switch (channelmode_downmix) {
            case EARS_CHANNELCONVERTMODE_PAN:
                ears_buffer_pan1d(ob, buf, buf, numchannels, 0.5, EARS_PAN_MODE_LINEAR, EARS_PAN_LAW_COSINE, 1., true);
                break;
                
            default:
                ears_buffer_set_numchannels_preserve(ob, buf, numchannels, channelmode_downmix);
                break;
        }
    } else if (numchannels > num_orig_channels) {

        switch (channelmode_upmix) {
            case EARS_CHANNELCONVERTMODE_PAN:
                ears_buffer_pan1d(ob, buf, buf, numchannels, 0.5, EARS_PAN_MODE_LINEAR, EARS_PAN_LAW_COSINE, 1., false);
                break;
                
            default:
                ears_buffer_set_numchannels_preserve(ob, buf, numchannels, channelmode_upmix);
                break;
        }
    }
    
    
    return err;
}


// like ears_buffer_set_size_samps() but keeps the samples
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
        ears_buffer_set_size_samps(ob, buf, sizeinsamps);
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
t_ears_err ears_buffer_convert_format(t_object *ob, t_buffer_obj *orig, t_buffer_obj *dest, e_ears_channel_convert_modes channelmode_upmix, e_ears_channel_convert_modes channelmode_downmix)
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
        ears_buffer_convert_numchannels(ob, dest, orig_channelcount, channelmode_upmix, channelmode_downmix);
    
//    dest_channelcount = buffer_getchannelcount(dest);
//    dest_sr = buffer_getsamplerate(dest);
    
    return EARS_ERR_NONE;
}


// N.B. to sample is EXCLUDED!!!! The taken interval is [start_sample end_sample[
// end_sample is then the FIRST sample of the region after the crop
t_ears_err ears_buffer_crop(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, long start_sample, long end_sample)
{
    if (source == dest)
        return ears_buffer_crop_inplace(ob, source, start_sample, end_sample);
    
    t_ears_err err = EARS_ERR_NONE;
    float *orig_sample = buffer_locksamples(source);
    
    if (!orig_sample) {
        if (!buffer_getframecount(source)) {
            object_warn((t_object *)ob, "Source buffer is empty!");
            ears_buffer_set_size_samps(ob, dest, 0);
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
            ears_buffer_set_size_samps(ob, dest, 0);
            
        } else if (end_sample <= start_sample) {
            object_warn((t_object *)ob, "Ending crop point precedes or coincides with starting crop point: empty buffer output.");
            ears_buffer_set_size_samps(ob, dest, 0);

        } else {
            
            long new_dest_frames = end_sample - start_sample;
            ears_buffer_set_size_and_numchannels(ob, dest, new_dest_frames, channelcount);
            ears_buffer_copy_format(ob, source, dest);

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



// N.B. end_sample is EXCLUDED!!!! The taken interval is [start_sample end_sample[
// end_sample is then the FIRST sample of the region after the crop
t_ears_err ears_buffer_crop_inplace(t_object *ob, t_buffer_obj *buf, long start_sample, long end_sample)
{
    t_ears_err err = EARS_ERR_NONE;
    float *sample = buffer_locksamples(buf);
    
    if (!sample) {
        if (!buffer_getframecount(buf)) {
            object_warn((t_object *)ob, "Source buffer is empty!");
            ears_buffer_set_size_samps(ob, buf, 0);
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
            ears_buffer_set_size_samps(ob, buf, 0);
            
        } else if (end_sample <= start_sample) {
            object_warn((t_object *)ob, "Ending crop point precedes or coincides with starting crop point: empty buffer output.");
            ears_buffer_set_size_samps(ob, buf, 0);
            
        } else {
            
            long new_dest_frames = end_sample - start_sample;
            float *temp = (float *)bach_newptr(channelcount * new_dest_frames * sizeof(float));
            sysmem_copyptr(sample + start_sample * channelcount, temp, channelcount * new_dest_frames * sizeof(float));
            
            buffer_unlocksamples(buf);
            ears_buffer_set_size_samps(ob, buf, new_dest_frames);
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
        ears_buffer_set_size_samps(ob, dest, framecount);

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
            ears_buffer_set_size_samps(ob, dest_left, 0);
            ears_buffer_set_size_samps(ob, dest_right, 0);
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
            ears_buffer_set_size_samps(ob, dest_right, 0);

        } else if (split_sample == 0) {
            ears_buffer_clone(ob, source, dest_right);
            ears_buffer_set_size_samps(ob, dest_left, 0);

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
                    ears_buffer_set_size_samps(ob, dest[i], 0);
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
                    ears_buffer_set_size_samps(ob, this_dest, 0);
                    
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




void ears_buffer_preprocess_sr_single(t_object *ob, double target_sr, t_buffer_obj *factor, float **factor_sample, long resamplingfiltersize, bool *resampled, t_atom_long *new_factor_sample)
{
    // Do we need to resample factor?
    double this_sr = ears_buffer_get_sr(ob, factor);
    if (this_sr != target_sr) {
        // must resample
        long num_channels = ears_buffer_get_numchannels(ob, factor);
        double resampling_factor = (target_sr/this_sr);
        long num_in_frames = ears_buffer_get_size_samps(ob, factor);
        long num_out_frames = ceil(num_in_frames * resampling_factor);
        float *new_samples = (float *)bach_newptr(num_out_frames * num_channels * sizeof(float));
        ears_resample(*factor_sample, num_in_frames, &new_samples, num_out_frames, resampling_factor, this_sr/2., this_sr, resamplingfiltersize, num_channels);
        buffer_unlocksamples(factor);
        *factor_sample = new_samples;
        *resampled = true;
        *new_factor_sample = num_out_frames;
    }
}



t_ears_err ears_buffer_sumchannel(t_object *ob, t_buffer_obj *source, long source_channel, t_buffer_obj *dest, long dest_channel, double resampling_sr, long resamplingfiltersize)
{
    if (!source || !dest)
        return EARS_ERR_NO_BUFFER;
    
    t_ears_err err = EARS_ERR_NONE;
    bool resampled = false;
    double sr = ears_buffer_get_sr(ob, source);
    float *source_sample = buffer_locksamples(source);
    
    if (!source_sample) {
        err = EARS_ERR_CANT_READ;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
    } else {
        
        t_atom_long    source_channelcount = buffer_getchannelcount(source);
        t_atom_long    source_framecount   = buffer_getframecount(source);
        
        if (resampling_sr > 0 && sr != resampling_sr) {
            ears_buffer_preprocess_sr_single(ob, resampling_sr, source, &source_sample, resamplingfiltersize, &resampled, &source_framecount);
        }
        
        float *dest_sample = buffer_locksamples(dest);
        if (!dest_sample) {
            err = EARS_ERR_CANT_READ;
            object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
        } else {
            t_atom_long    dest_channelcount = buffer_getchannelcount(dest);
            t_atom_long    dest_framecount   = buffer_getframecount(dest);
            
            for (long i = 0; i < source_framecount && i < dest_framecount; i++) {
                dest_sample[i * dest_channelcount + dest_channel] += source_sample[i * source_channelcount + source_channel];
            }
            buffer_unlocksamples(dest);
        }
        
        if (resampled)
            bach_freeptr(source_sample);
        else
            buffer_unlocksamples(source);
    }
    
    return err;
}


t_ears_err ears_buffer_copychannel(t_object *ob, t_buffer_obj *source, long source_channel, t_buffer_obj *dest, long dest_channel, double resampling_sr, long resamplingfiltersize)
{
    if (!source || !dest)
        return EARS_ERR_NO_BUFFER;
    
    t_ears_err err = EARS_ERR_NONE;
    bool resampled = false;
    double sr = ears_buffer_get_sr(ob, source);
    float *source_sample = buffer_locksamples(source);
    
    if (!source_sample) {
        err = EARS_ERR_CANT_READ;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
    } else {
        
        t_atom_long    source_channelcount = buffer_getchannelcount(source);
        t_atom_long    source_framecount   = buffer_getframecount(source);

        if (resampling_sr > 0 && sr != resampling_sr) {
            ears_buffer_preprocess_sr_single(ob, resampling_sr, source, &source_sample, resamplingfiltersize, &resampled, &source_framecount);
        }
        
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
        
        if (resampled)
            bach_freeptr(source_sample);
        else
            buffer_unlocksamples(source);
    }
    
    return err;
}


t_ears_err ears_buffer_setempty(t_object *ob, t_buffer_obj *buf, long num_channels)
{
    ears_buffer_set_size_and_numchannels(ob, buf, 0, num_channels);
    ears_buffer_set_sr(ob, buf, sys_getsr());
    return EARS_ERR_NONE;
}


t_ears_err ears_buffer_extractchannels(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, long num_channels, long *channels)
{
    t_ears_err this_err, err = EARS_ERR_NONE;
    long frames = ears_buffer_get_size_samps(ob, source);
    ears_buffer_copy_format(ob, source, dest);
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


void ears_buffer_preprocess_sr_policies_dry(t_object *ob, t_buffer_obj **source, long num_sources, e_ears_resamplingpolicy resamplingpolicy, long resamplingfiltersize, double *sr, bool *resampled)
{
    if (!ears_buffers_have_the_same_sr(ob, num_sources, source) && resamplingpolicy != EARS_RESAMPLINGPOLICY_DONT) {
        *resampled = true;
        *sr = ears_buffers_get_collective_sr(ob, num_sources, source, resamplingpolicy);
    }
}


t_ears_err ears_buffer_pack(t_object *ob, long num_sources, t_buffer_obj **source, t_buffer_obj *dest,
                            e_ears_resamplingpolicy resamplingpolicy, long resamplingfiltersize)
{
    t_ears_err err = EARS_ERR_NONE;
    
    if (!dest)
        return EARS_ERR_NO_BUFFER;
    
    if (num_sources == 0) {
        // empty buffer, 1 channel
        ears_buffer_setempty(ob, dest, 1);
        return EARS_ERR_NONE;
    }
    
    bool must_resample = false;
    double sr = ears_buffer_get_sr(ob, source[0]);

    ears_buffer_preprocess_sr_policies_dry(ob, source, num_sources, resamplingpolicy, resamplingfiltersize, &sr, &must_resample);

    ears_buffer_set_sr(ob, dest, sr);
    
    long num_channels = 0;
    long max_num_frames = 0;
    for (long i = 0; i < num_sources; i++) {
        num_channels += ears_buffer_get_numchannels(ob, source[i]);
        if (must_resample && sr > 0)
            max_num_frames = MAX(max_num_frames, ceil(ears_buffer_get_size_samps(ob, source[i]) * sr / ears_buffer_get_sr(ob, source[i])));
        else
            max_num_frames = MAX(max_num_frames, ears_buffer_get_size_samps(ob, source[i]));
    }

    ears_buffer_set_size_and_numchannels(ob, dest, max_num_frames, num_channels);

    long channel_cur = 0;
    for (long i = 0; i < num_sources; i++) {
        long this_num_channels = ears_buffer_get_numchannels(ob, source[i]);
        for (long c = 0; c < this_num_channels; c++) {
            t_ears_err this_err = ears_buffer_copychannel(ob, source[i], c, dest, channel_cur, must_resample ? sr : 0, resamplingfiltersize);
            if (err == EARS_ERR_NONE)
                err = this_err;
            channel_cur++;
        }
    }
    
    return err;
}


t_ears_err ears_buffer_pack_from_llll(t_object *ob, t_llll *sources_ll, t_buffer_obj *dest,
                                      e_ears_resamplingpolicy resamplingpolicy, long resamplingfiltersize)
{
    long num_sources = sources_ll->l_size;
    
    if (num_sources == 0)
        return ears_buffer_pack(ob, 0, NULL, dest, resamplingpolicy, resamplingfiltersize);
    else {
        long i = 0;
        t_buffer_obj **sources = (t_buffer_obj **)bach_newptr(num_sources * sizeof(t_buffer_obj *));
        for (t_llllelem *el = sources_ll->l_head; el; el = el->l_next, i++)
            sources[i] = (t_buffer_obj *)hatom_getobj(&el->l_hatom);
        t_max_err err = ears_buffer_pack(ob, num_sources, sources, dest, resamplingpolicy, resamplingfiltersize);
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
    
    if (!orig_sample) {
        err = EARS_ERR_CANT_READ;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
    } else {
        double factor = use_decibels ? ears_db_to_linear(gain_factor) : gain_factor;
        
        t_atom_long    channelcount = buffer_getchannelcount(source);        // number of floats in a frame
        t_atom_long    framecount   = buffer_getframecount(source);            // number of floats long the buffer is for a single channel
        
        if (source != dest) {
            ears_buffer_copy_format(ob, source, dest);
            ears_buffer_set_size_samps(ob, dest, framecount);
        }
        
        float *dest_sample = (source == dest ? orig_sample : buffer_locksamples(dest));
        
        if (!dest_sample) {
            err = EARS_ERR_CANT_WRITE;
            object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
        } else {
            
            for (long i = 0; i < channelcount * framecount; i++) {
                dest_sample[i] = orig_sample[i] * factor;
            }
            
            buffer_setdirty(dest);
            if (source != dest)
                buffer_unlocksamples(dest);
        }
        
        buffer_unlocksamples(source);
    }
    
    return err;
}



// also supports inplace operations
t_ears_err ears_buffer_clip(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, double clip_threshold, char use_decibels)
{
    if (!source || !dest)
        return EARS_ERR_NO_BUFFER;
    
    t_ears_err err = EARS_ERR_NONE;
    float *orig_sample = buffer_locksamples(source);
    
    if (!orig_sample) {
        err = EARS_ERR_CANT_READ;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
    } else {
        double threshold = use_decibels ? ears_db_to_linear(clip_threshold) : clip_threshold;
        
        t_atom_long    channelcount = buffer_getchannelcount(source);        // number of floats in a frame
        t_atom_long    framecount   = buffer_getframecount(source);            // number of floats long the buffer is for a single channel
        
        if (source != dest) {
            ears_buffer_copy_format(ob, source, dest);
            ears_buffer_set_size_samps(ob, dest, framecount);
        }

        float *dest_sample = (source == dest ? orig_sample : buffer_locksamples(dest));
        
        if (!dest_sample) {
            err = EARS_ERR_CANT_WRITE;
            object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
        } else {
            
            for (long i = 0; i < channelcount * framecount; i++) {
                if (fabs(orig_sample[i]) < threshold)
                    dest_sample[i] = orig_sample[i];
                else
                    dest_sample[i] = (orig_sample[i] >= 0 ? 1 : -1) * threshold;
            }
            
            buffer_setdirty(dest);
            if (source != dest)
                buffer_unlocksamples(dest);
        }
        
        buffer_unlocksamples(source);
    }
    
    return err;
}

// like Max's [overdrive] object
double ears_overdrive(double input, double drive)
{
    int signx = (input >= 0. ? 1 : -1);
    return CLAMP(1 - exp(drive * log(1 - input * signx)), 0, 1) * signx;
}

// also supports inplace operations
t_ears_err ears_buffer_overdrive(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, double drive)
{
    if (!source || !dest)
        return EARS_ERR_NO_BUFFER;
    
    t_ears_err err = EARS_ERR_NONE;
    float *orig_sample = buffer_locksamples(source);
    
    if (!orig_sample) {
        err = EARS_ERR_CANT_READ;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
    } else {
        t_atom_long    channelcount = buffer_getchannelcount(source);        // number of floats in a frame
        t_atom_long    framecount   = buffer_getframecount(source);            // number of floats long the buffer is for a single channel
        
        if (source != dest) {
            ears_buffer_copy_format(ob, source, dest);
            ears_buffer_set_size_samps(ob, dest, framecount);
        }
        
        float *dest_sample = (source == dest ? orig_sample : buffer_locksamples(dest));
        
        if (!dest_sample) {
            err = EARS_ERR_CANT_WRITE;
            object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
        } else {
            
            for (long i = 0; i < channelcount * framecount; i++) {
                dest_sample[i] = ears_overdrive(orig_sample[i], drive);
            }
            
            buffer_setdirty(dest);
            if (source != dest)
                buffer_unlocksamples(dest);
        }
        
        buffer_unlocksamples(source);
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



t_ears_err ears_buffer_pan1d_envelope(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, long num_out_channels, t_llll *env, e_ears_pan_modes pan_mode, e_ears_pan_laws pan_law, double multichannel_pan_aperture, char compensate_gain_for_multichannel_to_avoid_clipping, e_slope_mapping slopemapping)
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
            
            t_ears_envelope_iterator eei = ears_envelope_iterator_create(env, 0., false, slopemapping);
            
            if (channelcount == 1) { // panning a mono source
                for (long i = 0; i < framecount; i++) {
                    double pan = ears_envelope_iterator_walk_interp(&eei, i, framecount);
                    for (long c = 0; c < num_out_channels; c++)
                        dest_sample[i*num_out_channels + c] = orig_sample_wk[i] * ears_get_pan_factor(c, num_out_channels, pan, pan_mode, pan_law);
                }
            } else {
                if (multichannel_pan_aperture == 0) {
                    
                    for (long i = 0; i < framecount; i++) {
                        // mixdown to mono before panning
                        double s = 0;
                        for (long c = 0; c < channelcount; c++)
                            s += orig_sample_wk[i*channelcount + c];
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
                                dest_sample[i * num_out_channels + c] += orig_sample_wk[i * channelcount + d] * ears_get_pan_factor(c, num_out_channels, adjusted_pan[d], pan_mode, pan_law) / (compensate_gain_for_multichannel_to_avoid_clipping ? channelcount : 1);
                    }
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



t_ears_err ears_buffer_pan1d_buffer(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, long num_out_channels, t_buffer_obj *pan, e_ears_pan_modes pan_mode, e_ears_pan_laws pan_law, double multichannel_pan_aperture, char compensate_gain_for_multichannel_to_avoid_clipping)
{
    if (!source || !pan || !dest)
        return EARS_ERR_NO_BUFFER;
    
    t_ears_err err = EARS_ERR_NONE;
    float *source_sample = buffer_locksamples(source);
    float *source_sample_wk = NULL;

    if (!source_sample) {
        err = EARS_ERR_CANT_READ;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
    } else {
        t_atom_long    source_channelcount = buffer_getchannelcount(source);
        t_atom_long    source_framecount   = buffer_getframecount(source);
        
        if (source == dest) { // inplace operation!
            source_sample_wk = (float *)bach_newptr(source_channelcount * source_framecount * sizeof(float));
            sysmem_copyptr(source_sample, source_sample_wk, source_channelcount * source_framecount * sizeof(float));
            buffer_unlocksamples(source);
        } else {
            source_sample_wk = source_sample;
            ears_buffer_copy_format(ob, source, dest);
        }
        
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
                                dest_sample[i*num_out_channels + c] = source_sample_wk[i] * ears_get_pan_factor(c, num_out_channels, pan, pan_mode, pan_law);
                        }
                    } else {
                        if (multichannel_pan_aperture == 0) {
                            
                            for (long i = 0; i < max_sample; i++) {
                                // mixdown to mono before panning
                                double s = 0;
                                for (long c = 0; c < source_channelcount; c++)
                                    s += source_sample_wk[i*source_channelcount + c];
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
                                        dest_sample[i * num_out_channels + c] += source_sample_wk[i * source_channelcount + d] * ears_get_pan_factor(c, num_out_channels, adjusted_pan[d], pan_mode, pan_law) / (compensate_gain_for_multichannel_to_avoid_clipping ? source_channelcount : 1);
                            }
                        }
                    }
                }
                
                buffer_unlocksamples(pan);
            }
            buffer_setdirty(dest);
            buffer_unlocksamples(dest);
        }
        
        if (source == dest) // inplace operation!
            bach_freeptr(source_sample_wk);
        else
            buffer_unlocksamples(source);
    }
    
    return err;
}





// possibly resamples towards #buf
t_ears_err ears_buffer_multiply_inplace(t_object *ob, t_buffer_obj *buf, t_buffer_obj *factor, long resamplingfiltersize)
{
    if (!buf || !factor)
        return EARS_ERR_NO_BUFFER;
    
    t_ears_err err = EARS_ERR_NONE;
    double sr = ears_buffer_get_sr(ob, buf);
    float *buf_sample = buffer_locksamples(buf);
    
    if (!buf_sample) {
        err = EARS_ERR_CANT_READ;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
    } else {
        t_atom_long	channelcount = buffer_getchannelcount(buf);
        t_atom_long	framecount   = buffer_getframecount(buf);
        
        bool resampled = false;
        float *factor_sample = buffer_locksamples(factor);

        if (!factor_sample) {
            err = EARS_ERR_CANT_WRITE;
            object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
        } else {

            t_atom_long    factor_channelcount = buffer_getchannelcount(factor);
            t_atom_long    factor_framecount   = buffer_getframecount(factor);

            ears_buffer_preprocess_sr_single(ob, sr, factor, &factor_sample, resamplingfiltersize, &resampled, &factor_framecount);
            
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
            
            if (resampled)
                bach_freeptr(factor_sample);
            else
                buffer_unlocksamples(factor);
        }
        buffer_setdirty(buf);
        buffer_unlocksamples(buf);
    }
    
    return err;
}


t_ears_err ears_buffer_sum_inplace(t_object *ob, t_buffer_obj *buf, t_buffer_obj *addend, long resamplingfiltersize)
{
    if (!buf || !addend)
        return EARS_ERR_NO_BUFFER;
    
    t_ears_err err = EARS_ERR_NONE;
    bool resampled = false;
    double sr = ears_buffer_get_sr(ob, buf);
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

            t_atom_long    addend_channelcount = buffer_getchannelcount(buf);
            t_atom_long    addend_framecount   = buffer_getframecount(buf);

            ears_buffer_preprocess_sr_single(ob, sr, addend, &addend_sample, resamplingfiltersize, &resampled, &addend_framecount);
            
            for (long i = 0; i < framecount; i++) {
                if (i < addend_framecount) {
                    for (long c = 0; c < channelcount; c++) {
                        buf_sample[i*channelcount + c] += addend_sample[i*addend_channelcount + MIN(c, addend_channelcount - 1)];
                    }
                }
            }
            
            if (resampled)
                bach_freeptr(addend_sample);
            else
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



t_ears_err ears_buffer_from_clicks(t_object *ob, t_buffer_obj *buf, t_llll *onsets_samps, t_buffer_obj *impulse)
{
    if (!buf)
        return EARS_ERR_NO_BUFFER;

    if (!onsets_samps)
        return EARS_ERR_GENERIC;
    
    long num_channels = onsets_samps->l_size;
    if (num_channels < 1)
        return EARS_ERR_GENERIC;
    
    t_buffer_obj **chans = (t_buffer_obj **)bach_newptr(MAX(num_channels, 1) * sizeof(t_buffer_obj *));
    
    if (num_channels == 1) {
        chans[0] = buf;
    } else if (num_channels > 1) {
        for (long i = 0; i < num_channels; i++) {
            chans[i] = (t_buffer_obj *)object_new_typed(CLASS_BOX, gensym("buffer~"), 0, NULL);
            ears_buffer_copy_format(ob, impulse, chans[i]);
        }
    }
    for (t_llllelem *onset_el = onsets_samps->l_head; onset_el; onset_el = onset_el->l_next) {
        t_llll *these_onsets = hatom_getllll(&onset_el->l_hatom);
        if (these_onsets) {
            long num_onsets = these_onsets->l_size;
            t_llll *gains = llll_get();
            long *offset_samps = (long *)bach_newptr(MAX(num_onsets, 1) * sizeof(long));
            t_buffer_obj **sources = (t_buffer_obj **)bach_newptr(MAX(num_onsets, 1) * sizeof(t_buffer_obj *));
            for (long i = 0; i < num_onsets; i++) {
                llll_appenddouble(gains, 1.);
                sources[i] = impulse;
            }
            long j = 0;
            for (t_llllelem *el = these_onsets->l_head; el; el = el->l_next) {
                offset_samps[j] = hatom_getdouble(&el->l_hatom);
                j++;
            }
            ears_buffer_mix(ob, sources, num_onsets, buf, gains, offset_samps, EARS_NORMALIZE_DONT, k_SLOPE_MAPPING_BACH, EARS_RESAMPLINGPOLICY_DONT, 10);
            
            bach_freeptr(offset_samps);
            bach_freeptr(sources);
        }
    }
    
    if (num_channels > 1) {
        ears_buffer_pack(ob, num_channels, chans, buf, EARS_RESAMPLINGPOLICY_DONT, 10);
        for (long i = 0; i < num_channels; i++)
            object_free(chans[i]);
    }
    
    bach_freeptr(chans);

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



t_ears_envelope_iterator ears_envelope_iterator_create(t_llll *envelope, double default_val, char use_decibels, e_slope_mapping slopemapping)
{
    t_ears_envelope_iterator eei;
    eei.slopemapping = slopemapping;
    eei.env = envelope;
    eei.default_val = default_val;
    eei.left_el = NULL;
    eei.right_el = envelope ? envelope->l_head : NULL;
    if (eei.right_el)
        eei.left_pts = eei.right_pts = elem_to_pts(eei.right_el);
    eei.use_decibels = use_decibels;
    return eei;
}

t_ears_envelope_iterator ears_envelope_iterator_create_from_llllelem(t_llllelem *envelope, double default_val, char use_decibels, e_slope_mapping slopemapping)
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
    eei.slopemapping = slopemapping;
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

double ears_envelope_iterator_get_max_y(t_ears_envelope_iterator *eei)
{
    double max_y = DBL_MIN;
    for (t_llllelem *el = eei->env->l_head; el; el = el->l_next) {
        t_llll *ll = hatom_getllll(&el->l_hatom);
        if (ll && ll->l_head) {
            double this_y = hatom_getdouble(&ll->l_head->l_next->l_hatom);
            if (this_y > max_y)
                max_y = this_y;
        }
    }
    return max_y;
}

double ears_envelope_iterator_get_min_y(t_ears_envelope_iterator *eei)
{
    double min_y = DBL_MAX;
    for (t_llllelem *el = eei->env->l_head; el; el = el->l_next) {
        t_llll *ll = hatom_getllll(&el->l_hatom);
        if (ll && ll->l_head) {
            double this_y = hatom_getdouble(&ll->l_head->l_next->l_hatom);
            if (this_y < min_y)
                min_y = this_y;
        }
    }
    return min_y;
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
        amp = rescale_with_slope(pos_x, eei->left_pts.x, eei->right_pts.x, eei->left_pts.y, eei->right_pts.y, eei->right_pts.slope, eei->slopemapping);
    else if (eei->left_el)
        amp = eei->left_pts.y;
    else if (eei->right_el)
        amp = eei->right_pts.y;
    
    return eei->use_decibels ? ears_db_to_linear(amp) : amp;
}


t_ears_err ears_buffer_gain_envelope(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, t_llll *env, char use_decibels, e_slope_mapping slopemapping)
{
    if (!source || !dest)
        return EARS_ERR_NO_BUFFER;
    
    t_ears_err err = EARS_ERR_NONE;
    float *orig_sample = buffer_locksamples(source);
    
    if (!orig_sample) {
        err = EARS_ERR_CANT_READ;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
    } else {
        
        t_atom_long	channelcount = buffer_getchannelcount(source);		// number of floats in a frame
        t_atom_long	framecount   = buffer_getframecount(source);			// number of floats long the buffer is for a single channel
        
        if (source != dest) {
            ears_buffer_copy_format(ob, source, dest);
            ears_buffer_set_size_samps(ob, dest, framecount);
        }
        
        float *dest_sample = (source == dest ? orig_sample : buffer_locksamples(dest));
        
        if (!dest_sample) {
            err = EARS_ERR_CANT_WRITE;
            object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
        } else {
            
            t_ears_envelope_iterator eei = ears_envelope_iterator_create(env, 0., use_decibels, slopemapping);
            for (long i = 0; i < framecount; i++) {
                double factor = ears_envelope_iterator_walk_interp(&eei, i, framecount);
                
                for (long c = 0; c < channelcount; c++) {
                    long idx = channelcount * i + c;
                    dest_sample[idx] = orig_sample[idx] * factor;
                }
            }
            
            buffer_setdirty(dest);
            if (source != dest)
                buffer_unlocksamples(dest);
        }
        
        buffer_unlocksamples(source);

    }
    
    return err;
}


t_ears_err ears_buffer_clip_envelope(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, t_llll *thresh, char use_decibels, e_slope_mapping slopemapping)
{
    if (!source || !dest)
        return EARS_ERR_NO_BUFFER;
    
    t_ears_err err = EARS_ERR_NONE;
    float *orig_sample = buffer_locksamples(source);
    
    if (!orig_sample) {
        err = EARS_ERR_CANT_READ;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
    } else {
        
        t_atom_long    channelcount = buffer_getchannelcount(source);        // number of floats in a frame
        t_atom_long    framecount   = buffer_getframecount(source);            // number of floats long the buffer is for a single channel
        
        if (source != dest) {
            ears_buffer_copy_format(ob, source, dest);
            ears_buffer_set_size_samps(ob, dest, framecount);
        }
        
        float *dest_sample = (source == dest ? orig_sample : buffer_locksamples(dest));
        
        if (!dest_sample) {
            err = EARS_ERR_CANT_WRITE;
            object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
        } else {
            
            t_ears_envelope_iterator eei = ears_envelope_iterator_create(thresh, 0., use_decibels, slopemapping);
            for (long i = 0; i < framecount; i++) {
                double threshold = ears_envelope_iterator_walk_interp(&eei, i, framecount);
                
                for (long c = 0; c < channelcount; c++) {
                    long idx = channelcount * i + c;
                    if (fabs(orig_sample[idx]) <= threshold) {
                        dest_sample[idx] = orig_sample[idx];
                    } else {
                        dest_sample[idx] = (orig_sample[idx] >= 0 ? 1 : -1) * threshold;
                    }
                }
            }
            
            buffer_setdirty(dest);
            if (source != dest)
                buffer_unlocksamples(dest);
        }
        buffer_unlocksamples(source);
    }
    
    return err;
}


t_ears_err ears_buffer_overdrive_envelope(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, t_llll *drive, e_slope_mapping slopemapping)
{
    if (!source || !dest)
        return EARS_ERR_NO_BUFFER;
    
    t_ears_err err = EARS_ERR_NONE;
    float *orig_sample = buffer_locksamples(source);
    
    if (!orig_sample) {
        err = EARS_ERR_CANT_READ;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
    } else {
        
        t_atom_long    channelcount = buffer_getchannelcount(source);        // number of floats in a frame
        t_atom_long    framecount   = buffer_getframecount(source);            // number of floats long the buffer is for a single channel
        
        
        if (source != dest) {
            ears_buffer_copy_format(ob, source, dest);
            ears_buffer_set_size_samps(ob, dest, framecount);
        }
        
        float *dest_sample = (source == dest ? orig_sample : buffer_locksamples(dest));
        
        if (!dest_sample) {
            err = EARS_ERR_CANT_WRITE;
            object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
        } else {
            
            t_ears_envelope_iterator eei = ears_envelope_iterator_create(drive, 0., false, slopemapping);
            for (long i = 0; i < framecount; i++) {
                double this_drive = ears_envelope_iterator_walk_interp(&eei, i, framecount);
                
                for (long c = 0; c < channelcount; c++) {
                    long idx = channelcount * i + c;
                    dest_sample[idx] = ears_overdrive(orig_sample[idx], this_drive);
                }
            }
            
            buffer_setdirty(dest);
            if (source != dest)
                buffer_unlocksamples(dest);
        }
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
float ears_get_fade_factor(char in_or_out, e_ears_fade_types fade, double position, double curve, e_slope_mapping slopemapping)
{
    switch (fade) {
        case EARS_FADE_LINEAR:
            return position;
            break;
        case EARS_FADE_SINE:
            return sin(PIOVERTWO * position);
            break;
        case EARS_FADE_CURVE: // could find a faster formula, this is CPU consuming
            CLIP_ASSIGN(curve, -1., 1.);
            return rescale_with_slope(position, 0, 1., 0., 1., curve * in_or_out, slopemapping);
            break;
        case EARS_FADE_SCURVE:  // could find a faster formula, this is CPU consuming
            CLIP_ASSIGN(curve, -1., 1.);
            if (position == 0.5)
                return 0.5;
            else if (position < 0.5)
                return rescale_with_slope(position, 0., 0.5, 0., 0.5, curve * in_or_out, slopemapping);
            else
                return 1. - rescale_with_slope(1. - position, 0., 0.5, 0., 0.5, curve * in_or_out, slopemapping);
            break;
        default:
            return 1.;
            break;
    }
}

float ears_get_fade_in_factor(e_ears_fade_types fade, double position, double curve, e_slope_mapping slopemapping)
{
    return ears_get_fade_factor(1, fade, position, curve, slopemapping);
}

float ears_get_fade_out_factor(e_ears_fade_types fade, double position, double curve, e_slope_mapping slopemapping)
{
    return ears_get_fade_factor(-1, fade, position, curve, slopemapping);
}


t_ears_err ears_buffer_fade(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, long fade_in_samples, long fade_out_samples, e_ears_fade_types fade_in_type, e_ears_fade_types fade_out_type, double fade_in_curve, double fade_out_curve, e_slope_mapping slopemapping)
{
    if (source == dest)
        return ears_buffer_fade_inplace(ob, source, fade_in_samples, fade_out_samples, fade_in_type, fade_out_type, fade_in_curve, fade_out_curve, slopemapping);
    
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
        ears_buffer_copy_format(ob, source, dest);
        
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
                    dest_sample[j*dest_channelcount + c] *= ears_get_fade_in_factor(fade_in_type, ((float)j)/fade_in_samples, fade_in_curve, slopemapping);
            }

            // applying fade out
            for (j = framecount - actual_fade_out_samples; j < framecount; j++) {
                for (c = 0; c < dest_channelcount; c++)
                    dest_sample[j*dest_channelcount + c] *= ears_get_fade_out_factor(fade_out_type, (framecount - (float)j)/fade_out_samples, fade_out_curve, slopemapping);
            }
            
            buffer_setdirty(dest);
            buffer_unlocksamples(dest);
        }
        buffer_unlocksamples(source);
    }
    
    return err;
}


t_ears_err ears_buffer_fade_ms(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, long fade_in_ms, long fade_out_ms, e_ears_fade_types fade_in_type, e_ears_fade_types fade_out_type, double fade_in_curve, double fade_out_curve, e_slope_mapping slopemapping)
{
    if (source == dest)
        return ears_buffer_fade_ms_inplace(ob, source, fade_in_ms, fade_out_ms, fade_in_type, fade_out_type, fade_in_curve, fade_out_curve, slopemapping);

    
    float msr = buffer_getmillisamplerate(source);
    long fade_in_samps = round(fade_in_ms * msr);
    long fade_out_samps = round(fade_out_ms * msr);
    
    return ears_buffer_fade(ob, source, dest, fade_in_samps, fade_out_samps, fade_in_type, fade_out_type, fade_in_curve, fade_out_curve, slopemapping);
}


t_ears_err ears_buffer_fade_inplace(t_object *ob, t_buffer_obj *buf, long fade_in_samples, long fade_out_samples, e_ears_fade_types fade_in_type, e_ears_fade_types fade_out_type, double fade_in_curve, double fade_out_curve, e_slope_mapping slopemapping)
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
                sample[j*channelcount + c] *= ears_get_fade_in_factor(fade_in_type, ((float)j)/fade_in_samples, fade_in_curve, slopemapping);
        }
        
        // applying fade out
        for (j = framecount - actual_fade_out_samples; j < framecount; j++) {
            for (c = 0; c < channelcount; c++)
                sample[j*channelcount + c] *= ears_get_fade_out_factor(fade_out_type, (framecount - (float)j)/fade_out_samples, fade_out_curve, slopemapping);
        }
        
        buffer_setdirty(buf);
        buffer_unlocksamples(buf);
    }
    
    return err;
}

t_ears_err ears_buffer_fade_ms_inplace(t_object *ob, t_buffer_obj *buf, long fade_in_ms, long fade_out_ms, e_ears_fade_types fade_in_type, e_ears_fade_types fade_out_type, double fade_in_curve, double fade_out_curve, e_slope_mapping slopemapping)
{
    float msr = buffer_getmillisamplerate(buf);
    long fade_in_samps = round(fade_in_ms * msr);
    long fade_out_samps = round(fade_out_ms * msr);
    
    return ears_buffer_fade_inplace(ob, buf, fade_in_samps, fade_out_samps, fade_in_type, fade_out_type, fade_in_curve, fade_out_curve, slopemapping);
}


void ears_buffer_preprocess_sr_policies(t_object *ob, t_buffer_obj **source, long num_sources, e_ears_resamplingpolicy resamplingpolicy, long resamplingfiltersize, double *sr, bool *resampled, float **samples, long *num_samples, bool *must_free_samples, long *source_first_unique_idx)
{
    if (!ears_buffers_have_the_same_sr(ob, num_sources, source) && resamplingpolicy != EARS_RESAMPLINGPOLICY_DONT) {
        *resampled = true;
        *sr = ears_buffers_get_collective_sr(ob, num_sources, source, resamplingpolicy);
        for (long i = 0; i < num_sources; i++) {
            if (!samples[i])
                continue;
            
            double this_sr = ears_buffer_get_sr(ob, source[i]);
            if (this_sr != *sr && (!source_first_unique_idx || source_first_unique_idx[i] == i)) {
                // must resample
                long num_channels = ears_buffer_get_numchannels(ob, source[i]);
                double factor = (*sr/this_sr);
                long num_in_frames = ears_buffer_get_size_samps(ob, source[i]);
                long num_out_frames = ceil(num_in_frames * factor);
                float *new_samples = (float *)bach_newptr(num_out_frames * num_channels * sizeof(float));
                ears_resample(samples[i], num_in_frames, &new_samples, num_out_frames, factor, this_sr/2., this_sr, resamplingfiltersize, num_channels);
                buffer_unlocksamples(source[i]);
                samples[i] = new_samples;
                num_samples[i] = num_out_frames;
                must_free_samples[i] = true;
                
                if (source_first_unique_idx) {
                    for (long j = i+1; j < num_sources; j++) {
                        if (source_first_unique_idx[j] == i) {
                            samples[j] = samples[i];
                            num_samples[j] = num_samples[i];
                        }
                    }
                }
            }
        }
    }
}

void fill_source_first_unique_idx(t_buffer_obj **source, long num_sources, long *source_first_unique_idx)
{
    for (long i = 0; i < num_sources; i++)
        source_first_unique_idx[i] = i;
    for (long i = 0; i < num_sources; i++) {
        if (source_first_unique_idx[i] != i)
            continue;
        for (long j = i+1; j < num_sources; j++)
            if (source[j] == source[i])
                source_first_unique_idx[j] = i;
    }
}



t_ears_err ears_buffer_mix(t_object *ob, t_buffer_obj **source, long num_sources, t_buffer_obj *dest, t_llll *gains, long *offset_samps,
                           e_ears_normalization_modes normalization_mode, e_slope_mapping slopemapping,
                           e_ears_resamplingpolicy resamplingpolicy, long resamplingfiltersize)
{
    t_ears_err err = EARS_ERR_NONE;
    
    if (num_sources == 0) {
        ears_buffer_set_size_samps(ob, dest, 0);
        return EARS_ERR_NONE;
    }
    
    bool resampled = false;
    long i, num_locked = 0;
    float **samples = (float **)bach_newptr(num_sources * sizeof(float *));
    bool *must_free_samples = (bool *)bach_newptrclear(num_sources * sizeof(bool));
    long *num_samples = (long *)bach_newptr(num_sources * sizeof(long));
    long *num_channels = (long *)bach_newptr(num_sources * sizeof(long));
    long *source_first_unique_idx = (long *)bach_newptrclear(num_sources * sizeof(long));

    fill_source_first_unique_idx(source, num_sources, source_first_unique_idx);

    t_atom_long	channelcount = 0;
    double total_length = 0;
    float *dest_sample = NULL;
    double sr = ears_buffer_get_sr(ob, source[0]);
    
    channelcount = buffer_getchannelcount(source[0]);
    for (i = 0, num_locked = 0; i < num_sources; i++, num_locked++) {
        if (source_first_unique_idx[i] == i) {
            samples[i] = buffer_locksamples(source[i]);
            if (!samples[i]) {
                err = EARS_ERR_CANT_READ;
                object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
//                goto end;
            }
        } else {
            samples[i] = samples[source_first_unique_idx[i]];
        }
    }
    
    
    for (i = 0; i < num_sources; i++) {
        num_samples[i] = samples[i] ? buffer_getframecount(source[i]) : 0;
        num_channels[i] = samples[i] ? buffer_getchannelcount(source[i]) : 1;
        if (num_channels[i] > channelcount)
            channelcount = num_channels[i]; // using as number of channels the maximum number of channels in the input data
    }

    ears_buffer_preprocess_sr_policies(ob, source, num_sources, resamplingpolicy, resamplingfiltersize, &sr, &resampled, samples, num_samples, must_free_samples, source_first_unique_idx);
    
    ears_buffer_set_sr(ob, dest, sr);
    
    // Getting max num samples
    long st = 0;
    total_length = -1;
    while (st < num_sources) {
        if (samples[st]) {
            total_length = num_samples[st] + offset_samps[st];
            break;
        } else {
            st++;
        }
    }
    
    if (total_length < 0) {
        // hard error: there's no buffer to mix!
        err = EARS_ERR_CANT_READ;
        object_error((t_object *)ob, "Cannot read any of the buffers");
        goto end;
    }
    
    for (i = 1; i < num_sources; i++)
        if (samples[i]) {
            if (num_samples[i] + offset_samps[i] > total_length)
                total_length = num_samples[i] + offset_samps[i];
        }
    
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
            
            if (!samples[i])
                continue;
            
            long this_onset_samps = offset_samps[i] > 0 ? offset_samps[i] : 0;
            
            t_ears_envelope_iterator eei = ears_envelope_iterator_create_from_llllelem(elem, 1., false, slopemapping);
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
    for (i = 0; i < num_locked; i++) {
        if (must_free_samples[i] && samples[i])
            bach_freeptr(samples[i]);
        else if (source_first_unique_idx[i] == i)
            buffer_unlocksamples(source[i]);
    }
    
    bach_freeptr(source_first_unique_idx);
    bach_freeptr(must_free_samples);
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


t_ears_err ears_buffer_mix_subsampleprec(t_object *ob, t_buffer_obj **source, long num_sources, t_buffer_obj *dest,
                                          t_llll *gains, double *offset_samps,
                                          e_ears_normalization_modes normalization_mode, e_slope_mapping slopemapping,
                                          e_ears_resamplingpolicy resamplingpolicy, long resamplingfiltersize)
{
    t_ears_err err = EARS_ERR_NONE;
    
    if (num_sources == 0) {
        ears_buffer_set_size_samps(ob, dest, 0);
        return EARS_ERR_NONE;
    }
    
    bool resampled = false;
    long i, num_locked = 0;
    float **samples = (float **)bach_newptr(num_sources * sizeof(float *));
    bool *must_free_samples = (bool *)bach_newptrclear(num_sources * sizeof(bool));
    long *num_samples = (long *)bach_newptr(num_sources * sizeof(long));
    long *num_channels = (long *)bach_newptr(num_sources * sizeof(long));
    long *source_first_unique_idx = (long *)bach_newptrclear(num_sources * sizeof(long));
    
    fill_source_first_unique_idx(source, num_sources, source_first_unique_idx);
    
    t_atom_long    channelcount = 0;
    long total_length = 0;
    float *dest_sample = NULL;
    double sr = ears_buffer_get_sr(ob, source[0]);
    
    channelcount = buffer_getchannelcount(source[0]);        // number of floats in a frame
    for (i = 0, num_locked = 0; i < num_sources; i++, num_locked++) {
        if (source_first_unique_idx[i] == i) {
            samples[i] = buffer_locksamples(source[i]);
            if (!samples[i]) {
                err = EARS_ERR_CANT_WRITE;
                object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
                goto end;
            }
        } else {
            samples[i] = samples[source_first_unique_idx[i]];
        }
    }
    
    
    for (i = 0; i < num_sources; i++) {
        num_samples[i] = buffer_getframecount(source[i]);
        num_channels[i] = buffer_getchannelcount(source[i]);
    }
    
    ears_buffer_preprocess_sr_policies(ob, source, num_sources, resamplingpolicy, resamplingfiltersize, &sr, &resampled, samples, num_samples, must_free_samples, source_first_unique_idx);
    
    ears_buffer_set_sr(ob, dest, sr);
    
    // Getting max num samples
    total_length = num_samples[0] + (long)ceil(offset_samps[0]);
    for (i = 1; i < num_sources; i++)
        if (num_samples[i] + (long)ceil(offset_samps[i]) > total_length)
            total_length = num_samples[i] + (long)ceil(offset_samps[i]);
    
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
            
            double this_onset_samps = offset_samps[i] > 0 ? offset_samps[i] : 0;
            t_ears_envelope_iterator eei = ears_envelope_iterator_create_from_llllelem(elem, 1., false, slopemapping);

            if (fmod(this_onset_samps, 1.) == 0) {
                for (j = 0; j < num_samples[i]; j++) {
                    double this_gain = ears_envelope_iterator_walk_interp(&eei, j, num_samples[i]);
                    for (c = 0; c < num_channels[i] && c < channelcount; c++)
                        dest_sample[(j + (long)this_onset_samps) * channelcount + c] += samples[i][j * num_channels[i] + c] * this_gain;
                }
            } else {
                long this_onset_samps_floor = (long)floor(this_onset_samps);
                double diff = this_onset_samps - this_onset_samps_floor;
                long lower_limit = MAX(-resamplingfiltersize-1, 0);
                long upper_limit = MIN(num_samples[i] + resamplingfiltersize+1, total_length - this_onset_samps_floor);
                for (j = lower_limit; j < upper_limit; j++) {
                    double this_gain = ears_envelope_iterator_walk_interp(&eei, j, num_samples[i]);
                    for (c = 0; c < num_channels[i] && c < channelcount; c++) {
//                        double s = samples[i][j * num_channels[i] + c];
                        double s = ears_interp_bandlimited(&(samples[i][c]), num_samples[i], j - diff, resamplingfiltersize, num_channels[i]);
                        dest_sample[(j + this_onset_samps_floor) * channelcount + c] += s * this_gain;
                    }
                }
            }
        }
        
        buffer_setdirty(dest);
        buffer_unlocksamples(dest);
    }
    
end:
    for (i = 0; i < num_locked; i++) {
        if (must_free_samples[i])
            bach_freeptr(samples[i]);
        else if (source_first_unique_idx[i] == i)
            buffer_unlocksamples(source[i]);
    }
    
    bach_freeptr(source_first_unique_idx);
    bach_freeptr(must_free_samples);
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


t_ears_err ears_buffer_mix_from_llll(t_object *ob, t_llll *sources_ll, t_buffer_obj *dest, t_llll *gains, t_llll *offset_samps_ll, e_ears_normalization_modes normalization_mode, e_slope_mapping slopemapping, e_ears_resamplingpolicy resamplingpolicy, long resamplingfiltersize)
{
    long num_sources = sources_ll->l_size;
    
    if (num_sources == 0)
        return ears_buffer_pack(ob, 0, NULL, dest, resamplingpolicy, resamplingfiltersize);
    else {
        long i = 0;
        long *offset_samps = (long *)bach_newptrclear(num_sources * sizeof(long));
        t_buffer_obj **sources = (t_buffer_obj **)bach_newptr(num_sources * sizeof(t_buffer_obj *));

        for (t_llllelem *el = sources_ll->l_head; el; el = el->l_next, i++)
            sources[i] = (t_buffer_obj *)hatom_getobj(&el->l_hatom);
        
        i = 0;
        for (t_llllelem *el = offset_samps_ll->l_head; el; el = el->l_next, i++)
            offset_samps[i] = hatom_getlong(&el->l_hatom);

        t_max_err err = ears_buffer_mix(ob, sources, num_sources, dest, gains, offset_samps, normalization_mode, slopemapping, resamplingpolicy, resamplingfiltersize);
        bach_freeptr(sources);
        bach_freeptr(offset_samps);
        return err;
    }
}


t_ears_err ears_buffer_concat(t_object *ob, t_buffer_obj **source, long num_sources, t_buffer_obj *dest,
                              long *xfade_samples, char also_fade_boundaries,
                              e_ears_fade_types fade_type, double fade_curve, e_slope_mapping slopemapping,
                              e_ears_resamplingpolicy resamplingpolicy, long resamplingfiltersize)
{
    t_ears_err err = EARS_ERR_NONE;

    if (num_sources == 0) {
        ears_buffer_set_size_samps(ob, dest, 0);
        return EARS_ERR_NONE;
    } else if (num_sources == 1) {
        if (also_fade_boundaries)
            return ears_buffer_fade(ob, source[0], dest, xfade_samples[0], xfade_samples[0], fade_type, fade_type, fade_curve, fade_curve, slopemapping);
        else
            return ears_buffer_clone(ob, source[0], dest);
    }
    
    // here we have at least 2 buffers to concatenate
    
    bool resampled = false;
    long i, num_locked = 0;
    float **samples = (float **)bach_newptr(num_sources * sizeof(float *));
    bool *must_free_samples = (bool *)bach_newptrclear(num_sources * sizeof(bool));
    long *num_samples = (long *)bach_newptr(num_sources * sizeof(long));
    long *num_channels = (long *)bach_newptr(num_sources * sizeof(long));

    // position in samples of beginning, end, end of fade in and beginning of fade out, for each buffer
    long *sample_start = (long *)bach_newptr(num_sources * sizeof(long));
    long *sample_fadein_end = (long *)bach_newptr(num_sources * sizeof(long)); // this is the FIRST sample of the non-fade region
    long *sample_fadeout_start = (long *)bach_newptr(num_sources * sizeof(long));
    long *sample_end = (long *)bach_newptr(num_sources * sizeof(long)); // this is the FIRST sample of the region AFTER the buffer,
                                                                        // so that sample_end-sample_start = actual samples in the buffer
    long *source_first_unique_idx = (long *)bach_newptrclear(num_sources * sizeof(long));
    
    fill_source_first_unique_idx(source, num_sources, source_first_unique_idx);

    t_atom_long	channelcount = 0;
    double total_length = 0;
    float *dest_sample = NULL;
    double sr = ears_buffer_get_sr(ob, source[0]);
    
    for (i = 0, num_locked = 0; i < num_sources; i++, num_locked++) {
        if (source_first_unique_idx[i] == i) {
            samples[i] = buffer_locksamples(source[i]);
            if (!samples[i]) {
                err = EARS_ERR_CANT_WRITE;
                object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
                goto end;
            }
        } else {
            samples[i] = samples[source_first_unique_idx[i]];
        }
    }
    
    ears_buffer_preprocess_sr_policies(ob, source, num_sources, resamplingpolicy, resamplingfiltersize, &sr, &resampled, samples, num_samples, must_free_samples, source_first_unique_idx);
    
    ears_buffer_set_sr(ob, dest, sr);
    
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
            sample_fadein_end[i] = also_fade_boundaries ? sample_start[i] + MIN(xfade_samples[i]/2, num_samples[i]/2) : 0;
            sample_fadeout_start[i] = sample_end[i] - MIN(MIN(xfade_samples[i]/2, num_samples[i]/2), num_samples[i+1]/2);
        } else if (i == num_sources - 1) {
            sample_start[i] = sample_fadeout_start[i-1];
            sample_end[i] = sample_start[i] + num_samples[i];
            sample_fadein_end[i] = sample_start[i] + MIN(MIN(xfade_samples[i]/2, num_samples[i]/2), num_samples[i-1]/2);
            sample_fadeout_start[i] = also_fade_boundaries ? sample_end[i] - MIN(xfade_samples[i]/2, num_samples[i]/2) : sample_end[i];
        } else {
            sample_start[i] = sample_fadeout_start[i-1];
            sample_end[i] = sample_start[i] + num_samples[i];
            sample_fadein_end[i] = sample_start[i] + MIN(MIN(xfade_samples[i]/2, num_samples[i]/2), num_samples[i-1]/2);
            sample_fadeout_start[i] = sample_end[i] - MIN(MIN(xfade_samples[i]/2, num_samples[i]/2), num_samples[i+1]/2);
        }
    }

    total_length = sample_end[num_sources-1]; // global length
//    ears_buffer_set_size_samps(ob, dest, total_length);
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
                    dest_sample[j * channelcount + c] += samples[i][l * num_channels[i] + c] * ears_get_fade_in_factor(fade_type, ((float)l)/(sample_fadein_end[i] - sample_start[i]), fade_curve, slopemapping);
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
                    dest_sample[j * channelcount + c] += samples[i][l * num_channels[i] + c] * ears_get_fade_out_factor(fade_type, (sample_end[i] - ((float)j))/(sample_end[i] - sample_fadeout_start[i]), fade_curve, slopemapping);
                }
            }
        }
        
        buffer_setdirty(dest);
        buffer_unlocksamples(dest);
    }
    
end:
    for (i = 0; i < num_locked; i++) {
        if (must_free_samples[i])
            bach_freeptr(samples[i]);
        else if (source_first_unique_idx[i] == i)
            buffer_unlocksamples(source[i]);
    }
    
    bach_freeptr(source_first_unique_idx);
    bach_freeptr(samples);
    bach_freeptr(must_free_samples);
    bach_freeptr(num_samples);
    bach_freeptr(num_channels);
    bach_freeptr(sample_fadein_end);
    bach_freeptr(sample_end);
    
    return err;
}




// take an existing buffer and mixes another one over it
t_ears_err ears_buffer_assemble_once(t_object *ob, t_buffer_obj *basebuffer, t_buffer_obj *newbuffer, t_llll *gains, long offset_samps, e_slope_mapping slopemapping, e_ears_resamplingpolicy resamplingpolicy, long resamplingfiltersize, long *basebuffer_numframes, long *basebuffer_allocatedframes)
{
    t_ears_err err = EARS_ERR_NONE;
    
    if (!basebuffer) {
        return EARS_ERR_NO_BUFFER;
    }
    
    if (!newbuffer) {
        return EARS_ERR_NO_BUFFER;
    }
    
    char must_free = false;
    t_atom_long channelcount = ears_buffer_get_numchannels(ob, basebuffer);
    long numsamps = *basebuffer_numframes; //ears_buffer_get_size_samps(ob, basebuffer);
    double sr = ears_buffer_get_sr(ob, basebuffer);
    
    t_atom_long new_channelcount = ears_buffer_get_numchannels(ob, newbuffer);
    long new_numsamps = ears_buffer_get_size_samps(ob, newbuffer);
    double new_sr = ears_buffer_get_sr(ob, newbuffer);

    float *base_sample, *new_sample;
    
    new_sample = buffer_locksamples(newbuffer);
    
    if (new_sr != sr) {
        // must resample!
        double factor = (sr/new_sr);
        long num_in_frames = new_numsamps;
        long num_out_frames = ceil(num_in_frames * factor);
        float *resampled_samples = (float *)bach_newptr(num_out_frames * new_channelcount * sizeof(float));
        ears_resample(new_sample, num_in_frames, &resampled_samples, num_out_frames, factor, new_sr/2., new_sr, resamplingfiltersize, new_channelcount);
        buffer_unlocksamples(newbuffer);
        new_sample = resampled_samples;
        must_free = true;
    }
    
    // Getting max num samples
    long total_length = MAX(numsamps, new_numsamps + offset_samps);
    
    if (total_length > *basebuffer_allocatedframes) {
        long step = EARS_BUFFER_ASSEMBLE_ALLOCATION_STEP_SEC * sr;
        while (total_length > *basebuffer_allocatedframes)
            (*basebuffer_allocatedframes) += step;
        
        ears_buffer_set_size_samps_preserve(ob, basebuffer, *basebuffer_allocatedframes);
    }
    *basebuffer_numframes = total_length;
    
    base_sample = buffer_locksamples(basebuffer);
    if (!base_sample) {
        err = EARS_ERR_CANT_WRITE;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
    } else {
        // writing samples
        long j, c;
        t_llllelem *elem = gains ? gains->l_head : NULL;
        long this_onset_samps = offset_samps > 0 ? offset_samps : 0;
        
        t_ears_envelope_iterator eei = ears_envelope_iterator_create_from_llllelem(elem, 1., false, slopemapping);
        for (j = 0; j < new_numsamps; j++) {
            double this_gain = ears_envelope_iterator_walk_interp(&eei, j, new_numsamps);
            for (c = 0; c < new_channelcount && c < channelcount; c++) {
                base_sample[(j + this_onset_samps) * channelcount + c] += new_sample[j * new_channelcount + c] * this_gain;
            }
        }
        
        buffer_setdirty(basebuffer);
        buffer_unlocksamples(basebuffer);
    }
    
//end:
    if (must_free)
        bach_freeptr(new_sample);
    else
        buffer_unlocksamples(newbuffer);
    

    return err;
}

void ears_buffer_assemble_close(t_object *ob, t_buffer_obj *basebuffer, e_ears_normalization_modes normalization_mode, long length_samps)
{
    t_ears_err err = EARS_ERR_NONE;
    
    if (!basebuffer)
        return EARS_ERR_NO_BUFFER;
    
    if (length_samps < 0)
        return EARS_ERR_GENERIC;
    
    // Finally, we normalize if needed
    switch (normalization_mode) {
        case EARS_NORMALIZE_DO:
            ears_buffer_normalize_inplace(ob, basebuffer, 1.);
            break;
            
        case EARS_NORMALIZE_OVERLOAD_PROTECTION_ONLY:
        {
            double maxabs = 0.;
            t_ears_err err = ears_buffer_get_maxabs(ob, basebuffer, &maxabs);
            if (err == EARS_ERR_EMPTY_BUFFER)
                object_warn(ob, EARS_ERROR_BUF_EMPTY_BUFFER);
            if (err == EARS_ERR_NONE && maxabs > 1.) {
                object_warn(ob, "Mixdown peak is %.3f, output buffer will be normalized due to overload protection.", maxabs);
                ears_buffer_normalize_inplace(ob, basebuffer, 1.);
            }
        }
            break;
            
        case EARS_NORMALIZE_DONT:
        default:
            break;
    }
    
    // and crop the result
    ears_buffer_set_size_samps_preserve(ob, basebuffer, length_samps);

    return err;
}



t_symbol *default_filepath2buffername(t_symbol *filepath, long buffer_index)
{
    char temp[MAX_PATH_CHARS * 2];
    snprintf_zero(temp, MAX_PATH_CHARS * 2, "%s_%ld_earsbuf", filepath->s_name, buffer_index);
    return gensym(temp);
}

t_symbol *default_synth2buffername(long buffer_index)
{
    char temp[MAX_PATH_CHARS * 2];
    snprintf_zero(temp, MAX_PATH_CHARS * 2, "synth_%ld_earsbuf", buffer_index);
    return gensym(temp);
}

t_ears_err ears_buffer_from_buffer(t_object *ob, t_buffer_obj **dest, t_symbol *buffername, double start_ms, double end_ms, long buffer_idx)
{
    t_symbol *name = default_filepath2buffername(buffername, buffer_idx);
    t_atom a;
    atom_setsym(&a, name);
    *dest = (t_buffer_obj *)object_new_typed(CLASS_BOX, gensym("buffer~"), 1, &a);
    
    t_ears_err this_err = ears_buffer_clone(ob, ears_buffer_getobject(buffername), *dest);
    
    if (start_ms > 0 || end_ms >= 0)
        ears_buffer_crop_ms(ob, *dest, *dest, start_ms, end_ms);
    return this_err;
}

// This is not used by the [ears.read] object, but rather from other objects.
// The <dest> buffers are meant to be used and freed.
t_ears_err ears_buffer_from_file(t_object *ob, t_buffer_obj **dest, t_symbol *file, double start_ms, double end_ms, long buffer_idx)
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
            if (ears_symbol_ends_with(filepath, ".mp3", true)) {
                err = ears_buffer_read_handle_mp3(ob, filepath->s_name, start_ms, end_ms, *dest, EARS_TIMEUNIT_MS);
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
        
        *dest = (t_buffer_obj *)object_new_typed(CLASS_BOX, gensym("buffer~"), 0, NULL);
        ears_buffer_setempty(ob, *dest, 1);

    }
    
    return err;
}

// DEFINITELY NOT THE QUICKEST SYNTH ;-)
// No antialiasing happening here; this should be done later, if one wants it
double ears_synth_calc(double phase, e_ears_synthmode mode, float *wavetable, long wavetable_length, long resampling_filter_width)
{
    switch (mode) {
        case EARS_SYNTHMODE_SINUSOIDS:
            return sin(phase);
            break;

        case EARS_SYNTHMODE_TRIANGULAR:
            phase = positive_fmod(phase, TWOPI);
            return phase < PIOVERTWO ? phase/PIOVERTWO : (phase < 3*PIOVERTWO ? 2 - phase/PIOVERTWO : phase/PIOVERTWO-4);
            break;

        case EARS_SYNTHMODE_RECTANGULAR:
            phase = positive_fmod(phase, TWOPI);
            return phase > PI ? -1 : 1;
            break;

        case EARS_SYNTHMODE_SAWTOOTH:
            phase = positive_fmod(phase, TWOPI);
            return 2 * phase/TWOPI - 1;
            break;

        case EARS_SYNTHMODE_WAVETABLE:
        {
            
            phase = positive_fmod(phase, TWOPI);
            double idx = wavetable_length * (phase/TWOPI);
            if (false) { // LINEAR INTERPOLATION
                long idx_floor = floor(idx);
                long idx_ceil = idx_floor + 1;
                double diff = idx - idx_floor;
                if (idx_floor >= wavetable_length) idx_floor -= wavetable_length;
                if (idx_ceil >= wavetable_length) idx_ceil -= wavetable_length;
                return (1-diff) * wavetable[idx_floor] + diff * wavetable[idx_ceil]; // linear interpolation
            } else {
                return ears_interp_circular_bandlimited(wavetable, wavetable_length, idx, resampling_filter_width);
            }
            
        }
            break;

        default:
            return 0;
            break;
    }
}


t_ears_err ears_buffer_synth_from_duration_line(t_object *e_ob, t_buffer_obj **dest,
                                                e_ears_synthmode mode, float *wavetable, long wavetable_length,
                                                double midicents, double duration_ms, double velocity, t_llll *breakpoints,
                                                e_ears_veltoamp_modes veltoamp_mode, double amp_vel_min, double amp_vel_max,
                                                double middleAtuning, double sr, long buffer_idx, e_slope_mapping slopemapping,
                                                long oversampling, long resamplingfiltersize)
{
    if (mode == EARS_SYNTHMODE_SINUSOIDS)
        oversampling = 1; // no need for oversampling if we just use sinusoids, as we will not filter for antialiasing

    t_ears_err err = EARS_ERR_NONE;
    double sr_os = sr * oversampling;
    long duration_samps = (long)ceil(duration_ms * (sr_os/1000.));
    t_symbol *name = default_synth2buffername(buffer_idx);
    t_atom a;
    atom_setsym(&a, name);
    
    *dest = (t_buffer_obj *)object_new_typed(CLASS_BOX, gensym("buffer~"), 1, &a);

    ears_buffer_set_size_and_numchannels(e_ob, *dest, duration_samps, 1);
    ears_buffer_set_sr(e_ob, *dest, sr_os);

    float *sample = buffer_locksamples(*dest);

    if (mode == EARS_SYNTHMODE_WAVETABLE && (!wavetable || wavetable_length < 2)) {
        err = EARS_ERR_GENERIC;
        object_error(e_ob, "No wavetable provided, or wavetable is too short.");
    } else if (!sample) {
        err = EARS_ERR_CANT_READ;
        object_error(e_ob, EARS_ERROR_BUF_CANT_READ);
    } else {
        t_atom_long    framecount   = buffer_getframecount(*dest);            // number of floats long the buffer is for a single channel
        
        // building envelope iterators
        t_llll *pitchenv = NULL;
        t_llll *velocityenv = NULL;
        
        if (breakpoints) {
            pitchenv = llll_clone(breakpoints);
            velocityenv = llll_clone(breakpoints);
            if (breakpoints->l_head && hatom_gettype(&breakpoints->l_head->l_hatom) == H_SYM) {
                // removing "breakpoints" symbol
                llll_behead(pitchenv);
                llll_behead(velocityenv);
            }
            for (t_llllelem *el = pitchenv->l_head; el; el = el->l_next) {
                if (hatom_gettype(&el->l_hatom) == H_LLLL) {
                    t_llll *ll = hatom_getllll(&el->l_hatom);
                    if (ll->l_size == 4)
                        llll_destroyelem(ll->l_tail);
                }
            }
            
            ears_llll_to_env_samples(pitchenv, duration_samps, sr_os, EARS_TIMEUNIT_DURATION_RATIO);
            
            bool has_no_velocity = false;
            for (t_llllelem *el = velocityenv->l_head; el; el = el->l_next) {
                if (hatom_gettype(&el->l_hatom) == H_LLLL) {
                    t_llll *ll = hatom_getllll(&el->l_hatom);
                    if (ll->l_size < 4) {
                        has_no_velocity = true;
                        break;
                    } else if (ll->l_size == 4) {
                        llll_swapelems(ll->l_head->l_next, ll->l_tail);
                        llll_destroyelem(ll->l_tail);
                    }
                }
            }
            if (has_no_velocity) {
                llll_clear(velocityenv);
                llll_appenddouble(velocityenv, velocity);
            } else {
                ears_llll_to_env_samples(velocityenv, duration_samps, sr_os, EARS_TIMEUNIT_DURATION_RATIO);
            }
        }

        // building envelope iterators
        t_ears_envelope_iterator eei_deltapitch = ears_envelope_iterator_create(pitchenv, 0, false, slopemapping);
        t_ears_envelope_iterator eei_vel = ears_envelope_iterator_create(velocityenv, velocity, false, slopemapping);

        // synthesizing
        double running_phase = 0;
        double t_step = (1./sr_os);
        for (long i = 0; i < framecount; i++) {
            double cents = ears_envelope_iterator_walk_interp(&eei_deltapitch, i, framecount) + midicents;
            double vel = ears_envelope_iterator_walk_interp(&eei_vel, i, framecount);
            double freq = ears_cents_to_hz(cents, middleAtuning);
            double amp = 1;
            
            switch (veltoamp_mode) {
                case EARS_VELOCITY_TO_AMPLITUDE:
                    amp = rescale(vel, 0., 127., amp_vel_min, amp_vel_max);
                    break;
                case EARS_VELOCITY_TO_DECIBEL:
                    amp = ears_db_to_linear(rescale(vel, 0., 127., amp_vel_min, amp_vel_max));
                    break;
                default:
                    break;
            }
            
//            sample[i] = amp * sin(running_phase);

            // TO DO: this resamplingfiltersize may be different (smaller?) than the one used below for reducing the sample rate
            sample[i] = amp * ears_synth_calc(running_phase, mode, wavetable, wavetable_length, resamplingfiltersize);

            running_phase += TWOPI * freq * t_step;
            while (running_phase > TWOPI)
                running_phase -= TWOPI;
            
        }
        
        buffer_setdirty(*dest);
        buffer_unlocksamples(*dest);
        
        
        
        switch (mode) {
            case EARS_SYNTHMODE_SAWTOOTH:
            case EARS_SYNTHMODE_RECTANGULAR:
            case EARS_SYNTHMODE_TRIANGULAR:
            case EARS_SYNTHMODE_WAVETABLE:
                // a fourth order lowpass for antialiasing
                ears_buffer_onepole(e_ob, *dest, *dest, sr * 0.5, false);
                ears_buffer_onepole(e_ob, *dest, *dest, sr * 0.5, false);
                ears_buffer_onepole(e_ob, *dest, *dest, sr * 0.5, false);
                ears_buffer_onepole(e_ob, *dest, *dest, sr * 0.5, false);
                break;
                
            default:
                break;
        }
        
        if (oversampling > 1) {
            ears_buffer_resample(e_ob, *dest, 1./oversampling, resamplingfiltersize);
            ears_buffer_set_sr(e_ob, *dest, sr);
            
            // This would be faster but of course less precise:
            //            ears_buffer_decimate(e_ob, *dest, *dest, oversampling);
        }

        llll_free(pitchenv);
        llll_free(velocityenv);
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

        ears_buffer_set_size_samps(ob, dest, new_numsamples);
        
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


t_ears_err ears_buffer_offset(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, long shift_samps)
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
        
        ears_buffer_set_size_samps(ob, dest, framecount + shift_samps);
        
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


t_ears_err ears_buffer_offset_subsampleprec(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, double shift_samps, double resamplingfiltersize)
{
    if (floor(shift_samps) == shift_samps) // simple integer case
        return ears_buffer_offset(ob, source, dest, shift_samps);
    
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
        t_atom_long    out_framecount = MAX(0, framecount + shift_samps);
        
        if (source == dest) { // inplace operation!
            orig_sample_wk = (float *)bach_newptr(channelcount * framecount * sizeof(float));
            sysmem_copyptr(orig_sample, orig_sample_wk, channelcount * framecount * sizeof(float));
            buffer_unlocksamples(source);
        } else {
            orig_sample_wk = orig_sample;
            ears_buffer_copy_format(ob, source, dest);
        }
        
        ears_buffer_set_size_samps(ob, dest, out_framecount);
        
        float *dest_sample = buffer_locksamples(dest);
        
        if (!dest_sample) {
            err = EARS_ERR_CANT_WRITE;
            object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
        } else {
            //            t_atom_long    dest_channelcount = buffer_getchannelcount(dest);
            //            t_atom_long    dest_framecount   = buffer_getframecount(dest);
            
            for (long f = 0; f < out_framecount; f++) {
                for (long c = 0; c < channelcount; c++) {
                    double s = ears_interp_bandlimited(&(orig_sample_wk[c]), framecount, f - shift_samps, resamplingfiltersize, channelcount);
                    dest_sample[f * channelcount + c] = s;
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



t_ears_err ears_buffer_decimate(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, long factor)
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
        double sr = ears_buffer_get_sr(ob, source);
        
        if (source == dest) { // inplace operation!
            orig_sample_wk = (float *)bach_newptr(channelcount * framecount * sizeof(float));
            sysmem_copyptr(orig_sample, orig_sample_wk, channelcount * framecount * sizeof(float));
            buffer_unlocksamples(source);
        } else {
            orig_sample_wk = orig_sample;
            ears_buffer_copy_format(ob, source, dest);
        }

        ears_buffer_set_size_samps(ob, dest, framecount/factor);
        ears_buffer_set_sr(ob, dest, sr/factor);

        float *dest_sample = buffer_locksamples(dest);
        
        if (!dest_sample) {
            err = EARS_ERR_CANT_WRITE;
            object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
        } else {
            
            long g = 0;
            for (long f = 0; f < framecount; f += factor, g++) {
                for (long c = 0; c < channelcount; c++) {
                    dest_sample[g * channelcount + c] = orig_sample_wk[f * channelcount + c];
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
            ears_buffer_set_size_samps(ob, dest, framecount);
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
            ears_buffer_set_size_samps(ob, dest, framecount);
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



t_ears_err ears_buffer_transpose(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest)
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
        }
        
        ears_buffer_set_size_and_numchannels(ob, dest, channelcount, framecount);
        
        float *dest_sample = buffer_locksamples(dest);
        
        if (!dest_sample) {
            err = EARS_ERR_CANT_WRITE;
            object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
        } else {
            
            for (long f = 0; f < framecount; f++)
                for (long c = 0; c < channelcount; c++)
                    dest_sample[c * framecount + f] = orig_sample_wk[f * channelcount + c];
            
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



t_ears_err ears_buffer_expr(t_object *ob, t_lexpr *expr,
                            t_hatom *arguments, long num_arguments,
                            t_buffer_obj *dest, e_ears_normalization_modes normalization_mode, char envtimeunit, e_slope_mapping slopemapping,
                            e_ears_resamplingpolicy resamplingpolicy, long resamplingfiltersize)
{
    t_ears_err err = EARS_ERR_NONE;
    
    if (num_arguments == 0) {
        ears_buffer_set_size_samps(ob, dest, 0);
        return EARS_ERR_NONE;
    }
    
    // find reference buffer index (first introduced buffer)
    long ref_i = -1;
    for (long i = 0; i < num_arguments; i++)
        if (hatom_gettype(arguments+i) == H_OBJ) {
            ref_i = i;
            break;
        }
    
    if (ref_i < 0) {
        object_error(ob, "At least one buffer must be introduced as variable");
        ears_buffer_set_size_samps(ob, dest, 0);
        return EARS_ERR_GENERIC;
    }

    bool resampled = false;
    long i = 0;
    t_buffer_obj **source = (t_buffer_obj **)bach_newptrclear(num_arguments * sizeof(t_buffer_obj *));
    float **samples = (float **)bach_newptr(num_arguments * sizeof(float *));
    long *num_samples = (long *)bach_newptr(num_arguments * sizeof(long));
    bool *must_free_samples = (bool *)bach_newptrclear(num_arguments * sizeof(bool));
    long *num_channels = (long *)bach_newptr(num_arguments * sizeof(long));
    long *locked = (long *)bach_newptrclear(num_arguments * sizeof(long));
    long *argtype = (long *)bach_newptrclear(num_arguments * sizeof(long));
    double *numericargs = (double *)bach_newptrclear(num_arguments * sizeof(double));
    t_ears_envelope_iterator *eei = (t_ears_envelope_iterator *)bach_newptrclear(num_arguments * sizeof(t_ears_envelope_iterator));
    t_llll **eei_envs = (t_llll **)bach_newptrclear(num_arguments * sizeof(t_llll*));
    long *source_first_unique_idx = (long *)bach_newptrclear(num_arguments * sizeof(long));

    for (long i = 0; i < num_arguments; i++)
        source_first_unique_idx[i] = i;
    
    for (long i = 0; i < num_arguments; i++) {
        if (hatom_gettype(arguments+i) == H_OBJ) {
            argtype[i] = 0; // buffer
        } else if (is_hatom_number(arguments+i)) {
            argtype[i] = 1; // number
            numericargs[i] = hatom_getdouble(arguments + i);
        } else if (hatom_gettype(arguments + i) == H_LLLL){
            argtype[i] = 2; // breakpoint function
        } else {
            object_warn(ob, "Unknown argument type!");
            argtype[i] = -1;
        }
    }

    t_atom_long    channelcount = 0;
    long total_length_samps = 0;
    float *dest_sample = NULL;
    double sr = 0;
    
    channelcount = ears_buffer_get_numchannels(ob, (t_buffer_obj *)hatom_getobj(arguments+ref_i));
    sr = ears_buffer_get_sr(ob, (t_buffer_obj *)hatom_getobj(arguments+ref_i));
    
    for (i = 0; i < num_arguments; i++) {
        if (argtype[i] == 0) {
            source[i] = (t_buffer_obj *)hatom_getobj(arguments+i);
        }
    }

    for (long i = 0; i < num_arguments; i++) {
        if (!source[i] || source_first_unique_idx[i] != i)
            continue;
        for (long j = i+1; j < num_arguments; j++)
            if (source[j] && source[j] == source[i])
                source_first_unique_idx[j] = i;
    }
    
    for (i = 0; i < num_arguments; i++) {
        if (source[i] && (source_first_unique_idx[i] == i)) {
            samples[i] = buffer_locksamples((t_buffer_obj *)hatom_getobj(arguments+i));
            locked[i] = true;
            if (!samples[i]) {
                err = EARS_ERR_CANT_WRITE;
                object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
                goto end;
            }
        } else if (source[i]) {
            samples[i] = samples[source_first_unique_idx[i]];
        }
    }


    
    ears_buffer_copy_format(ob, source[ref_i], dest); // we consider the first as "master"
    
    /// All buffers have been locked. Now we need to harmonize numchannels and sampsize.
    /// As a rule: we take the property of the first one.
    for (i = 0; i < num_arguments; i++) {
        if (argtype[i] == 0) {
            num_channels[i] = ears_buffer_get_numchannels(ob, (t_buffer_obj *)hatom_getobj(arguments+i)); // it happens that copy_format doesn't work in changing the number of channels
            num_samples[i] = ears_buffer_get_size_samps(ob, (t_buffer_obj *)hatom_getobj(arguments+i));
        } else {
            // not a buffer
            num_channels[i] = 0;
            num_samples[i] = -1;
        }
    }
    
    ears_buffer_preprocess_sr_policies(ob, source, num_arguments, resamplingpolicy, resamplingfiltersize, &sr, &resampled, samples, num_samples, must_free_samples, source_first_unique_idx);
    
    ears_buffer_set_sr(ob, dest, sr);
    
    // Getting max num samples and max num channels
    total_length_samps = 0;
    channelcount = 0;
    for (i = 0; i < num_arguments; i++) {
        if (num_samples[i] > total_length_samps)
            total_length_samps = num_samples[i];
        if (num_channels[i] > channelcount)
            channelcount = num_channels[i];
    }
    
    ears_buffer_set_size_and_numchannels(ob, dest, total_length_samps, channelcount);
    
    // changing envelopes due to the fact tha
    for (long i = 0; i < num_arguments; i++) {
        if (argtype[i] == 2) {
            eei_envs[i] = llll_clone(hatom_getllll(arguments + i));
            ears_llll_to_env_samples(eei_envs[i], total_length_samps, sr, envtimeunit);
            eei[i] = ears_envelope_iterator_create(eei_envs[i], 0., false, slopemapping);
        }
    }
    
    dest_sample = buffer_locksamples(dest);
    if (!dest_sample) {
        err = EARS_ERR_CANT_WRITE;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
    } else {
        // erasing samples
        memset(dest_sample, 0, total_length_samps * channelcount * sizeof(float));
        
        t_hatom vars[LEXPR_MAX_VARS];
        t_hatom stack[L_MAX_TOKENS];
        hatom_setdouble(stack, 0);
        
/*        // clearing all variables
        for (long i = 0; i < LEXPR_MAX_VARS; i++)
            hatom_setdouble(vars + i, 0.);
  */
        
        // writing samples
        if (expr) {
            long j, c;
            // first: setting all numeric args which are going to be constant
            for (long i = 0; i < num_arguments && i < LEXPR_MAX_VARS; i++) {
                if (argtype[i] == 1) // number
                    hatom_setdouble(vars+i, numericargs[i]);
            }
            

            for (c = 0; c < channelcount; c++) {
                
//                hatom_setdouble(vars+251, c+1);
                
                for (long i = 0; i < num_arguments; i++)
                    if (argtype[i] == 2) // function
                        ears_envelope_iterator_reset(eei+i);
                
                for (j = 0; j < total_length_samps; j++) {
                    // setting variables
                    for (long i = 0; i < num_arguments && i < LEXPR_MAX_VARS; i++) {
                        // could be optimized by stripping the numeric arguments from the cycle, this is not the bottleneck though
                        switch (argtype[i]) {
                            case 0: // buffer
                                hatom_setdouble(vars+i, (c < num_channels[i] && j < num_samples[i]) ?
                                                samples[i][j * num_channels[i] + c] : 0);
                                break;
                                
                            case 2: // envelope
                                hatom_setdouble(vars+i, ears_envelope_iterator_walk_interp(eei+i, j, total_length_samps));
                                break;
                                
                            default:
                                break;
                        }
                    }
                    
                    // used to be like this:
                    // TO DO: optimize this stuff, should not be like this
                    //                    t_hatom *res = lexpr_eval(expr, vars);
//                    bach_freeptr(res);

//                    hatom_setdouble(vars+254, ((double)j)/sr);
  //                  hatom_setdouble(vars+253, total_length_samps > 0 ? ((double)j)/(total_length_samps - 1) : 0);
    //                hatom_setdouble(vars+252, j + 1);

                    lexpr_eval_upon(expr, vars, stack); // optimized version of lexpr_eval for many evaluations
                    dest_sample[j * channelcount + c] = hatom_getdouble(stack);
                }
            }
        }
        
        buffer_setdirty(dest);
        buffer_unlocksamples(dest);
    }
    
end:
    for (i = 0; i < num_arguments; i++) {
        if (locked[i]) {
            if (must_free_samples[i])
                bach_freeptr(samples[i]);
            else if (source_first_unique_idx[i] == i)
                buffer_unlocksamples((t_buffer_obj *)hatom_getobj(arguments+i));
        }
    }
    
    for (long i = 0; i < num_arguments; i++) {
        if (argtype[i] == 2) {
            llll_free(eei_envs[i]);
        }
    }
    
    bach_freeptr(source);
    bach_freeptr(samples);
    bach_freeptr(must_free_samples);
    bach_freeptr(num_samples);
    bach_freeptr(num_channels);
    bach_freeptr(locked);
    bach_freeptr(argtype);
    bach_freeptr(numericargs);
    bach_freeptr(eei);
    bach_freeptr(eei_envs);
    
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



t_symbol *get_conformed_resolved_path(t_symbol *filename)
{
    t_symbol *resolved_path = filename;
    path_absolutepath(&resolved_path, filename, NULL, 0);
    
    char conformed_path[MAX_PATH_CHARS];
    path_nameconform(resolved_path->s_name, conformed_path, PATH_STYLE_MAX, PATH_TYPE_BOOT);
    
    return gensym(conformed_path);
}




const char *get_filename_ext(const char *filename)
{
    const char *dot = strrchr(filename, '.');
    if (!dot || dot == filename) return "";
    return dot + 1;
}

void ears_buffer_write(t_object *buf, t_symbol *filename, t_object *culprit, t_ears_encoding_settings *settings)
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
        ears_writemp3(buf, filename, settings);
    else if (!strcmp(ext, "wv") || !strcmp(ext, "wavpack"))
        ears_writewavpack(buf, filename, settings);
    else {
        object_error(culprit, "Could not determine file tipe from extension.");
        object_error(culprit, "       Please use one of the following extensions: aif(f), wav(e), mp3, flac, wv (wavpack) or data.");
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


t_ears_err ears_buffer_get_split_points_samps_onset(t_object *ob, t_buffer_obj *buf, double attack_thresh_linear, double release_thresh_linear, double min_silence_samps, long lookahead_samps, long smoothingwin_samps, t_llll **samp_start, t_llll **samp_end, char keep_first)
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
        *samp_start = llll_get();
        *samp_end = llll_get();

        bool first_samp_is_attack = false;
        bool in_attack = 0;
        t_atom_long    channelcount = buffer_getchannelcount(buf);
        t_atom_long    framecount   = buffer_getframecount(buf);
        
        // build envelope with MAX window filtering
        float *ampenv = (float *)bach_newptr(framecount * sizeof(float));
        float *temp = (float *)bach_newptr(framecount * sizeof(float));

        for (long f = 0; f < framecount; f++) {
            float max_amp = 0;
            for (long c = 0; c < channelcount; c++)
                max_amp = MAX(max_amp, fabs(sample[f * channelcount + c]));
            temp[f] = max_amp;
        }

        // backwards MAX window: this is a slow implementation, there are niftier ways
        for (long f = 0; f < framecount; f++) {
            float max_amp = 0;
            for (int i = 0; i < smoothingwin_samps && f-i >= 0; i++) {
                max_amp = MAX(max_amp, temp[f-i]);
            }
            ampenv[f] = max_amp;
        }

        // finding split points
        llll_appendlong(*samp_start, 0);
        for (long f = 0; f < framecount; f++) {
            double amp = ampenv[f+lookahead_samps];
            if (f == 0) {
                for (long i = 0; i < lookahead_samps; i++)
                    amp = MAX(amp, ampenv[i]);
            }
            
            if (!in_attack) {
                if (amp > attack_thresh_linear) {
                    // start an attack here.
                    if (f > 0) {
                        llll_appendlong(*samp_end, f);
                        llll_appendlong(*samp_start, f);
                    } else {
                        first_samp_is_attack = true;
                    }
                    in_attack = true;
                } else {
                    // nothing happens, continue
                }
            } else {
                if (amp < release_thresh_linear) {
                    // attack ends
                    in_attack = false;
                }
            }
        }
        
        while ((*samp_end)->l_size < (*samp_start)->l_size)
            llll_appendlong(*samp_end, framecount);
        
        if (keep_first == 0 && !first_samp_is_attack && (*samp_start)->l_size > 0 && (*samp_end)->l_size > 0) {
            llll_destroyelem((*samp_start)->l_head);
            llll_destroyelem((*samp_end)->l_head);
        }
        
        bach_freeptr(ampenv);
        bach_freeptr(temp);
        buffer_unlocksamples(buf);
    }
    
    return err;
}


std::vector<float> ears_buffer_get_sample_vector_channel(t_object *ob, t_buffer_obj *buf, long channelnum)
{
    std::vector<float> res;
    if (!buf) {
        return res;
    }
    
    t_ears_err err = EARS_ERR_NONE;
    float *sample = buffer_locksamples(buf);
    
    if (!sample) {
        err = EARS_ERR_CANT_READ;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
        return res;
    } else {
        t_atom_long    channelcount = buffer_getchannelcount(buf);
        t_atom_long    framecount   = buffer_getframecount(buf);
        
        for (long f = 0; f < framecount; f++) {
            res.push_back(sample[f * channelcount + channelnum]);
        }
        buffer_unlocksamples(buf);
    }
    
    return res;
}

std::vector<std::vector<float>> ears_buffer_get_sample_vector(t_object *ob, t_buffer_obj *buf)
{
    std::vector<std::vector<float>> res;
    if (!buf) {
        return res;
    }
    
    long numchans = ears_buffer_get_numchannels(ob, buf);
    for (long i = 0; i < numchans; i++)
        res.push_back(ears_buffer_get_sample_vector_channel(ob, buf, i));
    return res;
}


std::vector<float> ears_buffer_get_sample_vector_mono(t_object *ob, t_buffer_obj *buf)
{
    std::vector<float> res;
    if (!buf) {
        return res;
    }
    
    t_ears_err err = EARS_ERR_NONE;
    float *sample = buffer_locksamples(buf);
    
    if (!sample) {
        err = EARS_ERR_CANT_READ;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
        return res;
    } else {
        t_atom_long    channelcount = buffer_getchannelcount(buf);
        t_atom_long    framecount   = buffer_getframecount(buf);
        
        for (long f = 0; f < framecount; f++) {
            double sum = 0;
            for (long c = 0; c < channelcount; c++)
                sum += sample[f * channelcount + c];
            res.push_back(sum/channelcount);
        }
        buffer_unlocksamples(buf);
    }
    
    return res;
}


t_ears_spectralbuf_metadata spectralbuf_metadata_get_empty(){
    t_ears_spectralbuf_metadata data;
    data.original_audio_signal_sr = 0;
    data.binsize = 0;
    data.binoffset = 0;
    data.bins = NULL;
    data.binunit = EARS_FREQUNIT_UNKNOWN;
    data.type = _llllobj_sym_none;
    return data;
}

void ears_spectralbuf_metadata_fill(t_ears_spectralbuf_metadata *data, double original_audio_signal_sr, double binsize, double binoffset, e_ears_frequnit binunit, t_symbol *type, t_llll *bins, bool also_free_bins)
{
    data->original_audio_signal_sr = original_audio_signal_sr;
    data->binsize = binsize;
    data->binoffset = binoffset;
    data->binunit = binunit;
    data->type = type;
    if (also_free_bins)
        llll_free(data->bins);
    data->bins = bins ? llll_clone(bins) : NULL;
}

t_llll *ears_ezarithmser(double from, double step, long numitems)
{
    t_llll *out = llll_get();
    double curr = from;
    for (long i = 0; i < numitems; i++) {
        llll_appenddouble(out, curr);
        curr += step;
    }
    return out;
}







t_ears_err ears_buffer_compress(t_object *ob, t_buffer_obj *source, t_buffer_obj *sidechain, t_buffer_obj *dest,
                                double threshold_dB, double ratio, double kneewidth_dB,
                                double attack_time_samps, double release_time_samps,
                                double makeup_dB)
{
    if (!source || !dest)
        return EARS_ERR_NO_BUFFER;
    
    t_ears_err err = EARS_ERR_NONE;
    double half_kneewidth_dB = kneewidth_dB / 2.;
    float *source_sample = buffer_locksamples(source);
    bool inplace = false, separate_sidechain = false;
    
    if (!source_sample) {
        err = EARS_ERR_CANT_READ;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
    } else {
        
        t_atom_long    source_channelcount = buffer_getchannelcount(source);
        t_atom_long    source_framecount   = buffer_getframecount(source);
        t_atom_long    sidechain_channelcount, sidechain_framecount;

        float *dest_sample = NULL;
        if (source != dest) {
            inplace = false;
            ears_buffer_copy_format(ob, source, dest);
            ears_buffer_set_size_samps(ob, dest, source_framecount);
            dest_sample = buffer_locksamples(dest);
        } else {
            inplace = true;
            dest_sample = source_sample;
        }
        
        float *sidechain_sample;
        if (sidechain == source || !sidechain) {
            separate_sidechain = false;
            sidechain_sample = source_sample;
            sidechain_channelcount = source_channelcount;
            sidechain_framecount = source_framecount;
        } else {
            separate_sidechain = true;
            sidechain_channelcount = buffer_getchannelcount(sidechain);
            sidechain_framecount = buffer_getframecount(sidechain);
            sidechain_sample = buffer_locksamples(sidechain);
        }
        
        if (!dest_sample) {
            err = EARS_ERR_CANT_READ;
            object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
        } else {
            t_atom_long    dest_channelcount = buffer_getchannelcount(dest);
            t_atom_long    dest_framecount   = buffer_getframecount(dest);
            t_atom_long    max_framecount = MAX(dest_framecount, source_framecount);

            double yL = 0, yL_prev = 0;
            for (long i = 0; i < max_framecount; i++) {

                // we use the page https://github.com/p-hlp/CTAGDRC as a tutorial
                
                // 1) level detection: it's the maximum across the channel of the sidechain signal
                float input = 0;
                if (i < sidechain_framecount) {
                    for (long c = 0; c < sidechain_channelcount; c++)
                        input = MAX(input, std::abs(sidechain_sample[i * sidechain_channelcount + c]));
                }
                
                // 2) dB conversion
                float input_db = ears_linear_to_db(input);
                float output_gain_db;

                // 3) gain computation
                if (input_db - threshold_dB < -half_kneewidth_dB) {
                    // nothing to do: copy
                    output_gain_db = 0;
                } else if (input_db - threshold_dB <= half_kneewidth_dB) {
                    float temp = input_db - threshold_dB + half_kneewidth_dB;
                    output_gain_db = (1./ratio - 1) * temp * temp / (2 * kneewidth_dB);
                } else {
                    output_gain_db = threshold_dB + (input_db-threshold_dB)/ratio - input_db;
                }
                
                // 4) ballistics
                double alpha_attack = exp(-1./attack_time_samps);
                double alpha_release = exp(-1./release_time_samps);
                
                // smooth branching peek detector
                if (output_gain_db > yL_prev) {
                    yL = alpha_attack * yL_prev + (1 - alpha_attack) * output_gain_db;
                } else {
                    yL = alpha_release * yL_prev + (1 - alpha_release) * output_gain_db;
                }
                
                yL_prev = yL;
                
                // 5) linear conversion
                double gain_to_apply_linear = ears_db_to_linear(yL + makeup_dB);
                
                for (long c = 0; c < dest_channelcount; c++)
                    dest_sample[i * dest_channelcount + c] = source_sample[i * dest_channelcount + c] * gain_to_apply_linear;
            }
            if (!inplace)
                buffer_unlocksamples(dest);
        }
        
        if (separate_sidechain)
            buffer_unlocksamples(sidechain);
        
        buffer_unlocksamples(source);
    }
    
    return err;
}



