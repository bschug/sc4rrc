/******************************************************************************
 *	file: main.cpp
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
#include <vector>
#include <time.h>

#include <iostream>
#include <fstream>

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

#include "LogManager.h"
#include "TriangleGrid.h"
#include "SmoothTriangleGrid.h"
#include "Perlin.h"
#include "SmoothTriangleDebug.h"

#ifdef _WIN32
#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>

    int initSDL (Uint32 flags)
    {
        int result = SDL_Init (flags);
        freopen("CON", "w", stdout);
        freopen("con", "w", stderr);
        return result;
    }
#else
    int initSDL (Uint32 flags) { return SDL_Init (flags); }
#endif

__inline float randf() { return (float)rand() / (float)RAND_MAX; }


int main(int argc, char** argv)
{
	SDL_Init(SDL_INIT_TIMER);

	// general options
	int width;
	int height;
	int level;
	int blur;
	int seed;

	enum TerrainGenerator { NOT_SET, STATIC, DYNAMIC, PERLIN, HERMITE, DEBUG };
	TerrainGenerator generator = NOT_SET;

	// triangle-grid specific
	float steepness;
	int detail;

	// perlin noise specific
	float roughness;
	int bottom, peak;
	float water;

	int seed_arg_nr;

	// General Options
	// If the options are not given in the command line, we ask the user to
	// type them in.
	if(argc < 6)
	{
		if(argc > 1)
		{
			if(std::string(argv[1])=="--fullreport")
			{
				LogManager::setFullReport(true);
			}
		}

		std::cout << "SC4 Random Region Creator" << std::endl;
		std::cout << "See readme.txt for detailed instructions." << std::endl;
		std::cout << "width: ";
		std::cin >> width;
		std::cout << "height: ";
		std::cin >> height;
		std::cout << "level: ";
		std::cin >> level;
		std::cout << "blur: ";
		std::cin >> blur;

		std::string gen;
		std::cout << "Select terrain generator:" << std::endl
				  << "  (t) Triangle Grid" << std::endl
				  << "  (h) Hermite Spline Triangle Grid" << std::endl
				  << "  (p) Perlin Noise" << std::endl;
		std::cin >> gen;
		if(gen=="t") generator = DYNAMIC;
		if(gen=="s") generator = STATIC;
		if(gen=="p") generator = PERLIN;
		if(gen=="h") generator = HERMITE;
		if(gen=="d") generator = DEBUG;
	}
	else
	{
		width = atoi(argv[1]);
		height = atoi(argv[2]);
		level = atoi(argv[3]);
		blur = atoi(argv[4]);

		if(argv[5][0]=='s') generator = STATIC;
		if(argv[5][0]=='t') generator = DYNAMIC;
		if(argv[5][0]=='p') generator = PERLIN;
		if(argv[5][0]=='h') generator = HERMITE;
		if(argv[5][0]=='d') generator = DEBUG;
	}

	if(generator==NOT_SET)
	{
		std::cout << "Invalid command line arguments." << std::endl
				  << "You need to specify a terrain generator." << std::endl
				  << "See readme.txt for detailed instructions." << std::endl;
		return -1;
	}
	// Triangle Grid Options
	else if(generator==DYNAMIC || generator==STATIC || generator==HERMITE || generator==DEBUG)
	{
		if(argc < 8)
		{
			std::cout << std::endl
					  << "Triangle Grid settings:" << std::endl
					  << "  steepness: ";
			std::cin >> steepness;
			std::cout << "  detail level: ";
			std::cin >> detail;
		}
		else
		{
			steepness = atof(argv[6]);
			detail = atoi(argv[7]);
		}

		seed_arg_nr = 8;
	}
	else if( generator == PERLIN )
	{
		if(argc < 11)
		{
			std::cout << std::endl
					  << "Perlin Noise settings:" << std::endl
					  << "  roughness: ";
			std::cin >> roughness;
			std::cout << "  detail level: ";
			std::cin >> detail;
			std::cout << "  peak: ";
			std::cin >> peak;
			std::cout << "  bottom: ";
			std::cin >> bottom;
			std::cout << "  water percentage: ";
			std::cin >> water;
		}
		else
		{
			roughness = atof(argv[6]);
			detail = atoi(argv[7]);
			bottom = atoi(argv[8]);
			peak = atoi(argv[9]);
			water = atof(argv[10]);
		}

		seed_arg_nr = 11;
	}


	// Let the user choose a seed or create one randomly.
	std::string seed_str;

	if(argc > seed_arg_nr)
	{
		seed_str = argv[seed_arg_nr];
	}
	else
	{
		std::cout << "Seed (type 'r' for random seed): ";
		std::cin >> seed_str; 
	}	

	if(seed_str == "r")
	{
#		ifdef WIN32
			seed = GetTickCount();
#		else
			// if anyone knows a better way of doing this without 
			// windows-specific functions, please tell me
			seed = time(0);
#		endif
	}
	else
	{
		seed = atoi(seed_str.c_str());
	}

    if (argc > seed_arg_nr + 1)
    {
        if (std::string(argv[seed_arg_nr + 1])=="--fullreport")
            LogManager::setFullReport (true);
    }
	
	if(generator == STATIC)
	{
		StaticTriangleGrid region(width,height,level,blur,detail,steepness,seed);
		region.writeImage("region.bmp");
	}

	if(generator == DYNAMIC)
	{
		DynamicTriangleGrid region(width,height,level,blur,detail,steepness,seed);
		region.writeImage("region.bmp");
	}

	if(generator == PERLIN)
	{
		Perlin region(width,height,level,blur,seed,detail,roughness,bottom,peak,water);
		region.writeImage("region.bmp");
	}

	if(generator == HERMITE)
	{
		SmoothTriangleGrid region(width,height,level,blur,detail,steepness,seed);
		region.writeImage("region.bmp");
	}

	if(generator == DEBUG)
	{
		debugtriangle::DynamicTriangleGrid region(width,height,level,blur,detail,steepness,seed);
		region.writeImage("region.bmp");
	}

	return 0;
}