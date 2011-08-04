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


#include <cstdlib>
#include <math.h>
#include <SDL/SDL.h>
#include <assert.h>

#include "LogManager.h"
#include "SmoothTriangleDebug.h"

#pragma warning(disable:4244)

namespace debugtriangle
{

__inline float randf() { return (float)rand() / (float)RAND_MAX; }
__inline int MAX(int a, int b) { return a>b?a:b; }
__inline int MIN(int a, int b) { return a<b?a:b; }

const int COLORDEPTH = 32;
const int BYTESPERPIXEL = 4;

//-----------------------------------------------------------------------------

const float DynamicTriangleGrid::MAX_HEIGHT = 255.0f;

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

	A = Vertex(        0.f,         0.f, 0.f, rand() );
	B = Vertex( width*64.f,         0.f, 0.f, rand() );
	C = Vertex( width*64.f, height*64.f, 0.f, rand() );
	D = Vertex(        0.f, height*64.f, 0.f, rand() );

	A.pos.z = createHeight( A.seed, level, MAX_HEIGHT );
	B.pos.z = createHeight( B.seed, level, MAX_HEIGHT );
	C.pos.z = createHeight( C.seed, level, MAX_HEIGHT );
	D.pos.z = createHeight( D.seed, level, MAX_HEIGHT );
}

//-----------------------------------------------------------------------------

float DynamicTriangleGrid::createHeight( int seed, float base, float max )
{
	srand(seed);
	float deviation = max * steepness * randf() - (max*steepness)/2.0f;
	return MAX(0.0f, MIN( MAX_HEIGHT, base + deviation ));
}

//-----------------------------------------------------------------------------

float DynamicTriangleGrid::getHeightAt(float x, float y, int detail)
{
	// find out which top-level triangle the point is on
	Vec2f u = B.pos2D()-A.pos2D();
	Vec2f v = D.pos2D()-A.pos2D();
	Vec2f p = Vec2f(x,y) - A.pos2D();

	float lambda = (p.x*v.y-p.y*v.x)/(u.x*v.y-u.y*v.x);
	float mue = (p.y*u.x-p.x*u.y)/(u.x*v.y-u.y*v.x);

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

float DynamicTriangleGrid::_getHeightAtTriangle(float x, float y, 
												Vertex a, Vertex b, Vertex c)
{
	// find position on triangle using barycentric coordinates
	Vec2f u = b.pos2D()-a.pos2D();
	Vec2f v = c.pos2D()-a.pos2D();
	Vec2f p = Vec2f(x,y) - a.pos2D();

	float lambda = (p.x*v.y-p.y*v.x)/(u.x*v.y-u.y*v.x);
	float mue = (p.y*u.x-p.x*u.y)/(u.x*v.y-u.y*v.x);

	// interpolate height values of the vertices
	return (1-lambda-mue)*a.pos.z + lambda*b.pos.z + mue*c.pos.z;
}

//-----------------------------------------------------------------------------

float DynamicTriangleGrid::_getHeightAt(float x, float y, 
										Vertex a, Vertex b, Vertex c, int depth)
{
	if(depth==0) return _getHeightAtTriangle(x,y,a,b,c);

	Vec2f u = b.pos2D()-a.pos2D();
	Vec2f v = c.pos2D()-a.pos2D();
	Vec2f w = c.pos2D()-b.pos2D();
	Vec2f p = Vec2f(x,y) - a.pos2D();

	float ab_length = length(u);
	float ac_length = length(v);
	float bc_length = length(w);

	float lambda = (p.x*v.y-p.y*v.x)/(u.x*v.y-u.y*v.x);
	float mue = (p.y*u.x-p.x*u.y)/(u.x*v.y-u.y*v.x);

	// create seeds at edge midpoints
	float s_ab = interpolateSeeds(a.seed,b.seed);
	float s_ac = interpolateSeeds(a.seed,c.seed);
	float s_bc = interpolateSeeds(b.seed,c.seed);

	// create heights at edge midpoints
	float h_ab = createHeight( s_ab, (a.pos.z+b.pos.z)/2, length(u)*0.5 );
	float h_ac = createHeight( s_ac, (a.pos.z+c.pos.z)/2, length(v)*0.5 );
	float h_bc = createHeight( s_bc, (b.pos.z+c.pos.z)/2, length(w)*0.5 );

	// create edge midpoints
	Vertex AB = Vertex( (a.pos.x+b.pos.x)*0.5, (a.pos.y+b.pos.y)*0.5, h_ab, s_ab );
	Vertex AC = Vertex( (a.pos.x+c.pos.x)*0.5, (a.pos.y+c.pos.y)*0.5, h_ac, s_ac );
	Vertex BC = Vertex( (c.pos.x+b.pos.x)*0.5, (c.pos.y+b.pos.y)*0.5, h_bc, s_bc );

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

void DynamicTriangleGrid::writeImage(const char *filename)
{
	SDL_Surface* image = SDL_CreateRGBSurface(SDL_SWSURFACE,width+1,height+1,8,
											  0xff,0xff,0xff,0);
	SDL_Surface* preview = SDL_CreateRGBSurface(SDL_SWSURFACE,width+1,height+1,
												32,0,0,0,0);
	
	SDL_LockSurface(image);
	SDL_LockSurface(preview);

	LogManager::log("creating heightmap",true);

	// this is the only difference from the StaticTriangleGrid's writeImage()
	for( int y=0; y<height+1; y++ )
		for( int x=0; x<width+1; x++ )
		{	
			int h = (int) getHeightAt( (float)x, (float)y, detail );
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

}