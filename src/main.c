/*
Hand coded 3d software renderer project.
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
    #define M_PI 3.14159265358979323846
#elif _WIN32
    #include <SDL.h>
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
    if( SDL_Init( SDL_INIT_EVERYTHING) < 0 )
    {
        printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
        return 0;
    }
    SDL_Window* window = SDL_CreateWindow(
        "Pixel buffer Playground :P", SDL_WINDOWPOS_UNDEFINED,
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
    //grab cursor
    SDL_SetRelativeMouseMode(true);

    //Allocate pixel buffer
    uint32_t* pixels = (uint32_t*) malloc(SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint32_t));
    PixelBuffer pixelBuffer = {pixels, SCREEN_WIDTH, SCREEN_HEIGHT};

    //Load level from file and add level entities to entity list
    Level level = loadLevel("../res/levels/level2.lvl");

    //Create player
    Entity player = {.pos={896, 256}};

    //Get input devices' states
    SDL_Joystick* gamePad = SDL_JoystickOpen(0);
    const uint8_t* keyState = SDL_GetKeyboardState(NULL);    
    
    bool running = true;
    bool paused = false;

    //tmp frame rate counter
    int framerateDisplayDelayCounter = 0;

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
                    case SDLK_SPACE:
                        paused = !paused;
                        break;
                }
                break;
            case SDL_MOUSEMOTION:
                player.rotation += e.motion.xrel * 0.001;
            }
        }
        //Joystick input
        //HACK Should decouple Input and Game Logic
        {
            const int JOYSTICK_DEAD_ZONE = 8000;
            int moveVel = 3;
            if( SDL_JoystickGetAxis(gamePad, 0) < -JOYSTICK_DEAD_ZONE )
            {
                player.pos.y += moveVel * cosf(player.rotation + M_PI/2);
                player.pos.x += moveVel * sinf(player.rotation + M_PI/2);
            }
            //Right of dead zone
            else if( SDL_JoystickGetAxis(gamePad, 0) > JOYSTICK_DEAD_ZONE )
            {
                player.pos.y += moveVel * cosf(player.rotation - M_PI/2);
                player.pos.x += moveVel * sinf(player.rotation - M_PI/2);
            }
            //Left of dead zone
            if( SDL_JoystickGetAxis(gamePad, 1) < -JOYSTICK_DEAD_ZONE )
            {
                player.pos.y += moveVel * cosf(player.rotation);
                player.pos.x += moveVel * sinf(player.rotation);
            }
            //Right of dead zone
            else if( SDL_JoystickGetAxis(gamePad, 1) > JOYSTICK_DEAD_ZONE )
            {
                player.pos.y += moveVel * cosf(player.rotation + M_PI);
                player.pos.x += moveVel * sinf(player.rotation + M_PI);
            }
            //Left of dead zone
            if( SDL_JoystickGetAxis(gamePad, 2) < -JOYSTICK_DEAD_ZONE )
            {
                player.rotation += 0.04 * -SDL_JoystickGetAxis(gamePad, 2) / 32767.f;
            }
            //Right of dead zone
            else if( SDL_JoystickGetAxis(gamePad, 2) > JOYSTICK_DEAD_ZONE )
            {
                player.rotation -= 0.04 * SDL_JoystickGetAxis(gamePad, 2) / 32767.f;
            }
        }
        //Keyboard Input
        { 
            Vector2 oldPlayerPos = player.pos;
            int moveVel = 3; 
            if (keyState[SDL_SCANCODE_A])
            {
                player.pos.x += moveVel * cosf(player.rotation - M_PI/2);
                player.pos.y += moveVel * sinf(player.rotation - M_PI/2);
            }
            if (keyState[SDL_SCANCODE_D])
            {
                player.pos.x += moveVel * cosf(player.rotation + M_PI/2);
                player.pos.y += moveVel * sinf(player.rotation + M_PI/2);
            }
            if (keyState[SDL_SCANCODE_S])
            {
                player.pos.x += moveVel * cosf(player.rotation + M_PI);
                player.pos.y += moveVel * sinf(player.rotation + M_PI);
            }
            if (keyState[SDL_SCANCODE_W])
            {
                player.pos.x += moveVel * cosf(player.rotation);
                player.pos.y += moveVel * sinf(player.rotation);
            }
            if (keyState[SDL_SCANCODE_LEFT])
            {
                player.rotation += 0.02;
            }
            if (keyState[SDL_SCANCODE_RIGHT])
            {
                player.rotation -= 0.02;
            }
            if (level.data[posToTileIndex(player.pos.x, player.pos.y, level)] == '#') {
                player.pos = oldPlayerPos;
            }
        }

        //fix angle
        while (player.rotation > M_PI) player.rotation -= 2 * M_PI;
        while (player.rotation < -M_PI) player.rotation += 2 * M_PI;

        //Game Logic
        if(!paused)
        {
            
        }    
       
        //Send game entities to gfx engine to be rendered 
        draw(player, level, pixelBuffer);

        //Render the pixel buffer to the screen
        SDL_UpdateTexture(screenTexture, NULL, pixelBuffer.pixels, SCREEN_WIDTH * sizeof(uint32_t));        
        SDL_RenderCopy(renderer, screenTexture, NULL, NULL);
        SDL_RenderPresent(renderer);

        //Clear the pixel buffer
        memset((void*)pixelBuffer.pixels, 200, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint32_t));

        //Lock to 60 fps
        int delta = SDL_GetTicks() - frameStartTime;
        if (delta < 1000/60)
        {
            SDL_Delay(1000/60 - delta);
        }
        framerateDisplayDelayCounter = (framerateDisplayDelayCounter + 1) % 30;
        //if (framerateDisplayDelayCounter == 0)
            //SDL_Log("FPS: %f", 1000.f/(SDL_GetTicks() - frameStartTime));
    }
    return 0;
}
