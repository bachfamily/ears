// Reverb model implementation
//
// Written by Jezar at Dreampoint, June 2000
// http://www.dreampoint.co.uk
// This code is public domain

#include "revmodel.hpp"

void revmodel::initstuff(bool from_scratch)
{
    // allocating buffers
    combf = (comb ***)malloc(numcombs * sizeof(comb **));
    allpassf = (allpass ***)malloc(numallpasses * sizeof(allpass **));
    bufcomb = (float ***)malloc(numcombs * sizeof(float **));
    bufallpass = (float ***)malloc(numallpasses * sizeof(float **));
    
    int stereospread_sr = (int) (stereospread * (sr/44100.));

    for (long i = 0; i < numcombs; i++) {
        int base = combtuningL1;
        switch (i) {
            case 0: base = combtuningL1; break;
            case 1: base = combtuningL2; break;
            case 2: base = combtuningL3; break;
            case 3: base = combtuningL4; break;
            case 4: base = combtuningL5; break;
            case 5: base = combtuningL6; break;
            case 6: base = combtuningL7; break;
            case 7: base = combtuningL8; break;
            default: break;
        }

        // fix base according to sample rate
        int base_sr = (int)(base * (sr/44100.));
        
        bufcomb[i] = (float **)malloc(numchannels * sizeof(float *));
        for (long c = 0; c < numchannels; c++)
            bufcomb[i][c] = (float *)malloc((base_sr + stereospread_sr * c) * sizeof(float));
        combf[i] = (comb **)malloc(numchannels * sizeof(comb *));
        for (long c = 0; c < numchannels; c++) {
            combf[i][c] = new comb();
            combf[i][c]->setbuffer(bufcomb[i][c], (base_sr + stereospread_sr * c));
        }
    }
    
    for (long i = 0; i < numallpasses; i++) {
        int base = allpasstuningL1;
        switch (i) {
            case 0: base = allpasstuningL1; break;
            case 1: base = allpasstuningL2; break;
            case 2: base = allpasstuningL3; break;
            case 3: base = allpasstuningL4; break;
            default: break;
        }

        // fix base according to sample rate
        int base_sr = (int)(base * (sr/44100.));

        bufallpass[i] = (float **)malloc(numchannels * sizeof(float *));
        for (long c = 0; c < numchannels; c++)
            bufallpass[i][c] = (float *)malloc((base_sr + stereospread_sr * c) * sizeof(float));
        allpassf[i] = (allpass **)malloc(numchannels * sizeof(allpass *));
        for (long c = 0; c < numchannels; c++) {
            allpassf[i][c] = new allpass();
            allpassf[i][c]->setbuffer(bufallpass[i][c], (base_sr + stereospread_sr * c));
            allpassf[i][c]->setfeedback(0.5f);
        }
    }
    
    // Set default values
    if (from_scratch) {
        setwet(initialwet);
        setroomsize(initialroom);
        setdry(initialdry);
        setdamp(initialdamp);
        setwidth(initialwidth);
        setmode(initialmode);
    } else {
        update();
    }
    
    // Buffer will be full of rubbish - so we MUST mute them
    mute();
}

void revmodel::freestuff()
{
    for (int i = 0; i < numcombs; i++) {
        for (int c = 0; c < numchannels; c++) {
            delete combf[i][c]; // CRASH QUI
            free(bufcomb[i][c]);
        }
        free(combf[i]);
        free(bufcomb[i]);
    }
    free(combf);
    free(bufcomb);
    for (int i = 0; i < numallpasses; i++) {
        for (int c = 0; c < numchannels; c++) {
            delete allpassf[i][c];
            free(bufallpass[i][c]);
        }
        free(allpassf[i]);
        free(bufallpass[i]);
    }
    free(allpassf);
    free(bufallpass);
}

revmodel::revmodel()
{
    sr = 44100;
    numchannels = 2; // default
    initstuff(true);
}

revmodel::~revmodel()
{
    freestuff();
}


void revmodel::setnumchannels(long num_channels)
{
    if (num_channels != numchannels) {
        numchannels = num_channels;
        freestuff();
        initstuff(false);
    }
}

