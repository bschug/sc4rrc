/******************************************************************************
 *	file: TriangleGrid.cpp
 *
 *	Copyright (c) 2007, Benjamin Schug
 *
 *	This file is part of the SimCity 4 Random Region Creator (SC4RRC).
 *
 *	Sim City is a registered trademark of Electronic Arts.
 *	This program is in no way associated with Maxis or Electronic Arts.
 *
 *	SC4RRC is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	SC4RRC is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *	
 *****************************************************************************/

#define SC4RRC_LIB

#include <cstdlib>
#include <math.h>
#include <SDL/SDL.h>
#include <assert.h>

#include "LogManager.h"
#include "TriangleGrid.h"

#pragma warning(disable:4244)

__inline float randf() { return (float)rand() / (float)RAND_MAX; }
__inline int MAX(int a, int b) { return a>b?a:b; }
__inline int MIN(int a, int b) { return a<b?a:b; }

const int COLORDEPTH = 32;
const int BYTESPERPIXEL = 4;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	const uint RMASK = 0xff000000;
	const uint GMASK = 0x00ff0000;
	const uint BMASK = 0x0000ff00;
	const uint AMASK = 0x00000000;
#else
	const Uint32 RMASK = 0x000000ff;
	const Uint32 GMASK = 0x0000ff00;
	const Uint32 BMASK = 0x00ff0000;
	const Uint32 AMASK = 0x00000000;
#endif

//-----------------------------------------------------------------------------

DynamicTriangleGrid::DynamicTriangleGrid(int width, int height, int level, 
										 int blur, int detail, float steepness,
										 int seed)
: SC4Landscape(width,height,level,blur),steepness(steepness),detail(detail)
{
	std::ostringstream* o = new std::ostringstream;
	(*o) << "Settings: " << std::endl
		 << "width = " << width << std::endl
		 << "height = " << height << std::endl
		 << "level = " << level << std::endl
		 << "blur = " << blur << std::endl << std::endl
		 << "generator = DYNAMIC TRIANGLE GRID" << std::endl
		 << "steepness = " << steepness << std::endl
		 << "detail level = " << detail << std::endl << std::endl
		 << "seed = " << seed << std::endl;
	LogManager::log(o,true);

	srand(seed);

	A = Vertex(        0,         0, 0, rand() );
	B = Vertex( width*64,         0, 0, rand() );
	C = Vertex( width*64, height*64, 0, rand() );
	D = Vertex(        0, height*64, 0, rand() );

	A.z = createHeight( A.seed, level, MAX_HEIGHT );
	B.z = createHeight( B.seed, level, MAX_HEIGHT );
	C.z = createHeight( C.seed, level, MAX_HEIGHT );
	D.z = createHeight( D.seed, level, MAX_HEIGHT );
}

//-----------------------------------------------------------------------------

StaticTriangleGrid::StaticTriangleGrid(int width, int height, int level, 
									   int blur, int detail, float steepness,
									   int seed)
: SC4Landscape(width,height,level,blur),steepness(steepness),detail(detail)
{
	std::ostringstream* o = new std::ostringstream;
	(*o) << "Settings: " << std::endl
		 << "width = " << width << std::endl
		 << "height = " << height << std::endl
		 << "level = " << level << std::endl
		 << "blur = " << blur << std::endl << std::endl
		 << "generator = STATIC TRIANGLE GRID" << std::endl
		 << "steepness = " << steepness << std::endl
		 << "detail level = " << detail << std::endl << std::endl
		 << "seed = " << seed << std::endl;
	LogManager::log(o,true);

	srand(seed);

	A = Vertex(        0,         0, 0, rand() );
	B = Vertex( width*64,         0, 0, rand() );
	C = Vertex( width*64, height*64, 0, rand() );
	D = Vertex(        0, height*64, 0, rand() );

	A.z = createHeight( A.seed, level, MAX_HEIGHT );
	B.z = createHeight( B.seed, level, MAX_HEIGHT );
	C.z = createHeight( C.seed, level, MAX_HEIGHT );
	D.z = createHeight( D.seed, level, MAX_HEIGHT );
}

//-----------------------------------------------------------------------------

int StaticTriangleGrid::createHeight( int seed, int base, int max )
{
	srand(seed);
	int deviation = (float)max * steepness * randf() - (max*steepness)/2;
	return MAX(0, MIN( MAX_HEIGHT, base + deviation ));
}

