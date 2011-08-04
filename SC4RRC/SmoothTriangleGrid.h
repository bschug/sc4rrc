/******************************************************************************
 *	file: SmoothTriangleGrid.h
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

#ifndef SC4RRC__SMOOTHTRIANGLEGRID_H
#define SC4RRC__SMOOTHTRIANGLEGRID_H

#include "SC4Landscape.h"
#include "Vec3f.h"

/** Square function for floats. */
inline float sqf(float f) { return f*f; }

/** A vertex with a surface normal.
 *	This is used by the SmoothTriangleGrid class. It's like a normal Vertex
 *	but it also stores the normal of the terrain surface at that point. The
 *	normal is defined by the positions of the neighbouring vertices in the
 *	current detail level.
 */
struct SmoothVertex
{
	Vec3f pos;
	Vec3f normal;
	int seed;

	SmoothVertex() : pos(Vec3f()),normal(Vec3f()),seed(0) { }

	SmoothVertex( Vec3f pos, Vec3f normal, int seed ) 
	: pos(pos),normal(normal),seed(seed) { }

	inline Vec2f pos2d() const { return Vec2f(pos.x,pos.y); }
};


/** Dynamic Triangle Grid with Normal Mapping.
 *	This class applies a kind of normal mapping to the grid produced by the
 *	DynamicTriangleGrid terrain generator in order to avoid the visible
 *	artifacts like long straight crests along the edges of the first detail
 *	level's triangles. 
 *	The initial positions of the split points are still just in the middle of 
 *	the triangles edges, but in this case the edges are no longer straight 
 *	lines. Instead, they are curved, and the curvature is determined by the
 *	surface normals of the respective triangles.
 */
class SmoothTriangleGrid : public SC4Landscape
{
	SmoothVertex A; ///< upper left corner
	SmoothVertex B; ///< upper right corner
	SmoothVertex C;	///< lower right corner
	SmoothVertex D; ///< lower left corner

	float steepness; ///< influences the max deviation of the splitpoint height values
	int detail; ///< specifies how often the base triangle will be split

	const float MAX_HEIGHT; ///< maximum height of the terrain
	const float MIN_HEIGHT; ///< minimum height of the terrain

	/** Displaces the height value of a split point. 
	 *	The same input always returns the same output.
	 *	@param seed		The seed at the vertex you want to displace.
	 *	@param base		The current height of the vertex.
	 *	@param max		The maximum displacement allowed for this vertex.
	 */
	float displaceHeight( int seed, float base, float max );

	/** Computes the position of a split point.
	 *	The curvature of the edge which is defined by the surface normals at
	 *	the vertices is taken into account here. The edge is treated as a
	 *	Hermite Spline for that purpose.
	 *	This is the same as calling hermiteSpline(a,da,b,db,0.5).
	 *	@param a,b	The coordinates of the vertices that mark the ends of the
	 *				edge you want to split.
	 *	@param da, db	The direction of the edge at the respective vertex.
	 */
	Vec3f splitEdge(Vec3f a, Vec3f da, Vec3f b, Vec3f db)
	{
		// The general formula for the Hermite Spline is
		// H(t) = a  * (1-t)^2 * (1+2t)
		//      + da * t*(1-t)^2
		//		- db * t^2 * (1-t)
		//		+ b  * (3-2t) * t^2
		//
		// We are only interested in t=0.5, so the whole thing becomes a little
		// simpler: a*0.5 + da*0.125 - db*0.125 + b*0.5
		//
		// Reforming this allows for removing some more multiplications:
		return (a+b+(da-db)*0.25)*0.5;
	}

	/**	Creates new seed from the old ones. 
	 *	@note	If you want to change the way the new seed is computed, make 
	 *			sure that the function remains commutative, i.e. that
	 *			interpolateSeeds(a,b) = interpolateSeeds(b,a) for all a,b.
	 */
	__inline int interpolateSeeds( int seed1, int seed2 )
	{ 
		// If you just add the two seeds, the distribution of the random values
		// is far from being uniform. The 99 is just an arbitrary value, and 
		// the random values are still not really uniformly distributed but 
		// it's much better than with any other number I tried and should 
		// really be sufficient for our purposes.
		return seed1+seed2+99;
	}

	/**	Returns the terrain height at position (x|y).
	 *	This works just like DynamicTriangleGrid::getTerrainHeight().
	 *	@param detail	How often you want to split the triangles. 
	 */
	float getHeightAt(int x, int y, int detail);

	/**	Helper function for getHeightAt().
	 *	Recursively splits the triangle until the desired recursion depth is 
	 *	reached. Then it calls _getHeightAtTriangle().
	 *	When splitting triangles, the edges are treated as curves instead of 
	 *	straight lines. This should lead to much less visible discontinuities
	 *	in the resulting heightmap.
	 *	@param x,y		The coordinates of the point of which you want to know 
	 *					the height.
	 *	@param a,b,c	The corners of the current triangle.
	 *	@param depth	How often the triangle has to be split. This is reduced
	 *					by one in each iteration.
	 */
	float _getHeightAt( int x, int y, 
						SmoothVertex a, SmoothVertex b, SmoothVertex c, 
						int depth);

	/**	Helper function for _getHeightAt().
	 *	Returns the height value of the point (x|y) on the triangle abc.
	 *	In this step, the normals are again used to compute the actual height
	 *	value of the point.
	 */
	float _getHeightAtTriangle( int x, int y, 
								SmoothVertex a, SmoothVertex b, SmoothVertex c);

	/** Computes a point on the triangle from the viewpoint of vertex A.
	 *	You must interpolate the viewpoints of all three vertices to get a
	 *	continuous surface.
	 */
	Vec3f _getPointOnTriangle( const SmoothVertex& A, const SmoothVertex& B, 
							   const SmoothVertex& C, const Vec2f& p2d );
	
	/** Returns a the point at position t on the Hermite Spline that is defined
	 *	by the points a and b and the tangent vectors da and db.
	 */
	inline Vec3f hermiteSpline( Vec3f a, Vec3f da, Vec3f b, Vec3f db, float t )
	{
		// H(t) = a  * (1-t)^2 * (1+2t)
		//      + da * t*(1-t)^2
		//		- db * t^2 * (1-t)
		//		+ b  * (3-2t) * t^2
		return a  * sqf(1.f-t) * (1.f+2.f*t)
			 + da * t*sqf(1.f-t)
			 - db * sqf(t) * (1.f-t)
			 + b  * (3.f-2.f*t) * sqf(t);
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
	SmoothTriangleGrid( int width, int height, int level, int blur,
						int detail, float steepness, int seed );

	~SmoothTriangleGrid() { }

	/**	Creates a heightmap and a preview map and saves them. 
	 *	The heightmap is saved as an 8-Bit BMP file with the given filename.
	 *	The preview map that is only meant to give you a feel of what the 
	 *	region will look like in the game will be saved as a 32-Bit BMP file
	 *	called preview.bmp.
	 */
	void writeImage(const char* filename);
};


#endif