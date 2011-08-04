/******************************************************************************
 *	file: Vec3f.h
 *
 *	Copyright (c) 2005-2007 
 *	Universität des Saarlandes 
 *	Benjamin Schug
 *
 *	The original version of this file was published by the Computer Graphics 
 *	Department of the University of Saarbrücken ("Universität des Saarlandes")
 *	as a part of a simple raytracer framework for the students in the Computer 
 *	Graphics lecture.
 *	The Vec2f class and some additions and changes to the original Vec3f class
 *	were made by Benjamin Schug later on.
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

#ifndef OC__VEC3F_H
#define OC__VEC3F_H

#include <math.h>
#include <iostream>

using std::ostream;

enum Axis { XAXIS, YAXIS, ZAXIS };
__inline float MAX(float a, float b) { return a>b?a:b; }
__inline float MIN(float a, float b) { return a<b?a:b; }
const float EPSILON = 0.0000000001f;

/** Class for 2-dimensional vectors with float coordinates. */
class Vec2f
{
public:
	float x,y;

	Vec2f() { x = 0.0f;	y = 0.0f; }
	
	Vec2f(float x, float y) : x(x),y(y) { }

	Vec2f(int x, int y) : x((float)x),y((float)y) { }

	inline Vec2f& operator=(const Vec2f& b)
	{ x=b.x; y=b.y; return *this; }

	inline Axis maxDim() const
	{ return x>y ? XAXIS : YAXIS; }
};

/** Class for 3-dimensional vectors with float coordinates. */
class Vec3f
{
public:
	float x,y,z;

	Vec3f() { x = 0.0f;	y = 0.0f; z = 0.0f; };

	Vec3f(float x,float y, float z) : x(x),y(y),z(z) { };

	Vec3f(int x, int y, int z) : x((float)x),y((float)y),z((float)z) { }

	Vec3f(const Vec2f& v, float z) : x(v.x), y(v.y), z(z) { }

	inline const float& operator[](const int i) const
	{ return *(&x+i); };

	inline float& operator[](const int i)
	{ return *(&x+i); };

	inline Vec3f &operator=(const Vec3f &b)
	{ x = b.x; y = b.y; z = b.z; return *this;};

	/** Returns the dimension in which the vector extends most. */
	inline Axis maxDim() const
	{
		return x>y ? ( x>z ? XAXIS : ZAXIS ) : ( y>z ? YAXIS : ZAXIS );
	}

	/** Makes all coords less or equal to the respective coords of \a other. 
	 *	This is useful e.g. if you want to make sure that no coordinate is 
	 *	greater than 1: \code vec.SetMin( Vec3f(1,1,1) ); \endcode
	 */
	inline void setMin(Vec3f &other)
	{
		x = MIN(x,other.x);
		y = MIN(y,other.y);
		z = MIN(z,other.z);
	}

	/**	Makes all coords greater or equal to the respective coords of \a other.
	 *	This is useful e.g. if you want to make sure that no coordinate is
	 *	lower than 0: \code vec.SetMax( Vec3f(0,0,0) ); \endcode
	 */
	inline void setMax(Vec3f &other)
	{
		x = MAX(x,other.x);
		y = MAX(y,other.y);
		z = MAX(z,other.z);
	};

	/**	Clamps all coords to values between 0 and 1.
	 *	This is equivalent to calling setMin(Vec3f(1,1,1)) AND 
	 *	setMin(Vec3f(0,0,0)) on the object.
	 */
	inline Vec3f& validPixel()
	{
		x = MAX( 0, MIN( 1, x ) );
		y = MAX( 0, MIN( 1, y ) );
		z = MAX( 0, MIN( 1, z ) );
		return (*this);
	}
};

//-----------------------------------------------------------------------------

/*! dot product */
inline float dot(const Vec2f &a, const Vec2f &b)
{ return a.x * b.x + a.y * b.y; }

inline float dot(const Vec3f &a, const Vec3f &b)
{ return a.x * b.x + a.y * b.y + a.z * b.z; };


/*! component-wise product */
inline Vec2f product(const Vec2f &a, const Vec2f &b)
{ return Vec2f( a.x * b.x, a.y * b.y ); }

inline Vec3f product(const Vec3f &a, const Vec3f &b)
{ return Vec3f( a.x * b.x, a.y * b.y, a.z * b.z ); };


/*! vector (cross) product */
inline Vec3f cross(const Vec3f &a, const Vec3f &b)
{ 
	return Vec3f(a.y*b.z-a.z*b.y,
				 a.z*b.x-a.x*b.z,
				 a.x*b.y-a.y*b.x); 
};

inline Vec3f operator^(const Vec3f &a, const Vec3f &b)
{ 
	return Vec3f(a.y*b.z-a.z*b.y,
				 a.z*b.x-a.x*b.z,
				 a.x*b.y-a.y*b.x); 
};


/** negation */
inline Vec2f operator-(const Vec2f &v)
{ return Vec2f(-v.x,-v.y); }

