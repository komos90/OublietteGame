/*
Hand coded 3d software renderer project.
Graphics engine code.
Seoras Macdonald
seoras1@gmail.com
2015
*/

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <limits.h>

#ifdef __linux__
    #include <SDL2/SDL.h>
#elif __Win32
    #include <SDL.h>
#endif

#include "gfx_engine.h"
#include "engine_types.h"


void drawRect(int x, int y, int w, int h, uint32_t color,
              PixelBuffer pixelBuffer)
{
	for (int yy = y; yy < y + h; ++yy)
	{
		for (int xx = x; xx < x + w; ++xx)
		{
			pixelBuffer.pixels[yy * pixelBuffer.width + xx] = color;
		}
	}
}

//TODO use floats for depth buffer
//TODO Get proper per pixel z values for each polygon
void drawVector(Vector3 vector, uint32_t color, PixelBuffer pixelBuffer)
{
	if (vector.x >= 0 && vector.x < pixelBuffer.width &&
		vector.y >= 0 && vector.y < pixelBuffer.height && 
		vector.z < pixelBuffer.zBuffer[(int)vector.y  * pixelBuffer.width + (int)vector.x])
	{
		pixelBuffer.pixels[(int)vector.y  * pixelBuffer.width + (int)vector.x] = color;
		pixelBuffer.zBuffer[(int)vector.y  * pixelBuffer.width + (int)vector.x] = vector.z;
	}
}

