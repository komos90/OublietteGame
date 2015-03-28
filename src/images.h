/*
Raycaster wolfenstein 3d style game.
Seoras Macdonald
seoras1@gmail.com
2015
*/
#pragma once

#ifdef __linux__
    #include <SDL2/SDL.h>
    #include <SDL2/SDL_image.h>
#elif __WIN32
    #include <SDL.h>
    #include <SDL_image.h>
#endif


typedef struct
{
    SDL_Surface* caveTexture;
    SDL_Surface* doorTexture;
    SDL_Surface* secretDoorTexture;

    SDL_Surface* rubySprite;
    SDL_Surface* keySprite;
    SDL_Surface* monsterSprite;
} ImageManager;


extern ImageManager images;


void loadImages();