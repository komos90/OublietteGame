#include <float.h>
#include <math.h>
#include <stdlib.h>

#include "monster.h"
#include "engine_types.h"
#include "load_level.h"
#include "linked_list.h"

float getMonsterAngle(Entity* this)
{
    float monsterAngle = 0.f;
    switch(((Monster*)this->sub)->direction)
    {
    case DIR_DOWN:
        monsterAngle = M_PI/2;
        break;
    case DIR_LEFT:
        monsterAngle = M_PI;
        break;
    case DIR_UP:
        monsterAngle = -M_PI/2;
        break;
    }
    return monsterAngle;
}

void monsterMove(Entity* this)
{

    //Should probably check entity is actually a monster
    Monster* monster = (Monster*)this->sub;
    if (monster->pathList.front == NULL)
    {
        SDL_Log("null1");
        monsterMoveAStar(this);
        if (monster->pathList.front == NULL)
        {
            SDL_Log("null2");
            return;
        }
    }

    //SDL_Log("Monster move");
    Vector2Int targetTile = {monster->pathList.front->tile.x, monster->pathList.front->tile.y};
    for (ListNode* current = ((Monster*)this->sub)->pathList.front;
         current != NULL;
         current = current->next)
    {
        SDL_Log("x: %d, y: %d", current->tile.x, current->tile.y);
    }
    
    Vector2 dirOffsets[5] = { {0}, { .y=-1 }, { .y=1 }, { .x=-1 }, { .x=1 } };

    //Store old center to check whether center of tile was crossed.
    Vector2Int oldTileByCenter = { 
        .x=this->pos.x / TILE_DIMS + 0.5,
        .y=this->pos.y / TILE_DIMS + 0.5 };

    //Move in current direction
    float monsterVel = 2.f; 
    this->pos.x += monsterVel * dirOffsets[((Monster*)this->sub)->direction].x;
    this->pos.y += monsterVel * dirOffsets[((Monster*)this->sub)->direction].y;

    //Did monster cross tile centre?
    Vector2Int newTileByCenter = { 
        .x=this->pos.x / TILE_DIMS + 0.5,
        .y=this->pos.y / TILE_DIMS + 0.5 };
    bool crossedCenter = oldTileByCenter.x != newTileByCenter.x || oldTileByCenter.y != newTileByCenter.y;

    if (((Monster*)this->sub)->direction == DIR_NONE || crossedCenter)
    {
        Vector2 curTile = posToTileCoord(this->pos);
        Vector2 minTile = curTile;
        float minDistance = FLT_MAX; 

        for (int i = DIR_UP; i <= DIR_RIGHT; i++)
        {
            Vector2 consideringTile = {
                .x=curTile.x + dirOffsets[i].x,
                .y=curTile.y + dirOffsets[i].y };
            if (isTileSolid(posVecToIndex(consideringTile)))
            {
                continue;
            }

            float distance = sqrt(pow(targetTile.x - consideringTile.x, 2) + pow(targetTile.y - consideringTile.y, 2));
            if (distance < minDistance)
            {
                minDistance = distance;
                minTile = consideringTile;
                ((Monster*)this->sub)->direction = i; //THIS IS A BIT DANGEROUS
            }
        }
    }
    
    if ((int)(this->pos.x / TILE_DIMS) == targetTile.x &&
        (int)(this->pos.y / TILE_DIMS) == targetTile.y)
    {
        
        linkedListRemoveFront(&monster->pathList);
        monsterMoveAStar(this);
    }

}

bool isValidTileForPath(int x, int y)
{
    int tileIndex = posToTileIndex(x, y);
    if (isTileIndexValid(tileIndex) && !isTileSolid(tileIndex)) return true;
    return false;
}

float generateHeuristic(PathTile pathTile, Vector2Int target, Entity* monster)
{
    float f = abs(pathTile.x - monster->pos.x) + abs(pathTile.y - monster->pos.y);
    float g = abs(pathTile.x - target.x) + abs(pathTile.y - target.y);
    return f + 2 * g;
}

bool reachedPlayer(ListNode* current, Vector2Int target)
{
    //SDL_Log("playerTile: %f, %f", playerTile.x + 1, playerTile.y + 1);
    return (current->tile.x == target.x &&
            current->tile.y == target.y);
}

void findPath() {

}

