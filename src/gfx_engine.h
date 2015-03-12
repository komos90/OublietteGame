/*
Hand coded 3d software renderer project.
Graphics engine code.
Seoras Macdonald
seoras1@gmail.com
2015
*/
#pragma once

#ifdef __linux__
    #include "SDL2/SDL.h"
    #define M_PI 3.14159265358979323846
#elif _WIN32
    #include "SDL.h"
#endif 

#include "engine_types.h"
#include "load_level.h"

//Projection Constants
static const int TILE_DIMS     = 64;
static const float H_FOV       = M_PI/2;
static const float V_FOV       = M_PI/3;

//IDEA have pixelBuffer as static variable in gfx_engine, use a function to set it?
void drawRect (SDL_Rect rect, uint32_t color, PixelBuffer pixelBuffer);
void drawPoint (int x, int y, uint32_t color, PixelBuffer pixelBuffer);
void draw (Entity player, Level level, PixelBuffer pixelBuffer);
