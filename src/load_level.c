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


int posToTileIndex(int x, int y, Level level)
{
    int index = (int)((y / TILE_DIMS) * level.width + (x / TILE_DIMS));
    return index;
}

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
