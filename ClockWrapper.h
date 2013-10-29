/************************************************************************/
/*																		*/
/*	ClockWrapper.h --	header file for ClockWrapper.c					*/
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

#ifndef _ClockWrapper_H_INCLUDED
#define _ClockWrapper_H_INCLUDED

#include <WProgram.h>

#define MONDAY		1
#define TUESDAY		2
#define WEDNESDAY	3
#define THURSDAY	4
#define FRIDAY		5
#define SATURDAY	6
#define SUNDAY		7

struct WrappedTime  {
	uint8_t year;
	uint8_t month;
	uint8_t day;
	uint8_t week;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
};

struct	WrappedTime 	ClockGetTime();
void 	ClockInit();
void 	ClockSet(struct WrappedTime);
void	ClockSetTime(char * buffer);
void	ClockSetDate(char * buffer);
void 	ClockPrintTime();
void 	ClockGetShortDateString(char *, WrappedTime);
void 	ClockGetShortTimeString(char * buffer, WrappedTime time);
void 	ClockGetShortDateTimeString(char * buffer, WrappedTime time);
void 	ClockGetHttpTimeString(char * buffer, WrappedTime time);
void 	ClockGetDOWStr(char * buffer, WrappedTime time, uint8_t format);
void 	ClockGetMonthStr(char * buffer, WrappedTime time, uint8_t format);
int 	DayOfWeek(int d, int m, int y);
long	now();
long	timeAsEpoch(WrappedTime tm);

#endif
