/************************************************************************/
/*																		*/
/*	Parser.cpp	--	HTTP Request Message Parsing Routines				*/
/*																		*/
/************************************************************************/
/*	Author: 	Gene Apperson											*/
/*	Copyright 2012, Digilent Inc, All Rights Reserved					*/
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
/*	This module contains functions to parse HTTP request messages.		*/
/*																		*/
/************************************************************************/
/*  Revision History:													*/
/*																		*/
/*	05/27/2012(GeneA): created											*/
/*  09/01/2013(Andy Coulson): Modified for WiH2O							*/
/*																		*/
/************************************************************************/


/* ------------------------------------------------------------ */
/*				Include File Definitions						*/
/* ------------------------------------------------------------ */

#include	<WProgram.h>
#include	<SD.h>
#include	<string.h>

#include	"App.h"
#include	"Parser.h"
#include	"Render.h"
#include	"BoardDefs.h"
#include	"ClockWrapper.h"

/* ------------------------------------------------------------ */
/*				Local Type and Constant Definitions				*/
/* ------------------------------------------------------------ */

/* Symbols to set the limits on resources used by the parser.
*/
#define		cchResourcePathMax	256
#define		cchParamNameMax		64
#define		cchParamValueMax	64

/* ------------------------------------------------------------ */
/*				Global Variables								*/
/* ------------------------------------------------------------ */

int		errParse;
int		idresRequest;
int		rtypResource;			//data type of resource
int		cbResource;				//data length of resource

extern uint8_t	rgpinLed[];
extern uint8_t	rgvalLedState[];

extern bool	fSDfs;
extern File	fhData;

extern ZONE_INFO zones[MAX_EVENTS] ;

/* ------------------------------------------------------------ */
/*				Local Variables									*/
/* ------------------------------------------------------------ */

char *	pchInputCur;
//char	szParamLine[cchParamNameMax+cchParamValueMax+2]; 		
char	szResourcePath[cchResourcePathMax+1];
char	szParamName[cchParamNameMax+1];
char	szParamValue[cchParamValueMax+1];

/* ------------------------------------------------------------ */
/*				Forward Declarations							*/
/* ------------------------------------------------------------ */

void ParseGetRequest();
void ParseHeadRequest();
void ParsePostRequest();
void ParsePutRequest();
void ParseTraceRequest();
void ParseOptionsRequest();
void ParseDeleteRequest();
void ParseFileResource();
void ParseFileType(char szRes[], char szTyp[]);
void ParseSetPgmStatus();
void ParseSetTime();
void ParseSaveEvt();
void ParseSetZone();
void ParseFormParameter(char * szName, char * szValue);
void ParseToken(char * szOutput, char chDelim);
char PeekChar();
char NextChar();
void SkipSpace();
void urldecode(char *dst, const char *src);

/* ------------------------------------------------------------ */
/*				Procedure Definitions							*/
/* ------------------------------------------------------------ */
/***	InitLineParse
**
**	Parameters:
**		szLine		- pointer to line to start parsing
**
**	Return Value:
**
**	Errors:
**
**	Description:
**
*/

void InitLineParse(char * szLine) {

	//errParse = errOK;
	pchInputCur = szLine;

}

/* ------------------------------------------------------------ */
/***	ParseHttpRequest
**
**	Parameters:
**		none
**
**	Return Value:
**		none
**
**	Errors:
**		none
**
**	Description:
**		This function parses the start line of the HTTP Request
**		message. The parsing is done using a very simple recursive
**		descent parser.
*/

