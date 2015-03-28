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
#include "images.h"


static uint32_t keyColors[MAX_KEYS] = {0xFFFF0000, 0xFF00FF00, 0xFF0000FF, 0xFF00AA88};

static PixelBuffer pixelBuffer = {0};
static float* zBuffer = NULL;


void drawRect(Rectangle rect, uint32_t color)
{
    //screen boundary checks
    if (rect.x >= pixelBuffer.width || rect.y >= pixelBuffer.height) return;
    if (rect.x + rect.w >= pixelBuffer.width) rect.w -= rect.x + rect.w - pixelBuffer.width;
    if (rect.y + rect.h >= pixelBuffer.height) rect.h -= rect.y + rect.h - pixelBuffer.height;  
    if (rect.y < 0)
    {
        rect.h += rect.y;
        rect.y = 0;
    }
    if (rect.x < 0)
    {
        rect.w += rect.x;
        rect.x = 0;
    }

    for (int y = rect.y; y < rect.y + rect.h; ++y)
    {
        for (int x = rect.x; x < rect.x + rect.w; ++x)
        {
            pixelBuffer.pixels[y * pixelBuffer.width + x] = color;
        }
    }
}

void drawPoint(int x, int y, uint32_t color)
{
    pixelBuffer.pixels[y  * pixelBuffer.width + x] = color;
}

void drawText(char* text, SDL_Rect rect, uint32_t color, SpriteFont spriteFont)
{
    //get text length
    int textLength = 0;
    for (textLength = 0; text[textLength] != '\0'; textLength++);

    for (int i = 0; i < textLength; i++)
    {
        int spriteX = (text[i] - 32) * spriteFont.charW;

        for (int y = 0; y < spriteFont.charH; y++)
        {
            for (int x = 0; x < spriteFont.charW; x++)
            {
                uint32_t pixelColor = ((uint32_t*)spriteFont.sprite->pixels)[y * spriteFont.sprite->w + x + spriteX] & color;
                if (pixelColor & 0xFF000000) {
                    pixelBuffer.pixels[(rect.y + y) * pixelBuffer.width + (rect.x + i * spriteFont.charW + x)] = pixelColor;
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

Vector2Int getTileHorzIntersection(Vector2 pos, float angle, int* tileIndex)
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
    while (isTileIndexValid(posToTileIndex(xInter + xDir, yInter + yDir)) &&
           !isTileSolid(posToTileIndex(xInter + xDir, yInter + yDir)))
    {   
        yInter += yPositive ? -TILE_DIMS : TILE_DIMS;
        xInter += xInc;
    }
    *tileIndex = posToTileIndex(xInter + xDir, yInter + yDir);
    Vector2Int intersectVector = { .x=xInter, .y=yInter };
    return intersectVector;
}

Vector2Int getTileVertIntersection(Vector2 pos, float angle, int* tileIndex)
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
    while (isTileIndexValid(posToTileIndex(xInter + xDir, yInter + yDir)) &&
           !isTileSolid(posToTileIndex(xInter + xDir, yInter + yDir)))
    {   
        xInter += xPositive ? TILE_DIMS : -TILE_DIMS;
        yInter += yInc;
    }
    *tileIndex = posToTileIndex(xInter + xDir, yInter + yDir);
    Vector2Int intersectVector = { .x=xInter, .y=yInter };
    return intersectVector;
}

float wallDistanceToHeight(float distance)
{
    float screenViewHeight = distance * tanf(V_FOV/2.f);
    float displayHeight = TILE_DIMS * (pixelBuffer.height / screenViewHeight);
    return displayHeight;
}

void draw(Player player, EntityArray entities)
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
        int intersectTileIndex;
        {
            int hTileIndex;
            int vTileIndex;
            Vector2Int hIntersect = getTileHorzIntersection(player.pos, angle, &hTileIndex);
            float hDistance = sqrt(pow(hIntersect.x - player.pos.x, 2) + pow(hIntersect.y - player.pos.y, 2)) * cos(angle - player.rotation);

            Vector2Int vIntersect = getTileVertIntersection(player.pos, angle, &vTileIndex);
            float vDistance = sqrt(pow(vIntersect.x - player.pos.x, 2) + pow(vIntersect.y - player.pos.y, 2)) * cos(angle - player.rotation);

            intersectPos = hDistance <= vDistance ? hIntersect : vIntersect;
            intersectTileIndex = hDistance <= vDistance ? hTileIndex : vTileIndex;
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
                
                SDL_Surface* tileTexture = getTileTexture(intersectTileIndex);

                uint32_t* texPixels = (uint32_t*)(tileTexture->pixels);
                int index = yTexCoord * tileTexture->w + xTexCoord;
                color32 = texPixels[index];
            }
            //ColorKey for doors
            if (color32 == 0xFFFF00FF)
            {
                char tile = getLevelTile(intersectTileIndex);
                if (tile == TILE_DOOR0) color32 = keyColors[0];
                else if (tile == TILE_DOOR1) color32 = keyColors[1];
                else if (tile == TILE_DOOR2) color32 = keyColors[2];
                else if (tile == TILE_DOOR3) color32 = keyColors[3];
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
        
        //Change from pos at centre to pos at top left corner
        //Don't think this is needed!!
        //entityPos.x -= entity.base->sprite->w / 2;
        //entityPos.y -= entity.base->sprite->h / 2;

        // TODO Write matrix transform function and matrix struct etc.
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
        int scaledSpriteY = pixelBuffer.height / 2 - scaledSpriteH/2 - entity.zPos * projHRatio;

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
                    //inefficient
                    if (pixelColor == 0xFFFF00FF && entity.base->type == ENTITY_TYPE_KEY)
                    {
                        pixelColor = keyColors[((Key*)entity.sub)->id];
                    }
                    pixelBuffer.pixels[y * pixelBuffer.width + x] = pixelColor;
                }
            }
        }
    }
}
