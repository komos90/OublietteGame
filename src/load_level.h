/*
 Level loading code for 3d Software Renderer
 Seoras Macdonald
 seoras1@gmail.com
 2015
 */
#pragma once

#include "engine_types.h"

typedef struct
{
    int width;
    int height;
    char* data;
} Level;

Level loadLevel(char* fileName);