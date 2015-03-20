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

static const char PLAYER_START  = 'p';
static const char FLOOR         = '.';
static const char RUBY          = '*';
static const char MONSTER       = '!';
static const char LEVEL_END     = 'e';
static const char WALL          = '#';
static const char KEY1          = '1';
static const char DOOR1         = 'q';
static const char SECRET_DOOR   = '?'; 

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