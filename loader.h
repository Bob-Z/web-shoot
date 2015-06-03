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

#include <pthread.h>
#include "image_fifo.h"
#include "engine.h"

enum image_size {
	SIZE_SMALL
	,SIZE_MEDIUM
	,SIZE_LARGE
	,SIZE_NUM
};

enum filter_activated {
	FILTER_ON
	,FILTER_OFF
	,FILTER_NUM
};

enum engine_type {
	ENG_TEST
	,ENG_YANDEX
	,ENG_FILE
	,ENG_WIKIMEDIA
	,ENG_DEVIANTART
	,ENG_NUM
};

typedef struct loader {
	engine_t * engine;
	image_fifo_t * image_fifo;
	pthread_t * thread_array;
} loader_t;

loader_t * loader_init(int engine, int max_img, char * keyword, int size, int filter);
void loader_delete(loader_t * loader);
img_t * loader_get_img(loader_t * loader);