void ParseHttpRequest() {
	char	szMethod[16];
	char	szVersion[16];

	errParse = errOK;
	idresRequest = idresNil;
	rtypResource = rtypNil;
	cbResource = 0;

	/* The start line of an HTTP request contains three tokens
	** delimited by space characters (ASCII 0x20) or end of line.
	**    method - specifies what action to take
	**    url - specifies the server resource being requested
	**    version - specifies the HTTP version being used
	*/
	SkipSpace();
	ParseToken(szMethod, ' ');

	/* Branch based on the method. The only method currently supported
	** is GET.
	*/
	if (stricmp(szMethod, "GET") == 0) {
		ParseGetRequest();
	}
	else if (stricmp(szMethod, "HEAD") == 0) {
		ParseHeadRequest();
	}
	else if (stricmp(szMethod, "POST") == 0) {
		ParsePostRequest;
	}
	else if (stricmp(szMethod, "PUT") == 0) {
		ParsePutRequest;
	}
	else if (stricmp(szMethod, "TRACE") == 0) {
		ParseTraceRequest;
	}
	else if (stricmp(szMethod, "OPTIONS") == 0) {
		ParseOptionsRequest;
	}
	else if (stricmp(szMethod, "DELETE") == 0) {
		ParseDeleteRequest;
	}
	else {
		/* Unknown method
		*/
		errParse = errBadRequest;
	}

	/* Parse the HTTP version string from the end. We don't
	** currently do anything with this.
	*/
	SkipSpace();
	ParseToken(szVersion, ' ');
}

/* ------------------------------------------------------------ */
/***	ParseGetRequest
**
**	Parameters:
**		none
**
**	Return Value:
**		none
**
**	Errors:
**		none
**
**	Description:
**		The second token on an HTTP Request start line is the URL
**		of the resource being referenced. In general, this is a path
**		to the resource. If using forms, this can also specify the
**		list of parameters for the form.
*/

void ParseGetRequest() {

	/* Get the resource path. This is delimited by a space character
	** if there are no form parameters, or a '?' if there are
	** form paramters.
	** If the resource path isn't one of the hard codes 'special'
	** URLs, it is presumed to be the name of a file on an SD card
	** attached to the default SPI port.
	*/
	SkipSpace();
	ParseToken(szResourcePath, '?');
	/* See which resource is being requested */
	if (stricmp(szResourcePath, szResHomePage) == 0) {
		/* Request for the 'home' page. */
		idresRequest = idresHomePage;
		rtypResource = rtypHtml;
	}
	else if (stricmp(szResourcePath, szResStatusPage) == 0) {
		/* Request for the status */
		idresRequest = idresStatusPage;
		rtypResource = rtypText;
	}
	else if (stricmp(szResourcePath, szResSetStatus) == 0) {
		/* Request for RESTful service to set program status on/off		*/		
		NextChar();			// Eat the '?' that delimited the path part of the URL
		ParseSetPgmStatus();		// parse the form parameters and set status
		idresRequest = idresSetStatus;
		rtypResource = rtypText;
	}
	else if (stricmp(szResourcePath, szResSetTime) == 0) {
		/* Request for RESTful service to set date/time		*/		
		NextChar();			// Eat the '?' that delimited the path part of the URL
		ParseSetTime();		// parse the form parameters and set date and time
		idresRequest = idresSetTime;
		rtypResource = rtypText;
	}
	else if (stricmp(szResourcePath, szResSaveEvent) == 0) {
		/* Request for save of event info		*/		
		NextChar();
		ParseSaveEvt();
		idresRequest = idresSaveEvent;
		rtypResource = rtypText;
	}
	else if (stricmp(szResourcePath, szResSetZoneState) == 0) {
		/* Request for save of event info		*/		
		NextChar();
		ParseSetZone();
		idresRequest = idresSaveEvent;
		rtypResource = rtypText;
	}
	else if (stricmp(szResourcePath, szResGetEvents) == 0) {
		/* Request for event list		*/		
		idresRequest = idresEventList;
		rtypResource = rtypText;
	}
	else {
		/* The requested resource isn't one of the hard coded URLs. The
		** presumption then is that it is a file in the local file system.
		*/
		if (fSDfs) {
			/* We have a file system, see if the file exists.
			*/
			ParseFileResource();
		}
		else {
			/* No file system available, respond that resource not found.
			*/
			errParse = errNotFound;
		}
	}
}

/* ------------------------------------------------------------ */
/***	ParseHeadRequest
**
**	Parameters:
**		none
**
**	Return Value:
**		none
**
**	Errors:
**		Returns parse errors in errParse global
**
**	Description:
**		This function is a placeholder for parsing the HTTP HEAD
**		method.
*/

void
ParseHeadRequest()
	{

	errParse = errNotImplemented;


}

