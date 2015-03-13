#pragma once

#include <stdint.h>
#include <stdbool.h>

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
    Vector2 pos;
    float rotation;
} Entity;

typedef struct
{
    int length;
    Entity* data;
} EntityArray;

Vector2 vec2Unit(Vector2 vector);