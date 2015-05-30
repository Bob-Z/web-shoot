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

typedef struct internal {
	network_page_t * page;
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

        sprintf(buf,"http://commons.wikimedia.org/wiki/Special:Random/File");
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
                printd(DEBUG_ERROR,"Can't get URL from Wikimedia engine\n");
                return -1;
        }

        if ( web_to_memory(url,internal->page) == -1 ) {
                printd(DEBUG_ERROR,"web_to_memory error\n");
                free(url);
                return -1;
        }

        free(url);

        return 0;
}

/*******************************
  parse_response_page

return string MUST be freed
 ******************************/
static char * parse_response_page(internal_t * internal)
{
        char * substring = NULL;
        char * substring_end = NULL;
        char * url = NULL;
	char buf[SMALL_BUF];

	if( internal == NULL || internal->page == NULL || internal->page->data == NULL) {
		printd(DEBUG_ERROR,"Invalid memory block\n");
		return 0;
	}

	substring=strstr(internal->page->data,"upload.wikimedia.org");

	if( substring == NULL ) {
		printd(DEBUG_ERROR,"No more URL on this page\n");
		return NULL;
	}

	/* get the url */
	substring_end = strstr(substring,"\">");

	url = strndup(substring,substring_end-substring);
	snprintf(buf,sizeof(buf),"http://%s",url);
	printd(DEBUG_URL,"URL: %s\n",buf);

	return url;
}

/*******************************
  engine_get_url

Return string MUST be freed
 ******************************/
static char * engine_get_url(engine_t * engine)
{
	char * url = NULL;
	int res;
	internal_t * internal = engine->internal;

	pthread_mutex_lock(&internal->page_mutex);

	while( url == NULL ) {
		res = get_response_page(internal);
		if( res == -1 || internal->page->data == NULL) {
			printd(DEBUG_PAGE | DEBUG_HTTP,"Can not get result, starting back\n");
			continue;
		}

		url =  parse_response_page(internal);

		if( url == NULL ) {
			printd(DEBUG_PAGE | DEBUG_HTTP,"No URL in this page\n");
		}
	}

	pthread_mutex_unlock(&internal->page_mutex);
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

		free(internal);
	}

	return 0;
}

/******************************
 wikimedia_engine_init
return 0 if no error
******************************/
int wikimedia_engine_init(engine_t * engine,const char * keyword,int size,int filter)
{
	internal_t * internal;

	internal = malloc(sizeof(internal_t));
	memset(internal,0,sizeof(internal_t));

	internal->page = malloc(sizeof(network_page_t));
	memset(internal->page,0,sizeof(network_page_t));

	engine->internal = internal;

	pthread_mutex_init(&internal->page_mutex,NULL);

	engine->engine_destroy=engine_destroy;
	engine->engine_get_url=engine_get_url;

	return 0;
}
