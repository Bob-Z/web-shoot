#ifndef OPENGL_H
#define OPENGL_H

#include "image.h"

double opengl_init(int, int);
int    opengl_init_texture(img_t * img);
void   opengl_delete_texture(img_t * img);
void   opengl_blit(int pixel_ref_size, double x, double y, img_t * src, double size, double a);
void   opengl_clear_screen();

#endif
