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
	

#ifndef TRIANGLEGRID_H
#define TRIANGLEGRID_H

#include "SC4Landscape.h"

/**	A vertex of a FractalTriangle.
 *	It stores the coordinates and a seed. The seed is necessary to ensure that
 *	if two triangles share an edge, the deviation of the split points along
 *	that edge is identical on both triangles.
 */
struct Vertex
{
	float x;
	float y;
	int z;
	int seed;

	Vertex() : x(0),y(0),z(0),seed(1) { }
	Vertex( float x, float y, int z, int seed ) : x(x),y(y),z(z),seed(seed) { }
};


/** Fractal terrain generator using a pregenerated triangle grid.
 *	This terrain generator uses the diamond-square algorithm to subdivide the
 *	map into a set of triangles with random height shifts applied to the 
 *	vertices.
 *	First, the rectangular map is split into two triangles. The vertices of 
 *	these triangles are moved up or down by a random amount. Then, depending
 *	on the detail level, the triangles are split into four new triangles again
 *	and again, applying a random shift to the newly created vertices each time.
 *
 *	This terrain generator has a few weaknesses, however: The most problematic
 *	one is its enormous memory hunger, especially on higher detail levels. This
 *	is because it pre-builds the whole random triangle grid and keeps it in 
 *	memory until the intial (unfiltered) heightmap has been drawn. The other 
 *	problem is that the resulting landscape often still looks quite "edgy",
 *	even after blurring it. This is a problem of the underlying triangle grid
 *	algorithm.
 *
 *	Just don't use this one anymore, it's hopelessly deprecated.
 *	I just keep it in here because it is easy to understand and if you 
 *	understand this one, you will also understand the DynamicTriangleGrid.
 */
class StaticTriangleGrid : public SC4Landscape
{
	///////////////////////////////////////////////////////////////////////////
	//			Classes used by StaticTriangleGrid
	///////////////////////////////////////////////////////////////////////////

	/** Base class for the static fractal triangle mesh. */
	struct FractalTriangle
	{
		virtual ~FractalTriangle() { };
		virtual int getHeightAt(float x, float y) = 0;
	};

	/** FractalTriangle that is split into smaller triangles.
	 *	You can see the triangle mesh as a tree where the CompositeTriangles are
	 *	the inner nodes and the AtomicTriangles are the leafs. 
	 */
	struct CompositeTriangle : public FractalTriangle
	{
		Vertex A;
		Vertex B;
		Vertex C;

		FractalTriangle* tri_I;		///< sub-triangle at point A
		FractalTriangle* tri_II;	///< sub-triangle at point B
		FractalTriangle* tri_III;	///< sub-triangle at point C
		FractalTriangle* tri_IV;	///< sub-triangle in the middle

		virtual ~CompositeTriangle();
		virtual int getHeightAt(float x, float y);
	};

	/** Triangles that are not split. */
	struct AtomicTriangle : public FractalTriangle
	{
		Vertex A;
		Vertex B;
		Vertex C;

		AtomicTriangle(Vertex A, Vertex B, Vertex C) : A(A),B(B),C(C) { }
		virtual ~AtomicTriangle() { }
		virtual int getHeightAt(float x, float y);
	};


	///////////////////////////////////////////////////////////////////////////
	//		member variables and functions
	///////////////////////////////////////////////////////////////////////////

	/** Builds the triangle mesh statically.
	 *	@param A,B,C	the corners of the base triangle.
	 *	@param depth	how often the triangle should be split.
	 *	@return			a FractalTriangle containing the complete mesh.
	 */
	FractalTriangle* buildTriangleMesh(Vertex A,Vertex B,Vertex C,int depth);

	Vertex A; ///< upper left corner
	Vertex B; ///< upper right corner
	Vertex C; ///< lower right corner
	Vertex D; ///< lower left corner

	int detail;
	float steepness;

	/**	Creates a height for a vertex with a given seed.
	 *	For the same input values, you always get the same output.
	 *	@param seed		the seed at the vertex
	 *	@param base		the base height at the vertex (avg of parents)
	 *	@param max		the max deviation from the base
	 */
	int createHeight( int seed, int base, int max );

	/**	Creates new seed from the old ones. 
	 *	@note	If you want to change the way the new seed is computed, make 
	 *			sure that the function remains commutative.
	 */
	__inline int interpolateSeeds( int seed1, int seed2 )
	{ 
		// If you just add the two seeds, the distribution of the random values
		// is far from equal. The 99 is just an arbitrary value, and the random
		// values are still not really uniformly distributed but it's much 
		// better than with any other number I tried and should really be 
		// sufficient for our purposes.
		return seed1+seed2+99;
	}


public:
	/**	@param detail	
	 *		Detail level for the triangle grid. Defines how often the base 
	 *		triangles will be split. Be warned that the	memory requirements 
	 *		increase exponentially with the	detail level.
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
	StaticTriangleGrid( int width, int height, int level, int blur,
						int detail, float steepness, int seed );

	virtual ~StaticTriangleGrid() { }

	/**	Build the heightmap.
	 *	This maps the height values from the triangle mesh to the pixels of the
	 *	image. The size of the image is defined by the width and height values.
	 */
	void writeImage(const char* filename);
};


//-----------------------------------------------------------------------------


/**	Fractal terrain generator that subdivides the triangles on the fly.
 *	This uses the same subdivision algorithm as the StaticTriangleGrid (and
 *	should produce the same results) but it doesn't have to precompute the
 *	whole mesh. This leads to much lower memory requirements, allowing you to
 *	use much higher detail levels.
 */
class DynamicTriangleGrid : public SC4Landscape
{
	Vertex A; ///< upper left corner
	Vertex B; ///< upper right corner
	Vertex C; ///< lower right corner
	Vertex D; ///< lower left corner

	float steepness; ///< steepness, influences the max deviation of the split point height values
	int detail; ///< detail level, specifies how often the base triangles will be split

	/** The absolute maximum height on the heightmap. */
	static const int MAX_HEIGHT = 255;

	/**	Creates a height for a vertex with a given seed.
	 *	For the same input values, you always get the same output.
	 *	@param seed		the seed at the vertex
	 *	@param base		the base height at the vertex (avg of parents)
	 *	@param max		the max deviation from the base
	 */
	int createHeight( int seed, int base, int max );

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
	int getHeightAt(int x, int y, int detail);

	/**	Helper function for getHeightAt().
	 *	Recursively splits the triangle until the desired recursion depth is 
	 *	reached. Then it calls _getHeightAtTriangle().
	 *	@param x,y		The coordinates of the point of which you want to know the height.
	 *	@param a,b,c	The corners of the current triangle.
	 *	@param depth	How often the triangle has to be split.
	 */
	int _getHeightAt(int x, int y, Vertex a, Vertex b, Vertex c, int depth);

	/**	Helper function for _getHeightAt().
	 *	Returns the height value of the point (x|y) on the triangle abc.
	 */
	int _getHeightAtTriangle(int x, int y, Vertex a, Vertex b, Vertex c);

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

#endif // TRIANGLEGRID_H