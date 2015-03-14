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
    Level level = loadLevel("../res/levels/level1.lvl");

    //Load texture
    SDL_Surface* caveTexture = IMG_Load("../res/textures/cave.png");

    //Create player
    Entity player = {.pos={96, 96}};

    //Get input devices' states
    SDL_Joystick* gamePad = SDL_JoystickOpen(0);
    const uint8_t* keyState = SDL_GetKeyboardState(NULL);    
    
    bool running = true;

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
            if (level.data[posToTileIndex(player.pos.x, player.pos.y, level)] == '#') {
                player.pos = oldPlayerPos;
            }
        }

        //Game Logic

       
        //Send game entities to gfx engine to be rendered 
        draw(player, level, caveTexture);

        //Render the pixel buffer to the screen
        SDL_UpdateTexture(screenTexture, NULL, pixelBuffer->pixels, SCREEN_WIDTH * sizeof(uint32_t));        
        SDL_RenderCopy(renderer, screenTexture, NULL, NULL);
        SDL_RenderPresent(renderer);

        //Clear the pixel buffer
        //SHOULD BE IN GFX ENGINE
        

        //Lock to 60 fps
        int delta = SDL_GetTicks() - frameStartTime;
        if (delta < 1000/60)
        {
            SDL_Delay(1000/60 - delta);
        }
    }
    return 0;
}
