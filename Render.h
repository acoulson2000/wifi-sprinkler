/************************************************************************/
/*																		*/
/*	Render.h	--	Interface Declarations for Render.cp				*/
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
/*																		*/
/************************************************************************/
/*  Revision History:													*/
/*																		*/
/*	05/28/2012(GeneA): created											*/
/*  09/01/2013(Andy Coulson): Modified for WiH2O						*/
/*																		*/
/************************************************************************/

#if !defined(_RENDER_H)
#define	_RENDER_H

/* ------------------------------------------------------------ */
/*					Miscellaneous Declarations					*/
/* ------------------------------------------------------------ */

#if defined(_BOARD_UNO32_) || defined(_BOARD_UNO_) || defined(_BOARD_CEREBOT_MX3CK_)
#define	cbFileBufMax	512
#else
#define	cbFileBufMax	512
#endif

/* ------------------------------------------------------------ */
/*					General Type Declarations					*/
/* ------------------------------------------------------------ */



/* ------------------------------------------------------------ */
/*					Object Class Declarations					*/
/* ------------------------------------------------------------ */



/* ------------------------------------------------------------ */
/*					Variable Declarations						*/
/* ------------------------------------------------------------ */



/* ------------------------------------------------------------ */
/*					Procedure Declarations						*/
/* ------------------------------------------------------------ */

void InitHttpRequest();
void PutResponseStart();
void PutResponseHeader();
void PutResponseBody();
int  ReadFileBuffer(uint8_t * rgbBuf, int cbReq);

/* ------------------------------------------------------------ */

#endif	// _RENDER_H

/************************************************************************/
