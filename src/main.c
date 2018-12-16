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


//Temp Globals
static uint32_t keyColorsTemp[MAX_KEYS] = {0xFFFF0000, 0xFF00FF00, 0xFF0000FF, 0xFF00AA88};
static float monsterSightRadius = 256.f;    //Should be in monster entity base
static float monsterFov = M_PI/2;           //Should be in monster entity base
static float monsterChaseTimeLimit = 5000;  //Should be in monster entity base


bool oneInXChance(int x) {
    return rand() % x == 0;
}

EntityArray initLevel(
    Player* player, PlayerData* playerData, EntityTemplate* rubyTemplate,
    EntityTemplate* keyTemplate, EntityTemplate* monsterTemplate, EntityTemplate* endPortalTemplate)
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
    Entity endPortalEntity = { .pos=getLevelEndPos(),
                               .zPos=0,
                               .xClip=0,
                               .yClip=0,
                               .base=endPortalTemplate,
                               .sub=NULL};

    EntityArray entities;
    entities.size = rubies.size + keys.size + monsters.size + 1;
    entities.data = (Entity*)malloc(entities.size * sizeof(Entity));
    memcpy((void*)entities.data, (void*)rubies.data, rubies.size * sizeof(Entity));
    memcpy((void*)(entities.data + rubies.size), (void*)keys.data, keys.size * sizeof(Entity));
    memcpy((void*)(entities.data + rubies.size + keys.size), (void*)monsters.data, monsters.size * sizeof(Entity));
    entities.data[entities.size - 1] = endPortalEntity;
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
    if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 1024) == -1)
    {
        printf ("SDL_mixer could not open audio! SDL_Error: %s\n", SDL_GetError());
        return false;
    }
    if (Mix_Init(MIX_INIT_OGG) != MIX_INIT_OGG)
    {
        printf ("SDL_mixer could not initialize! SDL_Error: %s\n", SDL_GetError());
        return false;
    }
    //Set screen size depending on aspect ratio
    SDL_DisplayMode displayMode;
    SDL_GetCurrentDisplayMode(0, &displayMode);
    SCREEN_WIDTH = (int)SCREEN_HEIGHT * ((float)displayMode.w / (float)displayMode.h);
    *window = SDL_CreateWindow(
        "Oubliette", SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT,
        0);
    if (*window == NULL)
    {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    *renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_PRESENTVSYNC);
    return true;
}

bool loadLevel(EntityArray* entities, Player* player,
    PlayerData* playerData, EntityTemplate* rubyTemplate,
    EntityTemplate* keyTemplate, EntityTemplate* monsterTemplate, EntityTemplate* levelEndPortal)
{
    char nextLevelFilePath[LEVEL_FILE_PATH_MAX_LEN];
    sprintf(nextLevelFilePath, "res/levels/level%d.lvl", playerData->levelNumber);
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
        (*entities) = initLevel(player, playerData, rubyTemplate, keyTemplate, monsterTemplate, levelEndPortal);
        return true;
    }
    else return false;
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
    bool* paused = args[0];
    *paused = false;
}

