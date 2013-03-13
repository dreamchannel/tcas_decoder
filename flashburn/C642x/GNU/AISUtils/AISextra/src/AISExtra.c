
/**************************************************************************
*                                                                         *
*   (C) Copyright 2008                                                    *
*   Texas Instruments, Inc.  <www.ti.com>                                 * 
*                                                                         *
*   See file CREDITS for list of people who contributed to this project.  *
*                                                                         *
**************************************************************************/


/**************************************************************************
*                                                                         *
*   This file is part of the DaVinci Flash and Boot Utilities.            *
*                                                                         *
*   This program is free software: you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation, either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful, but   *
*   WITHOUT ANY WARRANTY; without even the implied warranty of            *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
*   General Public License for more details.                              *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program.  if not, write to the Free Software          *
*   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307   *
*   USA                                                                   *
*                                                                         *
**************************************************************************/

/* --------------------------------------------------------------------------
  FILE        : AISExtra.c 				                             	 	        
  PROJECT     : TI Booting and Flashing Utilities
  AUTHOR      : Daniel Allred
  DESC        : AIS Extra commands for use in AIS Scripts
 ----------------------------------------------------------------------------- */

// General type include
#include "tistdtypes.h"
 
// Device module
#include "device.h"

// This module's header file
#include "AISExtra.h"

/************************************************************
* Explicit External Declarations                            *
************************************************************/


/************************************************************
* Local Macro Declarations                                  *
************************************************************/


/************************************************************
* Local Typedef Declarations                                *
************************************************************/


/************************************************************
* Local Function Declarations                               *
************************************************************/


/************************************************************
* Local Variable Definitions                                *
************************************************************/


/************************************************************
* Global Variable Definitions                               *
************************************************************/

#pragma DATA_SECTION(params,".params");
VUint32 params[32];

#pragma DATA_SECTION(seq,".data:uart");
const Uint8 seq[8] = {0x20,0x20,0x20,0x44,0x4F,0x4E,0x45};

/************************************************************
* Global Function Definitions                               *
************************************************************/

#pragma FUNC_EXT_CALLED(LPSCTransition)
void LPSCTransition()
{
	VUint32 module = params[0];
	VUint32 state  = params[1];
    while (PSC->PTSTAT & 0x00000001);
	PSC->MDCTL[module] = ((PSC->MDCTL[module]) & (0xFFFFFFE0)) | (state);
	PSC->PTCMD |= 0x00000001;
	while ((PSC->PTSTAT) & 0x00000001);
	while (((PSC->MDSTAT[module]) & 0x1F) != state);	
}

#pragma FUNC_EXT_CALLED(UARTSendDONE)
#pragma CODE_SECTION(UARTSendDONE,".text:uart")
void UARTSendDONE()
{
	Uint32 status,i,len = 8,timeOutCnt = 0;
	#define TIMEOUTMAX (0x01000000)

	for(i=0;i<len;i++)
	{
		/* Enable Timer one time */
		do{
			status = (UART0->LSR)&(0x20);
			timeOutCnt++;
		} while ((!status) && (timeOutCnt < TIMEOUTMAX) );

		if(timeOutCnt == TIMEOUTMAX)		
			return;
		
		// Send byte 
		(UART0->THR) = seq[i];
	}
}

/************************************************************
* End file                                                  *
************************************************************/

