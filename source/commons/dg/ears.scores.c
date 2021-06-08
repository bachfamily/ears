#include "ears.scores.h"
#include <string>
#include <sstream>
#include <iostream>


#define LOCK_BIT 1
#define MUTE_BIT 2
#define SOLO_BIT 4

t_llll *ears_sliceheader(t_llll *gs)
{
    // returns the header, sliced from the incoming gs
    t_llll *out = NULL;
    t_llllelem *nhel = get_first_nonheader_elem(gs);
    if (!nhel) {
        out = llll_clone(gs);
        llll_clear(gs);
    } else {
        t_llllelem *elem, *nextelem = NULL;
        out = llll_get();
        for (elem = gs->l_head; elem && elem != nhel; elem = elem->l_next)
            llll_appendhatom_clone(out, &elem->l_hatom);
        for (elem = gs->l_head; elem && elem != nhel; elem = nextelem) {
            nextelem = elem->l_next;
            llll_destroyelem(elem);
        }
    }
    
    return out;
}


char should_voice_be_bounced(char there_are_solos, char flag)
{
    if ((flag & MUTE_BIT) > 0)
        return 0;
    
    return 1;
}


char should_be_bounced(char there_are_solos, char flag)
{
    if ((flag & MUTE_BIT) > 0)
        return 0;
    
    if (there_are_solos)
        return ((flag & SOLO_BIT) > 0);
    else
        return 1;
}

char ears_roll_there_are_solos(t_llll *body)
{
    for (t_llllelem *voice_el = body->l_head; voice_el; voice_el = voice_el->l_next) {
        if (hatom_gettype(&voice_el->l_hatom) != H_LLLL)
            continue;
        
        t_llll *voice_ll = hatom_getllll(&voice_el->l_hatom);
        if (voice_ll->l_tail && hatom_gettype(&voice_ll->l_tail->l_hatom) == H_LONG && (hatom_getlong(&voice_ll->l_tail->l_hatom) & SOLO_BIT))
            return true;
        
        for (t_llllelem *chord_el = voice_ll->l_head; chord_el; chord_el = chord_el->l_next) {
            if (hatom_gettype(&chord_el->l_hatom) != H_LLLL)
                continue;
            
            t_llll *chord_ll = hatom_getllll(&chord_el->l_hatom);
            if (chord_ll->l_tail && hatom_gettype(&chord_ll->l_tail->l_hatom) == H_LONG && (hatom_getlong(&chord_ll->l_tail->l_hatom) & SOLO_BIT))
                return true;

            for (t_llllelem *note_el = chord_ll->l_head; note_el; note_el = note_el->l_next) {
                if (hatom_gettype(&note_el->l_hatom) != H_LLLL)
                    continue;
                t_llll *note_ll = hatom_getllll(&note_el->l_hatom);
                if (note_ll->l_tail && note_ll->l_size > 3 && hatom_gettype(&note_ll->l_tail->l_hatom) == H_LONG && (hatom_getlong(&note_ll->l_tail->l_hatom) & SOLO_BIT))
                    return true;
            }
        }
    }
    
    return false;
}


t_symbol *get_filename_from_note_llll(t_llll *ll, long filename_slot)
{
    for (t_llllelem *el = ll->l_head; el; el = el->l_next) {
        if (hatom_gettype(&el->l_hatom) == H_LLLL) {
            t_llll *subll = hatom_getllll(&el->l_hatom);
            if (subll && subll->l_head && hatom_gettype(&subll->l_head->l_hatom) == H_SYM && hatom_getsym(&subll->l_head->l_hatom) == _llllobj_sym_slots) {
                
                for (t_llllelem *slotel = subll->l_head; slotel; slotel = slotel->l_next) {
                    if (hatom_gettype(&slotel->l_hatom) == H_LLLL) {
                        t_llll *slotll = hatom_getllll(&slotel->l_hatom);
                        if (slotll && slotll->l_head && hatom_getlong(&slotll->l_head->l_hatom) == filename_slot) {
                            if (slotll->l_size > 2 && hatom_gettype(&slotll->l_tail->l_hatom) == H_LONG) {
                                long selected_item = hatom_getlong(&slotll->l_tail->l_hatom);
                                t_llllelem *okel = llll_getindex(slotll, selected_item + 1, I_STANDARD);
                                if (okel)
                                    return hatom_getsym(&okel->l_hatom);
                            } else {
                                if (slotll->l_head->l_next)
                                    return hatom_getsym(&slotll->l_head->l_next->l_hatom);
                                else
                                    return NULL;
                            }
                        }
                    }
                }
                
            }
        }
    }
    return NULL;
}




t_llll *get_slot_from_note_llll(t_llll *ll, long slotnum)
{
    t_llll *out = NULL;
    for (t_llllelem *el = ll->l_head; el; el = el->l_next) {
        if (hatom_gettype(&el->l_hatom) == H_LLLL) {
            t_llll *subll = hatom_getllll(&el->l_hatom);
            if (subll && subll->l_head && hatom_gettype(&subll->l_head->l_hatom) == H_SYM && hatom_getsym(&subll->l_head->l_hatom) == _llllobj_sym_slots) {
                
                for (t_llllelem *slotel = subll->l_head; slotel; slotel = slotel->l_next) {
                    if (hatom_gettype(&slotel->l_hatom) == H_LLLL) {
                        t_llll *slotll = hatom_getllll(&slotel->l_hatom);
                        if (slotll && slotll->l_head && hatom_getlong(&slotll->l_head->l_hatom) == slotnum) {
                            out = llll_clone(slotll);
                            llll_behead(out);
                            return out;
                        }
                    }
                }
                
            }
        }
    }
    return out;
}

double get_slot_from_note_as_double(t_llll *ll, long slotnum, double defaultval, long index = 1, char *has_slot = NULL)
{
    double res = defaultval;
    if (has_slot) *has_slot = false;
    if (slotnum <= 0)
        return res;
    for (t_llllelem *el = ll->l_head; el; el = el->l_next) {
        if (hatom_gettype(&el->l_hatom) == H_LLLL) {
            t_llll *subll = hatom_getllll(&el->l_hatom);
            if (subll && subll->l_head && hatom_gettype(&subll->l_head->l_hatom) == H_SYM && hatom_getsym(&subll->l_head->l_hatom) == _llllobj_sym_slots) {
                
                for (t_llllelem *slotel = subll->l_head; slotel; slotel = slotel->l_next) {
                    if (hatom_gettype(&slotel->l_hatom) == H_LLLL) {
                        t_llll *slotll = hatom_getllll(&slotel->l_hatom);
                        if (slotll && slotll->l_head && hatom_getlong(&slotll->l_head->l_hatom) == slotnum) {
                            if (has_slot) *has_slot = true;
                            if (index > 1) {
                                t_llllelem *slel = llll_getindex(slotll, index + 1, I_STANDARD);
                                if (slel && is_hatom_number(&slel->l_hatom)) {
                                    res = hatom_getdouble(&slel->l_hatom);
                                    return res;
                                }
                            } else {
                                if (slotll->l_head->l_next && is_hatom_number(&slotll->l_head->l_next->l_hatom)) {
                                    res = hatom_getdouble(&slotll->l_head->l_next->l_hatom);
                                    return res;
                                }
                            }
                        }
                    }
                }
                
            }
        }
    }
    return res;
}