void onLevelEndTransitionEnd(void** args, int length)
{
    static int frameCounter = 0;
    frameCounter++;

    PlayerData*     playerData =        args[0];

    void***  transitionArgs =            args[1];
    void***  levelStartTransitionArgs =  args[2];
    void (**onTransitionDone) (void**, int) = args[3];
    void (*onLevelStartTransitionEnd) (void**, int) = args[4];
    bool* shouldReloadLevel = args[5];
    int* transitionDirection = args[6];
    //ImageManager* images = args[7]; // images is global, no need to pass.
    SDL_Texture** screenTexture = args[8];
    SDL_Renderer** renderer = args[9];
    SpriteFont* spriteFont = args[10];

    char nextLevelFilePath[LEVEL_FILE_PATH_MAX_LEN];
    sprintf(nextLevelFilePath, "res/levels/level%d.lvl", playerData->levelNumber + 1);
    if(fileExists(nextLevelFilePath))
    {

        uint32_t fadeColour = 0x000000;
        SDL_Rect topRect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
        drawRect(topRect, fadeColour);
        SDL_Rect tmpRect = {SCREEN_WIDTH / 2 - 2 * images.rubySprite->w + 2, SCREEN_HEIGHT / 2 - 4, images.rubySprite->w, images.rubySprite->h};
        blitToPixelBuffer(images.rubySprite, tmpRect, 0);

        {char levelEndText[32];
        sprintf(levelEndText, "LEVEL %d COMPLETE", playerData->levelNumber + 1);
        SDL_Rect textRect = { SCREEN_WIDTH / 2, SCREEN_HEIGHT/3, 0, 0 };
        drawText(levelEndText, textRect, 0xFF7A0927, *spriteFont, true);}

        {char rubyCountStr[32];
        sprintf(rubyCountStr, "%d/%d", playerData->rubiesCollected, getTotalLevelRubies());
        SDL_Rect textRect = { SCREEN_WIDTH / 2 - 2, SCREEN_HEIGHT/2, 0, 0 };
        drawText(rubyCountStr, textRect, 0xFF7A0927, *spriteFont, false);}

        //Render the pixel buffer to the screen
        SDL_UpdateTexture(*screenTexture, NULL, getPixelBuffer()->pixels, SCREEN_WIDTH * sizeof(uint32_t));
        SDL_RenderCopy(*renderer, *screenTexture, NULL, NULL);
        SDL_RenderPresent(*renderer);
        SDL_Delay(4000);

        (*playerData).levelNumber++;
        (*playerData).totalRubiesCollected += (*playerData).rubiesCollected;
        (*playerData).totalRubies += getTotalLevelRubies();
        //loadLevel(entities, player, playerData, rubyTemplate, keyTemplate, monsterTemplate);
        *shouldReloadLevel = true;
        *transitionArgs = (void**)levelStartTransitionArgs;
        *onTransitionDone = onLevelStartTransitionEnd;
        *transitionDirection = -1;
    }
    else
    {
        (*playerData).levelNumber++;
        (*playerData).totalRubiesCollected += (*playerData).rubiesCollected;
        (*playerData).totalRubies += getTotalLevelRubies();

        uint32_t fadeColour = 0x000000;
        SDL_Rect topRect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
        drawRect(topRect, fadeColour);

        SDL_Rect tmpRect = {SCREEN_WIDTH / 2 - 2 * images.rubySprite->w + 2, SCREEN_HEIGHT / 2 - 4, images.rubySprite->w, images.rubySprite->h};
        blitToPixelBuffer(images.rubySprite, tmpRect, 0);

        {char levelEndText[32];
        sprintf(levelEndText, "THE END");
        SDL_Rect textRect = { SCREEN_WIDTH / 2, SCREEN_HEIGHT/3, 0, 0 };
        drawText(levelEndText, textRect, 0xFF7A0927, *spriteFont, true);}

        {char levelEndText[32];
        sprintf(levelEndText, "A GAME BY SEORAS MACDONALD");
        SDL_Rect textRect = { SCREEN_WIDTH / 2, (SCREEN_HEIGHT * 9/10), 0, 0 };
        drawText(levelEndText, textRect, 0xFF7A0927, *spriteFont, true);}

        {char rubyCountStr[32];
        sprintf(rubyCountStr, "%d/%d", playerData->totalRubiesCollected, playerData->totalRubies);
        SDL_Rect textRect = { SCREEN_WIDTH / 2 - 2, SCREEN_HEIGHT/2, 0, 0 };
        drawText(rubyCountStr, textRect, 0xFF7A0927, *spriteFont, false);}

        //Render the pixel buffer to the screen
        SDL_UpdateTexture(*screenTexture, NULL, getPixelBuffer()->pixels, SCREEN_WIDTH * sizeof(uint32_t));
        SDL_RenderCopy(*renderer, *screenTexture, NULL, NULL);
        SDL_RenderPresent(*renderer);
        SDL_Delay(8000);
        exit(0);
    }
}

