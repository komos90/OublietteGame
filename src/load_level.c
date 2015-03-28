/*
 Level loading code for 3d Software Renderer
 Seoras Macdonald
 seoras1@gmail.com
 2015
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#ifdef __linux__
    #include <SDL2/SDL.h>
#elif _WIN32
    #include <SDL.h>
#endif

#include "load_level.h"
#include "images.h"
#include "monster.h"

static Level level = {0};

bool isTileIndexValid(int i)
{
    return i >= 0 && i < level.width * level.height;
}

//TODO get rid of some of these by chaining together some of them.
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
            if (level.data[y * level.width + x] == TILE_PLAYER_START)
            {
                Vector2 playerPos = { x * TILE_DIMS + TILE_DIMS/2, y * TILE_DIMS + TILE_DIMS/2 };
                return playerPos;
            }
        }
    }
    Vector2 errorVector = {0, 0};
    return errorVector;
}

//Should be a more elegant way to do this. Time to overhall tile representation?
bool isTileSolid(int index)
{
    return level.data[index] == TILE_WALL ||
           level.data[index] == TILE_SECRET_DOOR ||
           level.data[index] == TILE_DOOR0 ||
           level.data[index] == TILE_DOOR1 ||
           level.data[index] == TILE_DOOR2 ||
           level.data[index] == TILE_DOOR3;
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
            if (level.data[i] == TILE_RUBY)
                rubyCount++;

        //Set correct ruby total
        level.rubyCount = rubyCount;
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
            if (level.data[y * level.width + x] == TILE_RUBY)
            {
                Vector2 tmp = { x * TILE_DIMS + TILE_DIMS/2, y * TILE_DIMS + TILE_DIMS/2 };
                rubyArray.data[rubyIndex].pos = tmp;
                rubyArray.data[rubyIndex].zPos = -16;
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
            if (level.data[i] == TILE_KEY0 ||
                level.data[i] == TILE_KEY1 ||
                level.data[i] == TILE_KEY2 ||
                level.data[i] == TILE_KEY3)
            {
                keyCount++;
            }

        //MALLOC should free when loading a new level
        keyArray.size = keyCount;
        keyArray.data = (Entity*)malloc(keyCount * sizeof(Entity));
    }

    //Populate array with key coords
    int keyIndex = 0;
    for (int y = 0; y < level.height; y++)
    {
        for (int x = 0; x < level.width; x++)
        {
            if (level.data[y * level.width + x] == TILE_KEY0 ||
                level.data[y * level.width + x] == TILE_KEY1 ||
                level.data[y * level.width + x] == TILE_KEY2 ||
                level.data[y * level.width + x] == TILE_KEY3)
            {
                Vector2 tmp = { x * TILE_DIMS + TILE_DIMS/2, y * TILE_DIMS + TILE_DIMS/2 };
                keyArray.data[keyIndex].pos = tmp;
                keyArray.data[keyIndex].zPos = -12;
                keyArray.data[keyIndex].base = keyTemplate;
                //MALLOC should free on new level
                keyArray.data[keyIndex].sub = malloc(sizeof(Key));

                //Determine id of key, i.e. it's colour
                if (level.data[y * level.width + x] == TILE_KEY0)
                {
                    ((Key*)(keyArray.data[keyIndex].sub))->id = 0;
                }
                else if (level.data[y * level.width + x] == TILE_KEY1)
                {
                    ((Key*)(keyArray.data[keyIndex].sub))->id = 1;
                }
                else if (level.data[y * level.width + x] == TILE_KEY2)
                {
                    ((Key*)(keyArray.data[keyIndex].sub))->id = 2;
                }
                else
                {
                    ((Key*)(keyArray.data[keyIndex].sub))->id = 3;
                }
                
                keyIndex++;
            }
        }
    }
    return keyArray;
}

EntityArray getLevelMonsters(EntityTemplate* monsterTemplate)
{
    //Create EntityArray of correct size
    EntityArray monsterArray = {0};
    {
        int monsterCount = 0;
        for (int i = 0; i < level.width * level.height; i++)
            if (level.data[i] == TILE_MONSTER)
                monsterCount++;

        //MALLOC should free when loading a new level
        monsterArray.size = monsterCount;
        monsterArray.data = (Entity*)malloc(monsterCount * sizeof(Entity));
    }

    //Populate array with monster coords
    int monsterIndex = 0;
    for (int y = 0; y < level.height; y++)
    {
        for (int x = 0; x < level.width; x++)
        {
            if (level.data[y * level.width + x] == TILE_MONSTER)
            {
                Vector2 tmp = { x * TILE_DIMS + TILE_DIMS/2, y * TILE_DIMS + TILE_DIMS/2 };
                monsterArray.data[monsterIndex].pos = tmp;
                monsterArray.data[monsterIndex].zPos = -12;
                monsterArray.data[monsterIndex].base = monsterTemplate;
                //MALLOC should free on new level
                monsterArray.data[monsterIndex].sub = malloc(sizeof(Monster));
                ((Monster*)monsterArray.data[monsterIndex].sub)->direction = DIR_NONE;
                monsterIndex++;
            }
        }
    }
    return monsterArray;
}

SDL_Surface* getTileTexture(int index) {
    assert(index >=0 && index < level.width * level.height);
    if(level.data[index] == TILE_WALL)
    {
        return images.caveTexture;
    }
    else if (level.data[index] == TILE_DOOR0 ||
             level.data[index] == TILE_DOOR1 ||
             level.data[index] == TILE_DOOR2 ||
             level.data[index] == TILE_DOOR3)
    {
        return images.doorTexture;
    }
    else if (level.data[index] == TILE_SECRET_DOOR)
    {
        return images.secretDoorTexture;
    }
}

bool fileExists(char* filePath)
{
    FILE* file = fopen(filePath, "r");
    return file == NULL ? false : true;
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
    
    //Go back to beginning of file
    fsetpos(file, &filePos);
    //MALLOC should free when loading a new level
    if (level.data != NULL) free(level.data);
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
}
