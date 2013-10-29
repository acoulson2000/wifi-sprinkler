
/************************************************************************/
/*																		*/
/*	App.cpp	--	Embedded Application Main Module						*/
/*																		*/
/************************************************************************/
/*	Author: 	Gene Apperson											*/
/*	Copyright 2012, Digilent Inc.										*/
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
/*																		*/
/*																		*/
/************************************************************************/
/*  Revision History:													*/
/*																		*/
/*	06/21/2012(GeneA): created											*/
/*  09/01/2013(Andy Coulson): Modified for WiH2O							*/
/*																		*/
/************************************************************************/


/* ------------------------------------------------------------ */
/*				Include File Definitions						*/
/* ------------------------------------------------------------ */
#include	<WProgram.h>
#include	<SD.h>
#include	<string.h>
#include	"BoardDefs.h"
#include	"Parser.h"
#include	"Render.h"
#include	"HtmlSource.h"
#include	"App.h"

#if defined(RTCC_DS1302)
 #include <chipKITDS1302.h>
#elif defined(RTCC_PIC32)
 #include <RTCC.h>
#endif

#include	"ClockWrapper.h"

/* ------------------------------------------------------------ */
/*				Local Type and Constant Definitions				*/
/* ------------------------------------------------------------ */

const uint32_t	msHeartbeat = 1000;
const uint32_t	msHeartbeatDuration = 200;

// Use PIN_LED2 (pin 12 on uC32 and WF32) for heartbeat
const int    pinLedStatus = PIN_LED2;

#define	pinSdCs	PIN_SDCS


/* ------------------------------------------------------------ */
/*				Global Variables								*/
/* ------------------------------------------------------------ */

bool	fSDfs = false;	// true if we have an SD file system
bool	forcedOn[20] = {false, false, false, false, false, false, false, false, false, false,
						false, false, false, false, false, false, false, false, false, false};	// make sure you have at least as many as MAX_ZONES specifies

bool	forcedOff[20] = {false, false, false, false, false, false, false, false, false, false,
						false, false, false, false, false, false, false, false, false, false};	// make sure you have at least as many as MAX_ZONES specifies

/* ------------------------------------------------------------ */
/*				Local Variables									*/
/* ------------------------------------------------------------ */
// Can only handle 5 on Arduino Uno due to memory limits.
// Maybe 10 or 20 on Mega/chipKIT?
ZONE_INFO events[MAX_EVENTS] = {
  {"Event 1                         ",0,'N',0,0,0,0,0,"NNNNNNN"}, 
  {"Event 2                         ",0,'N',0,0,0,0,0,"NNNNNNN"}, 
  {"Event 3                         ",0,'N',0,0,0,0,0,"NNNNNNN"}, 
  {"Event 4                         ",0,'N',0,0,0,0,0,"NNNNNNN"}, 
  {"Event 5                         ",0,'N',0,0,0,0,0,"NNNNNNN"}, 
  {"Event 6                         ",0,'N',0,0,0,0,0,"NNNNNNN"}, 
  {"Event 7                         ",0,'N',0,0,0,0,0,"NNNNNNN"}, 
  {"Event 8                         ",0,'N',0,0,0,0,0,"NNNNNNN"}, 
  {"Event 9                         ",0,'N',0,0,0,0,0,"NNNNNNN"}, 
  {"Event 10                        ",0,'N',0,0,0,0,0,"NNNNNNN"}, 
  {"Event 11                        ",0,'N',0,0,0,0,0,"NNNNNNN"}, 
  {"Event 12                        ",0,'N',0,0,0,0,0,"NNNNNNN"},
  {"Event 13                        ",0,'N',0,0,0,0,0,"NNNNNNN"}, 
  {"Event 14                        ",0,'N',0,0,0,0,0,"NNNNNNN"}, 
  {"Event 15                        ",0,'N',0,0,0,0,0,"NNNNNNN"}/*, 
  {"Event 16                        ",0,'N',0,0,0,0,0,"NNNNNNN"}, 
  {"Event 17                        ",0,'N',0,0,0,0,0,"NNNNNNN"}, 
  {"Event 18                        ",0,'N',0,0,0,0,0,"NNNNNNN"}, 
  {"Event 19                        ",0,'N',0,0,0,0,0,"NNNNNNN"}, 
  {"Event 20                        ",0,'N',0,0,0,0,0,"NNNNNNN"}*/
};

