#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __linux__
    #include <SDL2/SDL.h>
#elif __WIN32
    #include <SDL.h>
#endif

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

typedef enum
{
    ENTITY_TYPE_RUBY,
} EntityType;

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
    int width;
    int height;
    float rotation;
} Player;

typedef struct
{
    SDL_Surface* sprite;
    int width;
    int height;
} EntityTemplate;

typedef struct
{
    Vector2 pos;
    EntityTemplate* base;
} Entity;

typedef struct
{
    int size;
    Entity* data;
} EntityArray;

typedef struct
{
    int levelNumber;
    int rubiesCollected;
} PlayerData;

Vector2 vec2Unit(Vector2 vector);
float constrainAngle(float angle);
float distanceFormula(Vector2 vec0, Vector2 vec1);
bool rectsIntersect(SDL_Rect rect0, SDL_Rect rect1);