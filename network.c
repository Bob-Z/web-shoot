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
#include "engine.h"
#include "misc.h"
#include <pthread.h>

/*******************************
  data_to_mem callback
 ******************************/
static size_t data_to_mem(void *buffer, size_t size, size_t nmemb, void *userp)
{
	network_page_t * page = (network_page_t*) userp;

	page->data = realloc(page->data,page->size + (size*nmemb) + 1 ); /*for terminal NULL */
	if ( page->data == NULL ) {
		printd(DEBUG_HTTP,"Cannot realloc\n");
		return 0;
	}
	memcpy(page->data+page->size, buffer, size*nmemb);
	page->size += (size*nmemb);
	page->data[page->size] = 0; /*terminal NULL */

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

/*******************************
  async_perform callback
 ******************************/
static void * async_perform(void * arg)
{
	int err;

	err = curl_easy_perform((CURL *)arg);

	if( err != CURLE_OK ) {
		return (void*)-1;
	}
	return NULL;
}

/*******************************
  web_to_memory

return -1 on error
return 0 on success
 ******************************/
int web_to_memory( char * url, network_page_t * page)
{
	CURL * easyhandle;
	char * proxy;
	int err;
	pthread_t thread;
	void * thread_ret;
	struct timespec t;
	char curl_error_buffer[CURL_ERROR_SIZE];

	easyhandle = curl_easy_init();
	if(easyhandle == NULL) {
		printd(DEBUG_ERROR,"curl_easy_init failed: %s",curl_error_buffer);
		return -1;
	}

	proxy = getenv("http_proxy");
	if(proxy) {
		printd(DEBUG_HTTP,"Set proxy to %s\n",proxy);
		curl_easy_setopt(easyhandle, CURLOPT_PROXY, proxy);
	}

	if(page->data) {
		free(page->data);
		page->data=NULL;
		page->size=0;
	}

	curl_easy_setopt(easyhandle, CURLOPT_URL, url);
	curl_easy_setopt(easyhandle, CURLOPT_WRITEFUNCTION, data_to_mem);
	curl_easy_setopt(easyhandle, CURLOPT_WRITEDATA, (void *)page);
	curl_easy_setopt(easyhandle, CURLOPT_TIMEOUT, DEF_HTTP_TIMEOUT);
	curl_easy_setopt(easyhandle, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(easyhandle, CURLOPT_ERRORBUFFER, curl_error_buffer);
	curl_easy_setopt(easyhandle, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(easyhandle, CURLOPT_USERAGENT, "web-shooter/1.0");
	curl_easy_setopt(easyhandle, CURLOPT_VERBOSE, 1L);

	/* experimental */
	//curl_easy_setopt(easyhandle, CURLOPT_AUTOREFERER, 1);
	//curl_easy_setopt(easyhandle, CURLOPT_TRANSFER_ENCODING, 1);

	pthread_create(&thread,NULL,async_perform,easyhandle);

	clock_gettime(CLOCK_REALTIME, &t);
	t.tv_sec += DEF_HTTP_TIMEOUT + 2;
	err = pthread_timedjoin_np(thread,&thread_ret,&t);
	if(err) {
		pthread_cancel(thread);
		printd(DEBUG_ERROR,"pthread_timedjoin_np failed: %s\n",strerror(err));
		curl_easy_cleanup(easyhandle);
		return -1;
	}
	if(thread_ret != NULL) {
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
int web_to_disk( char * url)
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
	char * tmp_dir;

	tmp_dir = get_tmp_dir();
	sprintf(filename,"%s/%s.%d",tmp_dir,TMP_FILE,(int)pthread_self());
	free(tmp_dir);

	fd = open(filename,O_CREAT| O_TRUNC | O_RDWR, S_IRWXU);
	if( fd == -1 ) {
		printd(DEBUG_ERROR,"Can't open %s\n", filename);
		return -1;
	}

	easyhandle = curl_easy_init();
	if(easyhandle == NULL) {
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
	curl_easy_setopt(easyhandle, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(easyhandle, CURLOPT_USERAGENT, "web-shooter/1.0");

	pthread_create(&thread,NULL,async_perform,easyhandle);

	clock_gettime(CLOCK_REALTIME, &t);
	t.tv_sec += DEF_HTTP_TIMEOUT + 2;
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
	printd(DEBUG_ERROR,"%s ==> %s OK\n",url,filename);

	return 0;
}
