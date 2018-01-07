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

    @file nbgmmapdataimpl.h
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

#ifndef _NBGM_MAP_DATA_IMPL_H_
#define _NBGM_MAP_DATA_IMPL_H_
#include "nbgmmapview.h"
#include "palerror.h"
#include "nbrevector3.h"
#include "nbretypes.h"
#include "nbgmvectortiledata.h"
#include "nbgmresourcemanager.h"

class NBGM_IMapDataLayer;

/*! \addtogroup NBGM_Manager
*  @{
*/
class NBGM_MapDataImpl:
    public NBGM_IMapData
{
public:
    NBGM_MapDataImpl(const NBGM_Point2d& refCenter);
    virtual ~NBGM_MapDataImpl();
    
    PAL_Error CreateNBREMapData(NBGM_ResourceContext& resourceContext, const NBRE_String& nbmDataId, NBRE_Mapdata** mapdata);
    virtual NBGM_IPolylineLayer* AddPolylineLayer(float nearVisibility, float farVisibility);
private:
    NBGM_Point2d mRefCenter;
    NBRE_Vector<NBGM_IMapDataLayer*> mLayers;
};

class NBGM_IMapDataLayer
{
public:
    virtual ~NBGM_IMapDataLayer() {}
    virtual PAL_Error CreateLayerData(NBGM_ResourceContext& resourceContext, const NBRE_String& nbmDataId, NBRE_Layer** data) = 0;
};

class NBGM_MapMaterialImpl:
    public NBGM_IMapMaterial
{
public:
    NBGM_MapMaterialImpl():mId(0) {}
    virtual ~NBGM_MapMaterialImpl() {}
    virtual NBRE_MapMaterial* CreateNBREMaterial(PAL_Instance* palInstance, NBRE_IRenderPal& renderPal, NBRE_TextureManager& textureManager, const NBRE_String& nbmDataId, nb_boolean isExternalMaterial) = 0;
    uint16 GetId() { return mId; }
    void SetId(uint16 id) { mId = id; }
private:
    uint16 mId;
};

class NBGM_PolylineLayerImpl:
    public NBGM_IPolylineLayer,
    public NBGM_IMapDataLayer
{
    struct PolylineData 
    {
    public:
        PolylineData(const NBGM_ElementId& id, vector<NBGM_Point2d>& pts, float width, uint16 exteralMateral, NBGM_IMapMaterial* material);
        ~PolylineData();
    public:
        NBGM_ElementId id;
        vector<NBGM_Point2d> points;
        float width;
        uint16 exteralMateral;
        NBGM_MapMaterialImpl* material;
    };
public:
    NBGM_PolylineLayerImpl(float nearVisibility, float farVisibility);
    virtual ~NBGM_PolylineLayerImpl();
    virtual void AddPolyline(const std::string& id, vector<NBGM_Point2d>& pts, float width, uint16 exteralMateral );
    virtual void AddPolyline(const std::string& id, vector<NBGM_Point2d>& pts, float width, NBGM_IMapMaterial* material);
    virtual PAL_Error CreateLayerData(NBGM_ResourceContext& resourceContext, const NBRE_String& nbmDataId, NBRE_Layer** data);

private:
    float nearVisibility;
    float farVisibility;
    NBRE_Vector<PolylineData*> mPolylines;
};

class NBGM_StandardColorMapMaterialImpl:
    public NBGM_MapMaterialImpl
{
public:
    NBGM_StandardColorMapMaterialImpl(uint32 color);
    virtual ~NBGM_StandardColorMapMaterialImpl();
    virtual NBRE_MapMaterial* CreateNBREMaterial(PAL_Instance* palInstance, NBRE_IRenderPal& renderPal, NBRE_TextureManager& textureManager, const NBRE_String& nbmDataId, nb_boolean isExternalMaterial);

private:
    uint32 mColor;
};


/*! @} */

#endif


