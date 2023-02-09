#include "ears.freeverb_commons.h"
#include "ears.commons.h"

#define EARS_FREEVERB_INITIAL_DEFAULT_TAIL_PAD_SEC 10

// IMPORTANT: the model should ALREADY be initialized ot the number of channels of the source
t_ears_err ears_buffer_freeverb(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, revmodel *model, long tail_samps)
{
    double sr = ears_buffer_get_sr(ob, source);

    model->mute();
    
    if (!source || !dest)
        return EARS_ERR_NO_BUFFER;
    
    t_ears_err err = EARS_ERR_NONE;
    float *orig_sample = ears_buffer_locksamples(source);
    float *orig_sample_wk = NULL;
    
    if (!orig_sample) {
        err = EARS_ERR_CANT_READ;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
    } else {
        t_atom_long    channelcount = buffer_getchannelcount(source);        // number of floats in a frame
        t_atom_long    framecount   = buffer_getframecount(source);            // number of floats long the buffer is for a single channel
        t_atom_long    framepad = (tail_samps >= 0 ? tail_samps : sr * EARS_FREEVERB_INITIAL_DEFAULT_TAIL_PAD_SEC); //  padding due to reverb tail
        
        orig_sample_wk = (float *)bach_newptrclear(channelcount * (framecount + framepad) * sizeof(float));
        sysmem_copyptr(orig_sample, orig_sample_wk, channelcount * framecount * sizeof(float));
        ears_buffer_unlocksamples(source);
        
        if (source == dest) {
            ears_buffer_set_size_samps(ob, dest, framecount + framepad);
        } else {
            ears_buffer_copy_format_and_set_size_samps(ob, source, dest, framecount + framepad);
        }
        
        float *dest_sample = ears_buffer_locksamples(dest);
        
        if (!dest_sample) {
            err = EARS_ERR_CANT_WRITE;
            object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
        } else {
            
            long end_sample = -1;
            
            model->processreplace(orig_sample_wk, dest_sample, framecount+framepad);

            if (tail_samps < 0) {
                // automatic tail trimming:
                // checking where the tail ACTUALLY ends
                long s = framecount+framepad - 1;
                while (s >= framecount) {
                    double maxsamp = 0.;
                    for (long c = 0; c < channelcount; c++)
                        maxsamp = MAX(maxsamp, dest_sample[channelcount*s + c]);
                    if (maxsamp > 0.) {
                        break;
                    } else {
                        s--;
                    }
                }
                end_sample = s + 1;
            }

            ears_buffer_unlocksamples(dest);
            if (end_sample > 0)
                ears_buffer_crop_inplace(ob, dest, 0, end_sample);
//                ears_buffer_set_size_samps(ob, dest, end_sample);
            buffer_setdirty(dest);
        }
        
        bach_freeptr(orig_sample_wk);
    }
    
    return err;
}



// IMPORTANT: the model should ALREADY be initialized ot the number of channels of the source
t_ears_err ears_buffer_freeverb_envelope(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, revmodel *model, long tail_samps, t_llll *dry_env, t_llll *wet_env, double dry_default, double wet_default, e_slope_mapping slopemapping)
{
    double sr = ears_buffer_get_sr(ob, source);
    
    model->mute();
    
    if (!source || !dest)
        return EARS_ERR_NO_BUFFER;
    
    t_ears_err err = EARS_ERR_NONE;
    float *orig_sample = ears_buffer_locksamples(source);
    float *orig_sample_wk = NULL;
    
    if (!orig_sample) {
        err = EARS_ERR_CANT_READ;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
    } else {
        t_atom_long    channelcount = buffer_getchannelcount(source);        // number of floats in a frame
        t_atom_long    framecount   = buffer_getframecount(source);            // number of floats long the buffer is for a single channel
        t_atom_long    framepad = (tail_samps >= 0 ? tail_samps : sr * EARS_FREEVERB_INITIAL_DEFAULT_TAIL_PAD_SEC); //  padding due to reverb tail
        
        float *dry = (float *)bach_newptrclear((framecount + framepad) * sizeof(float));
        float *wet = (float *)bach_newptrclear((framecount + framepad) * sizeof(float));
        
        // building envelopes
        t_ears_envelope_iterator eei_dry = ears_envelope_iterator_create(dry_env, dry_default, false, slopemapping);
        t_ears_envelope_iterator eei_wet = ears_envelope_iterator_create(wet_env, wet_default, false, slopemapping);
        for (long i = 0; i < framecount; i++) {
            dry[i] = ears_envelope_iterator_walk_interp(&eei_dry, i, framecount);
            wet[i] = ears_envelope_iterator_walk_interp(&eei_wet, i, framecount);
        }
        for (long i = framecount; i < framecount + framepad; i++) {
            dry[i] = dry[framecount-1];
            wet[i] = wet[framecount-1];
        }

        orig_sample_wk = (float *)bach_newptrclear(channelcount * (framecount + framepad) * sizeof(float));
        sysmem_copyptr(orig_sample, orig_sample_wk, channelcount * framecount * sizeof(float));
        ears_buffer_unlocksamples(source);
        
        if (source == dest) {
            ears_buffer_set_size_samps(ob, dest, framecount + framepad);
        } else {
            ears_buffer_copy_format_and_set_size_samps(ob, source, dest, framecount + framepad);
        }
        
        float *dest_sample = ears_buffer_locksamples(dest);
        
        if (!dest_sample) {
            err = EARS_ERR_CANT_WRITE;
            object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
        } else {
            
            long end_sample = -1;
            
            model->processreplace_envelopes(orig_sample_wk, dest_sample, framecount+framepad, dry, wet);
            
            if (tail_samps < 0) {
                // automatic tail trimming:
                // checking where the tail ACTUALLY ends
                long s = framecount+framepad - 1;
                while (s >= framecount) {
                    double maxsamp = 0.;
                    for (long c = 0; c < channelcount; c++)
                        maxsamp = MAX(maxsamp, dest_sample[channelcount*s + c]);
                    if (maxsamp > 0.) {
                        break;
                    } else {
                        s--;
                    }
                }
                end_sample = s + 1;
            }
            
            ears_buffer_unlocksamples(dest);
            if (end_sample > 0)
                ears_buffer_crop_inplace(ob, dest, 0, end_sample);
            //                ears_buffer_set_size_samps(ob, dest, end_sample);
            buffer_setdirty(dest);
        }
        
        bach_freeptr(orig_sample_wk);
        bach_freeptr(dry);
        bach_freeptr(wet);
    }
    
    return err;
}