// Format Zone data from SD card SCHED.TXT file will be:
// ex: "01My Zone 1                       Y193020NNYNNNY"
// Where	[32 character zone name]
//			ZZ = Zone ID (01-MAX_ZONES)
//			A = Active (Y/N)
//			HH = Hour (00-23)
//			MM = Minute (00-23)
//			RR = Runtime (00-99)
//			SMTWTFS = Day Flags for Sun, Mon, Tues, etc
const int ZONE_DATA_LENGTH = 48; // strlen("NAMEx32xxxxxxxxxxxxxxxxxxxxxxxxxZZAHHMMRRSMTWTFS");

uint32_t    msStatCur;
bool        stLedCur;
int         stLedHearbeat;
char 		buffer[50];
char 		zoneBuffer[ZONE_DATA_LENGTH+1];
char 		lastChar = ' ';
char		ch;
char *		pgmStatus;
char 		intHolder[] = "   "; 
unsigned long nowMillis;
unsigned long lasttime;
char 		lasttimestr[10];
char 		nowString[10];

char		szPgmStsRunning[] = { "Running" };
char		szPgmStsPaused[] = { "Paused" };
WrappedTime	t;

File 	schedFile;
extern char	szLineBuffer[64];

/* ------------------------------------------------------------ */
/*				Forward Declarations							*/
/* ------------------------------------------------------------ */
void parseZoneData(char* data, int zone);
void ReadSerialBytes(char * buf, int bytes);
void SerialCmdTask();
void setupSprinklerPins();
void checkForZoneAction(WrappedTime realtime);
void loadTime();
int sdReadln(File tFile, char *buff, int buffsz);

/* ------------------------------------------------------------ */
/*				Procedure Definitions							*/
/* ------------------------------------------------------------ */

void loadConfig() {
	char	ch;
	int		cchLineCur;
	char *	pchLineCur;
	int		lineIndex = 0;
	
	#if defined (RTCC_PIC32)
    loadTime();
	#endif
	
	cchLineCur = 0;
	pchLineCur = &szLineBuffer[0];
	schedFile = SD.open("sched.txt");
	if (schedFile) {
		while (schedFile.available()) {
			schedFile.read((byte *)&ch, 1);
			if (ch != 0x0D) {
				if (cchLineCur < 64) {
					*pchLineCur = ch;
					pchLineCur += 1;
					cchLineCur += 1;
				}
			}
			else {
				*pchLineCur = '\0';
				if (schedFile.available() > 0) {
					schedFile.read((byte *)&ch, 1);	// read rest of EOL (0x0A)
				}
				parseZoneData(&szLineBuffer[0], lineIndex);//NAME_1_xxxxxxxxxxxxxxxxxxxxxxxxx01Y0910200001001
				lineIndex += 1;					// Increment zone counter
				cchLineCur = 0;					// Reset buffer pointers
				pchLineCur = &szLineBuffer[0];
			}
		}
		schedFile.close();
	}
	
//	dumpZoneData();
}

void parseZoneData(char* data, int zoneNum) {
	strncpy(events[zoneNum].name, data, 32);
	// zone number
	strncpy(intHolder, data + 33, 2); intHolder[2] = 0;
	events[zoneNum].zone = (int)strtol(intHolder, NULL, 10);
	// active flag
	events[zoneNum].active = data[34];
	// start hour
	strncpy(intHolder, data + 35, 2); intHolder[2] = 0;
	events[zoneNum].hour = (int)strtol(intHolder, NULL, 10);
	// start minute
	strncpy(intHolder, data + 37, 2); intHolder[2] = 0;
	events[zoneNum].minute = (int)strtol(intHolder, NULL, 10);
	// run time
	strncpy(intHolder, data + 39, 2); intHolder[2] = 0;
	events[zoneNum].runTime = (int)strtol(intHolder, NULL, 10);
	events[zoneNum].runOnDay[0] = data[41];
	events[zoneNum].runOnDay[1] = data[42];
	events[zoneNum].runOnDay[2] = data[43];
	events[zoneNum].runOnDay[3] = data[44];
	events[zoneNum].runOnDay[4] = data[45];
	events[zoneNum].runOnDay[5] = data[46];
	events[zoneNum].runOnDay[6] = data[47];
}

