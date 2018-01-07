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

    @file nbrelightsource.h
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
#ifndef _NBRE_LIGHT_NODE_H_
#define _NBRE_LIGHT_NODE_H_
#include "nbretypes.h"
#include "nbrenode.h"
#include "nbrelight.h"
#include "nbrevisitor.h"

/** \addtogroup NBRE_RenderEngine
*  @{
*/
//! Light node class.
/** Before rendering scene, all light in the scene will be
collected to scenemanager, in world space.
*/
///TODO: change to
///class NBRE_LightSource: public NBRE_Node, public NBRE_ILightProvider
class NBRE_LightSource: public NBRE_Node, public NBRE_ILightProvider
{
public:
    NBRE_LightSource() {}
    virtual ~NBRE_LightSource() {}

public:
    ///From NBRE_Node
    virtual void UpdatePostion();
    virtual void Accept(NBRE_Visitor& visitor);

    ///From NBRE_ILightProvider
    virtual void UpdateLights(NBRE_LightList& lights);


    NBRE_LightType GetType() const { return mLightAbsolute.GetType(); }
    const NBRE_Vector3f& GetPosition() const { return mLightAbsolute.GetPosition(); }
    const NBRE_Vector3f& GetDirection() const { return mLightAbsolute.GetDirection(); }
    const NBRE_Color& GetAmbient() const { return mLightAbsolute.GetAmbient(); }
    const NBRE_Color& GetDiffuse() const { return mLightAbsolute.GetDiffuse(); }
    const NBRE_Color& GetSpecular() const { return mLightAbsolute.GetSpecular(); }
    float GetExponent() const { return mLightAbsolute.GetExponent(); }
    float GetCutoff() const { return mLightAbsolute.GetCutoff(); }

    void SetType(NBRE_LightType val) { mLightAbsolute.SetType(val); }
    void SetAmbient(const NBRE_Color& val) { mLightAbsolute.SetAmbient(val); }
    void SetDiffuse(const NBRE_Color& val) { mLightAbsolute.SetDiffuse(val); }
    void SetSpecular(const NBRE_Color& val) { mLightAbsolute.SetSpecular(val); }
    void SetExponent(float val) { mLightAbsolute.SetExponent(val); }
    void SetCutoff(float val) { mLightAbsolute.SetCutoff(val); }

private:
    /// light transform to world space
    NBRE_Light mLightAbsolute;
};

/*! @} */
#endif