/* ------------------------------------------------------------ */
/***	ParsePostRequest
**
**	Parameters:
**		none
**
**	Return Value:
**		none
**
**	Errors:
**		Returns parse errors in errParse global
**
**	Description:
**		This function is a placeholder for parsing the HTTP POST
**		method.
*/

void ParsePostRequest() {

	errParse = errNotImplemented;

}

/* ------------------------------------------------------------ */
/***	ParsePutRequest
**
**	Parameters:
**		none
**
**	Return Value:
**		none
**
**	Errors:
**		Returns parse errors in errParse global
**
**	Description:
**		This function is a placeholder for parsing the HTTP PUT
**		method.
*/

void ParsePutRequest() {

	errParse = errNotImplemented;

}

/* ------------------------------------------------------------ */
/***	ParseTraceRequest
**
**	Parameters:
**		none
**
**	Return Value:
**		none
**
**	Errors:
**		Returns parse errors in errParse global
**
**	Description:
**		This function is a placeholder for parsing the HTTP TRACE
**		method.
*/

void ParseTraceRequest() {

	errParse = errNotImplemented;

}

/* ------------------------------------------------------------ */
/***	ParseOptionsRequest
**
**	Parameters:
**		none
**
**	Return Value:
**		none
**
**	Errors:
**		Returns parse errors in errParse global
**
**	Description:
**		This function is a placeholder for parsing the HTTP OPTIONS
**		method.
*/

void ParseOptionsRequest() {

	errParse = errNotImplemented;

}

/* ------------------------------------------------------------ */
/***	ParseDeleteRequest
**
**	Parameters:
**		none
**
**	Return Value:
**		none
**
**	Errors:
**		Returns parse errors in errParse global
**
**	Description:
**		This function is a placeholder for parsing the HTTP DELETE
**		method.
*/

void ParseDeleteRequest() {

	errParse = errNotImplemented;

}

/* ------------------------------------------------------------ */
/***	ParstHttpHeader
**
**	Parameters:
**
**	Return Value:
**
**	Errors:
**
**	Description:
**		This function parses an HTTP Request header line and sets
**		the global state as appropriate. We don't support any headers,
**		so this function does nothing. It is a place holder where
**		header parsing logic could be added.
*/

void ParseHttpHeader() {
	char	szHdr[32];

	SkipSpace();
	ParseToken(szHdr, ':');
	// Resource is candidate for caching, browser sent If-Modified-Since
	// For now, send 304 in every such case (should really parse date and compare against now)
	if (stricmp(szHdr, "If-Modified-Since") == 0) {	
		errParse = errNotModified;
	}

}

/* ------------------------------------------------------------ */
/***	ParseFileResource
**
**	Parameters:
**		none
**
**	Return Value:
**		none
**
**	Errors:
**		none
**
**	Description:
**		This function parses the URL for a resource contained in
**		a file on the SD card file system. It determines the data type
**		of the file contents.
*/
void ParseFileResource() {
	char	szFileType[4];

	Serial.print("File: ");
	Serial.println(szResourcePath);

	idresRequest = idresFile;

	/* Determine it's length and type.
	*/
	//exists
	if (!SD.exists(szResourcePath)) {
		errParse = errNotFound;
		Serial.println("File not found");
		return;
	}

	ParseFileType(szResourcePath, szFileType);

	if (stricmp(szFileType, "txt") == 0 || stricmp(szFileType, "do") == 0) {	//Temporary JSON hack
		rtypResource = rtypText;
	}
	else if (stricmp(szFileType, "htm") == 0) {
		rtypResource = rtypHtml;
	}
	else if (stricmp(szFileType, "js") == 0) {
		rtypResource = rtypJs;
	}
	else if (stricmp(szFileType, "css") == 0) {
		rtypResource = rtypCss;
	}
	else if (stricmp(szFileType, "jpg") == 0) {
		rtypResource = rtypJpeg;
	}
	else if (stricmp(szFileType, "gif") == 0) {
		rtypResource = rtypGif;
	}
	else if (stricmp(szFileType, "png") == 0) {
		rtypResource = rtypPng;
	}
	else {
		rtypResource = rtypNil;
	}

}

