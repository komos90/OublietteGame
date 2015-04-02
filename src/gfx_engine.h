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

static const float H_FOV = M_PI/3;
static const float V_FOV = M_PI/3;


void drawRect (SDL_Rect rect, uint32_t color);
void drawPoint (int x, int y, uint32_t color);
void drawText(char* text, SDL_Rect rect, uint32_t color, SpriteFont spriteFont);
void createPixelBuffer(int width, int height);
PixelBuffer* getPixelBuffer();
void draw (Player player, EntityArray entities);
