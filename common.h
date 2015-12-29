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

#ifndef COMMON_H
#define COMMON_H

#include <stdlib.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <pthread.h>
#include <semaphore.h>

#define FALSE 0
#define TRUE 1

#define UNDEF_COORD (-99999)

#define SMALL_BUF 1024
#define LARGE_BUF 10240

#define NUM_THREAD (8)

char * backup_dir;

#endif