void saveConfig() {
	///char name[32];
	if (SD.exists("sched.txt")) {
	     SD.remove("sched.txt");
	}
	schedFile = SD.open("sched.txt", FILE_WRITE);
	// if the file opened okay, write to it:
	if (schedFile) {
		for (int i = 0; i < MAX_EVENTS; i++) {
			//strcpy(name, "                                ");
			strncpy(buffer, events[i].name, 32);
			buffer[32] = 0;
//Serial.print(events[i].name);Serial.print("*");Serial.println();
//Serial.print(buffer);Serial.print("*");Serial.println();
//Serial.print("zoneBuffer before: ");Serial.println(zoneBuffer);
			sprintf(zoneBuffer, "%-32s%02d%c%02d%02d%02d%c%c%c%c%c%c%c", buffer, events[i].zone, events[i].active, events[i].hour, 
				events[i].minute, events[i].runTime, events[i].runOnDay[0], events[i].runOnDay[1], events[i].runOnDay[2], 
				events[i].runOnDay[3], events[i].runOnDay[4], events[i].runOnDay[5], events[i].runOnDay[6]);
//Serial.print("zoneBuffer after: ");Serial.println(zoneBuffer);
			schedFile.println(zoneBuffer);
		}
		schedFile.close();
	}
}

void dumpZoneData() {
	Serial.println(MAX_EVENTS, DEC);
	for (int i = 0; i < MAX_EVENTS; i++) { 
		Serial.print(events[i].name);
		Serial.print(events[i].zone, DEC);
		Serial.print(events[i].active);
		Serial.print(events[i].hour, DEC);
		Serial.print(events[i].minute, DEC);
		Serial.print(events[i].runTime, DEC);
		Serial.print(events[i].runOnDay[0]);
		Serial.print(events[i].runOnDay[1]);
		Serial.print(events[i].runOnDay[2]);
		Serial.print(events[i].runOnDay[3]);
		Serial.print(events[i].runOnDay[4]);
		Serial.print(events[i].runOnDay[5]);
		Serial.print(events[i].runOnDay[6]);
		Serial.println();
	}
}

/***    AppInit
**
**  Parameters:
**      none
**
**  Return Value:
**      none
**
**  Errors:
**      none
**
**  Description:
**      Perform any one time initialization of the application
**      on sketch startup.
*/

void AppInit() {
	int		i;
	intHolder[2] = 0;
	pgmStatus = szPgmStsRunning;

	// Print date and time
	ClockGetShortDateTimeString(buffer, ClockGetTime());
	Serial.println(buffer);
	
	/* Set up the pin used for heartbeat status output.
	*/
	pinMode(pinLedStatus, OUTPUT);		//heartbeat LED
	digitalWrite(pinLedStatus, LOW);

	/* Set up the pins for control of sprinkler relays */
	setupSprinklerPins();

	/* Set the pin used to control the SS line on the SD card to
	** output.
	*/
	digitalWrite(pinSdCs, HIGH); 
	pinMode(pinSdCs, OUTPUT);

	/* Initialize the system tick counter used to control blinking
	** the heartbeat LED that indicdates that the sketch is alive.
	*/
	msStatCur = millis();
	stLedCur = false;

	/* See if there is an SD card connected */
	if (SD.begin(pinSdCs)) {
		/* Card successfully initialized, so we have a file system.
		*/
		fSDfs = true;
		Serial.print("SD card initialized, CS pin ");Serial.println(pinSdCs);

		Serial.println("Load config from SD");		
		loadConfig();
	}
	else {
		fSDfs = false;
		Serial.println("No SD card file system found.");
	}
}

void setupSprinklerPins() {
	// configure sprinkler control pins:
	for (int i = FIRST_VALVE_PIN; i < FIRST_VALVE_PIN+MAX_ZONES; i++) {
		pinMode(i, OUTPUT);
		delay(50);
		if (i == MASTER_VALVE_PIN) {
			if (INVERT_MASTER_PIN == 'Y')
				digitalWrite(i, HIGH);
			else
				digitalWrite(i, LOW);
		} else {
			if (INVERT_PINS == 'Y')
				digitalWrite(i, HIGH);
			else
				digitalWrite(i, LOW);
		}
	}
}
	
/* ------------------------------------------------------------ */
/***    AppTask
**
**  Parameters:
**      none
**
**  Return Value:
**      none
**
**  Errors:
**      none
**
**  Description:
**		This task handles any foreground processing required by the
**		application. Currently the only thing it does is blink the
**		heartbeat LED.
**		In a real application, this function would manage the behavior
**		of whatever embedded control/monitering was required for the
**		embedded application.
**		The presumption is that the HTTP server part is for remote
**		monitoring/control of the application.
*/

