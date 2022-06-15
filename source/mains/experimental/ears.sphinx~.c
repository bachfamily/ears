/**
 @file
 dada.sphinx~ 
 
 @ingroup	dada	
 */

#define _JPATCHER_SYMS_H_ // It's a HACK! indeed ps_free creates a conflict between the pocketsphinx library and the cycling '74 library
#define MODELDIR "/Users/danieleghisi/Desktop/PocketSphynx/pocketsphinx/model" 

#define SPHINX_FRAMES_PER_SECOND 100

#include <pocketsphinx.h>

#include "foundation/llllobj.h" // you must include this.
#include "ext.h"
#include "ext_obex.h"
#include "ext_common.h" // contains CLAMP macro
#include "z_dsp.h"
#include "ext_buffer.h"
#include "ext_globalsymbol.h"
#include "dada.pocketsphinx.h"

/* System headers. */
#include <stdio.h>
#include <assert.h>

/* SphinxBase headers. */
#include <sphinxbase/err.h>
#include <sphinxbase/strfuncs.h>
#include <sphinxbase/filename.h>
#include <sphinxbase/pio.h>

/* Local headers. */
#include "cmdln_macro.h"
/*
 #include "pocketsphinx_internal.h"
#include "ps_lattice_internal.h"
#include "phone_loop_search.h"
#include "fsg_search_internal.h"
#include "ngram_search.h"
#include "ngram_search_fwdtree.h"
#include "ngram_search_fwdflat.h"
*/


typedef struct _sphinx {
	t_llllobj_pxobject l_obj;
	t_buffer_ref *l_buffer_reference;

	cmd_ln_t *config;
	ps_decoder_t *ps;

	t_symbol	*hmm;			///< Hidden Markov Model
	t_symbol	*lm;			///< Language Model
	t_symbol	*dict;			///< Dictionary
	
	long		dsratio;		///< Downsampling ratio
	
	t_symbol *hmm_fullpath;
	t_symbol *lm_fullpath;
	t_symbol *dict_fullpath;
	
	// settings
	char	outputmode;

	// realtime fields
	char	uttering;			///< 1 while uttering in realtime, 0 otherwise
	double	window_size_ms;		///< Window size in milliseconds
	long	window_size_samps;	///< Window size in samples
	long	curr_samp;			///< Current sample index in rt_buf
	int16	*rt_buf;			///< Realtime audio buffer
	
} t_sphinx;


void sphinx_perform64(t_sphinx *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam);
void sphinx_dsp64(t_sphinx *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags);
void sphinx_set(t_sphinx *x, t_symbol *s);
t_sphinx *sphinx_new(t_symbol *s, short ac, t_atom *av);
void sphinx_free(t_sphinx *x);
t_max_err sphinx_notify(t_sphinx *x, t_symbol *s, t_symbol *msg, void *sender, void *data);
void sphinx_assist(t_sphinx *x, void *b, long m, long a, char *s);
void sphinx_dblclick(t_sphinx *x);
void sphinx_anything(t_sphinx *x, t_symbol *msg, long ac, t_atom *av);
void sphinx_int(t_sphinx *x, t_atom_long v);

void sphinx_decode_buffer(t_sphinx *x);
void sphinx_decode_file(t_sphinx *x, t_symbol *file_name);

void sphinx_set_sample_rate(t_sphinx *x, double sr);

t_symbol *ezlocate_file(t_symbol *file_name);
// t_symbol *ezlocate_folder(t_symbol *folder_name);

void sphinx_output_word_info(t_sphinx *x, int32 start);
void sphinx_output_hypothesis(t_sphinx *x, const char *hyp);

static t_class *sphinx_class;


