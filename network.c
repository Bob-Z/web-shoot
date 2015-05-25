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
 ******************************/
static char * create_url(mem_block_t * ctx)
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
  data_to_mem callback
 ******************************/
static size_t data_to_mem(void *buffer, size_t size, size_t nmemb, void *userp)
{
        mem_block_t * context;

        context = (mem_block_t *)userp;

        context->result_page = realloc(context->result_page,context->result_page_size + (size*nmemb) + 1 ); /*for terminal NULL */
        if ( context->result_page == NULL ) {
		printd(DEBUG_HTTP,"Cannot realloc\n");
		return 0;
	}
        memcpy(context->result_page+context->result_page_size, buffer, size*nmemb);
        context->result_page_size += (size*nmemb);
        context->result_page[context->result_page_size] = 0; /*terminal NULL */

        return (size*nmemb);
}

/*******************************
  data_to_file callback
 ******************************/
static size_t data_to_file(void *buffer, size_t size, size_t nmemb, void *userp)
{
        ssize_t ret = 0;
        int fd = *(int *)userp;

        ret = write(fd,buffer,size*nmemb);

        return ret;
}

static void * async_perform(void * arg)
{
	return (void *)curl_easy_perform((CURL *)arg);
}

/*******************************
  web_to_memory

return -1 on error
return 0 on success
 ******************************/
