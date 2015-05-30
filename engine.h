#ifndef ENGINE_H
#define ENGINE_H

#include <stdlib.h>
#include <pthread.h>

typedef struct engine {
	void * internal;
	int (*engine_destroy)(struct engine *);
	char * (*engine_get_url)(struct engine *);
} engine_t;

#endif
