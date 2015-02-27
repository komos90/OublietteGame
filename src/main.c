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
	#define M_PI 3.14159265358979323846
#elif _WIN32
	#include <SDL.h>
#endif


const int SCREEN_WIDTH = 640;//1366;//300;//640;
const int SCREEN_HEIGHT = 480;//768;//300;//480;
//Projection Constants
const int VIEW_WIDTH = 640;
const int VIEW_HEIGHT = 480;
const int Z_FAR = 500;
const int Z_NEAR = 10;
const float FOV_X = 1280;//1.5f;
const float FOV_Y = 960;//1.5f;

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
	float values[16];
} Matrix4;

typedef struct
{
	Vector3 origin;
	int polyCount;
	Triangle* polygons;
} Mesh;

typedef struct
{
	Matrix4 rotation;
	Matrix4 translation;
	Matrix4 scale;
} Transform;

typedef struct
{
	Vector3 position;
	Vector3 rotation;
	Vector3 scale;
	Mesh mesh;
} Entity;

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

Matrix4 mulMatrix4(Matrix4 mat1, Matrix4 mat2)
{
	Matrix4 result;
	for (int i = 0; i < 16; i++)
	{
		result.values[i] = mat1.values[(i/4)*4 + 0] * mat2.values[i%4 + 0*4] +
						   mat1.values[(i/4)*4 + 1] * mat2.values[i%4 + 1*4] +
						   mat1.values[(i/4)*4 + 2] * mat2.values[i%4 + 2*4] +
						   mat1.values[(i/4)*4 + 3] * mat2.values[i%4 + 3*4];
	}
	return result;
}

Vector3 transform(Matrix4 matrix, Vector3 vector, float w) 
{
	Vector3 result = {0};
	result.x = matrix.values[0] * vector.x + matrix.values[1] * vector.y +
			   matrix.values[2] * vector.z + matrix.values[3] * w;
	result.y = matrix.values[4] * vector.x + matrix.values[5] * vector.y +
			   matrix.values[6] * vector.z + matrix.values[7] * w;
	result.z = matrix.values[8] * vector.x + matrix.values[9] * vector.y +
			   matrix.values[10] * vector.z + matrix.values[11] * w;
	w        = matrix.values[12] * vector.x + matrix.values[13] * vector.y +
			   matrix.values[14] * vector.z + matrix.values[15] * w;

	if (w != 0.f && w != 1.f) {
		result.x /= w;
		result.y /= w;
		result.z /= w;
	}

	return result;
}

