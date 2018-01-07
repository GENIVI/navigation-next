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

/*--------------------------------------------------------------------------

(C) Copyright 2012 by Networks In Motion, Inc.

The information contained herein is confidential, proprietary
to Networks In Motion, Inc., and considered a trade secret as
defined in section 499C of the penal code of the State of
California. Use of this information by anyone other than
authorized employees of Networks In Motion is granted only
under a written non-disclosure agreement, expressly
prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
#include "nbgmlayoutelement.h"
#include "nbgmlayoutmanager.h"
#include "nbremath.h"
#include "palclock.h"

static inline uint32 Now()
{
    return PAL_ClockGetTimeMs();
}


const uint32 NBGM_LayoutElement::INVALID_DRAW_INDEX = NBRE_INVALID_INDEX;
const NBRE_String NBGM_LayoutElement::EMPTY_LAYER_ID = "";

NBGM_LayoutElement::NBGM_LayoutElement(NBGM_Context& nbgmContext)
    :mNBGMContext(nbgmContext)
    ,mPriority(0)
    ,mMinPixelPerUnit(0)
    ,mSquaredDistanceToEye(NBRE_Math::Infinity)
    ,mVisibleMs(0)
    ,mDrawIndex(INVALID_DRAW_INDEX)
    ,mEnable(TRUE)
    ,mUpdateTime(Now())
    ,mLastLayoutTime(0)
    ,mTypeId(NBGM_LET_UNKNOWN)
    ,mEnableLayout(TRUE)
{
}

NBGM_LayoutElement::NBGM_LayoutElement(NBGM_Context& nbgmContext, const NBRE_WString& text)
    :mNBGMContext(nbgmContext)
    ,mPriority(0)
    ,mText(text)
    ,mMinPixelPerUnit(0)
    ,mSquaredDistanceToEye(NBRE_Math::Infinity)
    ,mVisibleMs(0)
    ,mDrawIndex(INVALID_DRAW_INDEX)
    ,mEnable(TRUE)
    ,mUpdateTime(Now())
    ,mLastLayoutTime(0)
    ,mTypeId(NBGM_LET_UNKNOWN)
    ,mEnableLayout(TRUE)
{
}

NBGM_LayoutElement::~NBGM_LayoutElement()
{
    mNBGMContext.layoutManager->RemoveElement(this);
}

void NBGM_LayoutElement::AddToSence()
{
}

void NBGM_LayoutElement::RemoveFromVisibleSet()
{
    mDrawIndex = INVALID_DRAW_INDEX;
    mVisibleMs = 0;
    SetVisible(FALSE);
    mNBGMContext.layoutManager->RemoveElement(this);
}

void NBGM_LayoutElement::SetEnable(nb_boolean value)
{   
    if (value != mEnable)
    {
        mEnable = value;
        mUpdateTime = Now();
        if (!value)
        {
            RemoveFromVisibleSet();
        }
    }
}

void NBGM_LayoutElement::InvalidateLayout()
{
    mUpdateTime = Now();
}
