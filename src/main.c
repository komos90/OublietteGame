/*
Raycaster wolfenstein 3d style game.
Seoras Macdonald
seoras1@gmail.com
2015
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <limits.h>

#ifdef __linux__
    #include <SDL2/SDL.h>
    #include <SDL2/SDL_image.h>
    #include <SDL2/SDL_mixer.h>
#elif _WIN32
    #include <SDL.h>
    #include <SDL_image.h>
    #include <SDL_mixer.h>
#endif

#include "engine_types.h"
#include "load_level.h"
#include "gfx_engine.h"
#include "images.h"
#include "monster.h"

static const int SCREEN_WIDTH  = 213;//1366;//854;//213;//854;
static const int SCREEN_HEIGHT = 120;//768;//480;//120;//480;

//Temp Globals
static uint32_t keyColorsTemp[MAX_KEYS] = {0xFFFF0000, 0xFF00FF00, 0xFF0000FF, 0xFF00AA88};

EntityArray initLevel(
    Player* player, PlayerData* playerData, EntityTemplate* rubyTemplate,
    EntityTemplate* keyTemplate, EntityTemplate* monsterTemplate)
{
    player->pos = getPlayerStartPos();
    player->rotation = 0;
    playerData->rubiesCollected = 0;
    for (int i = 0; i < MAX_KEYS; i++)
    {
        playerData->keysCollected[i] = false;
    }

    EntityArray rubies = getLevelRubies(rubyTemplate);
    EntityArray keys = getLevelKeys(keyTemplate);
    EntityArray monsters = getLevelMonsters(monsterTemplate);
    
    EntityArray entities;
    entities.size = rubies.size + keys.size + monsters.size;
    entities.data = (Entity*)malloc(entities.size * sizeof(Entity));
    memcpy((void*)entities.data, (void*)rubies.data, rubies.size * sizeof(Entity));
    memcpy((void*)(entities.data + rubies.size), (void*)keys.data, keys.size * sizeof(Entity));
    memcpy((void*)(entities.data + rubies.size + keys.size), (void*)monsters.data, monsters.size * sizeof(Entity));

    //Free memory
    free(rubies.data);
    free(keys.data);
    free(monsters.data);

    return entities;
}

bool initSDL(SDL_Window** window, SDL_Renderer** renderer)
{
    //Initialise SDL ====
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0 || IMG_Init(IMG_INIT_PNG) < 0)
    {
        printf ("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return false;
    }
    //Init audio
    if (Mix_Init(MIX_INIT_OGG) != MIX_INIT_OGG)
    {
        printf ("SDL_mixer could not initialize! SDL_Error: %s\n", SDL_GetError());
        return false;
    }
    if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 4096) == -1)
    {
        printf ("SDL_mixer could not open audio! SDL_Error: %s\n", SDL_GetError());
        return false;
    }
    *window = SDL_CreateWindow(
        "The Caves", SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, 
        SDL_WINDOW_FULLSCREEN_DESKTOP);
    if (*window == NULL)
    {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    *renderer = SDL_CreateRenderer(*window, -1, 0);
    return true;
}

bool loadLevel(EntityArray* entities, Player* player,
    PlayerData* playerData, EntityTemplate* rubyTemplate,
    EntityTemplate* keyTemplate, EntityTemplate* monsterTemplate)
{
    char nextLevelFilePath[LEVEL_FILE_PATH_MAX_LEN];
    sprintf(nextLevelFilePath, "../res/levels/level%d.lvl", playerData->levelNumber);
    if(fileExists(nextLevelFilePath))
    { 
        loadLevelTiles(nextLevelFilePath);
        for (int i = 0; i < entities->size; i++)
        {
            free(entities->data[i].sub);
        }
        free(entities->data);
        (*entities) = initLevel(player, playerData, rubyTemplate, keyTemplate, monsterTemplate);
    }
}

void toggleFullscreen(SDL_Window* window)
{
    uint32_t flags = SDL_GetWindowFlags(window);
    if (flags & SDL_WINDOW_FULLSCREEN || flags & SDL_WINDOW_FULLSCREEN_DESKTOP)
    {
        SDL_SetWindowFullscreen(window, 0);
    }
    else
    {
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
    }
}

int main(int argc, char* args[])
{
    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;
    if(!initSDL(&window, &renderer))
    {
        SDL_Log("SDLinit failed.");
        return 1;
    }
    SDL_Texture* screenTexture = SDL_CreateTexture(
        renderer, SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH,
        SCREEN_HEIGHT);
    if (screenTexture == NULL)
    {
        SDL_Log("screenTexture is NULL.");
        return 1;
    }

    //Grab cursor
    SDL_SetRelativeMouseMode(true);

    //Allocate pixel buffer
    createPixelBuffer(SCREEN_WIDTH, SCREEN_HEIGHT);

    loadImages();

    //Load sprite font
    SpriteFont spriteFont = { .charW=8, .charH=8 };
    spriteFont.sprite = IMG_Load("../res/fonts/atari_font.png");
    spriteFont.sprite = SDL_ConvertSurfaceFormat(spriteFont.sprite, SDL_PIXELFORMAT_ARGB8888, 0);

    //Audio SHOULD EXTRACT TO SEPARATE FILE
    Mix_Music* gameBackgroundMusic = Mix_LoadMUS("../res/music/game_back.ogg");

    //Create player
    Player player = { .width=32, .height=32 };
    PlayerData playerData = { .levelNumber=0 };
    EntityTemplate rubyTemplate = { .sprite=images.rubySprite, .width=32, .height=32, .type=ENTITY_TYPE_RUBY };
    EntityTemplate keyTemplate = { .sprite=images.keySprite, .width=32, .width=32, .type=ENTITY_TYPE_KEY};
    EntityTemplate monsterTemplate = { .sprite=images.monsterSprite, .width=64, .height=64, .type=ENTITY_TYPE_MONSTER};

    //Init level
    EntityArray entities = {0};
    loadLevel(&entities, &player, &playerData, &rubyTemplate, &keyTemplate, &monsterTemplate);
    //TEMP START PLAYING MUSIC
    Mix_FadeInMusic(gameBackgroundMusic, -1, 1000);

    //Get input devices' states
    SDL_Joystick* gamePad = SDL_JoystickOpen(0);
    const uint8_t* keyState = SDL_GetKeyboardState(NULL);    
    
    bool running = true;
    bool shouldReloadLevel = false;
    int currentFps = 0;
    //Main Loop ====
    while(running) 
    {
        int frameStartTime = SDL_GetTicks();
        
        if (shouldReloadLevel)
        {
            loadLevel(&entities, &player, &playerData, &rubyTemplate, &keyTemplate, &monsterTemplate);
            shouldReloadLevel = false;
        }
        //SDL Event Loop
        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            switch(e.type)
            {
            case SDL_QUIT:
                running = false;
                break;
            case SDL_KEYDOWN:
                switch(e.key.keysym.sym)
                {
                    case SDLK_RETURN:
                        if (e.key.keysym.mod & KMOD_ALT)
                            toggleFullscreen(window);
                        break;
                }
                break;
            case SDL_MOUSEMOTION:
                player.rotation += e.motion.xrel * 0.001;
            }
        }
        //Joystick input
        Vector2 oldPlayerPos = player.pos;
        Vector2 moveVector = {0};
        int moveVel = 4;
        {
            const int JOYSTICK_DEAD_ZONE = 8000;
            Vector2 oldPlayerPos = player.pos;
            Vector2 moveVector = {0};
            int moveVel = 4;
            if( SDL_JoystickGetAxis(gamePad, 0) < -JOYSTICK_DEAD_ZONE )
            {
                moveVector.x += cosf(player.rotation - M_PI/2);
                moveVector.y += sinf(player.rotation - M_PI/2);
            }
            //Right of dead zone
            else if( SDL_JoystickGetAxis(gamePad, 0) > JOYSTICK_DEAD_ZONE )
            {
                moveVector.x += cosf(player.rotation + M_PI/2);
                moveVector.y += sinf(player.rotation + M_PI/2);
            }
            //Left of dead zone
            if( SDL_JoystickGetAxis(gamePad, 1) < -JOYSTICK_DEAD_ZONE )
            {
                moveVector.x += cosf(player.rotation);
                moveVector.y += sinf(player.rotation);
            }
            //Right of dead zone
            else if( SDL_JoystickGetAxis(gamePad, 1) > JOYSTICK_DEAD_ZONE )
            {
                moveVector.x += cosf(player.rotation + M_PI);
                moveVector.y += sinf(player.rotation + M_PI);
            }
            //Left of dead zone
            if( SDL_JoystickGetAxis(gamePad, 2) < -JOYSTICK_DEAD_ZONE )
            {
                player.rotation -= 0.04 * -SDL_JoystickGetAxis(gamePad, 2) / 32767.f;
            }
            //Right of dead zone
            else if( SDL_JoystickGetAxis(gamePad, 2) > JOYSTICK_DEAD_ZONE )
            {
                player.rotation += 0.04 * SDL_JoystickGetAxis(gamePad, 2) / 32767.f;
            }

            //Keyboard Input
            if (keyState[SDL_SCANCODE_A])
            {
                moveVector.x += cosf(player.rotation - M_PI/2);
                moveVector.y += sinf(player.rotation - M_PI/2);
            }
            if (keyState[SDL_SCANCODE_D])
            {
                moveVector.x += cosf(player.rotation + M_PI/2);
                moveVector.y += sinf(player.rotation + M_PI/2);
            }
            if (keyState[SDL_SCANCODE_S])
            {
                moveVector.x += cosf(player.rotation + M_PI);
                moveVector.y += sinf(player.rotation + M_PI);
            }
            if (keyState[SDL_SCANCODE_W])
            {
                moveVector.x += cosf(player.rotation);
                moveVector.y += sinf(player.rotation);
            }
            if (keyState[SDL_SCANCODE_LEFT])
            {
                player.rotation += 0.02;
            }
            if (keyState[SDL_SCANCODE_RIGHT])
            {
                player.rotation -= 0.02;
            }
            if (keyState[SDL_SCANCODE_SPACE])
            {
                Vector2 actionTile = player.pos;
                actionTile.x += cosf(player.rotation) * TILE_DIMS;
                actionTile.y += sinf(player.rotation) * TILE_DIMS;
                actionTile = posToTileCoord(actionTile);

                char tile = getLevelTile(posVecToIndex(actionTile));
                if (tile == TILE_SECRET_DOOR)
                {
                    setTileTo(posVecToIndex(actionTile), TILE_FLOOR);
                }
                else if ((tile == TILE_DOOR0 && playerData.keysCollected[0] == true) ||
                    (tile == TILE_DOOR1 && playerData.keysCollected[1] == true) ||
                    (tile == TILE_DOOR2 && playerData.keysCollected[2] == true) ||
                    (tile == TILE_DOOR3 && playerData.keysCollected[3] == true))
                {
                    setTileTo(posVecToIndex(actionTile), TILE_FLOOR);
                }
            }
            //Normalise moveVector
            moveVector = vec2Unit(moveVector);
            player.pos.x += moveVector.x * moveVel;
            player.pos.y += moveVector.y * moveVel;

            //Collision
            int tileIndex = posVecToTileIndex(player.pos);
            char tile = getLevelTile(tileIndex);
            if (isTileSolid(tileIndex))
            {
                player.pos = oldPlayerPos;
            }
            else if (tile == TILE_LEVEL_END)
            {
                //Load next level
                playerData.levelNumber++;
                loadLevel(&entities, &player, &playerData, &rubyTemplate, &keyTemplate, &monsterTemplate);
            }
        }

        //Game Logic ====
        //Main Entity Loop
        for (int i = 0; i < entities.size; i++)
        {
            //Entity Collision
            SDL_Rect playerRect = { player.pos.x, player.pos.y, player.width, player.height };
            Entity entity = entities.data[i];
            SDL_Rect entityRect = { entity.pos.x, entity.pos.y, entity.base->width, entity.base->height };
            
            switch(entity.base->type)
            {
            case ENTITY_TYPE_RUBY:
                {
                    if (rectsIntersect(playerRect, entityRect))
                    {
                        playerData.rubiesCollected++;
                        //Remove entity
                        entities.data[i] = entities.data[entities.size - 1];
                        entities.size--;
                    }
                    break;
                }
            case ENTITY_TYPE_KEY:
                {
                    if (rectsIntersect(playerRect, entityRect))
                    {
                        //TEMPORARY!
                        playerData.keysCollected[((Key*)entities.data[i].sub)->id] = true;
                        //Remove entity
                        entities.data[i] = entities.data[entities.size - 1];
                        entities.size--;
                    }
                    break;
                }
            case ENTITY_TYPE_MONSTER:
                {
                    if (rectsIntersect(playerRect, entityRect))
                    {
                        shouldReloadLevel = true;
                    }

                    //Movement Code
                    monsterMove(&entities.data[i], player);

                    break;
                } 
            }
        }
        
        //Draw ====
        //Send game entities to gfx engine to be rendered 
        draw(player, entities);
        //Draw test text
        {
            char rubyCountStr[32];
            sprintf(rubyCountStr, "%d/%d", playerData.rubiesCollected, getTotalLevelRubies());
            SDL_Rect textRect = { SCREEN_WIDTH/8, SCREEN_HEIGHT/16, 0, 0 };
            drawText(rubyCountStr, textRect, 0xFF7A0927, spriteFont);
        }
        {
            char fpsDisplayStr[32];
            sprintf(fpsDisplayStr, "Fps: %d", currentFps);
            SDL_Rect textRect = {0, SCREEN_HEIGHT - 16, 0, 0 };
            drawText(fpsDisplayStr, textRect, 0xFFFF0000, spriteFont);
        }
        //Draw keys collected
        {
            for (int i = 0; i < MAX_KEYS; i++)
            {
                if (playerData.keysCollected[i] == true)
                {
                    //VERY TEMPORARY!
                    //SHOULD DRAW WEE KEY ICON
                    SDL_Rect textRect = { SCREEN_WIDTH/3 + i * 16, SCREEN_HEIGHT/16, 0, 0 };
                    drawText("K", textRect, keyColorsTemp[i], spriteFont);
                }
            }
        }

        //Render the pixel buffer to the screen
        SDL_UpdateTexture(screenTexture, NULL, getPixelBuffer()->pixels, SCREEN_WIDTH * sizeof(uint32_t));        
        SDL_RenderCopy(renderer, screenTexture, NULL, NULL);
        SDL_RenderPresent(renderer);

        //Lock to 60 fps
        int delta = SDL_GetTicks() - frameStartTime;
        if (delta < 1000/60)
        {
            SDL_Delay(1000/60 - delta);
        }
        currentFps = 1000/(SDL_GetTicks() - frameStartTime);
    }
    return 0;
}
