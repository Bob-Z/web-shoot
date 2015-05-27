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
#include <pthread.h>

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
int web_to_memory( char * url, mem_block_t * context)
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

        curl_easy_cleanup(easyhandle);

	close(fd);

	return 0;
}

/* return NULL on error
   return a SDL_Surface on success
 */
static SDL_Surface * fetch_image(mem_block_t *context)
{
	SDL_Surface * image = NULL;
	char * url;
	int err;
	static int index = 0;
	char filename[SMALL_BUF];

	while( image == NULL ) {
		url =  engine_get_url(context);
		if( url == NULL ) {
			return NULL;
		}

		/* Is this an image ? */
/*
		if(strcasecmp(".jpg",url+strlen(url)-4) != 0 &&
				strcasecmp(".gif",url+strlen(url)-4) != 0 &&
				strcasecmp(".jpeg",url+strlen(url)-5) != 0 &&
				strcasecmp(".png",url+strlen(url)-4) != 0) {
			free(url);
			continue;
		}
*/

		/* Download the resource */
		err = web_to_disk(url,context,index);
		if( err == -1) {
			printd(DEBUG_ERROR,"Error fetching %s\n", url);
			free(url);
			continue;
		}

		/* Read image */
		sprintf(filename,"%s-%s.%d",TMP_FILE,context->keyword,index);
		image=IMG_Load(filename);
		if(image == NULL ) {
			printd(DEBUG_HTTP,"%s is a NOT an image\n",filename);
			free(url);
			continue;
		}

		/* Check OpenGL compatibility */
		if( image->format->BytesPerPixel != 4 && image->format->BytesPerPixel != 3 ) {
			printd(DEBUG_HTTP,"%s cannot be displayed \n",filename);
			SDL_FreeSurface(image);

			free(url);
			continue;
		}

		free(url);
	}

        return image;
}

void * network_load_image(void * arg)
{
        int i = 0;
        mem_block_t  context;
        SDL_Surface * img = NULL;
        load_context_t * load_ctx = (load_context_t *)arg;
        pic_t ** pic = load_ctx->image_array;

        printd(DEBUG_IMAGE_CACHE,"Entering for %s\n",load_ctx->type);
        memset(&pic[0],0,load_ctx->image_array_size *sizeof(pic_t *));

	engine_init(&context,load_ctx->keyword,load_ctx->size,load_ctx->filter);

        while(1) {
                if(pic[i]==NULL) {
                        img = fetch_image(&context);
                        if(img == NULL ) {
                                printd(DEBUG_ERROR,"No more pics for %s request\n",load_ctx->type);
				engine_destroy(&context);
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