static int web_to_memory( char * url, mem_block_t * context)
{
        CURL * easyhandle;
	char * proxy;
	int err;
	pthread_t thread;
	void * thread_ret;
	struct timespec t;
	char curl_error_buffer[CURL_ERROR_SIZE];

	t.tv_sec = time(NULL) + DEF_HTTP_TIMEOUT;
	t.tv_nsec = 0;

        easyhandle = curl_easy_init();
	if(easyhandle == NULL){
		printd(DEBUG_ERROR,"curl_easy_init failed: %s",curl_error_buffer);
		return -1;
	}

	proxy = getenv("http_proxy");
	if(proxy) {
		printd(DEBUG_HTTP,"Set proxy to %s\n",proxy);
        	curl_easy_setopt(easyhandle, CURLOPT_PROXY, proxy);
	}

        curl_easy_setopt(easyhandle, CURLOPT_URL, url);
        curl_easy_setopt(easyhandle, CURLOPT_WRITEFUNCTION, data_to_mem);
        curl_easy_setopt(easyhandle, CURLOPT_WRITEDATA, (void *)context);
	curl_easy_setopt(easyhandle, CURLOPT_TIMEOUT, DEF_HTTP_TIMEOUT);
	curl_easy_setopt(easyhandle, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(easyhandle, CURLOPT_ERRORBUFFER, curl_error_buffer);

	printd(DEBUG_HTTP,"curl_easy_perform\n");

	pthread_create(&thread,NULL,async_perform,easyhandle);
	err = pthread_timedjoin_np(thread,&thread_ret,&t);
	if(err) {
		pthread_cancel(thread);
		printd(DEBUG_ERROR,"pthread_timedjoin_np failed: %s\n",strerror(err));
		curl_easy_cleanup(easyhandle);
		return -1;
	}
	if(thread_ret != CURLE_OK) {
		printd(DEBUG_ERROR,"curl_easy_perform failed: %s\n",curl_error_buffer);
		curl_easy_cleanup(easyhandle);
		return -1;
	}

	printd(DEBUG_HTTP,"curl_easy_perform succeed\n");
        curl_easy_cleanup(easyhandle);

	return 0;
}

/*******************************
  web_to_disk

return -1 on error
return 0 on success
 ******************************/
static int web_to_disk( char * url, mem_block_t * context,int index)
{
        CURL * easyhandle;
	char * proxy;
	int err;
	int fd;
        char filename[1024];
        pthread_t thread;
        void * thread_ret;
	struct timespec t;
	char curl_error_buffer[CURL_ERROR_SIZE];

	t.tv_sec = time(NULL) + DEF_HTTP_TIMEOUT;
	t.tv_nsec = 0;

	sprintf(filename,"%s-%s.%d",TMP_FILE,context->keyword,index);

	fd = open(filename,O_CREAT| O_TRUNC | O_RDWR, S_IRWXU);
	if( fd == -1 ) {
		printd(DEBUG_ERROR,"Can't open %s\n", filename);
		return -1;
	}

        easyhandle = curl_easy_init();
	if(easyhandle == NULL){
		close(fd);
		printd(DEBUG_ERROR,"curl_easy_init failed");
		return -1;
	}

	proxy = getenv("http_proxy");
	if(proxy) {
		printd(DEBUG_HTTP,"Set proxy to %s\n",proxy);
        	curl_easy_setopt(easyhandle, CURLOPT_PROXY, proxy);
	}
        curl_easy_setopt(easyhandle, CURLOPT_URL, url);
        curl_easy_setopt(easyhandle, CURLOPT_WRITEFUNCTION, data_to_file);
        curl_easy_setopt(easyhandle, CURLOPT_WRITEDATA, (void *)&fd);
	curl_easy_setopt(easyhandle, CURLOPT_TIMEOUT, DEF_HTTP_TIMEOUT);
	curl_easy_setopt(easyhandle, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(easyhandle, CURLOPT_ERRORBUFFER, curl_error_buffer);

	printd(DEBUG_HTTP,"curl_easy_perform\n");

        pthread_create(&thread,NULL,async_perform,easyhandle);
        err = pthread_timedjoin_np(thread,&thread_ret,&t);
        if(err) {
                pthread_cancel(thread);
		printd(DEBUG_ERROR,"pthread_timedjoin_np failed: %s\n",strerror(err));
		curl_easy_cleanup(easyhandle);
		close(fd);
		return -1;
	}
	if(thread_ret != CURLE_OK) {
		printd(DEBUG_ERROR,"curl_easy_perform failed: %s\n",curl_error_buffer);
		curl_easy_cleanup(easyhandle);
		close(fd);
		return -1;
	}

	printd(DEBUG_HTTP,"curl_easy_perform succeed\n");
        curl_easy_cleanup(easyhandle);

	close(fd);

	return 0;
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
                printd(DEBUG_ERROR,"create_url error\n");
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

  returns NULL if no more image or system error
  returns  a SDL_Surface pointer to an image
 ******************************/
static SDL_Surface * parse_response_page(mem_block_t * context)
{
        char * substring = NULL;
        char * substring_start = NULL;
        char * substring_end = NULL;
        char * url = NULL;
	char filename[1024];
	SDL_Surface * image;
	static int index = 0;
	int err;

	if( context == NULL || context->result_page == NULL) {
		printd(DEBUG_ERROR,"Invalid memory block\n");
		return 0;
	}

	/* Find search result start */
	while((substring=strstr(context->result_page + context->result_read_index,"fullscreen&quot"))!= NULL){

		/* get the url */
		substring_start=strstr(substring,"http");
		substring_end = strstr(substring_start+strlen("http"),"&quot;");

		url = strndup(substring_start,substring_end-substring_start);
		printd(DEBUG_URL,"URL: %s\n",url);

		context->result_read_index = substring_end - context->result_page;

		/* Is this an image ? */
		if(strcasecmp(".jpg",url+strlen(url)-4) == 0 ||
				strcasecmp(".gif",url+strlen(url)-4) == 0 ||
				strcasecmp(".jpeg",url+strlen(url)-5) == 0 ||
				strcasecmp(".png",url+strlen(url)-4) == 0) {

			printd(DEBUG_HTTP,"Found a potential resource: %s\n",url);
			/* Download the resource */
			err = web_to_disk(url,context,index);

			if( err == -1) {
				printd(DEBUG_ERROR,"Error fetching %s\n", url);
				free(url);
				continue;
			}

			/*  image is ready */
			/* Try to read the image to be sure it is a valid image*/
			sprintf(filename,"%s-%s.%d",TMP_FILE,context->keyword,index);
			image=IMG_Load(filename);
			if(image) {
				/* this is an image */
				/* Check the compatibility with OpenGL */
				if( image->format->BytesPerPixel == 4 || image->format->BytesPerPixel == 3 ) {
					/* this is a valid image */
					printd(DEBUG_HTTP,"%s is a valid image\n",filename);

					free(url);
					index++;
					return image;
				}
			}
			printd(DEBUG_HTTP,"%s is a NOT valid image\n",filename);
		}
		free(url);
		url = NULL;
	}

	printd(DEBUG_HTTP,"No more URL in parsed page\n");
	return NULL;
}


/* return NULL on error
   return a SDL_Surface on success
 */
static SDL_Surface * fetch_image(mem_block_t *context)
{
	int res;
	SDL_Surface * img = NULL;
	int empty_page = 0;

	do {
		/* Get a results page if none exists */
		if( context->result_page == NULL ) {
			printd(DEBUG_HTTP,"Fetch result page %d for keyword \"%s\"\n",context->result_page_num,context->keyword);
			res = get_response_page(context);
			if( res == -1) {
				printd(DEBUG_PAGE | DEBUG_HTTP,"Can not get result page %d for keyword \"%s\", starting back\n",context->result_page_num,context->keyword);
				context->result_page = NULL;
				context->result_page_size = 0;
				context->result_read_index = 0;
				context->result_page_num = 0;	
				continue;
			}
			printd(DEBUG_PAGE | DEBUG_HTTP,"Got result page %d for keyword \"%s\"\n",context->result_page_num-1,context->keyword);
		}

		printd(DEBUG_HTTP,"Try to find a valid url in page %d\n",context->result_page_num);
		img = parse_response_page(context);
		if(img == NULL ) {
			free(context->result_page);
			context->result_page = NULL;
			context->result_page_size = 0;
			context->result_read_index = 0;
			printd(DEBUG_PAGE | DEBUG_HTTP,"No more image on result page %d for keyword \"%s\", trying page %d\n",context->result_page_num-1, context->keyword,context->result_page_num);
			empty_page++;
			if( empty_page > 2 ) {
				printd(DEBUG_PAGE | DEBUG_HTTP,"No more image for keyword \"%s\", starting back\n",context->keyword);
				context->result_page_num = 0;
			}
		}
		else {
			empty_page = 0;
		}
	} while(img == NULL);

        return img;
}

void * network_load_image(void * arg)
{
        int i = 0;
        mem_block_t  context;
        SDL_Surface * img = NULL;
        load_context_t * load_ctx = (load_context_t *)arg;
        pic_t ** pic = load_ctx->image_array;

        printd(DEBUG_IMAGE_CACHE,"Entering for %s\n",load_ctx->type);
        memset(&context,0,sizeof(mem_block_t));
        memset(&pic[0],0,load_ctx->image_array_size *sizeof(pic_t *));
        context.keyword = strdup(load_ctx->keyword);
        context.image_request_size = load_ctx->size;
        context.filter = load_ctx->filter;

        while(1) {
                if(pic[i]==NULL) {
                        img = fetch_image(&context);
                        if(img == NULL ) {
                                printd(DEBUG_ERROR,"No more pics for %s request\n",load_ctx->type);
                                return NULL;
                        }
                        printd(DEBUG_IMAGE_CACHE,"adding %s %d\n",load_ctx->type,i);

                        pic[i]=malloc(sizeof(pic_t));
                        memset(pic[i],0,sizeof(pic_t));
                        pic[i]->surf = img;
                        pic[i]->ratio = (double)(pic[i]->surf->w) / (double)(pic[i]->surf->h);
                        if( pic[i]->ratio > 1.0 ) {
                                pic[i]->w = 1.0;
                                pic[i]->h = 1.0/pic[i]->ratio;
                        }
                        else {
                                pic[i]->w = 1.0*pic[i]->ratio;
                                pic[i]->h = 1.0;
                        }
                        i = (i+1) % load_ctx->image_array_size;
                }

                usleep(10000);
        }
        return NULL;
}