void C74_EXPORT ext_main(void* moduleRef)
{
	common_symbols_init();
	llllobj_common_symbols_init();
	
	if (llllobj_check_version(BACH_LLLL_VERSION) || llllobj_test()) {
		error("bach: bad installation");
		return 1;
	}
	
	t_class *c = class_new("dada.sphinx~", (method)sphinx_new, (method)sphinx_free, sizeof(t_sphinx), 0L, A_GIMME, 0);
	
	class_dspinit(c);

	class_addmethod(c, (method)sphinx_dsp64, "dsp64", A_CANT, 0);
	class_addmethod(c, (method)sphinx_set, "set", A_SYM, 0);
	class_addmethod(c, (method)sphinx_int, "int", A_LONG, 0);
	class_addmethod(c, (method)sphinx_assist, "assist", A_CANT, 0);
	class_addmethod(c, (method)sphinx_decode_file, "decodefile", A_SYM, 0);
	class_addmethod(c, (method)sphinx_decode_buffer, "decode", 0);
	class_addmethod(c, (method)sphinx_dblclick, "dblclick", A_CANT, 0);
	class_addmethod(c, (method)sphinx_notify, "bachnotify", A_CANT, 0);
	

	llllobj_class_add_out_attr(c, LLLL_OBJ_MSP); 

	CLASS_STICKY_ATTR(c,"category",0,"Settings");
	
	CLASS_ATTR_SYM(c,"hmm",0, t_sphinx, hmm);
	CLASS_ATTR_STYLE_LABEL(c,"hmm",0,"text","Hidden Markov Model File");
	
	CLASS_ATTR_SYM(c,"lm",0, t_sphinx, lm);
	CLASS_ATTR_STYLE_LABEL(c,"lm",0,"text","Language Model File");

	CLASS_ATTR_SYM(c,"dict",0, t_sphinx, dict);
	CLASS_ATTR_STYLE_LABEL(c,"dict",0,"text","Dictionary File");

	CLASS_ATTR_LONG(c,"dsratio",0, t_sphinx, dsratio);
	CLASS_ATTR_STYLE_LABEL(c,"dsratio",0,"text","Downsampling");

	CLASS_ATTR_CHAR(c,"outputmode",0, t_sphinx, outputmode);
	CLASS_ATTR_STYLE_LABEL(c,"outputmode",0,"onoff","Output As One Symbol");

	CLASS_ATTR_DOUBLE(c,"winsize",0, t_sphinx, window_size_ms);
	CLASS_ATTR_STYLE_LABEL(c,"winsize",0,"text","Real-time Detection Window Size (In Milliseconds)");

	CLASS_STICKY_ATTR_CLEAR(c, "category");
	
	class_register(CLASS_BOX, c);
	sphinx_class = c;
	
}


void sphinx_int(t_sphinx *x, t_atom_long v)
{
	char uttering = x->uttering;
	if (v && !uttering) {
		// start uttering!
		x->uttering = 1;
		if (ps_start_utt(x->ps, "dadautterance") < 0)
			object_error((t_object *)x, "Error: cannot start utterance.");
	} else if (!v && uttering) {
		// stop uttering
		if (ps_end_utt(x->ps) < 0)
			object_error((t_object *)x, "Error: cannot end utterance.");
		x->uttering = 0;

		char const *hyp, *uttid;
		int32 score;
		hyp = ps_get_hyp(x->ps, &score, &uttid);
//		hyp = ps_get_hyp_final(x->ps, NULL);
		if (hyp == NULL) {
			object_error((t_object *)x, "Error: can't decode");
			return;
		}
		sphinx_output_word_info(x, 0);
		sphinx_output_hypothesis(x, hyp);
	}
}

long		DS = 1;