//-----------------------------------------------------------------------------

int DynamicTriangleGrid::createHeight( int seed, int base, int max )
{
	srand(seed);
	int deviation = (float)max * steepness * randf() - (max*steepness)/2;
	return MAX(0, MIN( MAX_HEIGHT, base + deviation ));
}

//-----------------------------------------------------------------------------

int DynamicTriangleGrid::getHeightAt(int x, int y, int detail)
{
	// find out which top-level triangle the point is on
	float ux = B.x - A.x;
	float uy = B.y - A.y;
	float vx = D.x - A.x;
	float vy = D.y - A.y;
	float px = (float)x - A.x;
	float py = (float)y - A.y;

	float lambda = (px*vy-py*vx)/(ux*vy-uy*vx);
	float mue = (py*ux-px*uy)/(ux*vy-uy*vx);

	if( lambda+mue <= 1 )
	{
		// triangle ABD
		return _getHeightAt(x,y,A,B,D,detail);
	}
	else
	{
		// triangle CDB
		return _getHeightAt(x,y,C,D,B,detail);
	}
}

//-----------------------------------------------------------------------------

int DynamicTriangleGrid::_getHeightAtTriangle(int x, int y, 
											  Vertex a, Vertex b, Vertex c)
{
	// find position on triangle using barycentric coordinates
	float ux = b.x - a.x;
	float uy = b.y - a.y;
	float vx = c.x - a.x;
	float vy = c.y - a.y;
	float px = (float)x - a.x;
	float py = (float)y - a.y;

	float lambda = (px*vy-py*vx)/(ux*vy-uy*vx);
	float mue = (py*ux-px*uy)/(ux*vy-uy*vx);

	// interpolate height values of the vertices
	return (1-lambda-mue)*a.z + lambda*b.z + mue*c.z;
}

//-----------------------------------------------------------------------------

int DynamicTriangleGrid::_getHeightAt(int x, int y, 
									  Vertex a, Vertex b, Vertex c, int depth)
{
	if(depth==0) return _getHeightAtTriangle(x,y,a,b,c);

	float ux = b.x - a.x;
	float uy = b.y - a.y;
	float vx = c.x - a.x;
	float vy = c.y - a.y;
	float wx = c.x - b.x;
	float wy = c.y - b.y;
	float px = (float)x - a.x;
	float py = (float)y - a.y;

	float ab_length = sqrt(ux*ux+uy*uy);
	float ac_length = sqrt(vx*vx+vy*vy);
	float bc_length = sqrt(wx*wx+wy*wy);

	float lambda = (px*vy-py*vx)/(ux*vy-uy*vx);
	float mue = (py*ux-px*uy)/(ux*vy-uy*vx);

	// create seeds at edge midpoints
	float s_ab = interpolateSeeds(a.seed,b.seed);
	float s_ac = interpolateSeeds(a.seed,c.seed);
	float s_bc = interpolateSeeds(b.seed,c.seed);

	// create heights at edge midpoints
	float h_ab = createHeight( s_ab, (a.z+b.z)/2, ab_length*0.5 );
	float h_ac = createHeight( s_ac, (a.z+c.z)/2, ac_length*0.5 );
	float h_bc = createHeight( s_bc, (b.z+c.z)/2, bc_length*0.5 );

	// create edge midpoints
	Vertex AB = Vertex( (a.x+b.x)*0.5, (a.y+b.y)*0.5, h_ab, s_ab );
	Vertex AC = Vertex( (a.x+c.x)*0.5, (a.y+c.y)*0.5, h_ac, s_ac );
	Vertex BC = Vertex( (c.x+b.x)*0.5, (c.y+b.y)*0.5, h_bc, s_bc );

	if( lambda+mue <= 0.5 )
	{
		// "lower left" triangle (at point a)
		return _getHeightAt(x,y,a,AB,AC,depth-1);
	}
	if( lambda > 0.5 )
	{
		// "lower right" triangle (at point b)
		return _getHeightAt(x,y,AB,b,BC,depth-1);
	}
	if( mue > 0.5 )
	{
		// "top" triangle (at point c)
		return _getHeightAt(x,y,AC,BC,c,depth-1);
	}
	else
	{
		// middle triangle
		return _getHeightAt(x,y,AB,AC,BC,depth-1);
	}

}

