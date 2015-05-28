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
#include <pthread.h>

/*******************************
  create_url

You MUST free the return string
 ******************************/
char * create_url(mem_block_t * ctx)
{
        char buf[1024];
        char * url = NULL;
        char word[1024];
	int i;
	int j;

	if ( ctx->image_request_size == NULL) {
		ctx->image_request_size = SIZE_ANY;
	}

	if ( ctx->filter == NULL ) {
		ctx->filter = FILTER_ON;
	}
	j=0;
	for(i=0;i<strlen(ctx->keyword);i++) {
		if(ctx->keyword[i] == ' ') {
			memcpy(&word[j],"%20",strlen("%20"));
			j=j+strlen("%20");
		}
		else {
			word[j] = ctx->keyword[i];
			j++;
		}
	}
	word[j]=0;

        sprintf(buf,"https://yandex.com/images/search?p=%d&text=%s&isize=%s%s",ctx->result_page_num*5,word,ctx->image_request_size,ctx->filter);
	printd(DEBUG_HTTP,"Creating URL : %s\n",buf);
        url = strdup(buf);

        return url;
}

/*******************************
 get_response_page
 return -1 on error
******************************/
static int  get_response_page(mem_block_t * context)
{
        char * url;

        url = create_url(context);
        if(url == NULL) {
                printd(DEBUG_ERROR,"Can't get URL from web engine\n");
                return -1;
        }

        if ( web_to_memory(url,context) == -1 ) {
                printd(DEBUG_ERROR,"web_to_memory error\n");
                free(url);
                return -1;
        }

        free(url);

        context->result_page_num++;

        return 0;
}

/*******************************
  parse_response_page

return string MUST be freed
 ******************************/
static char * parse_response_page(mem_block_t * context)
{
        char * substring = NULL;
        char * substring_start = NULL;
        char * substring_end = NULL;
        char * url = NULL;

	if( context == NULL || context->result_page == NULL) {
		printd(DEBUG_ERROR,"Invalid memory block\n");
		return 0;
	}

	substring=strstr(context->result_page + context->result_read_index,"fullscreen&quot");

	if( substring == NULL ) {
		printd(DEBUG_ERROR,"No more URL on page %d\n",context->result_page_num);
		return NULL;
	}

	/* get the url */
	substring_start=strstr(substring,"http");
	substring_end = strstr(substring_start+strlen("http"),"&quot;");

	url = strndup(substring_start,substring_end-substring_start);
	printd(DEBUG_URL,"URL: %s\n",url);

	context->result_read_index = substring_end - context->result_page;

	return url;
}

/******************************
 engine_create
return 0 if no error
******************************/
int engine_init(mem_block_t * context,const char * keyword,const char * size,const char * filter)
{
	context->result_page=NULL;
	context->result_page_size=0;
	context->result_page_num=0;
	context->result_read_index=0;
	context->image_request_size=strdup(size);
	context->keyword=strdup(keyword);
	context->filter=strdup(filter);
	pthread_mutex_init(&context->page_mutex,NULL);

	return 0;
}

/******************************
 engine_destroy
return 0 if no error
******************************/
int engine_destroy(mem_block_t * context)
{
	if(context->image_request_size) {
		free(context->image_request_size);
	}
	if(context->keyword) {
		free(context->keyword);
	}
	if(context->filter) {
		free(context->filter);
	}

	pthread_mutex_destroy(&context->page_mutex);

	return 0;
}

/*******************************
  engine_get_url

Return string MUST be freed
 ******************************/
char * engine_get_url(mem_block_t * context)
{
	int first_page = FALSE;
	char * url = NULL;
	int res;

	pthread_mutex_lock(&context->page_mutex);

	while( url == NULL ) {
		while( context->result_page == NULL ) {
			printd(DEBUG_PAGE | DEBUG_HTTP,"Reading result page %d for keyword \"%s\"\n",context->result_page_num,context->keyword);
			res = get_response_page(context);
			if( res == -1) {
				printd(DEBUG_PAGE | DEBUG_HTTP,"Can not get result page %d for keyword \"%s\", starting back\n",context->result_page_num,context->keyword);
				if (first_page == TRUE) {
					printd(DEBUG_PAGE | DEBUG_HTTP,"No URL for keyword \"%s\"\n",context->keyword);
					pthread_mutex_unlock(&context->page_mutex);
					return NULL;
				}

				context->result_page = NULL;
				context->result_page_size = 0;
				context->result_read_index = 0;
				context->result_page_num = 0;
				first_page = TRUE;
			}
			else {
				printd(DEBUG_PAGE | DEBUG_HTTP,"Got result page %d for keyword \"%s\"\n",context->result_page_num-1,context->keyword);
			}
		}

		url =  parse_response_page(context);
		if( url == NULL ) {
			printd(DEBUG_PAGE | DEBUG_HTTP,"No more URL for keyword \"%s\" on page %d\n",context->keyword,context->result_page_num);
			context->result_page_num++;
			free(context->result_page);
			context->result_page=NULL;
			context->result_page_size=0;
			context->result_read_index=0;
		}
	}

	pthread_mutex_unlock(&context->page_mutex);
	return url;
}
