#include "ears.scores.h"

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

double get_slot_from_note_as_double(t_llll *ll, long slotnum)
{
    double res = 0.;
    for (t_llllelem *el = ll->l_head; el; el = el->l_next) {
        if (hatom_gettype(&el->l_hatom) == H_LLLL) {
            t_llll *subll = hatom_getllll(&el->l_hatom);
            if (subll && subll->l_head && hatom_gettype(&subll->l_head->l_hatom) == H_SYM && hatom_getsym(&subll->l_head->l_hatom) == _llllobj_sym_slots) {
                
                for (t_llllelem *slotel = subll->l_head; slotel; slotel = slotel->l_next) {
                    if (hatom_gettype(&slotel->l_hatom) == H_LLLL) {
                        t_llll *slotll = hatom_getllll(&slotel->l_hatom);
                        if (slotll && slotll->l_head && hatom_getlong(&slotll->l_head->l_hatom) == slotnum) {
                            if (slotll->l_head->l_next) {
                                res = hatom_getdouble(&slotll->l_head->l_next->l_hatom);
                                return res;
                            }
                        }
                    }
                }
                
            }
        }
    }
    return res;
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
        if (strcasecmp(s->s_name, "db") || strcasecmp(s->s_name, "dbs") || strcasecmp(s->s_name, "decibel") || strcasecmp(s->s_name, "decibels"))
            ans = 1;
    }
    llll_free(repr_ll);
    return ans;
}


