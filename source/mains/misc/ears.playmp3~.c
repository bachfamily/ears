/**
 @file
 ears.playmp3~.c
 
 @name
 ears.playmp3~
 
 @realname
 ears.playmp3~
 
 @type
 object
 
 @module
 ears
 
 @author
 Daniele Ghisi
 
 @digest
 Play mp3 natively
 
 @description
 Plays portions of mp3 files without needing to entirely convert them to AIFF files first.
 
 @discussion
 The source code uses the mpg123 library to decode mpeg data.
 The object is designed for direct-to-disk usage. No Max audio buffer is used.
 
 @category
 ears playback
 
 @keywords
 play, mpeg, mp3, compressed, native, file
 
 @seealso
 ears.writetags, ears.readtags
 
 @owner
 Daniele Ghisi
 */
#include "ext.h"			// standard Max include, always required (except in Jitter)
#include "ext_obex.h"		// required for "new" style objects
#include "z_dsp.h"			// required for MSP objects
#include "foundation/llllobj.h"
#include "foundation/llll_commons_ext.h"
#include "ears.mp3.h"

#define EARS_PLAYMP3_ENABLE_SAMPLEACCURATELOOPING

#define EARS_PLAYMP3_DEFAULT_MAX_PLAYRATE 8           // this is an initial allocation for memory. Playrate can be increase, and pointer will be resized

#ifdef CONFIGURATION_Development
#define assert_debug(...)    assert(__VA_ARGS__)
#else
#define assert_debug(...)    ((void) 0)
#endif


// struct to represent the object's state
typedef struct _playmp3 {
	t_pxobject		ob;			// the object itself (t_pxobject in MSP instead of t_object)
    double			num_outs;   // number of outlets: 1 or 2
    
    long            loop;

    long            samplerate;
    
    double          play_rate;
    t_float         lastsample[2]; // memory of last sample (left and right channel) for resampling
    
    void        *m_clock;
    
#ifdef EARS_MP3_READ_SUPPORT
    mpg123_handle*  mh;
#endif
    unsigned char*  buffer;
    size_t          buffer_size;
    size_t          buffer_size_wo_resampling;

    long            curr_sample;
    
    long            fadein_samples;
    long            fadeout_samples;
    
    int             playing;
    int             paused;
    
    int             channels;
    int             encoding;
    long            rate;
    long            seek_to;
    double          loop_ms;
    
    int             need_send_bang;
    long            temp;
    
    void            *out[3];
    
    t_systhread_mutex   c_mutex;
} t_playmp3;


// method prototypes
void *playmp3_new(t_symbol *s, long argc, t_atom *argv);
void playmp3_free(t_playmp3 *x);
void playmp3_task(t_playmp3 *x);
void playmp3_assist(t_playmp3 *x, void *b, long m, long a, char *s);
void playmp3_dsp64(t_playmp3 *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags);
void playmp3_perform64(t_playmp3 *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam);
void playmp3_perform64_interp(t_playmp3 *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam);
void playmp3_play(t_playmp3 *x, t_symbol *s, long argc, t_atom *argv);
void playmp3_stop(t_playmp3 *x);
void playmp3_pause(t_playmp3 *x);
void playmp3_resume(t_playmp3 *x);
void playmp3_senddonebang(t_playmp3 *x, t_symbol *s, long argc, t_atom *argv);
int close_file(t_playmp3 *x);

int open_file(t_playmp3 *x, const char *path, double start_ms, double end_ms);


// global class pointer variable
static t_class *playmp3_class = NULL;


//***********************************************************************************************

