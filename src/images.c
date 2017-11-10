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
#elif _WIN32
    #include <SDL.h>
    #include <SDL_image.h>
#endif

#include "images.h"


ImageManager images = {0};

void loadImage(SDL_Surface** image, char* filePath) {
    //MALLOC no need to free, needed throughout program
    *image = IMG_Load(filePath);
    assert(*image != NULL);
    *image = SDL_ConvertSurfaceFormat(*image, SDL_PIXELFORMAT_ARGB8888, 0);
}

void loadImages(void) {
    loadImage(&images.caveTexture,          "res/textures/cave.png");
    loadImage(&images.doorTexture,          "res/textures/locked_door.png");
    loadImage(&images.secretDoorTexture,    "res/textures/secret_door.png");
    loadImage(&images.floorTexture,         "res/textures/floor.png");
    loadImage(&images.ceilingTexture,       "res/textures/ceiling.png");
    loadImage(&images.rubySprite,           "res/sprites/ruby.png");
    loadImage(&images.keySprite,            "res/sprites/key.png");
    loadImage(&images.monsterSprite,        "res/sprites/monster.png");
    loadImage(&images.mainMenuBack,         "res/sprites/main_menu_back.png");
    loadImage(&images.mainMenuTitle,        "res/sprites/main_menu_title.png");
    loadImage(&images.mainMenuStartButton,  "res/sprites/main_menu_start.png");
    loadImage(&images.levelEndPortal,       "res/sprites/level_end_portal.png");
    loadImage(&images.compass,              "res/sprites/compass.png");
    loadImage(&images.instructionsTexture1, "res/textures/instructions1.png");
    loadImage(&images.instructionsTexture2, "res/textures/instructions2.png");
    loadImage(&images.instructionsTexture3, "res/textures/instructions3.png");
    loadImage(&images.instructionsTexture4, "res/textures/instructions4.png");
}
