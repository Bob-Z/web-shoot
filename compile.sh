#!/bin/bash

CFLAGS=`pkg-config --cflags sdl`
LIBS=`pkg-config --libs sdl`

#gcc -o web-shoot main.c debug.c network.c disk.c opengl.c sprite.c background.c yandex.c misc.c image_fifo.c loader.c -g -O0 -Wall $CFLAGS $LIBS -lSDL_image -lcurl -lpthread -lreadline -lGL -lm
gcc -o web-shoot main.c debug.c network.c opengl.c background.c yandex.c misc.c image_fifo.c loader.c test_engine.c file_engine.c wikimedia_engine.c -g -O0 -Wall $CFLAGS $LIBS -lSDL_image -lcurl -lpthread -lreadline -lGL -lm
