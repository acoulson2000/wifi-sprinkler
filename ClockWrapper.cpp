/************************************************************************/
/*																		*/
/*	ClockWrapper.cpp --	Provides an implementation-independant wrapper	*/
/*		for the chipKIT PIC32 borads embedded Real Time Clock or one 	*/
/*		based on the DS1302. 											*/
/*																		*/
/*	This is my dumb way of implementing a generic API wrapper for two	*/
/*		different RTC libraries, since I don't knwo jack about c++		*/
/*		and how to implement configuratble polymorphic classes.			*/
/*																		*/
/*	To implement, define one or the other of RTCC_DS1302 or RTCC_PIC32	*/
/*		in your application's header file.								*/
/*	 ex:																*/
/*		#define   RTCC_PIC32
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

#include <WProgram.h>
#include "App.h"
#include "ClockWrapper.h"

#if defined(RTCC_DS1302)
 #include <chipKITDS1302.h>
#elif defined (RTCC_PIC32)
 #include <RTCC.h>
 #define TIMEZONE	"GMT"
#endif


// Set the DS1302 Real Time Clock pins
// Avoid lines used by WiFi shield or this server prog: 2,3,4,5,6,9,34,35,36 
#define SCLK        39
#define IO          40
#define CE          41


static  const 	uint8_t monthDays[] =	{31,28,31,30,31,30,31,31,30,31,30,31}; // API starts months from 1, this array starts from 0
char			intBuffer[] 		= 	"   ";

// leap year calulator expects year argument as years offset from 1970
#define LEAP_YEAR(Y)     (  !(Y%4) && ( (Y%100) || !(Y%400) ) )
#define SECS_PER_MIN  (60UL)
#define SECS_PER_HOUR (3600UL)
#define SECS_YR_2000  (946684800UL) // the time at the start of y2k
#define SECS_PER_DAY  (SECS_PER_HOUR * 24UL)

// Available clock formats
#define FORMAT_SHORT 1
#define FORMAT_LONG	2

//
// Forward refs
//
#if defined(RTCC_DS1302)
 DStime getDStime(WrappedTime);
#elif defined (RTCC_PIC32)
 
#endif

//
//  Implementations using chipKITDS1302 library
//
void ClockInit() {
	/* Initialize real time clock */
#if defined(RTCC_DS1302)
Serial.println("Init DS1302");
	DS1302Init(SCLK, IO, CE, "GMT");
#elif defined (RTCC_PIC32)
Serial.println("Init PIC32");
	RTCC.begin();
#endif
}

struct WrappedTime ClockGetTime() {
	WrappedTime wtime;
#if defined(RTCC_DS1302)
	DStime dsTime = DS1302GetTime();
	wtime.year = dsTime.year;
	wtime.month = dsTime.month;
	wtime.day = dsTime.day;
	wtime.week = dsTime.week;
	wtime.hour = dsTime.hour;
	wtime.minute = dsTime.minute;
	wtime.second = dsTime.second;
#elif defined (RTCC_PIC32)
	wtime.year = (uint8_t)RTCC.year();
	wtime.month = (uint8_t)RTCC.month();
	wtime.day = (uint8_t)RTCC.day();
	wtime.week = (uint8_t)RTCC.dayOfWeek();
	wtime.hour = (uint8_t)RTCC.hours();
	wtime.minute = (uint8_t)RTCC.minutes();
	wtime.second = (uint8_t)RTCC.seconds();
#endif
	return wtime;
}

void ClockSet(struct WrappedTime time) {
#if defined(RTCC_DS1302)
	DStime dsTime = getDStime(time);
	DS1302SetTime(dsTime);
#elif defined (RTCC_PIC32)
	// Set the time to something sensible
	RTCC.hours(time.hour);
	RTCC.minutes(time.minute);
	RTCC.seconds(time.second);
	RTCC.year(time.year);
	RTCC.month(time.month);
	RTCC.day(time.day);
	RTCC.dayOfWeek(time.week);
#endif
}

/* Accepts a string buffer containing time in the format hh:mm:ss	*/
void ClockSetTime(char * buffer) {
	WrappedTime t = ClockGetTime();
	strncpy(intBuffer, buffer, 2);
	t.hour = (int)strtol(intBuffer, NULL, 10);
	strncpy(intBuffer, buffer+3, 2);
	t.minute = (int)strtol(intBuffer, NULL, 10);
	strncpy(intBuffer, buffer+6, 2);
	t.second = (int)strtol(intBuffer, NULL, 10);
	ClockSet(t);
}

/* Accepts a string buffer containing date in the format MM/DD/YY	*/
void ClockSetDate(char * buffer) {
	WrappedTime t = ClockGetTime();
	strncpy(intBuffer, buffer, 2);
	t.month = (int)strtol(intBuffer, NULL, 10);
	strncpy(intBuffer, buffer+3, 2);
	t.day = (int)strtol(intBuffer, NULL, 10);
	strncpy(intBuffer, buffer+6, 2);
	t.year = (int)strtol(intBuffer, NULL, 10);
	ClockSet(t);
}

void ClockPrintTime() {
	WrappedTime wtime = ClockGetTime();
	char buf[20];
	ClockGetShortDateTimeString(buf, wtime);
	Serial.println(buf);
}

