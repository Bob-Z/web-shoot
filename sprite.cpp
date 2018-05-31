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
#include "network.h"
#include "opengl.h"
#include <pthread.h>
#include <unistd.h>
#include <math.h>

typedef struct {
	int img_num;
	double x;
	double y;
	double x_orig;
	double y_orig;
	double speed;
	Uint32 time_start;
} sprite_t;

sprite_t sp_array[SPRITE_NUM_MAX];
typedef struct shot_struct {
	struct shot_struct * next;
	sprite_t * shot;
} shot_t;

shot_t * shot_list = NULL;

pic_t *sp[MAX_SPRITE];
pic_t *pl[MAX_PLAYER];

double player_x;
double player_y;
int player_alive = 1;
int player_dying = 0;
int player_dead = 0;
int player_img_num = 0;

#define UP 0
#define DOWN 1
#define LEFT 2
#define RIGHT 3
int key_state[4] = { 0,0,0,0 };

load_context_t load_ctx_sp;
load_context_t load_ctx_pl;

void sprite_init(char * keyword_pl, char * keyword_sp,char * filter)
{
	int i;
	pthread_t sp_thread;
	pthread_t pl_thread;

	player_x = UNDEF_COORD;
	player_y = UNDEF_COORD;

	for(i=0; i<SPRITE_NUM_MAX; i++) {
		sp_array[i].img_num = -1;
	}

	for(i=0; i<MAX_SPRITE; i++) {
		sp[i]=NULL;
	}
	for(i=0; i<MAX_PLAYER; i++) {
		pl[i]=NULL;
	}

	load_ctx_pl.keyword = keyword_pl;
	load_ctx_pl.type = "player";
	load_ctx_pl.size = SIZE_ICON;
	load_ctx_pl.image_array = pl;
	load_ctx_pl.image_array_size = MAX_PLAYER;
	load_ctx_pl.filter = filter;
	load_ctx_pl.current_image_index = 0;
	pthread_mutex_init(&load_ctx_pl.image_index_mutex,NULL);
	sem_init(&load_ctx_pl.array_sem,0,load_ctx_pl.image_array_size);

	pthread_create(&pl_thread,NULL,network_load_image,(void *)&load_ctx_pl);
	while( pl[1] == NULL ) {
		usleep(1000);
	}

	load_ctx_sp.keyword = keyword_sp;
	load_ctx_sp.type = "sprite";
	load_ctx_sp.size = SIZE_ICON;
	load_ctx_sp.image_array = sp;
	load_ctx_sp.image_array_size = MAX_SPRITE;
	load_ctx_sp.filter = filter;
	load_ctx_sp.current_image_index = 0;
	pthread_mutex_init(&load_ctx_sp.image_index_mutex,NULL);
	sem_init(&load_ctx_sp.array_sem,0,load_ctx_sp.image_array_size);

	pthread_create(&sp_thread,NULL,network_load_image,(void *)&load_ctx_sp);
}
static void add_shot(double x, double y)
{
	shot_t * current;

	if( shot_list == NULL ) {
		shot_list = malloc(sizeof(shot_t));
		current = shot_list;
	} else {
		current = shot_list;
		while( current->next != NULL ) {
			current = current->next;
		}

		current->next = malloc(sizeof(shot_t));
		current = current->next;
	}

	current->shot = malloc(sizeof(sprite_t));
	;
	current->shot->x = x;
	current->shot->y = y;
	current->shot->x_orig = x;
	current->shot->y_orig = y;
	current->shot->time_start = SDL_GetTicks();
	current->shot->img_num = player_img_num +1 ;
	current->next = NULL;
}

static shot_t * remove_shot(shot_t * shot)
{
	shot_t * current;
	shot_t * next;

	if( shot_list == shot ) {
		shot_list = shot->next;
		free(shot->shot);
		free(shot);
		return shot_list;
	} else {
		current = shot_list;

		while(current->next != shot) {
			if( current->next != NULL ) {
				current = current->next;
			} else {
				return NULL;
			}
		}

		next = current->next->next;
		free(current->next->shot);
		free(current->next);
		current->next = next;
		return current->next;
	}
}

