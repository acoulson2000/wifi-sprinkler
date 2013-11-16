/************************************************************************/
/*																		*/
/*	Render.cpp	--	HTML Document Output Rendering Routines				*/
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
/*	This module contains utility functions to render hard coded HTML	*/
/*	strings for the hard coded pages returned by the HTTP server.		*/
/*																		*/
/************************************************************************/
/*  Revision History:													*/
/*																		*/
/*	05/28/2012(GeneA): created											*/
/*	06/18/2012(GeneA): Moved all response generation code to this		*/
/*		module															*/
/*  09/01/2013(Andy Coulson): Modified for WiH2O							*/
/*																		*/
/************************************************************************/


/* ------------------------------------------------------------ */
/*				Include File Definitions						*/
/* ------------------------------------------------------------ */

#include	<WProgram.h>
#include	<inttypes.h>
#include    <DNETcK.h>
#include	<SD.h>

#include	"HtmlSource.h"
#include	"Parser.h"
#include	"Render.h"
#include	"BoardDefs.h"

#include	"App.h"
#include	"ClockWrapper.h"


/* ------------------------------------------------------------ */
/*				Local Type and Constant Definitions				*/
/* ------------------------------------------------------------ */
/* ------------------------------------------------------------ */


/* ------------------------------------------------------------ */
/*				Global Variables								*/
/* ------------------------------------------------------------ */

extern TcpClient tcpClient;

extern uint8_t	rgpinBtn[];
extern uint8_t	rgpinSwt[];
extern uint8_t	rgpinLed[];
extern uint8_t	rgvalLedState[];

extern int		errParse;
extern int		idresRequest;
extern int		rtypResource;			//data type of resource
extern int		cbResource;				//data length of resource
extern bool		fSDfs;
extern char		szResourcePath[];

extern File	fhData;

extern int			requireAuth;
extern int			errHttpRequest;
extern ZONE_INFO	events[MAX_EVENTS] ;

/* ------------------------------------------------------------ */
/*				Local Variables									*/
/* ------------------------------------------------------------ */
uint8_t	rgbFileBuf[cbFileBufMax];
char 	stsBuf[256];


/* ------------------------------------------------------------ */
/*				Forward Declarations							*/
/* ------------------------------------------------------------ */

void RenderHomePage();
void RenderStatusPage();
void putCacheHeaders();
void RenderEventListPage();
void RenderOKPage();
void Render401Page();
void Render404Page();

/* ------------------------------------------------------------ */
/*				Procedure Definitions							*/
/* ------------------------------------------------------------ */
/***	InitHttpRequest
**
**	Parameters:
**
**	Return Value:
**
**	Errors:
**
**	Description:
**		This function initializes the global state at the beginning
**		of processing the next HTTP request message.
*/

void InitHttpRequest() {
	// If we are requiring auth credentials in http request, default 
	// errHttpRequest to errNotAuthorized - it will be set to errOK
	// if we get the correct authorization header.
	if (requireAuth == 1) {
		errHttpRequest = errNotAuthorized;
	} else {
		errHttpRequest = errOK;
	}
}

/* ------------------------------------------------------------ */
/***	PutResponseStart
**
**	Parameters:
**
**	Return Value:
**
**	Errors:
**
**	Description:
**
*/
void PutResponseStart() {

	if (errHttpRequest == errOK) {
		/* Send a request successful header. */
		Serial.println(szResponseOk);
		tcpClient.println(szResponseOk);
	}
	else if (errHttpRequest == errNotFound) {
		/* Send the resource not found header
		*/
		Serial.println(szResponseNotFound);
		tcpClient.println(szResponseNotFound);
	}
	else if (errHttpRequest == errNotImplemented) {
		/* Send the method not implemented header */
		Serial.println(szResponseNotImplemented);
		tcpClient.println(szResponseNotImplemented);
	}
	else if (errHttpRequest == errNotModified) {
		/* if the request was for a json response, it should never return a 304,
			even if there was a if-not-modified header */
		if (rtypResource == rtypJson) {
			Serial.println(szResponseOk);
			tcpClient.println(szResponseOk);
		} else {
			/* Send the method not modified header */
			Serial.println(szResponseNotModified);
			tcpClient.println(szResponseNotModified);
		}
	}
	else if (errHttpRequest == errNotAuthorized) {
		/* Send the method not authorized header */
		Serial.println(szResponseNotAuthorized);
		tcpClient.println(szResponseNotAuthorized);
	}
	else {
		/* Something else wrong with the request. Send
		** the bad request response.
		*/
		Serial.println(szResponseBadRequest);
		tcpClient.println(szResponseBadRequest);
	}
}

