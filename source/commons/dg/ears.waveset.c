#include "ears.waveset.h"

t_ears_err ears_buffer_waveset_getnum(t_object *ob, t_buffer_obj *source, long channel, long span, long *num)
{
    if (!source)
        return EARS_ERR_NO_BUFFER;
    
    long count_waveset = 0;
    t_ears_err err = EARS_ERR_NONE;
    float *orig_sample = buffer_locksamples(source);
    if (!orig_sample) {
        err = EARS_ERR_CANT_READ;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
    } else {
        t_atom_long    channelcount = buffer_getchannelcount(source);
        t_atom_long    framecount   = buffer_getframecount(source);
        
        if (channel >= 0 && channel < channelcount) {
            long c = channel;
            long pivot = 0;
            long span_count = 0;
            double max_abs = 0.;
            for (long f = 1; f < framecount; f++) {
                if (orig_sample[(f-1)*channelcount+c] < 0 && orig_sample[f*channelcount+c] >= 0) { // positive zero crossing
                    span_count++;
                    if (span_count % span == 0) {
                        // zero crossing found at index f
                        count_waveset++;
                        pivot = f;
                        max_abs = 0.;
                    }
                } else {
                    max_abs = MAX(max_abs, abs(orig_sample[(f-1)*channelcount+c]));
                }
            }
            // last portion
            if (pivot != framecount)
                count_waveset++;
        }
    }
    
    *num = count_waveset;
    return err;
}