t_llll *get_voicenames_field_from_header(t_llll *header)
{
    for (t_llllelem *el = header->l_head; el; el = el->l_next) {
        if (hatom_gettype(&el->l_hatom) == H_LLLL) {
            t_llll *subll = hatom_getllll(&el->l_hatom);
            if (subll && subll->l_head && hatom_gettype(&subll->l_head->l_hatom) == H_SYM && hatom_getsym(&subll->l_head->l_hatom) == _llllobj_sym_voicenames) {
                return llll_clone(subll);
            }
        }
    }
    return NULL;
}

t_llll *get_slotinfo_field_from_header(t_llll *header, long slotnum, t_symbol *field)
{
    t_llll *out = NULL;
    for (t_llllelem *el = header->l_head; el; el = el->l_next) {
        if (hatom_gettype(&el->l_hatom) == H_LLLL) {
            t_llll *subll = hatom_getllll(&el->l_hatom);
            if (subll && subll->l_head && hatom_gettype(&subll->l_head->l_hatom) == H_SYM && hatom_getsym(&subll->l_head->l_hatom) == _llllobj_sym_slotinfo) {
                
                for (t_llllelem *slotel = subll->l_head; slotel; slotel = slotel->l_next) {
                    if (hatom_gettype(&slotel->l_hatom) == H_LLLL) {
                        t_llll *slotll = hatom_getllll(&slotel->l_hatom);
                        if (slotll && slotll->l_head && hatom_getlong(&slotll->l_head->l_hatom) == slotnum) {
                            
                            for (t_llllelem *slotinfoel = slotll->l_head; slotinfoel; slotinfoel = slotinfoel->l_next) {
                                if (hatom_gettype(&slotinfoel->l_hatom) == H_LLLL) {
                                    t_llll *slotinfofield = hatom_getllll(&slotinfoel->l_hatom);
                                    if (slotinfofield && slotinfofield->l_head && hatom_gettype(&slotinfofield->l_head->l_hatom) == H_SYM &&
                                        hatom_getsym(&slotinfofield->l_head->l_hatom) == field) {
                                        
                                        out = llll_clone(slotinfofield);
                                        llll_behead(out);
                                        return out;
                                        
                                    }
                                }
                            }
                        }
                    }
                }
                
            }
        }
    }
    return out;
}



void get_range_from_header(t_llll *header, long slotnum, double *min, double *max)
{
    t_llll *range_ll = get_slotinfo_field_from_header(header, slotnum, _sym_range);
    if (range_ll && range_ll->l_head && is_hatom_number(&range_ll->l_head->l_hatom))
        *min = hatom_getdouble(&range_ll->l_head->l_hatom);
    if (range_ll && range_ll->l_head && range_ll->l_head->l_next && is_hatom_number(&range_ll->l_head->l_next->l_hatom))
        *max = hatom_getdouble(&range_ll->l_head->l_next->l_hatom);
    llll_free(range_ll);
}

char is_slot_in_decibel(t_llll *header, long slotnum)
{
    char ans = 0;
    t_llll *repr_ll = get_slotinfo_field_from_header(header, slotnum, _llllobj_sym_representation);
    if (repr_ll && repr_ll->l_head && hatom_gettype(&repr_ll->l_head->l_hatom) == H_SYM) {
        t_symbol *s = hatom_getsym(&repr_ll->l_head->l_hatom);
        if (strcasecmp(s->s_name, "db") == 0 || strcasecmp(s->s_name, "dbs") == 0 || strcasecmp(s->s_name, "decibel") == 0 || strcasecmp(s->s_name, "decibels") == 0)
            ans = 1;
    }
    llll_free(repr_ll);
    return ans;
}

char is_slot_temporal_absolute(t_llll *header, long slotnum)
{
    char ans = 0;
    t_llll *repr_ll = get_slotinfo_field_from_header(header, slotnum, _llllobj_sym_temporalmode);
    if (repr_ll && repr_ll->l_head && hatom_gettype(&repr_ll->l_head->l_hatom) == H_SYM) {
        t_symbol *s = hatom_getsym(&repr_ll->l_head->l_hatom);
        if (strcasecmp(s->s_name, "milliseconds") == 0 || strcasecmp(s->s_name, "ms") == 0)
            ans = 1;
    }
    llll_free(repr_ll);
    return ans;
}


