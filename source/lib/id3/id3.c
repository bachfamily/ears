/* id3.c
 *
 * ID3 manipulation tools.
 *
 * Tools for manipulating ID3 tags
*/

/*  id3tool:  An editor for ID3 tags.
 *  Copyright (C) 1999-2002  Christopher Collins
 *
 * This program was authored principly by Christopher Collins (aka
 * Crossfire).
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author, Christopher Collins, nor any of his
 *    aliases may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY CHRISTOPHER COLLINS ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL CHRISTOPHER COLLINS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
*/

//#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "id3.h"

struct style_s	id3_styles[] = {
	{0x00, "Blues"},
	{0x01, "Classic Rock"},
	{0x02, "Country"},
	{0x03, "Dance"},
	{0x04, "Disco"},
	{0x05, "Funk"},
	{0x06, "Grunge"},
	{0x07, "Hip-Hop"},
	{0x08, "Jazz"},
	{0x09, "Metal"},
	{0x0A, "New Age"},
	{0x0B, "Oldies"},
	{0x0C, "Other"},
	{0x0D, "Pop"},
	{0x0E, "R&B"},
	{0x0F, "Rap"},
	{0x10, "Reggae"},
	{0x11, "Rock"},
	{0x12, "Techno"},
	{0x13, "Industrial"},
	{0x14, "Alternative"},
	{0x15, "Ska"},
	{0x16, "Death Metal"},
	{0x17, "Pranks"},
	{0x18, "Soundtrack"},
	{0x19, "Euro-Techno"},
	{0x1A, "Ambient"},
	{0x1B, "Trip-Hop"},
	{0x1C, "Vocal"},
	{0x1D, "Jazz+Funk"},
	{0x1E, "Fusion"},
	{0x1F, "Trance"},
	{0x20, "Classical"},
	{0x21, "Instrumental"},
	{0x22, "Acid"},
	{0x23, "House"},
	{0x24, "Game"},
	{0x25, "Sound Clip"},
	{0x26, "Gospel"},
	{0x27, "Noise"},
	{0x28, "Alt. Rock"},
	{0x29, "Bass"},
	{0x2A, "Soul"},
	{0x2B, "Punk"},
	{0x2C, "Space"},
	{0x2D, "Meditative"},
	{0x2E, "Instrumental Pop"},
	{0x2F, "Instrumental Rock"},
	{0x30, "Ethnic"},
	{0x31, "Gothic"},
	{0x32, "Darkwave"},
	{0x33, "Techno-Industrial"},
	{0x34, "Electronic"},
	{0x35, "Pop-Folk"},
	{0x36, "Eurodance"},
	{0x37, "Dream"},
	{0x38, "Southern Rock"},
	{0x39, "Comedy"},
	{0x3A, "Cult"},
	{0x3B, "Gangsta"},
	{0x3C, "Top 40"},
	{0x3D, "Christian Rap"},
	{0x3E, "Pop/Funk"},
	{0x3F, "Jungle"},
	{0x40, "Native US"},
	{0x41, "Cabaret"},
	{0x42, "New Wave"},
	{0x43, "Psychadelic"},
	{0x44, "Rave"},
	{0x45, "Showtunes"},
	{0x46, "Trailer"},
	{0x47, "Lo-Fi"},
	{0x48, "Tribal"},
	{0x49, "Acid Punk"},
	{0x4A, "Acid Jazz"},
	{0x4B, "Polka"},
	{0x4C, "Retro"},
	{0x4D, "Musical"},
	{0x4E, "Rock & Roll"},
	{0x4F, "Hard Rock"},
	{0x50, "Folk"},
	{0x51, "Folk-Rock"},
	{0x52, "National Folk"},
	{0x53, "Swing"},
	{0x54, "Fast Fusion"},
	{0x55, "Bebob"},
	{0x56, "Latin"},
	{0x57, "Revival"},
	{0x58, "Celtic"},
	{0x59, "Bluegrass"},
        {0x5A, "Avantgarde"},
	{0x5B, "Gothic Rock"},
	{0x5C, "Progressive Rock"},
	{0x5D, "Psychedelic Rock"},
	{0x5E, "Symphonic Rock"},
	{0x5F, "Slow Rock"},
	{0x60, "Big Band"},
	{0x61, "Chorus"},
	{0x62, "Easy Listening"},
	{0x63, "Acoustic"},
	{0x64, "Humour"},
	{0x65, "Speech"},
	{0x66, "Chanson"},
	{0x67, "Opera"},
	{0x68, "Chamber Music"},
	{0x69, "Sonata"},
	{0x6A, "Symphony"},
	{0x6B, "Booty Bass"},
	{0x6C, "Primus"},
	{0x6D, "Porn Groove"},
	{0x6E, "Satire"},
	{0x6F, "Slow Jam"},
	{0x70, "Club"},
	{0x71, "Tango"},
	{0x72, "Samba"},
	{0x73, "Folklore"},
	{0x74, "Ballad"},
	{0x75, "Power Ballad"},
	{0x76, "Rhytmic Soul"},
	{0x77, "Freestyle"},
	{0x78, "Duet"},
	{0x79, "Punk Rock"},
	{0x7A, "Drum Solo"},
	{0x7B, "Acapella"},
	{0x7C, "Euro-House"},
	{0x7D, "Dance Hall"},
	{0x7E, "Goa"},
	{0x7F, "Drum & Bass"},
	{0x80, "Club-House"},
	{0x81, "Hardcore"},
	{0x82, "Terror"},
	{0x83, "Indie"},
	{0x84, "BritPop"},
	{0x85, "Negerpunk"},
	{0x86, "Polsk Punk"},
	{0x87, "Beat"},
	{0x88, "Christian Gangsta Rap"},
	{0x89, "Heavy Metal"},
	{0x8A, "Black Metal"},
	{0x8B, "Crossover"},
	{0x8C, "Contemporary Christian"},
	{0x8D, "Christian Rock"},
	{0x8E, "Merengue"}, 
	{0x8F, "Salsa"},
	{0x90, "Trash Metal"},
	{0xFF, NULL}
};

