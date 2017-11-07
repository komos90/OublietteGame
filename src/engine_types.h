#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __linux__
    #include <SDL2/SDL.h>
#elif _WIN32
    #include <SDL.h>
#endif

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif
#define MAX_KEYS 4


typedef enum
{
    ENTITY_TYPE_RUBY,
    ENTITY_TYPE_KEY,
    ENTITY_TYPE_MONSTER,
    ENTITY_TYPE_PORTAL
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
    int footstepSoundChannel;
} Player;

typedef struct
{
    SDL_Surface* sprite;
    EntityType type;
    int width;
    int height;
    int spriteWidth;
    int spriteHeight;
    int animationSpeed;
} EntityTemplate;

typedef struct
{
    Vector2 pos;
    float zPos;
    int xClip;
    int yClip;
    EntityTemplate* base;
    void* sub;
    int xClipCounter;
} Entity;

typedef struct
{
    int size;
    Entity* data;
} EntityArray;

typedef struct
{
    int id;
} Key;

typedef struct
{
    int levelNumber;
    int rubiesCollected;
    bool keysCollected[MAX_KEYS];
    int totalRubiesCollected;
    int totalRubies;
} PlayerData;

typedef struct
{
    bool active;
    int endTime;
} CountdownTimer;

typedef SDL_Rect Rectangle;

//TODO should have tile type with properties such as isSolid, keyId

Vector2 vec2Unit       (Vector2 vector);
float   constrainAngle (float angle);
float   distanceFormula(Vector2 vec0, Vector2 vec1);
bool    rectsIntersect (SDL_Rect rect0, SDL_Rect rect1);
