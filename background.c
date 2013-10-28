#include "common.h"
#include "parameter.h"
#include "opengl.h"
#include "network.h"
#include "disk.h"
#include <pthread.h>

static pic_t * bg[MAX_BACKGROUND];
static load_context_t load_ctx_bg;

/* Duration in ms */
/* speed in fss/s */
static double calc_speed()
{
	static Uint32 timer = 0;
        Uint32 current;
        double time_sec;
        double res;

	if(timer==0) {
		timer = SDL_GetTicks();
		return 0;
	}

	current = SDL_GetTicks();
	time_sec = (double)(current-timer)/1000.0;
	timer = current;
        res = BACKGROUND_SPEED * time_sec;
        return res;
}

static void draw_background(int pixel_ref_size, double screen_ratio)
{
        static int first=0;
        static double x_first=UNDEF_COORD;
	double move;
        int current=first;
        int next;
        double x_current=x_first;
        double size;

        if(x_first == UNDEF_COORD ) {
                x_first=1.0;
        }

        if(bg[current]==NULL) {
                return;
        }

        while(x_current < screen_ratio ) {
                printd(DEBUG_SDL,"Copying background %d to screen\n",current);
		/* Calc move in pixel */
                size = 1.0;
                if( bg[current]->ratio > 1.0 ) {
                        size = bg[current]->ratio;
                }
                opengl_blit(pixel_ref_size,x_current,0, bg[current],size,0.0);
                x_current = x_current + bg[current]->ratio;
                next = (current+1)%MAX_BACKGROUND;

                while( bg[next] != NULL ) {
                        if( opengl_init_texture(bg[next]) == 0 ) {
                                break;
                        }
                        next = (next+1)%MAX_BACKGROUND;
                }
                if( bg[next] != NULL ) {
                        current = next;
                }
        }

        if(-x_first > bg[first]->ratio) {
                x_first = x_first + bg[first]->ratio;

                next = (first+1)%MAX_BACKGROUND;
                current = first;

                while( bg[next] != NULL && opengl_init_texture(bg[next]) == -1) {
                        next = (next+1)%MAX_BACKGROUND;
                }

                if( bg[next] != NULL ) {
                        first = next;
                }

                while( first != current ) {
                        printd(DEBUG_IMAGE_CACHE,"Delete background %d\n",current);
                        opengl_delete_texture(bg[current]);
                        bg[current] = NULL;
                        current = (current + 1) % MAX_BACKGROUND;
                }
        }

	move = calc_speed();
        x_first-=move;
}

void background_init(char * keyword_bg,char * filter, void *(*load_routine) (void *))
{
	pthread_t bg_thread;
	int i;

	for(i=0;i<MAX_BACKGROUND;i++) {
		bg[i]=NULL;
	}

        load_ctx_bg.keyword = keyword_bg;
        load_ctx_bg.type = "background";
        load_ctx_bg.size = SIZE_LARGE;
        load_ctx_bg.image_array = bg;
        load_ctx_bg.image_array_size = MAX_BACKGROUND;
        load_ctx_bg.filter = filter;
        pthread_create(&bg_thread,NULL,load_routine,(void *)&load_ctx_bg);
}

void background_draw(int pixel_ref_size, double screen_ratio)
{
	draw_background(pixel_ref_size, screen_ratio);
}
