#include "engine_types.h"
#include "SDL2/SDL.h"

bool doBoxesCollide(Box box1, Box box2)
{

    //SDL_Log("%f, %f, %f, %f, %f, %f", box1.x, box1.y, box1.z, box2.x, box2.y, box2.z);
    //SDL_Log("%d", box1.x + box1.w > box2.x && box1.x < box2.x + box2.w);
    return box1.x + box1.w > box2.x && box1.x < box2.x + box2.w &&
           box1.y + box1.h > box2.y && box1.y < box2.y + box2.h &&
           box1.z + box1.d > box2.z && box1.z < box2.z + box2.d;
}
