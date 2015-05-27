#include <SDL.h>
#include <SDL_image.h>

#define SIZE_ANY "medium"
#define SIZE_LARGE "large"
#define SIZE_MEDIUM "medium"
#define SIZE_ICON "small"

#define FILTER_OFF "&ncrnd=9763"
#define FILTER_ON "&ncrnd=5290"

int web_to_memory( char * url, mem_block_t * context);
void * network_load_image(void * arg);
