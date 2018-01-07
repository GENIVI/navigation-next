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

    @file nbretexturestate.h
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
#ifndef _NBRE_TEXTURE_STATE_H_
#define _NBRE_TEXTURE_STATE_H_
#include "nbrerendercommon.h"

/*! \addtogroup NBRE_RenderPal
*  @{
*/

class NBRE_TextureState
{
public:
    NBRE_TextureState():mMinFilter(NBRE_TFT_LINEAR), mMagFilter(NBRE_TFT_LINEAR), mWrapS(NBRE_TAM_REPEAT), mWrapT(NBRE_TAM_REPEAT), mUsingAnisotropic(FALSE){}
    NBRE_TextureState(NBRE_TextureFilterType minFilter, NBRE_TextureFilterType magFilter,NBRE_TextureAddressMode wrapS,NBRE_TextureAddressMode wrapT, nb_boolean usingAnisotropic)
        :mMinFilter(minFilter), mMagFilter(magFilter), mWrapS(wrapS), mWrapT(wrapT), mUsingAnisotropic(usingAnisotropic) {}
    ~NBRE_TextureState(){}

public:
    NBRE_TextureFilterType GetMinFilter() const { return mMinFilter; }
    void SetMinFilter(NBRE_TextureFilterType val) { mMinFilter = val; }
    NBRE_TextureFilterType GetMagFilter() const { return mMagFilter; }
    void SetMagFilter(NBRE_TextureFilterType val) { mMagFilter = val; }
    NBRE_TextureAddressMode GetWrapS() const { return mWrapS; }
    void SetWrapS(NBRE_TextureAddressMode val) { mWrapS = val; }
    NBRE_TextureAddressMode GetWrapT() const { return mWrapT; }
    void SetWrapT(NBRE_TextureAddressMode val) { mWrapT = val; }

    nb_boolean UsingAnisotropic() const { return mUsingAnisotropic; }
    void SetUsingAnisotropic(nb_boolean val) { mUsingAnisotropic = val; }

    bool operator == (const NBRE_TextureState& ts) const { return mMinFilter == ts.mMinFilter && mMagFilter == ts.mMagFilter && mWrapS == ts.mWrapS && mWrapT == ts.mWrapT; }
    bool operator != (const NBRE_TextureState& ts) const { return !(*this == ts);}

private:
    NBRE_TextureFilterType mMinFilter;
    NBRE_TextureFilterType mMagFilter;
    NBRE_TextureAddressMode mWrapS;
    NBRE_TextureAddressMode mWrapT;

    nb_boolean mUsingAnisotropic;
};

/** @} */
#endif