t_ears_err ears_buffer_waveset_repeat(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, long howmany, long span, bool normalize)
{
    if (!source || !dest)
        return EARS_ERR_NO_BUFFER;
    
    if (howmany <= 0)
        return EARS_ERR_GENERIC;

    t_ears_err err = EARS_ERR_NONE;
    float *orig_sample = buffer_locksamples(source);
    float *orig_sample_wk = NULL;
    if (!orig_sample) {
        err = EARS_ERR_CANT_READ;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
    } else {
        t_atom_long    channelcount = buffer_getchannelcount(source);
        t_atom_long    framecount   = buffer_getframecount(source);
        
        
        if (source == dest) { // inplace operation!
            orig_sample_wk = (float *)bach_newptr(channelcount * framecount * sizeof(float));
            sysmem_copyptr(orig_sample, orig_sample_wk, channelcount * framecount * sizeof(float));
            buffer_unlocksamples(source);
        } else {
            orig_sample_wk = orig_sample;
            ears_buffer_copy_format(ob, source, dest, true); // won't change channels/framecount: that'll be done later on
        }
        
        ears_buffer_set_size_and_numchannels(ob, dest, howmany * framecount, channelcount);
        
        float *dest_sample = buffer_locksamples(dest);
        
        if (!dest_sample) {
            err = EARS_ERR_CANT_WRITE;
            object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
        } else {
            
            t_atom_long    dest_channelcount = buffer_getchannelcount(dest);
            t_atom_long    dest_framecount   = buffer_getframecount(dest);
            
            if (dest_channelcount != channelcount) { // should never happen
                channelcount = MIN(dest_channelcount, channelcount);
                dest_channelcount = channelcount;
            }

            for (long c = 0; c < channelcount; c++) {
                long pivot = 0;
                long span_count = 0;
                long dest_cur = 0;
                double max_abs = 0.;
                for (long f = 1; f < framecount; f++) {
                    if (orig_sample[(f-1)*channelcount+c] < 0 && orig_sample[f*channelcount+c] >= 0) { // positive zero crossing
                        span_count++;
                        if (span_count % span == 0) {
                            // zero crossing found at index f
                            for (long n = 0; n < howmany; n++) {
                                for (long j = pivot; j < f; j++) {
                                    if (dest_cur >= dest_framecount) {
                                        object_post(ob, "Internal error!");
                                    } else {
                                        if (normalize == 0 || max_abs == 0)
                                            dest_sample[(dest_cur++) * dest_channelcount + c] = orig_sample[j * channelcount + c];
                                        else // could be optimized
                                            dest_sample[(dest_cur++) * dest_channelcount + c] = orig_sample[j * channelcount + c]*(1./max_abs);
//                                            dest_sample[(dest_cur++) * dest_channelcount + c] = (1-normalize)*orig_sample[j * channelcount + c] + normalize*orig_sample[j * channelcount + c]*(1./max_abs);
                                    }
                                }
                            }
                            pivot = f;
                            max_abs = abs(orig_sample[f*channelcount+c]);
                        } else {
                            max_abs = MAX(max_abs, abs(orig_sample[(f-1)*channelcount+c]));
                        }
                    } else {
                        max_abs = MAX(max_abs, abs(orig_sample[(f-1)*channelcount+c]));
                    }
                }
                // last portion
                max_abs = MAX(max_abs, abs(orig_sample[(framecount-1)*channelcount+c]));
                for (long n = 0; n < howmany; n++) {
                    for (long j = pivot; j < framecount; j++) {
                        if (dest_cur >= dest_framecount) {
                            long foo = 0;
                            foo++;
                        } else {
                            if (normalize == 0 || max_abs == 0)
                                dest_sample[(dest_cur++) * dest_channelcount + c] = orig_sample[j * channelcount + c];
                            else // could be optimized
                                dest_sample[(dest_cur++) * dest_channelcount + c] = (1-normalize)*orig_sample[j * channelcount + c] + normalize*orig_sample[j * channelcount + c]*(1./max_abs);
                        }
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



t_ears_err ears_buffer_waveset_subs(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, t_buffer_obj *waveform, long span, long resampling_filter_size)
{
    if (!source || !dest)
        return EARS_ERR_NO_BUFFER;
    
    t_ears_err err = EARS_ERR_NONE;

    float *waveform_sample = buffer_locksamples(waveform);
    if (!waveform_sample) {
        object_error(ob, "No waveform defined!");
        return EARS_ERR_NO_BUFFER;
    }
    
    long waveform_len = ears_buffer_get_size_samps(ob, waveform);
    if (!waveform_sample) {
        object_error(ob, "Waveform is empty!");
        buffer_unlocksamples(waveform);
        return EARS_ERR_NO_BUFFER;
    }
    if (ears_buffer_get_numchannels(ob, waveform) > 1) {
        object_error(ob, "Waveform must have a single channel!");
        buffer_unlocksamples(waveform);
        return EARS_ERR_GENERIC;
    }
    

    double sr = ears_buffer_get_sr(ob, source);
    float *orig_sample = buffer_locksamples(source);
    float *orig_sample_wk = NULL;
    if (!orig_sample) {
        err = EARS_ERR_CANT_READ;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
    } else {
        t_atom_long    channelcount = buffer_getchannelcount(source);
        t_atom_long    framecount   = buffer_getframecount(source);
        
        if (source == dest) { // inplace operation!
            orig_sample_wk = (float *)bach_newptr(channelcount * framecount * sizeof(float));
            sysmem_copyptr(orig_sample, orig_sample_wk, channelcount * framecount * sizeof(float));
            buffer_unlocksamples(source);
        } else {
            orig_sample_wk = orig_sample;
            ears_buffer_copy_format(ob, source, dest, true); // won't change channels/framecount: that'll be done later on
        }
        
        ears_buffer_set_size_and_numchannels(ob, dest, framecount, channelcount);
        
        float *dest_sample = buffer_locksamples(dest);
        float *temp = (float *)bach_newptr(framecount * sizeof(float));
        
        if (!dest_sample) {
            err = EARS_ERR_CANT_WRITE;
            object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
        } else {
            
            t_atom_long    dest_channelcount = buffer_getchannelcount(dest);
            t_atom_long    dest_framecount   = buffer_getframecount(dest);
            
            if (dest_channelcount != channelcount) { // should never happen
                channelcount = MIN(dest_channelcount, channelcount);
                dest_channelcount = channelcount;
            }
            if (dest_framecount != framecount) { // should never happen
                framecount = MIN(dest_framecount, framecount);
                dest_framecount = framecount;
            }

            for (long c = 0; c < channelcount; c++) {
                long pivot = 0;
                long span_count = 0;
                double max_abs = 0.;
                for (long f = 1; f < framecount; f++) {
                    if (orig_sample[(f-1)*channelcount+c] < 0 && orig_sample[f*channelcount+c] >= 0) { // positive zero crossing
                        span_count++;
                        if (span_count % span == 0) {
                            // zero crossing found at index f
                            ears_resample(waveform_sample, waveform_len, &temp, framecount, ((double)(f-pivot))/waveform_len, sr/2., sr, resampling_filter_size, 1);
                            for (long j = pivot; j < f; j++) {
                                dest_sample[j*dest_channelcount + c] = temp[(j-pivot) * channelcount + c] * max_abs;
                            }
                            pivot = f;
                            max_abs = abs(orig_sample[f*channelcount+c]);
                        } else {
                            max_abs = MAX(max_abs, abs(orig_sample[(f-1)*channelcount+c]));
                        }
                    } else {
                        max_abs = MAX(max_abs, abs(orig_sample[(f-1)*channelcount+c]));
                    }
                }
                // last portion
                max_abs = MAX(max_abs, abs(orig_sample[(framecount-1)*channelcount+c]));
                if (pivot < framecount) {
                    ears_resample(waveform_sample, waveform_len, &temp, framecount, ((double)(framecount-pivot))/waveform_len, sr/2., sr, resampling_filter_size, 1);
                    for (long j = pivot; j < framecount; j++) {
                        dest_sample[j*dest_channelcount + c] = temp[(j-pivot) * channelcount + c] * max_abs;
                    }
                }
            }
            
            buffer_setdirty(dest);
            buffer_unlocksamples(dest);
        }
        bach_freeptr(temp);
    
        if (source == dest) // inplace operation!
            bach_freeptr(orig_sample_wk);
        else
            buffer_unlocksamples(source);
    }
    
    buffer_unlocksamples(waveform);
    
    return err;
}





t_ears_err ears_buffer_waveset_split(t_object *ob, t_buffer_obj *source, t_buffer_obj **dest, long channel, long dest_size, long span, bool normalize)
{
    if (!source || !dest || dest_size <= 0)
        return EARS_ERR_NO_BUFFER;

    t_ears_err err = EARS_ERR_NONE;
    float *orig_sample = buffer_locksamples(source);
    if (!orig_sample) {
        err = EARS_ERR_CANT_READ;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
    } else {
        t_atom_long    channelcount = buffer_getchannelcount(source);
        t_atom_long    framecount   = buffer_getframecount(source);
        
        if (channel >= 0 && channel < channelcount) {
            long count = 0;
            long c = channel;
            long pivot = 0;
            long span_count = 0;
            for (long f = 1; f < framecount && count < dest_size; f++) {
                if (orig_sample[(f-1)*channelcount+c] < 0 && orig_sample[f*channelcount+c] >= 0) { // positive zero crossing
                    span_count++;
                    if (span_count % span == 0) {
                        // zero crossing found at index f
                        ears_buffer_crop(ob, source, dest[count], pivot, f);
                        if (normalize > 0) {
                            ears_buffer_normalize_inplace(ob, dest[count], 1.);
                        }
                        pivot = f;
                        count++;
                    }
                }
            }
            // last portion
            if (count < dest_size && pivot < framecount) {
                ears_buffer_crop(ob, source, dest[count], pivot, framecount);
                count++;
            }
        }
    }
    
    return err;
}




t_ears_err ears_buffer_waveset_shuffle(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, long span, long group)
{
    t_ears_err err = EARS_ERR_NONE;
    long numframes = ears_buffer_get_size_samps(ob, source);

    if (!source || !dest || numframes <= 0)
        return EARS_ERR_NO_BUFFER;
    
    if (group == 1) {
        ears_buffer_clone(ob, source, dest);
        return EARS_ERR_NONE;
    }
    
    t_buffer_obj *tempbuf = ears_buffer_make(NULL);
    
    ears_buffer_copy_format_and_set_size_samps(ob, source, dest, numframes);
    
    long channelnum = ears_buffer_get_numchannels(ob, source);
    for (long c = 0; c < channelnum; c++) {
        long num = 0;
        ears_buffer_waveset_getnum(ob, source, c, span, &num);
        if (num > 0) {
            t_buffer_obj **wsts = (t_buffer_obj **)bach_newptr(num * sizeof(t_buffer_obj *));
            for (long i = 0; i < num; i++) {
                wsts[i] = ears_buffer_make(NULL);
            }

            err = ears_buffer_waveset_split(ob, source, wsts, c, num, span, false);

            t_llll *wsts_ll = llll_get();
            for (long i = 0; i < num; i++)
                llll_appendobj(wsts_ll, wsts[i]);
            llll_groupllll(wsts_ll, group, 0);
            for (t_llllelem *el = wsts_ll->l_head; el; el = el->l_next) {
                if (hatom_gettype(&el->l_hatom) == H_LLLL) {
                    t_llll *ll = hatom_getllll(&el->l_hatom);
                    llll_scramble(ll, 0, 0);
                }
            }
            llll_flatten(wsts_ll, 0, 0);
            
            if (wsts_ll->l_size != num) {
                object_error(ob, "Internal error!");
            }
            long i = 0;
            for (t_llllelem *el = wsts_ll->l_head; el && i < num; el = el->l_next, i++) {
                wsts[i] = (t_buffer_obj *)hatom_getobj(&el->l_hatom);
            }
            
            ears_buffer_join(ob, wsts, num, tempbuf, NULL, false, EARS_FADE_NONE, 0, k_SLOPE_MAPPING_BACH, EARS_RESAMPLINGPOLICY_DONT, 3);
            
            ears_buffer_copychannel(ob, tempbuf, 0, dest, c);
            
            for (long i = 0; i < num; i++)
                ears_buffer_free(wsts[i]);

            bach_freeptr(wsts);
        } else {
            ears_buffer_clearchannel(ob, dest, c);
        }
    }
    
    ears_buffer_free(tempbuf);
    
    return err;
}

