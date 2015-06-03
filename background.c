/*
   Web-shooter is a shoot them up game with random graphics.
   Copyright (C) 2013-2015 carabobz@gmail.com

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
*/

#include "common.h"
#include "parameter.h"
#include "opengl.h"
#include "network.h"
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

		current = (current+1) % MAX_IMG;
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
//	loader = loader_init(ENG_FILE,8,keyword,SIZE_LARGE,filter);
//	loader = loader_init(ENG_WIKIMEDIA,8,keyword,SIZE_LARGE,filter);
	loader = loader_init(ENG_DEVIANTART,8,keyword,SIZE_LARGE,filter);
}

void background_draw(int pixel_ref_size, double screen_ratio)
{
	draw_background(pixel_ref_size, screen_ratio);
}
