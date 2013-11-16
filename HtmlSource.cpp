/************************************************************************/
/*																		*/
/*	HtmlSource.cpp	--	Content Data for HTTP Server					*/
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
/*	This module contains data declarations for string containing hard-	*/
/*	coded HTML used in rendering the hard-coded pages being served.		*/
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


/* ------------------------------------------------------------ */
/*				Local Type and Constant Definitions				*/
/* ------------------------------------------------------------ */


/* ------------------------------------------------------------ */
/*				Global Variables								*/
/* ------------------------------------------------------------ */


/* ------------------------------------------------------------ */
/*				Local Variables									*/
/* ------------------------------------------------------------ */


/* ------------------------------------------------------------ */
/*				Forward Declarations							*/
/* ------------------------------------------------------------ */


/* ------------------------------------------------------------ */
/*				Procedure Definitions							*/
/* ------------------------------------------------------------ */

/* These lines are used in HTTP Response messages.
*/
char szResponseOk[] = { "HTTP/1.1 200 OK" };
char szResponseNotModified[] = { "HTTP/1.1 304 Not Modified" };
char szResponseNotFound[] = { "HTTP/1.1 404 Not Found" };
char szResponseNotImplemented[] = { "HTTP/1.1 501 Not Implemented" };
char szResponseBadRequest[] = { "HTTP/1.1 400 Bad Request" };
char szResponseNotAuthorized[] = { "HTTP/1.1 401 Not Authorized\nWWW-Authenticate: Basic realm=\"WiH2O\"" };

/* Content type headers for the various types of content we know
** how to return.
*/
char szHeaderContentHtml[] = { "Content-type: text/html" };
char szHeaderContentText[] = { "Content-type: text/plain" };
char szHeaderContentJpeg[] = { "Content-type: image/jpeg" };
char szHeaderContentGif[]  = { "Content-type: image/gif" };
char szHeaderContentPng[]  = { "Content-type: image/png" };
char szHeaderContentJs[]   = { "Content-Type: application/javascript" };
char szHeaderContentJson[]   = { "Content-Type: application/json" };
char szHeaderContentCss[]  = { "Content-Type: text/css" };

char szHeaderLength[] = { "Content-length: " };

// Used to 'force' caching on browser
char szHeaderCache[] = { "Cache-Control: max-age=3153600\nExpires: Wed, 1 Jan 2099 00:00:01 GMT\nLast-Modified: %s" };

/* This text makes up the beginning of the home page being served
*/
char szHomePage[] = {
"<html>\r\n\
<head><meta http-equiv='refresh' content='0;url=/index.htm'>\r\n\
<script>\r\n\
	location.href = '/index.htm';\r\n\
</script>\r\n\
<\head>\r\n\
</html>\r\n"
};

char szStatusPage[] = {
	"{\"status\": \"%s\", \"date\": \"%s\", \"time\": \"%s\"}"
};

char sz401Page[] = {
"<html>\r\n<head><title>HTTP 401 Not Authorized</title></head>\r\n<strong>401 - You are not authorized to view this page.\r\n</html>\r\n"
};

char sz404Page[] = {
"<html>\r\n<head><title>HTTP 404 Not Found</title></head>\r\n<strong>404 - Requested resource not found.</strong><br/>\r\n Is SD card installed?\r\n</html>\r\n"
};

/* These are some simple snipets of HTML for line formatting.
*/
char szHtmlBr[] = { "<br />\r\n" };		//HTML break tag
char szHtmlHr[] = { "<hr />\r\n" };		//HTML horizontal rule tag


/* ------------------------------------------------------------ */

/************************************************************************/

