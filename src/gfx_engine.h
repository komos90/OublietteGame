/*
Raycaster wolfenstein 3d style game.
Seoras Macdonald
seoras1@gmail.com
2015
*/
#pragma once

#ifdef __linux__
    #include "SDL2/SDL.h"
    #include "SDL2/SDL_image.h"
#elif _WIN32
    #include "SDL.h"
    #include "SDL_image.h"
#endif 

#include "engine_types.h"
#include "load_level.h"


static const float H_FOV       = M_PI/3;
static const float V_FOV       = M_PI/3;


void drawRect (SDL_Rect rect, uint32_t color);
void drawPoint (int x, int y, uint32_t color);
PixelBuffer* createPixelBuffer(int width, int height);
void draw (Entity player, Level level, SDL_Surface* caveTexture);