void ext_main(void *r)
{
#ifdef EARS_MP3_READ_SUPPORT
    ears_mpg123_init();
#endif

	// object initialization, note the use of dsp_free for the freemethod, which is required
	// unless you need to free allocated memory, in which case you should call dsp_free from
	// your custom free function.

    // @method play @digest Play MP3 file
    // @description A <m>play</m> message, followed by a MP3 filename, will play the MP3 file natively.
    // Additional arguments provide the starting and ending point in milliseconds for playback.
    // @marg 0 @name start_ms @optional 1 @type float
    // @marg 1 @name end_ms @optional 1 @type float
	t_class *c = class_new("ears.playmp3~", (method)playmp3_new, (method)playmp3_free, (long)sizeof(t_playmp3), 0L, A_GIMME, 0);

	class_addmethod(c, (method)playmp3_dsp64,		"dsp64",	A_CANT, 0);
	class_addmethod(c, (method)playmp3_assist,      "assist",	A_CANT, 0);
    class_addmethod(c, (method)playmp3_play,        "play",     A_GIMME, 0);
    class_addmethod(c, (method)playmp3_play,        "start",     A_GIMME, 0);
    class_addmethod(c, (method)playmp3_stop,        "stop",     A_GIMME, 0);
    class_addmethod(c, (method)playmp3_pause,       "pause",    0);
    class_addmethod(c, (method)playmp3_resume,      "resume",   0);
    
    CLASS_ATTR_LONG(c, "loop",	0,	t_playmp3, loop);
    CLASS_ATTR_STYLE_LABEL(c, "loop", 0, "onoff", "Loop");
    // @description Toggles cyclic playing.

    CLASS_ATTR_LONG(c, "fadeinsamps",    0,    t_playmp3, fadein_samples);
    CLASS_ATTR_STYLE_LABEL(c, "fadeinsamps", 0, "text", "Number Of Fade In Samples");
    // @description Sets the number of fade in samples

    
	class_dspinit(c);
	class_register(CLASS_BOX, c);
	playmp3_class = c;
}

void *playmp3_new(t_symbol *s, long argc, t_atom *argv)
{
#ifdef EARS_MP3_READ_SUPPORT
	t_playmp3 *x = (t_playmp3 *)object_alloc(playmp3_class);
    long true_ac = attr_args_offset(argc, argv);

	if (x) {
        // @arg 0 @name outlets @optional 1 @type int @digest Number of outlets
        // @description The number of output channels can be either 1 or 2.
        
        int err = MPG123_OK;
        long i, num_outs = true_ac ? CLAMP(atom_getlong(argv), 1, 2) : 1;

        x->num_outs = num_outs;
        x->samplerate = ears_get_current_Max_sr();
        systhread_mutex_new(&x->c_mutex, 0);

        x->m_clock = clock_new((t_object *)x, (method) playmp3_task);

        x->fadein_samples = 0;
        x->fadeout_samples = 0;
        x->play_rate = 1.;

        attr_args_process(x, argc, argv);
        
        dsp_setup((t_pxobject *)x, 0);	// MSP inlets: arg is # of inlets and is REQUIRED!
        
        x->out[num_outs] = bangout(x);
        for (i = num_outs - 1; i >= 0; i--)
            x->out[i] = outlet_new(x, "signal");
		
        x->mh = mpg123_new(NULL, &err);
        
        if (err != MPG123_OK) {
            error("Can't create mpg123 handle.");
            object_free(x);
            return NULL;
        }
        
        if (mpg123_param(x->mh, MPG123_FLAGS, MPG123_SEEKBUFFER | MPG123_GAPLESS | MPG123_SKIP_ID3V2, 0) != MPG123_OK) {
            error("Unable to set mpg123 library options.");
            object_free(x);
            return NULL;
        }
	}
	return (x);
#else
    error("ears.playmp3~ is not supported on Windows");
    return NULL;
#endif
}



void playmp3_free(t_playmp3 *x)
{
#ifdef EARS_MP3_READ_SUPPORT
    object_free(x->m_clock);
    mpg123_delete(x->mh);
    systhread_mutex_free(x->c_mutex);
    sysmem_freeptr(x->buffer);
    dsp_free((t_pxobject *)x);
#endif
}



void playmp3_task(t_playmp3 *x)
{
#ifdef EARS_MP3_READ_SUPPORT
    if (x->loop) {
        mpg123_seek(x->mh, x->seek_to, SEEK_SET);
        clock_fdelay(x->m_clock, x->loop_ms);
    } else {
        close_file(x);
    }
#endif
}



