#include "engine.h"

int yandex_engine_init(engine_t * engine,const char * keyword,int size,int filter);
int yandex_engine_destroy(engine_t * engine);
char * yandex_engine_get_url(engine_t * engine);
