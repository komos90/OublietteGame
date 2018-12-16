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
#include "monster.h"


static uint32_t keyColors[MAX_KEYS] = {0xFFFF0000, 0xFF00FF00, 0xFF0000FF, 0xFF00AA88};

static PixelBuffer pixelBuffer = {0};
static float* zBuffer = NULL;

/*---------------------
 * Defines
 *-------------------*/
#define PLAYER_HEIGHT (TILE_DIMS / 2)


/*---------------------
 * Precomputed tangents
 *-------------------*/
static float tanHFovOver2;
static float tanVFovOver2;

/*---------------------------------
 * Precompute floorCeilingDistances
 *-------------------------------*/
static float *floorCeilingDistanceTable;

/*------------------------------------------------------------------------------
 * Input:
 *      int x: The x coordinate to draw point.
 *      int y: The y coordinate to draw point.
 *      uint32_t color: the colour the point should be draw.
 * Description:
 *      Draws a single point to the main pixel buffer
 *----------------------------------------------------------------------------*/
void drawPoint(int x, int y, uint32_t color)
{
    pixelBuffer.pixels[y  * pixelBuffer.width + x] = color;
}

/*------------------------------------------------------------------------------
 * Input: A rectangle that is to be drawn to the pixel buffer, and the colour it
 *        should be drawn in.
 * Description: Draws a filled rectangle, rect, of a solid colour, color, to the
 *              main pixel buffer.
 *----------------------------------------------------------------------------*/
void drawRect(Rectangle rect, uint32_t color)
{
    ////Check and adjust the rect if it crosses screen boundaries.
    //If the left side of rect > the screen width OR the top of the rect is
    //greater than the screen height THEN return
    if (rect.x >= pixelBuffer.width || rect.y >= pixelBuffer.height) return;
    //If the right side of the rect >= the screen width THEN reduce the width
    //of the rect acordingly.
    if (rect.x + rect.w >= pixelBuffer.width) rect.w -= rect.x + rect.w - pixelBuffer.width;
    //Same as above but for bottom of rect and screen height
    if (rect.y + rect.h >= pixelBuffer.height) rect.h -= rect.y + rect.h - pixelBuffer.height;
    //Cut off part of rect above the screen.
    if (rect.y < 0)
    {
        rect.h += rect.y;
        rect.y = 0;
    }
    //Cut off part of rect left of the screen.
    if (rect.x < 0)
    {
        rect.w += rect.x;
        rect.x = 0;
    }

    //Draw a point for every pixel in the rectangle in the specified color
    for (int y = rect.y; y < rect.y + rect.h; ++y)
    {
        for (int x = rect.x; x < rect.x + rect.w; ++x)
        {
            drawPoint(x, y, color);
        }
    }
}

/*------------------------------------------------------------------------------
 * Input:
 *      SDL_Surface* image: The image to retrive the pixel from
 *      int x: The x coordinate of the pixel you want the colour of.
 *      int y: The y coordinate of the pixel you want the colour of.
 * Description:
 *      Gets the colour of a specified pixel from a specified image.
 * Output:
 *      uint32 The colour of the pixel at (x,y) in image.
 *----------------------------------------------------------------------------*/
uint32_t getPixel(SDL_Surface* image, int x, int y) {
    return ((uint32_t*)image->pixels)[y * image->w + x];
}

uint32_t getPixelBufferPixel(int x, int y) {
    return pixelBuffer.pixels[y * pixelBuffer.width + x];
}

/*------------------------------------------------------------------------------
 * Input:
 *      SDL_Surface* image: The image to be drawn
 *      SDL_Rect destRect: The rectangle on the screen to draw the image
 *      uint32_t maskColor: The colour to replace 0xFFFF00FF with.
 * Description:
 *      Draws an image to the screen, scaling as neccessary, to the rectangle
 *      destRect. The colour 0xFFFF00FF (bright pink) is replaced with the
 *      colour maskColor.
 * To Do:
 *      Add srcRect parameter to allow the use of sprite sheets
 *----------------------------------------------------------------------------*/
