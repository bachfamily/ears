//
//  ears.map.h
//  ears
//
//  Created by andreaagostini on 05/04/2021.
//

#ifndef ears_map_h
#define ears_map_h

#include <ears.commons.h>
#include <ext_buffer.h>
#include <vector>
#include <unordered_map>
#include <map>

const int EARSMAP_MAX_INPUT_BUFFERS = 16;
const int EARSMAP_MAX_OUTPUT_BUFFERS = EARSMAP_MAX_INPUT_BUFFERS;
const int EARSMAP_MAX_DATA_INLETS = 16;
const int EARSMAP_MAX_DATA_OUTLETS = EARSMAP_MAX_DATA_INLETS;

const char EARSMAP_SPECIALSYM[] = "_x_x_ears.map~_x_x_";
const int EARSMAP_MAX_VS = 4096;


class bufferData {
public:
    t_buffer_obj *obj;
    t_buffer_ref *ref;
    float* samps;
    t_atom_long chans;
    t_atom_long frames;
    double sr;
    
    bufferData() : obj(nullptr) { }
    
    bufferData(t_object* x, t_symbol *name) {
        set(x, name);
    }
    
    void set(t_object* x, t_symbol *name) {
        ref = buffer_ref_new((t_object *) x, name);
        obj = buffer_ref_getobject(ref);
        samps = buffer_locksamples(obj);
        chans = ears_buffer_get_numchannels(x, obj);
        frames = ears_buffer_get_size_samps(x, obj);
        sr = ears_buffer_get_sr(x, obj);
    }
    
    virtual ~bufferData() {
        if (obj)
            buffer_unlocksamples(obj);
    }
};



typedef std::vector<t_sample> sampleVector;

class audioChannel {
private:
    static const t_atom_long maxAllocSize = 1 << 20;
    void increaseAllocSize() {
        if (lastAllocSize < maxAllocSize)
            lastAllocSize *= 2;
    }
    
    sampleVector* getNewSampleVector() {
        increaseAllocSize();
        sampleVector* sv = new sampleVector(lastAllocSize, 0);
        sVec.push_back(sv);
        totSize += lastAllocSize;
        currPosition = 0;
        return sv;
    }
    
public:
    static const std::vector<sampleVector*>::size_type svBlockSize = 16;
    t_atom_long frames;
    t_atom_long chans;
    std::vector<sampleVector*> sVec;
    t_atom_long lastAllocSize;
    t_atom_long totSize;
    t_atom_long position;
    t_atom_long currPosition;
    
    audioChannel() : frames(0), chans(0) { };
    
    audioChannel(t_atom_long fr, t_atom_long ch): frames(fr), chans(ch), lastAllocSize(EARSMAP_MAX_VS / 2), totSize(EARSMAP_MAX_VS / 2), position(0) {
        getNewSampleVector();
    }
    
    virtual ~audioChannel() {
        for (auto i : sVec)
            delete i;
    }
    
    void skipTo(t_atom_long pos) {
        while (totSize <= pos) {
            getNewSampleVector();
        }
        position = pos;
    }
    
    void insert(t_atom_long n, t_sample* samps) {
        sampleVector *currVec = sVec.back();
        t_atom_long currSize = currVec->size();
        
        t_atom_long s = 0;
        
        do {
            for ( ; s < n && currPosition < currSize; s++, currPosition++) {
                double foo = (*currVec)[currPosition];
                (*currVec)[currPosition] += samps[s];
                foo = (*currVec)[currPosition];
            }
            
            if (currPosition == currSize) {
                currVec = getNewSampleVector();
                currSize = lastAllocSize;
            }
        } while (s < n);
        
        position += s;
    }
    
    
    void copyToBuffer(bufferData* buf, t_atom_long maxSamps) {
        t_atom_long s = 0;
        float* tab = buf->samps;
        t_atom_long bufChans = buf->chans;
        tab += chans - 1;
        
        t_atom_long end = maxSamps < position ? maxSamps : position;
        if (end > buf->frames)
            end = buf->frames;
        for (sampleVector* thisVec : sVec) {
            for (t_sample v : *thisVec) {
                if (s == end)
                    break;
                *tab = v;
                tab += bufChans;
                s++;
            }
        }
    }
};


/*
 
// REINSTATE THIS WHEN
class bufAndChan {
public:
    unsigned short buf;
    t_atom_long chan;
    
    bufAndChan(unsigned short b, t_atom_long ch) : buf(b), chan(ch) { }
    
    bool operator==(const bufAndChan other) {
        return buf == other.buf && chan == other.chan;
    }
    
    size_t operator()(const bufAndChan& k) {
        return buf ^ (chan << 2);
    }
};
*/

typedef std::pair<unsigned short, t_atom_long> bufAndChan;

class audioChanMap {
public:
    std::map<bufAndChan, audioChannel*> theMap; // TODO: change to unordered_map
    std::vector<int> chansPerBuf;
    
    audioChanMap() : chansPerBuf(EARSMAP_MAX_OUTPUT_BUFFERS, 0) { }
    
    virtual ~audioChanMap() {
        for (auto i: theMap)
            delete i.second;
    }
    
    audioChannel* retrieveChannel(unsigned short buf, t_atom_long chan) {
        if (auto i = theMap.find(bufAndChan(buf, chan)); i != theMap.end()) {
            return i->second;
        } else
            return nullptr;
    }
    
    audioChannel* getChannel(unsigned short buf, t_atom_long chan) {
        audioChannel *ac = retrieveChannel(buf, chan);
        if (!ac) {
            ac = new audioChannel(buf, chan);
            theMap[bufAndChan(buf, chan)] = ac;
            if (chan > chansPerBuf[buf - 1])
                chansPerBuf[buf - 1] = chan;
        }
        return ac;
    }
    
};








t_object *getParentEarsMap(t_object *x);








#endif /* ears_map_h */