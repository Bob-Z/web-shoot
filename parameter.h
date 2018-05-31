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

#include "debug.h"

//#define DEBUG_LEVEL ( DEBUG_ERROR | DEBUG_HTTP | DEBUG_URL | DEBUG_SDL | DEBUG_IMAGE_CACHE)
//#define DEBUG_LEVEL ( DEBUG_ERROR | DEBUG_IMAGE_CACHE | DEBUG_HTTP)
#define DEBUG_LEVEL ( DEBUG_ERROR | DEBUG_DISK | DEBUG_IMAGE_CACHE)
//#define DEBUG_LEVEL ( 0 )

#define TMP_FILE "webshooter"

#define DEF_HTTP_TIMEOUT 10

#define WINDOW_SIZE_X 100
#define WINDOW_SIZE_Y 100

#define MAX_BACKGROUND 10
#define MAX_SPRITE 10
#define MAX_PLAYER 10

#define BACKGROUND_SPEED 0.2 // in pixels per frames

#define PLAYER_SIZE 0.20
#define PLAYER_SPEED 0.7

#define SHOT_SPEED 1.5
#define SHOT_ROTATION_SPEED 0.5
#define SHOT_SIZE 0.02

#define SPRITE_NUM_MAX 3
#define SPRITE_SIZE 0.20
//#define SPRITE_SPEED 0.6

#define PLAYER_DEAD_TIME 1.0
