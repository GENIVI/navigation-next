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

    @file     paldisplay.c
    @date     02/10/2012
    @defgroup PALDISPLAY_H PAL Display Functions

    @brief    Provides a platform-dependent display information

*/
/*
    See file description in header file.

    (C) Copyright 2011 by Telecommunication Systems, Inc.

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
#include "palimpl.h"

//#include <QDesktopWidget>
//#include <QApplication>


#define SCREEN_NONE           (-1)
#define DEFAULT_DPI           (170)
#define INCHES_IN_MILLIMETERS (0.0393700787)

static float g_dpi = 1;
static uint32 g_DisplayWidth = 1;
static uint32 g_DisplayHeight = 1;
static bool g_displayInitialized = false;

PAL_DEF PAL_Error PAL_DisplayGetInfo(
    PAL_DisplayIndex displayIndex,  /*<! Display index */
    uint32* width,                  /*<! Output width in pixels */
    uint32* height,                 /*<! Output height in pixels */
    uint32* dpi                     /*<! Output pixels per inch */
)
{
    PAL_Error error = PAL_ErrNoInit;
    if(!g_displayInitialized)
    {
        return error;
    }

    error = PAL_ErrNoData;
    if(displayIndex == PD_Main)
    {
        *width = g_DisplayWidth;
        *height = g_DisplayHeight;
        *dpi = g_dpi;
        error = PAL_Ok;
    }

    return error;
}

PAL_DEF PAL_Error PAL_DisplayInitialize(PAL_Instance* pal)
{
//    QApplication* pApplication = qobject_cast<QApplication*>(QCoreApplication::instance());
//    if (pApplication != NULL)
//    {
//        QDesktopWidget* desktop = pApplication->desktop();
//        if(desktop)
//        {
//            QRect rect = desktop->screenGeometry();
//            g_DisplayWidth = rect.size().width();
//            g_DisplayHeight = rect.size().height();
//            g_dpi = pal->config.dpi;
//            g_displayInitialized = true;
//        }
//    }

    g_DisplayWidth = 1280;
    g_DisplayHeight = 720;
    g_dpi = pal->config.dpi;
    g_displayInitialized = true;

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

