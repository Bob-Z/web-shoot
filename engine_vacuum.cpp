/*
   Web-shooter is a shoot them up game with random graphics.
   Copyright (C) 2018 carabobz@gmail.com

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
#include "loader.h"
#include <pthread.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "WebPage.h"

typedef struct internal {
	WebPage * page;
	char * url;
	pthread_mutex_t page_mutex;
} internal_t;

/******************************
 engine_destroy
return 0 if no error
******************************/
static int engine_destroy(engine_t * engine)
{
	internal_t * internal = (internal_t*)engine->internal;

	if(internal) {
		pthread_mutex_destroy(&internal->page_mutex);

		if(internal->page) {
			delete internal->page;
		}
		if(internal->url) {
			free(internal->url);
		}

		free(internal);
	}

	return 0;
}

/*******************************
  engine_get_url

Return string MUST be freed
 ******************************/
static char * engine_get_url(engine_t * engine)
{
	char * url = nullptr;
	internal_t * internal = static_cast<internal_t*>(engine->internal);

	pthread_mutex_lock(&internal->page_mutex);

	url = internal->page->getImageUrl();

	if( url == nullptr )
	{
		delete internal->page;
		internal->page = new WebPage(internal->url);
		url = internal->page->getImageUrl();
	}

	pthread_mutex_unlock(&internal->page_mutex);
	return url;
}

/******************************
 vacuum_engine_init
return 0 if no error
******************************/
int vacuum_engine_init(engine_t * engine,const char * url)
{
	srand(time(nullptr));

	internal_t * internal;

	printf("Vacuum engine\n");

	internal = static_cast<internal_t*>(malloc(sizeof(internal_t)));
	memset(internal,0,sizeof(internal_t));

	engine->internal = internal;

	internal->url = readline("Enter URL: ");
	if(internal->url[0] == 0 ) {
		free(internal->url);
		internal->url = strdup("www.google.com");
	}

	internal->page = new WebPage(internal->url);

	pthread_mutex_init(&internal->page_mutex,nullptr);

	engine->engine_destroy=engine_destroy;
	engine->engine_get_url=engine_get_url;

	return 0;
}
	
