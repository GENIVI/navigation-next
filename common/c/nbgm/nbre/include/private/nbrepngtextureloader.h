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

    @file nbrepngtextureloader.h
*/
/*
    (C) Copyright 2012 by Networks In Motion, Inc.

    The information contained herein is confidential, proprietary
    to Networks In Motion, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
#ifndef _NBRE_PNG_TEXTURE_LOADER_H_
#define _NBRE_PNG_TEXTURE_LOADER_H_
#include "nbreitextureloader.h"
#include "nbrecommon.h"

/** \addtogroup NBRE_Resource
*  @{
*/
/** \addtogroup NBRE_Image
*  @{
*/
class NBRE_Image;
class NBRE_PngCodec;

//! A simple png texture loader.
/** Using renderpal to generate mipmaps.
*/
class NBRE_PngTextureLoader: public NBRE_ITextureLoader
{
public:
    NBRE_PngTextureLoader(const char* path, uint32 offset, nb_boolean isUsingMipmaps);
    virtual ~NBRE_PngTextureLoader();

public:
    /// Is texture using mipmaps?
    virtual nb_boolean IsUsingMipmaps() const          { return mIsUsingMipmaps; }
    /// Do not manually generate mipmaps
    virtual nb_boolean IsManualGenerateMipmaps() const { return FALSE; }
    /// Get mipmaps levels
    virtual uint32 GetNumMipmaps() const               { return 1; }
    /// Get images of mipmap level, only level 0 is supported
    virtual NBRE_Image** GetImages();

private:
    DISABLE_COPY_AND_ASSIGN(NBRE_PngTextureLoader);

protected:
    nb_boolean mIsUsingMipmaps;
    NBRE_PngCodec* mImageCodec;
};

/** @} */
/** @} */
#endif