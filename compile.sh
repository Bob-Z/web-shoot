#!/bin/bash

CFLAGS=`pkg-config --cflags sdl`
LIBS=`pkg-config --libs sdl`

#gcc main.c debug.c network.c -g -O0 -Wall $CFLAGS $LIBS -lSDL_image -lSDL_gfx -lcurl -lpthread -lreadline
gcc -Wall -o web-shoot main.c debug.c network.c disk.c opengl.c sprite.c background.c -g -O0 -Wall $CFLAGS $LIBS -lSDL_image -lcurl -lpthread -lreadline -lGL -lm