static void draw_shot(int pixel_ref_size,double screen_ratio)
{
	shot_t * current;
	Uint32 now;
	double time_elapsed;
	double angle;
	double size;
	double x;
	double y;

	if(shot_list == NULL ) {
		return;
	}

	current = shot_list;

	do {
		now = SDL_GetTicks();
		time_elapsed =  ((double)(now-current->shot->time_start)/1000.0);

		angle = time_elapsed/SHOT_ROTATION_SPEED*360.0;
		size = SHOT_SIZE;

		y = current->shot->y_orig;
		x = current->shot->x_orig + (time_elapsed * SHOT_SPEED);
		current->shot->x = x;
		current->shot->y = y;

		opengl_blit(pixel_ref_size,x,y,pl[current->shot->img_num],size,angle);

		if( current->shot->x > screen_ratio ) {
			current = remove_shot(current);
		} else {
			current = current->next;
		}
	} while( current != NULL );
}

static void draw_sprite(int pixel_ref_size,double screen_ratio)
{
	static int first=0;
	int i;
	int j;
	int current=first;
	int next;
	int img_num;
	double x;
	double y;

	if(sp[first]==NULL) {
		return;
	}

	for( i=0; i<SPRITE_NUM_MAX; i++) {
		/* Init if needed */
		if( sp_array[i].img_num == -1 ) {
			if(!player_alive) {
				continue;
			}
			sp_array[i].x = screen_ratio;
			sp_array[i].x_orig = sp_array[i].x;
			sp_array[i].y = (double)(random()%10000)/10000.0;
			sp_array[i].y_orig = sp_array[i].y;
			sp_array[i].speed = (double)(((random()%8)+2))/10.0;
			sp_array[i].img_num = current;
			sp_array[i].time_start = SDL_GetTicks();

			next = (current+1)%MAX_SPRITE;
			while( sp[next] != NULL ) {
				if( opengl_init_texture(sp[next]) == 0 ) {
					first =next;
					break;
				}
				next = (next+1)%MAX_SPRITE;
			}
			if( sp[next] != NULL ) {
				current = next;
			}
		}

		/* Draw */
		printd(DEBUG_SDL,"Copying sprite %d to screen\n",sp_array[i].img_num);
//              dest.y = sp_array[i].y;
		y = sp_array[i].y_orig + (SPRITE_SIZE / 2.0 * sin( (double)(SDL_GetTicks()-sp_array[i].time_start)/1000.0 ));;
		x = sp_array[i].x = sp_array[i].x_orig - (sp_array[i].speed * (double)((SDL_GetTicks()-sp_array[i].time_start) / 1000.0));

		opengl_blit(pixel_ref_size,x,y,sp[sp_array[i].img_num],SPRITE_SIZE,0.0);

		/* Delete if out of screen */
		if( sp_array[i].x < -sp[sp_array[i].img_num]->w ) {
			img_num = sp_array[i].img_num;
			sp_array[i].img_num = -1;

			while( img_num != first ) {
				/* is this sprite still used */
				for(j=0; j<SPRITE_NUM_MAX; j++) {
					if(sp_array[j].img_num==img_num) {
						break;
					}
				}
				/* Not used anymore, delete */
				if(j == SPRITE_NUM_MAX && sp[img_num] != NULL) {
					printd(DEBUG_IMAGE_CACHE,"Delete sprite %d\n",img_num);
					opengl_delete_texture(sp[img_num]);
					sp[img_num] = NULL;
					sem_post(&load_ctx_sp.array_sem);
				}
				img_num = (img_num +1 ) %MAX_SPRITE;
			}
		}
	}
}

