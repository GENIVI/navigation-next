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

    @file     palimageblender.c

*/
/*
    (C) Copyright 2012 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunications Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunications Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

#include "palimageblender.h"
#include "palstdlib.h"

PAL_DEF PAL_Error PAL_ImageBlend(PAL_Image* baseImage, PAL_Image* overlayImage, double overlayImageOffsetX, double overlayImageOffsetY,
							     PAL_ImageFormat targetFormat, PAL_Image** targetImage)
{
    PAL_Image* pImage = NULL;

    if (baseImage == NULL || overlayImage == NULL || targetImage == NULL ||
        baseImage->imageBuffer == NULL || baseImage->imageBufferSize == 0 ||
        overlayImage->imageBuffer == NULL || overlayImage->imageBufferSize == 0)
    {
        return PAL_ErrBadParam;
    }

    // todo - implement functionality to blend two images
    // now just temp copy first image to targetImage
    pImage = (PAL_Image*)nsl_malloc(sizeof(PAL_Image));
    if (pImage == NULL)
    {
        return PAL_ErrNoMem;
    }
    pImage->imageBuffer = (byte*)nsl_malloc(baseImage->imageBufferSize);
    if (pImage->imageBuffer == NULL)
    {
        return PAL_ErrNoMem;
    }

    pImage->imageBufferSize = baseImage->imageBufferSize;
    nsl_memcpy(pImage->imageBuffer, baseImage->imageBuffer, pImage->imageBufferSize);
    pImage->width = baseImage->width;
    pImage->height = baseImage->height;

    *targetImage = pImage;

    return PAL_Ok;
}

/*! @} */