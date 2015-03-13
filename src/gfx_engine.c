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
    #include <SDL2/SDL_image.h>
    #define M_PI 3.14159265358979323846
#elif __Win32
    #include <SDL.h>
    #include <SDL_image.h>
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

bool isTileIndexValid(int i, Level level) {
    return i >= 0 && i < level.width * level.height;
}

Vector2Int getTileHorzIntersection(Vector2 pos, float angle, Level level, PixelBuffer pixelBuffer)
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
    Vector2Int intersectVector = { .x=xInter, .y=yInter };
    return intersectVector;//sqrt(pow(xInter - pos.x, 2) + pow(yInter - pos.y, 2));
}

Vector2Int getTileVertIntersection(Vector2 pos, float angle, Level level, PixelBuffer pixelBuffer)
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
    Vector2Int intersectVector = { .x=xInter, .y=yInter };
    return intersectVector;//sqrt(pow(xInter - pos.x, 2) + pow(yInter - pos.y, 2));
}

float wallDistanceToHeight(float distance, PixelBuffer pixelBuffer)
{
    float screenViewHeight = distance * tanf(V_FOV/2.f);
    float displayHeight = TILE_DIMS * (pixelBuffer.height / screenViewHeight);
    return displayHeight;
}

//Has some temp debug parameters
void draw(Entity player, Level level, SDL_Surface* caveTexture, PixelBuffer pixelBuffer)
{
    //Draw floor
    {
        SDL_Rect floorRect = {0, pixelBuffer.height / 2, pixelBuffer.width, pixelBuffer.height};
        drawRect(floorRect, 0x000E0904, pixelBuffer);
    }

    float angle = -H_FOV/2 + player.rotation;
    for (int screenColumn = 0; screenColumn < pixelBuffer.width; screenColumn++)
    {
        //Get distance and intersect pos
        float distance;
        Vector2Int intersectPos;
        {
            Vector2Int hIntersect = getTileHorzIntersection(player.pos, angle, level, pixelBuffer);
            float hDistance = sqrt(pow(hIntersect.x - player.pos.x, 2) + pow(hIntersect.y - player.pos.y, 2)) * cos(angle - player.rotation);

            Vector2Int vIntersect = getTileVertIntersection(player.pos, angle, level, pixelBuffer);
            float vDistance = sqrt(pow(vIntersect.x - player.pos.x, 2) + pow(vIntersect.y - player.pos.y, 2)) * cos(angle - player.rotation);

            intersectPos = hDistance <= vDistance ? hIntersect : vIntersect;
            distance = hDistance <= vDistance ? hDistance : vDistance;
        }
        float height = wallDistanceToHeight(distance, pixelBuffer);

        for (int y = (pixelBuffer.height - height) / 2; y < (pixelBuffer.height + height) / 2; y++)
        {
            if (y >= pixelBuffer.height) break;
            if (y < 0) y = 0;
            uint32_t color32 = 0;
            uint8_t color[3]; //0x000000FF - distance / 4;
            //if (color > 0x000000FF) color = 0;
            float depthColor[3];
            depthColor[0] = (0.5 / distance) * 100;
            depthColor[1] = (0.5 / distance) * 100;
            depthColor[2] = (0.5 / distance) * 100;
            //Texture Mapping
            {
                int yTexCoord = ((TILE_DIMS / (height)) * (y - (pixelBuffer.height - height) / 2));
                int xTexCoord = intersectPos.x % TILE_DIMS + intersectPos.y % TILE_DIMS;
                if (yTexCoord < 0) yTexCoord = 0;
                else if (yTexCoord >= TILE_DIMS) yTexCoord = TILE_DIMS - 1;

                uint8_t* texPixels = (uint8_t*)caveTexture->pixels;
                memcpy(&color32, &texPixels[yTexCoord * caveTexture->pitch + xTexCoord * caveTexture->format->BytesPerPixel], caveTexture->format->BytesPerPixel);
                //SDL_Log("%d", caveTexture->format->BytesPerPixel);
            }

            uint8_t finalColor[3];
            for (int i = 0; i < 3; i++)
            {
                color[i] = (color32 >> i * 8);
                finalColor[i] = color[i] * depthColor[i];
                if (depthColor[i] >= 1.0) finalColor[i] = color[i];
            }
            uint32_t finalColor32 = (finalColor[0] << 16) | (finalColor[1] << 8) | finalColor[2];

            drawPoint(screenColumn, y, (uint32_t)finalColor32, pixelBuffer);
        }

        angle += (H_FOV / pixelBuffer.width);
        //fix angle
        while (angle > M_PI) angle -= 2 * M_PI;
        while (angle < -M_PI) angle += 2 * M_PI;
    }
}