/*
Raycaster wolfenstein 3d style game.
Seoras Macdonald
seoras1@gmail.com
2015
*/
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

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

uint32_t getPixel(SDL_Surface* image, int x, int y) {
    return ((uint32_t*)image->pixels)[y * image->w + x];
}

//Can add srcRect later for animation
void blitToPixelBuffer(SDL_Surface* image, Rectangle destRect, uint32_t maskColor)
{
    //Modifying destRect to fit inside screen
    if (destRect.x < 0)
    {
        destRect.w += destRect.x;
        destRect.x = 0;
    }
    else if (destRect.x >= pixelBuffer.width)
    {
        destRect.w += (pixelBuffer.width - 1) - destRect.x;
        destRect.x = pixelBuffer.width - 1;
    }
    if (destRect.y < 0)
    {
        destRect.h += destRect.y;
        destRect.y = 0;
    }
    else if (destRect.y >= pixelBuffer.height)
    {
        destRect.h += (pixelBuffer.height - 1) - destRect.y;
        destRect.y = pixelBuffer.height - 1;
    }

    for (int y = 0; y < destRect.h; y++)
    {
        for (int x = 0; x < destRect.w; x++)
        {
            int texCoordX = x * (image->w / (float)destRect.w);
            int texCoordY = y * (image->h / (float)destRect.h);
            uint32_t color = getPixel(image, texCoordX, texCoordY);
            //if color not fully transparent
            if (color & 0xFF000000)
            {
                if (color == 0xFFFF00FF)
                {
                    color = maskColor;
                }
                drawPoint(destRect.x + x, destRect.y + y, color);
            }
        }
    }
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
                uint32_t pixelColor = getPixel(spriteFont.sprite, x + spriteX, y) & color;
                if (pixelColor & 0xFF000000) {
                    drawPoint(rect.x + i * spriteFont.charW + x, rect.y + y, pixelColor);
                }
            }
        }
    }
}

void createPixelBuffer(int width, int height)
{
    //MALLOC no need to free, kept until program end.
    uint32_t* pixels = (uint32_t*)malloc(width * height * sizeof(uint32_t));
    zBuffer = (float*)malloc(width * sizeof(float));
    pixelBuffer.pixels = pixels;
    pixelBuffer.width = width;
    pixelBuffer.height = height;
}

PixelBuffer* getPixelBuffer()
{
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
    float screenViewHeight = 2 * distance * tanf(V_FOV/2.f);
    float displayHeight = TILE_DIMS * (pixelBuffer.height / screenViewHeight);
    return displayHeight;
}

float getWallIntersectionData(Vector2Int* intersectPos, int* intersectTileIndex, const Player* player, float angle)
{
    int hTileIndex;
    int vTileIndex;
    Vector2Int hIntersect = getTileHorzIntersection(player->pos, angle, &hTileIndex);
    float hDistance = sqrt(pow(hIntersect.x - player->pos.x, 2) + pow(hIntersect.y - player->pos.y, 2)) * cosf(angle - player->rotation);

    Vector2Int vIntersect = getTileVertIntersection(player->pos, angle, &vTileIndex);
    float vDistance = sqrt(pow(vIntersect.x - player->pos.x, 2) + pow(vIntersect.y - player->pos.y, 2)) * cosf(angle - player->rotation);

    *intersectPos = hDistance <= vDistance ? hIntersect : vIntersect;
    *intersectTileIndex = hDistance <= vDistance ? hTileIndex : vTileIndex;
    return hDistance <= vDistance ? hDistance : vDistance;
}

uint32_t depthShading(uint32_t inColor, float distance)
{
    float intensity = (0.5 / distance) * 150;
    if (intensity >= 1.0) return inColor;

    uint8_t outColor8[3];
    for (int i = 0; i < 3; i++)
    {
        outColor8[i] = (uint8_t)(inColor >> i * 8) * intensity;
    }
    return (outColor8[2] << 16) | (outColor8[1] << 8) | outColor8[0];
}

