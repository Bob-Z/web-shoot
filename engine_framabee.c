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
#include "loader.h"
#include <pthread.h>
#include <readline/readline.h>
#include <readline/history.h>

#define FIRST_PAGE (1)

typedef struct internal {
	network_page_t * page;
	int page_num;
	int read_index;
	char * keyword;
	pthread_mutex_t page_mutex;
} internal_t;

/*******************************
  create_url

You MUST free the return string
 ******************************/
static char * create_url(internal_t * internal)
{
	char buf[1024];
	char * url = NULL;
	char word[1024];

	url_percent(internal->keyword,word);

	sprintf(buf,"https://framabee.org/?q=%s&pageno=%d&category_images",word,internal->page_num);
	printd(DEBUG_HTTP,"Creating URL : %s\n",buf);
	url = strdup(buf);

	return url;
}

/*******************************
 get_response_page
 return -1 on error
******************************/
static int  get_response_page(internal_t * internal)
{
	char * url;

	url = create_url(internal);
	if(url == NULL) {
		printd(DEBUG_ERROR,"Can't get URL from Yandex engine\n");
		return -1;
	}

	if ( web_to_memory(url,internal->page) == -1 ) {
		printd(DEBUG_ERROR,"web_to_memory error\n");
		free(url);
		return -1;
	}

	free(url);

	internal->page_num++;

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

	substring=internal->page->data + internal->read_index;

	if( internal->read_index == 0) {
		substring=strstr(internal->page->data + internal->read_index,"<div class=\"result result-images\">");

		if( substring == NULL ) {
			printd(DEBUG_ERROR,"No more URL on page %d\n",internal->page_num);
			return NULL;
		}
	}

	substring=strstr(substring,"<a href=");

	if( substring == NULL ) {
		printd(DEBUG_ERROR,"No more URL on page %d\n",internal->page_num);
		return NULL;
	}

	/* get the url */
	substring_start=strstr(substring,"http");
	substring_end = strstr(substring_start+strlen("http"),"\"");

	url = strndup(substring_start,substring_end-substring_start);
	printd(DEBUG_URL,"URL: %s\n",url);

	internal->read_index = substring_end - internal->page->data;

	return url;
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

		if(internal->keyword) {
			free(internal->keyword);
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
	int first_page = FALSE;
	char * url = NULL;
	int res;
	internal_t * internal = engine->internal;

	pthread_mutex_lock(&internal->page_mutex);

	while( url == NULL ) {
		while( internal->page->data == NULL ) {
			printd(DEBUG_PAGE | DEBUG_HTTP,"Reading result page %d for keyword \"%s\"\n",internal->page_num,internal->keyword);
			res = get_response_page(internal);
			if( res == -1 || internal->page->data == NULL) {
				printd(DEBUG_PAGE | DEBUG_HTTP,"Can not get result page %d for keyword \"%s\", starting back\n",internal->page_num,internal->keyword);

				internal->read_index = 0;
				internal->page_num = FIRST_PAGE;

				if (first_page == TRUE) {
					printd(DEBUG_PAGE | DEBUG_HTTP,"No URL for keyword \"%s\"\n",internal->keyword);
					pthread_mutex_unlock(&internal->page_mutex);
					return NULL;
				}

				first_page = TRUE;
			} else {
				printd(DEBUG_PAGE | DEBUG_HTTP,"Got result page %d for keyword \"%s\"\n",internal->page_num-1,internal->keyword);
			}
		}

		url =  parse_response_page(internal);
		if( url == NULL ) {
			printd(DEBUG_PAGE | DEBUG_HTTP,"No more URL for keyword \"%s\" on page %d\n",internal->keyword,internal->page_num);
			free(internal->page->data);
			internal->page->data=NULL;
			internal->page->size=0;
			internal->read_index=0;
		}
	}

	pthread_mutex_unlock(&internal->page_mutex);
	return url;
}

/******************************
 yandex_engine_init
return 0 if no error
******************************/
int framabee_engine_init(engine_t * engine,const char * keyword,int size,int filter)
{
	internal_t * internal;

	printf("Framabee engine\n");

	internal = malloc(sizeof(internal_t));
	memset(internal,0,sizeof(internal_t));

	internal->page = malloc(sizeof(network_page_t));
	memset(internal->page,0,sizeof(network_page_t));

	engine->internal = internal;

	internal->page_num=FIRST_PAGE;
	internal->read_index=0;

	if(keyword == NULL) {
		internal->keyword = readline("Enter key word: ");
		if(internal->keyword[0] == 0 ) {
			free(internal->keyword);
			internal->keyword = getenv("USER");
		}
	} else {
		internal->keyword = strdup(keyword);
	}

	pthread_mutex_init(&internal->page_mutex,NULL);

	engine->engine_destroy=engine_destroy;
	engine->engine_get_url=engine_get_url;

	return 0;
}
