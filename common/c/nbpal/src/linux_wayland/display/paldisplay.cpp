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

    @file     paldisplay.cpp
    @date     07/04/2014
    @defgroup PALDISPLAY_H PAL Display Functions

    @brief    Provides a platform-dependent display information

*/
/*
    See file description in header file.

    (C) Copyright 2014 by Telecommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to Telecommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Telecommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#include "paldisplay.h"
#include "palmath.h"
#include "palstdlib.h"
#include "paltestlog.h"

#define WAYLAND_PIX_WIDTH       1280
#define WAYLAND_PIX_HEIGHT      720
#define WAYLAND_DPI             96
#define SCREEN_NONE           (-1)
#define DEFAULT_DPI           (170)

uint32 g_DisplayWidth = 0;
uint32 g_DisplayHeight = 0;

/* Get dots per inch
 */
static PAL_Error GetDPIAndWidthHeight( uint32* pixwidth,uint32* pixheight,uint32* dpi,const int scridx)
{
    /*todo:Wayland is now in the heavy development,there is no API which is similar to X11 to get the accurate display width, height and dpi.
      So that use the fixed value to set them.
    */
    if (!pixwidth || !pixheight || !dpi)
    {
        return PAL_ErrBadParam;
    }

    *pixwidth = WAYLAND_PIX_WIDTH;
    *pixheight = WAYLAND_PIX_HEIGHT;
    *dpi = WAYLAND_DPI;
    return PAL_Ok;
}


PAL_DEF PAL_Error PAL_DisplayGetInfo(
    PAL_DisplayIndex displayIndex,  /*<! Display index */
    uint32* width,                  /*<! Output width in pixels */
    uint32* height,                 /*<! Output height in pixels */
    uint32* dpi                     /*<! Output pixels per inch */
)
{
    int displays_count = 0;
    int32 screenIndex = SCREEN_NONE;
    PAL_Error error = PAL_ErrNoData;

    if (!width || !height || !dpi)
    {
        return PAL_ErrBadParam;
    }

    switch (displayIndex)
    {
        case PD_Main:
            screenIndex = 0;
            break;
        case PD_External:
            if (displays_count > 1)
            {
                screenIndex = 1;
            }
            break;
        default:
            break;
    }

    if (screenIndex != SCREEN_NONE)
    {
        error = GetDPIAndWidthHeight(width,height,dpi,screenIndex);
    }

    return error;
}

PAL_DEF PAL_Error PAL_DisplayInitialize()
{
    uint32 width = 0;
    uint32 height = 0;
    uint32 dpi = 0;
    PAL_Error result = PAL_ErrNoData;

    result = PAL_DisplayGetInfo(PD_Main, &width, &height, &dpi);
    if ( result != PAL_Ok )
    {
        NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelError, "get width and height fail!");

        return result;
    }

    if ((g_DisplayWidth != width) || (g_DisplayHeight != height))
    {
        NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelInfo, "update width %d height %d dpi %d", width, height, dpi);
        g_DisplayWidth = width;
        g_DisplayHeight = height;
    }


    return PAL_Ok;
}

PAL_DEF uint32 PAL_DisplayGetWidth()
{
    return g_DisplayWidth;
}

PAL_DEF uint32 PAL_DisplayGetHeight()
{
    return g_DisplayHeight;
}
PAL_DEF double PAL_DisplayGetDensity()
{
    uint32 width = 0;
    uint32 height = 0;
    uint32 dpi = 0;
    PAL_DisplayGetInfo(PD_Main, &width, &height, &dpi);
    return (double)dpi / DEFAULT_DPI;
}

/*! @} */