// open files, also seeks and schedules end
// doesn't work
int open_file_new(t_playmp3 *x, const char *path, double start_ms, double end_ms)
{
    int err = 0;
    
#ifdef EARS_MP3_READ_SUPPORT
    err = MPG123_OK;
    int channels, encoding;
    long srate;
    char paused = x->paused;
    long seek_to = 0;
    
    systhread_mutex_lock(x->c_mutex);
    
    x->paused = true; // to avoid complications in audio thread. BEWARE: this does not guarantee thread safety. Need to ask andrea...
    
    if (x->mh) mpg123_close(x->mh);
    
    char conformed_path[MAX_PATH_CHARS];
    path_nameconform(path, conformed_path, PATH_STYLE_MAX, PATH_TYPE_BOOT);
    
    // checking if name is correct
    long len = strlen(conformed_path);
    if (len >= 4 && strcmp(conformed_path + len - 4, ".mp3") != 0) {
        mpg123_close(x->mh);
        x->playing = x->paused = paused = false;
        object_error((t_object *)x, "File doesn't seem to have .mp3 extension.");
        err = MPG123_ERR;
        goto end;
    }
    
    if (mpg123_open(x->mh, conformed_path) != MPG123_OK) {
        // should never happen... hopefully...
        object_error((t_object *)x, mpg123_strerror(x->mh));
        err = MPG123_ERR;
        goto end;
    }
    
    if (mpg123_getformat(x->mh, &srate, &channels, &encoding) != MPG123_OK) {
        object_error((t_object *)x, "Failed to get mp3 information.");
        err = MPG123_ERR;
        goto end;
    }
    
    //    mpg123_format_all(mh);
    
    mpg123_format_none(x->mh);
    mpg123_format(x->mh, srate, channels, MPG123_ENC_FLOAT_32);

    x->playing = true;
    x->channels = channels;
    
#ifdef CONFIGURATION_Development
//    object_post((t_object *)x,"Opened mp3 file - sample rate: %ld, channels: %ld, encoding: %ld", srate, channels, encoding);
#endif
    
    // seeking?
    seek_to = start_ms * x->samplerate / 1000.;
    if (start_ms > 0)
        if (mpg123_seek(x->mh, seek_to, SEEK_SET) < 0)
            object_post((t_object *)x, "Could not seek position in mp3 file");
    x->seek_to = seek_to;
    
    // scheduling end?
    if (end_ms >= 0) {
        double dur_ms = end_ms - start_ms;
        clock_fdelay(x->m_clock, dur_ms);
        x->loop_ms = dur_ms;
    }
    
    x->need_send_bang = true;
    
    
    
    /*
     long err = mpg123_open(x->mh, conformed_path);
     if (err == MPG123_OK) {
     mpg123_getformat(x->mh, &srate, &channels, &encoding);
     mpg123_format_none(x->mh);
     mpg123_format(x->mh, srate, channels, MPG123_ENC_FLOAT_32);
     } else {
     mpg123_close(x->mh);
     x->playing = x->paused = paused = false;
     object_error((t_object *)x, "Cannot recognize mp3 file.");
     err = MPG123_ERR;
     goto end;
     }
     
     
     
     if (!x->mh) {
     // should never happen... hopefully...
     object_error((t_object *)x, "Cannot open mp3 file.");
     err = MPG123_ERR;
     } else {
     x->playing = true;
     //            if (mpg123_getformat(mh, &srate, &channels, &encoding) == MPG123_OK) {
     x->channels = channels;
     #ifdef CONFIGURATION_Development
     object_post((t_object *)x,"Opened mp3 file - sample rate: %ld, channels: %ld, encoding: %ld", srate, channels, encoding);
     #endif
     
     // seeking?
     long seek_to = start_ms * x->samplerate / 1000.;
     if (start_ms > 0)
     if (mpg123_seek(x->mh, seek_to, SEEK_SET) < 0)
     object_post((t_object *)x, "Could not seek position in mp3 file");
     x->seek_to = seek_to;
     
     // scheduling end?
     if (end_ms >= 0) {
     double dur_ms = end_ms - start_ms;
     clock_fdelay(x->m_clock, dur_ms);
     x->loop_ms = dur_ms;
     }
     
     x->need_send_bang = true;
     } else {
     mpg123_close(x->mh);
     x->playing = x->paused = paused = false;
     object_error((t_object *)x, "Cannot recognize mp3 file.");
     err = MPG123_ERR;
     }
     }
     } */
    
end:
    
    x->paused = paused;
    systhread_mutex_unlock(x->c_mutex);
    
#endif

    return err;
}


