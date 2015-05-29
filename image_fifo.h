#include <stdlib.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <pthread.h>
#include <semaphore.h>
#include "image.h"

typedef struct image_fifo {
        img_t ** image_array;
        int image_array_size;
	int read_index;
	pthread_mutex_t read_mutex;
	int write_index;
	pthread_mutex_t write_mutex;
	sem_t available_entry;
} image_fifo_t;

image_fifo_t * image_fifo_init(int max_img);
void image_fifo_push(image_fifo_t * fifo, img_t * img);
img_t * image_fifo_pop(image_fifo_t * fifo);
void image_fifo_delete(image_fifo_t * fifo);
