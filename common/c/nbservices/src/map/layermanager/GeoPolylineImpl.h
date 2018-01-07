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
   @file         GeoPolylineImpl.h
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
#ifndef _GEOPOLYLINEIMPL_H_
#define _GEOPOLYLINEIMPL_H_

#include "GeoPolyline.h"
#include "ExternalMaterial.h"
#include "nbmacros.h"
#include "TileKey.h"
#include "GeographicLayer.h"

namespace NbmApi
{
class NBMPolylineMgr;
struct PolylineAttr;
}

namespace nbcommon
{
class DataStream;
}

namespace nbmap
{

typedef shared_ptr<const vector<GeoPoint> > GeoPointsPtr;
typedef shared_ptr<const vector<GeoPolylineAttributeEx*> > GeoPolylineAttrsPtr;
typedef pair<GeoPointsPtr, GeoPolylineAttrsPtr>   GeoPolylineData;

class GeoPolylineImpl;
class GeographicPolylineLayer;

/*! Wrapper of NbmPolylineMaker*/
class GeoPolylineTileGenerator : public GeographicTileGenerator
{
public:
    GeoPolylineTileGenerator(const shared_ptr<const string>& category);
    virtual ~GeoPolylineTileGenerator();

    // Refer to GeographicTileGenerator for description.
    virtual void GetMaterials(vector<ExternalMaterialPtr>& materials) const;
    virtual NB_Error GenerateTileData(const TileKeyPtr& key,
                                      shared_ptr<nbcommon::DataStream>& data);
    /*! Resets this generator.

      @return None.
    */
    virtual void Reset();

    int  PrepareForZoomLevel(int32 zoomLevel);

    /*! Feed this generator with polyline data. */
    void FeedWithPolyline(GeoPolylineImpl& polyline);

private:
    NB_DISABLE_COPY(GeoPolylineTileGenerator);

    void FeedPolylineWithColor(const GeoPolylineImpl& polyline);
    void FeedPolylineWithCap(const GeoPolylineImpl& polyline);

    void SetupCapAttr(NbmApi::PolylineAttr& attr,
                      GeoPolylineCapPtr cap,
                      bool isStart,
                      uint32 lineWidth);

    void SetupOutlineAttr(uint32 outlineColor, uint8 outlineWidth);

    shared_ptr<const string> m_highlightedMaterialName;
    shared_ptr<const string> m_unhighlightedMaterialName;
    shared_ptr<const string> m_bmpPatternMaterialName;
    shared_ptr<NbmApi::NBMPolylineMgr> m_tileGenerator;

    bool m_hasColor;
    bool m_isBmpPattern;
};

typedef shared_ptr<GeoPolylineTileGenerator> GeoPolylineTileGeneratorPtr;

/*! Internal implementation of GeoPolyline. */
class GeoPolylineImpl : public GeoPolyline
{
public:
    friend class GeoPolylineTileGenerator;
    GeoPolylineImpl(GeographicPolylineLayer& layer, GeoPolylineOption option);
    virtual ~GeoPolylineImpl();

    // Refer to Geopolyline for description.
    virtual void SetColors(shared_ptr<const vector<GeoPolylineAttributeEx*> > attrs);
    virtual void SetWidth(uint32 width);
    virtual void Submit();
    virtual void Show(bool bShow);
    virtual void SetSelected(bool isSelected);
    virtual bool IsSelected();
    virtual void SetDrawOrder (int order);
    virtual void SetParentGeographic(Geographic* parent);
    virtual Geographic* GetParentGeographic();

    uint32 GetDrawOrder();
    bool   NeedsUpdate()
    {
        return m_needUpdate;
    }
    GeoPolylineAttrsPtr m_attrs;
private:
    NB_DISABLE_COPY(GeoPolylineImpl);

    GeographicPolylineLayer& m_layer;

    GeoPointsPtr        m_points;

    GeoPolylineCapPtr   m_startCap;
    GeoPolylineCapPtr   m_endCap;

    bool   m_isSelected;                /*!< Flag to indicate if it is selected. */
    uint32 m_width;                     /*!< Width of polyline. */
    uint32 m_unhighlightedColor;        /*!< unhighlighted Color of this line. */
    uint32 m_outlineColor;              /*!<Outline color, in r, g, b, a order */
    uint8  m_outlineWidth;              /*!<Outline width, as %a of the road width. */
    bool   m_isEnabled;

    bool   m_needUpdate;
};

typedef shared_ptr<GeoPolylineImpl> GeoPolylineImplPtr;


// Polyline Cap Parameter.

class PolylineRoundCap : public GeoPolylineCap
{
public:
    friend class GeoPolylineTileGenerator;
    PolylineRoundCap(uint32 radius);
    virtual ~PolylineRoundCap();
private:
    uint32 m_radius;
};

class PolylineArrowCap : public GeoPolylineCap
{
public:
    friend class GeoPolylineTileGenerator;
    PolylineArrowCap(uint32 shapeWidth, uint32 shapeHeight);
    virtual ~PolylineArrowCap();

private:
    uint32 m_width;
    uint32 m_height;
};
}


#endif /* _GEOPOLYLINEIMPL_H_ */
/*! @} */