//-----------------------------------------------------------------------------

/**	Blurs the image.
 *	This function assigns to each pixel the average of all surrounding pixels.
 *	This is repeated <blur_amount> times.
 */
void blurImage(SDL_Surface* image, int blur_amount)
{
	LogManager::log("blurring image",true);

	for(int i=0; i<blur_amount; i++)
		for(int y=1; y<image->h-1; y++)
			for(int x=1; x<image->w-1; x++)
			{
				int sum=0;
				int nr_of_samples=0;

				for(int yy=y-1; yy<=y+1; yy++)
				{
					for(int xx=x-1; xx<=x+1; xx++)
					{
						int ofs = xx + yy * image->pitch;
						sum += ((Uint8*)image->pixels)[ofs];
						nr_of_samples++;
					}
				}

				int ofs = x+y*image->pitch;
				((Uint8*)image->pixels)[ofs] = (sum/nr_of_samples);
			}
}

//-----------------------------------------------------------------------------

void StaticTriangleGrid::writeImage(const char *filename)
{
	// 8-bit grayscale surface for the heightmap
	SDL_Surface* image = SDL_CreateRGBSurface(SDL_SWSURFACE,width+1,height+1,8,
											  0x000000ff,0x000000ff,0x000000ff,0);
	// 32-bit color surface for the preview image
	SDL_Surface* preview = SDL_CreateRGBSurface(SDL_SWSURFACE,width+1,height+1,
												32,RMASK,GMASK,BMASK,AMASK);
	
	LogManager::log("building triangle mesh",true);
	FractalTriangle* abd = buildTriangleMesh(A,B,D,detail);
	FractalTriangle* cdb = buildTriangleMesh(C,D,B,detail);
	FractalTriangle* current;

	SDL_LockSurface(image);
	SDL_LockSurface(preview);

	LogManager::log("creating heightmap",true);

	for( int y=0; y<height+1; y++ )
		for( int x=0; x<width+1; x++ )
		{	
			// choose which base triangle the point is lying on
			float lambda = (x-A.x)/(B.x-A.x);
			float mue = (y-A.y)/(C.y-A.y);
			if(lambda+mue < 1)
				current = abd;
			else
				current = cdb;

			// use height value as pixel color
			int h = current->getHeightAt(x,y);
			int ofs = x + y * image->pitch;
			((Uint8*)image->pixels)[ofs] = (Uint8)h;
		}

	blurImage(image,blur);

	LogManager::log("creating preview",true);

	for(int y=0; y<height+1; y++)
		for(int x=0; x<width+1; x++)
		{
			// get height value from the grayscale heightmap
			int ofs = x + y * image->pitch;
			Uint8 h = ((Uint8*)image->pixels)[ofs];

			// choose color depending on terrain height
			Uint8 r,g,b;
			if(h <= 83)
			{
				// water is blue - the deeper, the darker
				r = (100*h)/83;
				g = (100*h)/83;
				b = 150 + (100*h)/83;
			}
			else
			{
				// land color goes from green (low) to red (mountains)
				h -= 83;
				r = 80 + (40*h)/172;
				g = 120 - (40*h)/172;
				b = 30;
			}
			ofs = x + y * preview->pitch / 4;
			((Uint32*)preview->pixels)[ofs] = SDL_MapRGB(preview->format,r,g,b);
		}

	SDL_UnlockSurface(image);
	SDL_UnlockSurface(preview);

	SDL_SaveBMP(image,filename);
	SDL_SaveBMP(preview,"preview.bmp");

	SDL_FreeSurface(image);
	SDL_FreeSurface(preview);

	LogManager::log("unloading triangle mesh",true);
	delete abd;
	delete cdb;
}

//-----------------------------------------------------------------------------

