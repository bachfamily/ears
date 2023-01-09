#include "ears.h"

#if defined EARS_MP3_WRITE_SUPPORT || defined EARS_MP3_READ_SUPPORT
#include "ears.mp3.h"
#endif

#ifdef EARS_MP3_READ_SUPPORT
bool mpg123_has_been_initialized = false;

void *ears_mpg123_quit(t_symbol *s, short argc, t_atom *argv)
{
    mpg123_exit();
    return NULL;
}

void ears_mpg123_init()
{
    if (!mpg123_has_been_initialized) {
        if (mpg123_init() != MPG123_OK)
            error("Error while loading mpg123 library.");
        quittask_install((method)ears_mpg123_quit, NULL);
        mpg123_has_been_initialized = true;
    }
}


long ears_buffer_read_handle_mp3(t_object *ob, char *filename, double start, double end, t_buffer_obj *buf, e_ears_timeunit timeunit)
{
    long ears_err = EARS_ERR_NONE;
    
    double start_sample = start;
    double end_sample = end;
    
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
    
    switch (timeunit) {
        case EARS_TIMEUNIT_SAMPS:
            start_sample = start;
            end_sample = end;
            break;

        case EARS_TIMEUNIT_MS:
            start_sample = start >= 0 ? ears_ms_to_samps(start, rate) : -1;
            end_sample = end >= 0 ? ears_ms_to_samps(end, rate) : -1;
            break;

        case EARS_TIMEUNIT_DURATION_RATIO:
        {
            long numframes = mpg123_length(mh);
            start_sample = start >= 0 ? start * numframes : -1;
            end_sample = end >= 0 ? end * numframes : -1;
        }
            break;

        default:
            break;
    }
    
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
    unsigned char *buffer = (unsigned char*) bach_newptrclear(buffer_size * sizeof(unsigned char));
    
    
    mpg123_seek(mh, start_sample, SEEK_SET);
    
    size_t done;
    res = mpg123_read(mh, buffer, buffer_size, &done);
    
    if (done > 0) { // we have something to write
        ears_buffer_set_sr(ob, buf, rate);
        ears_buffer_set_size_and_numchannels(ob, buf, num_samples, channels);
        
        
        long numsamps = ears_buffer_get_size_samps(ob, buf);
        
        float *sample = ears_buffer_locksamples(buf);
        
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
            ears_buffer_unlocksamples(buf);
            
        }
    }
    
    if (mh) {
        mpg123_close(mh);
        mpg123_delete(mh);
    }
    
    bach_freeptr(buffer);
    
    return ears_err;
}

#endif // EARS_MP3_READ_SUPPORT



#ifdef EARS_MP3_WRITE_SUPPORT

void ears_set_mp3_encodingsettings(lame_t lame, t_ears_encoding_settings *settings)
{
    if (!settings) {
        lame_set_VBR(lame, vbr_default);
        return;
    }
    
    switch (settings->vbr_type) {
        case EARS_MP3_VBRMODE_CBR:
            // Constant Bitrate
            lame_set_VBR(lame, vbr_off);
            if (settings->bitrate > 0)
                lame_set_brate(lame, settings->bitrate);
            break;
        case EARS_MP3_VBRMODE_ABR:
            // Average Bitrate
            lame_set_VBR(lame, vbr_abr);
            if (settings->bitrate > 0)
                lame_set_VBR_mean_bitrate_kbps(lame, settings->bitrate);
            if (settings->bitrate_min > 0)
                lame_set_VBR_min_bitrate_kbps(lame, settings->bitrate_min);
            if (settings->bitrate_max > 0)
                lame_set_VBR_max_bitrate_kbps(lame, settings->bitrate_max);
            break;
        case EARS_MP3_VBRMODE_VBR:
            // Variable Bitrate (new)
            lame_set_VBR(lame, vbr_default);
            if (settings->quality >= 0)
                lame_set_VBR_quality(lame, settings->quality);
            if (settings->bitrate_min > 0)
                lame_set_VBR_min_bitrate_kbps(lame, settings->bitrate_min);
            if (settings->bitrate_max > 0)
                lame_set_VBR_min_bitrate_kbps(lame, settings->bitrate_max);
            break;
        default:
            lame_set_VBR(lame, vbr_default);
            break;
    }
    
}


