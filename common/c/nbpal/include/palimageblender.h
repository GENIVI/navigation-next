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

@file palimageblender.h

@brief Provides a platform-independent image blender interface

*/
/*
    (C) Copyright 2012 by TeleCommunication Systems

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret
    as defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly prescribing
    the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

#ifndef PALIMAGEBLENDER_H
#define	PALIMAGEBLENDER_H

#include "pal.h"


/*! Image format */
typedef enum
{
    PAL_IF_Unknown,
    PAL_IF_ARGB32           /*!< The image is stored using a 32-bit ARGB format (0xAARRGGBB). */

} PAL_ImageFormat;


/*! Image structure */
typedef struct
{
    byte*  imageBuffer;     /*!< Buffer with image data. */
    uint32 imageBufferSize; /*!< imageBuffer size. */
    int    width;           /*!< Width of image. */
    int    height;          /*!< Height of image. */
    PAL_ImageFormat format; /*!< Image format. */
} PAL_Image;


/*! Creates a composite PNG image by merging the base and overlay images together
*/
PAL_DEC PAL_Error PAL_ImageBlend(PAL_Image* baseImage,        /*!< Pointer to the base image. */
                            PAL_Image* overlayImage,          /*!< Pointer to the overlay image. */
                            double overlayImageOffsetX,       /*!< Offset of the overlay image.
                                                              Coordinate system will be translated using this offset
                                                              before performing overlap. */
                            double overlayImageOffsetY,       /*!< Offset of the overlay image.
                                                              Coordinate system will be translated using this offset
                                                              before performing overlap. */
                            PAL_ImageFormat targetFormat,     /*!< Desired format of the composite image. */
                            PAL_Image** targetImage           /*!< Composite image on success, NULL otherwise. User is responsible to release memory
                                                              allocated for targetImage and targetImage->imageBuffer. Use nsl_free(). */
                            );

/*! Resizes an image (specifid by file path) and overwrites the original file.
    Currently image format assumed is PNG.
*/
PAL_DEC PAL_Error PAL_ResizeImage(const char* fullPath,     /*!< Full path of file to resize. */
                                  uint32 newWidth,          /*!< new width */
                                  uint32 newHeight          /*!< new height */
                                  );

#endif //PALIMAGEBLENDER_H

/*! @} */
