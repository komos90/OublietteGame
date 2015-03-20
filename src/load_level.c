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
#elif _WIN32
    #include <SDL.h>
#endif

#include "load_level.h"

static Level level = {0};

bool isTileIndexValid(int i)
{
    return i >= 0 && i < level.width * level.height;
}

int posToTileIndex(int x, int y)
{
    int index = (int)((y / TILE_DIMS) * level.width + (x / TILE_DIMS));
    return index;
}

int posVecToTileIndex(Vector2 pos)
{
    return (int)(pos.y / TILE_DIMS) * level.width + (int)(pos.x / TILE_DIMS);
}

int posVecToIndex(Vector2 pos)
{
    return (int)(pos.y * level.width + pos.x);
}

Vector2 posToTileCoord(Vector2 pos)
{
    Vector2 coordVec = { (int)pos.x / TILE_DIMS, (int)pos.y / TILE_DIMS };
    return coordVec;
}

Vector2 getPlayerStartPos()
{
    for (int y = 0; y < level.height; y++)
    {
        for (int x = 0; x < level.width; x++)
        {
            if (level.data[y * level.width + x] == PLAYER_START)
            {
                Vector2 playerPos = { x * TILE_DIMS + TILE_DIMS/2, y * TILE_DIMS + TILE_DIMS/2 };
                return playerPos;
            }
        }
    }
    Vector2 errorVector = {0, 0};
    return errorVector;
}

bool isTileSolid(int index)
{
    return level.data[index] == WALL || level.data[index] == SECRET_DOOR;
}

void setTileTo(int index, char tile)
{
    level.data[index] = tile;
}

int getTotalLevelRubies()
{
    return level.rubyCount;
}

char getLevelTile(int index)
{
    return level.data[index];
}

EntityArray getLevelRubies(EntityTemplate* rubyTemplate)
{
    //Create EntityArray of correct size
    EntityArray rubyArray = {0};
    {
        int rubyCount = 0;
        for (int i = 0; i < level.width * level.height; i++)
            if (level.data[i] == RUBY)
                rubyCount++;

        //MALLOC should free when loading a new level
        rubyArray.size = rubyCount;
        rubyArray.data = (Entity*)malloc(rubyCount * sizeof(Entity));
    }

    //Populate array with ruby coords
    int rubyIndex = 0;
    for (int y = 0; y < level.height; y++)
    {
        for (int x = 0; x < level.width; x++)
        {
            if (level.data[y * level.width + x] == RUBY)
            {
                Vector2 tmp = { x * TILE_DIMS + TILE_DIMS/2, y * TILE_DIMS + TILE_DIMS/2 };
                rubyArray.data[rubyIndex].pos = tmp;
                rubyArray.data[rubyIndex].base = rubyTemplate;
                rubyIndex++;
            }
        }
    }
    return rubyArray;
}

EntityArray getLevelKeys(EntityTemplate* keyTemplate)
{
    //Create EntityArray of correct size
    EntityArray keyArray = {0};
    {
        int keyCount = 0;
        for (int i = 0; i < level.width * level.height; i++)
            if (level.data[i] == KEY1)
                keyCount++;

        //MALLOC should free when loading a new level
        keyArray.size = keyCount;
        keyArray.data = (Entity*)malloc(keyCount * sizeof(Entity));
    }

    //Populate array with ruby coords
    int keyIndex = 0;
    for (int y = 0; y < level.height; y++)
    {
        for (int x = 0; x < level.width; x++)
        {
            if (level.data[y * level.width + x] == KEY1)
            {
                Vector2 tmp = { x * TILE_DIMS + TILE_DIMS/2, y * TILE_DIMS + TILE_DIMS/2 };
                keyArray.data[keyIndex].pos = tmp;
                keyArray.data[keyIndex].base = keyTemplate;
                keyIndex++;
            }
        }
    }
    return keyArray;
}

//TODO Fix levels breaking if they don't end on a blank line
void loadLevel(char* fileName)
{
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
    
    level.rubyCount = 0;
    //Go back to beginning of file
    fsetpos(file, &filePos);
    //MALLOC should free when loading a new level
    level.data = (char*)malloc(level.width * level.height * sizeof(char));
    //Re-read the file, loading the actual data into the level structure. 
    {
        int ch;
        int i = 0;
        while (EOF != (ch=getc(file)))
        {
            if (ch != '\n')
            {
                if (ch == RUBY) level.rubyCount++;
                level.data[i] = ch;
                i++;
            }
        }
    }
}
