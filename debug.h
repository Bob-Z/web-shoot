#define DEBUG_ERROR 1
#define DEBUG_URL 1<<1
#define DEBUG_SDL 1<<2
#define DEBUG_HTTP 1<<3
#define DEBUG_IMAGE_CACHE 1<<4
#define DEBUG_PAGE 1<<5
#define DEBUG_DISK 1<<6

#define printd(filter,fmt, ...) print_debug(__FILE__,__LINE__,__func__,filter,fmt, ##__VA_ARGS__)
int print_debug(const char *file, int line, const char * function, int filter,const char *fmt, ...);
