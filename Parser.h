/************************************************************************/
/*																		*/
/*	stdfmt.h	--														*/
/*																		*/
/************************************************************************/
/*	Author:																*/
/*	Copyright															*/
/************************************************************************/
/*  File Description:													*/
/*																		*/
/*																		*/
/************************************************************************/
/*  Revision History:													*/
/*																		*/
/*  09/01/2013(Andy Coulson): Modified for WiH2O							*/
/*																		*/
/************************************************************************/

#if !defined(_PARSER_H)
#define _PARSER_H

/* ------------------------------------------------------------ */
/*					Miscellaneous Declarations					*/
/* ------------------------------------------------------------ */

/* Resource identifiers.
*/
#define	idresNil			0
#define	idresHomePage		1
#define	idresHardwarePage	2
#define	idresStatusPage		3
#define	idresSetStatus		4
#define	idresSetTime		5
#define	idresEventList		6
#define idresSaveEvent		7
#define	idresLedForm		8
#define	idresFile			9
#define	idres404			10
#define	idres401			11

/* Resource names.
*/
#define	szResHomePage		"/"
#define	szResHardwarePage	"/hardware"
#define	szResStatusPage		"/getsts.do"
#define	szResSetStatus		"/setsts.do"
#define	szResSaveEvent		"/saveevnt.do"
#define	szResSetZoneState	"/setzone.do"
#define	szResGetEvents		"/getevnts.do"
#define	szResSetTime		"/settime.do"

/* Resource data types.
*/
#define	rtypNil				0		//unknown/undefined data type
#define	rtypText			1		//plain text
#define	rtypHtml			2		//HTML text
#define	rtypJpeg			3		//JPEG image
#define	rtypGif				4		//GIF image
#define	rtypPng				5		//PNG image
#define	rtypJs				6		//Javascript image
#define	rtypCss				7		//CSS image
#define	rtypJson			8		//JSON data request

/* Parser error codes.
*/
#define	errOK				0		//no error
#define	errNotFound			1		//resource not found
#define	errNotImplemented	2		//method not supported
#define	errBadRequest		3		//unknown method requested
#define	errNotModified		4		//not-changed (cache)
#define	errNotAuthorized	5		//not-authorized (HTTP 401 Not Authorized)

/* ------------------------------------------------------------ */
/*					Variable Declarations						*/
/* ------------------------------------------------------------ */


/* ------------------------------------------------------------ */
/*					Procedure Declarations						*/
/* ------------------------------------------------------------ */

void	InitLineParse(char * szLine);
void	ParseHttpRequest();
void	ParseHttpHeader();

/* ------------------------------------------------------------ */

#endif	// _PARSER_H

/************************************************************************/