/* ------------------------------------------------------------ */
/***	ParseFileType
**
**	Parameters:
**		szRes		- string containing the resource URL
**		szTyp		- buffer to receive file type string
**
**	Return Value:
**		none
**
**	Errors:
**		none
**
**	Description:
**		Skip ahead to the file type and then copy the characters
**		to the output string.
*/

void ParseFileType(char szRes[], char szTyp[]) {
	char	ch;
	int		ich;
	char *	pchIn;

	pchIn = &szRes[0];

	szTyp[0] = '\0';

	/* Find the '.' that delimits the file type.
	*/
	while ((ch = *pchIn++) != '\0') {
		if (ch == '.') {
			break;
		}
	}

	/* Copy up to three characters to the output string.
	*/
	for (ich = 0; ich < 3; ich++) {
		if ((ch = *pchIn++) == '\0') {
			break;
		}
		szTyp[ich] = ch;
	}

	szTyp[ich] = '\0';

}

/* ------------------------------------------------------------ */
/***	ParseSetPgmStatus
**	Description:
**		The URL for the form GET request looks like this:
**
**		/setsts.do?state=Running
*/
void ParseSetPgmStatus() {
	while (PeekChar() != ' ') {
		/* Get the next NAME, VALUE pair from the line */
		ParseFormParameter(szParamName, szParamValue);

		/* Map the parameter value to Paused or Running */
		if (stricmp(szParamValue, "Paused") == 0) {
			pgmStatus = szPgmStsPaused;
		}
		else if (stricmp(szParamValue, "Running") == 0) {
			pgmStatus = szPgmStsRunning;
		}
		else {
			/* Have an invalid paramter value */
			errParse = errBadRequest;
		}
	}
}

/* ------------------------------------------------------------ */
/***	ParseSetTime
**	Description:
**		The URL for the form GET request looks like this:
**
**		/settime.do?date=MM-DD-SS&time=hh:mm:ss
*/
void ParseSetTime() {
	while (PeekChar() != ' ') {
		/* Get the next NAME, VALUE pair from the line */
		ParseFormParameter(szParamName, szParamValue);

		/* set date to provided value */
		if (stricmp(szParamName, "date") == 0) {
			ClockSetDate(szParamValue);			
		} else if (stricmp(szParamName, "time") == 0) {
			/* set time to provided value */
			ClockSetTime(szParamValue);
		} else {
			/* Have an invalid paramter value */
			errParse = errBadRequest;
		}

		/* Eat the '&' so we are ready to parse the next parameter.		*/
		if (PeekChar() == '&') { NextChar(); }
	}
}

/* ------------------------------------------------------------ */
/***	ParseSetZone
**
**	Parameters:
**		state
**
**	Return Value:
**		none
**
**	Errors:
**		none
**
**	Description:
**		The URL for the form GET request looks like this:
**
**		/setzone.do?zoneNum=0&zoneState=1 (where zoneState= 0 or 1 for off or on)
*/
void ParseSetZone() {
	int			eventIdx = 0;
	
	while (PeekChar() != ' ') {
		/* Get the next NAME, VALUE pair from the line */
		ParseFormParameter(szParamName, szParamValue);

		if (stricmp(szParamName, "zoneNum") == 0) {
			eventIdx = atoi(szParamValue);
		} else if (stricmp(szParamName, "zoneState") == 0) {
//Serial.print("setzone: "); Serial.print(eventIdx); Serial.println(atoi(szParamValue), DEC);
			if (atoi(szParamValue) == 0) {
				turnOff(eventIdx-1);
			} else if (atoi(szParamValue) == 1)  {
				turnOn(eventIdx-1);
			}
		} else {
			/* Have an invalid parameter name. */
			errParse = errBadRequest;
		}
				
		/* Eat any '&' so we are ready to parse the next parameter.		*/
		if (PeekChar() == '&') { NextChar(); }
	}
	errParse = errOK;
}
	