t_ears_err ears_roll_to_buffer_get_buffer(t_earsbufobj *e_ob,
                                          std::vector<t_ears_note_buffer> &noteinfo,
                                          t_buffer_obj **buf,
                                          char        *is_new,
                                          t_symbol    *filename,
                                          double      rate,
                                          double      start_ms,
                                          double      end_ms,
                                          t_llll      *breakpoints,
                                          t_llll      *gain_env,
                                          t_llll      *pan_env,
                                          double      voice_pan,
                                          double      fadein_amount,
                                          double      fadeout_amount,
                                          double      transp_mc,
                                          double      stretch_factor,
                                          double      sr,
                                          long        *buffer_index,
                                          bool optimize_for_identical_samples)
{
    if (optimize_for_identical_samples) {
        for (long i = 0; i < noteinfo.size(); i++) {
            if (noteinfo[i].filename == filename &&
                noteinfo[i].rate == rate &&
                noteinfo[i].start_ms == start_ms &&
                noteinfo[i].end_ms == end_ms &&
                ((!noteinfo[i].breakpoints && !breakpoints) || llll_eq_ignoretype(noteinfo[i].breakpoints, breakpoints)) &&
                ((!noteinfo[i].gain_env && !gain_env) || llll_eq_ignoretype(noteinfo[i].gain_env, gain_env)) &&
                ((pan_env && llll_eq_ignoretype(noteinfo[i].pan_env, pan_env)) || (!pan_env && noteinfo[i].voice_pan == voice_pan)) &&
                noteinfo[i].fadein_amount == fadein_amount &&
                noteinfo[i].fadeout_amount == fadeout_amount &&
                noteinfo[i].transp_mc == transp_mc &&
                noteinfo[i].stretch_factor == stretch_factor) {
                
                *buf = noteinfo[i].buffer;
                *is_new = 0;
                return EARS_ERR_NONE;
            }
        }
    }
    
    // not found: new item
    t_ears_err this_err = EARS_ERR_NONE;
    t_ears_note_buffer newinfo;
    if (optimize_for_identical_samples) {
        newinfo.filename = filename;
        newinfo.rate = rate;
        newinfo.start_ms = start_ms;
        newinfo.end_ms = end_ms;
        newinfo.breakpoints = breakpoints;
        newinfo.gain_env = gain_env;
        newinfo.pan_env = pan_env;
        newinfo.voice_pan = voice_pan;
        newinfo.fadein_amount = fadein_amount;
        newinfo.fadeout_amount = fadeout_amount;
        newinfo.transp_mc = transp_mc;
        newinfo.stretch_factor = stretch_factor;
    }
    
    if (!ears_ezlocate_file(filename, NULL) && ears_buffer_symbol_is_buffer(filename)) {
        // it's a buffer!
        this_err = ears_buffer_from_buffer((t_object *)e_ob, buf, filename, start_ms, end_ms, (*buffer_index)++);
   } else {
        // it's (possibly) a file!
        this_err = ears_buffer_from_file((t_object *)e_ob, buf, filename, start_ms, end_ms, (*buffer_index)++);
    }
    
    newinfo.buffer = *buf;
    
    if (rate != 1) {
        ears_buffer_resample((t_object *)e_ob, *buf, 1./rate, 11);
    }
    /*                    if (ps_slot || ts_slot) {
     t_llll *ps_env = get_slot_from_note_llll(note_ll, ps_slot);
     t_llll *ts_env = get_slot_from_note_llll(note_ll, ts_slot);
     if (!ps_env) {
     ps_env = llll_get();
     llll_appendlong(ps_env, 0);
     }
     if (!ts_env) {
     ts_env = llll_get();
     llll_appendlong(ps_env, 1);
     }
     
     ears_llll_to_env_samples(ps_env, ears_ms_to_samps(note_duration_ms, sr), sr, e_ob->l_envtimeunit);
     ears_llll_to_env_samples(ts_env, ears_ms_to_samps(note_duration_ms, sr), sr, e_ob->l_envtimeunit);
     
     //                        ears_buffer_rubberband((t_object *)e_ob, buf, buf, ts_env, ps_env, buf_rubberband_get_default_options(), 8820, earsbufobj_get_slope_mapping(e_ob));
     
     llll_free(ps_env);
     llll_free(ts_env);
     } */
    
    if (this_err) {
        object_error((t_object *)e_ob, "Error loading file %s", filename ? filename->s_name : "");
    }
    
    if (optimize_for_identical_samples)
        noteinfo.push_back(newinfo);
    
    *is_new = 1;

    return this_err;
}


                                          
// mode: sinusoids or samples
t_ears_err ears_roll_to_buffer(t_earsbufobj *e_ob, e_ears_scoretobuf_mode mode, t_llll *roll_gs, t_buffer_obj *dest,
                               e_ears_synthmode synthmode, float *wavetable, long wavetable_length,
                               char use_mute_solos, char use_durations,
                               long num_channels,
                               long filename_slot, long offset_slot, long gain_slot, long pan_slot, long rate_slot,
                               // these two below are currently unused
                               long ps_slot, long ts_slot,
                               double sr, e_ears_normalization_modes normalization_mode, e_ears_channel_convert_modes convertchannelsmode,
                               double fadein_amount, double fadeout_amount, e_ears_fade_types fade_in_type, e_ears_fade_types fade_out_type,
                               double fade_in_curve, double fade_out_curve,
                               t_llll *voice_pan, e_ears_pan_modes pan_mode, e_ears_pan_laws pan_law,
                               double multichannel_pan_aperture, char compensate_gain_for_multichannel_to_avoid_clipping,
                               e_ears_veltoamp_modes veltoamp_mode, double amp_vel_min, double amp_vel_max,
                               double middleAtuning, long oversampling, long resamplingfiltersize,
                               bool optimize_for_identical_samples)
{
    t_ears_err err = EARS_ERR_NONE;
    t_llll *body = llll_clone(roll_gs);
    t_llll *header = ears_sliceheader(body);
    
    t_llll *sources = llll_get();
    t_llll *gains = llll_get();
    t_llll *offset_samps = llll_get();
    
    char there_are_solos = ears_roll_there_are_solos(body);
    
    if (mode != EARS_SCORETOBUF_MODE_SYNTHESIS && mode != EARS_SCORETOBUF_MODE_SAMPLING) {
        object_error((t_object *)e_ob, "Unsupported mode!");
        return EARS_ERR_GENERIC;
    }

    
    double pan_min = 0, pan_max = 1;
    if (pan_slot)
        get_range_from_header(header, pan_slot, &pan_min, &pan_max);

    double gain_min = 0, gain_max = 1;
    char gain_is_in_decibel = false;
    if (gain_slot) {
        gain_is_in_decibel = is_slot_in_decibel(header, gain_slot);
        get_range_from_header(header, gain_slot, &gain_min, &gain_max);
    }
    
    
    ears_buffer_set_sr((t_object *)e_ob, dest, sr);
    ears_buffer_set_numchannels((t_object *)e_ob, dest, num_channels);
    
    std::vector<t_ears_note_buffer> noteinfo;
    long buffer_index = 1, voice_num = 1;
    for (t_llllelem *voice_el = body->l_head; voice_el; voice_el = voice_el->l_next, voice_num++) {
        if (hatom_gettype(&voice_el->l_hatom) != H_LLLL)
            continue;
        
        t_llll *voice_ll = hatom_getllll(&voice_el->l_hatom);
        long voice_flag = (voice_ll->l_tail && hatom_gettype(&voice_ll->l_tail->l_hatom) == H_LONG) ? hatom_getlong(&voice_ll->l_tail->l_hatom) : 0;
        
        if (!should_voice_be_bounced(there_are_solos, voice_flag))
            continue;
        
        for (t_llllelem *chord_el = voice_ll->l_head; chord_el; chord_el = chord_el->l_next) {
            if (hatom_gettype(&chord_el->l_hatom) != H_LLLL)
                continue;
            
            t_llll *chord_ll = hatom_getllll(&chord_el->l_hatom);
            double onset_ms = chord_ll->l_head ? hatom_getdouble(&chord_ll->l_head->l_hatom) : 0;
            long chord_flag = voice_flag | ((chord_ll->l_tail && hatom_gettype(&chord_ll->l_tail->l_hatom) == H_LONG) ? hatom_getlong(&chord_ll->l_tail->l_hatom) : 0);
            
            for (t_llllelem *note_el = chord_ll->l_head; note_el; note_el = note_el->l_next) {
                if (hatom_gettype(&note_el->l_hatom) != H_LLLL)
                    continue;
                
                t_llll *note_ll = hatom_getllll(&note_el->l_hatom);
                double note_pitch_cents = note_ll->l_head ? hatom_getdouble(&note_ll->l_head->l_hatom) : 6000;
                double note_duration_ms = note_ll->l_head && note_ll->l_head->l_next ? hatom_getdouble(&note_ll->l_head->l_next->l_hatom) : 0;
                double note_velocity = note_ll->l_head && note_ll->l_head->l_next && note_ll->l_head->l_next->l_next ? hatom_getdouble(&note_ll->l_head->l_next->l_next->l_hatom) : 0;
                long note_flag = chord_flag | ((note_ll->l_tail && hatom_gettype(&note_ll->l_tail->l_hatom) == H_LONG) ? hatom_getlong(&note_ll->l_tail->l_hatom) : 0);
                if (!should_be_bounced(there_are_solos, note_flag))
                    continue;
                
                // bounce note!
                t_buffer_obj *buf = NULL;
                double start = get_slot_from_note_as_double(note_ll, offset_slot, 0.);
                double end = use_durations ? start + note_duration_ms : -1;
                
                // find breakpoints
                t_llll *breakpoints = NULL;
                for (t_llllelem *el = note_ll->l_head; el; el = el->l_next) {
                    if (hatom_gettype(&el->l_hatom) == H_LLLL) {
                        t_llll *ll = hatom_getllll(&el->l_hatom);
                        if (ll && ll->l_head && hatom_gettype(&ll->l_head->l_hatom) == H_SYM &&
                            hatom_getsym(&ll->l_head->l_hatom) == _llllobj_sym_breakpoints) {
                            breakpoints = ll;
                            break;
                        }
                    }
                }
                
                t_ears_err this_err = EARS_ERR_NONE;
                char new_buffer = true;
                t_llll *gain_env = gain_slot ? get_slot_from_note_llll(note_ll, gain_slot) : NULL;
                t_llll *pan_env = pan_slot ? get_slot_from_note_llll(note_ll, pan_slot) : NULL;
                double this_voice_pan = DBL_MAX;
                if (voice_pan && voice_pan->l_size > 0) {
                    t_llllelem *pan_el = llll_getindex(voice_pan, voice_num, I_STANDARD);
                    if (pan_el && is_hatom_number(&pan_el->l_hatom)) {
                        this_voice_pan = hatom_getdouble(&pan_el->l_hatom);
                    }
                }
                
                double sr_os = sr;
                if (mode == EARS_SCORETOBUF_MODE_SYNTHESIS) {
                    this_err = ears_buffer_synth_from_duration_line((t_object *)e_ob, &buf,
                                                                    synthmode, wavetable, wavetable_length,
                                                                    note_pitch_cents, note_duration_ms, note_velocity, breakpoints,
                                                                    veltoamp_mode, amp_vel_min, amp_vel_max, middleAtuning, sr, buffer_index++, earsbufobj_get_slope_mapping(e_ob), oversampling, resamplingfiltersize);
                } else if (mode == EARS_SCORETOBUF_MODE_SAMPLING) {
                    t_symbol *filename = get_filename_from_note_llll(note_ll, filename_slot);
                    double rate = 1.;
                    
                    sr_os = sr * oversampling;
                    
                    if (rate_slot) {
                        t_llll *rate_ll = get_slot_from_note_llll(note_ll, rate_slot);
                        if (rate_ll && rate_ll->l_head && is_hatom_number(&rate_ll->l_head->l_hatom))
                            rate = hatom_getdouble(&rate_ll->l_head->l_hatom);
                        llll_free(rate_ll);
                    }
                    
                    if (end > 0 && rate != 1)
                        end = start + (end-start) * rate;


                    // this function optimizes for identical samples (if needed):
                    this_err = ears_roll_to_buffer_get_buffer(e_ob, noteinfo, &buf, &new_buffer, filename, rate, start, end, NULL, gain_env, pan_env, this_voice_pan, fadein_amount, fadeout_amount, 0, 1., sr_os, &buffer_index, optimize_for_identical_samples);
                    
                }
                
                if (new_buffer) {
                    // Now a sequence of in-place operations to modify the buffer
                    // only if the buffer is a new one, and is not exactly the same as a previous one (in sampling mode)
                    
                    if (buffer_getsamplerate(buf) != sr_os)
                        ears_buffer_convert_sr((t_object *)e_ob, buf, sr_os);

                    if (gain_env) {
                        t_llll *gain_env_remapped = earsbufobj_llll_convert_envtimeunit_and_normalize_range(e_ob, gain_env, buf, EARS_TIMEUNIT_SAMPS, gain_min, gain_max, false);
                        ears_buffer_gain_envelope((t_object *)e_ob, buf, buf, gain_env_remapped, gain_is_in_decibel, earsbufobj_get_slope_mapping(e_ob));
                        llll_free(gain_env_remapped);
                    }
                    
                    if (pan_env) {
                        t_llll *pan_env_remapped = earsbufobj_llll_convert_envtimeunit_and_normalize_range(e_ob, pan_env, buf, EARS_TIMEUNIT_SAMPS, pan_min, pan_max, false);
                        ears_buffer_pan1d_envelope((t_object *)e_ob, buf, buf, num_channels, pan_env_remapped, pan_mode, pan_law, multichannel_pan_aperture, compensate_gain_for_multichannel_to_avoid_clipping, earsbufobj_get_slope_mapping(e_ob));
                        llll_free(pan_env_remapped);
                    } else if (this_voice_pan != DBL_MAX) {
                        ears_buffer_pan1d((t_object *)e_ob, buf, buf, num_channels, this_voice_pan, pan_mode, pan_law, multichannel_pan_aperture, compensate_gain_for_multichannel_to_avoid_clipping);
                    }
                    
                    if (buffer_getchannelcount(buf) != num_channels)
                        ears_buffer_convert_numchannels((t_object *)e_ob, buf, num_channels, convertchannelsmode, convertchannelsmode);
                    
                    if (fadein_amount > 0 || fadeout_amount > 0)
                        ears_buffer_fade((t_object *)e_ob, buf, buf, earsbufobj_time_to_samps(e_ob, fadein_amount, buf), earsbufobj_time_to_samps(e_ob, fadeout_amount, buf), fade_in_type, fade_out_type, fade_in_curve, fade_out_curve, earsbufobj_get_slope_mapping(e_ob));
                }
                
                llll_free(gain_env);
                llll_free(pan_env);

                if (err == EARS_ERR_NONE && this_err != EARS_ERR_NONE)
                    err = this_err;
                
                llll_appendobj(sources, buf);

                double note_gain = 1.;
                if (mode == EARS_SCORETOBUF_MODE_SAMPLING) { // otherwise it's already encoded by ears_buffer_synth_from_duration_line()
                    switch (veltoamp_mode) {
                        case EARS_VELOCITY_TO_AMPLITUDE:
                            note_gain = rescale(note_velocity, 0., 127., amp_vel_min, amp_vel_max);
                            break;
                            
                        case EARS_VELOCITY_TO_DECIBEL:
                            note_gain = ears_db_to_linear(rescale(note_velocity, 0., 127., amp_vel_min, amp_vel_max));
                            break;
                            
                        default:
                            break;
                    }
                }
                llll_appenddouble(gains, note_gain);
                
                llll_appenddouble(offset_samps, ears_ms_to_samps(onset_ms, sr_os));
            }
        }
    }

    
    // mixing
    ears_buffer_mix_from_llll((t_object *)e_ob, sources, dest, gains, offset_samps, normalization_mode,
                              earsbufobj_get_slope_mapping(e_ob), (e_ears_resamplingpolicy)e_ob->l_resamplingpolicy, e_ob->l_resamplingfilterwidth);
    
    if (oversampling > 1) {
        ears_buffer_resample((t_object *)e_ob, dest, 1./oversampling, resamplingfiltersize);
        ears_buffer_set_sr((t_object *)e_ob, dest, sr);
    }
    
    // freeing buffers
    if (optimize_for_identical_samples) {
        for (long i = 0; i < noteinfo.size(); i++)
            object_free(noteinfo[i].buffer);
    } else {
        for (t_llllelem *el = sources->l_head; el; el = el->l_next) {
            t_buffer_obj *buf = (t_buffer_obj *)hatom_getobj(&el->l_hatom);
            object_free(buf);
        }
    }
    
    // freeing lllls
    llll_free(sources);
    llll_free(gains);
    llll_free(offset_samps);
    llll_free(body);
    llll_free(header);
    
    return err;
}