void sphinx_perform64(t_sphinx *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, 
					  long sampleframes, long flags, void *userparam)
{
	
	char uttering = x->uttering;
	if (uttering) {
		double		*in = ins[0];     // first inlet
		long		i, j; 

		for (i = 0, j = 0; j < sampleframes; i++, j += DS)
			x->rt_buf[i] = 0; //(int16)floor(in[j] * 32767);

		int num_processed_frames = ps_process_raw(x->ps, x->rt_buf, sampleframes / DS, TRUE, FALSE);
		post("––– PROCESS %d frames, %d samples", num_processed_frames, sampleframes / DS);

		
		
		/*
		long curr_samp = x->curr_samp;
		long win_size_samps = x->window_size_samps;
		if (curr_samp + sampleframes < win_size_samps) {
			for (i = curr_samp, j = 0; j < sampleframes && i < win_size_samps; i++, j += DS)
				x->rt_buf[i] = (int16)floor(in[j] * 32767);
			// nothing else to do, we don't process now.
			x->curr_samp += sampleframes;
			post("Nothingtodo");
		} else {
			// we need to process, rt_buf is gonna be full
			for (i = curr_samp, j = 0; j < sampleframes && i < win_size_samps; i++, j += DS)
				x->rt_buf[i] = (int16)floor(in[j] * 32767);
			
			int num_processed_frames = ps_process_raw(x->ps, x->rt_buf, win_size_samps / DS, TRUE, FALSE);
			post("––– PROCESS %d frames, %d samples", num_processed_frames, win_size_samps);
			
			vad_state = ps_get_vad_state(ps);
			if (vad_state && !cur_vad_state) {
				//silence -> speech transition,
				// let user know that he is heard
				printf("Listening...\n");
				fflush(stdout);
			}
			if (!vad_state && cur_vad_state) {
				//speech -> silence transition, 
				//time to start new utterance
				ps_end_utt(ps); 
			
			
			
			for (i = 0; j < sampleframes && i < win_size_samps; i++, j += DS)
				x->rt_buf[i] = (int16)floor(in[j] * 32767);
			x->curr_samp = i;
		} */
	
	}
}

void sphinx_anything(t_sphinx *x, t_symbol *msg, long ac, t_atom *av)
{
	t_llll *parsed = llllobj_parse_llll((t_object *) x, LLLL_OBJ_MSP, msg, ac, av, LLLL_PARSE_CLONE);
	if (parsed && parsed->l_head && hatom_gettype(&parsed->l_head->l_hatom) == H_SYM) {
		t_symbol *router = hatom_getsym(&parsed->l_head->l_hatom);
		llll_destroyelem(parsed->l_head);
		
		if (router == gensym("decodefile") && parsed->l_head && hatom_gettype(&parsed->l_head->l_hatom) == A_SYM)
			sphinx_decode_file(x, hatom_getsym(&parsed->l_head->l_hatom));
		
	}
	llll_free(parsed);
}



// here's where we set the buffer~ we're going to access
void sphinx_set(t_sphinx *x, t_symbol *s)
{
	if (!x->l_buffer_reference)
		x->l_buffer_reference = buffer_ref_new((t_object*)x, s);
	else
		buffer_ref_set(x->l_buffer_reference, s);
}


void sphinx_dsp64(t_sphinx *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
	samplerate = samplerate / DS;
	sphinx_set_sample_rate(x, samplerate);
	x->window_size_samps = round(samplerate * x->window_size_ms / 1000.);
	if (x->rt_buf)
		sysmem_freeptr(x->rt_buf);
	x->rt_buf = (int16 *)sysmem_newptr(x->window_size_samps * sizeof(int16));

    dsp_add64(dsp64, (t_object*)x, (t_perfroutine64)sphinx_perform64, 0, NULL);
}


// this lets us double-click on sphinx~ to open up the buffer~ it references
void sphinx_dblclick(t_sphinx *x)
{
	buffer_view(buffer_ref_getobject(x->l_buffer_reference));
}

void sphinx_assist(t_sphinx *x, void *b, long m, long a, char *s)
{
	if (m == ASSIST_OUTLET)
		sprintf(s,"(signal) Sample Value at Index");
	else {
		switch (a) {	
			case 0:	sprintf(s,"Detected Words");	break;
			case 1:	sprintf(s,"Audio Channel In buffer~");	break;
		}
	}
}

