#ifndef IMAGE_H
#define IMAGE_H

#include <stdlib.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <pthread.h>
#include <semaphore.h>

typedef struct img {
        SDL_Surface * surf;
        GLuint  tex;
        double ratio;
        double w;
        double h;
        int init;
} img_t;
#endif