void ears_playmp3_ezlocate_file_char(const char *filename_in, char *filename_out, t_fourcc *file_type)
{
    char filename[MAX_FILENAME_CHARS];
    short path = 0;
    
    if (!filename_in)
        return;
    
    if (file_type) *file_type = 0;
    
    if (path_frompathname(filename_in, &path, filename)) {
        t_fourcc type;
        char file_path_str[MAX_FILENAME_CHARS];
        strncpy_zero(file_path_str, filename_in, MAX_FILENAME_CHARS);
        if (!locatefile_extended(file_path_str, &path, &type, &type, -1))  {
            path_topathname(path, file_path_str, filename);
            path_nameconform(filename, filename_out, PATH_STYLE_MAX, PATH_TYPE_BOOT);
            if (file_type) *file_type = type;
        }
    } else {
        char filenameok[MAX_FILENAME_CHARS];
        path_topathname(path, filename, filenameok);
        path_nameconform(filenameok, filename_out, PATH_STYLE_MAX, PATH_TYPE_BOOT);
    }
}


// open files, also seeks and schedules end
int open_file(t_playmp3 *x, const char *path, double start_ms, double end_ms, double play_rate)
{
    int err = 0;
    
#ifdef EARS_MP3_READ_SUPPORT
    err = MPG123_OK;
    int channels, encoding, orig_channels, orig_encoding;
    long srate, orig_srate;
    char paused = x->paused;
    
    systhread_mutex_lock(x->c_mutex);

    x->paused = true; // to avoid complications in audio thread. BEWARE: this does not guarantee thread safety. Need to ask andrea...
    x->curr_sample = 0;
    x->play_rate = play_rate;
    
    mpg123_handle* mh = x->mh;
    if (mh) mpg123_close(mh);

    // checking buffer size allocation
    if (play_rate > EARS_PLAYMP3_DEFAULT_MAX_PLAYRATE && x->buffer_size < x->buffer_size_wo_resampling * play_rate) {
        x->buffer_size = x->buffer_size_wo_resampling * ((long)ceil(play_rate));
        x->buffer = (unsigned char*) sysmem_resizeptr(x->buffer, x->buffer_size * sizeof(unsigned char));
    }
    
    char conformed_path[MAX_PATH_CHARS];
    ears_playmp3_ezlocate_file_char(path, conformed_path, NULL);
    // was:
//    path_nameconform(path, conformed_path, PATH_STYLE_MAX, PATH_TYPE_BOOT);
    
    
    // checking if name is correct
    long len = strlen(conformed_path);
    if (len >= 4 && strcmp(conformed_path + len - 4, ".mp3") == 0) {
        
        mpg123_format_none(mh);
        mpg123_format(mh, x->samplerate, MPG123_MONO | MPG123_STEREO, MPG123_ENC_FLOAT_32);
        //MPG123_ENC_FLOAT_32); // MPG123_ENC_UNSIGNED_8);
        
        mpg123_param(mh, MPG123_FORCE_RATE, x->samplerate, 0.);

/*        mpg123_getformat(mh, &orig_srate, &orig_channels, &orig_encoding);
        if (orig_srate != x->samplerate)
            object_warn((t_object *)x, "File sample rate differ from system's. Crude resampling.");
*/
        mpg123_open(mh, conformed_path);
        
        
        if (!mh) {
            // should never happen... hopefully...
            object_error((t_object *)x, "Cannot seem to open mp3 file.");
            err = MPG123_ERR;
        } else {
            x->playing = true;
            if (mpg123_getformat(mh, &srate, &channels, &encoding) == MPG123_OK) {
                x->channels = channels;
#ifdef CONFIGURATION_Development
//                object_post((t_object *)x,"Opened mp3 file - sample rate: %ld, channels: %ld, encoding: %ld", rate, channels, encoding);
#endif
                
                // seeking?
                long seek_to = start_ms * x->samplerate / 1000.;
                if (start_ms > 0)
                    if (mpg123_seek(mh, seek_to, SEEK_SET) < 0)
                        object_post((t_object *)x, "Could not seek position in mp3 file.");
                x->seek_to = seek_to;
                
                // scheduling end?
                if (end_ms >= 0) {
                    double dur_ms = end_ms - start_ms;
                    clock_fdelay(x->m_clock, dur_ms);
                    x->loop_ms = dur_ms;
                }
                
                x->need_send_bang = true;
            } else {
                mpg123_close(x->mh);
                x->playing = x->paused = paused = false;
                object_error((t_object *)x, "There are issue with mp3 file decoding.");
                err = MPG123_ERR;
            }
        }
    } else {
        mpg123_close(x->mh);
        x->playing = x->paused = paused = false;
        if (len == 0)
            object_error((t_object *)x, "Cannot locate file.");
        else
            object_error((t_object *)x, "File doesn't seem to have .mp3 extension.");
        err = MPG123_ERR;
    }
    
    x->paused = paused;
    systhread_mutex_unlock(x->c_mutex);
#endif
    
    return err;
}



