// ID3V1 addition to the id3lib header files by Daniele Ghisi

#ifndef __ID3ADD_H__
#define __ID3ADD_H__

#include "id3/utils.h"
#include "id3/misc_support.h"
#include "id3/readers.h"


typedef struct _id3 {
    char tag[3];
    char title[30];
    char artist[30];
    char album[30];
    char year[4];
    /* With ID3 v1.0, the comment is 30 chars long */
    /* With ID3 v1.1, if comment[28] == 0 then comment[29] == tracknum */
    char comment[30];
    unsigned char genre;
} t_id3v1tag;


long code_to_id3v2_frameid(const char *code);
long string_to_frameid(const char *code);

#endif /* __ID3ADD_H__ */