t_ears_err ears_roll_to_buffer(t_earsbufobj *e_ob, t_llll *roll_gs, t_buffer_obj *dest,
                               char use_mute_solos, char use_durations,
                               long num_channels,
                               long filename_slot, long offset_slot, long gain_slot, long pan_slot,
                               double sr, e_ears_normalization_modes normalization_mode, e_ears_channel_convert_modes convertchannelsmode,
                               double fadein_amount, double fadeout_amount, e_ears_fade_types fade_in_type, e_ears_fade_types fade_out_type,
                               double fade_in_curve, double fade_out_curve,
                               e_ears_pan_modes pan_mode, e_ears_pan_laws pan_law,
                               double multichannel_pan_aperture, char compensate_gain_for_multichannel_to_avoid_clipping,
                               e_ears_veltoamp_modes veltoamp_mode, double amp_vel_min, double amp_vel_max)
{
    t_ears_err err = EARS_ERR_NONE;
    t_llll *body = llll_clone(roll_gs);
    t_llll *header = ears_sliceheader(body);
    
    t_llll *sources = llll_get();
    t_llll *gains = llll_get();
    t_llll *offset_samps = llll_get();
    
    char there_are_solos = ears_roll_there_are_solos(body);
    
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
    
    long buffer_index = 1;
    for (t_llllelem *voice_el = body->l_head; voice_el; voice_el = voice_el->l_next) {
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
            
            if (!should_be_bounced(there_are_solos, chord_flag))
                continue;

            for (t_llllelem *note_el = chord_ll->l_head; note_el; note_el = note_el->l_next) {
                if (hatom_gettype(&note_el->l_hatom) != H_LLLL)
                    continue;
                
                t_llll *note_ll = hatom_getllll(&note_el->l_hatom);
                double note_duration_ms = note_ll->l_head && note_ll->l_head->l_next ? hatom_getdouble(&note_ll->l_head->l_next->l_hatom) : 0;
                double note_velocity = note_ll->l_head && note_ll->l_head->l_next && note_ll->l_head->l_next->l_next ? hatom_getdouble(&note_ll->l_head->l_next->l_next->l_hatom) : 0;
                long note_flag = chord_flag | ((note_ll->l_tail && hatom_gettype(&note_ll->l_tail->l_hatom) == H_LONG) ? hatom_getlong(&note_ll->l_tail->l_hatom) : 0);
                if (!should_be_bounced(there_are_solos, note_flag))
                    continue;
                
                // bounce note!
                t_buffer_obj *buf = NULL;
                double start = offset_slot ? get_slot_from_note_as_double(note_ll, offset_slot) : 0;
                double end = use_durations ? start + note_duration_ms : -1;
                t_symbol *filename = get_filename_from_note_llll(note_ll, filename_slot);
                
                t_ears_err this_err = ears_buffer_from_file((t_object *)e_ob, &buf, filename, start, end, sr, buffer_index++);

                
                // Now a sequence of in-place operations to modify the buffer
                
                if (buffer_getsamplerate(buf) != sr)
                    ears_buffer_convert_sr((t_object *)e_ob, buf, sr);
                
                if (gain_slot) {
                    t_llll *gain_env = get_slot_from_note_llll(note_ll, gain_slot);
                    if (gain_env) {
                        t_llll *gain_env_remapped = earsbufobj_llll_remap_y_to_0_1_and_x_to_samples(e_ob, gain_env, buf, gain_min, gain_max, false);
                        ears_buffer_gain_envelope((t_object *)e_ob, buf, buf, gain_env_remapped, gain_is_in_decibel);
                        llll_free(gain_env_remapped);
                        llll_free(gain_env);
                    }
                }
                
                if (pan_slot) {
                    t_llll *pan_env = get_slot_from_note_llll(note_ll, pan_slot);
                    if (pan_env) {
                        t_llll *pan_env_remapped = earsbufobj_llll_remap_y_to_0_1_and_x_to_samples(e_ob, pan_env, buf, pan_min, pan_max, false);
                        ears_buffer_pan1d_envelope((t_object *)e_ob, buf, buf, num_channels, pan_env_remapped, pan_mode, pan_law, multichannel_pan_aperture, compensate_gain_for_multichannel_to_avoid_clipping);
                        llll_free(pan_env_remapped);
                        llll_free(pan_env);
                    }
                }
                
                if (buffer_getchannelcount(buf) != num_channels)
                    ears_buffer_convert_numchannels((t_object *)e_ob, buf, num_channels, convertchannelsmode);
                
                if (fadein_amount > 0 || fadeout_amount > 0)
                    ears_buffer_fade((t_object *)e_ob, buf, buf, earsbufobj_input_to_samps(e_ob, fadein_amount, buf), earsbufobj_input_to_samps(e_ob, fadeout_amount, buf), fade_in_type, fade_out_type, fade_in_curve, fade_out_curve);
                
                if (err == EARS_ERR_NONE && this_err != EARS_ERR_NONE)
                    err = this_err;
                
                llll_appendobj(sources, buf);

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
                llll_appenddouble(gains, note_gain);
                
                llll_appenddouble(offset_samps, ears_ms_to_samps(onset_ms, sr));
            }
        }
    }
    
    /*
    float *sample1 = buffer_locksamples((t_buffer_obj *)hatom_getobj(&sources->l_head->l_hatom));
    cpost("sample[1000]: %.2f", sample1[1000]);
    buffer_unlocksamples((t_buffer_obj *)hatom_getobj(&sources->l_head->l_hatom));
    
    float *sample2 = buffer_locksamples((t_buffer_obj *)hatom_getobj(&sources->l_head->l_next->l_hatom));
    cpost("sample[1000]: %.2f", sample2[1000]);
    buffer_unlocksamples((t_buffer_obj *)hatom_getobj(&sources->l_head->l_next->l_hatom));
    */
    
    ears_buffer_mix_from_llll((t_object *)e_ob, sources, dest, gains, offset_samps, normalization_mode);
    
    for (t_llllelem *el = sources->l_head; el; el = el->l_next) {
        t_buffer_obj *buf = (t_buffer_obj *)hatom_getobj(&el->l_hatom);
        object_free(buf);
    }
    
    llll_free(sources);
    llll_free(gains);
    llll_free(offset_samps);
    llll_free(body);
    llll_free(header);
    
    return err;
}