/* ------------------------------------------------------------ */
/***	PutResponseHeader
**
**	Parameters:
**
**	Return Value:
**
**	Errors:
**
**	Description:
**
*/
void PutResponseHeader() {
	/* Send the content type of the response. */
	switch(rtypResource) {
		case rtypJs:
			//Serial.println(szHeaderContentJs);
			tcpClient.println(szHeaderContentJs);
			// Encourage browser caching
			putCacheHeaders();
			break;

		case rtypCss:
			//Serial.println(szHeaderContentCss);
			tcpClient.println(szHeaderContentCss);
			putCacheHeaders();
			break;

		case rtypText:
			//Serial.println(szHeaderContentText);
			tcpClient.println(szHeaderContentText);
			break;

		case rtypJson:
			//Serial.println(szHeaderContentJson);
			tcpClient.println(szHeaderContentJson);
			break;

		case rtypHtml:
			//Serial.println(szHeaderContentHtml);
			tcpClient.println(szHeaderContentHtml);
			//Serial.println(szHeaderCache);	// seems to cause problems with IE and FF
			//tcpClient.println(szHeaderCache);
			break;

		case rtypJpeg:
			//Serial.println(szHeaderContentJpeg);
			tcpClient.println(szHeaderContentJpeg);
			putCacheHeaders();
			break;

		case rtypGif:
			//Serial.println(szHeaderContentGif);
			tcpClient.println(szHeaderContentGif);
			putCacheHeaders();
			break;

		case rtypPng:
			//Serial.println(szHeaderContentPng);
			tcpClient.println(szHeaderContentPng);
			putCacheHeaders();
			break;
	}

	/* Send the content length of the response.
	*/
	if (cbResource != 0) {
		tcpClient.print(szHeaderLength);
		tcpClient.println(cbResource, DEC);
	}

	/* Put out a blank line to indicate the end of the header.
	*/
	tcpClient.println("");

}

void putCacheHeaders() {
	char hdrBuf[128];
	char timeBuf[50];
	WrappedTime t = ClockGetTime();
	t.hour = 0; t.minute = 0; t.second = 0;
	ClockGetHttpTimeString(timeBuf, t);
	sprintf(hdrBuf, szHeaderCache, timeBuf);
	tcpClient.println(hdrBuf);
}

/* ------------------------------------------------------------ */
/***	PutReponseBody
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
**		This function will generate the body of the HTTP response
**		message back to the client.
*/

void PutResponseBody() {

	/* Switch based on the requested resource.
	*/
	switch(idresRequest) {
		case idresHomePage:
			RenderHomePage();
			break;

		case idresStatusPage:
			RenderStatusPage();
			break;

		case idresSetStatus:
			RenderStatusPage();
			break;
			
		case idresSetTime:
			RenderOKPage();
			break;
			
		case idresSaveEvent:
			RenderOKPage();
			break;
			
		case idres401:
			Render401Page();
			break;
			
		case idres404:
			Render404Page();
			break;
			
		case idresEventList:
			RenderEventListPage();
			break;
	}
}

/* ------------------------------------------------------------ */
/***	ReadFileBuffer
**
**	Parameters:
**		rgbBuf		- buffer to receive the data
**		cbReq		- number of bytes to read
**
**	Return Value:
**		Returns number of bytes actually read
**
**	Errors:
**		none
**
**	Description:
**		Read the requested number of bytes from the data file.
*/

