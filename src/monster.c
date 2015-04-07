#include <float.h>
#include <math.h>

#include "monster.h"
#include "engine_types.h"
#include "load_level.h"


void monsterMove(Entity* this, Player player)
{
    //Should probably check entity is actually a monster
    Vector2 targetTile = { .x=player.pos.x / TILE_DIMS, .y=player.pos.y / TILE_DIMS };
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
}

void monsterMoveAStar(Entity* this, Player player)
{
    //TODO, monster should:
    //  + Patrol/Move randomly
    //  + Check if player is in line of sight
    //  + if so, chase down using A*
    //  + if player not in line of sight for X seconds, resume patrol/random movement
}