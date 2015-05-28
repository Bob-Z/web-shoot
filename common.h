#include <stdlib.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <pthread.h>
#include <semaphore.h>

#define FALSE 0
#define TRUE 1

#define UNDEF_COORD (-99999)

#define SMALL_BUF 1024

#define NUM_THREAD (8)

typedef struct mem_block {
        char * result_page;
	size_t result_page_size;
	int result_page_num;
        int result_read_index;
        char * image_request_size;
        char * keyword;
        char * filter;
	pthread_mutex_t page_mutex;
} mem_block_t;

typedef struct pic {
        SDL_Surface * surf;
        GLuint  tex;
        double ratio;
        double w;
        double h;
        int init;
} pic_t;

typedef struct load_context {
        char * keyword;
        char * type;
        char * size;
        char * filter;
        pic_t ** image_array;
	int current_image_index;
	pthread_mutex_t image_index_mutex;
        int image_array_size;
	mem_block_t mem_block;
	sem_t array_sem;
} load_context_t;