void monsterMoveAStar(Entity* this)
{
    //NOTE:
    //  + Monster struct should have Vector2Int targetTile. pathfinding should use this target.
    //  Changing from patrol to chasing player and back should be done outside of the pathfinding function, by changing targetTile
    //TODO, monster should:
    //  + Patrol
    //  + Check if player is in line of sight
    //  + if so, chase down using A*
    //  + if player not in line of sight for X seconds, resume patrol

    LinkedList searchTiles = {0};
    LinkedList removedTiles = {0};
    LinkedList finalPath = {0};
    Vector2 thisTile = posToTileCoord(this->pos);
    Vector2Int target = ((Monster*)this->sub)->targetTile;
    
    if ((int)(this->pos.x / TILE_DIMS) == target.x &&
        (int)(this->pos.y / TILE_DIMS) == target.y)
    {
        return;
    }

    //Vector2Int target = {1, 3};

    PathTile nearMon[4] = { { x:thisTile.x - 1, y:thisTile.y     },
                            { x:thisTile.x + 1, y:thisTile.y     }, 
                            { x:thisTile.x,     y:thisTile.y - 1 }, 
                            { x:thisTile.x,     y:thisTile.y + 1 } };

    for (int i = 0; i < 4; i++)
    {
        if (!isTileSolid(coordToTileIndex(nearMon[i].x, nearMon[i].y)))
        {
            nearMon[i].heuristic = generateHeuristic(nearMon[i], target, this);
            linkedListMinPriorityAdd(&searchTiles, nearMon[i]);
        }
    }
    //TMP
    for (ListNode* current = searchTiles.front;
         current != NULL && !reachedPlayer(current, target);
         current = searchTiles.front)
    {
        thisTile.x = current->tile.x;
        thisTile.y = current->tile.y;

        PathTile nearMonCur[4] = { { x:thisTile.x - 1, y:thisTile.y    , parentX: thisTile.x, parentY: thisTile.y},
                                   { x:thisTile.x + 1, y:thisTile.y    , parentX: thisTile.x, parentY: thisTile.y}, 
                                   { x:thisTile.x,     y:thisTile.y - 1, parentX: thisTile.x, parentY: thisTile.y}, 
                                   { x:thisTile.x,     y:thisTile.y + 1, parentX: thisTile.x, parentY: thisTile.y} };

        for (int i = 0; i < 4; i++)
        {
            if (!linkedListContainsTile(&searchTiles, nearMonCur[i]) &&
                !linkedListContainsTile(&removedTiles, nearMonCur[i]) &&
                !isTileSolid(coordToTileIndex(nearMonCur[i].x, nearMonCur[i].y)))
            {
                nearMonCur[i].heuristic = generateHeuristic(nearMonCur[i], target, this);
                linkedListMinPriorityAdd(&searchTiles, nearMonCur[i]);
                //SDL_Log("x:%d, y:%d, h:%f", current->tile.x, current->tile.y, current->tile.heuristic);
            }   
        }
        //SDL_Log("x: %d, y: %d", current->tile.x, current->tile.y);
        linkedListAddFront(&removedTiles, current->tile);
        //SDL_Log("%d, %d, %d, %d", current->tile.x, current->tile.y, current->tile.parentX, current->tile.parentY);
        linkedListRemoveTile(&searchTiles, current->tile);
    }
    SDL_Log("c");
    //SDL_Log("x: %d, y: %d", removedTiles.front->tile.x, removedTiles.front->tile.y);
    PathTile tmp = { x:target.x, y:target.y};
    linkedListAddFront(&finalPath, tmp);
    if (removedTiles.front != NULL)
    {
        tmp = removedTiles.front->tile;
        for (;;)
        {
            if (tmp.parentX == 0 && tmp.parentY == 0) break;
            linkedListAddFront(&finalPath, tmp);
            SDL_Log("%d, %d, %d, %d", tmp.x, tmp.y, tmp.parentX, tmp.parentY);
            PathTile* tmpPtr = linkedListFindTile(&removedTiles, tmp.parentX, tmp.parentY);
            if (tmpPtr == NULL) 
            {
                break;
            } else {
                tmp = *tmpPtr;
            }
        }
    }
    
    SDL_Log("d");
    ((Monster*)this->sub)->pathList = finalPath;

    //TMP DEBUG
    for (ListNode* current = ((Monster*)this->sub)->pathList.front;
         current != NULL;
         current = current->next)
    {
        //SDL_Log("x: %d, y: %d", current->tile.x, current->tile.y);
    }
    //SDL_Log("astarend%d%d", ((Monster*)this->sub)->pathList.front->tile.x, ((Monster*)this->sub)->pathList.front->tile.y);
    // NEED TO DELETE LINKED LISTS
}