/////////// REAPER CONVERSIONS ///////////////

std::string strippath(std::string s)
{
    long start = s.find_last_of("\\/");
    if (start < 0)
        return s;
    return s.substr(start, s.length());
}

std::string getextension(std::string s)
{
    long start = s.find_last_of(".");
    if (start < 0)
        return "";
    return s.substr(start, s.length());
}


std::string getbasename(std::string s)
{
    long start = s.find_last_of(".");
    if (start < 0)
        return s;
    return s.substr(0, start);
}


const char *getReaperSourceType(std::string s)
{
    std::string ext = getextension(s);
    if (ext == ".mp3")
        return "MP3";
    if (ext == ".flac")
        return "FLAC";
    return "WAVE";
}



int ears_fade_type_to_reaper_type(e_ears_fade_types fade)
{
    switch (fade) {
        case EARS_FADE_LINEAR:
            return 0;
            break;
        case EARS_FADE_SINE:
            return 1;
            break;
        case EARS_FADE_CURVE:
            return 0;
            break;
        case EARS_FADE_SCURVE:
            return 5;
        default:
            return 0;
            break;
    }
}

void make_filepath_unique_with_long(const char *filename, long num, char *out_filename, long out_filename_size)
{
    std::string ext = getextension(filename);
    std::string base = getbasename(filename);
    snprintf_zero(out_filename, out_filename_size, "%s%ld%s", base.c_str(), num, ext.c_str());
}


