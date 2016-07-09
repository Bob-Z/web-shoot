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
#include <dirent.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>

typedef struct internal {
	char * path;
	char ** file;
	int num_file;
	int index;
	pthread_mutex_t engine_mutex;
} internal_t;

/******************************
 scan path
return 0 if no error
******************************/
int scan_disk(internal_t * internal,char * path)
{
	DIR * d;
	struct dirent * e;
	char buf[LARGE_BUF];

	d = opendir(path);
	if(d == NULL) {
		printd(DEBUG_ERROR,"Can not open directory %s\n",path);
		return -1;
	}

	while( (e = readdir(d)) != 0 ) {
		if(!strcmp(e->d_name,".")) {
			continue;
		}
		if(!strcmp(e->d_name,"..")) {
			continue;
		}
		if(e->d_type == DT_DIR) {
			sprintf(buf,"%s/%s",path,e->d_name);
			scan_disk(internal,buf);
			continue;
		}

		if(e->d_type != DT_REG) {
			continue;
		}
		sprintf(buf,"%s/%s",path,e->d_name);
		internal->num_file++;
		internal->file = realloc(internal->file,internal->num_file*sizeof(char *));
		internal->file[internal->num_file-1] = strdup(buf);
	}

	closedir(d);
	return 0;
}

/******************************
 engine_destroy
return 0 if no error
******************************/
static int engine_destroy(engine_t * engine)
{
	internal_t * internal = (internal_t*)engine->internal;
	int i;

	if(internal) {
		if(internal->path) {
			free(internal->path);
		}

		if(internal->file) {
			for(i=0; i<internal->num_file; i++) {
				free(internal->file[i]);
			}
			free(internal->file);
		}

		pthread_mutex_destroy(&internal->engine_mutex);

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
	internal_t * internal = (internal_t*)engine->internal;
	char * url;
	char buf[LARGE_BUF];

	pthread_mutex_lock(&internal->engine_mutex);

	if( internal->file == NULL ) {
		if( scan_disk(internal,internal->path) == -1 ) {
			pthread_mutex_unlock(&internal->engine_mutex);
			return NULL;
		}
	}

	url = internal->file[internal->index];
	internal->index = random() % internal->num_file;

	pthread_mutex_unlock(&internal->engine_mutex);

	snprintf(buf,LARGE_BUF,"file://%s",url);

	return strdup(buf);
}

/******************************
 engine_create
return 0 if no error
******************************/
int file_engine_init(engine_t * engine,const char * keyword,int size,int filter)
{
	internal_t * internal;

	printf("File engine\n");

	internal = malloc(sizeof(internal_t));
	memset(internal,0,sizeof(internal_t));
	engine->internal = internal;

	if(keyword == NULL) {
		internal->path = readline("Enter path: ");
		if(internal->path[0] == 0 ) {
			free(internal->path);
			internal->path = getenv("HOME");
		}
	} else {
		internal->path = strdup(keyword);
	}

	srandom(time(NULL));

	pthread_mutex_init(&internal->engine_mutex,NULL);

	engine->engine_destroy=engine_destroy;
	engine->engine_get_url=engine_get_url;

	return 0;
}
