/******************************************************************************
 *	file: TriangleGrid.h
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


/** @file
 *	Triangle grid based terrain generators.
 *	The terrain generator classes in this file subdivide the map into a 
 *	triangle mesh and apply some sort of random height changes to each triangle
 *	vertex.
 */
	

#ifndef SMOOTHTRIANGLEDEBUG_H
#define SMOOTHTRIANGLEDEBUG_H

#include "config.hpp"
#include "SC4Landscape.h"
#include "Vec3f.h"

namespace debugtriangle
{

/**	A vertex of a FractalTriangle.
 *	It stores the coordinates and a seed. The seed is necessary to ensure that
 *	if two triangles share an edge, the deviation of the split points along
 *	that edge is identical on both triangles.
 */
struct Vertex
{
	Vec3f pos;
	int seed;

	Vertex() : seed(1) { }
	Vertex( float x, float y, float z, int seed ) : pos(x,y,z),seed(seed) { }

	Vec2f pos2D() { return Vec2f(pos.x,pos.y); }
};


//-----------------------------------------------------------------------------


/**	Fractal terrain generator that subdivides the triangles on the fly.
 *	This uses the same subdivision algorithm as the StaticTriangleGrid (and
 *	should produce the same results) but it doesn't have to precompute the
 *	whole mesh. This leads to much lower memory requirements, allowing you to
 *	use much higher detail levels.
 */
class SC4RRC_API DynamicTriangleGrid : public SC4Landscape
{
	Vertex A; ///< upper left corner
	Vertex B; ///< upper right corner
	Vertex C; ///< lower right corner
	Vertex D; ///< lower left corner

	float steepness; ///< steepness, influences the max deviation of the split point height values
	int detail; ///< detail level, specifies how often the base triangles will be split

	/** The absolute maximum height on the heightmap. */
	static const float MAX_HEIGHT;

	/**	Creates a height for a vertex with a given seed.
	 *	For the same input values, you always get the same output.
	 *	@param seed		the seed at the vertex
	 *	@param base		the base height at the vertex (avg of parents)
	 *	@param max		the max deviation from the base
	 */
	float createHeight( int seed, float base, float max );

	/**	Creates new seed from the old ones. 
	 *	@note	If you want to change the way the new seed is computed, make 
	 *			sure that the function remains commutative.
	 */
	__inline int interpolateSeeds( int seed1, int seed2 )
	{ 
		// If you just add the two seeds, the distribution of the random values
		// is far from uniform. The 99 is just an arbitrary value, and the random
		// values are still not really uniformly distributed but it's much 
		// better than with any other number I tried and should really be 
		// sufficient for our purposes.
		return seed1+seed2+99;
	}
	
	/**	Returns the terrain height at position (x|y).
	 *	This method computes the terrain height dynamically without storing the
	 *	complete triangle mesh in memory. This allows for much more detail than
	 *	the static approach.
	 *	@param detail	How often you want to split the triangles. 
	 */
	float getHeightAt(float x, float y, int detail);

	/**	Helper function for getHeightAt().
	 *	Recursively splits the triangle until the desired recursion depth is 
	 *	reached. Then it calls _getHeightAtTriangle().
	 *	@param x,y		The coordinates of the point of which you want to know the height.
	 *	@param a,b,c	The corners of the current triangle.
	 *	@param depth	How often the triangle has to be split.
	 */
	float _getHeightAt(float x, float y, Vertex a, Vertex b, Vertex c, int depth);

	/**	Helper function for _getHeightAt().
	 *	Returns the height value of the point (x|y) on the triangle abc.
	 */
	float _getHeightAtTriangle(float x, float y, Vertex a, Vertex b, Vertex c);

public:
	/**	@param detail	
	 *		Detail level for the triangle grid. Defines how often the base 
	 *		triangles will be split. 
	 *
	 *	@param steepness
	 *		Influences how "rough" the resulting landscape will be. The newly
	 *		created vertices' height values are always changed by a random
	 *		amount. The maximum change is defined by the length of the edge
	 *		that has been split multiplied by the steepness factor. So, lower
	 *		values give you a smoother landscape while higher values give you
	 *		higher mountains and deeper lakes.
	 *
	 *	@param seed
	 *		The seed for the pseudo-random function. If you use the same seed,
	 *		you will always get the same landscape. This way, if you find a
	 *		landscape with a shape that you like, you can adjust the level, 
	 *		blur and steepness settings to give it that final touch.
	 */
	DynamicTriangleGrid( int width, int height, int level, int blur,
						 int detail, float steepness, int seed );

	virtual ~DynamicTriangleGrid() { }

	/**	Creates a heightmap and a preview map and saves them. 
	 *	The heightmap is saved as a 8-Bit BMP file with the given filename.
	 *	The preview map that is only meant to give you a feel of what the 
	 *	region will look like in the game will be saved as a 32-Bit BMP file
	 *	called preview.bmp.
	 */
	void writeImage(const char* filename);
};

} // namespace debugtriangle

#endif // SMOOTHTRIANGLEDEBUG_H