/**
    @file
    ears.spectralannotations.h
    Utilities to write and parse spectral annotations
 
    by Daniele Ghisi
 */

#ifndef _EARS_BUF_SPECTRALANNOTATIONS_H_
#define _EARS_BUF_SPECTRALANNOTATIONS_H_

#include "ears.commons.h"
#include "ears.libtag_commons.h"


char *ears_spectralannotation_get_string(t_object *ob, double sr, t_ears_spectralbuf_metadata *data)
{
    t_symbol *unit = ears_frequnit_to_symbol(data->binunit);
    char *bins_buf = NULL;
    llll_to_text_buf(data->bins, &bins_buf, 0, 6, 0, LLLL_T_NONE, LLLL_TE_SMART, NULL);
    char *annotation = (char *)bach_newptr(((bins_buf ? strlen(bins_buf) : 0)+4096) * sizeof(char));
    snprintf_zero(annotation, 4096, "earsspectral=1;sr=%.6f;audiosr=%.6f;spectype=%s;binsize=%.6f;binoffset=%.6f;binunit=%s;bins=%s",
                  sr,
                  data->original_audio_signal_sr,
                  data->type ? data->type->s_name : "none",
                  data->binsize,
                  data->binoffset,
                  unit ? unit->s_name : "none",
                  bins_buf ? bins_buf : "");
    bach_freeptr(bins_buf);
    return annotation;
}

void ears_spectralannotation_write(t_object *ob, t_symbol *filename, double sr, t_ears_spectralbuf_metadata *data) {
    
    if (!filename || !filename->s_name || !data)
        return;
    
    if (ears_symbol_ends_with(filename, ".aif", true) || ears_symbol_ends_with(filename, ".aiff", true)) {
        // we handle spectralannotation via ANNOTATION
        AIFF_Ref ref = AIFF_OpenFile(filename->s_name, F_RDONLY);
        uint64_t nSamples;
        int channels;
        double samplingRate;
        int bitsPerSample;
        int segmentSize;
        int32_t *samples = NULL;
        if (AIFF_GetAudioFormat(ref,&nSamples,&channels,
                                &samplingRate,&bitsPerSample,
                                &segmentSize) < 1 ) {
            object_error(ob, "Error writing spectral annotation.");
            AIFF_CloseFile(ref);
            return;
        } else {
            samples = (int32_t *)bach_newptr(channels * nSamples * sizeof(int32_t));
            int numReadSamples = AIFF_ReadSamples32Bit(ref, samples, channels * nSamples);
            if (numReadSamples != channels * nSamples) {
                object_warn(ob, "The output file may be corrupted.");
            }
        }
        AIFF_CloseFile(ref);
        
        ref = AIFF_OpenFile(filename->s_name, F_WRONLY);
        if (ref) {
            // gotta re-write the samples
            AIFF_SetAudioFormat(ref, channels, sr, bitsPerSample);
            if (AIFF_StartWritingSamples(ref) < 1) {
                object_error(ob, "Error writing file");
            } else {
                if (AIFF_WriteSamples32Bit(ref, samples, channels * nSamples) < 1) {
                    object_warn(ob, "The output file may be corrupted.");
                }
                AIFF_EndWritingSamples(ref);
            }
            
            
            if (data) { // add spectral annotations
                char *annotation = ears_spectralannotation_get_string(ob, sr, data);
                AIFF_SetAttribute(ref, AIFF_ANNO, annotation);
                bach_freeptr(annotation);
            }

            AIFF_CloseFile(ref);
        }
        
    } else if (ears_symbol_ends_with(filename, ".wav", true) || ears_symbol_ends_with(filename, ".wave", true)) {
        // we handle spectralannotation via INFO tag
        TagLib::FileRef f(filename->s_name);
        TagLib::File *file = f.file();
        if (file) {
            TagLib::RIFF::File *RIFFfile = dynamic_cast<TagLib::RIFF::File *>(file);
            if (RIFFfile) {
                TagLib::RIFF::WAV::File *RIFFWAVfile = dynamic_cast<TagLib::RIFF::WAV::File *>(file);
                if (RIFFWAVfile) {
                    char *annotation = ears_spectralannotation_get_string(ob, sr, data);
                    RIFFWAVfile->InfoTag()->setFieldText("EARS", annotation);
                    bach_freeptr(annotation);
                    f.save();
                }
            }
        }
    } else if (ears_symbol_ends_with(filename, ".wv", true)){
        // we handle spectralannotation via an APE tag
        TagLib::FileRef f(filename->s_name);
        TagLib::File *file = f.file();
        if (file) {
            TagLib::WavPack::File *WVfile = dynamic_cast<TagLib::WavPack::File *>(file);
            if (WVfile) {
                char *annotation = ears_spectralannotation_get_string(ob, sr, data);
                TagLib::APE::Item item = TagLib::APE::Item("EARS", annotation);
                WVfile->APETag()->setItem("EARS", item);
                bach_freeptr(annotation);
                f.save();
            }
        }
    }
}

#endif // _EARS_BUF_SPECTRALANNOTATIONS_H_

