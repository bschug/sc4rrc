/******************************************************************************
 *  file:  LogManager.h
 *
 *	Copyright (c) 2006, Benjamin Schug
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

#ifndef OC__LOGMANAGER_H
#define OC__LOGMANAGER_H

#include <string>
#include <sstream>
#include <fstream>

// forward declaration
struct SDL_mutex;

/**	This singleton manages all access to the log file. 
 *
 *	@attention	This class needs the SDL library for its thread access 
 *				functionality. However, if you don't use multiple threads (or
 *				at least you don't access the LogManager from multiple threads)
 *				you can define the necessary functions and data structures
 *				yourself as some dummy functions. Then you don't need to link
 *				against the SDL library. \n
 *				You must define the struct SDL_mutex and the functions: \n
 *				SDL_mutex* SDL_CreateMutex() \n
 *				void SDL_mutexP(SDL_mutex*) \n
 *				void SDL_mutexV(SDL_mutex*) \n
 *				int SDL_GetTicks()
 */
class LogManager 
{
private:
	/** Points to the only allowed instance of the class. */
	static LogManager* singleton;

	/** If this is true, all entries will be logged, even if _DEBUG is not
	 *	defined. This flag is only relevant for release builds.
	 */
	static bool fullreport;

	/** Mutex for multi-threaded applications. */
	SDL_mutex* mutex;

	std::string filename;
	std::ofstream file;

	/** For internal use only.
	 *	You can't create a LogManager object by yourself.
	 *	The LogManager::log() method will do that if necessary.
	 */
	LogManager();

	~LogManager();

	void _log(const std::string &descr);
	void _endl();

public:
	/**	Writes a string to the log file.
	 *	@param	descr	The text that should be written to the log file.
	 *	@param	always	If this is false, the entry will only be written in
	 *					debug builds (_DEBUG must be defined). Set it to true 
	 *					for log entries that should also appear in your release
	 *					build.
	 */
	static inline void log(const std::string &descr, bool always=false)
	{
#ifndef _DEBUG
		if(always || fullreport)
#endif
		{
			if(singleton==NULL) singleton = new LogManager();
			singleton->_log(descr);
		}
	}

	/**	Writes the content of a stream to the log file.
	 *	This works basically just like LogManager::log(const std::string&,bool)
	 *	but it also resets the stream so you can reuse it for creating new log
	 *	entries.
	 *	@param descr	This must be a pointer to a ostringstream that was 
	 *					created on the heap because it will be destroyed and
	 *					recreated after logging. This seems to be the only way
	 *					to clean the content of the stream.
	 *	@see LogManager::log(const std::string&, bool)
	 */
	static inline void log( std::ostringstream* &descr, bool always=false )
	{
		log(descr->str(),always);
		delete descr;
		descr = new std::ostringstream;
	}

	/**	Writes an empty line to the log file.
	 *
	 *	@note	Every entry in the log file starts in a new line so you don't
	 *			have to call this after every entry you make. This is just
	 *			useful if you want to add an empty line without a timestamp
	 *			to bring some structure and readability to the log file.
	 */
	static inline void endl()
	{
		if(singleton==NULL) singleton = new LogManager();
		singleton->_endl();
	}

	/**	Forces output of all log entries.
	 *	If you set fullreport to true, all messages will be written to the log
	 *	file, just as if _DEBUG was defined.
	 */
	static void setFullReport(bool b) { fullreport = b; }
};

#endif
