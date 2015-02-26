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

#ifdef __linux__
	#include <SDL2/SDL.h>
#elif _WIN32
	#include <SDL.h>
#endif


const int SCREEN_WIDTH = 640;//1366;//300;//640;
const int SCREEN_HEIGHT = 480;//768;//300;//480;

//Temp Globals
static uint32_t globalCounter;


typedef struct 
{
	uint32_t* pixels;
	int width;
	int height;
} PixelBuffer;

typedef struct 
{
	float x;
	float y;
	float z;
} Vector3;

typedef struct
{
	Vector3 vectors[3];
} Triangle;

typedef struct
{
	Vector3 origin;
	int polyCount;
	Triangle* polygons;
} Mesh;

void drawRect(int x, int y, int w, int h, uint32_t color, PixelBuffer* pixelBuffer)
{
	for (int yy = y; yy < y + h; ++yy)
	{
		for (int xx = x; xx < x + w; ++xx)
		{
			pixelBuffer->pixels[yy * pixelBuffer->width + xx] = color;
		}
	}
}

void drawVector(Vector3 vector, uint32_t color, PixelBuffer* pixelBuffer)
{
	if (vector.x >= 0 && vector.x < pixelBuffer->width &&
		vector.y >= 0 && vector.y < pixelBuffer->height)
	{
		pixelBuffer->pixels[(int)vector.y  * pixelBuffer->width + (int)vector.x] = color;
	}
}

void drawLine(Vector3 start, Vector3 end, uint32_t color, PixelBuffer* pixelBuffer)
{
	int x0 = (int)start.x;
    int y0 = (int)start.y;
    int x1 = (int)end.x;
    int y1 = (int)end.y;
    
	//Distance between x0 and x1, y0 and y1
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
	//Determin whether to step backwards or forwards through the line
   	int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;

    int err = dx - dy;

    for(;;)
	{
		//Draw the current pixel
		Vector3 tmp = {x0, y0};
        drawVector(tmp, color, pixelBuffer);

		//Break if we reach the end of the line.
        if ((x0 == x1) && (y0 == y1)) break;

        int e2 = 2 * err;
		//Step x
        if (e2 > -dy)
		{ 
			err -= dy;
			x0 += sx;
		}
		//Step y
        if (e2 < dx)
		{
			err += dx;
			y0 += sy;
		}
    }
}

void draw(PixelBuffer* pixelBuffer, Mesh* cube)
{
	//Bob cube
	cube->origin.y = 240 + 100 * sinf(globalCounter / 100.f);
	
	// Rotation angles, y-axis then x-axis
	float a = 0.01;
	float b = 0.02;
	uint32_t lineColor = 0xffffffff;
	for (int i = 0; i < cube->polyCount; i++)
	{	
		Triangle* poly = &cube->polygons[i];
		Triangle displayPoly;

		for (int j = 0; j < 3; j++)
		{
			//Rotate =====
			/*
			 * 3d y axis rotation matrix = |cos(a)  0 sin(a)|
			 *							   | 0      1     0 |
			 * 							   |-sin(a) 0 cos(a)|
			 */
			//rotate around y axis
			Vector3 tmp;
			tmp.x = cos(a) * poly->vectors[j].x + sin(a) * poly->vectors[j].z;
			tmp.y = poly->vectors[j].y;
			tmp.z = (-sin(a)) * poly->vectors[j].x + cos(a) * poly->vectors[j].z;
			poly->vectors[j] = tmp;

			//rotate around x axis
			tmp.x = poly->vectors[j].x;
			tmp.y = cos(b) * poly->vectors[j].y + (-sin(b)) * poly->vectors[j].z;
			tmp.z = sin(b) * poly->vectors[j].y + cos(b) * poly->vectors[j].z;
			poly->vectors[j] = tmp;

			//Shift display poly to world coords
			displayPoly.vectors[j].x = poly->vectors[j].x + cube->origin.x;		
			displayPoly.vectors[j].y = poly->vectors[j].y + cube->origin.y;		
			displayPoly.vectors[j].z = poly->vectors[j].z + cube->origin.z;
		}
		drawLine(displayPoly.vectors[0], displayPoly.vectors[1], lineColor, pixelBuffer);		
		drawLine(displayPoly.vectors[1], displayPoly.vectors[2], lineColor, pixelBuffer);		
		drawLine(displayPoly.vectors[2], displayPoly.vectors[0], lineColor, pixelBuffer);		
	}
}

