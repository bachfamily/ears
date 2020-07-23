// ID3V1 addition to the id3lib header files by Daniele Ghisi
#include "id3add.h"


long string_to_frameid(const char *code)
{
    if (strcmp(code, "title") == 0 || strcmp(code, "song") == 0 || strcmp(code, "Title") == 0 || strcmp(code, "Song") == 0)
        return ID3FID_TITLE;

    if (strcmp(code, "artist") == 0 || strcmp(code, "Artist") == 0)
        return ID3FID_LEADARTIST;

    if (strcmp(code, "album") == 0 || strcmp(code, "Album") == 0)
        return ID3FID_ALBUM;

    if (strcmp(code, "year") == 0 || strcmp(code, "Year") == 0)
        return ID3FID_YEAR;

    if (strcmp(code, "comment") == 0 || strcmp(code, "Comment") == 0)
        return ID3FID_COMMENT;

    if (strcmp(code, "track") == 0 || strcmp(code, "Track") == 0)
        return ID3FID_TRACKNUM;

    if (strcmp(code, "genre") == 0 || strcmp(code, "Genre") == 0 || strcmp(code, "style") == 0 || strcmp(code, "Style") == 0)
        return ID3FID_CONTENTTYPE;

    return ID3FID_NOFRAME;
}


long code_to_id3v2_frameid(const char *code)
{
    uint32_t fourchars_code = 0;
    long i;
    //    x = (a[0] << 24) | (a[1] << 16) | (a[2] << 8) | a[3];
    for (i = 0; i < 4 && i < strlen(code); i++)
        fourchars_code |= (code[i] << (8 * (4 - i - 1)));
    
    switch (fourchars_code) {
        case 'AENC': return ID3FID_AUDIOCRYPTO;
        case 'APIC':    return ID3FID_PICTURE;
        case 'COMM':    return ID3FID_COMMENT;
            /* COMR too complex */
        case 'ENCR':    return ID3FID_CRYPTOREG;
        case 'EQUA':    return ID3FID_EQUALIZATION;
        case 'ETCO':    return ID3FID_EVENTTIMING;
        case 'GEOB':    return ID3FID_GENERALOBJECT;
        case 'GRID':    return ID3FID_GROUPINGREG;
        case 'IPLS':    return ID3FID_INVOLVEDPEOPLE;
        case 'LINK':    return ID3FID_LINKEDINFO;
        case 'MCDI':    return ID3FID_CDID;
        case 'MLLT':    return ID3FID_MPEGLOOKUP;
        case 'OWNE':    return ID3FID_OWNERSHIP;
        case 'PRIV':    return ID3FID_PRIVATE;
        case 'PCNT':    return ID3FID_PLAYCOUNTER;
        case 'POPM':    return ID3FID_POPULARIMETER;
        case 'POSS':    return ID3FID_POSITIONSYNC;
        case 'RBUF':    return ID3FID_BUFFERSIZE;
        case 'RVAD':    return ID3FID_VOLUMEADJ;
        case 'RVRB':    return ID3FID_REVERB;
        case 'SYLT':    return ID3FID_SYNCEDLYRICS;
        case 'SYTC':    return ID3FID_SYNCEDTEMPO;
        case 'TALB':    return ID3FID_ALBUM;
        case 'TBPM':    return ID3FID_BPM;
        case 'TCOM':    return ID3FID_COMPOSER;
        case 'TCON':    return ID3FID_CONTENTTYPE;
        case 'TCOP':    return ID3FID_COPYRIGHT;
        case 'TDAT':    return ID3FID_DATE;
        case 'TDLY':    return ID3FID_PLAYLISTDELAY;
        case 'TENC':    return ID3FID_ENCODEDBY;
        case 'TEXT':    return ID3FID_LYRICIST;
        case 'TFLT':    return ID3FID_FILETYPE;
        case 'TIME':    return ID3FID_TIME;
        case 'TIT1':    return ID3FID_CONTENTGROUP;
        case 'TIT2':    return ID3FID_TITLE;
        case 'TIT3':    return ID3FID_SUBTITLE;
        case 'TKEY':    return ID3FID_INITIALKEY;
        case 'TLAN':    return ID3FID_LANGUAGE;
        case 'TLEN':    return ID3FID_SONGLEN;
        case 'TMED':    return ID3FID_MEDIATYPE;
        case 'TOAL':    return ID3FID_ORIGALBUM;
        case 'TOFN':    return ID3FID_ORIGFILENAME;
        case 'TOLY':    return ID3FID_ORIGLYRICIST;
        case 'TOPE':    return ID3FID_ORIGARTIST;
        case 'TORY':    return ID3FID_ORIGYEAR;
        case 'TOWN':    return ID3FID_FILEOWNER;
        case 'TPE1':    return ID3FID_LEADARTIST;
        case 'TPE2':    return ID3FID_BAND;
        case 'TPE3':    return ID3FID_CONDUCTOR;
        case 'TPE4':    return ID3FID_MIXARTIST;
        case 'TPOS':    return ID3FID_PARTINSET;
        case 'TPUB':    return ID3FID_PUBLISHER;
        case 'TRCK':    return ID3FID_TRACKNUM;
        case 'TRDA':    return ID3FID_RECORDINGDATES;
        case 'TRSN':    return ID3FID_NETRADIOSTATION;
        case 'TRSO':    return ID3FID_NETRADIOOWNER;
        case 'TSIZ':    return ID3FID_SIZE;
        case 'TSRC':    return ID3FID_ISRC;
        case 'TSSE':    return ID3FID_ENCODERSETTINGS;
        case 'TXXX':    return ID3FID_USERTEXT;
        case 'TYER':    return ID3FID_YEAR;
        case 'UFID':    return ID3FID_UNIQUEFILEID;
        case 'USER':    return ID3FID_TERMSOFUSE;
        case 'USLT':    return ID3FID_UNSYNCEDLYRICS;
        case 'WCOM':    return ID3FID_WWWCOMMERCIALINFO;
        case 'WCOP':    return ID3FID_WWWCOPYRIGHT;
        case 'WOAF':    return ID3FID_WWWAUDIOFILE;
        case 'WOAR':    return ID3FID_WWWARTIST;
        case 'WOAS':    return ID3FID_WWWAUDIOSOURCE;
        case 'WORS':    return ID3FID_WWWRADIOPAGE;
        case 'WPAY':    return ID3FID_WWWPAYMENT;
        case 'WPUB':    return ID3FID_WWWPUBLISHER;
        case 'WXXX':    return ID3FID_WWWUSER;
        default:        return ID3FID_NOFRAME;
    }
}
