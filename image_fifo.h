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

#include <stdlib.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <pthread.h>
#include <semaphore.h>
#include "image.h"

typedef struct image_fifo {
        img_t ** image_array;
        int image_array_size;
	int read_index;
	pthread_mutex_t read_mutex;
	int write_index;
	pthread_mutex_t write_mutex;
	sem_t available_entry;
} image_fifo_t;

image_fifo_t * image_fifo_init(int max_img);
void image_fifo_push(image_fifo_t * fifo, img_t * img);
img_t * image_fifo_pop(image_fifo_t * fifo);
void image_fifo_delete(image_fifo_t * fifo);
