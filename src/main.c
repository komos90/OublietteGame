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


//Resolution
static const int SCREEN_WIDTH  = 320;
static const int SCREEN_HEIGHT = 240;

//Temp Globals
static uint32_t keyColorsTemp[MAX_KEYS] = {0xFFFF0000, 0xFF00FF00, 0xFF0000FF, 0xFF00AA88};
static float monsterSightRadius = 256.f;    //Should be in monster entity base
static float monsterFov = M_PI/2;           //Should be in monster entity base
static float monsterChaseTimeLimit = 5000;           //Should be in monster entity base


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
    EntityArray monsters = getLevelMonsters(monsterTemplate, playerData->levelNumber);
    
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
    if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 1024) == -1)
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
            if (entities->data[i].base->type == ENTITY_TYPE_MONSTER)
            {
                free(((Monster*)entities->data[i].sub)->patrolPoints);
            }
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

void onLevelStartTransitionEnd(void** args, int length)
{
    SDL_Log("onLevelStartTransitionEnd called");
    bool* paused = args[0];
    *paused = false;
    SDL_Log("onLevelStartTransitionEnd call ended");
}

void onLevelEndTransitionEnd(void** args, int length)
{
    SDL_Log("onLevelEndTransitionEnd called");
    PlayerData*     playerData =        args[0];

    void***  transitionArgs =            args[1];
    void***  levelStartTransitionArgs =  args[2];
    void (**onTransitionDone) (void**, int) = args[3];
    void (*onLevelStartTransitionEnd) (void**, int) = args[4];
    bool* shouldReloadLevel = args[5];
    int* transitionDirection = args[6];

    (*playerData).levelNumber++;
    //loadLevel(entities, player, playerData, rubyTemplate, keyTemplate, monsterTemplate);
    *shouldReloadLevel = true;
    *transitionArgs = (void**)levelStartTransitionArgs;
    SDL_Log("ptrs: %x, %x, %d", levelStartTransitionArgs, *levelStartTransitionArgs, *(bool*)*levelStartTransitionArgs);
    *onTransitionDone = onLevelStartTransitionEnd;
    *transitionDirection = -1;
    SDL_Log("onLevelEndTransitionEnd call ended");
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
    Mix_Chunk* rubySfx = Mix_LoadWAV("../res/sfx/ruby_pickup.ogg");

    //Create player
    Player player = { .width=32, .height=32 };
    PlayerData playerData = { .levelNumber=0 };
    EntityTemplate rubyTemplate = { .sprite=images.rubySprite, .width=32, .height=32, .spriteWidth=16, .spriteHeight=16, .type=ENTITY_TYPE_RUBY };
    EntityTemplate keyTemplate = { .sprite=images.keySprite, .width=32, .width=32, .spriteWidth=16, .spriteHeight=16, .type=ENTITY_TYPE_KEY};
    EntityTemplate monsterTemplate = { .sprite=images.monsterSprite, .width=64, .height=64, .spriteWidth=64, .spriteHeight=64, .type=ENTITY_TYPE_MONSTER};

    //Init level
    EntityArray entities = {0};
    loadLevel(&entities, &player, &playerData, &rubyTemplate, &keyTemplate, &monsterTemplate);
    //TEMP START PLAYING MUSIC
    Mix_FadeInMusic(gameBackgroundMusic, -1, 1000);

    //WRITING TEST
    /*{
        char* rubyCountStr = "They";
        SDL_Rect textRect = { 1, 0, 0, 0 };
        drawTextToSurface(rubyCountStr, images.secretDoorTexture, textRect, 0xFFFFFFFF, spriteFont);
        rubyCountStr = "are";
        textRect.y = 8;
        drawTextToSurface(rubyCountStr, images.secretDoorTexture, textRect, 0xFFFFFFFF, spriteFont);
        rubyCountStr = "coming";
        textRect.y = 16;
        drawTextToSurface(rubyCountStr, images.secretDoorTexture, textRect, 0xFFFF0000, spriteFont);
    }*/

    bool paused = true;
    bool shouldReloadLevel = false;

    //should encapsulate in transition object
    float transitionFraction = 1.0f;
    float transitionSpeed = 0.01f;
    int transitionDirection = -1;
    bool transitionJustFinished = false;
    void** transitionArgs;
    int transitionArgsLength;
    void (*onTransitionDone)(void* args, int length);

    //Setup level start transition
    void* levelStartTransitionArgs[1] = {&paused};
    transitionArgs = levelStartTransitionArgs;
    transitionArgsLength = 1;
    onTransitionDone = (void (*)(void*, int))onLevelStartTransitionEnd; //ugly function pointer cast

    //Setup level end transition
    void* levelEndTransitionArgs[] = {&playerData, &transitionArgs,
        &levelStartTransitionArgs, &onTransitionDone, 
        onLevelStartTransitionEnd, &shouldReloadLevel, &transitionDirection};

    SDL_Log("ptrs: %x, %x, %d", levelStartTransitionArgs, *levelStartTransitionArgs, *(bool*)*levelStartTransitionArgs);
    //Get input devices' states
    SDL_Joystick* gamePad = SDL_JoystickOpen(0);
    const uint8_t* keyState = SDL_GetKeyboardState(NULL);    
    
    bool running = true;
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
                    case SDLK_p:
                        paused = !paused;
                        break;
                }
                break;
            case SDL_MOUSEMOTION:
                if (paused) break;
                player.rotation += e.motion.xrel * 0.001;
            }
        }
        if (!paused)
        {
            //SDL_Log("Not paused!!");
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
                onTransitionDone = (void (*)(void*, int))onLevelEndTransitionEnd;
                transitionArgs = levelEndTransitionArgs;
                transitionDirection = 1;
                paused = true;
                //playerData.levelNumber++;
                //loadLevel(&entities, &player, &playerData, &rubyTemplate, &keyTemplate, &monsterTemplate);
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
                            //Play SFX
                            Mix_PlayChannel(-1, rubySfx, 0);
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
                        //This should be in a function
                        //  monster.c:
                        //      update(void)
                        //      {
                        //          updateTargetTile(void);
                        //          pathFind(void);
                        //      }
                        Entity* entity = &entities.data[i];
                        Monster* monster = (Monster*)entity->sub;
                        /*-----------------------------
                         *Check for aiState transitions
                         *---------------------------*/

                        if (rectsIntersect(playerRect, entityRect))
                        {
                            shouldReloadLevel = true;
                        }
                        if (distanceFormula(player.pos, entity->pos) < monsterSightRadius)
                        {
                            float monsterAngle = getMonsterAngle(entity);
                            Vector2 tmp1 = { .x=player.pos.x - entity->pos.x, .y=player.pos.y - entity->pos.y};
                            Vector2 tmp2 = {0};
                            tmp2.x = cos(monsterAngle) * tmp1.x + sin(monsterAngle) * tmp1.y;
                            tmp2.y = -sin(monsterAngle) * tmp1.x + cos(monsterAngle) * tmp1.y;
                            float playerAngle = atan2(tmp2.y, tmp2.x);

                            if (fabs(playerAngle) < monsterFov / 2)
                            {
                                switch(monster->aiState)
                                {
                                    case AI_PATROL:
                                    {
                                        monster->aiState = AI_CHASE;
                                        monster->giveUpChaseTimer.active = true;
                                        monster->giveUpChaseTimer.endTime = SDL_GetTicks() + monsterChaseTimeLimit;
                                        break;
                                    }
                                    case AI_CHASE:
                                    {
                                        monster->giveUpChaseTimer.endTime = SDL_GetTicks() + monsterChaseTimeLimit;
                                        break;
                                    }
                                }
                            }
                            else if (monster->aiState == AI_CHASE && monster->giveUpChaseTimer.active &&
                                monster->giveUpChaseTimer.endTime < SDL_GetTicks())
                            {
                                monster->aiState = AI_PATROL;
                            }
                        }
                        
                        /*-------------------------------------
                         * Select target tile, based on aiState
                         *-----------------------------------*/
                        switch (monster->aiState)
                        {
                            case AI_PATROL:
                            {
                                if ((int)(entities.data[i].pos.x / TILE_DIMS) == monster->targetTile.x &&
                                    (int)(entities.data[i].pos.y / TILE_DIMS) == monster->targetTile.y)
                                {
                                    monster->patrolIndex = (monster->patrolIndex + 1) % monster->patrolLength;
                                    monsterMoveAStar(entity);
                                }

                                Vector2Int tmp = { .x=monster->patrolPoints[monster->patrolIndex].x,
                                                   .y=monster->patrolPoints[monster->patrolIndex].y };
                                monster->targetTile = tmp;
                                break;
                            }
                            case AI_CHASE:
                            {
                                Vector2Int tmp = { .x=player.pos.x / TILE_DIMS, .y=player.pos.y / TILE_DIMS};
                                monster->targetTile = tmp;
                                break;
                            }
                        }
                        
                        /*---------
                         * Pathfind
                         *-------*/
                        monsterMove(&entities.data[i]);
                        break;
                    } 
                }
            }
        }
        //Update screen fade
        if (transitionDirection != 0)
        {
            transitionFraction += transitionDirection * transitionSpeed;
            if (transitionDirection > 0 && transitionFraction > 1.f) 
            {
                transitionFraction = 1.f;
                transitionJustFinished = true;
            }
            if (transitionDirection < 0 && transitionFraction < 0.f)
            {
                 transitionFraction = 0.f;
                 transitionJustFinished = true;
            }
        }
        if (transitionJustFinished)
        {
            transitionJustFinished = false;
            transitionDirection = 0;
            if (onTransitionDone != 0)
            {
                (*onTransitionDone)(transitionArgs, transitionArgsLength);
            }
            else {
                SDL_Log("Error: No transition function!");
            }
        }
        //Draw ====
        //Send game entities to gfx engine to be rendered 
        draw(player, entities);
        //All this should be in a drawUI() function in gfx_engine.c
        //Draw rubies collected
        {
            Rectangle rubyImageRect = { SCREEN_WIDTH/32, SCREEN_HEIGHT/32, 11, 11 };
            blitToPixelBuffer(images.rubySprite, rubyImageRect, 0);
            char rubyCountStr[32];
            sprintf(rubyCountStr, "%d/%d", playerData.rubiesCollected, getTotalLevelRubies());
            SDL_Rect textRect = { SCREEN_WIDTH/8, SCREEN_HEIGHT/16, 0, 0 };
            drawText(rubyCountStr, textRect, 0xFF7A0927, spriteFont);
        }
        /*{
            char* patrolStateString = "Patrol State.";
            char* chaseStateString = "Chase State!";
            char* aiString = changeToChaseState ? chaseStateString : patrolStateString;
            char finalString[32];
            sprintf(finalString, "AI State: %s", aiString);
            SDL_Rect textRect = {0, SCREEN_HEIGHT - 16, 0, 0 };
            drawText(finalString, textRect, 0xFFFF0000, spriteFont);
        }/**/
        //Draw keys collected
        {
            for (int i = 0; i < MAX_KEYS; i++)
            {
                if (playerData.keysCollected[i] == true)
                {
                    Rectangle keyImageRect = { SCREEN_WIDTH/3 + i * 16, SCREEN_HEIGHT/32, 11, 11 };
                    blitToPixelBuffer(images.keySprite, keyImageRect, keyColorsTemp[i]);
                }
            }
        }

        //Draw screen fade to black
        {
            uint32_t fadeColour = 0x000000; 
            SDL_Rect topRect = {0, 0, SCREEN_WIDTH, (SCREEN_HEIGHT / 2) * transitionFraction};
            SDL_Rect botRect = {0, SCREEN_HEIGHT - (SCREEN_HEIGHT / 2) * transitionFraction,
                SCREEN_WIDTH, (SCREEN_HEIGHT / 2) * transitionFraction};
            drawRect(topRect, fadeColour);
            drawRect(botRect, fadeColour);
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
