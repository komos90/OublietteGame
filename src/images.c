/*
Raycaster wolfenstein 3d style game.
Seoras Macdonald
seoras1@gmail.com
2015
*/
#include <stdbool.h>
#include <assert.h>

#ifdef __linux__
    #include <SDL2/SDL.h>
    #include <SDL2/SDL_image.h>
#elif __WIN32
    #include <SDL.h>
    #include <SDL_image.h>
#endif

#include "images.h"

ImageManager images = {0};

void loadImages() {

    //MALLOC These images are needed throught the program, no need to free.
    images.caveTexture = IMG_Load("../res/textures/cave.png");
    images.caveTexture = SDL_ConvertSurfaceFormat(images.caveTexture, SDL_PIXELFORMAT_ARGB8888, 0);
    assert(images.caveTexture != NULL);

    images.doorTexture = IMG_Load("../res/textures/locked_door.png");
    images.doorTexture = SDL_ConvertSurfaceFormat(images.doorTexture, SDL_PIXELFORMAT_ARGB8888, 0);
    assert(images.doorTexture != NULL);

    images.secretDoorTexture = IMG_Load("../res/textures/secret_door.png");
    images.secretDoorTexture = SDL_ConvertSurfaceFormat(images.secretDoorTexture, SDL_PIXELFORMAT_ARGB8888, 0);
    assert(images.secretDoorTexture != NULL);

    images.rubySprite  = IMG_Load("../res/sprites/ruby.png");
    images.rubySprite  = SDL_ConvertSurfaceFormat(images.rubySprite, SDL_PIXELFORMAT_ARGB8888, 0);
    assert(images.rubySprite != NULL);
    
    images.keySprite   = IMG_Load("../res/sprites/key.png");
    images.keySprite   = SDL_ConvertSurfaceFormat(images.keySprite, SDL_PIXELFORMAT_ARGB8888, 0);
    assert(images.keySprite != NULL);

    images.monsterSprite   = IMG_Load("../res/sprites/monster.png");
    images.monsterSprite   = SDL_ConvertSurfaceFormat(images.monsterSprite, SDL_PIXELFORMAT_ARGB8888, 0);
    assert(images.monsterSprite != NULL);
}