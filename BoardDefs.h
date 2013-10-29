/************************************************************************/
/*																		*/
/*	BoardDefs.h	--	Declarations for Supported Boards 					*/
/*																		*/
/************************************************************************/
/*	Author:		Gene Apperson											*/
/*    Copyright 2013, Digilent Inc.                                     */
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
/*  File Description:													*/
/*																		*/
/*	This header contains pin definitions for the I/O devices (switches,	*/
/*	buttons, LEDs, etc) used by the sketch.								*/
/*																		*/
/************************************************************************/
/*  Revision History:													*/
/*																		*/
/* 05/24/2012(GeneA): Created											*/
/* 7/21/2013	KeithV: reworked for the WebServer application          */
/*																		*/
/************************************************************************/

#if !defined(_BOARDDEFS_H)
#define	_BOARDDEFS_H

/* ------------------------------------------------------------ */
/*			General Board Declarations							*/
/* ------------------------------------------------------------ */

/* ------------------------------------------------------------ */
/*		Uno32 or uC32 and 64 pin WF32 prototype SD Card	        */
/* ------------------------------------------------------------ */

#if defined(_BOARD_UNO32_) || defined(_BOARD_UNO_) || defined(_BOARD_UC32_) || defined(_BOARD_WF32_64_)

#define	PIN_SDCS	    4
#define PIN_LED_SAFE    PIN_LED2

/* ------------------------------------------------------------ */
/*					WF32 on board SD Card	                    */
/* ------------------------------------------------------------ */

#elif defined(_BOARD_WF32_)

#define	PIN_SDCS	    51
#define PIN_LED_SAFE    PIN_LED1

/* ------------------------------------------------------------ */
/*					Max32 SD Card			                    */
/* ------------------------------------------------------------ */

#elif defined (_BOARD_MAX32_) || defined(_BOARD_MEGA_)

#define	PIN_SDCS	    4
#define PIN_LED_SAFE    PIN_LED1

/* ------------------------------------------------------------ */
/*					Cerebot MX3cK With Pmods					*/
/* ------------------------------------------------------------ */

#elif defined(_BOARD_CEREBOT_MX3CK_)

#define	PIN_SDCS		16
#define PIN_LED_SAFE    PIN_LED1

/* ------------------------------------------------------------ */
/*					Cerebot MX4cK With Pmods					*/
/* ------------------------------------------------------------ */

#elif defined(_BOARD_CEREBOT_MX4CK_)

#define	PIN_SDCS		64
#define PIN_LED_SAFE    PIN_LED1

/* ------------------------------------------------------------ */
/*					Cerebot MX7cK With Pmods					*/
/* ------------------------------------------------------------ */

#elif defined(_BOARD_CEREBOT_MX7CK_)

#define	PIN_SDCS		SS
#define PIN_LED_SAFE    PIN_LED1

/* ------------------------------------------------------------ */
/*						Unknown Board							*/
/* ------------------------------------------------------------ */

#else

#error "System board not defined"

#endif

/* ------------------------------------------------------------ */
/*			Non-Board Specific Declarations						*/
/* ------------------------------------------------------------ */

/* ------------------------------------------------------------ */

#endif

/************************************************************************/
