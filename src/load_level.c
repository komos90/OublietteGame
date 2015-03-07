/*
 Level loading code for 3d Software Renderer
 Seoras Macdonald
 seoras1@gmail.com
 2015
 */
#include <stdio.h>
#include <stdlib.h>

#ifdef __linux__
    #include <SDL2/SDL.h>
    #define M_PI 3.14159265358979323846
#elif _WIN32
    #include <SDL.h>
#endif

#include "load_level.h"
#include "meshes.h"

Level loadLevel(char* fileName)
{
    Level level;
    FILE* file = fopen(fileName, "r");
    if (file == NULL)
    {
        SDL_Log("Could not open level file.");
        SDL_Log(fileName);
        exit(1);
    }

    //Save the pos beginning of file
    fpos_t filePos;
    fgetpos(file, &filePos);
    //Count the number of lines in the file
    {
        int ch;
        int lineCount = 0;
        int charCount = 0;
        while (EOF != (ch=getc(file)))
        {
            ++charCount;
            if (ch == '\n')
                ++lineCount;
        }
        level.height = lineCount;
        level.width = (charCount / lineCount) - 1;
    }
    

    //Go back to beginning of file
    fsetpos(file, &filePos);
    level.data = (char*)malloc(level.width * level.height * sizeof(char));
    //Re-read the file, loading the actual data into the level structure. 
    {
        int ch;
        int i = 0;
        while (EOF != (ch=getc(file)))
        {
            if (ch != '\n')
            {
                level.data[i] = ch;
                i++;
            }
        }
    }
    return level;
}

EntityArray createLevelEntities(Level level)
{
    //Count the numberof entitieswe need to store 
    int entityCount = 0;
    for (int i = 0; i < level.width * level.height; i++)
    {
        if (level.data[i] == '.')
        {
            if (i - level.width < 0 || level.data[i - level.width] == '#')
                entityCount++;
            if (i + level.width > level.width * level.height || level.data[i + level.width] == '#')
                entityCount++;
            if (i - 1 < 0 || level.data[i - 1] == '#')
                entityCount++;
            if (i + 1 > level.width * level.height || level.data[i + 1] == '#')
                entityCount++;
        }

    }

    EntityArray entities = {.length = entityCount};
    //MALLOC should be freed when a new level is created
    entities.data = (Entity*)malloc(entityCount * sizeof(Entity));
    
    //Create the actual entities and store int the entitiyArray
    int entityIndex = 0;
    for (int i = 0; i < level.width * level.height; i++)
    {
        int x = i % level.width;
        int z = i / level.width;
        
        if (level.data[i] == '.') {
            
            //Create entities  
            Entity northWall = { .position = {x * 100, 0, z * 100 - 50},    .scale = {50, 50, 50},
                                 .rotation = {0, 0, 0},       .mesh=plane,  .color=0x00FF0000};
            Entity southWall = { .position = {x * 100, 0, z * 100 + 50},    .scale = {50, 50, 50},
                                 .rotation = {M_PI, 0, 0},    .mesh=plane,  .color=0x0000FF00};
            Entity eastWall  = { .position = {x * 100 - 50, 0, z * 100},    .scale = {50, 50, 50},
                                 .rotation = {0, M_PI/2, 0},  .mesh=plane,  .color=0x000000FF};
            Entity westWall  = { .position = {x * 100 + 50, 0, z * 100},    .scale = {50, 50, 50},
                                 .rotation = {0, -M_PI/2, 0}, .mesh=plane,  .color=0x00FF00FF};
            
            //Add entities needed for this tile
            if (i + 1 > level.width * level.height || level.data[i + 1] == '#')
                entities.data[entityIndex++] = westWall;
            if (i - level.width < 0 || level.data[i - level.width] == '#')
                entities.data[entityIndex++] = northWall;
            if (i + level.width > level.width * level.height || level.data[i + level.width] == '#')
                entities.data[entityIndex++] = southWall;
            if (i - 1 < 0 || level.data[i - 1] == '#')
                entities.data[entityIndex++] = eastWall;
        }
    }
    return entities;
}
