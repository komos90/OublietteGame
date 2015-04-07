
gcc -std=c99 -O3 -pg src/*.c -IC:libs/SDL/include/SDL2 -LC:libs/SDL/lib -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_mixer -o bin/wingame
@pause