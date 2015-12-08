#/bin/bash
gcc -std=c99 -Wall -O3 -g src/*.c -lSDL2 -lSDL2main -lSDL2_image -lSDL2_mixer -lm -o bin/game.elf

