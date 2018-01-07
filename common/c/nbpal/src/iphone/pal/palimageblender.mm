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

@file palimageblender.mm

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

#import "palimageblender.h"
#import <UIKit/UIKit.h>
#import "palstdlib.h"


PAL_DEF PAL_Error
PAL_ImageBlend(PAL_Image* baseImage,
               PAL_Image* overlayImage,
               double overlayImageOffsetX,
               double overlayImageOffsetY,
               PAL_ImageFormat targetFormat,
               PAL_Image** targetImage
               )
{
    if (baseImage == NULL || overlayImage == NULL || targetImage == NULL ||
        baseImage->imageBuffer == NULL || baseImage->imageBufferSize == 0 ||
        overlayImage->imageBuffer == NULL || overlayImage->imageBufferSize == 0)
    {
        return PAL_ErrBadParam;
    }

    @autoreleasepool
    {
        NSData* baseImageData = [NSData dataWithBytesNoCopy:baseImage->imageBuffer length:baseImage->imageBufferSize freeWhenDone:NO];
        if (baseImageData == nil)
        {
            return PAL_ErrBadParam;
        }

        UIImage* baseUIImage = [UIImage imageWithData:baseImageData];
        if (baseUIImage == nil)
        {
            return PAL_ErrBadParam;
        }

        NSData* overlayImageData = [NSData dataWithBytesNoCopy:overlayImage->imageBuffer length:overlayImage->imageBufferSize freeWhenDone:NO];
        if (overlayImageData == nil)
        {
            return PAL_ErrBadParam;
        }

        UIImage* overlayUIImage = [UIImage imageWithData:overlayImageData];
        if (overlayUIImage == nil)
        {
            return PAL_ErrBadParam;
        }

        UIGraphicsBeginImageContext(baseUIImage.size);

        [baseUIImage drawAtPoint:CGPointZero];
        [overlayUIImage drawAtPoint:CGPointMake(overlayImageOffsetX, overlayImageOffsetY)];

        UIImage* targetUIImage = UIGraphicsGetImageFromCurrentImageContext();

        UIGraphicsEndImageContext();

        if (targetUIImage == nil)
        {
            return PAL_ErrNoMem; 
        }

        NSData* targetImageData = UIImagePNGRepresentation(targetUIImage);
        if (targetImageData == nil || [targetImageData bytes] == nil || [targetImageData length] == 0)
        {
            return PAL_ErrNoMem;
        }

        PAL_Image* targetPalImage = (PAL_Image*)nsl_malloc(sizeof(PAL_Image));
        if (targetPalImage == NULL)
        {
            return PAL_ErrNoMem;
        }

        nsl_memset(targetPalImage, 0, sizeof(PAL_Image));

        targetPalImage->imageBuffer = (byte*)nsl_malloc([targetImageData length]);
        if (targetPalImage->imageBuffer == NULL)
        {
            nsl_free(targetPalImage);
            targetPalImage = NULL;
            return PAL_ErrNoMem;
        }

        nsl_memcpy(targetPalImage->imageBuffer, [targetImageData bytes], [targetImageData length]);

        targetPalImage->imageBufferSize = [targetImageData length];
        targetPalImage->format = PAL_IF_Unknown;
        targetPalImage->width = targetUIImage.size.width;
        targetPalImage->height = targetUIImage.size.height;

        *targetImage = targetPalImage;
        targetPalImage = NULL;
    }

    return PAL_Ok;
}

PAL_DEF PAL_Error
PAL_ResizeImage(const char* fullPath, uint32 newWidth, uint32 newHeight)
{
    if (fullPath == NULL || newWidth == 0 || newHeight == 0)
    {
        return PAL_ErrBadParam;
    }
    @autoreleasepool
    {
        NSString* filePath = [NSString stringWithUTF8String:fullPath];
        UIImage* baseUIImage = [UIImage imageWithContentsOfFile:filePath];
        if (baseUIImage == nil)
        {
            return PAL_ErrBadParam;
        }
        CGSize newSize;
        newSize.width = newWidth;
        newSize.height = newHeight;
        UIGraphicsBeginImageContext(newSize);
        CGRect rect;
        rect.size = newSize;
        rect.origin.x = rect.origin.y = 0;
        [baseUIImage drawInRect:rect];
        UIImage* targetUIImage = UIGraphicsGetImageFromCurrentImageContext();
        UIGraphicsEndImageContext();
        NSData* targetImageData = UIImagePNGRepresentation(targetUIImage);
        if (targetImageData == nil || [targetImageData bytes] == nil || [targetImageData length] == 0)
        {
            return PAL_ErrNoMem;
        }
        if (![targetImageData writeToFile:filePath atomically:YES])
        {
            return PAL_ErrFileFailed;
        }
    }
    return PAL_Ok;
}

/*! @} */
