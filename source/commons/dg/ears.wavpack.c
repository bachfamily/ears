#include "ears.wavpack.h"



long ears_buffer_read_handle_wavpack(t_object *ob, char *filename, long start, long end, t_buffer_obj *buf, t_symbol **sampleformat, e_ears_timeunit timeunit)
{
    char error = 0;
    long start_sample = start;
    long end_sample = end;
    long ears_err = EARS_ERR_NONE;
    int flags = OPEN_NORMALIZE | OPEN_WVC;
    int norm_offset = 0;
    WavpackContext *wpc = WavpackOpenFileInput(filename, &error, flags, norm_offset);
    
    if (!wpc) {
        ears_err = EARS_ERR_CANT_READ;
        object_error(ob, EARS_ERROR_BUF_CANT_READ);
        return ears_err;
    }
    
    long tot_num_samples = WavpackGetNumSamples(wpc), num_samples = tot_num_samples;
    long num_channels = WavpackGetNumChannels(wpc);
    int bitspersample = WavpackGetBitsPerSample (wpc);
    long sr = WavpackGetSampleRate(wpc);
    
    // set sample format
    *sampleformat = _llllobj_sym_unknown;
    if (WavpackGetMode(wpc) & MODE_FLOAT) {
        switch (bitspersample) {
            case 32: *sampleformat = _sym_float32; break;
            // float64 is not supported by wavpack
            default: break;
        }
    } else {
        switch (bitspersample) {
            case 8: *sampleformat = _sym_int8; break;
            case 16: *sampleformat = _sym_int16; break;
            case 24: *sampleformat = _sym_int24; break;
            case 32: *sampleformat = _sym_int32; break;
            default: break;
        }
    }
    
    switch (timeunit) {
        case EARS_TIMEUNIT_MS:
            start_sample = start >= 0 ? ears_ms_to_samps(start, sr) : -1;
            end_sample = end >= 0 ? ears_ms_to_samps(end, sr) : -1;
            break;

        case EARS_TIMEUNIT_SAMPS:
            start_sample = start;
            end_sample = end;
            break;

        case EARS_TIMEUNIT_DURATION_RATIO:
            start_sample = start >= 0 ? start * num_samples : -1;
            end_sample = end >= 0 ? end * num_samples : -1;
            break;

        default:
            break;
    }

    
    if (start_sample < 0)
        start_sample = 0;
    if (end_sample < 0)
        end_sample = tot_num_samples;
    
    if (start_sample > 0) {
        if (WavpackSeekSample64(wpc, start_sample)) {
            // seek successful
            num_samples = MIN(tot_num_samples - start_sample, end_sample - start_sample);
        } else {
            object_warn(ob, EARS_WARNING_BUF_CANT_SEEK);
        }
    } else {
        num_samples = end_sample;
    }
    
    ears_buffer_set_size_and_numchannels(ob, buf, num_samples, num_channels);
    ears_buffer_set_sr(ob, buf, sr);
    
    float *sample = ears_buffer_locksamples(buf);
    
    if (!sample) {
        ears_err = EARS_ERR_CANT_READ;
        object_error(ob, EARS_ERROR_BUF_CANT_READ);
    } else {
        
        int samplesUnpacked = WavpackUnpackSamples(wpc, (int32_t *)sample, num_samples);
        if (samplesUnpacked <= 0) {
            ears_err = EARS_ERR_CANT_READ;
            object_error(ob, EARS_ERROR_BUF_CANT_READ);
        } else {
            if (WavpackGetMode(wpc) & MODE_FLOAT) {
                // nothing to do, it's already a floating point value
            } else {
                // rescaling the output
                const double temp = 1. / (uint32_t(1) << (bitspersample - 1));
                for (long i = 0; i < num_samples * num_channels; i++)
                    sample[i] = (*(int32_t *)(sample + i)) * temp;
            }
        }
        buffer_setdirty(buf);
        ears_buffer_unlocksamples(buf);
    }
    
    WavpackSeekSample(wpc, start_sample);
    WavpackCloseFile(wpc);
    return ears_err;
}


