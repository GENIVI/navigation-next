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

@file paldisplay.mm
@defgroup PALDISPLAY_H PAL Display Functions

@brief Provides a platform-dependent display information

*/
/*
    (C) Copyright 2011 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret
    as defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly prescribing
    the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */


#include "paldisplay.h"
#include "palconfig.h"
#import "UIKit/UIKit.h"
#include "palstdlib.h"

static float deviceDpi = 160.0f;

// Not sure if this is correct?

PAL_DEF PAL_Error 
PAL_DisplayGetInfo(
                   PAL_DisplayIndex displayIndex,
                   uint32* width,
                   uint32* height,
                   uint32* dpi)
{
    if (!width || !height || !dpi)
    {
        return PAL_ErrBadParam;
    }

    switch (displayIndex)
    {
        case PD_Main:
        {
            UIScreenMode*		currentMode = [UIScreen mainScreen].currentMode;
            *width  = currentMode.size.width;
            *height = currentMode.size.height;
            *dpi    = (int)deviceDpi;
            return PAL_Ok;
        }

        case PD_External:
            return PAL_ErrNoData;
    }

    return PAL_ErrNoData;
}

PAL_DEF PAL_Error PAL_DisplayInitialize(PAL_Instance *pal)
{
    PAL_Config config;
    nsl_memset(&config, 0, sizeof(PAL_Config));
    PAL_ConfigGet(pal, &config);
    float dpi = config.dpi;
    if (dpi > 0)
    {
        deviceDpi = dpi;
    }
    return PAL_Ok;
}

PAL_DEF uint32 PAL_DisplayGetWidth()
{
    return -1;
}

PAL_DEF uint32 PAL_DisplayGetHeight()
{
    return -1;
}

PAL_DEF double PAL_DisplayGetDensity()
{
    return 0.0;
}

/*! @} */
