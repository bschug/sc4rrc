/******************************************************************************
 *	file: Perlin.cpp
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

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <list>
#include <vector>

#include <limits>
using std::numeric_limits;

#define _USE_MATH_DEFINES
#include <math.h>
#include <memory.h>

#include <SDL/SDL.h>

#include "Perlin.h"
#include "LogManager.h"
#include "postprocessing.h"

__inline float randf() { return float(rand()-(RAND_MAX/2)) / float(RAND_MAX); }
__inline float randf(float min, float max) { return min + fabs(randf()) * (max - min); }

__inline int MAX(int a, int b) { return a>b?a:b; }
__inline int MIN(int a, int b) { return a<b?a:b; }


//-----------------------------------------------------------------------------


Perlin::Perlin( int width, int height, int level, int blur, uint seed,
				int detail, float roughness, int bottom, int peak, float water )
 :	SC4Landscape(width,height,level,blur), detail(detail), 
	roughness(roughness), bottom(bottom), peak(peak), water(water)
{
	if(bottom < 0 || bottom > 255)
	{
		LogManager::log("Invalid bottom value. Using 0.",true);
		bottom = 0;
	}
	if(peak > 255 || peak < bottom)
	{
		LogManager::log("Invalid peak value. Using 255.",true);
		peak = 255;
	}
	if(water < 0.0f || water > 1.0f)
	{
		LogManager::log("Invalid water percentage. Using 1.0",true);
		water = 0.2f;
	}
	

	std::ostringstream* o = new std::ostringstream;
	LogManager::endl();
	LogManager::log("Perlin Noise Terrain Generator",true);
	LogManager::log("Settings:",true);
	(*o) << "  map size: " << width << " x " << height;
	LogManager::log(o,true);
	(*o) << "  minimal terrain height: " << bottom;
	LogManager::log(o,true);
	(*o) << "  maximal terrain height: " << peak;
	LogManager::log(o,true);
	(*o) << "  water percentage: " << water;
	LogManager::log(o,true);
	(*o) << "  blur amount: " << blur;
	LogManager::log(o,true);
	(*o) << "  roughness: " << roughness;
	LogManager::log(o,true);
	(*o) << "  detail level: " << detail;
	LogManager::log(o,true);
	(*o) << "  seed: " << seed;
	LogManager::log(o,true);
	LogManager::endl();

	LogManager::log("Initializing pseudorandom number generator");

	srand(seed);
}

//-----------------------------------------------------------------------------

Perlin::~Perlin()
{
}

//-----------------------------------------------------------------------------

void Perlin::writeImage(const char *filename)
{
	// 8-bit grayscale surface for the heightmap
	SDL_Surface* image = SDL_CreateRGBSurface(SDL_SWSURFACE,width+1,height+1,8,
											  0x000000ff,0x000000ff,0x000000ff,0);
	// 32-bit color surface for the preview image
	SDL_Surface* preview = SDL_CreateRGBSurface(SDL_SWSURFACE,width+1,height+1,
												32,0,0,0,0);

	SDL_LockSurface(image);
	SDL_LockSurface(preview);

	LogManager::log("creating heightmap",true);
	float* heightmap = buildHeightmap();

	LogManager::log("adjusting heightmap");
	adjustHeightmap(heightmap);

	// build the actual heightmap image
	for(int y=0; y<height; y++)
	{
		for(int x=0; x<width; x++)
		{
			Uint8 h = MIN( 255, MAX( 0, int(heightmap[x+y*width]) ) );

			int ofs = x + y * image->pitch;
			((Uint8*)image->pixels)[ofs] = h;
		}
	}

	// the temporary heightmap is not needed anymore
	delete[] heightmap;

	blurImage(image,blur);
    adjustWaterPercentage (image, water);
    adjustLevels (image);

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
}

//-----------------------------------------------------------------------------

void Perlin::addFrequency(float *heightmap, int frequency, float amplitude)
{
	float* gridmap = new float[(frequency+1)*(frequency+1)];
	for(int i=0; i<(frequency+1)*(frequency+1); i++)
	{
		gridmap[i] = randf() * amplitude;
	}

	// current position on the gridmap
	float gx = 0.0f;
	float gy = 0.0f;

	// how far you move on the gridmap for each step on the heightmap
	float x_step = float(frequency) / float(width);
	float y_step = float(frequency) / float(height);

	for(int y=0; y<height; y++)
	{
		for(int x=0; x<width; x++)
		{
			heightmap[x+y*width] += getValue(gridmap,frequency+1,gx,gy);
			gx += x_step;
		}
		gx = 0.0f;
		gy += y_step;
	}
}

//-----------------------------------------------------------------------------

float* Perlin::buildHeightmap()
{
	// temporary heightmap for accumulating the different frequencies
	float* heightmap = new float[width*height];
	for(int i=0; i<width*height; i++)
		heightmap[i] = 0.0f;

	// add all the frequencies together
	int frequency = 1;
	float amplitude = 1.0f;
	for(int d=0; d<detail; d++)
	{
		addFrequency(heightmap,frequency,amplitude);
		frequency *= 2;
		amplitude *= roughness;
	}

	return heightmap;
}

//-----------------------------------------------------------------------------

void Perlin::adjustMinMax(float *heightmap)
{
	// find min and max
	float min = std::numeric_limits<float>::max();
	float max = std::numeric_limits<float>::min();
	for(int i=0; i < width*height; i++)
	{
		min = heightmap[i] < min ? heightmap[i] : min;
		max = heightmap[i] > max ? heightmap[i] : max;
	}

	// bring values to desired range
	float shift = min < 0 ? -min : 0;
	float factor = float(peak) / (max-min);
	for(int i=0; i < width*height; i++)
	{
		heightmap[i] += shift;
		heightmap[i] *= factor;
		heightmap[i] += float(bottom);
	}
}