void blitToPixelBuffer(SDL_Surface* image, Rectangle destRect, uint32_t maskColor)
{
    ////Keep destRect inside screen boundaries
    //IF the left side of the rectangle < left side of screen THEN
    //shift the rectangle to the right
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

void rotatedBlitToPixelBuffer(SDL_Surface* image, Rectangle destRect, uint32_t maskColor, float angle)
{
    ////Keep destRect inside screen boundaries
    //IF the left side of the rectangle < left side of screen THEN
    //shift the rectangle to the right
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

    float cosTheta = cosf(angle);
    float sinTheta = sinf(angle);
    for (int y = -destRect.h / 2; y < destRect.h / 2; y++)
    {
        for (int x = -destRect.w / 2; x < destRect.w / 2; x++)
        {
            int texCoordX = ((x * cosTheta + y * sinTheta) + destRect.w / 2) * (image->w / (float)destRect.w);
            int texCoordY = ((- x * sinTheta + y * cosTheta) + destRect.h / 2) * (image->h / (float)destRect.h);
            uint32_t color = 0x00000000;
            if (texCoordX >= 0 && texCoordX < image->w &&
                texCoordY >= 0 && texCoordY < image->h)
            {
                color = getPixel(image, texCoordX, texCoordY);
            }
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

void drawText(char* text, SDL_Rect rect, uint32_t color, SpriteFont spriteFont, bool centered)
{
    //get text length
    int textLength = 0;
    for (textLength = 0; text[textLength] != '\0'; textLength++);

    if (centered)
    {
        rect.x -= (int)((textLength / 2.0) * spriteFont.charW);
        rect.y -= spriteFont.charH / 2;
    }

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

void drawTextToSurface(char* text, SDL_Surface* surface, SDL_Rect rect, uint32_t color, SpriteFont spriteFont)
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
                    ((uint32_t*)surface->pixels)[(rect.y + y) * surface->w + rect.x + i * spriteFont.charW + x ] = pixelColor;
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

    //Init precomputed trig functions
    tanHFovOver2 = tanf(H_FOV/2.f);
    tanVFovOver2 = tanf(V_FOV/2.f);

    // Precompute distances for floor / ceiling rendering.
    // MALLOC: No free, used for duration of program
    floorCeilingDistanceTable = malloc(sizeof(float) * height);
    for (int i = 0; i < SCREEN_HEIGHT; i++) {
        floorCeilingDistanceTable[i] = (TILE_DIMS - PLAYER_HEIGHT) / fabs(tanf((i - pixelBuffer.height/2) * (V_FOV / pixelBuffer.height)));
    }
}

PixelBuffer* getPixelBuffer(void)
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
    float tanXAngle = tanf(xAngle);
    float xInter = (fabsf(yInter - pos.y) * tanXAngle) + pos.x;
    float xInc = TILE_DIMS * tanXAngle;

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
    float tanYAngle = tanf(yAngle);
    float yInter = (fabsf(xInter - pos.x) * tanYAngle) + pos.y;
    float yInc = TILE_DIMS * tanYAngle;

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
    float screenViewHeight = 2 * distance * tanVFovOver2;
    float displayHeight = TILE_DIMS * (pixelBuffer.height / screenViewHeight);
    return displayHeight;
}

float getWallIntersectionData(Vector2Int* intersectPos, int* intersectTileIndex, const Player* player, float angle, float cosScreenAngle)
{
    int hTileIndex;
    int vTileIndex;
    Vector2Int hIntersect = getTileHorzIntersection(player->pos, angle, &hTileIndex);
    float hDistance = sqrt(pow(hIntersect.x - player->pos.x, 2) + pow(hIntersect.y - player->pos.y, 2)) * cosScreenAngle;

    Vector2Int vIntersect = getTileVertIntersection(player->pos, angle, &vTileIndex);
    float vDistance = sqrt(pow(vIntersect.x - player->pos.x, 2) + pow(vIntersect.y - player->pos.y, 2)) * cosScreenAngle;

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

void drawFloorCeiling(int y, int screenColumn, float sinAngle, float cosAngle, float cosScreenAngle, Player* player, SDL_Surface* texture) {
    float distance = floorCeilingDistanceTable[y];

    int texX = (int)((cosAngle * distance) * (1/cosScreenAngle) + player->pos.x) % TILE_DIMS;
    int texY = (int)((sinAngle * distance) * (1/cosScreenAngle) + player->pos.y) % TILE_DIMS;

    //Should be able to scale texture
    uint32_t color = getPixel(texture, texX, texY);

    //Depth shading
    color = depthShading(color, distance);
    drawPoint(screenColumn, y, color);
}

void sortSprites(EntityArray* entities, Vector2 playerPos)
{
    for (int i = entities->size - 1; i >= 0; i--)
    {
        for (int j = 0; j < i; j++)
        {
            if (distanceFormula(entities->data[j].pos, playerPos) <
                distanceFormula(entities->data[j + 1].pos, playerPos))
            {
                //Swap
                Entity tmp = entities->data[j];
                entities->data[j] = entities->data[j + 1];
                entities->data[j + 1] = tmp;
            }
        }
    }
}

void pixelateScreen(int n) {
    for (int y = 0; y < pixelBuffer.height; y += n)
    {
        for (int x = 0; x < pixelBuffer.width; x += n)
        {
            SDL_Rect tmpRect = {x, y, n, n};
            drawRect(tmpRect, getPixelBufferPixel(x, y));
        }
    }
}

void fadeToColor(uint32_t addColor, float ratio)
{
    for (int y = 0; y < pixelBuffer.height; y += 1)
    {
        for (int x = 0; x < pixelBuffer.width; x += 1)
        {
            uint32_t inColor = getPixelBufferPixel(x, y);

            uint8_t outColor8[3];
            for (int i = 0; i < 3; i++)
            {
                outColor8[i] = (uint8_t)(ratio * (uint8_t)(addColor >> i * 8) + (1.f - ratio) * (uint8_t)(inColor >> i * 8));
                //SDL_Log("ratio %f, addColori %u, inColori %u", ratio, (addColor >> i * 8), (inColor >> i * 8));
            }
            drawPoint(x, y, (outColor8[2] << 16) | (outColor8[1] << 8) | outColor8[0]);
        }
    }

}

void draw(Player player, EntityArray entities)
{
    //player rotation is now fixed, so precomputing trig functions to speed up
    const float sinPlayerAngle = sinf(player.rotation);
    const float cosPlayerAngle = cosf(player.rotation);

    {
        float angle = -H_FOV/2 + player.rotation;
        for (int screenColumn = 0; screenColumn < pixelBuffer.width; screenColumn++)
        {
            //angle, and player rotation are now fixed, so precomputing to speed up
            const float sinAngle = sinf(angle);
            const float cosAngle = cosf(angle);
            const float cosScreenAngle = cosf(angle - player.rotation);

            //Get distance and intersect pos
            Vector2Int intersectPos;
            int intersectTileIndex;
            float distance = getWallIntersectionData(&intersectPos, &intersectTileIndex, &player, angle, cosScreenAngle);
            float height = wallDistanceToHeight(distance);
            SDL_Surface* tileTexture = getTileTexture(intersectTileIndex);

            //Save column distance in zBuffer
            zBuffer[screenColumn] = distance;

            //This should be extracted into a function
            int y = 0;
            //Ceiling
            for (; y < (pixelBuffer.height - height) / 2; y++)
            {
                drawFloorCeiling(y, screenColumn, sinAngle, cosAngle, cosScreenAngle, &player, images.ceilingTexture);
            }
            //walls
            for (; y < (pixelBuffer.height + height) / 2; y++)
            {
                //Range corrections
                if (y >= pixelBuffer.height) break;

                //Texture Mapping
                int yTexCoord = ((TILE_DIMS / (height)) * (y - (pixelBuffer.height - height) / 2));
                int xTexCoord = intersectPos.x % TILE_DIMS + intersectPos.y % TILE_DIMS;
                uint32_t color32 = getPixel(tileTexture, xTexCoord, yTexCoord);

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
                drawFloorCeiling(y, screenColumn, sinAngle, cosAngle, cosScreenAngle, &player, images.floorTexture);
            }
            angle += (H_FOV / pixelBuffer.width);
        }
    }

    //Sprite drawing ====
    //Sort sprites by distatnce
    sortSprites(&entities, player.pos);

    for (int entityIndex = 0; entityIndex < entities.size; entityIndex++)
    {
        Entity entity = entities.data[entityIndex];
        Vector2 entityPos = {entity.pos.x - player.pos.x, entity.pos.y - player.pos.y};

        {
            Vector2 rotatedPos;
            rotatedPos.x = (entityPos.x * cosPlayerAngle + entityPos.y * sinPlayerAngle);
            rotatedPos.y = (entityPos.x * -sinPlayerAngle + entityPos.y * cosPlayerAngle);
            entityPos = rotatedPos;
        }

        float projW = 2 * (entityPos.x * tanHFovOver2);
        float projH = 2 * (entityPos.x * tanVFovOver2);
        float projWRatio = projW == 0 ? 1 : (pixelBuffer.width / projW);
        float projHRatio = projH == 0 ? 1 : (pixelBuffer.height / projH);

        float scaledSpriteW = projWRatio * entity.base->spriteWidth;
        float scaledSpriteH = projHRatio * entity.base->spriteHeight;
        int scaledSpriteX = projWRatio * entityPos.y + pixelBuffer.width / 2 - scaledSpriteW/2;
        int scaledSpriteY = pixelBuffer.height / 2 - scaledSpriteH/2 - entity.zPos * projHRatio;

        if (entity.base->type == ENTITY_TYPE_MONSTER)
        {
            float monsterAngle = constrainAngle(player.rotation - getMonsterAngle(&entity));

            if (monsterAngle <= -3*M_PI/4 || monsterAngle >= 3*M_PI/4)
            {
                entity.yClip = 0;
            }
            if (monsterAngle >= -M_PI/4 && monsterAngle <= M_PI/4)
            {
                entity.yClip = 1;
            }
            if (monsterAngle >= M_PI/4 && monsterAngle <= 3*M_PI/4)
            {
                entity.yClip = 2;
            }
            if (monsterAngle <= -M_PI/4 && monsterAngle >= -3*M_PI/4)
            {
                entity.yClip = 3;
            }
        }

        for (int x = scaledSpriteX; x < scaledSpriteX + scaledSpriteW; x++)
        {
            if (x >= pixelBuffer.width) break;
            //SDL_Log("%f < %f", zBuffer[x], entityPos.x);
            float angle = -H_FOV/2 + player.rotation + (H_FOV / pixelBuffer.width) * x;
            float spriteDistance = sqrt(pow(entityPos.x, 2) + pow(entityPos.y, 2)) * cosf(angle - player.rotation);
            if (x < 0 || zBuffer[x] < spriteDistance) continue;

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
                    uint32_t finalColor32 = depthShading(pixelColor, spriteDistance);

                    pixelBuffer.pixels[y * pixelBuffer.width + x] = finalColor32;
                }
            }
        }
    }
}