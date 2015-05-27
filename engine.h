#include <SDL.h>
#include <SDL_image.h>

int engine_init(mem_block_t * context,const char * keyword,const char * size,const char * filter);
int engine_destroy(mem_block_t * context);
char * engine_get_url(mem_block_t * context);