void drawLine(Vector3 start, Vector3 end, uint32_t color,
              PixelBuffer pixelBuffer)
{
	int x0 = (int)start.x;
    int y0 = (int)start.y;
    int x1 = (int)end.x;
    int y1 = (int)end.y;
    
	//Distance between x0 and x1, y0 and y1
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
	//Determine whether to step backwards or forwards through the line
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

Vector3 transform(Matrix4 matrix, Vector3 vector) 
{
    float w;
    Vector3 result;
	result.x = matrix.values[0] * vector.x + matrix.values[1] * vector.y +
			   matrix.values[2] * vector.z + matrix.values[3] * 1.f;
	result.y = matrix.values[4] * vector.x + matrix.values[5] * vector.y +
			   matrix.values[6] * vector.z + matrix.values[7] * 1.f;
	result.z = matrix.values[8] * vector.x + matrix.values[9] * vector.y +
			   matrix.values[10] * vector.z + matrix.values[11] * 1.f;
	w = matrix.values[12] * vector.x + matrix.values[13] * vector.y +
			   matrix.values[14] * vector.z + matrix.values[15] * 1.f;

	if (w != 0.f && w != 1.f) {
		result.x /= w;
		result.y /= w;
		result.z /= w;
		//result.w /= result.w;
	}
    //SDL_Log("%f, %f, %f, %f", result.x, result.y, result.z, result.w);
	return result;
}

void rasterizePolygon(Triangle poly, uint32_t color,
                      PixelBuffer pixelBuffer)
{
    /*
    Pseudo Code ====
    Find the top vertex
    curX0, curY0 = 0
    for(;;)
    {
        find curX1 for scanline curY0
        fill scanline
        if (curX1, curY1) == rightmostVertex
            topR = rightmostVertex
            rightmostVertex = leftmostVertex
        if (curX0, curY0) == leftmostVertex
            topL = leftmostVertex
            leftmostVertex = rightmostVertex
        if (curX0, curY0) == rightmostVertex
            end
        find curX0
    }

    int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
    int dy = abs(y1-y0), sy = y0<y1 ? 1 : -1; 
    int err = (dx>dy ? dx : -dy)/2, e2;
    for(;;){
        setPixel(x0,y0);
        if (x0==x1 && y0==y1) break;
        e2 = err;
        if (e2 >-dx) { err -= dy; x0 += sx; }
        if (e2 < dy) { err += dx; y0 += sy; }
    }
    */

    int topIndex = 0;
    int leftIndex = 0;
    int rightIndex = 0;
    //Find top vertex
    for (int i = 1; i < 3; i++)
    {
        if (poly.vectors[i].y < poly.vectors[topIndex].y)
        {
            topIndex = i;
        }
        else if (poly.vectors[i].y == poly.vectors[topIndex].y)
        {
            if (poly.vectors[i].x < poly.vectors[(i+1)%3].x ||
                poly.vectors[i].x < poly.vectors[(i+2)%3].x)
            {
                topIndex = i;
            }
        }
    }
    //Find left and right vertices
    leftIndex  = (topIndex + 2) % 3;
    rightIndex  = (topIndex + 1) % 3;

    
    //Initilise vertices for triangle drawing
    Vector2Int topL  = {(int)poly.vectors[topIndex].x,
                        (int)poly.vectors[topIndex].y};
    Vector2Int left  = {(int)poly.vectors[leftIndex].x,
                        (int)poly.vectors[leftIndex].y};
    Vector2Int right = {(int)poly.vectors[rightIndex].x,
                        (int)poly.vectors[rightIndex].y};
    Vector2Int topR  = topL;

    //Line drawing variables for left line
    int dxL = abs(left.x-topL.x);
    int dyL = abs(left.y-topL.y);
    int sxL = topL.x<left.x ? 1 : -1;
    int syL = topL.y<left.y ? 1 : -1; 
    int errL = (dxL>dyL ? dxL : -dyL)/2;
    int e2L;

    //Line drawing variables for right line
    int dxR = abs(right.x-topR.x);
    int dyR = abs(right.y-topR.y);
    int sxR = topR.x<right.x ? 1 : -1;
    int syR = topR.y<right.y ? 1 : -1; 
    int errR = (dxR>dyR ? dxR : -dyR)/2;
    int e2R;

    for(;;)
    {
        //Draw current scanline
        {
            for(;;)
            {
                //Handle breakpoint on right line
                if (topR.x == right.x && topR.y == right.y)
                {
                    right = left;
                    dxR = abs(right.x-topR.x);
                    dyR = abs(right.y-topR.y);
                    sxR = topR.x<right.x ? 1 : -1;
                    syR = topR.y<right.y ? 1 : -1; 
                    errR = (dxR>dyR ? dxR : -dyR)/2;
                }
                if (topL.y == topR.y) break;
                e2R = errR;
                if (e2R >-dxR) { errR -= dyR; topR.x += sxR; }
                if (e2R < dyR) { errR += dxR; topR.y += syR; }
            }
            //Fill scanline
            for (int i = topL.x; i < topR.x; i++)
            {
                Vector3 tmp = {(int)i, (int)topL.y, poly.vectors[0].z};
                drawVector(tmp, color, pixelBuffer);
            }
        } 
        if (topL.x==left.x && topL.y==left.y)
        {
            if (right.y <= topL.y)
                break;
            else
            {
                left = right;
                dxL = abs(left.x-topL.x);
                dyL = abs(left.y-topL.y);
                sxL = topL.x<left.x ? 1 : -1;
                syL = topL.y<left.y ? 1 : -1; 
                errL = (dxL>dyL ? dxL : -dyL)/2;
            }
        }
        e2L = errL;
        if (e2L >-dxL) { errL -= dyL; topL.x += sxL; }
        if (e2L < dyL) { errL += dxL; topL.y += syL; }
    } 
}

//Has some temp debug parameters
void draw(PixelBuffer pixelBuffer, Entity* camera,
          Entity* entityList, int entityCount,
          bool shouldDrawWireframe,
          bool shouldDrawSurfaces)
{
	for (int k = 0; k < entityCount; k++) 
	{
		Entity* entity = &entityList[k];
		// Rotation angles, y-axis then x-axis
		uint32_t lineColor = 0xffffffff;
        uint32_t fillColor = 0x55555555;
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
            int w = pixelBuffer.width;
            int h = pixelBuffer.height;
            int d = INT_MAX;
			float tmp[16] = { w/2, 0  ,0  , w/2,
						 	  0  , h/2,0  , h/2,
						 	  0  , 0  ,d/2, d/2,
		 	 				  0  , 0  ,0  , 1  };
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
				displayPoly.vectors[j] = transform(finalTransform, displayPoly.vectors[j]);
				
				//Cull vertices
				if (displayPoly.vectors[j].x < -1.f || displayPoly.vectors[j].x > 1.f ||
					displayPoly.vectors[j].y < -1.f || displayPoly.vectors[j].y > 1.f ||
					displayPoly.vectors[j].z < -1.f || displayPoly.vectors[j].z > 1.f)
				{
					isVectorCulled[j] = true;
				}

				//Projection Space -> Screen Friendly
				displayPoly.vectors[j] = transform(correctForScreen, displayPoly.vectors[j]);
			}
            if(!(isVectorCulled[0] || isVectorCulled[1] || isVectorCulled[2]) && shouldDrawSurfaces)
            {
            	rasterizePolygon(displayPoly, fillColor, pixelBuffer);
            }
            fillColor = ~fillColor;
			//Only draw lines between vectors that haven't been culled
            if(shouldDrawWireframe)
            {
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

