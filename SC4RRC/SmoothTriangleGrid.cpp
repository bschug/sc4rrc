/******************************************************************************
 *	file: SmoothTriangleGrid.cpp
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

#include <SDL/SDL.h>

#include "SmoothTriangleGrid.h"
#include "LogManager.h"


__inline float randf() { return (float)rand() / (float)RAND_MAX; }
__inline int MAX(int a, int b) { return a>b?a:b; }
__inline int MIN(int a, int b) { return a<b?a:b; }

// implemented in TriangleGrid.cpp
void blurImage(SDL_Surface* image, int blur_amount);

//-----------------------------------------------------------------------------

SmoothTriangleGrid::SmoothTriangleGrid( int width, int height, int level, 
									    int blur, int detail, float steepness,
										int seed )
										: SC4Landscape(width,height,level,blur),
										  detail(detail),steepness(steepness),
										  MAX_HEIGHT(255.0f), MIN_HEIGHT(0.0f)
{
	std::ostringstream* o = new std::ostringstream;
	(*o) << "Settings: " << std::endl
		 << "width = " << width << std::endl
		 << "height = " << height << std::endl
		 << "level = " << level << std::endl
		 << "blur = " << blur << std::endl << std::endl
		 << "generator = SMOOTH TRIANGLE GRID" << std::endl
		 << "steepness = " << steepness << std::endl
		 << "detail level = " << detail << std::endl << std::endl
		 << "seed = " << seed << std::endl;
	LogManager::log(o,true);

	srand(seed);

	A = SmoothVertex( Vec3f(    0,       0,    0), Vec3f(0,0,1), rand() );
	B = SmoothVertex( Vec3f(width*64,    0,	   0), Vec3f(0,0,1), rand() );
	C = SmoothVertex( Vec3f(width*64,height*64,0), Vec3f(0,0,1), rand() );
	D = SmoothVertex( Vec3f(	0,	 height*64,0), Vec3f(0,0,1), rand() );

	A.pos.z = displaceHeight( A.seed, (float)level, MAX_HEIGHT );
	B.pos.z = displaceHeight( B.seed, (float)level, MAX_HEIGHT );
	C.pos.z = displaceHeight( C.seed, (float)level, MAX_HEIGHT );
	D.pos.z = displaceHeight( D.seed, (float)level, MAX_HEIGHT );
}

//-----------------------------------------------------------------------------

float SmoothTriangleGrid::displaceHeight(int seed, float base, float max)
{
	srand(seed);
	float deviation = max * randf() - max/2.0f;
	return MAX(MIN_HEIGHT, MIN( MAX_HEIGHT, base + deviation ));
}

//-----------------------------------------------------------------------------

float SmoothTriangleGrid::getHeightAt(int x, int y, int detail)
{
	// find out which top-level triangle the point is on
	Vec2f u = B.pos2d()-A.pos2d();
	Vec2f v = D.pos2d()-A.pos2d();
	Vec2f p = Vec2f(x,y)-A.pos2d();

	float lambda = ( p.x * v.y - p.y * v.x ) / ( u.x * v.y - u.y * v.x );
	float mue =    ( p.y * u.x - p.x * u.y ) / ( u.x * v.y - u.y * v.x );

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

float SmoothTriangleGrid::_getHeightAt( int x, int y, 
										SmoothVertex a, 
										SmoothVertex b, 
										SmoothVertex c, 
										int depth)
{
	if(depth==0) return _getHeightAtTriangle(x,y,a,b,c);

	Vec3f u = b.pos-a.pos;
	Vec3f v = c.pos-a.pos;
	Vec3f w = c.pos-b.pos;
	Vec3f p = Vec3f(x,y,0) - a.pos;

	float lambda = ( p.x * v.y - p.y * v.x ) / ( u.x * v.y - u.y * v.x );
	float mue =	   ( p.y * u.x - p.x * u.y ) / ( u.x * v.y - u.y * v.x );

	// create seeds at edge midpoints
	int s_ab = interpolateSeeds(a.seed,b.seed);
	int s_ac = interpolateSeeds(a.seed,c.seed);
	int s_bc = interpolateSeeds(b.seed,c.seed);

	// compute edge midpoints
	Vec3f ab = splitEdge( a.pos, cross(a.normal,Normalize(cross(u,a.normal))),
						  b.pos, cross(b.normal,Normalize(cross(-u,b.normal))));
	Vec3f ac = splitEdge( a.pos, cross(a.normal,Normalize(cross(v,a.normal))),
						  c.pos, cross(c.normal,Normalize(cross(-v,c.normal))));
	Vec3f bc = splitEdge( b.pos, cross(b.normal,Normalize(cross(w,b.normal))),
						  c.pos, cross(c.normal,Normalize(cross(-w,c.normal))));

	// displace split points
	ab.z = displaceHeight( s_ab, ab.z, length(u)*steepness );
	ac.z = displaceHeight( s_ac, ac.z, length(v)*steepness );
	bc.z = displaceHeight( s_bc, bc.z, length(w)*steepness );

	// create edge midpoints
	SmoothVertex AB = SmoothVertex( ab, Normalize((a.normal+b.normal)*0.5), s_ab );
	SmoothVertex AC = SmoothVertex( ac, Normalize((a.normal+c.normal)*0.5), s_ac );
	SmoothVertex BC = SmoothVertex( bc, Normalize((c.normal+b.normal)*0.5), s_bc );

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

float SmoothTriangleGrid::_getHeightAtTriangle( int x, int y, 
												SmoothVertex a, 
												SmoothVertex b, 
												SmoothVertex c)
{
	// find position on triangle using barycentric coordinates
	float ux = b.pos.x - a.pos.x;
	float uy = b.pos.y - a.pos.y;
	float vx = c.pos.x - a.pos.x;
	float vy = c.pos.y - a.pos.y;
	float px = (float)x - a.pos.x;
	float py = (float)y - a.pos.y;

	float lambda = (px*vy-py*vx)/(ux*vy-uy*vx);
	float mue = (py*ux-px*uy)/(ux*vy-uy*vx);

	// interpolate height values of the vertices
	return (1-lambda-mue)*a.pos.z + lambda*b.pos.z + mue*c.pos.z;
}

//-----------------------------------------------------------------------------


// this is identical to DynamicTriangleGrid::writeImage()
void SmoothTriangleGrid::writeImage(const char *filename)
{
	SDL_Surface* image = SDL_CreateRGBSurface(SDL_SWSURFACE,width+1,height+1,8,
											  0x000000ff,0x000000ff,0x000000ff,0);
	SDL_Surface* preview = SDL_CreateRGBSurface(SDL_SWSURFACE,width+1,height+1,
												32,0,0,0,0);
	
	SDL_LockSurface(image);
	SDL_LockSurface(preview);

	LogManager::log("creating heightmap",true);

	for( int y=0; y<height+1; y++ )
		for( int x=0; x<width+1; x++ )
		{	
			int h = getHeightAt(x,y,detail);
			h = MIN(255,MAX(0,h));
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