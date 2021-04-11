/**
	@file
	ears.libtag_commons.h
	Some libtag functions bridged to ears
 
	by Daniele Ghisi
 */

#ifndef _EARS_BUF_LIBTAG_COMMONS_H_
#define _EARS_BUF_LIBTAG_COMMONS_H_


#include <tlist.h>
#include <fileref.h>
#include <tfile.h>
#include <tag.h>
#include <tpropertymap.h>
#include <mpegfile.h>

#include <apefile.h>
#include <id3v2tag.h>
#include <id3v1tag.h>
#include <apetag.h>
#include <xiphcomment.h>
#include <rifffile.h>
#include <wavfile.h>
#include <aifffile.h>
#include <flacfile.h>
#include <wavpackfile.h>
#include <oggfile.h>
#include <oggflacfile.h>
#include <opusfile.h>
#include <speexfile.h>
#include <vorbisfile.h>

#include <id3v2frame.h>
#include <uniquefileidentifierframe.h>
#include <textidentificationframe.h>
#include <attachedpictureframe.h>
#include <unsynchronizedlyricsframe.h>
#include <synchronizedlyricsframe.h>
#include <eventtimingcodesframe.h>
#include <generalencapsulatedobjectframe.h>
#include <relativevolumeframe.h>
#include <popularimeterframe.h>
#include <urllinkframe.h>
#include <ownershipframe.h>
#include <unknownframe.h>
#include <chapterframe.h>
#include <tableofcontentsframe.h>
#include <commentsframe.h>
#include <podcastframe.h>
#include <privateframe.h>


const char *frameTranslation[][2] = {
    // Text information frames
    { "TALB", "ALBUM"},
    { "TBPM", "BPM" },
    { "TCOM", "COMPOSER" },
    { "TCON", "GENRE" },
    { "TCOP", "COPYRIGHT" },
    { "TDEN", "ENCODINGTIME" },
    { "TDLY", "PLAYLISTDELAY" },
    { "TDOR", "ORIGINALDATE" },
    { "TDRC", "DATE" },
    // { "TRDA", "DATE" }, // id3 v2.3, replaced by TDRC in v2.4
    // { "TDAT", "DATE" }, // id3 v2.3, replaced by TDRC in v2.4
    // { "TYER", "DATE" }, // id3 v2.3, replaced by TDRC in v2.4
    // { "TIME", "DATE" }, // id3 v2.3, replaced by TDRC in v2.4
    { "TDRL", "RELEASEDATE" },
    { "TDTG", "TAGGINGDATE" },
    { "TENC", "ENCODEDBY" },
    { "TEXT", "LYRICIST" },
    { "TFLT", "FILETYPE" },
    //{ "TIPL", "INVOLVEDPEOPLE" }, handled separately
    { "TIT1", "CONTENTGROUP" }, // 'Work' in iTunes
    { "TIT2", "TITLE"},
    { "TIT3", "SUBTITLE" },
    { "TKEY", "INITIALKEY" },
    { "TLAN", "LANGUAGE" },
    { "TLEN", "LENGTH" },
    //{ "TMCL", "MUSICIANCREDITS" }, handled separately
    { "TMED", "MEDIA" },
    { "TMOO", "MOOD" },
    { "TOAL", "ORIGINALALBUM" },
    { "TOFN", "ORIGINALFILENAME" },
    { "TOLY", "ORIGINALLYRICIST" },
    { "TOPE", "ORIGINALARTIST" },
    { "TOWN", "OWNER" },
    { "TPE1", "ARTIST"},
    { "TPE2", "ALBUMARTIST" }, // id3's spec says 'PERFORMER', but most programs use 'ALBUMARTIST'
    { "TPE3", "CONDUCTOR" },
    { "TPE4", "REMIXER" }, // could also be ARRANGER
    { "TPOS", "DISCNUMBER" },
    { "TPRO", "PRODUCEDNOTICE" },
    { "TPUB", "LABEL" },
    { "TRCK", "TRACKNUMBER" },
    { "TRSN", "RADIOSTATION" },
    { "TRSO", "RADIOSTATIONOWNER" },
    { "TSOA", "ALBUMSORT" },
    { "TSOC", "COMPOSERSORT" },
    { "TSOP", "ARTISTSORT" },
    { "TSOT", "TITLESORT" },
    { "TSO2", "ALBUMARTISTSORT" }, // non-standard, used by iTunes
    { "TSRC", "ISRC" },
    { "TSSE", "ENCODING" },
    // URL frames
    { "WCOP", "COPYRIGHTURL" },
    { "WOAF", "FILEWEBPAGE" },
    { "WOAR", "ARTISTWEBPAGE" },
    { "WOAS", "AUDIOSOURCEWEBPAGE" },
    { "WORS", "RADIOSTATIONWEBPAGE" },
    { "WPAY", "PAYMENTWEBPAGE" },
    { "WPUB", "PUBLISHERWEBPAGE" },
    //{ "WXXX", "URL"}, handled specially
    // Other frames
    { "COMM", "COMMENT" },
    //{ "USLT", "LYRICS" }, handled specially
    // Apple iTunes proprietary frames
    { "PCST", "PODCAST" },
    { "TCAT", "PODCASTCATEGORY" },
    { "TDES", "PODCASTDESC" },
    { "TGID", "PODCASTID" },
    { "WFED", "PODCASTURL" },
    { "MVNM", "MOVEMENTNAME" },
    { "MVIN", "MOVEMENTNUMBER" },
    { "GRP1", "GROUPING" },
};
const size_t frameTranslationSize = sizeof(frameTranslation) / sizeof(frameTranslation[0]);


// list of deprecated frames and their successors
const char *deprecatedFrames[][2] = {
    {"TRDA", "TDRC"}, // 2.3 -> 2.4 (http://en.wikipedia.org/wiki/ID3)
    {"TDAT", "TDRC"}, // 2.3 -> 2.4
    {"TYER", "TDRC"}, // 2.3 -> 2.4
    {"TIME", "TDRC"}, // 2.3 -> 2.4
};
const size_t deprecatedFramesSize = sizeof(deprecatedFrames) / sizeof(deprecatedFrames[0]);


TagLib::ByteVector keyToFrameID(const TagLib::String &s)
{
    const TagLib::String key = s.upper();
    for(size_t i = 0; i < frameTranslationSize; ++i) {
        if(key == frameTranslation[i][1])
            return frameTranslation[i][0];
    }
    return TagLib::ByteVector();
}

TagLib::String frameIDToKey(const TagLib::ByteVector &id)
{
    TagLib::ByteVector id24 = id;
    for(size_t i = 0; i < deprecatedFramesSize; ++i) {
        if(id24 == deprecatedFrames[i][0]) {
            id24 = deprecatedFrames[i][1];
            break;
        }
    }
    for(size_t i = 0; i < frameTranslationSize; ++i) {
        if(id24 == frameTranslation[i][0])
            return frameTranslation[i][1];
    }
    return TagLib::String();
}


#endif // _EARS_BUF_RUBBERBAND_COMMONS_H_
