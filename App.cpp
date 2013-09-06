
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

/* The Uno32 must use either a WiFiShield or PmodShield
** to support any network hardware. Since the SPI2 SCK
** conficts with LED1 on the Uno32, use the LED on
** the WiFiShield or PmodShield instead.  
*/
const int    pinLedStatus = 6;

#define	pinSdCs	PIN_SDCS


/* ------------------------------------------------------------ */
/*				Global Variables								*/
/* ------------------------------------------------------------ */

bool	fSDfs = false;	// true if we have an SD file system

/* ------------------------------------------------------------ */
/*				Local Variables									*/
/* ------------------------------------------------------------ */
// Can only handle 5 on Arduino Uno
// Maybe 10 or 20 on Mega/chipKIT?
ZONE_INFO zones[MAX_EVENTS] = {
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
//			ZZ = Zone ID (01-MAX_EVENTS)
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
void turnOn(int pin);
void turnOff(int pin);
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
	
	dumpZoneData();
}

void parseZoneData(char* data, int zoneNum) {
	strncpy(zones[zoneNum].name, data, 32);
	zones[zoneNum].name[32] = 0;
	// zone number
	strncpy(intHolder, data + 33, 2); intHolder[2] = 0;
	zones[zoneNum].zone = (int)strtol(intHolder, NULL, 10);
	// active flag
	zones[zoneNum].active = data[34];
	// start hour
	strncpy(intHolder, data + 35, 2); intHolder[2] = 0;
	zones[zoneNum].hour = (int)strtol(intHolder, NULL, 10);
	// start minute
	strncpy(intHolder, data + 37, 2); intHolder[2] = 0;
	zones[zoneNum].minute = (int)strtol(intHolder, NULL, 10);
	// run time
	strncpy(intHolder, data + 39, 2); intHolder[2] = 0;
	zones[zoneNum].runTime = (int)strtol(intHolder, NULL, 10);
	zones[zoneNum].runOnDay[0] = data[41];
	zones[zoneNum].runOnDay[1] = data[42];
	zones[zoneNum].runOnDay[2] = data[43];
	zones[zoneNum].runOnDay[3] = data[44];
	zones[zoneNum].runOnDay[4] = data[45];
	zones[zoneNum].runOnDay[5] = data[46];
	zones[zoneNum].runOnDay[6] = data[47];
}

void saveConfig() {
        char name[33];
	if (SD.exists("sched.txt")) {
	     SD.remove("sched.txt");
	}
	schedFile = SD.open("sched.txt", FILE_WRITE);
	// if the file opened okay, write to it:
	if (schedFile) {
		for (int i = 0; i < MAX_EVENTS; i++) {
			strcpy(name, "                                ");
			strncpy(name, zones[i].name, 32);
			sprintf(zoneBuffer, "%32s%02d%c%02d%02d%02d%c%c%c%c%c%c%c", name, zones[i].zone, zones[i].active, zones[i].hour, 
				zones[i].minute, zones[i].runTime, zones[i].runOnDay[0], zones[i].runOnDay[1], zones[i].runOnDay[2], 
				zones[i].runOnDay[3], zones[i].runOnDay[4], zones[i].runOnDay[5], zones[i].runOnDay[6]);
			zoneBuffer[ZONE_DATA_LENGTH] = 0;
			schedFile.println(zoneBuffer);
		}
		schedFile.close();
	}
}