inline Vec3f operator-(const Vec3f &v)
{ return Vec3f(-v.x,-v.y,-v.z); };


/** Euclidean length */
inline float length(const Vec2f &v)
{ return sqrt(dot(v,v)); }

inline float length(const Vec3f &v)
{ return sqrt(dot(v,v)); };


/** scalar product */
inline Vec2f operator*(const float f, const Vec2f &v)
{ return Vec2f(f*v.x, f*v.y); }

inline Vec3f operator*(const float f, const Vec3f &v)
{ return Vec3f(f*v.x, f*v.y, f*v.z); };


/** scalar product */
inline Vec2f operator*(const Vec2f &v, const float f)
{ return Vec2f(f*v.x, f*v.y); };

inline Vec3f operator*(const Vec3f &v, const float f)
{ return Vec3f(f*v.x, f*v.y, f*v.z); };


/** scalar product */
inline void operator*=(Vec2f &v, const float f)
{ v.x *= f; v.y *= f; };

inline void operator*=(Vec3f &v, const float f)
{ v.x *= f; v.y *= f; v.z *= f; };


/** component-wise product */
inline void operator*=(Vec2f &v, const Vec2f &f)
{ v.x *= f.x; v.y *= f.y; }

inline void operator*=(Vec3f &v, const Vec3f &f)
{ v.x *= f.x; v.y *= f.y; v.z *= f.z; };


/** scalar division */
inline Vec2f operator/(const Vec2f &v, const float f)
{ return (1/f)*v; };

inline Vec3f operator/(const Vec3f &v, const float f)
{ return (1/f)*v; };


/** scalar division */
inline void operator/=(Vec2f &v, const float f)
{ v *= (1/f); };

inline void operator/=(Vec3f &v, const float f)
{ v *= (1/f); };


/** addition */
inline Vec2f operator+(const Vec2f &a, const Vec2f &b)
{ return Vec2f(a.x+b.x, a.y+b.y); };

inline Vec3f operator+(const Vec3f &a, const Vec3f &b)
{ return Vec3f(a.x+b.x, a.y+b.y, a.z+b.z); };


/** addition */
inline Vec2f& operator+=(Vec2f &a, const Vec2f &b)
{ a.x += b.x; a.y += b.y; return a; }

inline Vec3f& operator+=(Vec3f &a, const Vec3f &b)
{ a.x += b.x; a.y += b.y; a.z += b.z; return a; }


/**	@note	This doesn't simply check equality because of possible rounding
 *			errors. It keeps a tolerance of EPSILON. (see constants.h)
 */
inline bool operator==(const Vec2f& a, const Vec2f& b)
{ return fabsf(a.x-b.x)<EPSILON && fabsf(a.y-b.y)<EPSILON; }

inline bool operator==(const Vec3f& a, const Vec3f& b)
{ 
	return fabsf(a.x-b.x)<EPSILON && 
		   fabsf(a.y-b.y)<EPSILON && 
		   fabsf(a.z-b.z)<EPSILON; 
}


/** subtraction */
inline Vec2f operator-(const Vec2f &a, const Vec2f &b)
{ return Vec2f(a.x-b.x, a.y-b.y); };

inline Vec3f operator-(const Vec3f &a, const Vec3f &b)
{ return Vec3f(a.x-b.x, a.y-b.y, a.z-b.z); };


/** normalization */
inline void normalize(Vec2f &v)
{ v *= (1.0f/length(v)); }

inline void normalize(Vec3f &v)
{ v *= (1.0f/length(v)); };

inline Vec3f Normalize(const Vec3f &v)
{ return v * (1.0f/length(v)); }


/** output */
inline ostream &operator<<(ostream &o,const Vec2f &v)
{ o << "(" << v.x << "," << v.y << ")"; return o; }

inline ostream &operator<<(ostream &o,const Vec3f &v)
{ o << "(" << v.x << "," << v.y << "," << v.z << ")"; return o; }


/** Every dimension is minimized separately:
 *	<tt> min( (1,2,3), (2,1,0) ) = (1,1,0) </tt>
 */
inline Vec3f min(const Vec3f a, const Vec3f b)
{ return Vec3f(MIN(a.x,b.x),MIN(a.y,b.y),MIN(a.z,b.z)); }


/** Every dimension is maximized separately:
 *	<tt> max( (1,2,3), (2,1,0) ) = (2,2,3) </tt>
 */
inline Vec3f max(const Vec3f a, const Vec3f b)
{ return Vec3f(MAX(a.x,b.x),MAX(a.y,b.y),MAX(a.z,b.z)); }


/** @return	The largest coordinate value in the vector. */
inline float maxValue(const Vec3f a)
{
	return a[a.maxDim()];
}

/** Removes all negative signs from the coordinate values. */
inline Vec3f abs(const Vec3f &a)
{
	return Vec3f(fabs(a.x),fabs(a.y),fabs(a.z));
}

#endif // OC__VEC3F_H


