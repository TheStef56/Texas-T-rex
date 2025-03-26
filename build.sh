#!/bin/bash

set -xe

cp -r SDL2-deps-linux build
cd build && make -j16

#cd dependencies && make
cc -o trex main.c -g -Wall -Wextra -I./include -L./lib -lSDL2 -lSDL2main -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lm
