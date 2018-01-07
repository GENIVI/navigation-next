/*
Copyright (c) 2018, TeleCommunication Systems, Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
   * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the TeleCommunication Systems, Inc., nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, ARE
DISCLAIMED. IN NO EVENT SHALL TELECOMMUNICATION SYSTEMS, INC.BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*!--------------------------------------------------------------------------

@file paldisplay.h
@defgroup PALDISPLAY_H PAL Display Functions

@brief Provides a platform-dependent display information

*/
/*
(C) Copyright 2014 by TeleCommunication Systems, Inc.

The information contained herein is confidential, proprietary
to TeleCommunication Systems, Inc., and considered a trade secret as
defined in section 499C of the penal code of the State of
California. Use of this information by anyone other than
authorized employees of TeleCommunication Systems is granted only
under a written non-disclosure agreement, expressly
prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

#ifndef PALDISPLAY_H
#define PALDISPLAY_H

#include "pal.h"

typedef enum
{
    PD_Main = 0,      /*<! Main display index */
    PD_External = 1   /*<! External display index */
} PAL_DisplayIndex;


/* Get display information

This function returns the height and width of a display in pixels, and
the display resolution in dpi. Returns PAL_ErrNoData if the requested
display does not exist.

@return PAL_Error
*/
PAL_DEC PAL_Error PAL_DisplayGetInfo(
    PAL_DisplayIndex displayIndex,  /*<! Display index */
    uint32* width,                  /*<! Output width in pixels */
    uint32* height,                 /*<! Output height in pixels */
    uint32* dpi                     /*<! Output pixels per inch */
);

/* initialize display information

This function returns the result of initialization. Returns PAL_ErrNoData if initialize failed.

@return PAL_Error
*/
PAL_DEC PAL_Error PAL_DisplayInitialize(PAL_Instance *pal);

/* initialize display information

This function returns the width of display.
*/
PAL_DEC uint32 PAL_DisplayGetWidth();

/* get display height

This function returns the height of display.
*/
PAL_DEC uint32 PAL_DisplayGetHeight();

/* get display density

The logical density of the display. This is a scaling factor for the Density Independent Pixel unit, where one DIP is one pixel on an approximately 160 dpi screen (for example a 240x320, 1.5"x2" screen), providing the baseline of the system's display. Thus on a 160dpi screen this density value will be 1; on a 120 dpi screen it would be .75; etc
*/

PAL_DEC double PAL_DisplayGetDensity();

#endif

/*! @} */
