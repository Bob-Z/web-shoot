#include "common.h"
#include "parameter.h"
#include "opengl.h"
#include "network.h"
#include "disk.h"
#include <pthread.h>
#include "loader.h"

#define MAX_IMG (8)

static loader_t * loader;
static img_t * bg[MAX_IMG];

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
        double x_current=x_first;
        double size;
	int i;

        if(x_first == UNDEF_COORD ) {
                x_first=1.0;
        }

	/* Fill array */
	for(i=0;i<MAX_IMG;i++) {
		if( bg[i] == NULL ) {
			break;
		}
	}

	if( i < MAX_IMG ) {
		bg[i] = loader_get_img(loader);
	}

	current = 0;

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

		current++;
		if(bg[current] == NULL) {
			current = 0;
		}
        }

        if(-x_first > bg[0]->ratio) {
                x_first = x_first + bg[0]->ratio;
		opengl_delete_texture(bg[0]);

		for( i=0;i<MAX_IMG-1;i++) {
			bg[i] = bg[i+1];
		}
		bg[i] = NULL;
        }

	move = calc_speed();
        x_first-=move;
}

void background_init(char * keyword,int filter)
{
	int i;

	for(i=0;i<MAX_IMG;i++) {
		bg[i] = NULL;
	}
//	loader = loader_init(ENG_YANDEX,8,keyword,SIZE_LARGE,filter);
//	loader = loader_init(ENG_TEST,8,keyword,SIZE_LARGE,filter);
	loader = loader_init(ENG_FILE,8,keyword,SIZE_LARGE,filter);
}

void background_draw(int pixel_ref_size, double screen_ratio)
{
	draw_background(pixel_ref_size, screen_ratio);
}