int main( int argc, char* args[] )
{
	SDL_Window* window = NULL;
	SDL_Renderer* renderer = NULL;
	SDL_Texture* screenTexture = NULL;
	uint32_t* pixels = NULL;
	bool running = true;

	if( SDL_Init( SDL_INIT_EVERYTHING) < 0 )
	{
		printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
		return 0;
	}

	window = SDL_CreateWindow(
		"Pixel buffer Playground :P", SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT,
		SDL_WINDOW_FULLSCREEN_DESKTOP);
	if( window == NULL )
	{
		printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
		return 0;
	}

	renderer = SDL_CreateRenderer(window, -1, 0);
	screenTexture = SDL_CreateTexture(
		renderer, SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH,
		SCREEN_HEIGHT);

	pixels = (uint32_t*) malloc(SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint32_t));
	PixelBuffer pixelBuffer = {pixels, SCREEN_WIDTH, SCREEN_HEIGHT};

	//temp
	Mesh cube;
	Vector3 meshOrigin = {320, 240, 150};
	cube.origin = meshOrigin;
	cube.polyCount = 12;
	cube.polygons = malloc(cube.polyCount * sizeof(Triangle));
	
	//Vertices
	Vector3 v0 = {50, -50, -50};
	Vector3 v1 = {50, 50, -50};
	Vector3 v2 = {-50, 50, -50};
	Vector3 v3 = {-50, -50, -50};
	Vector3 v4 = {50, -50, 50};
	Vector3 v5 = {50, 50, 50};
	Vector3 v6 = {-50, 50, 50};
	Vector3 v7 = {-50, -50, 50};
	
	//Polygons
	cube.polygons[0].vectors[0] = v0;
	cube.polygons[0].vectors[1] = v1;
	cube.polygons[0].vectors[2] = v2;
	cube.polygons[1].vectors[0] = v3;
	cube.polygons[1].vectors[1] = v0;
	cube.polygons[1].vectors[2] = v2;
	cube.polygons[2].vectors[0] = v7;
	cube.polygons[2].vectors[1] = v6;
	cube.polygons[2].vectors[2] = v5;
	cube.polygons[3].vectors[0] = v4;
	cube.polygons[3].vectors[1] = v7;
	cube.polygons[3].vectors[2] = v5;
	cube.polygons[4].vectors[0] = v4;
	cube.polygons[4].vectors[1] = v5;
	cube.polygons[4].vectors[2] = v1;
	cube.polygons[5].vectors[0] = v0;
	cube.polygons[5].vectors[1] = v4;
	cube.polygons[5].vectors[2] = v1;
	cube.polygons[6].vectors[0] = v3;
	cube.polygons[6].vectors[1] = v2;
	cube.polygons[6].vectors[2] = v6;
	cube.polygons[7].vectors[0] = v7;
	cube.polygons[7].vectors[1] = v3;
	cube.polygons[7].vectors[2] = v6;
	cube.polygons[8].vectors[0] = v7;
	cube.polygons[8].vectors[1] = v4;
	cube.polygons[8].vectors[2] = v0;
	cube.polygons[9].vectors[0] = v3;
	cube.polygons[9].vectors[1] = v7;
	cube.polygons[9].vectors[2] = v0;
	cube.polygons[10].vectors[0] = v6;
	cube.polygons[10].vectors[1] = v5;
	cube.polygons[10].vectors[2] = v1;
	cube.polygons[11].vectors[0] = v2;
	cube.polygons[11].vectors[1] = v6;
	cube.polygons[11].vectors[2] = v1;
	
	//Main Loop ====
	while(running)	
	{
		int curTime = SDL_GetTicks();
		
		//SDL Event Loop
		SDL_Event event;
    	while (SDL_PollEvent(&event))
		{
			switch(event.type)
			{
			case SDL_QUIT:
				running = false;
				break;
			}
    	}
		globalCounter++;

		// Where all the drawing happens
		draw(&pixelBuffer, &cube);

		//Rendering pixel buffer to the screen
		SDL_UpdateTexture(screenTexture, NULL, pixelBuffer.pixels, SCREEN_WIDTH * sizeof(uint32_t));		
		SDL_RenderCopy(renderer, screenTexture, NULL, NULL);
		SDL_RenderPresent(renderer);
		//Clear the pixel buffer
		memset((void*)pixelBuffer.pixels, 0, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint32_t));

		//Lock to 60 fps
		int delta = SDL_GetTicks() - curTime;
		if (delta < 1000/60)
		{
			SDL_Delay(1000/60 - delta);
		}
	}
	return 0;
}