/* ------------------------------------------------------------ */
/***	ParseSaveEvt
**
**	Parameters:
**		state
**
**	Return Value:
**		none
**
**	Errors:
**		none
**
**	Description:
**		The URL for the form GET request looks like this:
**
**		/saveevnt.do?eventNumber=0&eventName=-unspecified-&zoneNumber=1&eventStart=12%3A00%3A00&eventRuntime=10&eventSunday=1&eventWednesday=1
*/
void ParseSaveEvt() {
	int			eventIdx = 0;
	char  		eventName[64];
	char  		decodedName[64];
	int			zoneNum = 0;
	char		eventStart[10];
	int 		eventStartHour = 0;
	int 		eventStartMinute = 0;
	int 		eventRuntime = 0;
	char		eventSunday = 'N';
	char		eventMonday = 'N';
	char		eventTuesday = 'N';
	char		eventWednesday = 'N';
	char		eventThursday = 'N';
	char		eventFriday = 'N';
	char		eventSaturday = 'N';
	char		szTemp[10];

	eventName[0] = 0;
	
	while (PeekChar() != ' ') {
		/* Get the next NAME, VALUE pair from the line
		*/
		ParseFormParameter(szParamName, szParamValue);

		if (stricmp(szParamName, "eventIdx") == 0) {
			eventIdx = atoi(szParamValue);
		}
		else if (stricmp(szParamName, "eventName") == 0) {
			strncpy(eventName, szParamValue, strlen(szParamValue));
			eventName[strlen(szParamValue)] = 0;
//Serial.print("eventName: "); Serial.println(eventName);
		}
		else if (stricmp(szParamName, "zoneNumber") == 0) {
			zoneNum = atoi(szParamValue);
//Serial.print("zoneNum: "); Serial.println(zoneNum, DEC);
		}
		else if (stricmp(szParamName, "eventStart") == 0) {
			strncpy(szTemp, szParamValue, 2);szTemp[2] =0;
			eventStartHour = atoi(szTemp);
			strncpy(szTemp, szParamValue+5, 2);szTemp[2] =0;
			eventStartMinute = atoi(szTemp);
//Serial.print("eventStartHour: "); Serial.println(eventStartHour, DEC);
//Serial.print("eventStartMinute: "); Serial.println(eventStartMinute, DEC);
		}
		else if (stricmp(szParamName, "eventRuntime") == 0) {
			eventRuntime = atoi(szParamValue);
//Serial.print("eventRuntime: "); Serial.println(eventRuntime, DEC);
		}
		else if (stricmp(szParamName, "eventSun") == 0) {
			eventSunday = 'Y';
		}
		else if (stricmp(szParamName, "eventMon") == 0) {
			eventMonday = 'Y';
		}
		else if (stricmp(szParamName, "eventTues") == 0) {
			eventTuesday = 'Y';
		}
		else if (stricmp(szParamName, "eventWed") == 0) {
			eventWednesday = 'Y';
		}
		else if (stricmp(szParamName, "eventThurs") == 0) {
			eventThursday = 'Y';
		}
		else if (stricmp(szParamName, "eventFri") == 0) {
			eventFriday = 'Y';
		}
		else if (stricmp(szParamName, "eventSat") == 0) {
			eventSaturday = 'Y';
//Serial.print("eventdays: "); Serial.println(eventSunday); Serial.println(eventMonday); Serial.println(eventTuesday); 
//Serial.println(eventWednesday); Serial.println(eventThursday); Serial.println(eventFriday); Serial.println(eventSaturday); Serial.println("");
		}
		else {
			/* Have an invalid parameter name.
			*/
			errParse = errBadRequest;
		}
				
		/* Eat the '&' so we are ready to parse the next parameter.		*/
		if (PeekChar() == '&') { NextChar(); }
	}

	//urldecode(decodedName, eventName);
	strncpy(zones[eventIdx].name, eventName, 32);zones[eventIdx].name[33] = 0;
	if (zoneNum > 0) {
		zones[eventIdx].active = 'Y';
		zones[eventIdx].zone = zoneNum;
	} else {
		zones[eventIdx].active = 'N';
		zones[eventIdx].zone = 0;
	}
	zones[eventIdx].hour = eventStartHour;
	zones[eventIdx].minute = eventStartMinute;
	zones[eventIdx].runTime = eventRuntime;
	zones[eventIdx].runOnDay[0] = eventSunday;
	zones[eventIdx].runOnDay[1] = eventMonday;
	zones[eventIdx].runOnDay[2] = eventTuesday;
	zones[eventIdx].runOnDay[3] = eventWednesday;
	zones[eventIdx].runOnDay[4] = eventThursday;
	zones[eventIdx].runOnDay[5] = eventFriday;
	zones[eventIdx].runOnDay[6] = eventSaturday;

	saveConfig();
	//dumpZoneData();
	errParse = errOK;
}

