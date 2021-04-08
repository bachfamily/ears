#include "ears.rubberband_commons.h"
#include "ears.commons.h"

#define RUBBERBAND_MAX_RATIO 128
using namespace RubberBand;

RubberBandStretcher::Options buf_rubberband_get_default_options()
{
    RubberBand::RubberBandStretcher::Options options = 0;
    options |= RubberBand::RubberBandStretcher::OptionPitchHighConsistency;
    options |= RubberBand::RubberBandStretcher::OptionStretchElastic;
    options |= RubberBand::RubberBandStretcher::OptionTransientsSmooth;
    options |= RubberBand::RubberBandStretcher::OptionDetectorCompound;
    options |= RubberBand::RubberBandStretcher::OptionPhaseLaminar;
    options |= RubberBand::RubberBandStretcher::OptionFormantShifted;
    options |= RubberBand::RubberBandStretcher::OptionThreadingNever;
    options |= RubberBand::RubberBandStretcher::OptionWindowStandard;
    return options;
}


t_ears_err ears_buffer_rubberband(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, t_llll *timestretch_factor, t_llll *pitchshift_factor, RubberBandStretcher::Options options, long blocksize_samps, e_slope_mapping slopemapping)
{
    int mask = (RubberBand::RubberBandStretcher::OptionPitchHighQuality |
                RubberBand::RubberBandStretcher::OptionPitchHighSpeed |
                RubberBand::RubberBandStretcher::OptionPitchHighConsistency);

    if (!source || !dest)
        return EARS_ERR_NO_BUFFER;
    
    if (!timestretch_factor || !pitchshift_factor) {
        ears_buffer_clone(ob, source, dest);
        return EARS_ERR_GENERIC;
    }
    
    double sr = ears_buffer_get_sr(ob, source);
    
    char ts_is_static = !(timestretch_factor->l_size == 1 && timestretch_factor->l_depth == 1);
    char ps_is_static = !(pitchshift_factor->l_size == 1 && pitchshift_factor->l_depth == 1);
    char all_static = ts_is_static && ps_is_static;
    
    if (all_static)
        options |= RubberBand::RubberBandStretcher::OptionProcessOffline;
    else
        options |= RubberBand::RubberBandStretcher::OptionProcessRealTime;

    t_ears_envelope_iterator ts_eei = ears_envelope_iterator_create(timestretch_factor, 0., false, slopemapping);
    t_ears_envelope_iterator ps_eei = ears_envelope_iterator_create(pitchshift_factor, 0., false, slopemapping);

//    double frequencyshift = 1.;
//    if (pitchshift_cents != 0.0)
//        frequencyshift *= pow(2.0, pitchshift_cents / 1200);
    
    t_ears_err err = EARS_ERR_NONE;

    float *orig_sample = buffer_locksamples(source);
    float *orig_sample_wk = NULL;
    
    if (!orig_sample) {
        err = EARS_ERR_CANT_READ;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
    } else {
        t_atom_long    channelcount = buffer_getchannelcount(source);        // number of floats in a frame
        t_atom_long    framecount   = buffer_getframecount(source);            // number of floats long the buffer is for a single channel
        
        orig_sample_wk = (float *)bach_newptr(channelcount * framecount * sizeof(float));
        sysmem_copyptr(orig_sample, orig_sample_wk, channelcount * framecount * sizeof(float));
        buffer_unlocksamples(source);
        
        // creating RubberBand stretcher
        RubberBandStretcher rb((long)sr, channelcount, options, ears_envelope_iterator_walk_interp(&ts_eei, 0, framecount), pow(2, ears_envelope_iterator_walk_interp(&ps_eei, 0, framecount)/1200.));
        rb.setExpectedInputDuration(framecount);
        
        // we need to chop the file into blocks for the algo to be studied
        long blocksize = blocksize_samps;
        long numblocks = (long)ceil(((double)framecount) / blocksize);
        float *fbuf = new float[channelcount * blocksize];
        float **ibuf = new float *[channelcount];
        for (size_t i = 0; i < channelcount; ++i)
            ibuf[i] = new float[blocksize];

        rb.setMaxProcessSize(blocksize);
        
        // STUDYING...
        for (long i = 0; i < numblocks; i++) {
            if (!all_static) {
                double new_time_ratio = ears_envelope_iterator_walk_interp(&ts_eei, i*blocksize, framecount);
                double new_pitch_scale = pow(2.0, ears_envelope_iterator_walk_interp(&ps_eei, i*blocksize, framecount)/1200.);
                // gotta ensure that new_time_ratio and new_pitch_scale are within reasonable limits, otherwise
                new_time_ratio = CLAMP(new_time_ratio, 0., RUBBERBAND_MAX_RATIO);
                new_pitch_scale = CLAMP(new_pitch_scale, 0., RUBBERBAND_MAX_RATIO);
                rb.setTimeRatio(new_time_ratio);
                rb.setPitchScale(new_pitch_scale);
            }
            
            long count = ((i+1)*blocksize > framecount ? framecount - i*blocksize : blocksize);
            for (long c = 0; c < channelcount; c++) {
                for (long j = 0; j < count; j++)
                    ibuf[c][j] = orig_sample_wk[(i*blocksize+j)*channelcount + c];
            }
            rb.study(ibuf, count, i==(numblocks-1));
        }
        ears_envelope_iterator_reset(&ts_eei);
        ears_envelope_iterator_reset(&ps_eei);
        

        // PROCESSING...
        long outframecount = 0;
        double old_pitch_scale = -1;
        long output_sample_wk_allocated_frames = framecount;
        float *output_sample_wk = (float *)bach_newptr(channelcount * output_sample_wk_allocated_frames * sizeof(float));
        for (long i = 0; i < numblocks; i++) {
            char pitch_transit_across_1 = false;
            long count = ((i+1)*blocksize > framecount ? framecount - i*blocksize : blocksize);
            for (long c = 0; c < channelcount; c++) {
                for (long j = 0; j < count; j++)
                    ibuf[c][j] = orig_sample_wk[(i*blocksize+j)*channelcount + c];
            }
            
            if (!all_static) {
                double new_time_ratio = ears_envelope_iterator_walk_interp(&ts_eei, i*blocksize, framecount);
                double new_pitch_scale = pow(2.0, ears_envelope_iterator_walk_interp(&ps_eei, i*blocksize, framecount)/1200.);
                // gotta ensure that new_time_ratio and new_pitch_scale are within reasonable limits, otherwise
                new_time_ratio = CLAMP(new_time_ratio, 0., RUBBERBAND_MAX_RATIO);
                new_pitch_scale = CLAMP(new_pitch_scale, 0., RUBBERBAND_MAX_RATIO);
                
                // This handles one particular case in which rubberband fails, ie. when the pitch goes from below to above 0
                // the standard pitch modes have issues and create clicks; we force the consistency
                if (old_pitch_scale > 0. && ((old_pitch_scale < 1 && new_pitch_scale >= 1.) || (old_pitch_scale >= 1. && new_pitch_scale < 1.))){                     pitch_transit_across_1 = true;
                }
                old_pitch_scale = new_pitch_scale;
                
                rb.setTimeRatio(new_time_ratio);
                rb.setPitchScale(new_pitch_scale);
                
            }
            
            rb.process(ibuf, count, i==(numblocks-1));
            
            int avail = rb.available();
            if (avail > 0) { // we've got something as output
                float **obf = new float *[channelcount];
                long pivot = outframecount;
                for (long c = 0; c < channelcount; c++) {
                    obf[c] = new float[avail];
                }
                rb.retrieve(obf, avail);
                outframecount += avail;
                
                while (outframecount > output_sample_wk_allocated_frames) {
                    // gotta re-allocate more
                    output_sample_wk_allocated_frames += framecount;
                    output_sample_wk = (float *)bach_resizeptr(output_sample_wk, channelcount * output_sample_wk_allocated_frames * sizeof(float));
                }
                
                for (long c = 0; c < channelcount; c++) {
                    for (int i = 0; i < avail; i++) {
                        output_sample_wk[(pivot + i)*channelcount + c] = obf[c][i];
                    }
                }
                
                // rubberband doesn't seem to handle passage from negative to positive transposition smoothly, there's a click in here
                // if would be nice to smooth it out but this doesn't seem to work. I am no expert in click removal
                // it would be probably best to fix the rubberband library directly
/*                if (pitch_transit_across_1 && !(options & RubberBand::RubberBandStretcher::OptionPitchHighConsistency)) {
                    const int NUM_SAMPLES_TO_SMOOTH_ACROSS = 20;
                    const int NUM_FILTER_APPLICATIONS = 20;
                    int HALF_NUM_SAMPLES_TO_SMOOTH_ACROSS = NUM_SAMPLES_TO_SMOOTH_ACROSS / 2;
                    for (long c = 0; c < channelcount; c++) {
                        float orig_channel[2 * HALF_NUM_SAMPLES_TO_SMOOTH_ACROSS + 1];
                        for (int r = 0; r < NUM_FILTER_APPLICATIONS; r++) {
                            for (long j = MAX(0, pivot - HALF_NUM_SAMPLES_TO_SMOOTH_ACROSS);
                                 j < MIN(pivot + avail, pivot + HALF_NUM_SAMPLES_TO_SMOOTH_ACROSS); j++)
                                orig_channel[j - (pivot - HALF_NUM_SAMPLES_TO_SMOOTH_ACROSS)] = output_sample_wk[j*channelcount + c];
                            for (long j = MAX(0, pivot - HALF_NUM_SAMPLES_TO_SMOOTH_ACROSS) + 1;
                                 j < MIN(pivot + avail, pivot + HALF_NUM_SAMPLES_TO_SMOOTH_ACROSS) - 1; j++) {
                                output_sample_wk[j*channelcount + c] = (orig_channel[(j-1) - (pivot - HALF_NUM_SAMPLES_TO_SMOOTH_ACROSS)] +
                                                                        orig_channel[(j) - (pivot - HALF_NUM_SAMPLES_TO_SMOOTH_ACROSS)] + orig_channel[(j+1) - (pivot - HALF_NUM_SAMPLES_TO_SMOOTH_ACROSS)])/3.;
                            }
                        }
                    }
                } */

                for (long c = 0; c < channelcount; c++)
                    delete[] obf[c];
                delete[] obf;
            }

            
            
        }
        
        if (source == dest) { // inplace operation!
            ears_buffer_set_size_samps(ob, source, outframecount);
        } else {
            ears_buffer_copy_format(ob, source, dest);
            ears_buffer_set_size_samps(ob, dest, outframecount);
        }
        
        float *dest_sample = buffer_locksamples(dest);
        
        if (!dest_sample) {
            err = EARS_ERR_CANT_WRITE;
            object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
        } else {
            sysmem_copyptr(output_sample_wk, dest_sample, channelcount * outframecount * sizeof(float));
            
            buffer_setdirty(dest);
            buffer_unlocksamples(dest);
        }
        
        for (size_t i = 0; i < channelcount; ++i)
            delete[] ibuf[i];
        delete[] fbuf;
        delete[] ibuf;
        bach_freeptr(orig_sample_wk);
        bach_freeptr(output_sample_wk);
    }
    return err;
}



