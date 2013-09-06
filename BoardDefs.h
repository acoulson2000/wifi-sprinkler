/************************************************************************/
/*																		*/
/*	BoardDefs.h	--	Declarations for Supported Boards 					*/
/*																		*/
/************************************************************************/
/*	Author:		Gene Apperson											*/
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
/*  File Description:													*/
/*																		*/
/*	This header contains pin definitions for the I/O devices (switches,	*/
/*	buttons, LEDs, etc) used by the sketch.								*/
/*																		*/
/************************************************************************/
/*  Revision History:													*/
/*																		*/
/* 05/24/2012(GeneA): Created											*/
/*																		*/
/************************************************************************/

#if !defined(_BOARDDEFS_H)
#define	_BOARDDEFS_H

/* ------------------------------------------------------------ */
/*			General Board Declarations							*/
/* ------------------------------------------------------------ */

#define	NUM_BIO_BTN 4
#define	NUM_BIO_SWT 4
#define	NUM_BIO_LED	8

/* ------------------------------------------------------------ */
/*				Uno32 or uC32 With Basic I/O Shield				*/
/* ------------------------------------------------------------ */

#if defined(_BOARD_UNO32_) || defined(_BOARD_UNO_) || defined(_BOARD_UC32_)

/* Push Buttons
*/
#define	PIN_BIO_BTN1	4
#define	PIN_BIO_BTN2	34
#define	PIN_BIO_BTN3	36
#define	PIN_BIO_BTN4	37

/* Slide Switches
*/
#define	PIN_BIO_SWT1	2
#define	PIN_BIO_SWT2	7
#define	PIN_BIO_SWT3	8
#define	PIN_BIO_SWT4	35

/* Discrete LEDs
*/
#define	PIN_BIO_LED1	33
#define	PIN_BIO_LED2	32
#define	PIN_BIO_LED3	31
#define	PIN_BIO_LED4	30
#define	PIN_BIO_LED5	29
#define	PIN_BIO_LED6	28
#define	PIN_BIO_LED7	27
#define	PIN_BIO_LED8	26

#define	PIN_SDCS	4

/* ------------------------------------------------------------ */
/*					Max32 With Basic I/O Shield					*/
/* ------------------------------------------------------------ */

#elif defined (_BOARD_MAX32_) || defined(_BOARD_MEGA_)

/* Push Buttons
*/
#define	PIN_BIO_BTN1	4
#define	PIN_BIO_BTN2	78
#define	PIN_BIO_BTN3	80
#define	PIN_BIO_BTN4	81

/* Slide Switches
*/
#define	PIN_BIO_SWT1	2
#define	PIN_BIO_SWT2	7
#define	PIN_BIO_SWT3	8
#define	PIN_BIO_SWT4	79

/* Discrete LEDs
*/
#define	PIN_BIO_LED1	77
#define	PIN_BIO_LED2	76
#define	PIN_BIO_LED3	75
#define	PIN_BIO_LED4	74
#define	PIN_BIO_LED5	73
#define	PIN_BIO_LED6	72
#define	PIN_BIO_LED7	71
#define	PIN_BIO_LED8	70

#define	PIN_SDCS	4

/* ------------------------------------------------------------ */
/*					Cerebot MX3cK With Pmods					*/
/* ------------------------------------------------------------ */

#elif defined(_BOARD_CEREBOT_MX3CK_)

#define	PIN_BIO_BTN1	4	// JA lower row of pins
#define	PIN_BIO_BTN2	5
#define	PIN_BIO_BTN3	6
#define	PIN_BIO_BTN4	7

#define	PIN_BIO_SWT1	12	// JB lower row of pins
#define	PIN_BIO_SWT2	13
#define	PIN_BIO_SWT3	14
#define	PIN_BIO_SWT4	15

#define	PIN_BIO_LED1	24	// JD
#define	PIN_BIO_LED2	25
#define	PIN_BIO_LED3	26
#define	PIN_BIO_LED4	27
#define	PIN_BIO_LED5	28
#define	PIN_BIO_LED6	29
#define	PIN_BIO_LED7	30
#define	PIN_BIO_LED8	31

#define	PIN_SDCS		16

/* ------------------------------------------------------------ */
/*					Cerebot MX4cK With Pmods					*/
/* ------------------------------------------------------------ */

#elif defined(_BOARD_CEREBOT_MX4CK_)

#define	PIN_BIO_BTN1	20	// JC lower row of pins
#define	PIN_BIO_BTN2	21
#define	PIN_BIO_BTN3	22
#define	PIN_BIO_BTN4	23

#define	PIN_BIO_SWT1	28	// JD lower row of pins
#define	PIN_BIO_SWT2	29
#define	PIN_BIO_SWT3	30
#define	PIN_BIO_SWT4	31

#define	PIN_BIO_LED1	0	// JA
#define	PIN_BIO_LED2	1
#define	PIN_BIO_LED3	2
#define	PIN_BIO_LED4	3
#define	PIN_BIO_LED5	4
#define	PIN_BIO_LED6	5
#define	PIN_BIO_LED7	6
#define	PIN_BIO_LED8	7

#define	PIN_SDCS		64

/* ------------------------------------------------------------ */
/*					Cerebot MX7cK With Pmods					*/
/* ------------------------------------------------------------ */

#elif defined(_BOARD_CEREBOT_MX7CK_)

#define	PIN_BIO_BTN1	12	// JB lower row of pins
#define	PIN_BIO_BTN2	13
#define	PIN_BIO_BTN3	14
#define	PIN_BIO_BTN4	15

#define	PIN_BIO_SWT1	20	// JC lower row of pins
#define	PIN_BIO_SWT2	21
#define	PIN_BIO_SWT3	22
#define	PIN_BIO_SWT4	23

#define	PIN_BIO_LED1	0	// JA
#define	PIN_BIO_LED2	1
#define	PIN_BIO_LED3	2
#define	PIN_BIO_LED4	3
#define	PIN_BIO_LED5	4
#define	PIN_BIO_LED6	5
#define	PIN_BIO_LED7	6
#define	PIN_BIO_LED8	7

#define	PIN_SDCS		SS

/* ------------------------------------------------------------ */
/*						Unknown Board							*/
/* ------------------------------------------------------------ */

#else

#error "System board not defined"

#endif

/* ------------------------------------------------------------ */
/*			Non-Board Specific Declarations						*/
/* ------------------------------------------------------------ */

#define PIN_BIO_POT A0

/* ------------------------------------------------------------ */

#endif

/************************************************************************/
