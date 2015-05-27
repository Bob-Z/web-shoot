#include <stdlib.h>
#include <SDL.h>
#include <SDL_opengl.h>

#define FALSE 0
#define TRUE 1

#define UNDEF_COORD (-99999)

#define SMALL_BUF 1024

typedef struct mem_block {
        char * result_page;
	size_t result_page_size;
	int result_page_num;
        int result_read_index;
        char * image_request_size;
        char * keyword;
        char * filter;
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
        int image_array_size;
} load_context_t;