int sign(int x) {
    return (x > 0) - (x < 0);
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
    spriteFont.sprite = IMG_Load("res/fonts/atari_font.png");
    spriteFont.sprite = SDL_ConvertSurfaceFormat(spriteFont.sprite, SDL_PIXELFORMAT_ARGB8888, 0);

    //Audio SHOULD EXTRACT TO SEPARATE FILE
    Mix_Music* gameBackgroundMusic = Mix_LoadMUS("res/music/thrum.ogg");
    Mix_Music* levelEndMusic = Mix_LoadMUS("res/music/thrum_outsync_double_reverse.ogg");
    Mix_Music* titleMusic = Mix_LoadMUS("res/music/title_menu.ogg");
    Mix_Chunk* rubySfx = Mix_LoadWAV("res/sfx/ruby_pickup.ogg");
    Mix_Chunk* keySfx = Mix_LoadWAV("res/sfx/key_pickup.ogg");
    Mix_Chunk* unlockDoorSfx = Mix_LoadWAV("res/sfx/unlock_door.ogg");
    Mix_Chunk* lockedDoorSfx = Mix_LoadWAV("res/sfx/locked_door.ogg");
    Mix_Chunk* playerFootstepSfx = Mix_LoadWAV("res/sfx/player_footstep.ogg");
    Mix_Chunk* roarSfx = Mix_LoadWAV("res/sfx/monster_roar.ogg");
    Mix_Chunk* playerFinishedLevelSfx = Mix_LoadWAV("res/sfx/player_finished_level.ogg");
    Mix_Chunk* secretDoorSfx = Mix_LoadWAV("res/sfx/secret_door.ogg");
    Mix_Chunk* playerDeathSfx = Mix_LoadWAV("res/sfx/player_death.ogg");

    Mix_VolumeChunk(roarSfx, 128);
    Mix_VolumeChunk(playerFootstepSfx, 40);

    //Create player
    Player player = { .width=32, .height=32, .footstepSoundChannel=-1};
    PlayerData playerData = { .levelNumber=0 };
    EntityTemplate rubyTemplate = { .sprite=images.rubySprite, .width=16, .height=16, .spriteWidth=16, .spriteHeight=16, .type=ENTITY_TYPE_RUBY };
    EntityTemplate keyTemplate = { .sprite=images.keySprite, .width=16, .height=16, .spriteWidth=16, .spriteHeight=16, .type=ENTITY_TYPE_KEY};
    EntityTemplate monsterTemplate = { .sprite=images.monsterSprite, .width=64, .height=64, .spriteWidth=64, .spriteHeight=64, .type=ENTITY_TYPE_MONSTER};
    EntityTemplate endPortalTemplate = { .sprite=images.levelEndPortal, .width=64, .height=64, .spriteWidth=64, .spriteHeight=64, .animationSpeed=30, .type=ENTITY_TYPE_PORTAL};

    //Init level
    EntityArray entities = {0};
    loadLevel(&entities, &player, &playerData, &rubyTemplate, &keyTemplate, &monsterTemplate, &endPortalTemplate);
    //TEMP START PLAYING MUSIC
    Mix_FadeInMusic(titleMusic, -1, 1000);

    bool paused = true;
    bool shouldReloadLevel = false;

    //should encapsulate in transition object
    bool deathEffectActive = false;
    int deathEffectCounter = 0;
    int deathEffectDirection = 1;

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
        onLevelStartTransitionEnd, &shouldReloadLevel, &transitionDirection, &images,
        &screenTexture, &renderer, &spriteFont};

    //Get input devices' states
    SDL_Joystick* gamePad = SDL_JoystickOpen(0);
    const uint8_t* keyState = SDL_GetKeyboardState(NULL);

    bool running = true;

    //Main Menu Loop (Complete hack but whatever)
    while(running) {
        int frameStartTime = SDL_GetTicks();

        //SDL Event Loop
        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            switch(e.type)
            {
            case SDL_QUIT:
                exit(0);
                break;
            case SDL_KEYDOWN:
                switch(e.key.keysym.sym)
                {
                    case SDLK_RETURN:
                        if (e.key.keysym.mod & KMOD_ALT)
                        {
                            toggleFullscreen(window);
                        } else {
                            running = false;
                        }
                        break;
                    case SDLK_ESCAPE:
                        exit(0);
                        break;
                }
                break;
            }
        }
        {
                SDL_Rect tmpRect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
            blitToPixelBuffer(images.mainMenuBack, tmpRect, 0);
        }
        {
                SDL_Rect tmpRect = {(SCREEN_WIDTH - images.mainMenuTitle->w) / 2,
                SCREEN_HEIGHT / 5, images.mainMenuTitle->w, images.mainMenuTitle->h};
            blitToPixelBuffer(images.mainMenuTitle, tmpRect, 0);
        }
        {
                SDL_Rect tmpRect = {2.f * sinf(SDL_GetTicks() / 600.0) + (SCREEN_WIDTH - images.mainMenuStartButton->w) / 2,
                6.f * sinf(SDL_GetTicks() / 300.0) + (SCREEN_HEIGHT * 4) / 5, images.mainMenuStartButton->w, images.mainMenuStartButton->h};
            blitToPixelBuffer(images.mainMenuStartButton, tmpRect, 0);
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
        //currentFps = 1000/(SDL_GetTicks() - frameStartTime);
    }
    Mix_FadeOutMusic(1000);
    Mix_FadeInMusic(gameBackgroundMusic, -1, 1000);

    running = true;
    //Main Loop ====
    while(running)
    {
        int frameStartTime = SDL_GetTicks();

        if (shouldReloadLevel)
        {
            Mix_HaltMusic();
            Mix_PlayMusic(gameBackgroundMusic, -1);
            loadLevel(&entities, &player, &playerData, &rubyTemplate, &keyTemplate, &monsterTemplate, &endPortalTemplate);
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
                    case SDLK_ESCAPE:
                        running = false;
                        break;
                    case SDLK_SPACE: {
                        Vector2 actionTile = player.pos;
                        actionTile.x += cosf(player.rotation) * TILE_DIMS;
                        actionTile.y += sinf(player.rotation) * TILE_DIMS;
                        actionTile = posToTileCoord(actionTile);

                        char tile = getLevelTile(posVecToIndex(actionTile));
                        if (tile == TILE_SECRET_DOOR)
                        {
                            setTileTo(posVecToIndex(actionTile), TILE_FLOOR);
                            Mix_PlayChannel(-1, secretDoorSfx, 0);
                        }
                        else if ((tile == TILE_DOOR0 && playerData.keysCollected[0] == true) ||
                            (tile == TILE_DOOR1 && playerData.keysCollected[1] == true) ||
                            (tile == TILE_DOOR2 && playerData.keysCollected[2] == true) ||
                            (tile == TILE_DOOR3 && playerData.keysCollected[3] == true))
                        {
                            setTileTo(posVecToIndex(actionTile), TILE_FLOOR);
                            Mix_PlayChannel(-1, unlockDoorSfx, 0);
                        }
                        else if (tile == TILE_DOOR0 ||
                            tile == TILE_DOOR1 ||
                            tile == TILE_DOOR2 ||
                            tile == TILE_DOOR3)
                        {
                            Mix_PlayChannel(-1, lockedDoorSfx, 0);
                        }
                    }
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
        //Joystick input
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
            if (keyState[SDL_SCANCODE_R]) {
                shouldReloadLevel = true;
            }

            //Normalise moveVector
            moveVector = vec2Unit(moveVector);
            player.pos.x += moveVector.x * moveVel;
            player.pos.y += moveVector.y * moveVel;
            if (moveVector.x == 0 && moveVector.y == 0)
            {
                //Mix_HaltChannel(player.footstepSoundChannel);
            }
            else if (!Mix_Playing(player.footstepSoundChannel))
            {
                player.footstepSoundChannel = Mix_PlayChannel(-1, playerFootstepSfx, 0);
            }

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
                Mix_PlayChannel(-1, playerFinishedLevelSfx, 0);
                Mix_HaltMusic();
                Mix_PlayMusic(levelEndMusic, -1);

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
                SDL_Rect playerRect = { player.pos.x - player.width / 2, player.pos.y - player.height / 2, player.width, player.height };
                Entity *entity = &entities.data[i];
                SDL_Rect entityRect = { entity->pos.x - entity->base->width / 2, entity->pos.y - entity->base->height / 2, entity->base->width, entity->base->height };

                //update animation
                entities.data[i].xClipCounter++;
                if (entities.data[i].xClipCounter > entities.data[i].base->animationSpeed)
                {
                    entities.data[i].xClipCounter = 0;
                    entities.data[i].xClip++;
                    if (entities.data[i].xClip * entities.data[i].base->spriteWidth >= entities.data[i].base->width)
                    {
                        entities.data[i].xClip = 0;
                    }
                }

                switch(entity->base->type)
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
                            //Play SFX
                            Mix_PlayChannel(-1, keySfx, 0);
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
                        Monster* monster = (Monster*)entity->sub;
                        /*-----------------------------
                         *Check for aiState transitions
                         *---------------------------*/

                        if (rectsIntersect(playerRect, entityRect))
                        {
                            //shouldReloadLevel = true;
                            paused = true;
                            deathEffectActive = true;
                            deathEffectCounter = 0;
                            deathEffectDirection = 1;
                            Mix_PlayChannel(-1, playerDeathSfx, 0);
                        }
                        /*--------------------------------
                         *Check if monster has seen player
                         *------------------------------*/
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
                                /*------------------------------------
                                 *Cast ray and check for walls between
                                 *player and monster
                                 *----------------------------------*/
                                bool isWallBetween = false;
                                {
                                    int playerPos[] = {player.pos.x, player.pos.y};
                                    int cursorPoint[] = {entity->pos.x, entity->pos.y};
                                    int dif[] = {player.pos.x - entity->pos.x, player.pos.y - entity->pos.y};
                                    int maxAxis = 1;
                                    int minAxis = 0;
                                    if (abs(dif[0]) > abs(dif[1]))
                                    {
                                        maxAxis = 0;
                                        minAxis = 1;
                                    }
                                    if (dif[maxAxis] != 0)
                                    {
                                        float gradient = (float)dif[minAxis] / (float)dif[maxAxis];
                                        float step = dif[maxAxis] < 0 ? - TILE_DIMS : TILE_DIMS;
                                        do
                                        {
                                            int tileIndex = posToTileIndex(cursorPoint[0], cursorPoint[1]);
                                            if (!isTileIndexValid(tileIndex)) break;
                                            if (isTileSolid(tileIndex))
                                            {
                                                isWallBetween = true;
                                                break;
                                            }
                                            cursorPoint[maxAxis] += step;
                                            cursorPoint[minAxis] += step * gradient;
                                        } while (sign(dif[maxAxis]) == sign(playerPos[maxAxis] - cursorPoint[maxAxis]));
                                    }
                                }

                                if (!isWallBetween)
                                {
                                    switch(monster->aiState)
                                    {
                                        case AI_PATROL:
                                        {
                                            monster->aiState = AI_CHASE;
                                            monster->giveUpChaseTimer.active = true;
                                            monster->giveUpChaseTimer.endTime = SDL_GetTicks() + monsterChaseTimeLimit;
                                            Mix_PlayChannel(-1, roarSfx, 0);
                                            break;
                                        }
                                        case AI_CHASE:
                                        {
                                            monster->giveUpChaseTimer.endTime = SDL_GetTicks() + monsterChaseTimeLimit;
                                            break;
                                        }
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
                                if (!Mix_Playing(monster->roarSoundChannel) && oneInXChance(60))
                                {
                                    monster->roarSoundChannel = Mix_PlayChannel(-1, roarSfx, 0);
                                }
                                Vector2Int tmp = { .x=player.pos.x / TILE_DIMS, .y=player.pos.y / TILE_DIMS};
                                monster->targetTile = tmp;
                                break;
                            }
                        }

                        /*---------
                         * Pathfind
                         *-------*/
                        monsterMove(&entities.data[i]);
                        int distVolume = distanceFormula(player.pos, entity->pos) / 4;
                        Mix_SetPosition(
                            monster->roarSoundChannel,
                            (constrainAngle(getMonsterAngle(entity) - player.rotation) + M_PI) * (360.0 / (2 * M_PI)),
                            distVolume > 255 ? 255 : distVolume
                        );
                        break;
                    }
                    case ENTITY_TYPE_PORTAL:
                        break;
                }
            }
        }
        //Update screen fade
        if (transitionDirection != 0)
        {
            //Mix_HaltChannel(player.footstepSoundChannel);
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
            Rectangle rubyImageRect = { SCREEN_WIDTH/8 - 1.5 * images.rubySprite->w, SCREEN_HEIGHT/16 - 2, 11, 11 };
            blitToPixelBuffer(images.rubySprite, rubyImageRect, 0);
            char rubyCountStr[32];
            sprintf(rubyCountStr, "%d/%d", playerData.rubiesCollected, getTotalLevelRubies());
            SDL_Rect textRect = { SCREEN_WIDTH/8, SCREEN_HEIGHT/16, 0, 0 };
            drawText(rubyCountStr, textRect, 0xFF7A0927, spriteFont, false);
        }

        //Draw keys collected
        {
            for (int i = 0; i < MAX_KEYS; i++)
            {
                if (playerData.keysCollected[i] == true)
                {
                    Rectangle keyImageRect = { SCREEN_WIDTH/3 + i * 16, SCREEN_HEIGHT/16, 11, 11 };
                    blitToPixelBuffer(images.keySprite, keyImageRect, keyColorsTemp[i]);
                }
            }
        }

        //Draw compass
        Rectangle compassRect = { (SCREEN_WIDTH*7)/8, SCREEN_HEIGHT/8 - 8, 32, 32 };
        rotatedBlitToPixelBuffer(images.compass, compassRect, 0, -player.rotation);

        //Draw screen fade to black
        {
            uint32_t fadeColour = 0x000000;
            SDL_Rect topRect = {0, 0, SCREEN_WIDTH, (SCREEN_HEIGHT / 2) * transitionFraction};
            SDL_Rect botRect = {0, SCREEN_HEIGHT - (SCREEN_HEIGHT / 2) * transitionFraction,
                SCREEN_WIDTH, 1 + (SCREEN_HEIGHT / 2) * transitionFraction};
            drawRect(topRect, fadeColour);
            drawRect(botRect, fadeColour);
        }

        //Draw death effect
        if (deathEffectActive)
        {
            pixelateScreen((deathEffectCounter / 3) + 1);
            fadeToColor(0x00401010, (float)deathEffectCounter / ((SCREEN_WIDTH * 3) / 8.f));

            if (deathEffectDirection == 1)
            {
                deathEffectCounter++;
                if ((deathEffectCounter / 3) > SCREEN_WIDTH / 8) {
                    deathEffectDirection = -1;
                    shouldReloadLevel = true;
                }
            }
            else
            {
                deathEffectCounter--;
                if (deathEffectCounter == 1)
                {
                    deathEffectActive = false;
                    paused = false;
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
        //currentFps = 1000/(SDL_GetTicks() - frameStartTime);
    }
    return 0;
}
