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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "SDL.h"
#include "SDL_image.h"
#include <SDL_opengl.h>
#include "errno.h"
#include "debug.h"
#include "parameter.h"
#include "common.h"
#include "network.h"
#include "loader.h"
#include <pthread.h>

/******************************
 engine_destroy
return 0 if no error
******************************/
static int engine_destroy(engine_t * engine)
{
	return 0;
}

/*******************************
  engine_get_url

Return string MUST be freed
 ******************************/
static char * engine_get_url(engine_t * engine)
{
	return strdup("http://www.smashbros.com/images/og/mario.jpg");
}

/******************************
 engine_create
return 0 if no error
******************************/
int test_engine_init(engine_t * engine,const char * keyword,int size,int filter)
{
	engine->engine_destroy=engine_destroy;
	engine->engine_get_url=engine_get_url;

	return 0;
}
