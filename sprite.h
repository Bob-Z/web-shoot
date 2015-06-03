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

void sprite_init(char * keyword_pl, char * keyword_sp,char * filter);
void sprite_draw(int pixel_ref_size,double screen_ratio);
void sprite_control_shoot();
void sprite_control_up(int active);
void sprite_control_right(int active);
void sprite_control_down(int active);
void sprite_control_left(int active);
void sprite_control_restart();
