/*
Raycaster wolfenstein 3d style game.
Seoras Macdonald
seoras1@gmail.com
2015
*/
#include <math.h>
#include <stdbool.h>

#include "engine_types.h"

/*------------------------------------------------------------------------------
 * Input: A 2D Vector.
 * Output: A unit vector in the direction of the input vector.
 * Note: If the input vector is (0, 0) then (0, 0) will be returned.
 *----------------------------------------------------------------------------*/
Vector2 vec2Unit(Vector2 vector)
{
    Vector2 retVec = {0};
    float magnitude = sqrt(vector.x * vector.x + vector.y * vector.y);
    if (magnitude != 0)
    {
        retVec.x = vector.x / magnitude;
        retVec.y = vector.y / magnitude;
    }
    return retVec;
}

/*------------------------------------------------------------------------------
 * Input: An angle in radians.
 * Output: An angle equivalent to the input angle in the range [-pi,pi].
 *----------------------------------------------------------------------------*/
float constrainAngle(float angle)
{
    // Use modulo to put (angle + pi) into the range [-2pi,2pi]
    angle = fmod(angle + M_PI, M_PI * 2);
    // put (angle + pi) into the range [0, 2pi]
    if (angle < 0)
        angle += M_PI * 2;
    //return angle in range [-pi, pi]
    return angle - M_PI;
}

/*------------------------------------------------------------------------------
 * Input: 2 2D vectors: vec0, vec1
 * Output: the distance between these two vectors
 *----------------------------------------------------------------------------*/
float distanceFormula(Vector2 vec0, Vector2 vec1)
{
    return sqrt((vec0.x - vec1.x) * (vec0.x - vec1.x) + (vec0.y - vec1.y) * (vec0.y - vec1.y));
}

/*------------------------------------------------------------------------------
 * Input: 2 Rectangles rect0, rect 1
 * Output: True if the two rectangles overlap, false otherwise.
 *----------------------------------------------------------------------------*/
bool rectsIntersect(SDL_Rect rect0, SDL_Rect rect1)
{
    return rect0.x + rect0.w > rect1.x && rect0.x < rect1.x + rect1.w &&
           rect0.y + rect0.h > rect1.y && rect0.y < rect1.y + rect1.h;
}