t_sphinx *sphinx_new(t_symbol *s, short ac, t_atom *av)
{
	t_sphinx *x = NULL;
	if (x = (t_sphinx *)object_alloc(sphinx_class)) {
		
		// default files
/*		x->hmm = gensym(MODELDIR "/hmm/en_US/hub4wsj_sc_8k");
		x->lm = gensym(MODELDIR "/lm/en_US/hub4.5000.DMP");
		x->dict = gensym(MODELDIR "/lm/en_US/hub4.5000.dic");*/
		x->hmm = gensym("hub4wsj_sc_8k");
		x->lm = gensym("hub4.5000.DMP");
		x->dict = gensym("hub4.5000.dic");
		x->hmm_fullpath = gensym(MODELDIR "/hmm/en_US/hub4wsj_sc_8k");
		x->lm_fullpath = ezlocate_file(x->lm);
		x->dict_fullpath = ezlocate_file(x->dict);
		x->dsratio = 1;
		x->window_size_ms = 200;
		x->rt_buf = NULL;
		
		attr_args_process(x, ac, av); // this must be called before llllobj_obj_setup

		llllobj_pxobj_setup((t_llllobj_pxobject *) x, 1, "44a", NULL); // "4" means that we have one outlet, and it will output lllls according to the out attribute

		dsp_setup((t_pxobject *)x, 1);
		
		if (ac && av && atom_gettype(av) == A_SYM)
			sphinx_set(x, atom_getsym(av));

		// initializing decoder
		x->config = cmd_ln_init(NULL, ps_args(), TRUE,
							 "-hmm", x->hmm_fullpath->s_name,
							 "-lm", x->lm_fullpath->s_name,
							 "-dict", x->dict_fullpath->s_name,
							 "-samprate", "16000",
							 NULL);
		if (x->config)
			x->ps = ps_init(x->config);
		
		if (!x->config || !x->ps) {
			object_free(x);
			x = NULL;
		}
		
	}
	
	return (x);
}


void sphinx_free(t_sphinx *x)
{
	dsp_free((t_pxobject*)x);
	ps_free(x->ps);
	if (x->rt_buf)
		sysmem_freeptr(x->rt_buf);
	object_free(x->l_buffer_reference);
}


t_max_err sphinx_notify(t_sphinx *x, t_symbol *s, t_symbol *msg, void *sender, void *data)
{
	if (msg == _sym_attr_modified) {
		t_symbol *attr_name = (t_symbol *)object_method((t_object *)data, _sym_getname);

		if (attr_name == gensym("hmm")) {
			x->hmm_fullpath = ezlocate_file(x->hmm);
			if (x->hmm_fullpath)
				cmd_ln_set_str_r(x->config, "-hmm", x->hmm_fullpath->s_name);
			else
				object_error((t_object *)x, "Error: file not found!");
			
		} else if (attr_name == gensym("lm")) {
			x->lm_fullpath = ezlocate_file(x->lm);
			if (x->lm_fullpath)
				cmd_ln_set_str_r(x->config, "-lm", x->lm_fullpath->s_name);
			else
				object_error((t_object *)x, "Error: file not found!");

		} else if (attr_name == gensym("dict")) {
			x->dict_fullpath = ezlocate_file(x->dict);
			if (x->dict_fullpath)
				cmd_ln_set_str_r(x->config, "-dict", x->dict_fullpath->s_name);
			else
				object_error((t_object *)x, "Error: file not found!");

		} else if (attr_name == gensym("dsratio")) {
			cmd_ln_set_int_r(x->config, "-dsratio", x->dsratio);
		}
		
		if (x->config)
			if (ps_reinit(x->ps, x->config))
				object_error((t_object *)x, "Error: could not change attribute!");

	}
	
	return buffer_ref_notify(x->l_buffer_reference, s, msg, sender, data);
}