int close_file(t_playmp3 *x)
{
#ifdef EARS_MP3_READ_SUPPORT
    systhread_mutex_lock(x->c_mutex);
    if (x->mh) mpg123_close(x->mh);
    x->playing = x->paused = false;
    x->lastsample[0] = x->lastsample[1] = 0.;
    x->need_send_bang = false;
    systhread_mutex_unlock(x->c_mutex);

    playmp3_senddonebang(x, NULL, 0, NULL);

    return MPG123_OK;
#else
    return 0;
#endif
}



void playmp3_play(t_playmp3 *x, t_symbol *s, long argc, t_atom *argv)
{
    if (argc >= 1 && atom_gettype(argv) == A_SYM) {
        double start_ms = (argc >= 2 ? atom_getfloat(argv + 1) : 0);
        double end_ms = (argc >= 3 ? atom_getfloat(argv + 2) : -1);
        double play_rate = (argc >= 4 ? atom_getfloat(argv + 3) : 1.);
        if (play_rate != 1. && play_rate != 0.)
            end_ms = (start_ms + (end_ms - start_ms) / play_rate);
        open_file(x, atom_getsym(argv)->s_name, start_ms, end_ms, play_rate);
    }
}

void playmp3_stop(t_playmp3 *x)
{
    x->lastsample[0] = x->lastsample[1] = 0.;
    clock_unset(x->m_clock);
    close_file(x);
}

void playmp3_pause(t_playmp3 *x)
{
    x->paused = true;
}

void playmp3_resume(t_playmp3 *x)
{
    x->paused = false;
}



void playmp3_assist(t_playmp3 *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) { // @in 0 @type anything @digest Messages to play, stop, pause, resume
		sprintf(s, "play/stop/pause/resume Messages");
	}
	else {	// outlet
        if (a < x->num_outs)
            sprintf(s, "Signal for Channel %ld", a); // @out 0 @loop 1 @type signal @digest Signal output channel
        else
            sprintf(s, "bang When Done Playing"); // @out -1 @type bang @digest bang when playing has ended
	}
}



// registers a function for the signal chain in Max
void playmp3_dsp64(t_playmp3 *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
#ifdef EARS_MP3_READ_SUPPORT
    x->samplerate = samplerate;
    mpg123_getformat(x->mh, &x->rate, &x->channels, &x->encoding);

#ifdef Configuration_DEVELOPMENT
//    post("Current sample rate: %.2f; rate: %ld, channels: %ld, encoding: %ld", samplerate, x->rate, x->channels, x->encoding);
#endif
    
    x->buffer_size_wo_resampling = MAX(mpg123_outblock(x->mh), (x->channels > 0 ? x->channels : 1) * 4 * maxvectorsize); // 32-bit = 4 bytes...
    x->buffer_size = x->buffer_size_wo_resampling * EARS_PLAYMP3_DEFAULT_MAX_PLAYRATE; // allocating a default multiple of the needed buffersize w/o resampling
    
    if (x->buffer) sysmem_freeptr(x->buffer);
    x->buffer = (unsigned char*) sysmem_newptr(x->buffer_size * sizeof(unsigned char));
    
    

	// instead of calling dsp_add(), we send the "dsp_add64" message to the object representing the dsp chain
	// the arguments passed are:
	// 1: the dsp64 object passed-in by the calling function
	// 2: the symbol of the "dsp_add64" message we are sending
	// 3: a pointer to your object
	// 4: a pointer to your 64-bit perform method
	// 5: flags to alter how the signal chain handles your object -- just pass 0
	// 6: a generic pointer that you can use to pass any additional data to your perform method

	object_method(dsp64, gensym("dsp_add64"), x, playmp3_perform64, 0, NULL);
#endif
}


