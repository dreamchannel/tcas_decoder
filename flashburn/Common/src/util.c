
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
  FILE        : util.c                                                   
  PURPOSE     : Utility and misc. file
  PROJECT     : Flashing Utilities
  AUTHOR      : Daniel Allred
  DESC        : Utility functions for flashing applications
 ----------------------------------------------------------------------------- */

// General type include
#include "tistdtypes.h"

// This module's header file
#include "util.h"

// Device specific CSL
#include "device.h"


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

static Uint32 LOCAL_reflectNum(Uint32 inVal, Uint32 num);


/************************************************************
* Local Variable Definitions                                *
************************************************************/


/************************************************************
* Global Variable Definitions                               *
************************************************************/

// Global memory allocation pointer
static VUint32 currMemPtr;


/************************************************************
* Global Function Definitions                               *
************************************************************/

// DDR Memory allocation routines (for storing large data)
void *UTIL_getCurrMemPtr(void)
{
  return ((void *)currMemPtr);
}

// Setup for an adhoc heap
void UTIL_setCurrMemPtr(void *value)
{
  currMemPtr = (VUint32)value;
}

// Allocate memory from the ad-hoc heap
void *UTIL_allocMem(Uint32 size)
{
  void *cPtr;
  Uint32 size_temp;

  // Ensure word boundaries
  size_temp = ((size + 4) >> 2 ) << 2;
  
  if((currMemPtr + size_temp) > (DEVICE_DDR2_RAM_SIZE))
  {
    return NULL;
  }

  cPtr = (void *) (DEVICE_DDR2_START_ADDR + currMemPtr);
  currMemPtr += size_temp;

  return cPtr;
}

// Simple wait loop - comes in handy.
void UTIL_waitLoop(Uint32 loopcnt)
{
  Uint32 i;
  for (i = 0; i<loopcnt; i++)
  {
    asm("   NOP");
  }
}

// Accurate n = ((t us * f MHz) - 5) / 1.65 
void UTIL_waitLoopAccurate (Uint32 loopcnt)
{
#if defined(_TMS320C6X)
//  asm ("      SUB     B15, 8, B15     ");                 // Done by compiler 
//  asm ("      STW     A4, *+B15[1]    ");                 // Done by compiler 
  asm ("      STW     B0, *+B15[2]    ");
  asm ("      SUB     A4, 24, A4      ");          // Total cycles taken by this function, with n = 0, including clocks taken to jump to this function 
  asm ("      CMPGT   A4, 0, B0       ");
  asm ("loop:                         ");
  asm (" [B0] B       loop            ");
  asm (" [B0] SUB     A4, 6, A4       ");          // Cycles taken by loop 
  asm ("      CMPGT   A4, 0, B0       ");
  asm ("      NOP     3               ");
  asm ("      LDW     *+B15[2], B0    ");
//  asm ("      B       B3              ");                 // Done by compiler 
//  asm ("      ADD     B15, 8, B15     ");                 // Done by compiler 
//  asm ("      NOP     4     
#elif defined(_TMS320C5XX) || defined(__TMS320C55X__)
#elif defined(_TMS320C28X)
#elif (defined(__TMS470__) || defined(__GNUC__))
#endif
}

// CRC-32 routine (relflected, init xor val = 0xFFFFFFFF, final xor val = 0xFFFFFFFF)
Uint32 UTIL_calcCRC32(Uint8* lutCRC, Uint8 *data, Uint32 size, Uint32 currCRC)
{
  unsigned int len = size;
  Uint32 crc = currCRC;
  int i;

  // Prepare input to get back into calculation state ( this means initial
  // input when starting fresh should be 0x00000000 )
  crc = currCRC ^ 0xFFFFFFFF;

  // Perform the algorithm on each byte
  for (i = 0; i < len; i++)
  {
    crc = (crc >> 8) ^ lutCRC[(crc & 0xFF) ^ data[i]];
  }

  // Exclusive OR the result with the specified value
  crc = crc ^ 0XFFFFFFFF;

  return crc;
}

// Function to build a reflected CRC-32 table (for standard CRC-32 algorithm)
// lutCRC must have 1024 bytes (256 32-bit words) available
void UTIL_buildCRC32Table(Uint32* lutCRC, Uint32 poly)
{
  Uint32 i,j,tableLen = 256;
  Uint32 crcAccum;
  
  for (i = 0; i < tableLen; i++)
  {
    crcAccum = LOCAL_reflectNum(i, 8) << (32 - 8);
    for (j = 0; j < 8; j++)
    {
      if ((crcAccum & 0x80000000) != 0x00000000)
      {
        crcAccum = (crcAccum << 1) ^ poly;
      }
      else
      {
        crcAccum = (crcAccum << 1);
      }
      lutCRC[i] = LOCAL_reflectNum(crcAccum, 32);
    }
  }
}

// CRC-16 routine (relflected, init xor val = 0xFFFF, final xor val = 0xFFFF)
Uint16 UTIL_calcCRC16(Uint8* lutCRC, Uint8 *data, Uint32 size, Uint16 currCRC)
{
  unsigned int len = size;
  Uint16 crc = currCRC;
  int i;

  // Prepare input to get back into calculation state ( this means initial
  // input when starting fresh should be 0x00000000 )
  crc = currCRC ^ 0xFFFF;

  // Perform the algorithm on each byte
  for (i = 0; i < len; i++)
  {
    crc = (crc >> 8) ^ lutCRC[(crc & 0xFF) ^ data[i]];
  }

  // Exclusive OR the result with the specified value
  crc = crc ^ 0xFFFF;

  return crc;
}

// Function to build a reflected CRC-16 table (for standard CRC-16 algorithm)
// lutCRC must have 512 bytes (256 16-bit words) available
void UTIL_buildCRC16Table(Uint16* lutCRC, Uint16 poly)
{
  Uint32 i,j,tableLen = 256;
  Uint16 crcAccum;
  
  for (i = 0; i < tableLen; i++)
  {
    crcAccum = (Uint16) LOCAL_reflectNum(i, 8) << (16 - 8);
    for (j = 0; j < 8; j++)
    {
      if ((crcAccum & 0x8000) != 0x0000)
      {
        crcAccum = (crcAccum << 1) ^ poly;
      }
      else
      {
        crcAccum = (crcAccum << 1);
      }
      lutCRC[i] = (Uint16) LOCAL_reflectNum(crcAccum, 16);
    }
  }
}


/***********************************************************
* Local Function Definitions                               *
***********************************************************/
static Uint32 LOCAL_reflectNum(Uint32 inVal, Uint32 num)
{
  Uint32 i,outVal = 0x0;

  for (i = 1; i < (num + 1); i++)
  {
    outVal |= (Uint32)( ((inVal & 0x1)) << (num - i));
    inVal >>= 1;
  }
  return outVal;
}


/***********************************************************
* End file                                                 *
***********************************************************/

/* --------------------------------------------------------------------------
    HISTORY
        v1.00  -  DJA  -  16-Aug-2007
           Initial release 
 ----------------------------------------------------------------------------- */



