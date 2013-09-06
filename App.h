/************************************************************************/
/*																		*/
/*	App.cpp	--	Embedded Application Main Module Header file			*/
/*																		*/
/************************************************************************/
/*	Author: 	Andy Coulson											*/
/*	Copyright 2013, Practical Apps, LLC									*/
/************************************************************************/
/*
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License (GNU LGPL) as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  To obtain a copy of the GNU LGPL, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
/************************************************************************/
/*  Module Description: 												*/
/*	Defines some variables, structure and prototypes required by		*/
/*	the WiH2O modified version of App.h and related modules.			*/
/*																		*/
/************************************************************************/
#ifndef _APP_H
#define	_APP_H

#define MAX_EVENTS			15
#define FIRST_VALVE_PIN		26
#define MASTER_VALVE_PIN	32

#define INVERT_PINS 		'Y' 	// If you want Arduino pin to be HIGH for Off, set to 'Y', normally set to 'N'
									// (my triac board uses reverse levels)
#define INVERT_MASTER_PIN	'Y'

// Determine which RealTime Clock you want to use by uncommenting one of the following.
// While there is a RTCC built into the PIC chip of the chipKIT Uno32, uC32 and Max32 boards, it requires
// that you solder a crystal onto the board and the time does not survive power failures. I happened to 
// obtain a very inexpensive (<$4) DS1302-based board so I cobbled together a library for reading and
// writing from it. This library, which I have included in the source repository for this project,
// will be included automatically if RTCC_DS1302 is defined here. If RTCC_PIC32 is defined,
// the library user-contributed library from here http://sourceforge.net/projects/chipkitrtcc/
// must be available in the libraries directory.
#define   RTCC_DS1302
//#define   RTCC_PIC32

extern char	szPgmStsRunning[];
extern char	szPgmStsPaused[];

extern char *		pgmStatus;

typedef struct {
  char		name[33];
  uint8_t	zone; 
  char		active; 
  uint8_t	currentState; 
  long		lastStart; 			// as seconds since Jan 1 1970
  uint8_t	hour; 
  uint8_t	minute;
  uint8_t	runTime;
  char		runOnDay[8];
} ZONE_INFO;

/**
 * Prototypes
 */
void loadConfig();
void saveConfig();
void saveTime();
void dumpZoneData();

#endif	// _APP_H