void revmodel::setsr(long new_sr)
{
    if (new_sr != sr) {
        sr = new_sr;
        freestuff();
        initstuff(false);
    }
}



void revmodel::mute()
{
	if (getmode() >= freezemode)
		return;

	for (int i=0;i<numcombs;i++){
        for (int c = 0; c < numchannels; c++)
            combf[i][c]->mute();
	}
	for (int i=0;i<numallpasses;i++){
        for (int c = 0; c < numchannels; c++)
            allpassf[i][c]->mute();
	}
    
}

void revmodel::processreplace(float *input, float *output, long numsamples)
{
    float *out = (float *)malloc(numchannels * sizeof(float));
    float inputmix;

    wet1 = wet*(width/2 + 0.5f);
    wet2 = wet*((1-width)/2);

    
    double w1 = wet * ((1 - width) * 1 + width * 1./numchannels);
    double w2 = numchannels <= 1 ? 0. : (wet - w1)/(numchannels - 1);
    
	while (numsamples-- > 0)
    {
        for (int c = 0; c < numchannels; c++)
            out[c] = 0;
       
        inputmix = 0;
        for (int c = 0; c < numchannels; c++)
            inputmix += *(input + c);
        inputmix *= (2./numchannels) * gain; // normalize as if stereo was the default

		// Accumulate comb filters in parallel
		for (int i=0; i<numcombs; i++){
            for (int c=0; c < numchannels; c++)
                out[c] += combf[i][c]->process(inputmix);
		}

		// Feed through allpasses in series
        for (int i=0; i<numallpasses; i++) {
            for (int c=0; c < numchannels; c++)
                out[c] = allpassf[i][c]->process(out[c]);
		}

		// Calculate output REPLACING anything already there
        for (int c=0; c < numchannels; c++) {
            *(output + c) = *(input + c) * dry;
            for (int d=0; d < numchannels; d++) {
                *(output + c) += out[c] * (c==d ? w1 : w2);
            }
        }

		// Increment sample pointers, allowing for interleave (if any)
		input += numchannels;
		output += numchannels;
	}
    
    free(out);
}


void revmodel::update()
{
// Recalculate internal values after parameter change

	int i;

	wet1 = wet*(width/2 + 0.5f);
	wet2 = wet*((1-width)/2);

	if (mode >= freezemode)
	{
		roomsize1 = 1;
		damp1 = 0;
		gain = muted;
	}
	else
	{
		roomsize1 = roomsize;
		damp1 = damp;
		gain = fixedgain;
	}

	for(i=0; i<numcombs; i++){
        for (int c = 0; c < numchannels; c++)
            combf[i][c]->setfeedback(roomsize);
	}

	for(i=0; i<numcombs; i++){
        for (int c = 0; c < numchannels; c++)
            combf[i][c]->setdamp(damp1);
	}
}

// The following get/set functions are not inlined, because
// speed is never an issue when calling them, and also
// because as you develop the reverb model, you may
// wish to take dynamic action when they are called.

void revmodel::setroomsize(float value)
{
	roomsize = (value*scaleroom) + offsetroom;
	update();
}

float revmodel::getroomsize()
{
	return (roomsize-offsetroom)/scaleroom;
}

void revmodel::setdamp(float value)
{
	damp = value*scaledamp;
	update();
}

float revmodel::getdamp()
{
	return damp/scaledamp;
}

void revmodel::setwet(float value)
{
	wet = value*scalewet;
	update();
}

float revmodel::getwet()
{
	return wet/scalewet;
}

void revmodel::setdry(float value)
{
	dry = value*scaledry;
}

float revmodel::getdry()
{
	return dry/scaledry;
}

void revmodel::setwidth(float value)
{
	width = value;
	update();
}

float revmodel::getwidth()
{
	return width;
}

void revmodel::setmode(float value)
{
	mode = value;
	update();
}

float revmodel::getmode()
{
	if (mode >= freezemode)
		return 1;
	else
		return 0;
}

//ends
