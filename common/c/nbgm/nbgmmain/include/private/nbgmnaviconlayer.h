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

    @file nbgmnaviconlayer.h
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

#ifndef _NBGM_NAV_ICON_LAYER_H_
#define _NBGM_NAV_ICON_LAYER_H_
#include "nbretypes.h"
#include "nbgmnavdata.h"
#include "nbrescenemanager.h"
#include "nbrerenderengine.h"
#include "nbrebillboardnode.h"
#include "nbgmvectortile.h"
#include "nbgmvectortiledata.h"
#include "nbgmlayoutelement.h"
#include "nbgmmaplayer.h"
#include "nbgmcontext.h"
#include "nbgmmaplayer.h"
#include "nbgmlayoutmanager.h"

class NBGM_Icon;
/*! \addtogroup NBRE_Service
*  @{
*/

/** Icon layer for navigation
*/
class NBGM_NavIconLayer:
    public NBGM_ILayoutProvider
{
    const static uint32       NODECOUNT = 3;
public:
    typedef NBRE_Vector<NBGM_Poi*> PoiList;
    typedef NBRE_Vector<NBGM_Icon*> IconList;
public:
    NBGM_NavIconLayer(NBGM_Context& nbgmContext, NBRE_Node& parentNode, NBRE_BillboardSet* billboardSet);
    virtual ~NBGM_NavIconLayer();

public:
    /// Add to Scene for rendering
    void AddToSence();
    /// Refresh pois
    void Refresh(const PoiList& pois);
    /// Set priority
    void SetPriority(int32 priority) { mPriority = priority; }
    /// Update pois
    void Update();
    /// Notify pois extend
    void NotifyExtend(const NBGM_TileMapBoundaryExtendType& extendType);
    /// Update layout list
    virtual void UpdateLayoutList(NBGM_LayoutElementList& layoutElements);

private:
    /// Create poi shader
    NBRE_ShaderPtr CreatePoiShader(const NBRE_String& path);

private:
    DISABLE_COPY_AND_ASSIGN(NBGM_NavIconLayer);

private:
    NBGM_Context&              mNBGMContext;
    NBRE_Node&                 mParentNode;
    IconList                   mIcons;
    NBRE_BillboardSet*         mBillboardSet;
    NBRE_NodePtr               mNode[NODECOUNT];
    int32                      mPriority;
    NBGM_TileMapBoundaryExtendType mExtendType;
};

/*! @} */

#endif