static void draw_player(int pixel_ref_size,double screen_ratio)
{
	static double key_count[4] = { 0,0,0,0 };
	static Uint32 key_time[4] = { 0,0,0,0 };
	static Uint32 player_dead_time;
	Uint32 now;
	double time_elapsed;
	double angle;
	double x;
	double y;
	double size;

	if(player_alive) {
		player_dead = 0;
		player_dying = 0;
	}

	if(player_dead) {
		return;
	}

	if( !player_alive && !player_dying) {
		player_dying = 1;
		player_dead_time = SDL_GetTicks();
		return;
	}

	if(pl[player_img_num]==NULL) {
		return;
	}

	if(pl[player_img_num+1]==NULL) {
		return;
	}

	if (player_x == UNDEF_COORD ) {
		player_x = 0.0 ;
	}

	if (player_y == UNDEF_COORD ) {
		player_y = 0.5-(pl[player_img_num]->h*PLAYER_SIZE/2.0) ;
	}

	if( player_dying ) {
		now = SDL_GetTicks();
		time_elapsed =  ((double)(now-player_dead_time)/1000.0);
		if( time_elapsed > PLAYER_DEAD_TIME ) {
			player_dead = 1;
			return;
		}

		angle = time_elapsed/PLAYER_DEAD_TIME*360.0;
		size = PLAYER_SIZE * (PLAYER_DEAD_TIME-time_elapsed/PLAYER_DEAD_TIME);

		y = player_y + ((size/2.0)*time_elapsed/PLAYER_DEAD_TIME);
		x = player_x + ((size/2.0)*time_elapsed/PLAYER_DEAD_TIME);
		opengl_blit(pixel_ref_size,x,y,pl[player_img_num],size,angle);

		return;
	}

	if( key_state[UP] == 0 ) {
		key_count[UP] = 0;
		key_time[UP] = 0;
	}
	if( key_state[DOWN] == 0 ) {
		key_count[DOWN] = 0;
		key_time[DOWN] = 0;
	}
	if( key_state[LEFT] == 0 ) {
		key_count[LEFT] = 0;
		key_time[LEFT] = 0;
	}
	if( key_state[RIGHT] == 0 ) {
		key_count[RIGHT] = 0;
		key_time[RIGHT] = 0;
	}

	if( key_state[UP] == 1 ) {
		if( player_y > 0 ) {
			if( key_time[UP] == 0 ) {
				key_time[UP] = SDL_GetTicks();
			} else {
				key_count[UP] += PLAYER_SPEED*((double)(SDL_GetTicks()-key_time[UP]))/1000.0;
				key_time[UP] = SDL_GetTicks();
				player_y -= key_count[UP];
				if(player_y<0.0) {
					player_y = 0.0;
				}
			}
		}
	}
	if( key_state[DOWN] == 1 ) {
		if( player_y < (1.0-(PLAYER_SIZE*pl[player_img_num]->h))) {
			if( key_time[DOWN] == 0 ) {
				key_time[DOWN] = SDL_GetTicks();
			} else {
				key_count[DOWN] += PLAYER_SPEED*((double)(SDL_GetTicks()-key_time[DOWN]))/1000.0;
				key_time[DOWN] = SDL_GetTicks();
				player_y += key_count[DOWN];
				if(player_y>1.0-(PLAYER_SIZE*pl[player_img_num]->h)) {
					player_y =1.0-(PLAYER_SIZE*pl[player_img_num]->h);
				}
			}
		}
	}

	if( key_state[LEFT] == 1 ) {
		if( player_x > 0 ) {
			if( key_time[LEFT] == 0 ) {
				key_time[LEFT] = SDL_GetTicks();
			} else {
				key_count[LEFT] += PLAYER_SPEED*((double)(SDL_GetTicks()-key_time[LEFT]))/1000.0;
				key_time[LEFT] = SDL_GetTicks();
				player_x -= key_count[LEFT];
				if(player_x<0.0) {
					player_x = 0.0;
				}
			}
		}
	}

	if( key_state[RIGHT] == 1 ) {
		if( player_x < screen_ratio-(PLAYER_SIZE*pl[player_img_num]->w) ) {
			if( key_time[RIGHT] == 0 ) {
				key_time[RIGHT] = SDL_GetTicks();
			} else {
				key_count[RIGHT] += PLAYER_SPEED*((double)(SDL_GetTicks()-key_time[RIGHT]))/1000.0;
				key_time[RIGHT] = SDL_GetTicks();
				player_x += key_count[RIGHT];
				if(player_x>screen_ratio-(PLAYER_SIZE*pl[player_img_num]->w)) {
					player_x=screen_ratio-(PLAYER_SIZE*pl[player_img_num]->w);
				}
			}
		}
	}

	/* Draw */
	printd(DEBUG_SDL,"Copying player to screen\n");
	opengl_blit(pixel_ref_size,player_x,player_y,pl[player_img_num],PLAYER_SIZE,0.0);
}