void dumpZoneData() {
	Serial.println(MAX_EVENTS, DEC);
	for (int i = 0; i < MAX_EVENTS; i++) { 
		Serial.print(zones[i].name);
		Serial.print(zones[i].zone, DEC);
		Serial.print(zones[i].active);
		Serial.print(zones[i].hour, DEC);
		Serial.print(zones[i].minute, DEC);
		Serial.print(zones[i].runTime, DEC);
		Serial.print(zones[i].runOnDay[0]);
		Serial.print(zones[i].runOnDay[1]);
		Serial.print(zones[i].runOnDay[2]);
		Serial.print(zones[i].runOnDay[3]);
		Serial.print(zones[i].runOnDay[4]);
		Serial.print(zones[i].runOnDay[5]);
		Serial.print(zones[i].runOnDay[6]);
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
	for (int i = FIRST_VALVE_PIN; i < FIRST_VALVE_PIN+6; i++) {
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
	
	// has .1 seconds passed?
	nowMillis = millis();
	if ((nowMillis - lasttime) > 100) {
		// set the Arduino Time library's time so now() it is accurate
		WrappedTime realtime = ClockGetTime();

		// Get current time as a string
		ClockGetShortTimeString(nowString, realtime);

		if (strcmp(pgmStatus, szPgmStsRunning) == 0 && strcmp(lasttimestr, nowString) != 0) {
			ClockPrintTime();
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
Serial.print(i, DEC);	Serial.print("  ");			
Serial.print(zones[i].runOnDay[6]);Serial.print(", ");Serial.print(realtime.week, DEC);Serial.print(", ");Serial.print(SATURDAY, DEC);Serial.println(zones[i].minute, DEC);
		// Check for zones to turn on
		// First, is it not already on, is it active and does it run on this day?
		if (zones[i].active == 'Y' 
			&& zones[i].currentState != 1
			&& 
			( (zones[i].runOnDay[0] == 'Y' && realtime.week == SUNDAY)
			|| (zones[i].runOnDay[1] == 'Y' && realtime.week == MONDAY)
			|| (zones[i].runOnDay[2] == 'Y' && realtime.week == TUESDAY)
			|| (zones[i].runOnDay[3] == 'Y' && realtime.week == WEDNESDAY)
			|| (zones[i].runOnDay[4] == 'Y' && realtime.week == THURSDAY)
			|| (zones[i].runOnDay[5] == 'Y' && realtime.week == FRIDAY)
			|| (zones[i].runOnDay[6] == 'Y' && realtime.week == SATURDAY)
			)
		) { 
Serial.print(zones[i].hour, DEC);Serial.print(":");Serial.println(zones[i].minute, DEC);
		
			// OK, is it time?
			if (zones[i].hour == realtime.hour && zones[i].minute ==realtime.minute) {
				// TURN IT ON!
				zones[i].currentState = 1;
				zones[i].lastStart = now();
				turnOn(i + FIRST_VALVE_PIN);
			}
		} //end check for on
		
		// Any to turn off?
		if (zones[i].active == 'Y' && zones[i].currentState == 1) {
			// On long enough?
			if (now() > (zones[i].lastStart + ((zones[i].runTime) * 60))) {
				// TURN IT OFF!
				turnOff(i + FIRST_VALVE_PIN);
				zones[i].currentState = 0;
			}
		}
		
		// NO zone should be on over 99 minutes
		if (zones[i].currentState == 1) {
			// On long enough?
			if (now() > (zones[i].lastStart + (99 * 60))) {
				// TURN IT OFF!
				turnOff(i + 2);
				zones[i].currentState = 0;
			}
		}
	}
}

void turnOff(int pin) {
	if (INVERT_MASTER_PIN == 'Y')
		digitalWrite(MASTER_VALVE_PIN, HIGH);
	else
		digitalWrite(MASTER_VALVE_PIN, LOW);

	if (INVERT_PINS == 'Y')
		digitalWrite(pin, HIGH);
	else
		digitalWrite(pin, LOW);
Serial.print(pin);Serial.println(" OFF!");				
}

void turnOn(int pin) {
	if (INVERT_MASTER_PIN == 'Y')
		digitalWrite(MASTER_VALVE_PIN, LOW);
	else
		digitalWrite(MASTER_VALVE_PIN, HIGH);

	if (INVERT_PINS == 'Y')
		digitalWrite(pin, LOW);
	else
		digitalWrite(pin, HIGH);
Serial.print(pin);Serial.println(" Pin ON!");				
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



