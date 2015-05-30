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
#include "loader.h"
#include "misc.h"
#include "yandex.h"
#include "test_engine.h"
#include "file_engine.h"
#include "wikimedia_engine.h"
#include <pthread.h>

/*******************************
 ******************************/
/* return NULL on error
   return a SDL_Surface on success
 */
static SDL_Surface * fetch_image(engine_t *engine)
{
        SDL_Surface * image = NULL;
        char * url;
        int err;
        char filename[SMALL_BUF];
        char * tmp_dir;

        while( image == NULL ) {
                url =  engine->engine_get_url(engine);
                if( url == NULL ) {
                        return NULL;
                }

                /* Download the resource */
                err = web_to_disk(url);
                if( err == -1) {
                        printd(DEBUG_ERROR,"Error fetching %s\n", url);
                        free(url);
                        continue;
                }

                /* Read image */
                tmp_dir = get_tmp_dir();
                sprintf(filename,"%s/%s.%d",tmp_dir,TMP_FILE,(int)pthread_self());
                free(tmp_dir);
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

/*******************************
 ******************************/
static void * thread_routine(void * arg)
{
	SDL_Surface * surf = NULL;
	loader_t * loader = (loader_t *)arg;
	engine_t * engine = loader->engine;
	image_fifo_t * image_fifo = loader->image_fifo;
	img_t *img;

	while(1) {
		surf = fetch_image(engine);
		if(surf == NULL ) {
			continue;
		}

		img=malloc(sizeof(img_t));
                memset(img,0,sizeof(img_t));
                img->surf = surf;
                img->ratio = (double)(img->surf->w) / (double)(img->surf->h);
                if( img->ratio > 1.0 ) {
			img->w = 1.0;
                        img->h = 1.0/img->ratio;
                }
                else {
                        img->w = 1.0*img->ratio;
                        img->h = 1.0;
                }

		image_fifo_push( image_fifo, img );
	}

	return NULL;
}

/*******************************
 ******************************/
loader_t * loader_init(int engine, int max_img, char * keyword, int size, int filter)
{
	loader_t * loader = NULL;
	int i;

	loader = malloc(sizeof(loader_t));
	memset(loader,0,sizeof(loader_t));

	loader->engine = malloc(sizeof(engine_t));
	memset(loader->engine,0,sizeof(engine_t));

	loader->image_fifo = image_fifo_init(max_img);

	switch( engine ) {
		case ENG_TEST:
			test_engine_init( loader->engine, keyword, size, filter);
			break;

		case ENG_YANDEX:
			yandex_engine_init( loader->engine, keyword, size, filter);
			break;

		case ENG_FILE:
			file_engine_init( loader->engine, keyword, size, filter);
			break;

		case ENG_WIKIMEDIA:
			wikimedia_engine_init( loader->engine, keyword, size, filter);
			break;

		default:
                        printd(DEBUG_ERROR,"Engine %d does not exist\n",engine);
			return NULL;
	}

        loader->thread_array = malloc( NUM_THREAD * sizeof(pthread_t) );

        for(i=0;i<NUM_THREAD;i++) {
                pthread_create(&loader->thread_array[i],NULL,thread_routine,loader);
        }

	return loader;
}
/*******************************
 ******************************/
void loader_delete(loader_t * loader)
{
	int i;

	for(i=0;i<NUM_THREAD;i++) {
                pthread_cancel(loader->thread_array[i]);
        }

	for(i=0;i<NUM_THREAD;i++) {
                pthread_join(loader->thread_array[i],NULL);
        }

	loader->engine->engine_destroy(loader->engine);

	image_fifo_delete(loader->image_fifo);

	free(loader);

}

/*******************************
 ******************************/
img_t * loader_get_img(loader_t * loader)
{
	return image_fifo_pop(loader->image_fifo);
}
