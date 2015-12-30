#!/bin/bash

CFLAGS=`pkg-config --cflags sdl`
LIBS=`pkg-config --libs sdl`

#gcc -o web-shoot main.c debug.c network.c disk.c opengl.c sprite.c background.c yandex.c misc.c image_fifo.c loader.c -g -O0 -Wall $CFLAGS $LIBS -lSDL_image -lcurl -lpthread -lreadline -lGL -lm
gcc -o web-shoot common.c main.c debug.c network.c opengl.c background.c misc.c image_fifo.c loader.c \
engine_yandex.c \
engine_test.c \
engine_file.c \
engine_wikimedia.c \
engine_deviantart.c \
engine_framabee.c \
engine_qwant.c \
-g -O0 -Wall $CFLAGS $LIBS -lSDL_image -lcurl -lpthread -lreadline -lGL -lm
