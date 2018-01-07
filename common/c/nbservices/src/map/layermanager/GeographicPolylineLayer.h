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
   @file         GeographicPolylineLayer.h
   @defgroup     nbmap
*/
/*
    (C) Copyright 2013 by TeleCommunications Systems, Inc.

   The information contained herein is confidential, proprietary to
   TeleCommunication Systems, Inc., and considered a trade secret as defined
   in section 499C of the penal code of the State of California. Use of this
   information by anyone other than authorized employees of TeleCommunication
   Systems is granted only under a written non-disclosure agreement, expressly
   prescribing the scope and manner of such use.
--------------------------------------------------------------------------*/

/*! @{ */
#ifndef _GEOGRAPHICPOLYLINELAYER_H_
#define _GEOGRAPHICPOLYLINELAYER_H_

#include "nbmacros.h"
#include "LayerIdGenerator.h"
#include "GeoPolylineImpl.h"

namespace nbmap
{

/*! GeographicPolylineLayer

    It inherits from GeographicLayer, and is exposed to LayerManager and Mapview, it get
    tiles form a TileGenerator, and return tiles to callers.
*/
class GeographicPolylineLayer : public GeographicLayer
{
public:
    GeographicPolylineLayer(NB_Context* context,
                            LayerIdGeneratorPtr layerIdGenerator,
                            GeographicLayerListener* listener,
                            const GeoPolylineOption& option);
    virtual ~GeographicPolylineLayer();

    // virtual void GraphicUpdated(const Geographic* graphic);

    /*! Get associated polyline object.

        @return GeoPolylineImpl*
    */
    GeoPolylineImpl* GetPolylineObject();

    void GraphicObjectSelected(bool selected);

    virtual std::string className() const { return "GeographicPolylineLayer"; }

private:
    NB_DISABLE_COPY(GeographicPolylineLayer);

    GeographicPolylineLayer(shared_ptr<string> parentName,
                            uint32 layerIdDigital,
                            NB_Context* context,
                            shared_ptr<const string> materialCategory,
                            GeoPolylineTileGeneratorPtr tileGenerator);
    virtual GeographicUpdateType ProcessUpdatedGraphic(Geographic* graphic);
    virtual void PostGraphicUpdated(Geographic* graphic);

    void LoadAndActivateExteranMaterials();
    void UnloadExternalMaterials(bool old=false);
    void FeedTileGenerator()
    {
        if (m_tileGenerator && m_polyline)
        {
            m_tileGenerator->FeedWithPolyline(*m_polyline);
        }
    }

    virtual void GetTile(TileKeyPtr tileKey,
                         TileKeyPtr convertedTileKey,
                         AsyncTileRequestWithRequestPtr callback,
                         uint32 priority,
                         bool cachedOnly);

    shared_ptr<string> GetContentId(const TileKeyPtr& tileKey);

    bool                        m_tileAvaiable;
    vector<ExternalMaterialPtr> m_materials;
    vector<ExternalMaterialPtr> m_oldMaterials;
    GeoPolylineImplPtr          m_polyline;
    GeoPolylineTileGeneratorPtr m_tileGenerator;
};

typedef shared_ptr<GeographicPolylineLayer> GeographicPolylineLayerPtr;

}

#endif /* _GEOGRAPHICPOLYLINELAYER_H_ */
/*! @} */
