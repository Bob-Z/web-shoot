#ifndef ENGINE_H
#define ENGINE_H

#include <stdlib.h>
#include <pthread.h>

typedef struct engine {
        char * result_page;
        size_t result_page_size;
        int result_page_num;
        int result_read_index;
        char * image_size;
        char * keyword;
        char * filter;
        pthread_mutex_t page_mutex;
	int (*engine_destroy)(struct engine *);
	char * (*engine_get_url)(struct engine *);
} engine_t;

#endif