void playmp3_senddonebang(t_playmp3 *x, t_symbol *s, long argc, t_atom *argv)
{
    outlet_bang(x->out[2]);
}



void playmp3_post(t_playmp3 *x, t_symbol *s, long argc, t_atom *argv)
{
    long len;
    char *buf = NULL;
    atom_gettext(argc, argv, &len, &buf, 0);
    object_post((t_object *)x, buf);
    sysmem_freeptr(buf);
}



// this is the 64-bit perform method audio vectors
void playmp3_perform64(t_playmp3 *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
#ifdef EARS_MP3_READ_SUPPORT
    if (x->play_rate != 1.) {
        playmp3_perform64_interp(x, dsp64, ins, numins, outs, numouts, sampleframes, flags, userparam);
        return;
    }
    
    t_double *outL = outs[0];	// we get audio for each outlet of the object from the **outs argument
    t_double *outR = numouts == 1 ? NULL : outs[1];	// we get audio for each outlet of the object from the **outs argument
    size_t done;
    int n = sampleframes;
    int channels = x->channels;
    size_t buffer_size = x->buffer_size;
    int res = MPG123_OK;
    long fadein_samples = x->fadein_samples, fadeout_samples = x->fadeout_samples; // fadeout unsupported for now
    long old_curr_sample = x->curr_sample;

    size_t memory_vector = channels * 4 * sampleframes;

    assert_debug(memory_vector <= buffer_size);
    
    
//    x->temp++;

    // readn bytes from mh handler and puts them into the buffer
    // 4 bytes because encoding is 32-bit float
    
    if (!x->paused) {
        res = mpg123_read(x->mh, x->buffer, memory_vector, &done);
        x->curr_sample += sampleframes;
    } else {
        done = 0;
        res = MPG123_DONE;
    }
    
//    t_atom av[2];
//    atom_setlong(av, done);
//    atom_setlong(av + 1, memory_vector);
//    schedule_defer(x, (method)playmp3_post, 0, NULL, 2, av);
 
    if (done > 0) { // we have something to write
        unsigned char *buffer = x->buffer;
        assert_debug(done <= buffer_size);
        assert_debug(done <= memory_vector);
        
#ifdef EARS_PLAYMP3_ENABLE_SAMPLEACCURATELOOPING
        if (x->loop && done < memory_vector) {
            // must add some initial samples
            long diff = memory_vector - done;
            size_t new_done = 0;
            mpg123_seek(x->mh, x->seek_to, SEEK_SET);
            mpg123_read(x->mh, x->buffer + done, diff, &new_done);
            done = MIN(done + new_done, memory_vector);
        }
#endif
        
        if (channels == 2) { // STEREO FILE
            int ns = done/8; // done / (sizeof(t_float)*channels)
            assert_debug(ns * 2 <= 2 * 4 * sampleframes);
            assert_debug(ns <= sampleframes);
            n = 0;
            
            if (old_curr_sample < fadein_samples) {  // fade in
                for (; n < ns; n++)
                    outL[n] = ((t_double) ((t_float *)buffer)[2 * n]) * CLAMP(((double)old_curr_sample + n)/fadein_samples, 0., 1.);
            } else {
                for (; n < ns; n++)
                    outL[n] = (t_double) ((t_float *)buffer)[2 * n];
            }
            for (; n < sampleframes; n++)
                outL[n] = 0.;
            
            if (outR) { // getting right channel
                n = 0;
                if (old_curr_sample < fadein_samples) { // fade in
                    for (; n < ns; n++)
                        outR[n] = ((t_double) ((t_float *)buffer)[2 * n + 1]) * CLAMP(((double)old_curr_sample + n)/fadein_samples, 0., 1.);
                } else {
                    for (; n < ns; n++)
                        outR[n] = (t_double) ((t_float *)buffer)[2 * n + 1];
                }
                for (; n < sampleframes; n++)
                    outR[n] = 0.;
            }
            
            
        } else if (channels == 1) { // MONO FILE
            int ns = done/4; // done / (sizeof(t_float)*channels)
            if (old_curr_sample < fadein_samples) { // fade in
                for (int n = 0; n < ns; n++)
                    outL[n] = (t_double) ((t_float *)buffer)[n] * CLAMP(((double)old_curr_sample + n)/fadein_samples, 0., 1.);
            } else {
                for (int n = 0; n < ns; n++)
                    outL[n] = (t_double) ((t_float *)buffer)[n];
            }
            for (; n < sampleframes; n++)
                outL[n] = 0.;
            if (outR) { // copying mono channel to stereo
                outL = outs[0];
                for (n = sampleframes; n > 0; n--)
                    *outR++ = *outL++;
            }
            
        } else { // UNSUPPORTED FILE
            for (n = sampleframes; n > 0; n--)
                *outL++ = 0.;
            if (outR) {
                for (n = 0; n > 0; n--)
                    *outR++ = 0.;
            }
        }

    } else {

        if (res == MPG123_DONE && x->need_send_bang) {
            if (x->loop) { // looping
                mpg123_seek(x->mh, x->seek_to, SEEK_SET);
            } else { // sending out bang
                x->need_send_bang = false;
                schedule_delay((t_object *)x, (method)playmp3_senddonebang, 0, NULL, 0, NULL);
            }
        }
        
        for (n = sampleframes; n > 0; n--)
            *outL++ = 0.;
        if (outR) {
            for (n = sampleframes; n > 0; n--)
                *outR++ = 0.;
        }
    }
#endif
}