void AppTask() {
	/* If we're using the embedded PIC32 clock, which does not have
		a backup battery, save clock time every 15 minutes, so we can
		restore it when we start up (in case of power failure. This isn't
		perfect, but will at least ensure that a cycle continues, 
		if interrupted. */
	#if defined (RTCC_PIC32)
    if (millis() > (msStatCur + (15*60*1000))) {
		saveTime();
	}
	#endif
	
	/* Check if it is time to toggle the hearbeat LED.    */
    if (millis() > (msStatCur + msHeartbeat)) {
        if (!stLedCur) {
            digitalWrite(pinLedStatus, HIGH);
            stLedCur = true;
       }
    }
    if (millis() > (msStatCur + msHeartbeat + msHeartbeatDuration)) {
        if (stLedCur) {
			digitalWrite(pinLedStatus, LOW);
            stLedCur = false;
            msStatCur += msHeartbeat;
        }
    }
	
	// has at least .5 second passed?
	nowMillis = millis();
	if ((nowMillis - lasttime) > 500) {
		WrappedTime realtime = ClockGetTime();

		// Get current time as a string
		ClockGetShortTimeString(nowString, realtime);
		
		// Check whether a zone should turn on.
		// To limit interference with manual actions, zones only turn on or off
		// in first couple seconds of each minute.
		if ( (realtime.second >= 0 && realtime.second <= 1)
			&& strcmp(pgmStatus, szPgmStsRunning) == 0 
			&& strcmp(lasttimestr, nowString) != 0) {
			//ClockPrintTime();
			checkForZoneAction(realtime);
		}
		strcpy(lasttimestr, nowString);
		lasttime = nowMillis;
	}

	if (Serial.available()) {
		SerialCmdTask();
	}
}

void SerialCmdTask() {
	lastChar = ch;
	ch = Serial.read();
	if (lastChar == 'W') {
		switch (ch) { 
			case 'T':
				buffer[0] = 0;
				ReadSerialBytes(&buffer[0], 8);
				ClockSetTime(buffer);
				break;
			case 'D':
				buffer[0] = 0;
				ReadSerialBytes(&buffer[0], 8);
				ClockSetDate(buffer);
				break;
			case 'z':
				break;
		}
	} else if (lastChar == 'R') {
		switch (ch) { 
			case 'T':
				buffer[0] = 0;
				t = ClockGetTime();
				ClockGetShortTimeString(buffer, t);
				Serial.println(buffer);
				break;
			case 'D':
				buffer[0] = 0;
				t = ClockGetTime();
				ClockGetShortDateString(buffer, t);
				Serial.println(buffer);
				break;
		}
	}
}

void ReadSerialBytes(char * buf, int bytes) {
	int i = 0;
	delay(50);
	while (i < bytes & Serial.available() > 0) {
		delay(10);
		buf[i] = Serial.read();
		Serial.print(buf[i]);
		i++;
	}
}

void checkForZoneAction(WrappedTime realtime) {
	for (int i = 0; i < MAX_EVENTS; i++) {
		// Check for zones to turn on

		if (events[i].active == 'Y') {
			// OK, is it active and does it run on this day?
			if (events[i].currentState != 1
				&& 
				( (events[i].runOnDay[0] == 'Y' && realtime.week == SUNDAY)
				|| (events[i].runOnDay[1] == 'Y' && realtime.week == MONDAY)
				|| (events[i].runOnDay[2] == 'Y' && realtime.week == TUESDAY)
				|| (events[i].runOnDay[3] == 'Y' && realtime.week == WEDNESDAY)
				|| (events[i].runOnDay[4] == 'Y' && realtime.week == THURSDAY)
				|| (events[i].runOnDay[5] == 'Y' && realtime.week == FRIDAY)
				|| (events[i].runOnDay[6] == 'Y' && realtime.week == SATURDAY) )
			) { 
				// OK, is it time?
				if ( events[i].hour == realtime.hour && events[i].minute == realtime.minute ) {
						turnOn(events[i].zone);
						events[i].currentState = 1;
						events[i].lastStart = now();
				} //end check for on
			}
//Serial.print(">>>currentState=");Serial.print(currentState,DEC);Serial.print(" now()=");Serial.print(now(),DEC);Serial.print(" test=");Serial.println((events[i].lastStart + (events[i].runTime * 60) ),DEC);
		}
		// Any to turn off?
		if (events[i].currentState == 1) {
			// On long enough?
			if ( now() > (events[i].lastStart + (events[i].runTime * 60) ) ){
				// TURN IT OFF!
				turnOff(events[i].zone);
				events[i].currentState = 0;
			}
		}
		
		// NO zone should be on over 99 minutes
		if (events[i].currentState == 1 || forcedOn[events[i].zone] ) {
			// On long enough?
			if (now() > (events[i].lastStart + (99 * 60))) {
				// TURN IT OFF!
				turnOff(events[i].zone);
				events[i].currentState = 0;
				forcedOn[events[i].zone] = false;
			}
		}
	}
}

