#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct 
{
    uint32_t* pixels;
    //float* zBuffer;
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

/*typedef struct
{
    float values[16];
} Matrix4;*/

typedef struct
{
    Vector2 pos;
    float rotation; //TODO Quaternion?
} Entity;

typedef struct
{
    int length;
    Entity* data;
} EntityArray;
