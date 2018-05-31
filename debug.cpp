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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdarg.h>
#include "parameter.h"
#include <time.h>
#include "SDL.h"

int debug = DEBUG_ERROR | DEBUG_HTTP | DEBUG_URL;
//int debug = DEBUG_ERROR | DEBUG_HTTP ;
//int debug = DEBUG_LEVEL;

/* printd */
int print_debug(const char * file, int line, const char * function, int filter,const char *fmt, ...)
{
	va_list ap;
	int ret;

	if(debug & filter) {
		printf("%7d:%12s(%3d)|%2d|%21s|",SDL_GetTicks(),file,line,filter,function);
		va_start(ap, fmt);
		ret = vprintf(fmt, ap);
		va_end(ap);
	}

	return ret;
}