void turnOff(int zone) {
	Serial.print(zone, DEC);Serial.print("** OFF :");
#if defined(USE_MASTER_VALVE)
	if (INVERT_MASTER_PIN == 'Y') {
		digitalWrite(MASTER_VALVE_PIN, HIGH);
		Serial.print("pin ");Serial.print(MASTER_VALVE_PIN);Serial.print(" HIGH, ");				
	} else {
		digitalWrite(MASTER_VALVE_PIN, LOW);
		Serial.print("pin ");Serial.print(MASTER_VALVE_PIN);Serial.print(" LOW, ");				
	}
#endif
				 
	if (INVERT_PINS == 'Y') {
		digitalWrite((zone + FIRST_VALVE_PIN - 1), HIGH);
		Serial.print("pin ");Serial.print(zone, DEC);Serial.println(" HIGH");				
	} else {
		digitalWrite((zone + FIRST_VALVE_PIN - 1), LOW);
		Serial.print("pin ");Serial.print(zone, DEC);Serial.println(" LOW");				
	}
}

void turnOn(int zone) {
	Serial.print(zone, DEC);Serial.print("** ON :");
#if defined(USE_MASTER_VALVE)
	if (INVERT_MASTER_PIN == 'Y') {
		digitalWrite(MASTER_VALVE_PIN, LOW);
		Serial.print("pin ");Serial.print(MASTER_VALVE_PIN);Serial.print(" LOW, ");				
	} else {
		digitalWrite(MASTER_VALVE_PIN, HIGH);
		Serial.print("pin ");Serial.print(MASTER_VALVE_PIN);Serial.print(" HIGH, ");				
	}
#endif

	if (INVERT_PINS == 'Y') {
		digitalWrite((zone + FIRST_VALVE_PIN - 1), LOW);
		Serial.print("pin ");Serial.print(zone, DEC);Serial.println(" LOW");				
	} else {
		digitalWrite((zone + FIRST_VALVE_PIN - 1), HIGH);
		Serial.print("pin ");Serial.print(zone, DEC);Serial.println(" HIGH");				
	}
}

void manualOff(int zone) {
	forcedOn[zone] = false;
	turnOff(zone);
}

void manualOn(int zone) {
	forcedOn[zone] = true;
	turnOn(zone);
}

void saveTime() {
	if (SD.exists("time.txt")) {
	     SD.remove("time.txt");
	}
	File tFile = SD.open("time.txt", FILE_WRITE);
	if (tFile) {
		WrappedTime realtime = ClockGetTime();
		ClockGetShortDateString(szLineBuffer, realtime);
		tFile.println(szLineBuffer);
		ClockGetShortTimeString(szLineBuffer, realtime);
		tFile.println(szLineBuffer);
		tFile.close();
	}
}

void loadTime() {
	if (SD.exists("time.txt")) {
		File tFile = SD.open("time.txt");
		if (tFile) {
			if (schedFile.available() > 0) {
				sdReadln(tFile, szLineBuffer, strlen(szLineBuffer));
				ClockSetDate(szLineBuffer);
			}
			if (schedFile.available() > 0) {
				sdReadln(tFile, szLineBuffer, strlen(szLineBuffer));
				ClockSetTime(szLineBuffer);
			}
			tFile.close();
		}
	}
}

int sdReadln(File tFile, char *buff, int buffsz) {
	char	*bp = buff;
	char	c;

	while(bp - buff < buffsz && (c = tFile.read()) > 0) {
		if (c == '\n')
			return (bp - buff);
		else *bp++ = c;
	}

	if ((int)c < 0)
		return -1;
	
	*bp++ = 0;
	
	return (bp - buff);
}



