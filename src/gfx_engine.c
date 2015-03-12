/*
Hand coded 3d software renderer project.
Graphics engine code.
Seoras Macdonald
seoras1@gmail.com
2015
*/
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <float.h>

#ifdef __linux__
    #include <SDL2/SDL.h>
    #define M_PI 3.14159265358979323846
#elif __Win32
    #include <SDL.h>
#endif

#include "gfx_engine.h"
#include "engine_types.h"
#include "load_level.h"


void drawRect(SDL_Rect rect, uint32_t color, PixelBuffer pixelBuffer)
{
    if (rect.y < 0) rect.y = 0;
    if (rect.x < 0) rect.x = 0;
    for (int yy = rect.y; yy < rect.y + rect.h; ++yy)
    {
        for (int xx = rect.x; xx < rect.x + rect.w; ++xx)
        {
            if (yy * pixelBuffer.width + xx >= pixelBuffer.width * pixelBuffer.height)
                return;
            pixelBuffer.pixels[yy * pixelBuffer.width + xx] = color;
        }
    }
}

void drawPoint(int x, int y, uint32_t color, PixelBuffer pixelBuffer)
{
    pixelBuffer.pixels[(int)y  * pixelBuffer.width + (int)x] = color;
}

int posToTileIndex(int x, int y, Level level)
{
    int index = (int)((y / TILE_DIMS) * level.width + (x / TILE_DIMS));
    return index;
}

bool isTileIndexValid(int i, Level level) {
    return i >= 0 && i < level.width * level.height;
}

float getTileHorzIntersectionDistance(Vector2 pos, float angle, Level level, PixelBuffer pixelBuffer)
{
    bool xPositive = angle >= -M_PI / 2 && angle <= M_PI / 2;
    bool yPositive = angle <= 0;

    float yInter = (float)(((int)pos.y / TILE_DIMS) * TILE_DIMS);
    if (!yPositive) yInter += TILE_DIMS;

    float xAngle = angle;
    if (xPositive && yPositive) xAngle = -M_PI/2 + angle;
    else if (!xPositive && yPositive) xAngle = angle - M_PI/2;
    else if (xPositive && !yPositive) xAngle = M_PI/2 - angle;
    else if (!xPositive && !yPositive) xAngle = -angle + M_PI/2;

    float xInter = (abs(yInter - pos.y) * tanf(xAngle)) + pos.x;
    float xInc = TILE_DIMS * tanf(xAngle);

    int xDir = xPositive ? -1 : 1;
    int yDir = yPositive ? -1 : 1;
    while (isTileIndexValid(posToTileIndex(xInter + xDir, yInter + yDir, level), level) &&
           level.data[posToTileIndex(xInter + xDir, yInter + yDir, level)] != '#')
    {   
        yInter += (!yPositive) ? TILE_DIMS : -TILE_DIMS;
        xInter += xInc;
    }
    return sqrt(pow(xInter - pos.x, 2) + pow(yInter - pos.y, 2));
}

float getTileVertIntersectionDistance(Vector2 pos, float angle, Level level, PixelBuffer pixelBuffer)
{
    bool xPositive = angle >= -M_PI / 2 && angle <= M_PI / 2;
    bool yPositive = angle <= 0;

    float xInter = (float)(((int)pos.x / TILE_DIMS) * TILE_DIMS);
    if (xPositive) xInter += TILE_DIMS;

    float yInter = (abs(xInter - pos.x) * tanf(xPositive ? angle : M_PI - angle)) + pos.y;
    float yInc = TILE_DIMS * tanf(xPositive ? angle : M_PI - angle);

    int xDir = xPositive ? 1 : -1;
    int yDir = yPositive ? 1 : -1;
    while (isTileIndexValid(posToTileIndex(xInter + xDir, yInter + yDir, level), level) &&
           level.data[posToTileIndex(xInter + xDir, yInter + yDir, level)] != '#')
    {   
        xInter += (xPositive) ? TILE_DIMS : -TILE_DIMS;
        yInter += yInc;
    }
    return sqrt(pow(xInter - pos.x, 2) + pow(yInter - pos.y, 2));
}

float wallDistanceToHeight(float distance, PixelBuffer pixelBuffer)
{
    float screenViewHeight = distance * tanf(V_FOV/2.f);
    float displayHeight = TILE_DIMS * (pixelBuffer.height / screenViewHeight);
    return displayHeight;
}

//Has some temp debug parameters
void draw(Entity player, Level level, PixelBuffer pixelBuffer)
{
    //Draw floor
    {
        SDL_Rect floorRect = {0, pixelBuffer.height / 2, pixelBuffer.width, pixelBuffer.height};
        drawRect(floorRect, 0x55555555, pixelBuffer);
    }

    float angle = -H_FOV/2 + player.rotation;
    for (int screenColumn = 0; screenColumn < pixelBuffer.width; screenColumn++)
    {
        float hDistance = getTileHorzIntersectionDistance(player.pos, angle, level, pixelBuffer);
        float vDistance = getTileVertIntersectionDistance(player.pos, angle, level, pixelBuffer);
        float distance = hDistance <= vDistance ? hDistance : vDistance;
        float height = wallDistanceToHeight(distance, pixelBuffer);

        for (int y = (pixelBuffer.height - height) / 2; y < (pixelBuffer.height + height) / 2; y++)
        {
            if (y >= pixelBuffer.height) break;
            if (y < 0) y = 0;

            drawPoint(screenColumn, y, 0xFFFF0000, pixelBuffer);
        }

        angle += (H_FOV / pixelBuffer.width);
        //fix angle
        while (angle > M_PI) angle -= 2 * M_PI;
        while (angle < -M_PI) angle += 2 * M_PI;
    }
}