int
ReadFileBuffer(uint8_t rgbBuf[], int cbReq)
	{
	int		cb;
	int		cbRem;
	int		ib;

	cbRem = fhData.available();
	cb = (cbRem < cbReq) ? cbRem : cbReq;

	for (ib = 0; ib < cb; ib++) {
		rgbBuf[ib] = fhData.read();
	}

	return cb;
}

/* ------------------------------------------------------------ */
/***	RenderHomePage
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
**		This function renders the hard coded HTML for the home
**		page.
*/

void RenderHomePage() {

	/* We're going to be using tcpClient.print to render the output for the page.
	** With the block time set to msImmediate, it is possible for data to be lost
	** if the sockeet buffer fills up. So, we set the block time to 5 sec. temporarily
	** while printing the page and then back to msImmediate when we are done.
	*/
	DNETcK::setDefaultBlockTime(5000);
    
	tcpClient.print(szHomePage);

	/* Resote block time to msImmediate.
	*/    
	DNETcK::setDefaultBlockTime(DNETcK::msImmediate);

}

/* ------------------------------------------------------------ */
/***	RenderStatusPage
**		This function renders the status as JSON.
*/

void RenderStatusPage() {

	/* We're going to be using tcpClient.print to render the output for the page.
	** With the block time set to msImmediate, it is possible for data to be lost
	** if the sockeet buffer fills up. So, we set the block time to 5 sec. temporarily
	** while printing the page and then back to msImmediate when we are done.
	*/
	DNETcK::setDefaultBlockTime(5000);
    
	char dateBuf[10];
	char timeBuf[10];
	WrappedTime t = ClockGetTime();
	ClockGetShortDateString(dateBuf, t);
	ClockGetShortTimeString(timeBuf, t);
	sprintf(stsBuf, szStatusPage, pgmStatus, dateBuf, timeBuf);
	tcpClient.println(stsBuf);

	/* Resote block time to msImmediate.
	*/    
	DNETcK::setDefaultBlockTime(DNETcK::msImmediate);

}

void RenderEventListPage() {
	DNETcK::setDefaultBlockTime(5000);
    //[{eventIdx: 1, eventName: 'Foobar1'}, {eventIdx: 2, eventName: 'Foobar2'}];
	tcpClient.print("{\"maxZones\": ");
	sprintf( stsBuf, "%d", MAX_ZONES );
	tcpClient.print(stsBuf);
	tcpClient.println(",\n \"events\": [");
	for (int i = 0; i < MAX_EVENTS; i++) {
		if (i > 0) 	tcpClient.println(",");
		stsBuf[0] = 0;
		sprintf(
			stsBuf, 
			"\t{\"eventIdx\": %d, \"eventName\": \"%s\", \"zoneNumber\": %d, \"eventActive\": \"%c\", \"eventTime\": \"%02d:%02d\", \"eventRuntime\": %d, \"eventRunOn\": \"%s\", \"lastStart\": \"%d\"}",
			i, events[i].name, events[i].zone, events[i].active, events[i].hour, events[i].minute, events[i].runTime, events[i].runOnDay, events[i].lastStart);
		tcpClient.println(stsBuf);
	}
	tcpClient.println("]}");

	DNETcK::setDefaultBlockTime(DNETcK::msImmediate);
}

void RenderOKPage() {
	DNETcK::setDefaultBlockTime(1000);
	tcpClient.println("OK");
	DNETcK::setDefaultBlockTime(DNETcK::msImmediate);
}

void Render401Page() {
	DNETcK::setDefaultBlockTime(5000);   
	tcpClient.print(sz401Page);
	DNETcK::setDefaultBlockTime(DNETcK::msImmediate);
}

void Render404Page() {
	DNETcK::setDefaultBlockTime(5000);   
	tcpClient.print(sz404Page);
	DNETcK::setDefaultBlockTime(DNETcK::msImmediate);
}

/************************************************************************/

