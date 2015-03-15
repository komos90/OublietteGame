#pragma once

#include <stdint.h>

#ifdef __linux__
    #include <SDL2/SDL.h>
#elif __WIN32
    #include <SDL.h>
#endif

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

typedef struct 
{
    uint32_t* pixels;
    int width;
    int height;
} PixelBuffer;

typedef struct
{
    int x;
    int y;
} Vector2Int;

typedef struct 
{
    float x;
    float y;
} Vector2;

typedef struct
{
    SDL_Surface* sprite;
    int charW;
    int charH;
} SpriteFont;

typedef struct
{
    Vector2 pos;
    float rotation;
} Entity;

typedef struct
{
    Vector2 pos;
    SDL_Surface* sprite;
} Drawable;

typedef struct
{
    int size;
    Drawable* data;
} DrawablesArray;

typedef struct
{
    int length;
    Entity* data;
} EntityArray;

Vector2 vec2Unit(Vector2 vector);
float constrainAngle(float angle);
float distanceFormula(Vector2 vec0, Vector2 vec1);