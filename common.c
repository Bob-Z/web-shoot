/*
   Web-shooter is a shoot them up game with random graphics.
   Copyright (C) 2013-2015 carabobz@gmail.com

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
*/

#include "common.h"
#include "debug.h"

#define NUM_CODEC 33
#define ESCAPE_CHAR '%'
#define ESC_CODE 1
#define NO_ESC_CODE 0

char * codec[NUM_CODEC][2]= {
	{ " ", "%20" }
	,{ "!", "%21" }
	,{ "\"", "%22" }
	,{ "#", "%23" }
	,{ "$", "%24" }
	,{ "%", "%25" }
	,{ "&", "%26" }
	,{ "'", "%27" }
	,{ "(", "%28" }
	,{ ")", "%29" }
	,{ "*", "%2A" }
	,{ "+", "%2B" }
	,{ ",", "%2C" }
	,{ "-", "%2D" }
	,{ ".", "%2E" }
	,{ "/", "%2F" }
	,{ ":", "%3A" }
	,{ ";", "%3B" }
	,{ "<", "%3C" }
	,{ "=", "%3D" }
	,{ ">", "%3E" }
	,{ "?", "%3F" }
	,{ "@", "%40" }
	,{ "[", "%5B" }
	,{ "\\", "%5C" }
	,{ "]", "%5D" }
	,{ "^", "%5E" }
	,{ "_", "%5F" }
	,{ "`", "%60" }
	,{ "{", "%7B" }
	,{ "|", "%7C" }
	,{ "}", "%7D" }
	,{ "~", "%7E" }
};


void url_percent(const char * src, char * dst)
{
	int si = 0;
	int di = 0;
	int i;
	int found;

	if( ! strncmp(src,"http://",strlen("http://")) ) {
		strncpy(dst,src,strlen("http://"));
		si += strlen("http://");
		di += strlen("http://");
	}
	if( ! strncmp(src,"https://",strlen("https://")) ) {
		strncpy(dst,src,strlen("https://"));
		si += strlen("https://");
		di += strlen("https://");
	}

	while( src[si] != ZCHAR ) {
		found = FALSE;
		dst[di] = ZCHAR;
		for(i=0; i<NUM_CODEC; i++) {
			if( src[si] == codec[i][NO_ESC_CODE][0] ) {
				strcat(dst,codec[i][ESC_CODE]);
				di += strlen(codec[i][1]);
				si++;
				found = TRUE;
				break;
			}
		}

		if( !found ) {
			dst[di]=src[si];
			di++;
			si++;
		}
	}
	dst[di] = ZCHAR;
}

void url_nopercent(const char * src, char * dst)
{
	int si = 0;
	int di = 0;
	int i;
	int found;

	if( ! strncmp(src,"http://",strlen("http://")) ) {
		strncpy(dst,src,strlen("http://"));
		si += strlen("http://");
		di += strlen("http://");
	}
	if( ! strncmp(src,"https://",strlen("https://")) ) {
		strncpy(dst,src,strlen("https://"));
		si += strlen("https://");
		di += strlen("https://");
	}
	if( ! strncmp(src,"http%3A%2F%2F",strlen("http%3A%2F%2F")) ) {
		strncpy(dst,"http://",strlen("http://"));
		si += strlen("http%3A%2F%2F");
		di += strlen("http://");
	}
	if( ! strncmp(src,"https%3A%2F%2F",strlen("https%3A%2F%2F")) ) {
		strncpy(dst,"https://",strlen("https://"));
		si += strlen("https%3A%2F%2F");
		di += strlen("https://");
	}

	while( src[si] != ZCHAR ) {
		found = FALSE;
		dst[di] = ZCHAR;
		if( src[si] == ESCAPE_CHAR) {
			for(i=0; i<NUM_CODEC; i++) {
				if( ! strncmp(&src[si],codec[i][ESC_CODE],strlen(codec[i][ESC_CODE]))) {
					dst[di] = codec[i][NO_ESC_CODE][0];
					di++;
					si += strlen(codec[i][ESC_CODE]);
					found = TRUE;
					break;
				}
			}
			if( ! found ) {
				printd(DEBUG_ERROR, "Unknown escape code : %s\n", &src[si]);
			}
		}

		if( !found ) {
			dst[di]=src[si];
			di++;
			si++;
		}
	}
	dst[di] = ZCHAR;
}

