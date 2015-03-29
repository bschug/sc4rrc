/******************************************************************************
 *  file:  Perlin.h
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

#ifndef SC4RRC__PERLIN_H
#define SC4RRC__PERLIN_H

#include <SDL/SDL_types.h>

#include "config.hpp"
#include "SC4Landscape.h"

typedef unsigned int uint;

__inline int intfloor(float f) { return f >= 0 ? int(f) : int(f)-1; }

/** Terrain generator using the Perlin Noise algorithm.
 *	This class implements the algorithm as it is found on
 *	http://wiki.delphigl.com/index.php/Perlin_Noise
 */
class SC4RRC_API Perlin : public SC4Landscape
{
	/** Interpolates between two values.
	 *	@param w	Weight of b. The heigher this is, the closer the result
	 *				will be to b.
	 */
	__inline float interpolate( float a, float b, float w )
	{
		return interpolateHermite(a,b,w);
	}

	/** Interpolates between four values. */
	__inline float interpolate( float x1y1, float x2y1, float x1y2, float x2y2, 
							    float w_x, float w_y )
	{
		return interpolate( interpolate(x1y1,x2y1,w_x),
							interpolate(x1y2,x2y2,w_x),
							w_y );
	}

	/** Hermite Spline interpolation function. 
	 *	f(t) = 3t²-2t³
	 */
	__inline float interpolateHermite( float a, float b, float w )
	{
		w = w*w*(3.0f-2.0f*w);
		return (1.0f-w)*a + w*b;
	}

	/** Adds random values of a certain frequency to the heightmap.
	 *	@param heightmap	An array of size width*height
	 *	@param frequency	Number of random grid points in x and y direction
	 *	@param factor		Strength of the current frequency
	 */
	void addFrequency( float* heightmap, int frequency, float factor );

	/** Takes a point on the gridmap and computes the interpolated value of
	 *	its four neighbouring grid points.
	 *	@param gridmap	The gridmap the values are taken from.
	 *	@param pitch	width of the gridmap
	 *	@param x		x coordinate of the point on the gridmap
	 *	@param y		y coordinate of the point
	 */
	__inline float getValue( float* gridmap, int pitch, float x, float y )
	{
		int x1 = intfloor(x);
		int y1 = intfloor(y);
		int x2 = x1+1;
		int y2 = y1+1;
		float wx = x-x1;
		float wy = y-y1;
		return interpolate( gridmap[x1+y1*pitch], gridmap[x2+y1*pitch],
							gridmap[x1+y2*pitch], gridmap[x2+y2*pitch], wx, wy );
	}

	/** Builds the heightmap.
	 *	This is where the actual Perlin Noise algorithm sits.
	 *	A grid with random heights and relatively large grid size is created
	 *	and the points between the grid vertices are interpolated.
	 *	This is repeated multiple times with smaller grid sizes and ranges for
	 *	the random values and all of these interpolated values are added 
	 *	together.
	 *
	 *	@return	An array of width*height float values.
	 */
	float* buildHeightmap();

	/** Adjusts the heightmap to fit into the desired range.
	 *	The values in the heightmap are transformed in such a way that no
	 *	point on the heightmap is higher than the peak value or lower than
	 *	the bottom value and that exactly the desired percentage of the
	 *	terrain is below the waterline.
	 *	The general features of the heightmap should still be perserved.
	 */
	__inline void adjustHeightmap(float* heightmap)
	{
		adjustMinMax(heightmap);
	}

	/** @see adjustHeightmap */
	void adjustMinMax(float* heightmap);

	float roughness;
	int detail;

	int bottom;
	int peak;
	float water;


public:
	/** @param width	@see SC4Landscape::SC4Landscape
	 *	@param height	@see SC4Landscape::SC4Landscape
	 *	@param level	This is ignored. Use water percentage instead.
	 *	@param blur		@see SC4Landscape::SC4Landscape
	 *	@param seed		Seed for the pseudorandom generator.
	 *	@param roughness	The higher this is, the steeper will the slopes on
	 *						the terrain be. Increasing this value makes the
	 *						mountains higher and the valleys deeper.
	 *						The default value is 0.5.
	 *	@param detail	Detail of computation. The higher this is, the more 
	 *					detailed the resulting landscape will be, but it will
	 *					also take longer to compute. Also, more detail is not
	 *					always what you want. Keep in mind that you still want
	 *					to build a city on the map.
	 *	@param bottom	Optional. Set the lowest value the algorithm is allowed
	 *					to generate. Must be a value between 0 and 255.
	 *	@param peak		Optional. Set the highest value the algorithm is 
	 *					allowed to generate. Must be a value between 0 and 255.
	 *					Must be greater or equal than bottom.
	 *	@param water	Water Percentage. The amount of water the heightmap
	 *					should contain.
	 */
	Perlin( int width, int height, int level, int blur, uint seed,
			int detail,	float roughness=0.5, int bottom=0, int peak=255,
			float water = 0.2f );

	virtual ~Perlin();

	virtual void writeImage(const char* filename);
};


#endif