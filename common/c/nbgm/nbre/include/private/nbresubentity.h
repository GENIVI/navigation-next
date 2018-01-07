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

    @file nbresubentity.h
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
#ifndef _NBRE_SUB_ENTITY_H_
#define _NBRE_SUB_ENTITY_H_
#include "nbretypes.h"
#include "nbreshader.h"
#include "nbresubmesh.h"
#include "nbreshadermanager.h"
#include "nbrecontext.h"
#include "nbreitextureanimation.h"
#include "nbresubmodel.h"

class NBRE_IVertexAnimation;
/*! \addtogroup NBRE_Scene
*  @{
*/

/** Utility class which defines the sub-parts of an Entity.
@remarks
    Just as meshes are split into sub meshes, an Entity is made up of
    potentially multiple SubMeshes. These are mainly here to provide the
    link between the Material which the SubEntity uses (which may be the
    default Material for the SubMesh or may have been changed for this
    object) and the SubMesh data.
@par
    The SubEntity also allows the application some flexibility in the
    material properties for this section of a particular instance of this
    Mesh, e.g. tinting the windows on a car model.
@par
    SubEntity instances are never created manually. They are created at
    the same time as their parent Entity by the SceneManager method
    createEntity.
*/
class NBRE_SubEntity
{
friend class NBRE_Entity;

public:

    void SetShader(const NBRE_ShaderPtr& shader){ mShader = shader;}
    const NBRE_ShaderPtr& GetShader() const;
    void SetShaderName(const NBRE_String& name) { mShaderName = name; }

    nb_boolean Visible() const{ return mVisible;}

    void SetVisible(nb_boolean visable){ mVisible = visable;}

    void SetTextureAnimation(NBRE_ITextureAnimation* textureAnimation);

    const NBRE_ITextureAnimation* TextureAnimation() const;
    uint32 GetTextureAnimationLastUpdated() const;
    void SetTextureAnimationLastUpdated(uint32 number);

private:
    /// NBRE_SubEntity do not create directly, it can only be created by NBRE_Entity
    explicit NBRE_SubEntity(NBRE_Context& context, NBRE_SubModel* subModel);
    ~NBRE_SubEntity();
    NBRE_SubModel* GetSubModel() const { return mSubModel; }
    DISABLE_COPY_AND_ASSIGN(NBRE_SubEntity);

private:
    NBRE_Context& mContext;
    // Pointer to the SubModel
    NBRE_SubModel* mSubModel;

    /// Is this SubEntity visible?
    nb_boolean mVisible;

    /// subEntity shader information 
    mutable NBRE_ShaderPtr mShader;
    /// shader name follow this format type\name
    NBRE_String mShaderName;

    mutable NBRE_ShaderTimeStamp mShaderLastUpdated;

    /// Every pass will be modified by this chain before rendering
    NBRE_ITextureAnimation* mTextureAnimation;
    uint32 mTextureAnimationLastUpdated;
};

/*! @} */

#endif