void DynamicTriangleGrid::writeImage(const char *filename)
{
	SDL_Surface* image = SDL_CreateRGBSurface(SDL_SWSURFACE,width+1,height+1,8,
											  0x000000ff,0x000000ff,0x000000ff,0);
	SDL_Surface* preview = SDL_CreateRGBSurface(SDL_SWSURFACE,width+1,height+1,
												32,RMASK,GMASK,BMASK,AMASK);
	
	SDL_LockSurface(image);
	SDL_LockSurface(preview);

	LogManager::log("creating heightmap",true);

	// this is the only difference from the StaticTriangleGrid's writeImage()
	for( int y=0; y<height+1; y++ )
		for( int x=0; x<width+1; x++ )
		{	
			int h = getHeightAt(x,y,detail);
			int ofs = x + y * image->pitch;
			((Uint8*)image->pixels)[ofs] = (Uint8)h;
		}

	blurImage(image,blur);

	LogManager::log("creating preview",true);
	for(int y=0; y<height+1; y++)
		for(int x=0; x<width+1; x++)
		{
			int ofs = x + y * image->pitch;
			Uint8 h = ((Uint8*)image->pixels)[ofs];

			Uint8 r,g,b;
			if(h <= 83)
			{
				r = (100*h)/83;
				g = (100*h)/83;
				b = 150 + (100*h)/83;
			}
			else
			{
				h -= 83;
				r = 80 + (40*h)/172;
				g = 120 - (40*h)/172;
				b = 30;
			}
			ofs = x + y * preview->pitch / 4;
			((Uint32*)preview->pixels)[ofs] = SDL_MapRGB(preview->format,r,g,b);
		}

	SDL_UnlockSurface(image);
	SDL_UnlockSurface(preview);

	SDL_SaveBMP(image,filename);
	SDL_SaveBMP(preview,"preview.bmp");

	SDL_FreeSurface(image);
	SDL_FreeSurface(preview);
}

//-----------------------------------------------------------------------------

int StaticTriangleGrid::CompositeTriangle::getHeightAt(float x, float y)
{
	float lambda = (x-A.x)/(B.x-A.x);
	float mue = (y-A.y)/(C.y-A.y);

	if(lambda+mue < 0.5) return tri_I->getHeightAt(x,y);
	if(lambda > 0.5) return tri_II->getHeightAt(x,y);
	if(mue > 0.5) return tri_III->getHeightAt(x,y);
	return tri_IV->getHeightAt(x,y);
}

//-----------------------------------------------------------------------------

int StaticTriangleGrid::AtomicTriangle::getHeightAt(float x, float y)
{
	float lambda = (x-A.x)/(B.x-A.x);
	float mue = (y-A.y)/(C.y-A.y);

	return (1-lambda-mue)*A.z + lambda*B.z + mue*C.z;
}

//-----------------------------------------------------------------------------

StaticTriangleGrid::CompositeTriangle::~CompositeTriangle()
{
	delete tri_I;
	delete tri_II;
	delete tri_III;
	delete tri_IV;
}

//-----------------------------------------------------------------------------

StaticTriangleGrid::FractalTriangle* 
StaticTriangleGrid::buildTriangleMesh(Vertex A, Vertex B, Vertex C, int depth)
{
	// this works only for axis-aligned rectangular triangles
	assert(A.x==C.x && A.y==B.y);

	if( depth==0 )
	{
		return new AtomicTriangle(A,B,C);
	}

	int s_ab = interpolateSeeds(A.seed,B.seed);
	int s_ac = interpolateSeeds(A.seed,C.seed);
	int s_bc = interpolateSeeds(B.seed,C.seed);
	
	int h_ab = createHeight(s_ab,(A.z+B.z)/2,fabs(B.x-A.x));
	int h_ac = createHeight(s_ac,(A.z+C.z)/2,fabs(C.x-A.x));
	int h_bc = createHeight(s_bc,(B.z+C.z)/2,fabs(C.x-B.x));

	Vertex AB( (A.x+B.x)*0.5, A.y, h_ab, s_ab );
	Vertex AC( A.x, (A.y+C.y)*0.5, h_ac, s_ac );
	Vertex BC( (B.x+C.x)*0.5, (B.y+C.y)*0.5, h_bc, s_bc );

	CompositeTriangle* tri = new CompositeTriangle;
	tri->A = A;
	tri->B = B;
	tri->C = C;
	tri->tri_I = buildTriangleMesh(A,AB,AC,depth-1);
	tri->tri_II = buildTriangleMesh(AB,B,BC,depth-1);
	tri->tri_III = buildTriangleMesh(AC,BC,C,depth-1);
	tri->tri_IV = buildTriangleMesh(BC,AC,AB,depth-1);
	return tri;
}