void sphinx_output_word_info(t_sphinx *x, int32 start)
{
	// We only process information if the outlet is NOT disabled
	t_llllobj_out *out = llllobj_get_out((t_object *)x, LLLL_OBJ_MSP);
	if (out[1].b_type != LLLL_O_DISABLED) {
		t_llll *info = llll_get();
		ps_seg_t *iter = ps_seg_iter(x->ps, NULL);
		
		while (iter != NULL) {
			int32 sf, ef, pprob;
			float conf;
			t_llll *this_info = llll_get();
			t_llll *this_times = llll_get();
			
			ps_seg_frames(iter, &sf, &ef);
			pprob = ps_seg_prob(iter, NULL, NULL, NULL);
			conf = logmath_exp(ps_get_logmath(x->ps), pprob);
			
			llll_appendsym(this_info, gensym(ps_seg_word(iter)), 0, WHITENULL_llll);
			llll_appenddouble(this_times, 1000. * (sf + start) / SPHINX_FRAMES_PER_SECOND, 0, WHITENULL_llll);
			llll_appenddouble(this_times, 1000. * (ef + start) / SPHINX_FRAMES_PER_SECOND, 0, WHITENULL_llll);
			llll_appendllll(this_info, this_times, 0, WHITENULL_llll);
			llll_appenddouble(this_info, conf, 0, WHITENULL_llll);
			llll_appendllll(info, this_info, 0, WHITENULL_llll);
			
			//		post("%s %f %f %f\n", ps_seg_word(iter), (sf + start) / 100.0, (ef + start) / 100.0, conf);
			iter = ps_seg_next(iter);
			
		}
		
		llllobj_outlet_llll((t_object *)x, LLLL_OBJ_MSP, 1, info);
		llll_free(info);
	}
}	

void sphinx_output_hypothesis(t_sphinx *x, const char *hyp)
{
	if (x->outputmode) {
		// output as one symbol
		t_symbol *s = gensym(hyp);
		llllobj_outlet_anything((t_object *)x, LLLL_OBJ_MSP, 0, s, 0, NULL);
	} else {
		long max_num_tokens = 10000;
		t_atom av[max_num_tokens];
		long ac = 0;
		char* pch;
		char *str = strdup(hyp);
		pch = strtok(str," ");
		while (pch != NULL && ac < max_num_tokens)
		{
			atom_setsym(av+ac, gensym(pch));
			ac++;
			pch = strtok (NULL, " ");
		}
		llllobj_outlet_anything((t_object *)x, LLLL_OBJ_MSP, 0, _sym_list, ac, av);
	}
}

void sphinx_set_sample_rate(t_sphinx *x, double sr)
{
	cmd_ln_set_float_r(x->config, "-samprate", sr);
}

void sphinx_decode_buffer(t_sphinx *x)
{
	
	t_buffer_obj *buffer = buffer_ref_getobject(x->l_buffer_reference);
	t_atom_long nsamp = buffer_getframecount(buffer);
	t_atom_long nc = buffer_getchannelcount(buffer);
	t_atom_float sr = buffer_getsamplerate(buffer);
	t_float		*tab;
	char const *hyp, *uttid;
	int rv;
	int32 score;
	
//	sphinx_set_sample_rate(x, sr);

	if (sr != 16000.) {
		// throw an error!
		object_error((t_object *)x, "Error: only 16kHz sample rate supported for decoding.");
		return;
	} 
	if (nc != 1) {
		// throw a warning!
		object_warn((t_object *)x, "Warning: only mono buffers are supported for decoding. Considering first channel only.");
	}
	
	tab = buffer_locksamples(buffer);
	if (!tab) {
		object_error((t_object *)x, "Error: can't read buffer");
		return;
	}	
	
	rv = ps_start_utt(x->ps, "dadautterance");
	if (rv < 0) {
		object_error((t_object *)x, "Error: can't decode");
		return;
	}
	
	const long maxsamples = 4800000; // CMU doesn't handle more than 32k frames (~ 5 minutes @ 100fps).
	int16 *buf = (int16 *)sysmem_newptr(maxsamples * sizeof(int16));
	long i;
	if (nc > 1) {
		for (i = 0; i < nsamp; i++) 
			buf[i] = (int16)floor(tab[i * nc] * 32767);
	} else {
		for (i = 0; i < nsamp; i++) 
			buf[i] = (int16)floor(tab[i] * 32767);
	}

	rv = ps_process_raw(x->ps, buf, nsamp, TRUE, FALSE);

	rv = ps_end_utt(x->ps);
	if (rv < 0) {
		object_error((t_object *)x, "Error: can't decode");
		return;
	}

	
	hyp = ps_get_hyp(x->ps, &score, &uttid);
	if (hyp == NULL) {
		object_error((t_object *)x, "Error: can't decode");
		return;
	}

	buffer_unlocksamples(buffer);
	sysmem_freeptr(buf);
	
	sphinx_output_word_info(x, 0);
	sphinx_output_hypothesis(x, hyp);

/*	fseek(fh, 0, SEEK_SET);
	rv = ps_start_utt(x->ps, "goforward");
	if (rv < 0) {
		object_error((t_object *)x, "Error: can't decode");
		return;
	}
	while (!feof(fh)) {
		size_t nsamp;
		nsamp = fread(buf, 2, 512, fh);
		rv = ps_process_raw(x->ps, buf, nsamp, FALSE, FALSE);
	}
	rv = ps_end_utt(x->ps);
	if (rv < 0) {
		object_error((t_object *)x, "Error: can't decode");
		return;
	}
	hyp = ps_get_hyp(x->ps, &score, &uttid);
	if (hyp == NULL) {
		object_error((t_object *)x, "Error: can't decode");
		return;
	} */


}