extern int
id3_readtag(FILE *fin, id3tag_t *id3tag)
{
	int	n;
	if (NULL == fin || NULL == id3tag) {
		return (-1);
	}
	fseek (fin, 0, SEEK_END);
	fseek (fin, ftell(fin)-128, SEEK_SET);
	n = fread (id3tag, 128, 1, fin);
	if (1 != n || strncmp(id3tag->magic, "TAG", 3)) {
		return (1);
	}
	return (0);
}

extern int
id3_appendtag(FILE *fout, id3tag_t *id3tag)
{
	if (NULL == fout || NULL == id3tag) {
		return (-1);
	}
	fseek (fout, 0, SEEK_END);
	
	fwrite (id3tag, 128, 1, fout);
	
	return (0);
}

extern int
id3_replacetag(FILE *fout, id3tag_t *id3tag)
{
	if (NULL == fout || NULL == id3tag) {
		return (-1);
	}
	fseek (fout, 0, SEEK_END);
	fseek (fout, ftell(fout) - 128, SEEK_SET);
	
	fwrite (id3tag, 128, 1, fout);
	
	return (0);  
}
  
extern void
id3_cleartag(id3tag_t *id3tag)
{
	if (NULL == id3tag) {
		return;
	}
	memset (id3tag, 0, sizeof (*id3tag));
	strncpy(id3tag->magic, "TAG", 3);
	id3tag->style = 0xFF;
}

extern char *
id3_findstyle(int styleid)
{
	int	ctr;
	for (ctr = 0; id3_styles[ctr].name != NULL; ctr++) {
		if (id3_styles[ctr].styleid == styleid) {
			return (id3_styles[ctr].name);
		}
	}
	return "Unknown Style";
}
