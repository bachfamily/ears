#include "ears.soundtouch_commons.h"
#include "ears.commons.h"

using namespace soundtouch;

t_ears_err ears_buffer_soundtouch(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, double stretch_factor, double pitchshift_semitones, int quickSeek, int noAntiAlias, bool speech)
{
    t_ears_err err = EARS_ERR_NONE;
    
    int nChannels = (int)ears_buffer_get_numchannels(ob, source);
    int sr = (int)ears_buffer_get_sr(ob, source);
    long nSamples = (long)ears_buffer_get_size_samps(ob, source);
    
    if (nChannels == 0) {
        object_error(ob, "Input buffer has no channels");
        ears_buffer_clone(ob, source, dest);
        return EARS_ERR_GENERIC;
    }
    
    if (nSamples == 0) {
        // empty buffer
        ears_buffer_clone(ob, source, dest);
        return EARS_ERR_NONE;
    }
    
    if (stretch_factor <= 0) {
        // this is unsupported by soundtouch
        object_warn(ob, "Stretch factor must be positive. Aborting.");
        ears_buffer_clone(ob, source, dest);
        return EARS_ERR_NONE;
    }

    SoundTouch pSoundTouch;
    pSoundTouch.setSampleRate(sr);
    pSoundTouch.setChannels(1); // we handle the number of channels ourselves, because of the limitations of the library
                                // we parallelize the operation ourselves, allowing for arbitrary number of channels

    pSoundTouch.setTempo(1./stretch_factor);
    pSoundTouch.setPitchSemiTones(pitchshift_semitones);
//    pSoundTouch->setRateChange(params->rateDelta);

    pSoundTouch.setSetting(SETTING_USE_QUICKSEEK, quickSeek);
    pSoundTouch.setSetting(SETTING_USE_AA_FILTER, !(noAntiAlias));

    if (speech)
    {
        // use settings for speech processing
        pSoundTouch.setSetting(SETTING_SEQUENCE_MS, 40);
        pSoundTouch.setSetting(SETTING_SEEKWINDOW_MS, 15);
        pSoundTouch.setSetting(SETTING_OVERLAP_MS, 8);
    }
    
    // processing

    int buffSizeSamples;
    int SAMPLE_BLOCK_SIZE = 2048;
    
    
    float *orig_sample = buffer_locksamples(source);
    float *orig_sample_channel = NULL, *processed_sample = NULL, *processed_sample_channel_wk;

    if (!orig_sample) {
        err = EARS_ERR_CANT_READ;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
    } else {
        t_atom_long    channelcount = buffer_getchannelcount(source);       // number of floats in a frame
        t_atom_long    framecount   = buffer_getframecount(source);         // number of floats long the buffer is for a single channel
        
        orig_sample_channel = (float *)bach_newptrclear(framecount * sizeof(float));

        long processed_alloc_frames = framecount;
        processed_sample = (float *)bach_newptrclear(processed_alloc_frames * channelcount * sizeof(float));
        processed_sample_channel_wk = (float *)bach_newptr(processed_alloc_frames * sizeof(float));

        long outframecount = 0;
        
        for (long c = 0; c < channelcount; c++) {
            // we do our own iteration on the channels, so that we overcome the issues of MAX CHANNELS in SoundTouch

            pSoundTouch.clear(); // start from scratch
            
            // Process samples read from the input file
            long processed_channel_cur = 0;
            int nReceivedSamples = 0;

            memset(processed_sample_channel_wk, 0, processed_alloc_frames * sizeof(float));
            
            // filling the channel
            for (long f = 0; f < framecount; f++)
                orig_sample_channel[f] = orig_sample[f * channelcount + c];

            
            for (long f = 0; f < framecount; f += SAMPLE_BLOCK_SIZE) {
                
                // Feed the samples into SoundTouch processor
                long numBlockSamples = (f + SAMPLE_BLOCK_SIZE <= framecount) ? SAMPLE_BLOCK_SIZE : framecount - f;

                pSoundTouch.putSamples(orig_sample_channel + f, numBlockSamples);
                
                // Read ready samples from SoundTouch processor & write them output file.
                // NOTES:
                // - 'receiveSamples' doesn't necessarily return any samples at all
                //   during some rounds!
                // - On the other hand, during some round 'receiveSamples' may have more
                //   ready samples than would fit into 'sampleBuffer', and for this reason
                //   the 'receiveSamples' call is iterated for as many times as it
                //   outputs samples.
                do {
                    // check allocation
                    while (processed_channel_cur + SAMPLE_BLOCK_SIZE > processed_alloc_frames) {
                        // gotta re-allocate more
                        processed_alloc_frames += framecount;
                        processed_sample = (float *)bach_resizeptr(processed_sample, processed_alloc_frames * channelcount * sizeof(float));
                        processed_sample_channel_wk = (float *)bach_resizeptr(processed_sample_channel_wk, processed_alloc_frames * sizeof(float));
                    }
                    
                    nReceivedSamples = pSoundTouch.receiveSamples(processed_sample_channel_wk+processed_channel_cur, SAMPLE_BLOCK_SIZE);
                    processed_channel_cur += nReceivedSamples;
                } while (nReceivedSamples != 0);
            }
            
            // Now the input file is processed, yet 'flush' few last samples that are
            // hiding in the SoundTouch's internal processing pipeline.
            pSoundTouch.flush();
            do
            {
                // check allocation
                while (processed_channel_cur + SAMPLE_BLOCK_SIZE > processed_alloc_frames) {
                    // gotta re-allocate more
                    processed_alloc_frames += framecount;
                    processed_sample = (float *)bach_resizeptr(processed_sample, processed_alloc_frames * channelcount * sizeof(float));
                    processed_sample_channel_wk = (float *)bach_resizeptr(processed_sample_channel_wk, processed_alloc_frames * sizeof(float));
                }
                
                nReceivedSamples = pSoundTouch.receiveSamples(processed_sample_channel_wk+processed_channel_cur, SAMPLE_BLOCK_SIZE);
                processed_channel_cur += nReceivedSamples;
            } while (nReceivedSamples != 0);

            if (processed_channel_cur > outframecount)
                outframecount = processed_channel_cur;
            
            // copying data
            for (long f = 0; f < processed_channel_cur; f++)
                processed_sample[f * channelcount + c] = processed_sample_channel_wk[f];
        }
        
        buffer_unlocksamples(source);
        
        if (source == dest) { // inplace operation!
            ears_buffer_set_size_samps(ob, source, outframecount);
        } else {
            ears_buffer_copy_format_and_set_size_samps(ob, source, dest, outframecount);
        }
        
        float *dest_sample = buffer_locksamples(dest);
        
        if (!dest_sample) {
            err = EARS_ERR_CANT_WRITE;
            object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
        } else {
            t_atom_long    channelcount = buffer_getchannelcount(dest);
            t_atom_long    framecount   = buffer_getframecount(dest);
            if (outframecount > framecount)
                outframecount = framecount;
            
            sysmem_copyptr(processed_sample, dest_sample, channelcount * outframecount * sizeof(float));
            
            buffer_setdirty(dest);
            buffer_unlocksamples(dest);
        }
        
        bach_freeptr(processed_sample);
        bach_freeptr(processed_sample_channel_wk);
        bach_freeptr(orig_sample_channel);
    }
    return err;
}



