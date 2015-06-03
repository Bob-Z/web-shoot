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
#include <curl/curl.h>
#include <curl/easy.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "SDL.h"
#include "SDL_image.h"
#include <SDL_opengl.h>
#include "errno.h"
#include "debug.h"
#include "parameter.h"
#include "common.h"
#include "network.h"
#include "engine.h"
#include "misc.h"
#include "image_fifo.h"
#include <pthread.h>
#include <assert.h>

/*******************************
 ******************************/
image_fifo_t * image_fifo_init(int max_img)
{
	image_fifo_t * fifo = NULL;

	fifo = malloc(sizeof(image_fifo_t));
	memset(fifo,0,sizeof(image_fifo_t));

	fifo->image_array = malloc(max_img*sizeof(img_t *));
	memset(fifo->image_array,0,max_img*sizeof(img_t *));
	fifo->image_array_size = max_img;

	pthread_mutex_init(&fifo->write_mutex,NULL);
	pthread_mutex_init(&fifo->read_mutex,NULL);

	sem_init(&fifo->available_entry,0,max_img);

	return fifo;
}
/*******************************
 ******************************/
void image_fifo_push(image_fifo_t * fifo, img_t * img)
{
	sem_wait(&fifo->available_entry);

	pthread_mutex_lock(&fifo->write_mutex);

	if(fifo->image_array[ fifo->write_index ] != NULL) {
		assert(0);
	}

	fifo->image_array[ fifo->write_index ] = img;
	fifo->write_index = (fifo->write_index + 1 )%fifo->image_array_size;

	pthread_mutex_unlock(&fifo->write_mutex);
}
/*******************************
 ******************************/
img_t * image_fifo_pop(image_fifo_t * fifo)
{
	img_t * img;

	pthread_mutex_lock(&fifo->read_mutex);

	img = fifo->image_array[ fifo->read_index ];
	if( img ) {
		fifo->image_array[ fifo->read_index ] = NULL;
		fifo->read_index = (fifo->read_index + 1 )%fifo->image_array_size;
		sem_post(&fifo->available_entry);
	}

	pthread_mutex_unlock(&fifo->read_mutex);

	return img;

}
/*******************************
 ******************************/
void image_fifo_delete(image_fifo_t * fifo)
{
	free(fifo->image_array);
	free(fifo);
}
