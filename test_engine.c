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
#include "loader.h"
#include <pthread.h>

/******************************
 engine_destroy
return 0 if no error
******************************/
static int engine_destroy(engine_t * engine)
{
	return 0;
}

/*******************************
  engine_get_url

Return string MUST be freed
 ******************************/
static char * engine_get_url(engine_t * engine)
{
	return strdup("http://www.smashbros.com/images/og/mario.jpg");
}

/******************************
 engine_create
return 0 if no error
******************************/
int test_engine_init(engine_t * engine,const char * keyword,int size,int filter)
{
	engine->engine_destroy=engine_destroy;
        engine->engine_get_url=engine_get_url;

	return 0;
}