// ch = channel = 0: mono, channel = 1: left, channel = 2: right
t_double playmp3_resample(t_playmp3 *x, t_float *buffer, long num_samples_in_buffer, double resampling_ratio, long n, int ch)
{
    // buffer has <num_samples_in_buffer> samples allocated
    // n goes from 0 to num_sample_frames-1
    double w = ((double)(n+1)) * resampling_ratio; ///num_sample_frames) * num_sample_frames_resampled;
    long f = (long)floor(w); // floor
    double r = w - f; // remainder
    double leftval = (f <= 0 ? (ch == 2 ? x->lastsample[1] : x->lastsample[0]) : (f > num_samples_in_buffer ? 0 : buffer[ch == 0 ? f-1 : (ch == 1 ? 2*(f-1) : 2*(f-1)+1)]));
    double rightval = (f >= num_samples_in_buffer ? 0 : buffer[ch == 0 ? f : (ch == 1 ? 2*f : 2*f+1)]);
    double res = leftval * (1 - r) + rightval * r;
    return res;
}

// this is the 64-bit perform method audio vectors
void playmp3_perform64_interp(t_playmp3 *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
#ifdef EARS_MP3_READ_SUPPORT
    t_double *outL = outs[0];    // we get audio for each outlet of the object from the **outs argument
    t_double *outR = numouts == 1 ? NULL : outs[1];    // we get audio for each outlet of the object from the **outs argument
    size_t done;
    int n = sampleframes;
    int channels = x->channels;
    size_t buffer_size = x->buffer_size;
    int res = MPG123_OK;
    long fadein_samples = x->fadein_samples, fadeout_samples = x->fadeout_samples; // fadeout unsupported for now
    long old_curr_sample = x->curr_sample;
    
    int sampleframes_res = (int)(x->play_rate * sampleframes);
    double resampling_ratio = ((double)sampleframes_res) / sampleframes; // not PRECISELY x->play_rate, since we approximate play_rate to the nearest sample
    
    size_t memory_vector = channels * 4 * sampleframes_res;
    
    assert_debug(memory_vector <= buffer_size);
    
    
    //    x->temp++;
    
    // readn bytes from mh handler and puts them into the buffer
    // 4 bytes because encoding is 32-bit float
    
    if (!x->paused) {
        res = mpg123_read(x->mh, x->buffer, memory_vector, &done);
        x->curr_sample += sampleframes_res;
    } else {
        done = 0;
        res = MPG123_DONE;
    }
    
    //    t_atom av[2];
    // atom_setlong(av, done);
    // atom_setlong(av + 1, memory_vector);
    // schedule_defer(x, (method)playmp3_post, 0, NULL, 2, av);
    
    if (done > 0) { // we have something to write
        unsigned char *buffer = x->buffer;
        assert_debug(done <= buffer_size);
        assert_debug(done <= memory_vector);
        
#ifdef EARS_PLAYMP3_ENABLE_SAMPLEACCURATELOOPING
        if (x->loop && done < memory_vector) {
            // must add some initial samples
            long diff = memory_vector - done;
            size_t new_done = 0;
            mpg123_seek(x->mh, x->seek_to, SEEK_SET);
            mpg123_read(x->mh, x->buffer + done, diff, &new_done);
            done = MIN(done + new_done, memory_vector);
        }
#endif
        
        if (channels == 2) { // STEREO FILE
            int ns = done/8; // done / (sizeof(t_float)*channels)
            assert_debug(ns * 2 <= 2 * 4 * sampleframes_res);
            assert_debug(ns <= sampleframes_res);
            n = 0;
            
            if (old_curr_sample < fadein_samples) {  // fade in
                for (; n < sampleframes; n++)
                    outL[n] = playmp3_resample(x, (t_float *)buffer, ns, resampling_ratio, n, 1) * CLAMP(((double)old_curr_sample + n)/fadein_samples, 0., 1.);
            } else {
                for (; n < sampleframes; n++)
                    outL[n] = playmp3_resample(x, (t_float *)buffer, ns, resampling_ratio, n, 1);
            }
            
            if (outR) { // getting right channel
                n = 0;
                if (old_curr_sample < fadein_samples) { // fade in
                    for (; n < sampleframes; n++)
                        outR[n] = playmp3_resample(x, (t_float *)buffer, ns, resampling_ratio, n, 2) * CLAMP(((double)old_curr_sample + n)/fadein_samples, 0., 1.);
                } else {
                    for (; n < sampleframes; n++)
                        outR[n] = playmp3_resample(x, (t_float *)buffer, ns, resampling_ratio, n, 2);
                }
                x->lastsample[0] = outL[sampleframes - 1];
                x->lastsample[1] = outR[sampleframes - 1];
            } else {
                x->lastsample[0] = x->lastsample[1] = outL[sampleframes - 1];
            }

        } else if (channels == 1) { // MONO FILE
            int ns = done/4; // done / (sizeof(t_float)*channels)

            if (old_curr_sample < fadein_samples) { // fade in
                for (int n = 0; n < sampleframes; n++)
                    outL[n] = playmp3_resample(x, (t_float *)buffer, ns, resampling_ratio, n, 0) * CLAMP(((double)old_curr_sample + n)/fadein_samples, 0., 1.);
            } else {
                for (int n = 0; n < sampleframes; n++)
                    outL[n] = playmp3_resample(x, (t_float *)buffer, ns, resampling_ratio, n, 0);
            }
            
            if (outR) { // copying mono channel to stereo
                outL = outs[0];
                for (n = sampleframes; n > 0; n--)
                    *outR++ = *outL++;
            }
            
            x->lastsample[0] = x->lastsample[1] = outL[sampleframes - 1];

        } else { // UNSUPPORTED FILE
            for (n = sampleframes; n > 0; n--)
                *outL++ = 0.;
            if (outR) {
                for (n = 0; n > 0; n--)
                    *outR++ = 0.;
            }
            x->lastsample[0] = x->lastsample[1] = 0;
        }
        
    } else {
        
        if (res == MPG123_DONE && x->need_send_bang) {
            if (x->loop) { // looping
                mpg123_seek(x->mh, x->seek_to, SEEK_SET);
            } else { // sending out bang
                x->need_send_bang = false;
                schedule_delay((t_object *)x, (method)playmp3_senddonebang, 0, NULL, 0, NULL);
            }
        }
        
        for (n = sampleframes; n > 0; n--)
            *outL++ = 0.;
        if (outR) {
            for (n = sampleframes; n > 0; n--)
                *outR++ = 0.;
        }
    }
#endif
}
