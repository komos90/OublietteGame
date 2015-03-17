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
#elif _WIN32
    #include <SDL.h>
    #include <SDL_image.h> 
#endif

#include "engine_types.h"
#include "load_level.h"
#include "gfx_engine.h"

static const int SCREEN_WIDTH  = 213;//854;
static const int SCREEN_HEIGHT = 120;//480;

//Temp Globals

int main( int argc, char* args[] )
{
    //Initialise SDL ====
    if( SDL_Init( SDL_INIT_EVERYTHING) < 0 || IMG_Init(IMG_INIT_PNG) < 0)
    {
        printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
        return 0;
    }
    SDL_Window* window = SDL_CreateWindow(
        "The Caves - Level 1", SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, 
        SDL_WINDOW_FULLSCREEN_DESKTOP);
    if (window == NULL)
    {
        printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
        return 0;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_Texture* screenTexture = SDL_CreateTexture(
        renderer, SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH,
        SCREEN_HEIGHT);
    //Grab cursor
    SDL_SetRelativeMouseMode(true);

    //Allocate pixel buffer
    PixelBuffer* pixelBuffer = createPixelBuffer(SCREEN_WIDTH, SCREEN_HEIGHT);

    //Load level from file and add level entities to entity list
    Level level = loadLevel("../res/levels/level0.lvl");

    //Load texture
    // TODO Generalise
    SDL_Surface* caveTexture = IMG_Load("../res/textures/cave.png");
    caveTexture = SDL_ConvertSurfaceFormat(caveTexture, SDL_PIXELFORMAT_ARGB8888, 0);
    SDL_Surface* rubySprite = IMG_Load("../res/sprites/ruby.png");
    rubySprite = SDL_ConvertSurfaceFormat(rubySprite, SDL_PIXELFORMAT_ARGB8888, 0);

    //Load sprite font
    SpriteFont spriteFont = { .charW=8, .charH=8 };
    spriteFont.sprite = IMG_Load("../res/fonts/atari_font.png");
    spriteFont.sprite = SDL_ConvertSurfaceFormat(spriteFont.sprite, SDL_PIXELFORMAT_ARGB8888, 0);

    //Create player
    Player player = { .width=32, .height=32, .pos=getPlayerStartPos(level) };
    PlayerData playerData = { .levelNumber=0 };

    EntityTemplate rubyTemplate = { .sprite = rubySprite, .width=32, .height=32 };
    EntityArray entities = getLevelRubies(level, &rubyTemplate);

    //Get input devices' states
    SDL_Joystick* gamePad = SDL_JoystickOpen(0);
    const uint8_t* keyState = SDL_GetKeyboardState(NULL);    
    
    bool running = true;
    int currentFps = 0;
    //Main Loop ====
    while(running) 
    {
        int frameStartTime = SDL_GetTicks();
        
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
                        {
                            uint32_t flags = SDL_GetWindowFlags(window);
                            if (flags & SDL_WINDOW_FULLSCREEN ||
                                flags & SDL_WINDOW_FULLSCREEN_DESKTOP)
                            {
                                SDL_SetWindowFullscreen(window, 0);
                            }
                            else
                            {
                                SDL_SetWindowFullscreen(
                                    window, SDL_WINDOW_FULLSCREEN_DESKTOP);
                            }
                        }
                        break;
                }
                break;
            case SDL_MOUSEMOTION:
                player.rotation += e.motion.xrel * 0.001;
            }
        }
        //Joystick input
        //HACK Should decouple Input and Game Logic
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
            //Normalise moveVector
            moveVector = vec2Unit(moveVector);
            player.pos.x += moveVector.x * moveVel;
            player.pos.y += moveVector.y * moveVel;

            //Collision
            if (isTileSolid(posToTileIndex(player.pos.x, player.pos.y, level), level))
            {
                player.pos = oldPlayerPos;
            }
        }

        //Game Logic ====
        //Check ruby collision
        for (int i = 0; i < entities.size; i++)
        {
            SDL_Rect playerRect = { player.pos.x, player.pos.y, player.width, player.height };
            Entity ruby = entities.data[i];
            SDL_Rect rubyRect = { ruby.pos.x, ruby.pos.y, ruby.base->width, ruby.base->height };
            if (rectsIntersect(playerRect, rubyRect))
            {
                playerData.rubiesCollected++;
                //Remove ruby
                entities.data[i] = entities.data[entities.size - 1];
                entities.size--;
            }
        }

        //Check for level end tile

        //Draw ====
        //Send game entities to gfx engine to be rendered 
        draw(player, level, caveTexture, entities);
        //Draw test text
        {
            char rubyCountStr[32];
            sprintf(rubyCountStr, "%d/%d", playerData.rubiesCollected, level.rubyCount);
            SDL_Rect textRect = { SCREEN_WIDTH/8, SCREEN_HEIGHT/16, 0, 0 };
            drawText(rubyCountStr, textRect, 0xFF7A0927, spriteFont);
        }
        {
            char fpsDisplayStr[32];
            sprintf(fpsDisplayStr, "Fps: %d", currentFps);
            SDL_Rect textRect = {0, SCREEN_HEIGHT - 16, 0, 0 };
            drawText(fpsDisplayStr, textRect, 0xFFFF0000, spriteFont);
        }

        //Render the pixel buffer to the screen
        SDL_UpdateTexture(screenTexture, NULL, pixelBuffer->pixels, SCREEN_WIDTH * sizeof(uint32_t));        
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