void ears_writemp3(t_object *buf, t_symbol *filename, t_ears_encoding_settings *settings)
{
    int write;
    t_symbol *conformed_path = get_conformed_resolved_path(filename);
    
    if (!conformed_path || !conformed_path->s_name || strlen(conformed_path->s_name)==0) {
        error("Cannot open file %s for write", filename->s_name);
        return;
    }
    
    
    FILE *mp3 = fopen(conformed_path->s_name, "wb");
    
    if (!mp3) {
        error("Cannot open file %s for write", conformed_path ? conformed_path->s_name : filename->s_name);
        return;
    }
    
    lame_t lame = lame_init();
    
    lame_set_in_samplerate(lame, ears_buffer_get_sr(NULL, buf));
    ears_set_mp3_encodingsettings(lame, settings);
    lame_set_write_id3tag_automatic(lame, 0);

    if (lame_init_params(lame) < 0) {
        error("Cannot write MP3 file %s with these parameters", conformed_path ? conformed_path->s_name : filename->s_name);
        return;
    }

    const int MP3_SIZE = LAME_MAXMP3BUFFER;
    const int PCM_SIZE = lame_get_maximum_number_of_samples(lame, MP3_SIZE);
    float pcm_buffer_l[PCM_SIZE];
    float pcm_buffer_r[PCM_SIZE];
    unsigned char mp3_buffer[MP3_SIZE];

/*    int sr = ears_buffer_get_sr(NULL, buf);
    
    lame_t lame = lame_init();
    lame_set_in_samplerate(lame, sr);
    ears_set_mp3_encodingsettings(lame, settings);
    lame_init_params(lame);
     */
    
    float *sample = ears_buffer_locksamples(buf);

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
        ears_buffer_unlocksamples(buf);
    }
    
    lame_close(lame);
    fclose(mp3);
}


/*
void ears_writemp3(t_object *buf, t_symbol *filename, t_ears_encoding_settings *settings)
{
    
    if (false){
        int write;
        // TEST
        lame_t lame = lame_init();
        lame_set_in_samplerate(lame, 44100);
//        lame_set_out_samplerate(lame, 44100);
        lame_set_VBR(lame, vbr_default);
//        lame_set_write_id3tag_automatic(lame, 0);
//        lame_set_mode(lame, STEREO);
//        lame_set_num_channels(lame, 2);
//        lame_set_bWriteVbrTag(lame, 0);
        lame_set_VBR_quality(lame, 4.);

        lame_init_params(lame);

        FILE *mp3 = fopen("/tmp/tmp.mp3", "wb");
        
        long nsamples = 1024;
        long mp3size = nsamples * 2;
        float pcm_buffer_l[nsamples];
        float pcm_buffer_r[nsamples];
        unsigned char mp3_buffer[nsamples];
        for (long j = 0; j < nsamples; j++) {
            pcm_buffer_l[j] = sin((double)j/100.);
            pcm_buffer_r[j] = sin((double)j/100.);
        }

        write = lame_encode_buffer_ieee_float(lame, pcm_buffer_l, pcm_buffer_r, nsamples, mp3_buffer, mp3size);
        fwrite(mp3_buffer, 1, write, mp3);

        write = lame_encode_flush(lame, mp3_buffer, mp3size);
        fwrite(mp3_buffer, 1, write, mp3);


        fclose(mp3);
        
        lame_close(lame);
        return;
    }
     
    
    int write;
    t_symbol *conformed_path = get_conformed_resolved_path(filename);
    
    if (!conformed_path || !conformed_path->s_name || strlen(conformed_path->s_name)==0) {
        error("Cannot open file %s for write", filename->s_name);
        return;
    }
    
    
    FILE *mp3 = fopen(conformed_path->s_name, "wb");
    
    if (!mp3) {
        error("Cannot open file %s for write", conformed_path ? conformed_path->s_name : filename->s_name);
        return;
    }
    
    lame_t lame = lame_init();
    
    lame_set_in_samplerate(lame, ears_buffer_get_sr(NULL, buf));
    lame_set_VBR(lame, vbr_default);
//    ears_set_mp3_encodingsettings(lame, settings);
    lame_set_write_id3tag_automatic(lame, 0);

    if (lame_init_params(lame) < 0) {
        error("Cannot write MP3 file %s with these parameters", conformed_path ? conformed_path->s_name : filename->s_name);
        return;
    }



    const int MP3_SIZE = LAME_MAXMP3BUFFER;
    const int PCM_SIZE = lame_get_maximum_number_of_samples(lame, MP3_SIZE);

    float pcm_buffer_l[PCM_SIZE];
    float pcm_buffer_r[PCM_SIZE];
    unsigned char mp3_buffer[MP3_SIZE];
//    int L = ((std::numeric_limits<int>::max)()) * 0.99;
    
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
                pcm_buffer_r[j] = (channelcount > 1 ? sample[(i+j) * channelcount + 1] : sample[(i+j) * channelcount]);

            }
//            write = lame_encode_buffer_int(lame, pcm_buffer_l, pcm_buffer_r, nsamples, mp3_buffer, sizeof(mp3_buffer));
            write = lame_encode_buffer_ieee_float(lame, pcm_buffer_l, pcm_buffer_r, nsamples, mp3_buffer, MP3_SIZE);
            if (fwrite(mp3_buffer, 1, write, mp3) != write) {
                error("Error writing MP3 output.");
            }
            
            i += nsamples;
        }
        
//        if (settings->nogap)
//            write = lame_encode_flush_nogap(lame, mp3_buffer, MP3_SIZE);
///        else
            write = lame_encode_flush(lame, mp3_buffer, MP3_SIZE);
        fwrite(mp3_buffer, 1, write, mp3);
        buffer_unlocksamples(buf);
    }
    
    lame_close(lame);
    fclose(mp3);
}
*/

#endif // EARS_MP3_WRITE_SUPPORT

