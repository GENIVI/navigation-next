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

    @file nbgmpolyline2entity.h
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
#ifndef _NBGM_POLYLINE2_ENTITY_H_
#define _NBGM_POLYLINE2_ENTITY_H_
#include "nbretypes.h"
#include "nbrevector2.h"
#include "nbrepolyline2.h"
#include "nbrepass.h"
#include "nbrevisitor.h"
#include "nbremesh.h"
#include "nbrematrix4x4.h"
#include "nbrecontext.h"

class NBRE_Overlay;
class NBRE_VertexData;
class NBRE_IndexData;
/*! \addtogroup NBRE_Scene
*  @{
*/

/** 2d polyline entity
*/
class NBGM_Polyline2Entity:
    public NBRE_IRenderOperationProvider
{
    struct PolylineEntityVertex
    {
        NBRE_Vector2f position;
        NBRE_Color color;
    };

public:
    NBGM_Polyline2Entity(NBRE_Context& nbreContext, const NBRE_Polyline2f& polyline, const NBRE_Color& color);
    virtual ~NBGM_Polyline2Entity();

public:
    // NBRE_IRenderOperationProvider members
    virtual void NotifyCamera(const NBRE_Camera& camera);
    virtual void UpdateRenderQueue(NBRE_Overlay& overlay);
    virtual int32 OverlayId() const;
    virtual nb_boolean Visible() const;
    virtual void UpdateSurfaceList();
    /// Set transform
    void SetTransform(const NBRE_Matrix4x4f& m);
    /// Rebuild vertex buffer
    void Rebuild(const NBRE_Polyline2f& polyline);
    /// Set visibility
    void SetVisible(nb_boolean value) { mVisible = value; }

private:
    void UpdateVertexBuffer();

private:
    DISABLE_COPY_AND_ASSIGN(NBGM_Polyline2Entity);

private:
    NBRE_Context& mContext;
    NBRE_Polyline2f mPolyline;
    NBRE_VertexData* mVertexData;
    NBRE_IndexData* mIndexData;
    NBRE_Matrix4x4d mTransform;
    NBRE_PassPtr mPassCache;
    NBRE_Color mColor;
    nb_boolean mVisible;
};


/*! @} */
#endif
