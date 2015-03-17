#include <math.h>
#include <stdbool.h>

#include "engine_types.h"


Vector2 vec2Unit(Vector2 vector)
{
    float magnitude = sqrt(vector.x * vector.x + vector.y * vector.y);
    if (magnitude == 0)
    {
        Vector2 retVec = {0};
        return retVec;
    }
    Vector2 retVec = { .x=(vector.x/magnitude), .y=(vector.y/magnitude), .y=(vector.y/magnitude) };
    return retVec;
}

float constrainAngle(float angle)
{
    angle = fmod(angle + M_PI, M_PI * 2);
    if (angle < 0)
        angle += M_PI * 2;
    return angle - M_PI;
}

float distanceFormula(Vector2 vec0, Vector2 vec1)
{
    return sqrt((vec0.x - vec1.x) * (vec0.x - vec1.x) + (vec0.y - vec1.y) * (vec0.y - vec1.y));
}

bool rectsIntersect(SDL_Rect rect0, SDL_Rect rect1)
{
    return rect0.x + rect0.w > rect1.x && rect0.x < rect1.x + rect1.w &&
           rect0.y + rect0.h > rect1.y && rect0.y < rect1.y + rect1.h;
}