/******************************************************************************
 *	file: SC4Landscape.h
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


#ifndef SC4LANDSCAPE_H
#define SC4LANDSCAPE_H

#include "config.hpp"


/**	Base class for fractal terrain generators. */
class SC4RRC_API SC4Landscape
{
protected:
	int width; 
	int height; 
	int level; 
	int blur;

	/** The absolute maximum height on the heightmap. */
	static const int MAX_HEIGHT = 255;

	/**	@param width	Width of the region in kilometers (1km = 1 small city)
	 *	@param height	Height of the region in kilometers
	 *	@param level	Average height above sea level. You can use this to
	 *					shift the land up or down.
	 *	@param blur		the amount of blur that should be added to the final heightmap
	 */
	SC4Landscape( int width, int height, int level, int blur)
	: width(width*64),height(height*64),level(level),blur(blur) 
	{ }

public:
	virtual ~SC4Landscape() { }

	/**	Creates a heightmap and a preview map and saves them. 
	 *	The heightmap is saved as an 8-Bit BMP file with the given filename.
	 *	The preview map that is only meant to give you a feel of what the 
	 *	region will look like in the game will be saved as a 32-Bit BMP file
	 *	called preview.bmp.
	 */
	virtual void writeImage(const char* filename) = 0;
};

#endif // SC4LANDSCAPE_H