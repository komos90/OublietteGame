#!/bin/bash
#W_FLAGS="-Wall -Werror -Wshadow" #-fmudflap
W_FLAGS="-Wall -Wshadow" #-fmudflap

clang -std=c99 $W_FLAGS -O3 -g src/*.c -lSDL2 -lSDL2main -lSDL2_image -lSDL2_mixer -lm -o bin/game.elf