/* ------------------------------------------------------------ */
/***	ParseFormParameter
**
**	Parameters:
**		szName		- buffer to receive parameter name
**		szValue		- buffer to receive parameter value
**
**	Return Value:
**		none
**
**	Errors:
**		none
**
**	Description:
**		Parse the next NAME,VALUE pair from the input string.
*/

void ParseFormParameter(char * szName, char * szValue) {

	/* Scan the next token up to the '=' that separates the
	** name from the value.
	*/
	ParseToken(szName, '=');

	/* Eat the '='
	*/
	NextChar();

	/* Scan the next token up to the '&' that delimits the next
	** parameter, or the ' ' that indicates the end.
	** Note: In general there could also be a '#' followed by a 
	** fragment name, but we're not supporting that in this implementation.
	*/
	ParseToken(szValue, '&');

}

/* ------------------------------------------------------------ */
/***	ParseToken
**
**	Parameters:
**		szOutput	- pointer to buffer to receive token
**		chDelim		- delimiter character for token
**
**	Return Value:
**		none
**
**	Errors:
**		none
**
**	Description:
**		Get the next token from the input line. Copies characters
**		up to the end of string, next space character or the specified delimiter
**		to the output buffer.
*/

void ParseToken(char * szOutput, char chDelim) {
	char	ch;

	/* Copy any characters up to, but not includeing, the delimiter to 
	** the output buffer.
	*/
	while(true) {
		ch = *pchInputCur;
		if ((ch == '\0') || (ch == ' ') || (ch == chDelim)) {
			break;
		}
		if (ch == '+') ch = ' '; 
		*szOutput = ch;
		pchInputCur += 1;
		szOutput += 1;
	}

	*szOutput = '\0';

}

/* ------------------------------------------------------------ */
/***	PeekChar
**
**	Parameters:
**		none
**
**	Return Value:
**		returns the next character in the input stream
**
**	Errors:
**
**	Description:
**		Returns the next character in the input stream without
**		removing it from the input stream.
*/

char
PeekChar()
	{
	return *pchInputCur;
}

/* ------------------------------------------------------------ */
/***	NextChar
**
**	Parameters:
**		none
**
**	Return Value:
**		next character in input stream
**
**	Errors:
**		none
**
**	Description:
**		Return the next character in the input stream, removing it
**		from the input stream.
*/

char
NextChar()
	{
	char	ch;

	if ((ch = *pchInputCur) != '\0') {
		pchInputCur += 1;
	}

	return ch;

}

/* ------------------------------------------------------------ */
/***	SkipSpace
**
**	Parameters:
**		none
**
**	Return Value:
**		none
**
**	Errors:
**		none
**
**	Description:
**		Skip over any space characters in the input string
*/

void
SkipSpace()
	{
	while (*pchInputCur == ' ') {
		pchInputCur += 1;
	}
}

void urldecode(char *dst, const char *src) {
        char a, b;
        while (*src) {
                if ((*src == '%') &&
                    ((a = src[1]) && (b = src[2])) &&
                    (isxdigit(a) && isxdigit(b))) {
                        if (a >= 'a')
                                a -= 'A'-'a';
                        if (a >= 'A')
                                a -= ('A' - 10);
                        else
                                a -= '0';
                        if (b >= 'a')
                                b -= 'A'-'a';
                        if (b >= 'A')
                                b -= ('A' - 10);
                        else
                                b -= '0';
                        *dst++ = 16*a+b;
                        src+=3;
                } else {
                        *dst++ = *src++;
                }
        }
        *dst++ = '\0';
}
/************************************************************************/

