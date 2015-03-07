#pragma once

typedef struct 
{
    uint32_t* pixels;
    float* zBuffer;
    int width;
    int height;
} PixelBuffer;

typedef struct
{
    int x;
    int y;
    int z;
} Vector3Int;

typedef struct 
{
    float x;
    float y;
    float z;
} Vector3;

typedef struct
{
    float x;
    float y;
    float z;
    float w;
} Vector4;

typedef struct
{
    Vector3 vectors[3];
} Triangle;

typedef struct
{
    float values[16];
} Matrix4;

typedef struct
{
    int polyCount;
    Triangle* polygons;
} Mesh;

typedef struct
{
    Vector3 position;
    Vector3 rotation; //TODO Quaternion?
    Vector3 scale;
    Mesh mesh;
    uint32_t color; //TEMP
} Entity;

typedef struct
{
    int length;
    Entity* data;
} EntityArray;
