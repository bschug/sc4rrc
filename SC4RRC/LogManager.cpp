/******************************************************************************
 *  file:  LogManager.cpp
 *
 *	Copyright (c) 2006/2007, Benjamin Schug
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

#include <assert.h>
#include <iostream>

#include <SDL/SDL_thread.h>
#include <SDL/SDL_timer.h>

#include "LogManager.h"


LogManager* LogManager::singleton = NULL;
bool LogManager::fullreport = false;

//-----------------------------------------------------------------------------

LogManager::LogManager()
{
	assert( singleton==NULL );

	filename = "sc4rrc.log";
	file.open(filename.c_str());

	file << "Sim City 4 Random Region Creator Log File\n";
	std::cerr << "SC4RRC Log System Enabled\n";


#ifndef _DEBUG
	if(fullreport)
#endif
	{
		file << "Detail level: Full\n";
		std::cerr << "Detail level: Full\n";
	}

	file << "\n";

	mutex = SDL_CreateMutex();
}

//-----------------------------------------------------------------------------

LogManager::~LogManager()
{
	_endl();
	_log("Unloading Log Manager.");

	assert( singleton );
	singleton = NULL;

	file.close();

	SDL_DestroyMutex(mutex);
}

//-----------------------------------------------------------------------------

void LogManager::_log(const std::string &descr)
{
	std::cerr << descr << "\n";

	SDL_mutexP(mutex);
	file << SDL_GetTicks() << "\t" << descr << "\n";
	SDL_mutexV(mutex);
}

//-----------------------------------------------------------------------------

void LogManager::_endl()
{
	std::cerr << "\n";

	SDL_mutexP(mutex);
	file << "\n";
	SDL_mutexV(mutex);
}

