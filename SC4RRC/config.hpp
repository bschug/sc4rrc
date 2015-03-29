/******************************************************************************
 *  file:  config.hpp
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
#ifndef SC4RRC_CONFIG_HPP
#define SC4RRC_CONFIG_HPP

#ifdef _WIN32
 #ifndef SC4RRC_LIB
  #define SC4RRC_API __declspec(dllimport)
 #else
  #define SC4RRC_API __declspec(dllexport)
 #endif
#else
 #define SC4RRC_API
#endif

#pragma warning(disable:4251)

#endif