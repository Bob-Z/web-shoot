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
#include "network.h"
#include "loader.h"
#include <pthread.h>
#include <readline/readline.h>
#include <readline/history.h>

typedef struct internal {
	network_page_t * page;
	int read_index;
	char * url;
	char * original_url;
	char * previous_url;
	pthread_mutex_t page_mutex;
} internal_t;

/*******************************
 get_response_page
 return -1 on error
******************************/
static int  get_response_page(internal_t * internal)
{
	if(internal->url == NULL) {
		printd(DEBUG_ERROR,"No URL provided\n");
		return -1;
	}

	if ( web_to_memory(internal->url,internal->page) == -1 ) {
		if(strcmp(internal->url, internal->original_url) == 0)
		{
			printd(DEBUG_ERROR,"web_to_memory error\n");
			return -1;
		}

		free(internal->url);
		internal->url = strdup(internal->previous_url);

		printd(DEBUG_URL,"Go back to previous URL: %s\n", internal->previous_url);

		free(internal->previous_url);
		internal->previous_url = strdup(internal->original_url);

		return 1;
	}

	return 0;
}

/*******************************
  parse_response_page

return string MUST be freed
 ******************************/
static char * parse_response_page(internal_t * internal)
{
	char * substring = NULL;
	char * substring_start = NULL;
	char * substring_end = NULL;
	char * url = NULL;

	if( internal == NULL || internal->page == NULL || internal->page->data == NULL) {
		printd(DEBUG_ERROR,"Invalid memory block\n");
		return 0;
	}

	// try to find an image's URL
	substring=strstr(&internal->page->data[internal->read_index],"<img ");
	if( substring == NULL ) {
		printd(DEBUG_ERROR,"No more image URL on page %s\n",internal->url);
		internal->read_index = 0;
		return NULL;
	}

	internal->read_index = substring - internal->page->data + strlen("<img");

	substring=strstr(&internal->page->data[internal->read_index],"src=");
	if( substring == NULL ) {
		printd(DEBUG_ERROR,"No more image URL on page %s\n",internal->url);
		internal->read_index = 0;
		return NULL;
	}

	// get the url
	substring_start = substring + strlen("src=") +1 ; // +1 is for " or ' after =
	char substring_final_char[2];
	substring_final_char[0] = *(substring_start - 1);
	substring_final_char[1] = 0;
	substring_end = strstr(substring_start,substring_final_char);

	char * partial_url = strndup(substring_start,substring_end-substring_start);

	if( strncmp(partial_url,"http",strlen("http")) == 0 )
	{
		url = strdup(partial_url);
	}
	else
	{
		char full_url[100000];
		strcpy(full_url,internal->url);
		strcat(full_url,partial_url);
		url = strdup(full_url);
	}

	free(partial_url);

	printd(DEBUG_URL,"image URL: %s\n",url);

	internal->read_index = substring_end - internal->page->data;

	return url;
}

/*******************************
  select random  URL in page

return string MUST be freed
 ******************************/
void random_url(internal_t * internal)
{
	char * substring = NULL;
	char * substring_start = NULL;
	char * substring_end = NULL;

	if( internal == NULL || internal->page == NULL || internal->page->data == NULL) {
		printd(DEBUG_ERROR,"Invalid memory block\n");
		return;
	}

	int read_index = 0;

	// try to find an image's URL
	do
	{
		substring=strstr(&internal->page->data[read_index],"href=\"");

		if( substring == NULL ) {
		//	printd(DEBUG_ERROR,"No more URL on page %d\n",internal->url);
			read_index = 0;
			continue;
		}

		// get the url
		substring_start = substring + sizeof("href=\"") - 1;
		substring_end = strstr(substring_start,"\"");

		read_index = substring_end - internal->page->data;
	}
	while( (rand() % 100) != 0 );

	free(internal->previous_url);
	internal->previous_url = internal->url;

	internal->url = strndup(substring_start,substring_end-substring_start);
	printd(DEBUG_URL,"new target URL: %s\n",internal->url);

	internal->read_index = 0;
}

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
			if(internal->page->data) {
				free(internal->page->data);
			}
			free(internal->page);
		}
		if(internal->url) {
			free(internal->url);
		}
		if(internal->original_url) {
			free(internal->original_url);
		}
		if(internal->previous_url) {
			free(internal->previous_url);
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
	char * url = NULL;
	internal_t * internal = static_cast<internal_t*>(engine->internal);

	pthread_mutex_lock(&internal->page_mutex);

	while( url == NULL ) {
		int res = get_response_page(internal);
		if( res == -1) {
			return NULL;
		}
		if( res == 0 )
		{
			url =  parse_response_page(internal);
		}

		if( url == NULL || res == 1) {
			printd(DEBUG_PAGE | DEBUG_HTTP,"No more image URL on page %d\n",internal->url);
			internal->read_index = 0;
			random_url(internal);
		}
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
	srand(time(NULL));

	internal_t * internal;

	printf("Vacuum engine\n");

	internal = static_cast<internal_t*>(malloc(sizeof(internal_t)));
	memset(internal,0,sizeof(internal_t));

	internal->page = static_cast<network_page_t*>(malloc(sizeof(network_page_t)));
	memset(internal->page,0,sizeof(network_page_t));

	engine->internal = internal;

	internal->url = readline("Enter URL: ");
	if(internal->url[0] == 0 ) {
		free(internal->url);
		internal->url = getenv("USER");
	}

	internal->original_url = strdup(internal->url);
	internal->previous_url = strdup(internal->url);

	internal->read_index=0;

	pthread_mutex_init(&internal->page_mutex,NULL);

	engine->engine_destroy=engine_destroy;
	engine->engine_get_url=engine_get_url;

	return 0;
}
	
