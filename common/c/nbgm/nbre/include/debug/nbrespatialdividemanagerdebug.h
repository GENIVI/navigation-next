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

    @file nbrespatialdividemanagerdebug.h
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
#ifndef _NBRE_SPATIAL_DIVIDE_MANAGER_DEBUG_H_
#define _NBRE_SPATIAL_DIVIDE_MANAGER_DEBUG_H_
#include "nbretypes.h"
#include "nbrerenderoperation.h"
#include "nbrevisitor.h"
#include "nbrecontext.h"
#include "nbredebugboundingbox.h"

class NBRE_DefaultSpatialDivideManager;
class NBRE_BinaryZone;
class NBRE_DebugBoundingBox;

/*! \addtogroup NBRE_Debug
*  @{
*/
class NBRE_SpatialDivideManagerDebug: public NBRE_IVisitable, public NBRE_IRenderOperationProvider
{
public:
    NBRE_SpatialDivideManagerDebug(NBRE_Context& context, NBRE_DefaultSpatialDivideManager& spatialMgr, int32 overlayId);
    ~NBRE_SpatialDivideManagerDebug();

public:

private:
    DISABLE_COPY_AND_ASSIGN(NBRE_SpatialDivideManagerDebug);

public:
    void SetVisible(nb_boolean visible);

    void SetOnlyShowHasObj(nb_boolean show);

    ///From NBRE_IVisitable
    virtual void Accept(NBRE_Visitor& visitor);

    ///From NBRE_RenderOperationProvider
    virtual void NotifyCamera(const NBRE_Camera& camera);
    virtual void UpdateRenderQueue(NBRE_Overlay& overlay);
    virtual int32 OverlayId() const;
    virtual nb_boolean Visible() const;
    virtual void UpdateSurfaceList() {};
    virtual uint32 GetPriority() const { return 0; }

private:
    void UpdateRenderQueue(NBRE_BinaryZone* zone, NBRE_Overlay& overlay);

private:

    NBRE_Context& mContext;

    typedef NBRE_Vector<NBRE_DebugBoundingBox*> DebugBoundingBoxArray;

    NBRE_DefaultSpatialDivideManager& mSpatialDivideManager;
    int32 mOverlayId;
    nb_boolean mVisible;
    nb_boolean mOnlyShowHasObj;
    int32 mCurrentBoundingBox;

    DebugBoundingBoxArray mDebugBoundingBoxs;

    static NBRE_Matrix4x4f mIdentity;

};


/*! @} */

#endif