typedef struct {
    uint32_t bytes_written, first_block_size;
    FILE *file;
    int error;
} write_id;


int DoWriteFile (FILE *hFile, void *lpBuffer, uint32_t nNumberOfBytesToWrite, uint32_t *lpNumberOfBytesWritten)
{
    uint32_t bcount;
    
    *lpNumberOfBytesWritten = 0;
    
    while (nNumberOfBytesToWrite) {
        bcount = (uint32_t) fwrite ((unsigned char *) lpBuffer + *lpNumberOfBytesWritten, 1, nNumberOfBytesToWrite, hFile);
        
        if (bcount) {
            *lpNumberOfBytesWritten += bcount;
            nNumberOfBytesToWrite -= bcount;
        }
        else
            break;
    }
    
    return !ferror (hFile);
}

int DoTruncateFile (FILE *hFile)
{
    if (hFile) {
        fflush (hFile);
#if defined(_WIN32)
        return !_chsize (_fileno (hFile), 0);
#else
        return !ftruncate(fileno (hFile), 0);
#endif
    }
    
    return 0;
}

int DoCloseHandle (FILE *hFile)
{
    return hFile ? !fclose (hFile) : 0;
}

static int write_block (void *id, void *data, int32_t length)
{
    write_id *wid = (write_id *) id;
    uint32_t bcount;
    
    if (wid->error)
        return false;
    
    if (wid && wid->file && data && length) {
        if (!DoWriteFile (wid->file, data, length, &bcount) || bcount != length) {
            DoTruncateFile (wid->file);
            DoCloseHandle (wid->file);
            wid->file = NULL;
            wid->error = 1;
            return false;
        } else {
            wid->bytes_written += length;
            
            if (!wid->first_block_size)
                wid->first_block_size = bcount;
        }
    }
    
    return TRUE;
}


