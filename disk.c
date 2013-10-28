#include "common.h"
#include "debug.h"
#include "SDL_image.h"
#include <sys/types.h>
#include <dirent.h>
#include <pthread.h>
#include <unistd.h>

typedef struct path_list {
	char * path;
	struct path_list * next;
	int last;
} path_list_t;

path_list_t * path_list = NULL;

int path_num = 0;

static void add_path(char * path)
{
	path_list_t * list = path_list;

	path_num++;

	if(path_list == NULL ) {
		path_list = malloc(sizeof(path_list_t));
		path_list->path = strdup(path);
		path_list->next = NULL;
		path_list->last = 0;
		return;
	}

	while(list->next != NULL) {
		list = list->next;
	}

	list->next = malloc(sizeof(path_list_t));
	list->next->path = strdup(path);
	list->next->next = NULL;
	list->next->last = 0;
}


static SDL_Surface * disk_fetch_image(mem_block_t *context)
{
	static 	path_list_t * list =  NULL;
	SDL_Surface * img = NULL;
//	static int already_read = 0;
	int r;
	int i;


	if(path_list == NULL ) {
		return NULL;
	}

	if(list == NULL) {
		list = path_list;
	}

/*
	if( !already_read) {
		printd(DEBUG_DISK,"got new image : %s\n",list->path);
		img = IMG_Load(list->path);
		if(img == NULL ) {
			printd(DEBUG_DISK,"failed to load : %s\n",list->path);
		}
		already_read = 1;
	}

	if( list->next && already_read) {
		list = list->next;
		printd(DEBUG_DISK,"next image : %s\n",list->path);
		already_read = 0;
	}

	if( list->last ) {
		list = path_list;
	}
*/

	r = random()%path_num;

	list = path_list;
	for(i=0;i<r;i++) {
		if( list->next == NULL ) {
			printd(DEBUG_ERROR,"Path number %d doea not exists\n",r);
			return NULL;
		}
		list = list->next;
	}

	img = IMG_Load(list->path);
	if(img == NULL ) {
		printd(DEBUG_DISK,"failed to load : %s\n",list->path);
	}

	return img;
}

void * scan_disk(void * arg)
{
	char * path = (char *)arg;
	DIR * d;
	struct dirent * e;
	SDL_Surface * img;
	char buf[10000];

	printd(DEBUG_DISK,"scanning path %s\n",path);
	d = opendir(path);
	if(d == NULL) {
		printd(DEBUG_ERROR,"Can not open directory %s\n",path);
		return NULL;
	}

        while( (e = readdir(d)) != 0 ) {
                if(!strcmp(e->d_name,".")) continue;
                if(!strcmp(e->d_name,"..")) continue;
                if(e->d_type == DT_DIR) {
                        sprintf(buf,"%s/%s",path,e->d_name);
                        scan_disk(buf);
                        continue;
                }

		if(e->d_type != DT_REG) continue;
		sprintf(buf,"%s/%s",path,e->d_name);
		img = IMG_Load(buf);
		if(img) {
			SDL_FreeSurface(img);
			printd(DEBUG_DISK,"adding path %s\n",buf);
			add_path(buf);
		}
	}

	closedir(d);
	return NULL;
}

void * disk_load_image(void * arg)
{
        int i = 0;
        mem_block_t  context;
        SDL_Surface * img;
        load_context_t * load_ctx = (load_context_t *)arg;
        pic_t ** pic = load_ctx->image_array;
	pthread_t thread;
	int err;
	void * retval = (void *)1;
	path_list_t * list;

        printd(DEBUG_IMAGE_CACHE,"Entering for %s\n",load_ctx->type);
        memset(&context,0,sizeof(mem_block_t));
        memset(&pic[0],0,load_ctx->image_array_size *sizeof(pic_t *));
        context.keyword = strdup(load_ctx->keyword);
        context.image_request_size = load_ctx->size;
        context.filter = load_ctx->filter;

	pthread_create(&thread,NULL,scan_disk,context.keyword);

        while(1) {
                if(pic[i]==NULL) {
                        img = disk_fetch_image(&context);
                        if(img == NULL ) {
				printd(DEBUG_IMAGE_CACHE,"Cannot fetch a new image\n");
				usleep(10000);
				continue;
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

		if( retval) {
			err = pthread_tryjoin_np(thread,&retval);
			if(err == 0 ) {
				printd(DEBUG_DISK,"scan disk finished\n");
				list = path_list;
				while(list->next != NULL ) {
					list = list->next;
				}
				list->last = 1;
			}
		}
                usleep(10000);
        }
        return NULL;
}
