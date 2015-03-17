/*
Raycaster wolfenstein 3d style game.
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
#elif __WIN32
    #include <SDL.h>
    #include <SDL_image.h>
#endif

#include "gfx_engine.h"
#include "engine_types.h"
#include "load_level.h"


static PixelBuffer pixelBuffer = {0};
static float* zBuffer = NULL;


void drawRect(SDL_Rect rect, uint32_t color)
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

void drawPoint(int x, int y, uint32_t color)
{
    pixelBuffer.pixels[(int)y  * pixelBuffer.width + (int)x] = color;
}

void drawText(char* text, SDL_Rect rect, uint32_t color, SpriteFont spriteFont)
{
    //get text length
    int textLength = 0;
    for (int i = 0; text[i] != '\0'; i++)
        textLength++;

    for (int i = 0; i < textLength; i++)
    {
        int spriteX = (text[i] - 32) * spriteFont.charW;

        for (int y = 0; y < spriteFont.charH; y++)
        {
            for (int x = spriteX; x < spriteX + spriteFont.charW; x++)
            {
                uint32_t pixelColor = ((uint32_t*)spriteFont.sprite->pixels)[y * spriteFont.sprite->w + x] & color;
                if (pixelColor & 0xFF000000) {
                    pixelBuffer.pixels[(rect.y + y) * pixelBuffer.width + (rect.x + i * spriteFont.charW + x - spriteX)] = pixelColor;
                }
            }
        }
    }
}

PixelBuffer* createPixelBuffer(int width, int height)
{
    //MALLOC no need to free, kept until program end.
    uint32_t* pixels = (uint32_t*)malloc(width * height * sizeof(uint32_t));
    zBuffer = (float*)malloc(width * sizeof(float));
    pixelBuffer.pixels = pixels;
    pixelBuffer.width = width;
    pixelBuffer.height = height;
    return &pixelBuffer;
}

bool isTileIndexValid(int i, Level level) {
    return i >= 0 && i < level.width * level.height;
}

Vector2Int getTileHorzIntersection(Vector2 pos, float angle, Level level)
{
    angle = constrainAngle(angle);
    bool xPositive = angle >= -M_PI / 2 && angle <= M_PI / 2;
    bool yPositive = angle <= 0;

    float yInter = (float)(((int)pos.y / TILE_DIMS) * TILE_DIMS);
    if (!yPositive) yInter += TILE_DIMS;

    float xAngle = yPositive ? angle - M_PI/2 : -(angle - M_PI/2);
    float xInter = (abs(yInter - pos.y) * tanf(xAngle)) + pos.x;
    float xInc = TILE_DIMS * tanf(xAngle);

    int xDir = xPositive ? 1 : -1;
    int yDir = yPositive ? -1 : 1;
    while (isTileIndexValid(posToTileIndex(xInter + xDir, yInter + yDir, level), level) &&
           level.data[posToTileIndex(xInter + xDir, yInter + yDir, level)] != '#')
    {   
        yInter += yPositive ? -TILE_DIMS : TILE_DIMS;
        xInter += xInc;
    }
    Vector2Int intersectVector = { .x=xInter, .y=yInter };
    return intersectVector;
}

Vector2Int getTileVertIntersection(Vector2 pos, float angle, Level level)
{
    angle = constrainAngle(angle);
    bool xPositive = angle >= -M_PI / 2 && angle <= M_PI / 2;
    bool yPositive = angle <= 0;

    float xInter = (float)(((int)pos.x / TILE_DIMS) * TILE_DIMS);
    if (xPositive) xInter += TILE_DIMS;

    float yAngle = xPositive ? angle : M_PI - angle;
    float yInter = (abs(xInter - pos.x) * tanf(yAngle)) + pos.y;
    float yInc = TILE_DIMS * tanf(yAngle);

    int xDir = xPositive ? 1 : -1;
    int yDir = yPositive ? -1 : 1;
    while (isTileIndexValid(posToTileIndex(xInter + xDir, yInter + yDir, level), level) &&
           level.data[posToTileIndex(xInter + xDir, yInter + yDir, level)] != '#')
    {   
        xInter += xPositive ? TILE_DIMS : -TILE_DIMS;
        yInter += yInc;
    }
    Vector2Int intersectVector = { .x=xInter, .y=yInter };
    return intersectVector;
}

float wallDistanceToHeight(float distance)
{
    float screenViewHeight = distance * tanf(V_FOV/2.f);
    float displayHeight = TILE_DIMS * (pixelBuffer.height / screenViewHeight);
    return displayHeight;
}

void draw(Player player, Level level, SDL_Surface* caveTexture, EntityArray entities)
{

    //Clear pixel buffer & draw ceiling
    {
        uint8_t ceilColor = 0x5;
        memset((void*)pixelBuffer.pixels, ceilColor, pixelBuffer.width * pixelBuffer.height * sizeof(uint32_t));
    }
    //Draw floor
    {
        SDL_Rect floorRect = {0, pixelBuffer.height / 2, pixelBuffer.width, pixelBuffer.height};
        drawRect(floorRect, 0x000E0904);
    }

    float angle = -H_FOV/2 + player.rotation;
    for (int screenColumn = 0; screenColumn < pixelBuffer.width; screenColumn++)
    {
        //Get distance and intersect pos
        float distance;
        Vector2Int intersectPos;
        {
            Vector2Int hIntersect = getTileHorzIntersection(player.pos, angle, level);
            float hDistance = sqrt(pow(hIntersect.x - player.pos.x, 2) + pow(hIntersect.y - player.pos.y, 2)) * cos(angle - player.rotation);

            Vector2Int vIntersect = getTileVertIntersection(player.pos, angle, level);
            float vDistance = sqrt(pow(vIntersect.x - player.pos.x, 2) + pow(vIntersect.y - player.pos.y, 2)) * cos(angle - player.rotation);

            intersectPos = hDistance <= vDistance ? hIntersect : vIntersect;
            distance = hDistance <= vDistance ? hDistance : vDistance;
        }

        //Save column distance in zBuffer
        zBuffer[screenColumn] = distance;

        float height = wallDistanceToHeight(distance);
        for (int y = (pixelBuffer.height - height) / 2; y < (pixelBuffer.height + height) / 2; y++)
        {
            //Range corrections
            if (y >= pixelBuffer.height) break;
            if (y < 0) y = 0;

            uint32_t color32 = 0;
            //Texture Mapping
            {
                int yTexCoord = ((TILE_DIMS / (height)) * (y - (pixelBuffer.height - height) / 2));
                int xTexCoord = intersectPos.x % TILE_DIMS + intersectPos.y % TILE_DIMS;
                if (yTexCoord < 0) yTexCoord = 0;
                else if (yTexCoord >= TILE_DIMS) yTexCoord = TILE_DIMS - 1;

                uint32_t* texPixels = (uint32_t*)caveTexture->pixels;
                int index = yTexCoord * caveTexture->w + xTexCoord;
                color32 = texPixels[index];
            }

            //Shade pixels for depth effect
            uint32_t finalColor32;
            {
                float intensity;
                intensity = (0.5 / distance) * 100;
                if (intensity >= 1.0) finalColor32 = color32;
                else
                {
                    uint8_t color[3];
                    uint8_t finalColor[3];
                    for (int i = 0; i < 3; i++)
                    {
                        color[i] = (color32 >> i * 8);
                        finalColor[i] = color[i] * intensity;
                    }
                    finalColor32 = (finalColor[2] << 16) | (finalColor[1] << 8) | finalColor[0];
                }
            }
            drawPoint(screenColumn, y, (uint32_t)finalColor32);
        }
        angle += (H_FOV / pixelBuffer.width);
    }

    //Sprite drawing ====
    //Sort sprites by distatnce
    for (int i = entities.size - 1; i >= 0; i--)
    {
        for (int j = 0; j < i; j++)
        {
            if (distanceFormula(entities.data[j].pos, player.pos) < 
                distanceFormula(entities.data[j + 1].pos, player.pos))
            {
                //Swap
                Entity tmp = entities.data[j];
                entities.data[j] = entities.data[j + 1];
                entities.data[j + 1] = tmp;
            }
        }
    }

    for ( int entityIndex = 0; entityIndex < entities.size; entityIndex++)
    {
        Entity entity = entities.data[entityIndex];
        Vector2 entityPos = {entity.pos.x - player.pos.x, entity.pos.y - player.pos.y};
        
        //TODO Write matrix transform function and matrix struct etc.
        {
            Vector2 rotatedPos;
            rotatedPos.x = entityPos.x * cosf(player.rotation) + entityPos.y * sinf(player.rotation);
            rotatedPos.y = entityPos.x * -sinf(player.rotation) + entityPos.y * cosf(player.rotation);
            entityPos = rotatedPos;
        }

        float projW = 2 * (entityPos.x * tan(H_FOV/2));
        float projH = 2 * (entityPos.x * tan(V_FOV/2));
        float projWRatio = projW == 0 ? 1 : (pixelBuffer.width / projW);
        float projHRatio = projH == 0 ? 1 : (pixelBuffer.height / projH);

        float scaledSpriteW = projWRatio * entity.base->sprite->w;
        float scaledSpriteH = projHRatio * entity.base->sprite->h;
        int scaledSpriteX = projWRatio * entityPos.y + pixelBuffer.width / 2 - scaledSpriteW/2;
        int scaledSpriteY = pixelBuffer.height / 2 - scaledSpriteH/2;

        for (int x = scaledSpriteX; x < scaledSpriteX + scaledSpriteW; x++) 
        {
            if (x >= pixelBuffer.width) break;
            if (x < 0 || zBuffer[x] < entityPos.x) continue;

            for (int y = scaledSpriteY; y < scaledSpriteY + scaledSpriteH; y++)
            {    
                if (y < 0) y = 0;
                if (y >= pixelBuffer.height) break;

                int spriteIndexX = ((float)(x - scaledSpriteX) / scaledSpriteW) * entity.base->sprite->w;
                int spriteIndexY = ((float)(y - scaledSpriteY) / scaledSpriteH) * entity.base->sprite->h;

                //Super basic alpha transparency
                uint32_t pixelColor = ((uint32_t*)entity.base->sprite->pixels)[spriteIndexY * entity.base->sprite->w + spriteIndexX];
                if (pixelColor & 0xFF000000)
                {
                    pixelBuffer.pixels[y * pixelBuffer.width + x] = ((uint32_t*)entity.base->sprite->pixels)[spriteIndexY * entity.base->sprite->w + spriteIndexX];
                }
            }
        }
    }
}
