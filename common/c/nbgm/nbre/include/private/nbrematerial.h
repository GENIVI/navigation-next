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

    @file nbrematerial.h
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
#ifndef _NBRE_MATERIAL_H_
#define _NBRE_MATERIAL_H_
#include "nbrecolor.h"
#include "nbresharedptr.h"

/** \addtogroup NBRE_Resource
*  @{
*/
/** \addtogroup NBRE_Shader
*  @{
*/

/** Class encapsulates material properties of an object.
*/
class NBRE_Material
{
public:
    NBRE_Material(NBRE_Color diffuse, NBRE_Color ambient, NBRE_Color specular, NBRE_Color emissive, float shiness);
    NBRE_Material();
    ~NBRE_Material();

public:
    const NBRE_Color&   Ambient()  const  { return mAmbient; }
    NBRE_Color&         Ambient()         { return mAmbient; }
    const NBRE_Color&   Diffuse()  const  { return mDiffuse; }
    NBRE_Color&         Diffuse()         { return mDiffuse; }
    const NBRE_Color&   Specular() const  { return mSpecular; }
    NBRE_Color&         Specular()        { return mSpecular; }
    const NBRE_Color&   Emissive() const  { return mEmissive; }
    NBRE_Color&         Emissive()        { return mEmissive; }
    float               Shiness()  const  { return mShiness; }
    
    void SetDiffuse(const NBRE_Color& val)   { mDiffuse = val; }
    void SetAmbient(const NBRE_Color& val)   { mAmbient = val; }
    void SetSpecular(const NBRE_Color& val)  { mSpecular = val; }
    void SetEmissive(const NBRE_Color& val)  { mEmissive = val; }
    void SetShiness(float val)               { mShiness = val; }

private:
    NBRE_Color          mDiffuse;        /*!< Diffuse color */
    NBRE_Color          mAmbient;        /*!< Ambient color */
    NBRE_Color          mSpecular;       /*!< Specular 'shininess' */
    NBRE_Color          mEmissive;       /*!< Emissive color */
    float               mShiness;        /*!< Sharpness if specular highlight */
};

typedef shared_ptr<NBRE_Material> NBRE_MaterialPtr;
/** @} */
/** @} */
#endif