void ears_writewavpack(t_object *buf, t_symbol *filename, t_ears_encoding_settings *settings)
{
    bool has_details = 0;
    t_symbol *conformed_path = get_conformed_resolved_path(filename);
    
    if (!conformed_path || !conformed_path->s_name || strlen(conformed_path->s_name)==0) {
        error("Cannot open file %s for write", filename->s_name);
        return;
    }
    
    write_id wv_file, wvc_file;
    wv_file.bytes_written = wv_file.error = wv_file.first_block_size = 0;
    wvc_file.bytes_written = wvc_file.error = wvc_file.first_block_size = 0;
    
    wv_file.file = fopen(conformed_path->s_name, "w+b");
    if (!wv_file.file) {
        error("Cannot open file %s for write", conformed_path ? conformed_path->s_name : filename->s_name);
        return;
    }
    
    if (settings->use_correction_file) {
        char correction_conformed_path[MAX_PATH_CHARS];
        snprintf(correction_conformed_path, MAX_PATH_CHARS, "%sc", conformed_path->s_name);
        
        if (!correction_conformed_path || strlen(correction_conformed_path)==0) {
            error("Cannot open correction file %s for write", correction_conformed_path);
            return;
        }
        
        has_details = 1;
        wvc_file.file = fopen(correction_conformed_path, "w+b");
        
        if (!wvc_file.file) {
            error("Cannot open correction file %s for write", correction_conformed_path);
            has_details = 0;
        }
    }
    
    
    WavpackContext *wpc = WavpackOpenFileOutput(write_block,  &wv_file, has_details ? &wvc_file : NULL);
    if (!wpc) {
        error("Cannot encode in WavPack format");
        return;
    }
    
    long tot_num_samples = ears_buffer_get_size_samps(NULL, buf);
    WavpackConfig config;
    bool output_floatingpoint = false;
    memset(&config, 0, sizeof(config));
    
    config.sample_rate = ears_buffer_get_sr(NULL, buf);
    config.num_channels = ears_buffer_get_numchannels(NULL, buf);
//    config.qmode |= QMODE_RAW_PCM; // | QMODE_SIGNED_BYTES;
    config.float_norm_exp = 0;
    
    if (has_details) {
        config.flags |= CONFIG_HYBRID_FLAG | CONFIG_CREATE_WVC | CONFIG_BITRATE_KBPS;
        config.bitrate = settings->bitrate;
    }

    if (config.num_channels <= 2)
        config.channel_mask = 0x5 - config.num_channels;
    else if (config.num_channels <= 18)
        config.channel_mask = (1 << config.num_channels) - 1;
    else
        config.channel_mask = 0x3ffff;
    
    if (settings->format == _sym_int8) {
        config.bits_per_sample = 8;
        config.bytes_per_sample = 1;
    } else if (settings->format == _sym_int16) {
        config.bits_per_sample = 8*2;
        config.bytes_per_sample = 1*2;
    } else if (settings->format == _sym_int24) {
        config.bits_per_sample = 8*3;
        config.bytes_per_sample = 1*3;
    } else if (settings->format == _sym_int32) {
        config.bits_per_sample = 8*4;
        config.bytes_per_sample = 1*4;
    } else if (settings->format == _sym_float32 || settings->format == _sym_float64 || settings->format == _sym_float) {
        config.bits_per_sample = 8*4;
        config.bytes_per_sample = 1*4;
        config.float_norm_exp = 127;
        output_floatingpoint = true;
    }
    
    if (!WavpackSetConfiguration64(wpc, &config, tot_num_samples, NULL)) {
        error("WavpackSetConfiguration failed: %s", WavpackGetErrorMessage(wpc));
        return;
    }
    
    if (!WavpackPackInit(wpc)) {
        error("WavpackPackInit failed: %s", WavpackGetErrorMessage(wpc));
        return;
    }
    
    int32_t *sample_int32 = (int32_t *)sysmem_newptr(tot_num_samples * config.num_channels * sizeof (int32_t));
    
    float *sample = ears_buffer_locksamples(buf);
    if (!sample) {
        error("Can't read buffer!");
    } else {
        t_atom_long    channelcount = buffer_getchannelcount(buf);        // number of floats in a frame
        t_atom_long    framecount   = buffer_getframecount(buf);            // number of floats long the buffer is for a single channel
        
        if (output_floatingpoint) {
            // just copy
            for (long i = 0; i < channelcount * framecount; i++)
                sample_int32[i] = *((int32_t *)(sample+i));
        } else {
            // scale
            const double temp = (uint32_t(1) << (config.bits_per_sample - 1));
            for (long i = 0; i < channelcount * framecount; i++) {
                if (sample[i] >= 1.0)
                    sample_int32[i] = ((long)temp)-1;
                else if (sample[i] <= -1.0)
                    sample_int32[i] = -((long)temp);
                else
                    sample_int32[i] = floor (sample[i] * temp);
            }
        }

        ears_buffer_unlocksamples(buf);
    }
    
    
    if (!WavpackPackSamples(wpc, sample_int32, tot_num_samples)) {
        error("WavpackPackSamples failed: %s", WavpackGetErrorMessage(wpc));
        sysmem_freeptr(sample_int32);
        return;
    }
    
    if (!WavpackFlushSamples(wpc)) {
        error("WavpackFlushSamples failed: %s", WavpackGetErrorMessage(wpc));
        sysmem_freeptr(sample_int32);
        return;
    }
    
    sysmem_freeptr(sample_int32);
    
    if (!DoCloseHandle (wv_file.file)) {
        error("Can't close WavPack file!");
    }

    if (has_details && !DoCloseHandle (wvc_file.file)) {
        error("Can't close WavPack correction file!");
    }

    
//    WavpackCloseFile (wpc);
    
    //    fclose(wv_file.file);
    //    if (wvc_file.file)
    //        fclose(wvc_file.file);
}



