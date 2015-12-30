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

#include <SDL.h>
#include <unistd.h>
#include <getopt.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "parameter.h"
#include "debug.h"
#include "common.h"
#include "network.h"
#include "opengl.h"
#include "sprite.h"
#include "background.h"
#include "loader.h"

const SDL_VideoInfo* video_info;

int window_w = WINDOW_SIZE_X;
int window_h = WINDOW_SIZE_Y;
int windowed_w = WINDOW_SIZE_X;
int windowed_h = WINDOW_SIZE_Y;

double screen_ratio = 1.0;
int fss;

int fullscreen = 0;

const char optstring[] = "?nfsb:c:p:B:e:";
const struct option longopts[] = {
	{ "nofilter",no_argument,NULL,'n' },
	{ "fullscreen",no_argument,NULL,'f' },
	{ "slideshow",no_argument,NULL,'s' },
	{ "background",required_argument,NULL,'b' },
	{ "cpu",required_argument,NULL,'c' },
	{ "player",required_argument,NULL,'p' },
	{ "backup",required_argument,NULL,'B' },
	{ "engine",required_argument,NULL,'e' },
	{NULL,0,NULL,0}
};

int main(int argc, char **argv)
{

	SDL_Event event;
	int bFin = 0;
	char * keyword_bg = NULL;
	char * keyword_sp = NULL;
	char * keyword_pl = NULL;
	char * engine = NULL;
	Uint8 *keystate;
	int fullscreen_w = 0;
	int fullscreen_h = 0;
	int filter = FILTER_ON;
	int opt_ret;
	int slideshow = 0;

	backup_dir = NULL;

	srand(time(NULL));

	while((opt_ret = getopt_long(argc, argv, optstring, longopts, NULL))!=-1) {
		switch(opt_ret) {
		case 'f':
			fullscreen = SDL_FULLSCREEN;
			break;
		case 'n':
			filter = FILTER_OFF;
			break;
		case 's':
			slideshow = 1;
			break;
		case 'b':
			keyword_bg = strdup(optarg);
			break;
		case 'c':
			keyword_sp = strdup(optarg);
			break;
		case 'p':
			keyword_pl = strdup(optarg);
			break;
		case 'B':
			backup_dir = strdup(optarg);
			break;
		case 'e':
			engine = strdup(optarg);
			break;
		default:
			printf("HELP:\n\n");
			printf("-f : start fullscreen\n");
			printf("-n : no filter on web request\n");
			printf("-s : slideshow mode\n");
			printf("-d : load image from disk\n");
			printf("-b : background theme (or path)\n");
			printf("\nIn game mode : \n");
			printf("-c : cpu controlled sprites theme (or path)\n");
			printf("-p : player sprites theme (or path)\n");
			printf("-B : backup path\n");
			printf("-e : select engine: 0- DeviantArt, 1- file, 2- Framabee, 3- Qwant, 4- Wikimedia, 5- Yandex\n");
			exit(0);
		}
	}

	fss = window_h;

	/* initialisation de SDL_Video */
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
		printd(DEBUG_ERROR, "Echec d'initialisation de SDL.\n");
		return 1;
	}

	if(!slideshow) {
		if(keyword_sp == NULL ) {
			keyword_sp = readline("cpu: ");
			if(keyword_sp[0] == 0 ) {
				free(keyword_sp);
				keyword_sp = "nothing";
			}
		}
		if(keyword_pl == NULL ) {
			keyword_pl = readline("player: ");
			if(keyword_pl[0] == 0 ) {
				free(keyword_pl);
				keyword_pl = "nothing";
			}
		}
	}

	video_info = SDL_GetVideoInfo();
	fullscreen_w = video_info->current_w;
	fullscreen_h = video_info->current_h;

	if(fullscreen) {
		window_w=fullscreen_w;
		window_h=fullscreen_h;
		fss = window_h;
	}

	SDL_SetVideoMode(window_w, window_h, 32, SDL_OPENGL | SDL_GL_DOUBLEBUFFER | SDL_RESIZABLE | fullscreen);

	screen_ratio = opengl_init(window_w,window_h);

	if( background_init(engine,keyword_bg,filter) == RET_FAIL) {
		return 1;
	}
#if 0
	if(!slideshow) {
		sprite_init(keyword_pl,keyword_sp,filter);
	}
#endif

	while (!bFin) {
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_VIDEORESIZE:
				window_w = event.resize.w;
				window_h = event.resize.h;
				windowed_w = window_w;
				windowed_h = window_h;
				SDL_SetVideoMode(window_w, window_h, 32, SDL_OPENGL | SDL_GL_DOUBLEBUFFER | SDL_RESIZABLE | fullscreen);
				fss = window_h;
				screen_ratio = opengl_init(window_w,window_h);
				break;
			case SDL_KEYDOWN:
				switch (event.key.keysym.sym) {
				case SDLK_ESCAPE:
					bFin = 1;
					break;
#if 0
				case SDLK_UP:
					sprite_control_up(1);
					break;
				case SDLK_RIGHT:
					sprite_control_right(1);
					break;
				case SDLK_DOWN:
					sprite_control_down(1);
					break;
				case SDLK_LEFT:
					sprite_control_left(1);
					break;
				case SDLK_SPACE:
					sprite_control_shoot();
					break;
#endif
				case SDLK_RETURN:
					keystate = SDL_GetKeyState(NULL);

					if( keystate[SDLK_RALT] || keystate[SDLK_LALT] ) {
						if(!fullscreen) {
							fullscreen = SDL_FULLSCREEN;
							window_w=fullscreen_w;
							window_h=fullscreen_h;
						} else {
							fullscreen = 0;
							window_w=windowed_w;
							window_h=windowed_h;
						}
						SDL_SetVideoMode(window_w, window_h, 32, SDL_OPENGL | SDL_GL_DOUBLEBUFFER | SDL_RESIZABLE | fullscreen);
						fss = window_h;
						screen_ratio = opengl_init(window_w,window_h);
						break;
					}
#if 0
					sprite_control_restart();
#endif
					break;
				default:
					printd(DEBUG_SDL,"Une touche à été pressée.\n");
				}
				break;

			case SDL_KEYUP:
				switch (event.key.keysym.sym) {
#if 0
				case SDLK_UP:
					sprite_control_up(0);
					break;
				case SDLK_RIGHT:
					sprite_control_right(0);
					break;
				case SDLK_DOWN:
					sprite_control_down(0);
					break;
				case SDLK_LEFT:
					sprite_control_left(0);
					break;
#endif
				default:
					;
				}
				break;

			case SDL_QUIT:
				bFin = 1;
				break;
			default:
				;
			}
		}

		opengl_clear_screen();
		background_draw(fss,screen_ratio);
#if 0
		sprite_draw(fss,screen_ratio);
#endif
		SDL_GL_SwapBuffers();
	}

	SDL_Quit();

	return 0;
}
