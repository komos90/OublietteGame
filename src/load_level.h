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

Level loadLevel(char* fileName);
int posToTileIndex(int x, int y, Level level);
Vector2 getPlayerStartPos(Level level);
EntityArray getLevelRubies(Level level, EntityTemplate* rubyTemplate);
bool isTileSolid(int index, Level level);