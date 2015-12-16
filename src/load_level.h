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
static const char TILE_INSTRUCTIONS1= 'i';
static const char TILE_INSTRUCTIONS2= 'j';
static const char TILE_INSTRUCTIONS3= 'k';
static const char TILE_INSTRUCTIONS4= 'l';

static const int LEVEL_FILE_PATH_MAX_LEN = 32;

typedef struct
{
    int width;
    int height;
    char* data;

    int rubyCount;
} Level;

bool isTileIndexValid(int i);
void loadLevelTiles(char* fileName);
int posVecToTileIndex(Vector2 pos);
int posVecToIndex(Vector2 pos);
int posToTileIndex(int x, int y);
int coordToTileIndex(int x, int y);
Vector2 posToTileCoord(Vector2 pos);
Vector2 getPlayerStartPos(void);
EntityArray getLevelRubies(EntityTemplate* rubyTemplate);
EntityArray getLevelKeys(EntityTemplate* keyTemplate);
EntityArray getLevelMonsters(EntityTemplate* monsterTemplate, int levelNumber);
bool isTileSolid(int index);
void setTileTo(int index, char tile);
int getTotalLevelRubies(void);
char getLevelTile(int index);
SDL_Surface* getTileTexture(int index);
bool fileExists(char* filePath);
Vector2 getLevelEndPos();