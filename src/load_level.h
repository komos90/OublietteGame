/*
 Level loading code for 3d Software Renderer
 Seoras Macdonald
 seoras1@gmail.com
 2015
 */
#pragma once

#include <stdbool.h>

#include "engine_types.h"

static const int TILE_DIMS = 64;

static const char TILE_PLAYER_START = 'p';
static const char TILE_FLOOR        = '.';
static const char TILE_RUBY         = '*';
static const char TILE_MONSTER      = '!';
static const char TILE_LEVEL_END    = 'g';
static const char TILE_WALL         = '#';
static const char TILE_KEY0         = '1';
static const char TILE_KEY1         = '2';
static const char TILE_KEY2         = '3';
static const char TILE_KEY3         = '4';
static const char TILE_DOOR0        = 'q';
static const char TILE_DOOR1        = 'w';
static const char TILE_DOOR2        = 'e';
static const char TILE_DOOR3        = 'r';
static const char TILE_SECRET_DOOR  = '?';

typedef struct
{
    int width;
    int height;
    char* data;

    int rubyCount;
} Level;

bool isTileIndexValid(int i);
void loadLevel(char* fileName);
int posVecToTileIndex(Vector2 pos);
int posVecToIndex(Vector2 pos);
int posToTileIndex(int x, int y);
Vector2 posToTileCoord(Vector2 pos);
Vector2 getPlayerStartPos();
EntityArray getLevelRubies(EntityTemplate* rubyTemplate);
EntityArray getLevelKeys(EntityTemplate* keyTemplate);
bool isTileSolid(int index);
void setTileTo(int index, char tile);
int getTotalLevelRubies();
char getLevelTile();
SDL_Surface* getTileTexture(int index);