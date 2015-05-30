#include "engine.h"

typedef struct network_page {
	char * data;
	size_t size;
} network_page_t;

int web_to_memory( char * url, network_page_t * page);
int web_to_disk( char * url);
