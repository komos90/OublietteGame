/*
Raycaster wolfenstein 3d style game.
Seoras Macdonald
seoras1@gmail.com
2015
*/
#pragma once

#include <stdbool.h>

#ifdef __linux__
    #include "SDL2/SDL.h"
    #include "SDL2/SDL_image.h"
#elif _WIN32
    #include "SDL.h"
    #include "SDL_image.h"
#endif

#include "engine_types.h"


//Resolution
static int SCREEN_WIDTH        = 0;
static const int SCREEN_HEIGHT = 256;

//Aspect Ratio
static const float H_FOV = M_PI/3;
static const float V_FOV = M_PI/4;//(3*M_PI)/16;


PixelBuffer* getPixelBuffer  (void);
void drawRect                (SDL_Rect rect, uint32_t color);
void drawPoint               (int x, int y, uint32_t color);
void blitToPixelBuffer       (SDL_Surface* image, Rectangle destRect, uint32_t maskColor);
void drawText                (char* text, SDL_Rect rect, uint32_t color, SpriteFont spriteFont, bool centered);
void drawTextToSurface       (char* text, SDL_Surface* surface, SDL_Rect rect, uint32_t color, SpriteFont spriteFont);
void createPixelBuffer       (int width, int height);
void draw                    (Player player, EntityArray entities);
void pixelateScreen          (int n);
void fadeToColor             (uint32_t addColor, float ratio);
void rotatedBlitToPixelBuffer(SDL_Surface* image, Rectangle destRect, uint32_t maskColor, float angle);