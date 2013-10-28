#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdarg.h>
#include "parameter.h"

//int debug = DEBUG_ERROR | DEBUG_HTTP | DEBUG_URL;
//int debug = DEBUG_ERROR | DEBUG_HTTP ;
int debug = DEBUG_LEVEL;

/* printd */
int print_debug(const char * function, int filter,const char *fmt, ...)
{
	va_list ap;
	int ret;

	if(debug & filter) {
		printf("%d|%21s: ",filter,function);
		va_start(ap, fmt);
		ret = vprintf(fmt, ap);
		va_end(ap);

	}

	return ret;
}
