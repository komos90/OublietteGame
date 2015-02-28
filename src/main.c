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
	//Vector3 origin;
	int polyCount;
	Triangle* polygons;
} Mesh;

typedef struct
{
	Vector3 position;
	Vector3 rotation; //TODO Quaternion?
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
		//ZAis Rotation Matrix
		Matrix4 zRotMat;
		{
			float c = entity->rotation.z;
			float tmp[16] = { cosf(c), sinf(c), 0, 0,
						   	 -sinf(c), cosf(c), 0, 0,
						      0      , 0      , 1, 0,
						      0      , 0      , 0, 1};
			memcpy((void*) zRotMat.values, tmp, 16*sizeof(float));
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
		finalTransform = mulMatrix4(zRotMat, finalTransform);	
		finalTransform = mulMatrix4(worldTranslate, finalTransform);
		//World Space -> View Space	
		finalTransform = mulMatrix4(cameraTranslate, finalTransform);	
		finalTransform = mulMatrix4(cameraYRotation, finalTransform);	
		//View Space -> Projection Space
		finalTransform = mulMatrix4(perspectiveProjection, finalTransform);	

		for (int i = 0; i < entity->mesh.polyCount; i++)
		{	
			Triangle displayPoly = entity->mesh.polygons[i];
			bool isVectorCulled[3] = {false, false, false};		

			for (int j = 0; j < 3; j++)
			{
				//Apply all transformations =====
				displayPoly.vectors[j] = transform(finalTransform, displayPoly.vectors[j], 1);
				
				//Cull vertices
				if (displayPoly.vectors[j].x < -1.f || displayPoly.vectors[j].x > 1.f ||
					displayPoly.vectors[j].y < -1.f || displayPoly.vectors[j].y > 1.f ||
					displayPoly.vectors[j].z < -1.f || displayPoly.vectors[j].z > 1.f)
				{
					isVectorCulled[j] = true;
				}

				//Projection Space -> Screen Friendly
				displayPoly.vectors[j] = transform(correctForScreen, displayPoly.vectors[j], 1);
			}
			//Only draw lines between vectors that haven't been culled
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

Mesh loadMeshFromFile(char* fileName)
{
	//Replace relative path with realpath for linux
	char fullFileName[256];
	#ifdef __linux__
		realpath(fileName, fullFileName);
	#else
		strcpy(fullFileName, fileName);
	#endif

	FILE* file = fopen(fullFileName, "r");
	Mesh mesh = {0};
	int lineCount = 0;

	if (file == NULL)
	{
		SDL_Log("Could not open mesh file.");
		SDL_Log(fullFileName);
	}

	//Save the pos beginning of file
	fpos_t filePos;
	fgetpos(file, &filePos);
	//Count the number of lines in the file
	{
		int ch;
		while (EOF != (ch=getc(file)))
	   		if (ch=='\n')
	        	++lineCount;
    }

    mesh.polyCount = lineCount;
    mesh.polygons = (Triangle*)malloc(lineCount * sizeof(Triangle));

    //Go back to beginning of file
    fsetpos(file, &filePos);
	char line[256] = {0};
	int i = 0;
	int k = 0;
	fgets(line, 255, file);
	while (!feof(file))
	{
		float vertices[9] = {0};
		//SDL_Log("Hello!");
		//Split line by spaces and store floats
		k = 0;
		while (k < 9)
		{
			if (k == 0)
				vertices[k] = atof(strtok(line, " "));
			else
				vertices[k] = atof(strtok(NULL, " "));
			k++;
		}
		//Store the loaded vertices into the return polygon
		mesh.polygons[i].vectors[0].x = vertices[0];
		mesh.polygons[i].vectors[0].y = vertices[1];
		mesh.polygons[i].vectors[0].z = vertices[2];
		mesh.polygons[i].vectors[1].x = vertices[3];
		mesh.polygons[i].vectors[1].y = vertices[4];
		mesh.polygons[i].vectors[1].z = vertices[5];
		mesh.polygons[i].vectors[2].x = vertices[6];
		mesh.polygons[i].vectors[2].y = vertices[7];
		mesh.polygons[i].vectors[2].z = vertices[8];
		fgets(line, 255, file);
		i++;
	}
	return mesh;
}

int main( int argc, char* args[] )
{
	SDL_Window* window = NULL;
	SDL_Renderer* renderer = NULL;
	SDL_Texture* screenTexture = NULL;
	uint32_t* pixels = NULL;
	bool running = true;

	//Initialise SDL ====
	if( SDL_Init( SDL_INIT_EVERYTHING) < 0 )
	{
		printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
		return 0;
	}

	window = SDL_CreateWindow(
		"Pixel buffer Playground :P", SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT,
		SDL_WINDOW_FULLSCREEN_DESKTOP); //TODO add key to toggle fullscreen while running.
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

	//Initialise Meshes and Entities ====
	//Load meshes
	Mesh cube  = loadMeshFromFile("../res/meshes/cube.raw");
	Mesh plane = loadMeshFromFile("../res/meshes/plane.raw");
	Mesh monkey  = loadMeshFromFile("../res/meshes/monkeyhd.raw");

	//Initialise entities
	Entity camera = {{0}};
	Entity cubeEntity = {  .position={0, 0, -200},    .mesh=monkey, 
                           .rotation={-M_PI/2, 0, 0}, .scale={100, 100, 100}};
	Entity cubeEntity2 = { .position={300, 0, 200},   .mesh=cube,
                           .scale   ={50, 50, 50}};

	//Temp corridore
	Entity hall1 = { .position={-300, 0, 200}, .mesh=plane,
                     .scale   ={50, 50, 50}};
	Entity hall2 = { .position={-300, 50, 150}, .mesh=plane,
                     .rotation={M_PI/2, 0, 0}, .scale={50, 50, 50}};
	Entity hall3 = { .position={-300, 0, 100}, .mesh=plane,
                     .scale   ={50, 50, 50}};
	//Create entity list and fill with entities
	int entityCount = 5;
	Entity** entityList = (Entity**)malloc(entityCount * sizeof(Entity*));
	entityList[0] = &cubeEntity;
	entityList[1] = &cubeEntity2;
	entityList[2] = &hall1;
	entityList[3] = &hall2;
	entityList[4] = &hall3;

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
		//cubeEntity2.rotation.x += 0.01;
		cubeEntity2.rotation.z += 0.01;
			
		//Where all the drawing happens
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
