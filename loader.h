#include <pthread.h>
#include "image_fifo.h"
#include "engine.h"

enum image_size {
	SIZE_SMALL
	,SIZE_MEDIUM
	,SIZE_LARGE
	,SIZE_NUM
	};

enum filter_activated {
	FILTER_ON
	,FILTER_OFF
	,FILTER_NUM
	};

enum engine_type {
	ENG_TEST
	,ENG_YANDEX
	,ENG_FILE
	,ENG_WIKIMEDIA
	,ENG_DEVIANTART
	,ENG_NUM
	};

typedef struct loader {
	engine_t * engine;
        image_fifo_t * image_fifo;
	pthread_t * thread_array;
} loader_t;

loader_t * loader_init(int engine, int max_img, char * keyword, int size, int filter);
void loader_delete(loader_t * loader);
img_t * loader_get_img(loader_t * loader);