void ClockGetShortDateString(char * buffer, WrappedTime time) {
#if defined(RTCC_DS1302)
	DStime dsTime = getDStime(time);
	DS1302GetShortDateString(buffer, dsTime);
#elif defined (RTCC_PIC32)
	sprintf(buffer, "%02d/%02d/%02d", time.month, time.day, time.year);
#endif
}

void ClockGetShortTimeString(char * buffer, WrappedTime time) {
#if defined(RTCC_DS1302)
	DStime dsTime = getDStime(time);
	DS1302GetShortTimeString(buffer, dsTime);
#elif defined (RTCC_PIC32)
	sprintf(buffer, "%02d:%02d:%02d", time.hour, time.minute, time.second);
#endif
}

void ClockGetShortDateTimeString(char * buffer, WrappedTime time) {
#if defined(RTCC_DS1302)
	DStime dsTime = getDStime(time);
	DS1302GetShortDateTimeString(buffer, dsTime);
#elif defined (RTCC_PIC32)
	sprintf(buffer, "%02d/%02d/%02d %02d:%02d:%02d", time.month, time.day, time.year, time.hour, time.minute, time.second);
#endif
}

void ClockGetHttpTimeString(char * buffer, WrappedTime time) {
#if defined(RTCC_DS1302)
	DStime dsTime = getDStime(time);
	DS1302GetHttpTimeString(buffer, dsTime);
#elif defined (RTCC_PIC32)
	char monBuff[10];
	char dowBuff[10];
	ClockGetDOWStr(dowBuff, time, FORMAT_SHORT);
	ClockGetMonthStr(monBuff, time, FORMAT_SHORT);
	sprintf(buffer, "%s, %d %s %d %02d:%02d:%02d %s", 
			dowBuff,
			time.day,
			monBuff,
			(2000 + time.year),
			time.hour, time.minute, time.second,
			TIMEZONE);	
#endif
}

void ClockGetDOWStr(char * buf, WrappedTime time, uint8_t format) {
#if defined(RTCC_DS1302)
	DStime dsTime = getDStime(time);
	DS1302GetDOWStr(buf, dsTime, format);
#elif defined (RTCC_PIC32)
	switch (time.week) {
		case MONDAY:
			sprintf(buf, "Monday");
			break;
		case TUESDAY:
			sprintf(buf, "Tuesday");
			break;
		case WEDNESDAY:
			sprintf(buf, "Wednesday");
			break;
		case THURSDAY:
			sprintf(buf, "Thursday");
			break;
		case FRIDAY:
			sprintf(buf, "Friday");
			break;
		case SATURDAY:
			sprintf(buf, "Saturday");
			break;
		case SUNDAY:
			sprintf(buf, "Sunday");
			break;
	}   
	if (format==FORMAT_SHORT)
		buf[3] = '\0';		
#endif
}

void ClockGetMonthStr(char * buf, WrappedTime time, uint8_t format) {
#if defined(RTCC_DS1302)
	DStime dsTime = getDStime(time);
	DS1302GetMonthStr(buf, dsTime, format);
#elif defined (RTCC_PIC32)
	switch (time.month) {
		case 1:
			sprintf(buf, "January");
			break;
		case 2:
			sprintf(buf, "February");
			break;
		case 3:
			sprintf(buf, "March");
			break;
		case 4:
			sprintf(buf, "April");
			break;
		case 5:
			sprintf(buf, "May");
			break;
		case 6:
			sprintf(buf, "June");
			break;
		case 7:
			sprintf(buf, "July");
			break;
		case 8:
			sprintf(buf, "August");
			break;
		case 9:
			sprintf(buf, "September");
			break;
		case 10:
			sprintf(buf, "October");
			break;
		case 11:
			sprintf(buf, "November");
			break;
		case 12:
			sprintf(buf, "December");
			break;
	}
	if (format==FORMAT_SHORT)
		buf[3] = '\0';
#endif
}

#if defined(RTCC_DS1302)
  DStime getDStime(WrappedTime time) {
	DStime dsTime;
	dsTime.year = time.year;
	dsTime.month = time.month;
	dsTime.day = time.day;
	dsTime.week = time.week;
	dsTime.hour = time.hour;
	dsTime.minute = time.minute;
	dsTime.second = time.second;
	return dsTime;
  }
#elif defined (RTCC_PIC32)
#endif

long now() {
	WrappedTime tm = ClockGetTime();
	int i;
	long seconds = SECS_YR_2000;

	// seconds from 2000 till 1 jan 00:00:00 of the given year
	seconds += (tm.year)*(SECS_PER_DAY * 365);
	for (i = 0; i < (tm.year); i++) {
		if (LEAP_YEAR(i)) {
			seconds +=  SECS_PER_DAY;   // add extra days for leap years
		}
	}

	// add days for this year, months start from 1
	for (i = 1; i < tm.month; i++) {
		if ( (i == 2) && LEAP_YEAR((tm.year))) { 
			seconds += SECS_PER_DAY * 29;
		} else {
			seconds += SECS_PER_DAY * monthDays[i-1];  //monthDay array starts from 0
		}
	}
	seconds += (tm.day-1) * SECS_PER_DAY;
	seconds += tm.hour * SECS_PER_HOUR;
	seconds += tm.minute * SECS_PER_MIN;
	seconds += tm.second;
	return seconds; 
}