// Only works with 16-bit 16kHz files
void sphinx_decode_file(t_sphinx *x, t_symbol *file_name)
{
	FILE *fh = NULL;
	char const *hyp, *uttid;
	int rv;
	int32 score;
	
	if (!file_name) {
		object_error((t_object *)x, "Error: can't open file");
		return;
	}
	
	t_symbol *full_path = ezlocate_file(file_name);
	if (full_path)
		fh = fopen(full_path->s_name, "rb");
	
	if (fh == NULL) {
		object_error((t_object *)x, "Error: can't open file");
		return;
	}
	
	rv = ps_decode_raw(x->ps, fh, "dadautterance", -1);
	if (rv < 0) {
		object_error((t_object *)x, "Error: no samples found");
		return;
	}
	hyp = ps_get_hyp(x->ps, &score, &uttid);
	if (hyp == NULL)  {
		object_error((t_object *)x, "Error: no decoding hypothesis available");
		return;
	}
	
	sphinx_output_word_info(x, 0);
	sphinx_output_hypothesis(x, hyp);

	fclose(fh);
}

/*
 t_symbol *ezlocate_folder(t_symbol *folder_name)
{
	
	char foldername[MAX_FILENAME_CHARS];
	short path = 0;
	t_fourcc type;
	
	if (!folder_name)
		return NULL;
	
	if (path_frompathname(folder_name->s_name, &path, foldername)) {
		t_fourcc type;
		strncpy_zero(foldername, folder_name->s_name, MAX_FILENAME_CHARS); 
		if (locatefile_extended(foldername, &path, &type, NULL, 0)) {
			error("folder %s not found", foldername);
		} else {
			post("folder %s, path %d", foldername, path);
		}
	} else {
		char foldernameok[MAX_FILENAME_CHARS];
		char foldernameok2[MAX_FILENAME_CHARS];
		path_topathname(path, foldername, foldernameok);
		path_nameconform(foldernameok, foldernameok2, PATH_STYLE_MAX, PATH_TYPE_BOOT);
		return gensym(foldernameok2);
	}
	
	return NULL;
}
*/
t_symbol *ezlocate_file(t_symbol *file_name)
{
	char filename[MAX_FILENAME_CHARS];
	short path = 0;

	if (!file_name)
		return NULL;
	
	if (path_frompathname(file_name->s_name, &path, filename)) {
		t_fourcc type;
		char file_path_str[MAX_FILENAME_CHARS]; 
		strncpy_zero(file_path_str, file_name->s_name, MAX_FILENAME_CHARS); 
		if (!locatefile_extended(file_path_str, &path, &type, &type, -1))  {
			char filenameok2[MAX_FILENAME_CHARS];
			path_topathname(path, file_path_str, filename);
			path_nameconform(filename, filenameok2, PATH_STYLE_MAX, PATH_TYPE_BOOT);
			return gensym(filenameok2);
		}
	} else {
		char filenameok[MAX_FILENAME_CHARS];
		char filenameok2[MAX_FILENAME_CHARS];
		path_topathname(path, filename, filenameok);
		path_nameconform(filenameok, filenameok2, PATH_STYLE_MAX, PATH_TYPE_BOOT);
		return gensym(filenameok2);
	}
	
	return NULL;
}
