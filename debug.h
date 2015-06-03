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

#define DEBUG_ERROR 1
#define DEBUG_URL 1<<1
#define DEBUG_SDL 1<<2
#define DEBUG_HTTP 1<<3
#define DEBUG_IMAGE_CACHE 1<<4
#define DEBUG_PAGE 1<<5
#define DEBUG_DISK 1<<6

#define printd(filter,fmt, ...) print_debug(__FILE__,__LINE__,__func__,filter,fmt, ##__VA_ARGS__)
int print_debug(const char *file, int line, const char * function, int filter,const char *fmt, ...);
