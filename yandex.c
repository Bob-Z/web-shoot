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

char * size_string[SIZE_NUM] = {
	"small"
	,"medium"
	,"large"
};

char * filter_string[FILTER_NUM] = {
	"&ncrnd=5290"
	,"&ncrnd=9763"
};

/*******************************
  create_url

You MUST free the return string
 ******************************/
char * create_url(engine_t * engine)
{
        char buf[1024];
        char * url = NULL;
        char word[1024];
	int i;
	int j;

	j=0;
	for(i=0;i<strlen(engine->keyword);i++) {
		if(engine->keyword[i] == ' ') {
			memcpy(&word[j],"%20",strlen("%20"));
			j=j+strlen("%20");
		}
		else {
			word[j] = engine->keyword[i];
			j++;
		}
	}
	word[j]=0;

        sprintf(buf,"https://yandex.com/images/search?p=%d&text=%s&isize=%s%s",engine->result_page_num*5,word,engine->image_size,engine->filter);
	printd(DEBUG_HTTP,"Creating URL : %s\n",buf);
        url = strdup(buf);

        return url;
}

/*******************************
 get_response_page
 return -1 on error
******************************/
static int  get_response_page(engine_t * engine)
{
        char * url;

        url = create_url(engine);
        if(url == NULL) {
                printd(DEBUG_ERROR,"Can't get URL from web engine\n");
                return -1;
        }

        if ( web_to_memory(url,engine) == -1 ) {
                printd(DEBUG_ERROR,"web_to_memory error\n");
                free(url);
                return -1;
        }

        free(url);

        engine->result_page_num++;

        return 0;
}

/*******************************
  parse_response_page

return string MUST be freed
 ******************************/
static char * parse_response_page(engine_t * engine)
{
        char * substring = NULL;
        char * substring_start = NULL;
        char * substring_end = NULL;
        char * url = NULL;

	if( engine == NULL || engine->result_page == NULL) {
		printd(DEBUG_ERROR,"Invalid memory block\n");
		return 0;
	}

	substring=strstr(engine->result_page + engine->result_read_index,"fullscreen&quot");

	if( substring == NULL ) {
		printd(DEBUG_ERROR,"No more URL on page %d\n",engine->result_page_num);
		return NULL;
	}

	/* get the url */
	substring_start=strstr(substring,"http");
	substring_end = strstr(substring_start+strlen("http"),"&quot;");

	url = strndup(substring_start,substring_end-substring_start);
	printd(DEBUG_URL,"URL: %s\n",url);

	engine->result_read_index = substring_end - engine->result_page;

	return url;
}

/******************************
 engine_destroy
return 0 if no error
******************************/
int yandex_engine_destroy(engine_t * engine)
{
	if(engine->keyword) {
		free(engine->keyword);
	}

	pthread_mutex_destroy(&engine->page_mutex);

	return 0;
}

/*******************************
  engine_get_url

Return string MUST be freed
 ******************************/
char * yandex_engine_get_url(engine_t * engine)
{
	int first_page = FALSE;
	char * url = NULL;
	int res;

	pthread_mutex_lock(&engine->page_mutex);

	while( url == NULL ) {
		while( engine->result_page == NULL ) {
			printd(DEBUG_PAGE | DEBUG_HTTP,"Reading result page %d for keyword \"%s\"\n",engine->result_page_num,engine->keyword);
			res = get_response_page(engine);
			if( res == -1 || engine->result_page == NULL) {
				printd(DEBUG_PAGE | DEBUG_HTTP,"Can not get result page %d for keyword \"%s\", starting back\n",engine->result_page_num,engine->keyword);

				engine->result_page = NULL;
				engine->result_page_size = 0;
				engine->result_read_index = 0;
				engine->result_page_num = 0;

				if (first_page == TRUE) {
					printd(DEBUG_PAGE | DEBUG_HTTP,"No URL for keyword \"%s\"\n",engine->keyword);
					pthread_mutex_unlock(&engine->page_mutex);
					return NULL;
				}

				first_page = TRUE;
			}
			else {
				printd(DEBUG_PAGE | DEBUG_HTTP,"Got result page %d for keyword \"%s\"\n",engine->result_page_num-1,engine->keyword);
			}
		}

		url =  parse_response_page(engine);
		if( url == NULL ) {
			printd(DEBUG_PAGE | DEBUG_HTTP,"No more URL for keyword \"%s\" on page %d\n",engine->keyword,engine->result_page_num);
			engine->result_page_num++;
			free(engine->result_page);
			engine->result_page=NULL;
			engine->result_page_size=0;
			engine->result_read_index=0;
		}
	}

	pthread_mutex_unlock(&engine->page_mutex);
	return url;
}

/******************************
 engine_create
return 0 if no error
******************************/
int yandex_engine_init(engine_t * engine,const char * keyword,int size,int filter)
{
	engine->result_page=NULL;
	engine->result_page_size=0;
	engine->result_page_num=0;
	engine->result_read_index=0;

	engine->image_size=size_string[size];
	engine->keyword=strdup(keyword);
	engine->filter=filter_string[filter];

	pthread_mutex_init(&engine->page_mutex,NULL);

	engine->engine_destroy=yandex_engine_destroy;
	engine->engine_get_url=yandex_engine_get_url;

	return 0;
}
