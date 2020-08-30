#include "ears.ambisonic.h"
#include <Hoa.hpp>

long ears_buffer_hoa_get_channel_count(int dimension, long order)
{
    if (dimension == 2)
        return order*2+1;
    if (dimension == 3)
        return (order+1)*(order+1);
    return 0;
}

long ears_ambisonic_num_channels_to_order(int dimension, long num_channels)
{
    if (dimension == 2) {
        if (num_channels % 2 == 0)
            return -1;
        else
            return (num_channels - 1)/2;
    }
    
    if (dimension == 3) {
        int sqrtnc = round(sqrt(num_channels));
        if (sqrtnc * sqrtnc == num_channels) {
            // perfect square
            return sqrtnc - 1;
        } else
            return -1;
    }
    
    return -1;
}
t_ears_err ears_buffer_hoa_encode(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, int dimension, long order, t_llll *azimuth, t_llll *elevation, t_llll *distance)
{
     
    if (!source || !dest)
        return EARS_ERR_NO_BUFFER;
    
    if (dimension != 2 && dimension != 3) {
        object_error(ob, "Dimension must be either 2 or 3.");
        return EARS_ERR_GENERIC;
    }

    if (order < 1) {
        object_error(ob, "Order must be at least 1.");
        return EARS_ERR_GENERIC;
    }

    if (azimuth->l_size < 1 || azimuth->l_depth < 1) {
        object_error(ob, "Invalid azimuth envelope.");
        return EARS_ERR_GENERIC;
    }

    if (elevation->l_size < 1 || elevation->l_depth < 1) {
        object_error(ob, "Invalid elevation envelope.");
        return EARS_ERR_GENERIC;
    }
    
    if (distance->l_size < 1 || distance->l_depth < 1) {
        object_error(ob, "Invalid distance envelope.");
        return EARS_ERR_GENERIC;
    }


    t_ears_err err = EARS_ERR_NONE;
    float *orig_sample = buffer_locksamples(source);
    float *orig_sample_wk = NULL;

    if (!orig_sample) {
        err = EARS_ERR_CANT_READ;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
    } else {
        t_atom_long    channelcount = buffer_getchannelcount(source);        // number of floats in a frame
        t_atom_long    framecount   = buffer_getframecount(source);            // number of floats long the buffer is for a single channel
        
        if (channelcount == 0) {
            object_error(ob, "Buffer has no channels!");
            buffer_unlocksamples(source);
            return EARS_ERR_GENERIC;
        }
        
        if (source == dest) { // inplace operation!
            orig_sample_wk = (float *)bach_newptr(channelcount * framecount * sizeof(float));
            sysmem_copyptr(orig_sample, orig_sample_wk, channelcount * framecount * sizeof(float));
            buffer_unlocksamples(source);
        } else {
            orig_sample_wk = orig_sample;
            ears_buffer_copy_format(ob, source, dest);
        }
        
        
        long outchannelcount = ears_buffer_hoa_get_channel_count(dimension, order);
        ears_buffer_set_size_and_numchannels(ob, dest, framecount, outchannelcount);

        if (dimension == 3) {
            hoa::Encoder<hoa::Hoa3d, float> encoder(order);
            
            t_ears_envelope_iterator azimuth_eei, elevation_eei, distance_eei;
            bool azimuth_is_envelope = (azimuth->l_depth > 1);
            bool elevation_is_envelope = (elevation->l_depth > 1);
            bool distance_is_envelope = (distance->l_depth > 1);

            double prev_azimuth = 0, prev_elevation = 0, prev_distance = 1.;
            
            if (azimuth_is_envelope) {
                azimuth_eei = ears_envelope_iterator_create(azimuth, 0., false);
            } else {
                prev_azimuth = hatom_getdouble(&azimuth->l_head->l_hatom);
            }
            if (elevation_is_envelope) {
                elevation_eei = ears_envelope_iterator_create(elevation, 0., false);
            } else {
                prev_elevation = hatom_getdouble(&elevation->l_head->l_hatom);
            }
            if (distance_is_envelope) {
                distance_eei = ears_envelope_iterator_create(distance, 0., false);
            } else {
                prev_distance = hatom_getdouble(&distance->l_head->l_hatom);
            }
            
            // HOALibrary convention is that angle increase clockwise,
            // as in Mathematics; however, musical convention often goes the other way around (e.g. in Ircam's spat)
            // we choose the latter
            encoder.setAzimuth(-prev_azimuth);
            encoder.setElevation(prev_elevation);
            encoder.setRadius(prev_distance);
            
            float *dest_sample = buffer_locksamples(dest);
            
            if (!dest_sample) {
                err = EARS_ERR_CANT_WRITE;
                object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
            } else {
                for (long i = 0; i < framecount; i++) {
                    if (azimuth_is_envelope) {
                        double new_azimuth = ears_envelope_iterator_walk_interp(&azimuth_eei, i, framecount);
                        if (new_azimuth != prev_azimuth) {
                            encoder.setAzimuth(-new_azimuth);
                            prev_azimuth = new_azimuth;
                        }
                    }
                    if (elevation_is_envelope) {
                        double new_elevation = ears_envelope_iterator_walk_interp(&elevation_eei, i, framecount);
                        if (new_elevation != prev_elevation) {
                            encoder.setElevation(new_elevation);
                            prev_elevation = new_elevation;
                        }
                    }
                    if (distance_is_envelope) {
                        double new_distance = ears_envelope_iterator_walk_interp(&distance_eei, i, framecount);
                        if (new_distance != prev_distance) {
                            encoder.setElevation(new_distance);
                            prev_distance = new_distance;
                        }
                    }
                    encoder.process(&orig_sample_wk[channelcount * i], &dest_sample[outchannelcount * i]);
                }
                
            }
            buffer_setdirty(dest);
            buffer_unlocksamples(dest);

        } else {

            hoa::Encoder<hoa::Hoa2d, float> encoder(order);
            
            t_ears_envelope_iterator azimuth_eei, distance_eei;
            bool azimuth_is_envelope = (azimuth->l_depth > 1);
            bool distance_is_envelope = (distance->l_depth > 1);
            
            double prev_azimuth = 0, prev_distance = 1.;
            
            if (azimuth_is_envelope) {
                azimuth_eei = ears_envelope_iterator_create(azimuth, 0., false);
            } else {
                prev_azimuth = hatom_getdouble(&azimuth->l_head->l_hatom);
            }
            if (distance_is_envelope) {
                distance_eei = ears_envelope_iterator_create(distance, 0., false);
            } else {
                prev_distance = hatom_getdouble(&distance->l_head->l_hatom);
            }
            
            encoder.setAzimuth(-prev_azimuth);
            encoder.setRadius(prev_distance);
            
            float *dest_sample = buffer_locksamples(dest);
            
            if (!dest_sample) {
                err = EARS_ERR_CANT_WRITE;
                object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
            } else {
                for (long i = 0; i < framecount; i++) {
                    if (azimuth_is_envelope) {
                        double new_azimuth = ears_envelope_iterator_walk_interp(&azimuth_eei, i, framecount);
                        if (new_azimuth != prev_azimuth) {
                            encoder.setAzimuth(-new_azimuth);
                            prev_azimuth = new_azimuth;
                        }
                    }
                    if (distance_is_envelope) {
                        double new_distance = ears_envelope_iterator_walk_interp(&distance_eei, i, framecount);
                        if (new_distance != prev_distance) {
                            encoder.setElevation(new_distance);
                            prev_distance = new_distance;
                        }
                    }
                    encoder.process(&orig_sample_wk[channelcount * i], &dest_sample[outchannelcount * i]);
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


t_ears_err ears_buffer_hoa_decode(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, int dimension,
                                  long num_out_channels, double *out_channels_azimuth, double *out_channels_elevation)
{
    
    if (!source || !dest)
        return EARS_ERR_NO_BUFFER;
    
    if (dimension != 2 && dimension != 3) {
        object_error(ob, "Dimension must be either 2 or 3.");
        return EARS_ERR_GENERIC;
    }
    
    if (num_out_channels < 1) {
        object_error(ob, "The number of output channels must be at least 1.");
        return EARS_ERR_GENERIC;
    }

    t_ears_err err = EARS_ERR_NONE;
    float *orig_sample = buffer_locksamples(source);
    float *orig_sample_wk = NULL;

    if (!orig_sample) {
        err = EARS_ERR_CANT_READ;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
    } else {
        t_atom_long    channelcount = buffer_getchannelcount(source);        // number of floats in a frame
        t_atom_long    framecount   = buffer_getframecount(source);            // number of floats long the buffer is for a single channel
        
        long order = ears_ambisonic_num_channels_to_order(dimension, channelcount);
        
        if (order <= 0) {
            object_error(ob, "Buffer has the wrong number of channels.");
            buffer_unlocksamples(source);
            return EARS_ERR_GENERIC;
        }
        
        if (num_out_channels < ears_buffer_hoa_get_channel_count(dimension, order)) {
            object_warn(ob, "The number of output channels is insufficient for ambisonic decoding");
        }

        
        if (source == dest) { // inplace operation!
            orig_sample_wk = (float *)bach_newptr(channelcount * framecount * sizeof(float));
            sysmem_copyptr(orig_sample, orig_sample_wk, channelcount * framecount * sizeof(float));
            buffer_unlocksamples(source);
        } else {
            orig_sample_wk = orig_sample;
            ears_buffer_copy_format(ob, source, dest);
        }
        
        ears_buffer_set_size_and_numchannels(ob, dest, framecount, num_out_channels);

        if (dimension == 3) {
            hoa::DecoderRegular<hoa::Hoa3d, float> decoder(order, num_out_channels);
            
            for (long c = 0; c < num_out_channels; c++) {
                decoder.setPlanewaveAzimuth(c, -out_channels_azimuth[c]); // flip rotation convention
                decoder.setPlanewaveElevation(c, out_channels_elevation[c]);
            }
            
            float *dest_sample = buffer_locksamples(dest);
            
            if (!dest_sample) {
                err = EARS_ERR_CANT_WRITE;
                object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
            } else {
                for (long i = 0; i < framecount; i++) {
                    decoder.process(&orig_sample_wk[channelcount * i], &dest_sample[num_out_channels * i]);
                }
                
            }
            buffer_setdirty(dest);
            buffer_unlocksamples(dest);
        } else {
            hoa::DecoderRegular<hoa::Hoa2d, float> decoder(order, num_out_channels);
            
            for (long c = 0; c < num_out_channels; c++) {
                decoder.setPlanewaveAzimuth(c, -out_channels_azimuth[c]); // flip rotation convention
            }
            
            float *dest_sample = buffer_locksamples(dest);
            
            if (!dest_sample) {
                err = EARS_ERR_CANT_WRITE;
                object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
            } else {
                for (long i = 0; i < framecount; i++) {
                    decoder.process(&orig_sample_wk[channelcount * i], &dest_sample[num_out_channels * i]);
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

t_ears_err ears_buffer_hoa_decode_binaural(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, int dimension)
{
    
    if (!source || !dest)
        return EARS_ERR_NO_BUFFER;
    
    if (dimension != 2 && dimension != 3) {
        object_error(ob, "Dimension must be either 2 or 3.");
        return EARS_ERR_GENERIC;
    }
    
    long num_out_channels = 2;
    t_ears_err err = EARS_ERR_NONE;
    float *orig_sample = buffer_locksamples(source);
    float *orig_sample_wk = NULL;

    if (!orig_sample) {
        err = EARS_ERR_CANT_READ;
        object_error((t_object *)ob, EARS_ERROR_BUF_CANT_READ);
    } else {
        t_atom_long    channelcount = buffer_getchannelcount(source);        // number of floats in a frame
        t_atom_long    framecount   = buffer_getframecount(source);            // number of floats long the buffer is for a single channel
        
        long order = ears_ambisonic_num_channels_to_order(dimension, channelcount);
        
        if (order <= 0) {
            object_error(ob, "Buffer has the wrong number of channels!");
            buffer_unlocksamples(source);
            return EARS_ERR_GENERIC;
        }
        
        
        if (source == dest) { // inplace operation!
            orig_sample_wk = (float *)bach_newptr(channelcount * framecount * sizeof(float));
            sysmem_copyptr(orig_sample, orig_sample_wk, channelcount * framecount * sizeof(float));
            buffer_unlocksamples(source);
        } else {
            orig_sample_wk = orig_sample;
            ears_buffer_copy_format(ob, source, dest);
        }
        
        ears_buffer_set_size_and_numchannels(ob, dest, framecount, num_out_channels);
        
        long block_size = 64;
        float **inputs = (float **) bach_newptr(channelcount * sizeof(float *));
        float **outputs = (float **) bach_newptr(2 * sizeof(float *));
        for (long c = 0; c < channelcount; c++)
            inputs[c] = (float *)bach_newptr(block_size * sizeof(float));
        for (long c = 0; c < 2; c++)
            outputs[c] = (float *)bach_newptr(block_size * sizeof(float));

        if (dimension == 3) {
#ifdef EARS_AMBISONIC_BINAURAL_SADIE
            hoa::DecoderBinaural<hoa::Hoa3d, float, hoa::hrir::Sadie_D2_3D> decoder(order);
#else
            hoa::DecoderBinaural<hoa::Hoa3d, float, hoa::hrir::Listen_1002C_3D> decoder(order);
#endif
            decoder.prepare(block_size);
            float *dest_sample = buffer_locksamples(dest);
            if (!dest_sample) {
                err = EARS_ERR_CANT_WRITE;
                object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
            } else {
                for (long i = 0; i < framecount; i += block_size) {
                    long b = block_size;
                    if (i + block_size > framecount) {
                        b = framecount - i;
                        decoder.prepare(b);
                    }
                    for (long c = 0; c < channelcount; c++)
                        for (int q = 0; q < b; q++)
                            inputs[c][q] = orig_sample_wk[channelcount * (i+q) + c];
                    
                    decoder.processBlock((const float **)inputs, outputs);
                    for (long c = 0; c < 2; c++)
                        for (int q = 0; q < b; q++)
                            dest_sample[2 * (i+q) + c] = outputs[c][q];
                }
            }
            buffer_setdirty(dest);
            buffer_unlocksamples(dest);
        } else {
#ifdef EARS_AMBISONIC_BINAURAL_SADIE
            hoa::DecoderBinaural<hoa::Hoa2d, float, hoa::hrir::Sadie_D2_2D> decoder(order);
#else
            hoa::DecoderBinaural<hoa::Hoa2d, float, hoa::hrir::Listen_1002C_2D> decoder(order);
#endif
            decoder.prepare(block_size);
            float *dest_sample = buffer_locksamples(dest);
            if (!dest_sample) {
                err = EARS_ERR_CANT_WRITE;
                object_error((t_object *)ob, EARS_ERROR_BUF_CANT_WRITE);
            } else {
                for (long i = 0; i < framecount; i++) {
                    decoder.process(&orig_sample_wk[channelcount * i], &dest_sample[num_out_channels * i]);
                }
            }
            buffer_setdirty(dest);
            buffer_unlocksamples(dest);
        }
        
        for (long c = 0; c < channelcount; c++)
            bach_freeptr(inputs[c]);
        bach_freeptr(inputs);
        for (long c = 0; c < 2; c++)
            bach_freeptr(outputs[c]);
        bach_freeptr(outputs);


        if (source == dest) // inplace operation!
            bach_freeptr(orig_sample_wk);
        else
            buffer_unlocksamples(source);
    }
    
    return err;
}
