#include <SDL.h>
#include <SDL_image.h>

#define SIZE_ANY "a"
#define SIZE_LARGE "l"
#define SIZE_MEDIUM "m"
#define SIZE_ICON "i"

#define FILTER_OFF "&safe=off"
#define FILTER_ON "&safe=active"

void * network_load_image(void * arg);
