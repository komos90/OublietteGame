#/bin/bash

gcc -std=c99 -Wall -O3 src/*.c -lSDL2 -lm -o bin/game.elf
