/*
Raycaster wolfenstein 3d style game.
Seoras Macdonald
seoras1@gmail.com
2015
*/
#pragma once

#ifdef __linux__
    #include <SDL2/SDL.h>
#elif __WIN32
    #include <SDL.h>
#endif


typedef struct
{
    SDL_Surface* caveTexture;
    SDL_Surface* doorTexture;
    SDL_Surface* secretDoorTexture;
    SDL_Surface* floorTexture;
    SDL_Surface* ceilingTexture;

    SDL_Surface* rubySprite;
    SDL_Surface* keySprite;
    SDL_Surface* monsterSprite;
    SDL_Surface* mainMenuBack;
    SDL_Surface* mainMenuTitle;
    SDL_Surface* mainMenuStartButton;
} ImageManager;

extern ImageManager images;


void loadImages(void);