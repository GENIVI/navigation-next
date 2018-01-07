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

    @file nbrelight.h
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
#ifndef _NBRE_LIGHT_H_
#define _NBRE_LIGHT_H_
#include "nbretypes.h"
#include "nbrecolor.h"
#include "nbrevector3.h"

enum NBRE_LightType
{
    /// Point light sources give off light equally in all directions, so require only position not direction
    NBRE_LT_POINT = 0,
    /// Directional lights simulate parallel light beams from a distant source, hence have direction but no position
    NBRE_LT_DIRECTIONAL = 1,
    /// Spotlights simulate a cone of light from a source so require position and direction, plus extra values for falloff
    NBRE_LT_SPOTLIGHT = 2
};

/** Representation of a dynamic light source in the scene.
*/
class NBRE_Light
{
public:
    NBRE_Light():
        mType(NBRE_LT_POINT),
        mPosition(0, 0, 0),
        mDirection(0, 0, 1),
        mAmbient(0, 0, 0, 1),
        mDiffuse(1, 1, 1, 1),
        mSpecular(0, 0, 0, 1),
        mExponent(0),
        mCutoff(180.0f),
        mRange(1)
        {
        }
    ~NBRE_Light() {}

public:
    NBRE_LightType GetType() const { return mType; }
    void SetType(NBRE_LightType val) { mType = val; }
    const NBRE_Vector3f& GetPosition() const { return mPosition; }
    void SetPosition(const NBRE_Vector3f& val) { mPosition = val; }
    const NBRE_Vector3f& GetDirection() const { return mDirection; }
    void SetDirection(const NBRE_Vector3f& val) { mDirection = val; }
    const NBRE_Color& GetAmbient() const { return mAmbient; }
    void SetAmbient(const NBRE_Color& val) { mAmbient = val; }
    const NBRE_Color& GetDiffuse() const { return mDiffuse; }
    void SetDiffuse(const NBRE_Color& val) { mDiffuse = val; }
    const NBRE_Color& GetSpecular() const { return mSpecular; }
    void SetSpecular(const NBRE_Color& val) { mSpecular = val; }
    float GetExponent() const { return mExponent; }
    void SetExponent(float val) { mExponent = val; }
    float GetCutoff() const { return mCutoff; }
    void SetCutoff(float val) { mCutoff = val; }
    float GetRange() const { return mRange; }
    void SetRange(float val) { mRange = val; }
private:
    NBRE_LightType mType;
    NBRE_Vector3f mPosition;
    NBRE_Vector3f mDirection;
    NBRE_Color mAmbient;
    NBRE_Color mDiffuse;
    NBRE_Color mSpecular;
    float mExponent;
    float mCutoff;
    float mRange;
};

typedef NBRE_List<NBRE_Light*> NBRE_LightList;
typedef NBRE_Vector<uint32> NBRE_LightIndexList;

#endif
