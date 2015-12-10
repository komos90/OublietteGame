/*
Raycaster wolfenstein 3d style game.
Seoras Macdonald
seoras1@gmail.com
2015
*/
#pragma once

#ifdef __linux__
    #include <SDL2/SDL.h>
    #include <SDL2/SDL_image.h>
    #include <SDL2/SDL_mixer.h>
#elif _WIN32
    #include <SDL.h>
    #include <SDL_image.h>
    #include <SDL_mixer.h>
#endif

#include "engine_types.h"
#include "linked_list.h"

typedef enum
{
    DIR_NONE,
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT,
} Direction;

typedef enum
{
    AI_PATROL,
    AI_CHASE
} AIMode;

typedef struct
{
    Direction direction;
    Vector2Int* patrolPoints;
    int patrolLength;
    int patrolIndex;
    Vector2Int targetTile;
    LinkedList pathList;
    AIMode aiState;
    CountdownTimer giveUpChaseTimer;
    int roarSoundChannel;
} Monster;

float getMonsterAngle(Entity* this);
void monsterMove(Entity* this);
void monsterMoveAStar(Entity* this);