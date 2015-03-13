#include <math.h>

#include "engine_types.h"

Vector2 vec2Unit(Vector2 vector)
{
    float magnitude = sqrt(vector.x * vector.x + vector.y * vector.y);
    if (magnitude == 0)
    {
        Vector2 retVec = {0};
        return retVec;
    };
    Vector2 retVec = { .x=(vector.x/magnitude), .y=(vector.y/magnitude), .y=(vector.y/magnitude) };
    return retVec;
}