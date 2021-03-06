/*
   Web-shooter is a shoot them up game with random graphics.
   Copyright (C) 2013-2018 carabobz@gmail.com

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

#include "engine.h"

typedef struct network_page {
	char * data;
	size_t size;
} network_page_t;

int web_to_memory(const char * url, network_page_t * page);
int web_to_disk(char * url,char * filename);