void draw(PixelBuffer* pixelBuffer, Entity* camera, Entity** entityList, int entityCount)
{
	for (int k = 0; k < entityCount; k++) 
	{
		Entity* entity = entityList[k];
		// Rotation angles, y-axis then x-axis
		uint32_t lineColor = 0xffffffff;
		//SCale Matrix
		Matrix4 scaleMat;
		{
			Vector3 s = entity->scale;
			float tmp[16] = {  s.x, 0  , 0  , 0,
						   	   0  , s.y, 0  , 0,
						       0  , 0  , s.z, 0,
						       0  , 0  , 0  , 1};
			memcpy((void*) scaleMat.values, tmp, 16*sizeof(float));
		}
		//YAxis Rotation Matrix
		Matrix4 yRotMat;
		{
			float a = entity->rotation.y;
			float tmp[16] = { cosf(a), 0, sinf(a), 0,
						   	  0      , 1, 0      , 0,
						     -sinf(a), 0, cosf(a), 0,
						      0      , 0, 0      , 1};
			memcpy((void*) yRotMat.values, tmp, 16*sizeof(float));
		}
		//XAxis Rotation Matrix
		Matrix4 xRotMat;
		{
			float b = entity->rotation.x;
			float tmp[16] = {1,  0      , 0      , 0,
						 	 0,  cosf(b), sinf(b), 0,
						 	 0, -sinf(b), cosf(b), 0,
		 	 				 0,  0      , 0      , 1};
			memcpy((void*) xRotMat.values, tmp, 16*sizeof(float));
		}
		Matrix4 worldTranslate;
		{
			Vector3 p = entity->position;
			float tmp[16] = { 1, 0, 0, p.x,
						 	  0, 1, 0, p.y,
						 	  0, 0, 1, p.z,
		 	 				  0, 0, 0, 1  };
			memcpy((void*) worldTranslate.values, tmp, 16*sizeof(float));
		}
		//Camera Rotation Matrix
		Matrix4 cameraYRotation;
		{
			float a = camera->rotation.y;
			float tmp[16] = { cosf(-a), 0, sinf(-a), 0,
						   	  0       , 1, 0       , 0,
						     -sinf(-a), 0, cosf(-a), 0,
						      0       , 0, 0       , 1};
			memcpy((void*) cameraYRotation.values, tmp, 16*sizeof(float));
		}
		//Camera translate Matrix	
		Matrix4 cameraTranslate;
		{
			float tmp[16] = { 1, 0, 0, -camera->position.x,
						 	  0, 1, 0, -camera->position.y,
						 	  0, 0, 1, -camera->position.z,
		 	 				  0, 0, 0, 1        };
			memcpy((void*) cameraTranslate.values, tmp, 16*sizeof(float));
		}
		Matrix4 cameraInvTranslate;
		{
			float tmp[16] = { 1, 0, 0, camera->position.x,
						 	  0, 1, 0, camera->position.y,
						 	  0, 0, 1, camera->position.z,
		 	 				  0, 0, 0, 1        };
			memcpy((void*) cameraInvTranslate.values, tmp, 16*sizeof(float));
		}
		//Camera translate Matrix	
		Matrix4 orthoProjection;
		{
			float tmp[16] = { 1.f/VIEW_WIDTH, 0              ,  0                   ,   0                                 ,
						 	  0             , 1.f/VIEW_HEIGHT,  0                   ,   0                                 ,
						 	  0             , 0              , -2.f/(Z_FAR - Z_NEAR), -(Z_FAR + Z_NEAR) / (Z_FAR - Z_NEAR),
		 	 				  0             , 0              ,  0                   ,   1                                 };
			memcpy((void*) orthoProjection.values, tmp, 16*sizeof(float));
		}
		// perspectiveProjection
		Matrix4 perspectiveProjection;
		{
			float tmp[16] = { atanf((FOV_X/VIEW_WIDTH)/2), 0                           ,  0                                  ,   0                                   ,
						 	  0                          , atanf((FOV_Y/VIEW_HEIGHT)/2),  0                                  ,   0                                   ,
						 	  0                          , 0                           , -(Z_FAR + Z_NEAR) / (Z_FAR - Z_NEAR), (-2 * (Z_FAR*Z_NEAR))/(Z_FAR - Z_NEAR),
		 	 				  0                          , 0                           , -1                                  ,   0                                   };
			memcpy((void*) perspectiveProjection.values, tmp, 16*sizeof(float));
		}
		Matrix4 correctForScreen;
		{
			float tmp[16] = { SCREEN_WIDTH/2, 0              ,0, SCREEN_WIDTH/2 ,
						 	  0             , SCREEN_HEIGHT/2,0, SCREEN_HEIGHT/2,
						 	  0             , 0              ,1, 1              ,
		 	 				  0             , 0              ,0, 1              };
			memcpy((void*) correctForScreen.values, tmp, 16*sizeof(float));
		}
		
		//Combine matrices into one transformation matrix
		//Model Space -> World Space
		Matrix4 finalTransform = mulMatrix4(xRotMat, scaleMat);
		finalTransform = mulMatrix4(yRotMat, finalTransform);	
		finalTransform = mulMatrix4(worldTranslate, finalTransform);
		//World Space -> View Space	
		finalTransform = mulMatrix4(cameraTranslate, finalTransform);	
		finalTransform = mulMatrix4(cameraTranslate, finalTransform);	
		finalTransform = mulMatrix4(cameraYRotation, finalTransform);	
		finalTransform = mulMatrix4(cameraInvTranslate, finalTransform);	
		//View Space -> Projection Space
		finalTransform = mulMatrix4(perspectiveProjection, finalTransform);	
		//Projection Space -> Screen Friendly
		//finalTransform = mulMatrix4(correctForScreen, finalTransform);	

		for (int i = 0; i < entity->mesh.polyCount; i++)
		{	
			Triangle* poly = &entity->mesh.polygons[i];
			Triangle displayPoly;
			bool isVectorCulled[3] = {false, false, false};		

			for (int j = 0; j < 3; j++)
			{
				//Apply all transformations =====
				displayPoly.vectors[j] = transform(finalTransform, poly->vectors[j], 1);
				
				//Cull vertices
				if (displayPoly.vectors[j].x < -1.f || displayPoly.vectors[j].x > 1.f ||
					displayPoly.vectors[j].y < -1.f || displayPoly.vectors[j].y > 1.f ||
					displayPoly.vectors[j].z < -1.f || displayPoly.vectors[j].z > 1.f)
				{
					isVectorCulled[j] = true;
				}

				//Transform to Screen Friendly view
				displayPoly.vectors[j] = transform(correctForScreen, displayPoly.vectors[j], 1);
			}
			if(!isVectorCulled[0] && !isVectorCulled[1])
			{
				drawLine(displayPoly.vectors[0], displayPoly.vectors[1], lineColor, pixelBuffer);		
			}		
			if(!isVectorCulled[1] && !isVectorCulled[2])
			{
				drawLine(displayPoly.vectors[1], displayPoly.vectors[2], lineColor, pixelBuffer);
			}		
			if(!isVectorCulled[0] && !isVectorCulled[2])
			{
				drawLine(displayPoly.vectors[2], displayPoly.vectors[0], lineColor, pixelBuffer);		
			}
		}
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
		0);//SDL_WINDOW_FULLSCREEN_DESKTOP);
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
	Entity camera = {{0}};//GCC bug temp fix
	Vector3 tmpCamPos = {0};
	camera.position = tmpCamPos;
	Mesh cube;
	Vector3 meshOrigin = {0, 0, -200};
	cube.origin = meshOrigin;
	cube.polyCount = 12;
	cube.polygons = malloc(cube.polyCount * sizeof(Triangle));
	Entity cubeEntity = {{0}};//GCC Bug temp fix
	cubeEntity.mesh = cube;
	cubeEntity.position = meshOrigin;
	//cubeEntity.rotation.x = 0.5;
	Vector3 tmpScale = {100, 100, 100};
	cubeEntity.scale = tmpScale;

	Entity cubeEntity2 = {{0}};
	cubeEntity2.mesh = cube;
	Vector3 cubePos = {300, 0, 200};
	cubeEntity2.position = cubePos;
	Vector3 tmpScale2 = {50, 50, 50};
	cubeEntity2.scale = tmpScale2;

	int entityCount = 2;
	Entity** entityList = (Entity**)malloc(entityCount * sizeof(Entity*));
	entityList[0] = &cubeEntity;
	entityList[1] = &cubeEntity2;

	//Vertices
	Vector3 v0 = { 1, -1, -1};
	Vector3 v1 = { 1,  1, -1};
	Vector3 v2 = {-1,  1, -1};
	Vector3 v3 = {-1, -1, -1};
	Vector3 v4 = { 1, -1,  1};
	Vector3 v5 = { 1,  1,  1};
	Vector3 v6 = {-1,  1,  1};
	Vector3 v7 = {-1, -1,  1};
	
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
		//Keyboard Input
		const uint8_t* keyState = SDL_GetKeyboardState(NULL);	
		{
			int moveVel = 3; 
			if (keyState[SDL_SCANCODE_A])
			{
				camera.position.z += moveVel * cosf(camera.rotation.y - M_PI/2);
				camera.position.x += moveVel * sinf(camera.rotation.y - M_PI/2);
			}
			if (keyState[SDL_SCANCODE_D])
			{
				camera.position.z += moveVel * cosf(camera.rotation.y + M_PI/2);
				camera.position.x += moveVel * sinf(camera.rotation.y + M_PI/2);
			}
			if (keyState[SDL_SCANCODE_S])
			{
				camera.position.z += moveVel * cosf(camera.rotation.y);
				camera.position.x += moveVel * sinf(camera.rotation.y);
			}
			if (keyState[SDL_SCANCODE_W])
			{
				camera.position.z += moveVel * cosf(camera.rotation.y + M_PI);
				camera.position.x += moveVel * sinf(camera.rotation.y + M_PI);
			}
			if (keyState[SDL_SCANCODE_LEFT])
			{
				camera.rotation.y += 0.02;
			}
			if (keyState[SDL_SCANCODE_RIGHT])
			{
				camera.rotation.y -= 0.02;
			}
		}
		globalCounter++;
		cubeEntity2.rotation.x += 0.01;
		cubeEntity2.rotation.y += 0.01;
			
		// Where all the drawing happens
		draw(&pixelBuffer, &camera, entityList, entityCount);

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