void drawFloorCeiling(int y, int screenColumn, float angle, Player* player, int playerHeight, SDL_Surface* texture) {
    float distance = (TILE_DIMS - playerHeight) / fabs(tanf((y - pixelBuffer.height/2) * (V_FOV / pixelBuffer.height)));

    int texX = (int)((cosf(angle) * distance) * (1/cosf(angle - player->rotation)) + player->pos.x) % TILE_DIMS;
    int texY = (int)((sinf(angle) * distance) * (1/cosf(angle - player->rotation)) + player->pos.y) % TILE_DIMS;

    //Should be able to scale texture
    uint32_t color = getPixel(texture, texX, texY);

    //Depth shading
    color = depthShading(color, distance);
    drawPoint(screenColumn, y, color);
}

void draw(Player player, EntityArray entities)
{
    float angle = -H_FOV/2 + player.rotation;
    for (int screenColumn = 0; screenColumn < pixelBuffer.width; screenColumn++)
    {
        //Get distance and intersect pos
        Vector2Int intersectPos;
        int intersectTileIndex;
        float distance = getWallIntersectionData(&intersectPos, &intersectTileIndex, &player, angle);
        float height = wallDistanceToHeight(distance);

        //Save column distance in zBuffer
        zBuffer[screenColumn] = distance;

        int playerHeight = TILE_DIMS / 2;  //Should be stored somewhere else and probably used in other calculations
        //This should be extracted into a function
        int y = 0;
        //Ceiling
        for (; y < (pixelBuffer.height - height) / 2; y++)
        {
            drawFloorCeiling(y, screenColumn, angle, &player, playerHeight, images.ceilingTexture);
        }
        //walls
        for (; y < (pixelBuffer.height + height) / 2; y++)
        {
            //Range corrections
            if (y >= pixelBuffer.height) break;

            //Texture Mapping
            int yTexCoord = ((TILE_DIMS / (height)) * (y - (pixelBuffer.height - height) / 2));
            int xTexCoord = intersectPos.x % TILE_DIMS + intersectPos.y % TILE_DIMS;
            uint32_t color32 = getPixel(getTileTexture(intersectTileIndex), xTexCoord, yTexCoord);

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
            color32 = depthShading(color32, distance);
            drawPoint(screenColumn, y, color32);
        }
        //Draw floor
        for (; y < pixelBuffer.height; y++)
        {
            drawFloorCeiling(y, screenColumn, angle, &player, playerHeight, images.floorTexture);
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

    for (int entityIndex = 0; entityIndex < entities.size; entityIndex++)
    {
        Entity entity = entities.data[entityIndex];
        Vector2 entityPos = {entity.pos.x - player.pos.x, entity.pos.y - player.pos.y};
        
        {
            Vector2 rotatedPos;
            rotatedPos.x = (entityPos.x * cosf(player.rotation) + entityPos.y * sinf(player.rotation));
            rotatedPos.y = (entityPos.x * -sinf(player.rotation) + entityPos.y * cosf(player.rotation));
            entityPos = rotatedPos;
        }

        float projW = 2 * (entityPos.x * tan(H_FOV/2));
        float projH = 2 * (entityPos.x * tan(V_FOV/2));
        float projWRatio = projW == 0 ? 1 : (pixelBuffer.width / projW);
        float projHRatio = projH == 0 ? 1 : (pixelBuffer.height / projH);

        float scaledSpriteW = projWRatio * entity.base->spriteWidth;
        float scaledSpriteH = projHRatio * entity.base->spriteHeight;
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

                int spriteIndexX = ((float)(x - scaledSpriteX) / scaledSpriteW) * entity.base->spriteWidth + entity.base->spriteWidth * entity.xClip;
                int spriteIndexY = ((float)(y - scaledSpriteY) / scaledSpriteH) * entity.base->spriteHeight + entity.base->spriteHeight * entity.yClip;

                //Super basic alpha transparency
                uint32_t pixelColor = getPixel(entity.base->sprite, spriteIndexX, spriteIndexY);
                if (pixelColor & 0xFF000000)
                {
                    if (pixelColor == 0xFFFF00FF && entity.base->type == ENTITY_TYPE_KEY)
                    {
                        pixelColor = keyColors[((Key*)entity.sub)->id];
                    }

                    //Not sure if this is in sync with texture shading
                    uint32_t finalColor32 = depthShading(pixelColor, entityPos.x);

                    pixelBuffer.pixels[y * pixelBuffer.width + x] = finalColor32;
                }
            }
        }
    }
}
