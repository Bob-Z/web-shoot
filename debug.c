#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdarg.h>
#include "parameter.h"
#include <time.h>
#include "SDL.h"

int debug = DEBUG_ERROR | DEBUG_HTTP | DEBUG_URL;
//int debug = DEBUG_ERROR | DEBUG_HTTP ;
//int debug = DEBUG_LEVEL;

/* printd */
int print_debug(const char * file, int line, const char * function, int filter,const char *fmt, ...)
{
	va_list ap;
	int ret;

	if(debug & filter) {
		printf("%7d:%12s(%3d)|%2d|%21s|",SDL_GetTicks(),file,line,filter,function);
		va_start(ap, fmt);
		ret = vprintf(fmt, ap);
		va_end(ap);

	}

	return ret;
}
