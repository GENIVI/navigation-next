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

    @file nbresubmodel.h
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

#ifndef _NBRE_SUBMODEL_H_
#define _NBRE_SUBMODEL_H_

#include "nbresubmesh.h"
#include "nbreshader.h"

/*! Defines an instance of a discrete object included NBRE_Model
@remarks
    This class contains the independent vertext data and shader data to
    describe part of a NBRE_Model. If NBRE_SubModel used shared shader, it
    only keeps name of it, otherwise it keeps the independent shader in 
    itself. Sometimes some similar NBRE_SubModels have the same vertex 
    data, so it may use shared vertex data in NBRE_Mesh.
*/ 

class NBRE_SubModel
{
    friend class NBRE_Model;

public:
    void SetShader(const NBRE_ShaderPtr& shader){ mShader = shader;}
    const NBRE_ShaderPtr& GetShader() const{ return mShader; }
    void SetShaderName(const NBRE_String& name) { mShaderName = name; }
    const NBRE_String& ShaderName() const { return mShaderName; }

    NBRE_SubMesh* GetSubMesh() { return mSubMesh; }

private:
    /// NBRE_SubModel do not create directly, it can only be created by NBRE_Model
    explicit NBRE_SubModel(NBRE_SubMesh* subMesh);
    ~NBRE_SubModel();
    DISABLE_COPY_AND_ASSIGN(NBRE_SubModel);

private:
    /// Vertex information
    NBRE_SubMesh* mSubMesh;
    /// shader information 
    mutable NBRE_ShaderPtr mShader;
    /// shader name
    NBRE_String mShaderName;
};

typedef NBRE_Vector<NBRE_SubModel*> NBRE_SubModelList;

#endif