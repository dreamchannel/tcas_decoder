
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
  FILE        : selfcopy.h
  PROJECT     : DaVinci Serial boot and Flashing Utils
  AUTHOR      : Daniel Allred
  DESC        : Self-copy code used for XiP in NOR Flash
 ----------------------------------------------------------------------------- */

#ifndef _SELFCOPY_H_
#define _SELFCOPY_H_

#include "tistdtypes.h"

// Prevent C++ name mangling
#ifdef __cplusplus
extern far "c" {
#endif

/***********************************************************
* Global Macro Declarations                                *
***********************************************************/


/***********************************************************
* Global Typedef declarations                              *
***********************************************************/


/***********************************************************
* Global Function Declarations                             *
***********************************************************/

/*
 * UBL_selfcopy() has naked attribute (doesn't save registers since it is the entry
 * point when the UBL is found at the base of the NOR Flash and then goes 
 * directly to the the boot() function, which is also naked). This setup 
 * requires that the gnu compiler uses the -nostdlib option. 
 */
#if defined(__TMS470__)
  extern void selfcopy( void );
#elif defined(__GNUC__)
  extern void selfcopy( void ) __attribute__((naked,section(".selfcopy")));
#endif

/***********************************************************
* End file                                                 *
***********************************************************/

#ifdef __cplusplus
}
#endif

#endif //_UBL_H_

/* --------------------------------------------------------------------------- 
  HISTORY
 		v1.00  -  DJA  -  22-Jan-2007
			Initial Release 	   
    v1.10  -  DJA  -  01-Feb-2007
      Added the fakeentry point function prototype.
    v1.11  -  DJA  -  07-Mar-2007
      Version number update
    v1.12  -  DJA  -  15-Mar-2007
      NAND fix
		v1.13  -  DJA  -  04-Jun-2007
		  Major NAND revisions
    v1.20  -  DJA  -  07-Nov-2007
      Flash util consolidation, refactoring, and cleanup
 ----------------------------------------------------------------------------- */