t_symbol *filetype_to_extension(t_symbol *buffer_filetype)
{
    if (buffer_filetype == gensym("aiff"))
        return gensym("aif");
    if (buffer_filetype == gensym("wave"))
        return gensym("wav");
    if (buffer_filetype == gensym("raw"))
        return gensym("raw");
    if (buffer_filetype == gensym("au"))
        return gensym("au");
    if (buffer_filetype == gensym("flac"))
        return gensym("flac");
    if (buffer_filetype == gensym("mp3"))
        return gensym("mp3");
    return gensym("aif");
}

t_symbol *filetype_to_message(t_symbol *buffer_filetype)
{
    if (buffer_filetype == gensym("aiff"))
        return gensym("writeaiff");
    if (buffer_filetype == gensym("wave"))
        return gensym("writewave");
    if (buffer_filetype == gensym("raw"))
        return gensym("writeraw");
    if (buffer_filetype == gensym("au"))
        return gensym("au");
    if (buffer_filetype == gensym("flac"))
        return gensym("writeflac");
    if (buffer_filetype == gensym("mp3"))
        return gensym("writemp3");
    return gensym("writeaiff");
}


// mode: only sampling mode supported
t_ears_err ears_roll_to_reaper(t_earsbufobj *e_ob, t_symbol *filename_sym, t_symbol *reaper_header,
                               e_ears_scoretobuf_mode mode, t_llll *roll_gs, char use_durations,
                               char pitch_is_transposition, long base_midicents,
                               long filename_slot, long offset_slot, long gain_slot, long pan_slot,
                               long transp_slot, long timestretch_slot, long fade_slot, long color_slot,
                               // these are the default fades, unless any fade_slot is defined
                               double default_fadein_amount, double default_fadeout_amount,
                               e_ears_fade_types fade_in_type, e_ears_fade_types fade_out_type,
                               double fade_in_curve, double fade_out_curve,
                               e_ears_veltoamp_modes veltoamp_mode, double amp_vel_min, double amp_vel_max,
                               t_llll *number_of_channels_per_voice, char auto_xfade,
                               char copy_media, t_symbol *media_folder_name, t_symbol *buffer_format, t_symbol *buffer_filetype)
{
    t_ears_err err = EARS_ERR_NONE;
    t_llll *body = llll_clone(roll_gs);
    t_llll *header = ears_sliceheader(body);

     t_llll *copied_files = llll_get();
     t_llll *copied_buffers = llll_get();

     if (mode != EARS_SCORETOBUF_MODE_SAMPLING) {
        object_error((t_object *)e_ob, "Unsupported mode for Reaper export!");
        return EARS_ERR_GENERIC;
    }
    
    std::stringstream filecontent;
    if (!filename_sym) {
        object_error((t_object *)e_ob, "Cannot write output file!");
        return EARS_ERR_NO_FILE;
    }
    
    short sessionpath = 0, mediapath = 0;
    if (copy_media) {
        if (!media_folder_name || strlen(media_folder_name->s_name) == 0) {
            copy_media = 0;
            object_warn((t_object *)e_ob, "Folder name for media copy is unspecified or empty.");
            object_warn((t_object *)e_ob, "     Media will not be copied.");
        } else {
            char temp[MAX_PATH_CHARS];
            if (path_frompathname(filename_sym->s_name, &sessionpath, temp) != 0) {
                copy_media = 0;
                object_warn((t_object *)e_ob, "Error creating output media folder.");
                object_warn((t_object *)e_ob, "     Media will not be copied.");
            } else {
                path_createfolder(sessionpath, media_folder_name->s_name, &mediapath);
            }
        }
    }
    
    // the hard-coded number seems to be some sort of version number, I guess
    filecontent << "<REAPER_PROJECT " << reaper_header->s_name << std::endl;

    if (auto_xfade) {
        filecontent << "  AUTOXFADE 1" << std::endl;
    } else {
        filecontent << "  ITEMMIX 1" << std::endl;
    }

    if (copy_media) {
        filecontent << "  RECORD_PATH \"" << media_folder_name->s_name << "\" \"\"" << std::endl;
    }
    
    double pan_min = 0, pan_max = 1;
    char pan_is_temporal_absolute = false;
    if (pan_slot) {
        pan_is_temporal_absolute = is_slot_temporal_absolute(header, pan_slot);
        get_range_from_header(header, pan_slot, &pan_min, &pan_max);
    }
    
    double gain_min = 0, gain_max = 1;
    char gain_is_in_decibel = false, gain_is_temporal_absolute = false;
    if (gain_slot) {
        gain_is_in_decibel = is_slot_in_decibel(header, gain_slot);
        gain_is_temporal_absolute = is_slot_temporal_absolute(header, gain_slot);
        get_range_from_header(header, gain_slot, &gain_min, &gain_max);
    }
    
    
    t_llll *voicenames = get_voicenames_field_from_header(header);
    if (voicenames && voicenames->l_head)
        llll_behead(voicenames);
    
    t_llllelem *voicename_el = voicenames ? voicenames->l_head : NULL;
    t_llllelem *voicechans_el = number_of_channels_per_voice ? number_of_channels_per_voice->l_head : NULL;
    
    for (t_llllelem *voice_el = body->l_head; voice_el; voice_el = voice_el->l_next,
         voicename_el = voicename_el ? voicename_el->l_next : NULL,
         voicechans_el = voicechans_el && voicechans_el->l_next ? voicechans_el->l_next : NULL) {
        if (hatom_gettype(&voice_el->l_hatom) != H_LLLL)
            continue;
        
        t_llll *voice_ll = hatom_getllll(&voice_el->l_hatom);
        long voice_flag = (voice_ll->l_tail && hatom_gettype(&voice_ll->l_tail->l_hatom) == H_LONG) ? hatom_getlong(&voice_ll->l_tail->l_hatom) : 0;
        
        // write voice
        filecontent << "  <TRACK" << std::endl;
        if (voicename_el && !(hatom_gettype(&voicename_el->l_hatom) == H_LLLL && hatom_getllll(&voicename_el->l_hatom)->l_size == 0)) {
            char *voicename_buf = NULL;
            hatom_to_text_buf(&voicename_el->l_hatom, &voicename_buf);
            if (voicename_buf && hatom_gettype(&voicename_el->l_hatom) == H_LLLL && strlen(voicename_buf) > 1) {
                // manually remove starting [ and ending ] from llll deparsing
                voicename_buf[strlen(voicename_buf)-2] = 0;
                filecontent << "    NAME \"" << voicename_buf+2 << "\"" << std::endl;
            } else {
                filecontent << "    NAME \"" << voicename_buf << "\"" << std::endl;
            }
            bach_freeptr(voicename_buf);
        } else  {
            filecontent << "    NAME \"\"" << std::endl;
        }
        
        // number of channels
        if (voicechans_el) {
            long nchan = hatom_getlong(&voicechans_el->l_hatom);
            filecontent << "    NCHAN " << (nchan >= 1 ? nchan : 2) << std::endl;
        }
        
        // flags
        if ((voice_flag & k_FLAG_ELEMENT_MUTED) || (voice_flag & k_FLAG_ELEMENT_SOLO))
            filecontent << "      MUTESOLO " << (voice_flag & k_FLAG_ELEMENT_MUTED ? 1 : 0) <<
                                             (voice_flag & k_FLAG_ELEMENT_SOLO ? 2 : 0) << " 0 "<< std::endl;

        
        for (t_llllelem *chord_el = voice_ll->l_head; chord_el; chord_el = chord_el->l_next) {
            if (hatom_gettype(&chord_el->l_hatom) != H_LLLL)
                continue;
            
            t_llll *chord_ll = hatom_getllll(&chord_el->l_hatom);
            double onset_ms = chord_ll->l_head ? hatom_getdouble(&chord_ll->l_head->l_hatom) : 0;
            long chord_flag = voice_flag | ((chord_ll->l_tail && hatom_gettype(&chord_ll->l_tail->l_hatom) == H_LONG) ? hatom_getlong(&chord_ll->l_tail->l_hatom) : 0);
            
            for (t_llllelem *note_el = chord_ll->l_head; note_el; note_el = note_el->l_next) {
                if (hatom_gettype(&note_el->l_hatom) != H_LLLL)
                    continue;
                
                t_llll *note_ll = hatom_getllll(&note_el->l_hatom);
                double note_pitch_cents = note_ll->l_head ? hatom_getdouble(&note_ll->l_head->l_hatom) : 6000;
                double note_duration_ms = note_ll->l_head && note_ll->l_head->l_next ? hatom_getdouble(&note_ll->l_head->l_next->l_hatom) : 0;
                double note_velocity = note_ll->l_head && note_ll->l_head->l_next && note_ll->l_head->l_next->l_next ? hatom_getdouble(&note_ll->l_head->l_next->l_next->l_hatom) : 0;
                long note_flag = chord_flag | ((note_ll->l_tail && hatom_gettype(&note_ll->l_tail->l_hatom) == H_LONG) ? hatom_getlong(&note_ll->l_tail->l_hatom) : 0);

                t_symbol *filename = get_filename_from_note_llll(note_ll, filename_slot);
                t_symbol *filepath = ears_ezlocate_file(filename, NULL);
                char is_buffer = false;
                if (!filepath && ears_buffer_symbol_is_buffer(filename)) {
                    is_buffer = true; // ok, it's a buffer... Which may come in handy if we have copymedia set to 1
                    if (!copy_media) {
                        char warnmsg[1024];
                        snprintf_zero(warnmsg, 1024, "The string '%s' seems to refer to a buffer, but the 'copymedia' attribute is not set.");
                        object_warn((t_object *)e_ob, warnmsg);
                        object_warn((t_object *)e_ob, "    The reference to the audio data in the Reaper session may be missing.");
                    }
                }
                
                double start = get_slot_from_note_as_double(note_ll, offset_slot, 0.);
                double fade_in_amount = get_slot_from_note_as_double(note_ll, fade_slot, default_fadein_amount);
                double fade_out_amount = get_slot_from_note_as_double(note_ll, fade_slot, default_fadeout_amount, 2);
                double fade_in_ms = fade_in_amount, fade_out_ms = fade_out_amount;
                t_buffer_obj *buf = NULL;;
                
                if (e_ob->l_timeunit != EARS_TIMEUNIT_MS || !use_durations) {
                    // This part is definitely overkill: if the timeunit is not MS we LOAD the content into a buffer just to get
                    // the proper samplerate/duration; there should be no need for this, one may probably get those info from the file itself
                    if (is_buffer) {
                        buf = ears_buffer_getobject(filename);
                        if (!buf)
                            object_warn((t_object *)e_ob, "Cannot retrieve buffer.");
                    } else {
                        double end = use_durations ? start + note_duration_ms : -1;
                        ears_buffer_from_file((t_object *)e_ob, &buf, filename, start, end, 0);
                    }
                    if (buf) {
                        fade_in_ms = earsbufobj_time_to_ms(e_ob, fade_in_amount, buf);
                        fade_out_ms = earsbufobj_time_to_ms(e_ob, fade_out_amount, buf);
                        if (!use_durations)
                            note_duration_ms = ears_buffer_get_size_ms((t_object *)e_ob, buf);
                    }
                }
                
                double note_gain = 1.;
                switch (veltoamp_mode) {
                    case EARS_VELOCITY_TO_AMPLITUDE:
                        note_gain = rescale(note_velocity, 0., 127., amp_vel_min, amp_vel_max);
                        break;
                    case EARS_VELOCITY_TO_DECIBEL:
                        note_gain = ears_db_to_linear(rescale(note_velocity, 0., 127., amp_vel_min, amp_vel_max));
                        break;
                    default:
                        break;
                }
                
                double note_pan = get_slot_from_note_as_double(note_ll, pan_slot, 0.);
                
                // find pitch breakpoints, if any
                t_llll *breakpoints = NULL;
                for (t_llllelem *el = note_ll->l_head; el; el = el->l_next) {
                    if (hatom_gettype(&el->l_hatom) == H_LLLL) {
                        t_llll *ll = hatom_getllll(&el->l_hatom);
                        if (ll && ll->l_head && hatom_gettype(&ll->l_head->l_hatom) == H_SYM &&
                            hatom_getsym(&ll->l_head->l_hatom) == _llllobj_sym_breakpoints) {
                            breakpoints = ll;
                            break;
                        }
                    }
                }
                
                // write note!
                filecontent << "    <ITEM" << std::endl;
                
                filecontent << "      POSITION " << onset_ms/1000. << std::endl;
                filecontent << "      LENGTH " << note_duration_ms/1000. << std::endl;
                filecontent << "      VOLPAN " << note_gain << " " << note_pan << " 1 -1" << std::endl;
                filecontent << "      SOFFS " << start/1000. << std::endl;
                if (filename) {
                    // strip path
                    filecontent << "      NAME \"" << strippath(filename->s_name) << "\"" << std::endl;
                    
                    if (filepath) {
                        if (copy_media) {
                            if (is_buffer) {
                                if (!is_symbol_in_llll_first_level(copied_buffers, filename)) {  // has it already been copied?
                                    t_buffer_obj *buf = ears_buffer_getobject(filename);
                                    llll_appendsym(copied_buffers, filename);
                                    
                                    char outfilename[MAX_FILENAME_CHARS];
                                    char outfilepath[MAX_PATH_CHARS];
                                    t_symbol *ext = filetype_to_extension(buffer_filetype);
                                    t_symbol *msg = filetype_to_message(buffer_filetype);
                                    snprintf_zero(outfilename, MAX_FILENAME_CHARS, "%s.%s", filename->s_name, ext ? ext->s_name : "aif");
                                    path_topathname(mediapath, outfilename, outfilepath);
                                    t_symbol *exportpath = gensym(outfilepath);
                                    earsbufobj_write_buffer(e_ob, buf, msg, exportpath, buffer_format);
                                }

                            } else {
                                if (!is_symbol_in_llll_first_level(copied_files, filepath)) {  // has it already been copied?
                                    llll_appendsym(copied_files, filepath);
                                    
                                    short srcpath = 0;
                                    char srcname[MAX_PATH_CHARS], destname[MAX_PATH_CHARS];
                                    if (path_frompathname(filepath->s_name, &srcpath, srcname)) {
                                        char warnmsg[2*MAX_PATH_CHARS];
                                        snprintf_zero(warnmsg, 2*MAX_PATH_CHARS, "Cannot locate file \"%s\"", filepath->s_name);
                                    } else {
                                        long count = 1;
                                        snprintf_zero(destname, MAX_PATH_CHARS, "%s", srcname);
                                        while (ears_file_exists(destname, mediapath))
                                            make_filepath_unique_with_long(srcname, count++, destname, MAX_PATH_CHARS);
                                        path_copyfile(srcpath, srcname, mediapath, destname);
                                        char newpath[MAX_PATH_CHARS];
                                        newpath[0] = 0;
                                        path_topathname(mediapath, destname, newpath);
                                        filepath = gensym(newpath);
                                    }
                                }
                            }
                        }
                        filecontent << "      <SOURCE " << getReaperSourceType(filename->s_name) << std::endl;
                        filecontent << "        FILE \"" << filepath->s_name << "\"" << std::endl;
                        filecontent << "      >" << std::endl;
                    }
                }

                filecontent << "      FADEIN " << ears_fade_type_to_reaper_type(fade_in_type) << " " << fade_in_ms/1000. << " " << fade_in_curve << " "  << ears_fade_type_to_reaper_type(fade_in_type) << " 0 " << fade_in_curve << std::endl;
                filecontent << "      FADEOUT "  << ears_fade_type_to_reaper_type(fade_out_type) << " " << fade_out_ms/1000. << " " << fade_out_curve << " " << ears_fade_type_to_reaper_type(fade_out_type) << " 0 "  << fade_out_curve << std::endl;
                
                double playrate = 1./get_slot_from_note_as_double(note_ll, timestretch_slot, 1.);
                double transp = get_slot_from_note_as_double(note_ll, transp_slot, 0.)/100.;
                if (pitch_is_transposition)
                    transp += (note_pitch_cents - base_midicents)/100.;
                
                filecontent << "      PLAYRATE " << playrate << " 1 " << transp << " -1 0 0.0025" << std::endl;

                // flags
                if (note_flag & k_FLAG_ELEMENT_MUTED)
                    filecontent << "      MUTE 1 0" << std::endl;
                if (note_flag & k_FLAG_ELEMENT_LOCKED)
                    filecontent << "      LOCK 1" << std::endl;

                if (color_slot) {
                    // custom color
                    char this_note_has_slot = false;
                    int r = round(255. * get_slot_from_note_as_double(note_ll, color_slot, 0, 1, &this_note_has_slot));
                    if (this_note_has_slot) {
                        int g = round(255. * get_slot_from_note_as_double(note_ll, color_slot, 0, 2));
                        int b = round(255. * get_slot_from_note_as_double(note_ll, color_slot, 0, 3));
                        filecontent << "      COLOR " << (256*256*256) + r*256*256 + g*256+b << " B" << std::endl;
                    }
                }
                
                // set pitch breakpoint envelope from duration line
                if (breakpoints) {
                    filecontent << "      <PITCHENV" << std::endl;
                    for (t_llllelem *bpel = breakpoints->l_head; bpel; bpel = bpel->l_next) {
                        if (hatom_gettype(&bpel->l_hatom) == H_LLLL) {
                            t_llll *ll = hatom_getllll(&bpel->l_hatom);
                            if (ll && ll->l_size >= 2) {
                                double xval = hatom_getdouble(&ll->l_head->l_hatom) * note_duration_ms/1000.;
                                double yval = hatom_getdouble(&ll->l_head->l_next->l_hatom) / 100.; // cents to semitones
                                double nextslope = 0;
                                if (bpel->l_next && hatom_gettype(&bpel->l_next->l_hatom) == H_LLLL && hatom_getllll(&bpel->l_next->l_hatom)->l_size >= 3)
                                    nextslope = hatom_getdouble(&hatom_getllll(&bpel->l_next->l_hatom)->l_head->l_next->l_next->l_hatom);
                                
                                filecontent << "        PT " << xval << " " << yval;
                                if (!bpel->l_next) {
                                    filecontent << " 0 0 1" << std::endl;
                                } else if (nextslope != 0) {
                                    filecontent << " 5 1 0 0 " << nextslope << std::endl;
                                } else {
                                    filecontent << " 0" << std::endl;
                                }
                            }
                        }
                    }
                    filecontent << "      >" << std::endl;
                }
                
                
                if (gain_slot) {
                    t_llll *gain_env = get_slot_from_note_llll(note_ll, gain_slot);
                    if (gain_env) {
                        t_llll *gain_env_remapped = earsbufobj_llll_convert_envtimeunit_and_normalize_range(e_ob, gain_env, buf, EARS_TIMEUNIT_MS, gain_min, gain_max, false);

                        filecontent << "      <VOLENV" << std::endl;
                        filecontent << "        VOLTYPE " << (gain_is_in_decibel ? 1 : 0) << std::endl;
                        for (t_llllelem *bpel = gain_env_remapped->l_head; bpel; bpel = bpel->l_next) {
                            if (hatom_gettype(&bpel->l_hatom) == H_LLLL) {
                                t_llll *ll = hatom_getllll(&bpel->l_hatom);
                                if (ll && ll->l_size >= 2) {
                                    double sec = hatom_getdouble(&ll->l_head->l_hatom) * (gain_is_temporal_absolute ? 1. : note_duration_ms)/1000.;
                                    double vol = hatom_getdouble(&ll->l_head->l_next->l_hatom);
                                    double nextslope = 0;
                                    if (bpel->l_next && hatom_gettype(&bpel->l_next->l_hatom) == H_LLLL && hatom_getllll(&bpel->l_next->l_hatom)->l_size >= 3)
                                        nextslope = hatom_getdouble(&hatom_getllll(&bpel->l_next->l_hatom)->l_head->l_next->l_next->l_hatom);
                                    
                                    filecontent << "        PT " << sec << " " << (gain_is_in_decibel ? ears_db_to_linear(vol) : vol);
                                    if (nextslope != 0) {
                                        filecontent << " 5 1 0 0 " << nextslope << std::endl;
                                    } else {
                                        filecontent << " 0" << std::endl;
                                    }
                                }
                            }
                        }
                        filecontent << "      >" << std::endl;
                        
                        llll_free(gain_env_remapped);
                        llll_free(gain_env);
                    }
                }
                
                if (pan_slot) {
                    t_llll *pan_env = get_slot_from_note_llll(note_ll, pan_slot);
                    if (pan_env) {
                        t_llll *pan_env_remapped = earsbufobj_llll_convert_envtimeunit_and_normalize_range(e_ob, pan_env, buf, EARS_TIMEUNIT_MS, pan_min, pan_max, false);
                        
                        filecontent << "      <PANENV" << std::endl;
                        filecontent << "        VOLTYPE " << (gain_is_in_decibel ? 1 : 0) << std::endl;
                        for (t_llllelem *bpel = pan_env_remapped->l_head; bpel; bpel = bpel->l_next) {
                            if (hatom_gettype(&bpel->l_hatom) == H_LLLL) {
                                t_llll *ll = hatom_getllll(&bpel->l_hatom);
                                if (ll && ll->l_size >= 2) {
                                    double sec = hatom_getdouble(&ll->l_head->l_hatom) * (pan_is_temporal_absolute ? 1. : note_duration_ms)/1000.;
                                    double pan = hatom_getdouble(&ll->l_head->l_next->l_hatom);
                                    double nextslope = 0;
                                    if (bpel->l_next && hatom_gettype(&bpel->l_next->l_hatom) == H_LLLL && hatom_getllll(&bpel->l_next->l_hatom)->l_size >= 3)
                                        nextslope = hatom_getdouble(&hatom_getllll(&bpel->l_next->l_hatom)->l_head->l_next->l_next->l_hatom);
                                    
                                    filecontent << "        PT " << sec << " " << (pan*2.-1.);
                                    if (nextslope != 0) {
                                        filecontent << " 5 1 0 0 " << nextslope << std::endl;
                                    } else {
                                        filecontent << " 0" << std::endl;
                                    }
                                }
                            }
                        }
                        filecontent << "      >" << std::endl;
                        
                        llll_free(pan_env_remapped);
                        llll_free(pan_env);
                    }
                }
                
                filecontent << "    >" << std::endl;
                
                if (!is_buffer)
                    object_free(buf);
            }
        }
        filecontent << "  >" << std::endl;
    }
    
    filecontent << ">" << std::endl;

    
    // freeing lllls
    llll_free(body);
    llll_free(header);
    llll_free(voicenames);
    llll_free(copied_files);
    llll_free(copied_buffers);

    t_ptr_size orig_count = strlen(filecontent.str().c_str());
    t_ptr_size count = orig_count;
    t_max_err writing_err = llll_write_text_file(filename_sym, &count, filecontent.str().c_str());
 
    if (writing_err != MAX_ERR_NONE) {
        object_error((t_object *)e_ob, "Error while writing file!");
    } else if (count != orig_count) {
        object_warn((t_object *)e_ob, "Less characters than it should have been written to Reaper project file!");
    }

    return err;
}
