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

/*****************************************************************/
/*                                                               */
/* (C) Copyright 2008 by Networks In Motion, Inc.                */
/*                                                               */
/* The information contained herein is confidential, proprietary */
/* to Networks In Motion, Inc., and considered a trade secret as */
/* defined in section 499C of the penal code of the State of     */
/* California. Use of this information by anyone other than      */
/* authorized employees of Networks In Motion is granted only    */
/* under a written non-disclosure agreement, expressly           */
/* prescribing the scope and manner of such use.                 */
/*                                                               */
/*****************************************************************/

#include "windows.h"
#include "palstdlib.h"
#include "paltypes.h"
#include "paldisplay.h"
#include "palconfig.h"


#define DEFAULT_DPI 160.0
static float deviceDpi = DEFAULT_DPI;

/* See paldisplay.h */
PAL_Error
PAL_DisplayGetInfo(PAL_DisplayIndex displayIndex, uint32* width, uint32* height, uint32* dpi)
{
    *width = GetSystemMetrics(SM_CXSCREEN);
    *height = GetSystemMetrics(SM_CYSCREEN);
    *dpi = (uint32)deviceDpi;
    return PAL_Ok;
}

PAL_DEF double PAL_DisplayGetDensity()
{
    uint32 width = 0;
    uint32 height = 0;
    uint32 dpi = 0;
    PAL_DisplayGetInfo(PD_Main, &width, &height, &dpi);
    return static_cast<double>(dpi) / DEFAULT_DPI;
}

/* See paldisplay.h */
PAL_Error 
PAL_DisplayInitialize(PAL_Instance* pal)
{
    PAL_Config config = {0};
    PAL_ConfigGet(pal, &config);
    float dpi = config.dpi;
    if (dpi > 0)
    {
        deviceDpi = dpi;
    }
    return PAL_Ok;
}

/* See paldisplay.h */
uint32 
PAL_DisplayGetWidth()
{
    // "Screen size", for Windows we don't really need to make it full-screen. Just set it to some reasonable value for now.
    return 1200;
}

/* See paldisplay.h */
uint32 
PAL_DisplayGetHeight()
{
    // "Screen size", for Windows we don't really need to make it full-screen. Just set it to some reasonable value for now.
    return 800;
}