static int check_box(double l1, double r1, double t1, double b1, double l2, double r2, double t2, double b2 )
{
	double w,h;

	if( l1 < l2 ) {
		l1 = l2;
	}
	if( t1 < t2 ) {
		t1 = t2;
	}
	if( r1 > r2 ) {
		r1 = r2;
	}
	if( b1 > b2 ) {
		b1 = b2;
	}
	w = r1 - l1;
	h = b1 - t1;

	return w>0 && h>0;
	/*
	        if( (l1 > l2 && l1 < r2) || (r1 > l2 && r1 < r2) ) {
	                if( (t1 > t2 && t1 < b2 ) || (b1 > t2 && b1 < b2) ) {
	                        return 1;
	                }
	        }
	        if( (l2 > l1 && l2 < r1) || (r2 > l1 && r2 < r1) ) {
	                if( (t2 > t1 && t2 < b1 ) || (b2 > t1 && b2 < b1) ) {
	                        return 1;
	                }
	        }

	        return 0;
	*/
}
static void check_collision()
{
	int i;
	double pL;
	double pr;
	double pt;
	double pb;

	double sl,sr,st,sb;
	double l,r,t,b;

	shot_t * current;

	if(!player_alive) {
		return;
	}

	if( pl[player_img_num] == NULL ) {
		return;
	}

	pL = player_x;
	pr = player_x + PLAYER_SIZE * pl[player_img_num]->w;
	pt = player_y;
	pb = player_y + PLAYER_SIZE * pl[player_img_num]->h;

	for(i=0; i<SPRITE_NUM_MAX; i++) {
		if( sp_array[i].img_num == -1 ) {
			continue;
		}

		sl = sp_array[i].x;
		sr = sp_array[i].x + SPRITE_SIZE * sp[sp_array[i].img_num]->w;
		st = sp_array[i].y;
		sb = sp_array[i].y + SPRITE_SIZE * sp[sp_array[i].img_num]->h;

		/* sprite versus player */
		if(check_box(pL,pr,pt,pb,sl,sr,st,sb)) {
			player_alive = 0;
		}

		/* sprites versus shots */
		current = shot_list;
		while(current != NULL ) {
			l = current->shot->x;
			r = current->shot->x + SHOT_SIZE * pl[current->shot->img_num]->w;
			t = current->shot->y;
			b = current->shot->y + SHOT_SIZE * pl[current->shot->img_num]->h;
			if(check_box(sl,sr,st,sb,l,r,t,b)) {
				sp_array[i].img_num = -1;
				current = remove_shot(current);
				continue;
			}

			if(current) {
				current = current->next;
			}
		}
	}
}

void sprite_control_shoot()
{
	if(player_alive) {
		add_shot(player_x + PLAYER_SIZE * pl[player_img_num]->w,
				 player_y + PLAYER_SIZE * pl[player_img_num]->h/2.0 - SHOT_SIZE * pl[player_img_num+1]->h/2.0);
	}
}

void sprite_control_restart()
{
	if(player_dead == 1) {
		if( pl[(player_img_num+3)%MAX_PLAYER] != NULL) {
			printd(DEBUG_IMAGE_CACHE,"Delete player %d\n",player_img_num);
			opengl_delete_texture(pl[player_img_num]);
			pl[player_img_num] = NULL;
			sem_post(&load_ctx_pl.array_sem);
			printd(DEBUG_IMAGE_CACHE,"Delete player %d\n",(player_img_num+1)%MAX_PLAYER);
			opengl_delete_texture(pl[(player_img_num+1)%MAX_PLAYER]);
			pl[(player_img_num+1)%MAX_PLAYER] = NULL;
			sem_post(&load_ctx_pl.array_sem);
			player_img_num = (player_img_num+2)%MAX_PLAYER;
		}
		player_alive = 1;
	}
}

void sprite_control_up(int active)
{
	key_state[UP] = active;
}

void sprite_control_right(int active)
{
	key_state[RIGHT] = active;
}

void sprite_control_down(int active)
{
	key_state[DOWN] = active;
}

void sprite_control_left(int active)
{
	key_state[LEFT] = active;
}

void sprite_draw(int pixel_ref_size,double screen_ratio)
{
	draw_sprite(pixel_ref_size,screen_ratio);
	draw_shot(pixel_ref_size,screen_ratio);
	draw_player(pixel_ref_size,screen_ratio);
	check_collision();
}
