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
   @file         GeoPolylineImpl.cpp
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

#include "GeoPolylineImpl.h"
#include "GeographicPolylineLayer.h"
#include "NBMPolylineMaker.h"
#include "datastreamimplementation.h"
#include "GeographicPrivate.h"
#include <algorithm>
#include <functional>
#include "paldebuglog.h"

#include <iterator>

using namespace std;
using namespace nbmap;
using namespace nbcommon;

#define RGBA_TO_BGRA(X) ((X & 0x00FF00FF) | ((X >> 16) & 0xFF00) | ((X & 0xFF00) << 16))

// Local variables..
//static const uint32 GEOPOLYLINE_DEFAULT_WIDHT  = 10;
//static const uint32 POLYLINE_HIGHLIGHTED_COLOR = 0x00FF00FF;
static const uint32 POLYLINE_OUTLINE_COLOR     = (uint32)-1;
static const uint32 POLYLINE_OUTLINE_WIDTH     = 10;
static const uint32 POLYLINE_INTERIOR_COLOR    = (uint32)-1;

static const string GEOGRAPHIC_MATERIAL_NAME_SUFIX("1");
static char TEXTURE_NAME[] = "textureName";

//#define LOG_PERFORMANCE
#ifdef LOG_PERFORMANCE
#include "palclock.h"

class PerformanceLogger
{
public:
    PerformanceLogger()
            : m_ts(PAL_ClockGetTimeMs()) {}
    virtual ~PerformanceLogger(){}
    void Log(const char* msg="")
    {
        DEBUGLOG(LOG_SS_NB_MAP, LOG_SEV_DEBUG, ("###PerformanceLogger: %s --- %lu msecs to last call\n", msg, PAL_ClockGetTimeMs() - m_ts));
        //fprintf(stderr, "PerformanceLogger: %s --- %lu msecs to last call\n",
                //msg, PAL_ClockGetTimeMs() - m_ts);
        m_ts = PAL_ClockGetTimeMs();
    }
    void Reset()
    {
        m_ts = PAL_ClockGetTimeMs();
    }
private:
    uint32 m_ts;
};

static PerformanceLogger g_PL;

#define PrepareLog() g_PL.Reset();
#define LogPerformance(X)       g_PL.Log(X);
#else
#define PrepareLog()
#define LogPerformance(X)
#endif



// Local classes ...........................................................................

/*! Functor used to turn GeoPoints to the one recognized by NbmApi */
class NbmApiGeoPointGenerator
{
public:
    NbmApiGeoPointGenerator(vector<GeoPolylineAttributeEx*>& attrs,
                            vector<NbmApi::Point>& points)
            : m_attrs(attrs),
              m_points(points),
              m_pIdx(0),
              m_aIdx(0){}
    NbmApiGeoPointGenerator(const NbmApiGeoPointGenerator& g)
            :m_attrs(g.m_attrs),
             m_points(g.m_points),
             m_pIdx(g.m_pIdx),
             m_aIdx(g.m_aIdx)    {}
    virtual ~NbmApiGeoPointGenerator(){}
    void operator()(const GeoPoint& point)
    {
        NbmApi::Point& p = m_points[m_pIdx];
        p.lat = (float)point.latitude;
        p.lon = (float)point.longitude;
        //DEBUGLOG(LOG_SS_NB_MAP, LOG_SEV_DEBUG, ("###Polyline m_pIdx[%d], lat[%f], lon[%f]", m_pIdx, p.lat, p.lon));
        //DEBUGLOG(LOG_SS_NB_MAP, LOG_SEV_DEBUG, ("###Polyline test %f %f, ", p.lat, p.lon));
        m_pIdx++;
    }

private:
    vector<GeoPolylineAttributeEx*>& m_attrs;
    vector<NbmApi::Point>& m_points;
    uint32 m_pIdx;
    uint32 m_aIdx;
    NbmApiGeoPointGenerator operator=(const NbmApiGeoPointGenerator&);
};


// Local function ..........................................................................

#define PI                  3.1313013314716454
#define TILE_WIDTH          256
#define RELATIVE_ZOOM_LEVEL 17 // defined by NBGM.

/*! Converts value of pixels into value of mercator coordinates

  @return converted value.
*/
static inline float PixelToMercator(uint32 value)
{
    // It is insane to set width of a polyline wider than 256 pixels...
    return (float)(2.0 * PI * (value > TILE_WIDTH ? TILE_WIDTH : value) /
                   ((1 << RELATIVE_ZOOM_LEVEL) * TILE_WIDTH));
}

/*! Transform TileBuf(provided by NBM library) into data stream. */
static inline DataStreamPtr TileBufToDataStream(const NbmApi::TileBuf& tileBuf)
{
    DataStreamPtr data;
    if (tileBuf.tileLen && tileBuf.tBuf)
    {
        data.reset(new DataStreamImplementation);
		if (data && data->AppendData((const uint8*)tileBuf.tBuf, tileBuf.tileLen) != NE_OK)
        {
            data.reset();
        }
    }
    return data;
}

bool GeoPolylineAttributeCompare(const GeoPolylineAttribute& left,
                                 const GeoPolylineAttribute& right)
{
    return left.m_endIndex < right.m_endIndex;
}

NbmApi::Point GeoPointToNbmPoint(const GeoPoint& point)
{
    NbmApi::Point p;
    p.lat = (float)point.latitude;
    p.lon = (float)point.longitude;
    return p;
}

static inline unsigned short GetPortional(uint32 value, uint32 baseValue)
{
    nsl_assert(baseValue);
    return static_cast<unsigned short>(value * 100 / baseValue);
}

// Implementation of GeoPolyline.
GeoPolyline::GeoPolyline(const std::string& id)
        : Geographic(id)
{
}

// Implementation of GeoPolylineOption.
GeoPolylineOption::GeoPolylineOption(const GeoPolylineOption& other)
{
   m_points = other.m_points;
   m_attrs = other.m_attrs;
   m_width = other.m_width;
   m_unhighlightedColor = other.m_unhighlightedColor;
   m_zOrder = other.m_zOrder;
   m_startCap = other.m_startCap;
   m_endCap = other.m_endCap;
   m_outlineColor = other.m_outlineColor;
   m_outlineWidth = other.m_outlineWidth;
}

// Implementation of GeoPolylineOptionOperation.
bool
GeoPolylineOptionOperation::isNeedCutPolyline(shared_ptr<vector<GeoPolylineOption> >& subPolylineOptions)
{
    int polylineTypeCount = m_option.m_attrs->size();
    if(polylineTypeCount <= 1)
    {
        return false;
    }

    //Process the multi-style polyline.
    const vector<GeoPoint>* points = m_option.m_points.get();
    const vector<GeoPolylineAttributeEx*>* attrs = m_option.m_attrs.get();

    vector<GeoPoint>::const_iterator it = points->begin();
    int offset = 0;
    for(int i=0; i<polylineTypeCount; ++i)
    {
        //Clone the current GeoPolylineOption as default value;
        GeoPolylineOption tmpOption(m_option);

        shared_ptr<vector<GeoPoint> > tmpSubPoints(new vector<GeoPoint>());

        int currentEndIndex = attrs->at(i)->m_endIndex;
        tmpSubPoints->assign(it + offset, it + currentEndIndex + 1);
        offset = currentEndIndex;
        shared_ptr<vector<GeoPolylineAttributeEx*> > tmpAttribute(new vector<GeoPolylineAttributeEx*>());
        GeoPolylineAttributeEx* subAttr = attrs->at(i);
        //PolylineType subPolylineType = subAttr->m_type;
        subAttr->m_endIndex = tmpSubPoints->size() -1;
        //uint32 subEndIndex = subAttr->m_endIndex;

//        if(i == polylineTypeCount-1)
//        {
//            vector<GeoPoint>::iterator it = tmpSubPoints->begin();
//            for(; it != tmpSubPoints->end(); it++)
//            {
//                DEBUGLOG(LOG_SS_NB_MAP, LOG_SEV_DEBUG, ("###Polyline after cutting [%d], size[%d], lat[%f], lon[%f]", i, tmpSubPoints->size(), (*it).latitude, (*it).longitude));
//            }
//        }

        tmpAttribute->push_back(subAttr);

        tmpOption.m_points = tmpSubPoints;
        tmpOption.m_attrs = tmpAttribute;

        //DEBUGLOG(LOG_SS_NB_MAP, LOG_SEV_DEBUG, ("###Polyline cutting, SUB LINE[%d], Type=[%s], sub total points[%d], endIndex[%d]", i, ((subPolylineType == PT_SOLID_LINE)?"SOLID":"DASH"), tmpOption.m_points->size(), subEndIndex));
        subPolylineOptions->push_back(tmpOption);
    }
    return true;
}

// Implementation of GeoPolylineImpl
/* See description in header file. */
GeoPolylineImpl::GeoPolylineImpl(GeographicPolylineLayer& layer, GeoPolylineOption option)
        : GeoPolyline(*(layer.GetTileDataType())),
          m_attrs(option.m_attrs),
          m_layer(layer),
          m_points(option.m_points),
          m_startCap(option.m_startCap),
          m_endCap(option.m_endCap),
          m_isSelected(false),
          m_width(option.m_width),
          m_unhighlightedColor(option.m_unhighlightedColor),
          m_outlineColor(option.m_outlineColor),
          m_outlineWidth(option.m_outlineWidth),
          m_isEnabled(false),
          m_needUpdate(true)
{
    m_private->m_drawOrder = option.m_zOrder;
}

/* See description in header file. */
GeoPolylineImpl::~GeoPolylineImpl()
{
}

void
GeoPolylineImpl::SetParentGeographic(Geographic* parent)
{
    m_parentGeographic = parent;
}

Geographic*
GeoPolylineImpl::GetParentGeographic()
{
    return m_parentGeographic;
}

/* See description in header file. */
void GeoPolylineImpl::SetColors(shared_ptr<const vector<GeoPolylineAttributeEx*> > attrs)
{
    m_attrs      = attrs;
    m_needUpdate = true;
    Submit();
}

/* See description in header file. */
void GeoPolylineImpl::SetWidth(uint32 width)
{
    m_width      = width;
    m_needUpdate = true;
    Submit();
}

/* See description in header file. */
void GeoPolylineImpl::Submit()
{
    m_layer.GraphicUpdated(this);
}

/* See description in header file. */
void GeoPolylineImpl::Show(bool bShow)
{
    if (m_isEnabled != bShow)
    {
        m_isEnabled = bShow;
        //DEBUGLOG(LOG_SS_NB_MAP, LOG_SEV_DEBUG, ("###Polyline layer enable[%s]", bShow ? "true":"false"));
        m_layer.GraphicEnabled(this, m_isEnabled);
    }
}

/* See description in header file. */
bool GeoPolylineImpl::IsSelected()
{
    return m_isSelected;
}

/* See description in header file. */
void GeoPolylineImpl::SetSelected(bool isSelected)
{
    if (m_isSelected != isSelected)
    {
        m_isSelected = isSelected;
        m_layer.GraphicObjectSelected(isSelected);
        if (m_private->m_listener != NULL)
        {
            m_private->m_listener->OnSelected(isSelected);
        }
    }
}

void GeoPolylineImpl::SetDrawOrder (int order)
{
    m_private->m_drawOrder = order;
    Submit();
}

uint32 GeoPolylineImpl::GetDrawOrder()
{
    return m_private->m_drawOrder;
}

// Implementation of GeoPolylineTileGenerator
/* See description in header file. */
GeoPolylineTileGenerator::GeoPolylineTileGenerator(const shared_ptr<const string>& category)
        : GeographicTileGenerator(category),
          m_highlightedMaterialName(CCC_NEW string("Highlighted")),
          m_unhighlightedMaterialName(CCC_NEW string("Unhighlighted")),
          m_bmpPatternMaterialName(CCC_NEW string("m_bmpPatternMaterialName")),
          m_hasColor(true),
          m_isBmpPattern(false)
{
}

/* See description in header file. */
GeoPolylineTileGenerator::~GeoPolylineTileGenerator()
{
}

/* See description in header file. */
void GeoPolylineTileGenerator::GetMaterials(vector<ExternalMaterialPtr>& materials) const
{
    vector<NbmApi::TileBuf> buffer;
    buffer.resize(2);
    materials.reserve(2);
    // First one is highlighted one.
    PrepareLog();
    NbmErrorCode error = NBM_NO_ERROR;

    if(m_isBmpPattern)
    {
        error = m_tileGenerator->BuildBmpPatternMaterial(buffer[0]);
        DEBUGLOG(LOG_SS_NB_MAP, LOG_SEV_DEBUG, ("###Polyline GetMaterials::create Bmp Pattern"));
    }
    else
    {
        if(m_hasColor)
        {
            error = m_tileGenerator->BuildSolidColorLineMaterial(buffer[0], buffer[1]);
            DEBUGLOG(LOG_SS_NB_MAP, LOG_SEV_DEBUG, ("###Polyline GetMaterials::create solid Pattern"));
        }
        else
        {
            error = m_tileGenerator->BuildPolylineMaterial(buffer[0]);
        }
    }

    LogPerformance ("after build materials");
    if (error != NBM_NO_ERROR)
    {
        return;
    }

    if(m_isBmpPattern)
    {
        DataStreamPtr data = TileBufToDataStream(buffer[0]);
            ExternalMaterialPtr material(CCC_NEW ExternalMaterial(m_materialCategory,
                                                                  m_bmpPatternMaterialName,
                                                                  data));
        if (data && material)
        {
            materials.push_back(material);
            DEBUGLOG(LOG_SS_NB_MAP, LOG_SEV_DEBUG, ("###Polyline GetMaterials::add Bmp Pattern"));
        }
    }
    else
    {
        DataStreamPtr data = TileBufToDataStream(buffer[0]);
            ExternalMaterialPtr material(CCC_NEW ExternalMaterial(m_materialCategory,
                                                                  m_highlightedMaterialName,
                                                                  data));
        if (data && material)
        {
            materials.push_back(material);
            DEBUGLOG(LOG_SS_NB_MAP, LOG_SEV_DEBUG, ("###Polyline GetMaterials::add solid highlight Pattern[%s]-[%s]-[%d]", m_materialCategory->c_str(), m_highlightedMaterialName->c_str(), data->GetDataSize()));
        }

        if (m_hasColor)
        {
            data = TileBufToDataStream(buffer[1]);
            material.reset(CCC_NEW ExternalMaterial(m_materialCategory,
                                                    m_unhighlightedMaterialName,
                                                    data));

            if (data && material)
            {
                materials.push_back(material);
                DEBUGLOG(LOG_SS_NB_MAP, LOG_SEV_DEBUG, ("###Polyline GetMaterials::add solid un-highlight Pattern[%s]-[%s]-[%d]", m_materialCategory->c_str(), m_unhighlightedMaterialName->c_str(), data->GetDataSize()));
            }
        }
    }

}

/* See description in header file. */
NB_Error GeoPolylineTileGenerator::GenerateTileData(const TileKeyPtr& key,
                                                    shared_ptr<nbcommon::DataStream>& data)
{
    struct NbmApi::TileBuf buffer;
    NB_Error error = NE_OK;
    NbmErrorCode nbmerr = m_tileGenerator->GetTile(key->m_x, key->m_y, key->m_zoomLevel, buffer);
    switch (nbmerr)
    {
        case NBM_NO_ERROR:
        {
            data = TileBufToDataStream(buffer);
            if (!data || !data->GetDataSize())
            {
                error = NE_NOMEM;
            }
            break;
        }
        case NBM_NO_MEMORY:
        {
            error = NE_NOMEM;
            break;
        }
        case NBM_NO_DATA:
        {
            error = NE_HTTP_NO_CONTENT;
            break;
        }
        default:
        {
            error = NE_UNEXPECTED;
            break;
        }
    }

    DEBUGLOG(LOG_SS_NB_MAP, LOG_SEV_DEBUG, ("###Polyline GenerateTileData[%d-%d-%d] return[%d]", key->m_x, key->m_y, key->m_zoomLevel, (int)nbmerr));
    return error;
}

/* See description in header file. */
int GeoPolylineTileGenerator::PrepareForZoomLevel(int32 zoomLevel)
{
    PrepareLog();
    int tileCount = m_tileGenerator->BuildPolylinesTiles(zoomLevel);
    LogPerformance("After buildRouteTiles");
    return tileCount;
}


/* See description in header file. */
void GeoPolylineTileGenerator::Reset()
{
    //@note: NbmApi::NBMPolylineMgr provides an API named Reset(), but it is not working
    //       correctly as expected, so I decide to not reuse this NBMPolylineMgr.
    m_tileGenerator.reset(CCC_NEW NbmApi::NBMPolylineMgr);

    // Outline settings of polyline is not exposed, need to handle it internally.
    map<unsigned int, NbmApi::PolylineAttr> polylineAttrs;
    NbmApi::PolylineAttr& attr = polylineAttrs[0];
    nsl_memset(&attr, 0, sizeof(NbmApi::PolylineAttr));

    //@todo: we should expose outline settings to user of CoreSDK...
    attr.polylineAttrId = 0;
    attr.interiorColor  = POLYLINE_INTERIOR_COLOR;
    attr.outlineColor   = 0;
    attr.outlineWidth   = 0; // Disable outline based on requirement.
    attr.radius         = 50;
    attr.startType      = NbmApi::Circle;
    attr.endType        = NbmApi::Circle;
    m_tileGenerator->SetPolylineAttr(polylineAttrs);

    // Need to ensure the materialName is unique...
    m_highlightedMaterialName.reset(new string(*m_highlightedMaterialName +
                                               GEOGRAPHIC_MATERIAL_NAME_SUFIX));
    m_unhighlightedMaterialName.reset(new string(*m_unhighlightedMaterialName +
                                                 GEOGRAPHIC_MATERIAL_NAME_SUFIX));
    m_bmpPatternMaterialName.reset(new string(*m_bmpPatternMaterialName +
                                              GEOGRAPHIC_MATERIAL_NAME_SUFIX));
}

/* See description in header file. */
void GeoPolylineTileGenerator::FeedWithPolyline(GeoPolylineImpl& polyline)
{
    if (!polyline.m_points || polyline.m_points->empty())
    {
        return;
    }

    // NCDB treat polylines differently internally.
    if (polyline.m_startCap ||polyline.m_endCap)
    {
        FeedPolylineWithCap(polyline);
    }
    else
    {
        FeedPolylineWithColor(polyline);
    }

    polyline.m_needUpdate = false;
}

void
GeoPolylineTileGenerator::SetupOutlineAttr(uint32 outlineColor, uint8 outlineWidth)
{
    map<unsigned int, NbmApi::PolylineAttr> polylineAttrs;
    NbmApi::PolylineAttr& attr = polylineAttrs[0];
    nsl_memset(&attr, 0, sizeof(NbmApi::PolylineAttr));

    attr.polylineAttrId = 0;
    attr.interiorColor  = POLYLINE_INTERIOR_COLOR;
    attr.outlineColor   = (unsigned int)outlineColor;
    attr.outlineWidth   = (unsigned char)outlineWidth;
    attr.radius         = 50;
    attr.startType      = NbmApi::Circle;
    attr.endType        = NbmApi::Circle;
    m_tileGenerator->SetPolylineAttr(polylineAttrs);
}

/* See description in header file. */
void GeoPolylineTileGenerator::FeedPolylineWithColor(const GeoPolylineImpl& polyline)
{
    const GeoPointsPtr& points = polyline.m_points;

    m_tileGenerator->SetDefaultColor(polyline.m_unhighlightedColor);
    m_tileGenerator->SetRoadWidth(PixelToMercator(polyline.m_width));

    shared_ptr<vector<GeoPolylineAttributeEx*> > attrs(new vector<GeoPolylineAttributeEx*>);
    // Reset polyline attributes if not provided.
    if (polyline.m_attrs && !polyline.m_attrs->empty())
    {
        copy(polyline.m_attrs->begin(), polyline.m_attrs->end(), back_inserter(*attrs));
    }

    //Now the design, at here, the attrs should have only 1 item.
    if(attrs->size() != 1)
    {
        DEBUGLOG(LOG_SS_NB_MAP, LOG_SEV_DEBUG, ("###Polyline error"));
        return;
    }

    GeoPolylineAttributeEx* attribute = (*attrs)[0];
    if(attribute->m_type == PT_SOLID_LINE)
    {
        if(polyline.m_outlineWidth != 0)
        {
            SetupOutlineAttr(polyline.m_outlineColor, polyline.m_outlineWidth);
        }

        m_isBmpPattern = false;
        m_tileGenerator->SetLineStyle(NbmApi::Solid);

        vector<NbmApi::PolylineColor> polylineColors;
        NbmApi::PolylineColor color = {0};
        GeoPolylineColorAttribute* colorAttribute = static_cast<GeoPolylineColorAttribute*>(attribute);
        color.color = colorAttribute->m_color;
        color.endIdx = colorAttribute->m_endIndex;
        polylineColors.push_back(color);
        m_tileGenerator->SetPolylineColorAttr(polylineColors);
        DEBUGLOG(LOG_SS_NB_MAP, LOG_SEV_DEBUG, ("###Polyline GeoPolylineTileGenerator::FeedPolylineWithColor- TOTAL points[%d]", polyline.m_points->size()));
        DEBUGLOG(LOG_SS_NB_MAP, LOG_SEV_DEBUG, ("###Polyline GeoPolylineTileGenerator::PolylineColor-highlight color[%d] unhighlight[%d], endIdx[%d]", color.color, polyline.m_unhighlightedColor, color.endIdx));

        // Some pointers and references are used for the sake of efficiency.
        struct NbmApi::PolylineComponent element;
        element.polylineId = *m_materialCategory;
        element.vPoints.resize(points->size()); // resize to avoid memory reallocation.
        PrepareLog();
        for_each(points->begin(), points->end(),
                 NbmApiGeoPointGenerator(*attrs, element.vPoints));
        LogPerformance ("after trans");
        PrepareLog();

        DEBUGLOG(LOG_SS_NB_MAP, LOG_SEV_DEBUG, ("###Polyline GeoPolylineTileGenerator::FeedPolylineWithColor- PolylineComponent total  points[%d], polylineId[%s]", element.vPoints.size(), element.polylineId.c_str()));
        m_tileGenerator->SetPolyline(element);
        LogPerformance ("after set to PolylineMaker");
    }
    else
    {
        m_tileGenerator->SetLineStyle(NbmApi::Dash);
        m_isBmpPattern = true;
        static int id = 100;

        vector<NbmApi::PolylineColor> polylineColors;
        NbmApi::PolylineColor color = {0};
        GeoPolylineColorAttribute* colorAttribute = static_cast<GeoPolylineColorAttribute*>(attribute);
        color.color = id;//colorAttribute->m_color;//
        color.endIdx = colorAttribute->m_endIndex;
        polylineColors.push_back(color);
        m_tileGenerator->SetPolylineColorAttr(polylineColors);

        shared_ptr<vector<NbmApi::BmpPattern> > bmpPattern(new vector<NbmApi::BmpPattern>);
        bmpPattern->resize(1);
        GeoPolylinePatternAttribute *patternAttribute = (GeoPolylinePatternAttribute*)attribute;
        if(patternAttribute)
        {
            (*bmpPattern)[0].color = id;

            (*bmpPattern)[0].textureName = TEXTURE_NAME;

            //TODO: need free this
            uint32 textureDataSize = patternAttribute->m_textureData->GetDataSize();
            uint8* buffer = CCC_NEW uint8[textureDataSize];
            nsl_memset(buffer, 0, textureDataSize);
            patternAttribute->m_textureData->GetData(buffer, 0, textureDataSize);

            (*bmpPattern)[0].textureBuf = (char*)buffer;
            (*bmpPattern)[0].textureBufLen = textureDataSize;
            (*bmpPattern)[0].distance = (unsigned char)patternAttribute->m_distance;
            (*bmpPattern)[0].width = static_cast<unsigned char>(patternAttribute->m_textureWidth);
            (*bmpPattern)[0].height = static_cast<unsigned char>(patternAttribute->m_textureHeight);

            m_tileGenerator->SetPolylinePattern(*bmpPattern);

            struct NbmApi::PolylineComponent element;
            element.polylineId = *m_materialCategory; //string(temp);
            element.vPoints.resize(points->size()); // resize to avoid memory reallocation.
            PrepareLog();
            for_each(points->begin(), points->end(),
                     NbmApiGeoPointGenerator(*attrs, element.vPoints));
            LogPerformance ("after trans");
            PrepareLog();

            DEBUGLOG(LOG_SS_NB_MAP, LOG_SEV_DEBUG, ("###Polyline GeoPolylineTileGenerator::FeedPolylineWithColor- PolylineComponent total  points[%d], polylineId[%s]", element.vPoints.size(), element.polylineId.c_str()));
            m_tileGenerator->SetPolyline(element);
            id++;
        }
    }
}

void GeoPolylineTileGenerator::SetupCapAttr(NbmApi::PolylineAttr& attr,
                                            GeoPolylineCapPtr cap,
                                            bool isStart,
                                            uint32 lineWidth)
{
    if (!cap || !lineWidth)
    {
        return;
    }

    NbmApi::CapType* pType = isStart ? &attr.startType : &attr.endType;
    PolylineCapType capType = cap->GetCapType();
    switch (capType)
    {
        case PCT_Arrow:
        {
            *pType = NbmApi::Arrow;
            PolylineArrowCap* pCap = static_cast<PolylineArrowCap*>(cap.get());
            if (cap)
            {
                attr.shapeWidth  = GetPortional(pCap->m_width, lineWidth);
                attr.shapeHeight = GetPortional(pCap->m_height,lineWidth);;
            }
            break;
        }
        case PCT_Round:
        {
            *pType = NbmApi::Circle;
            PolylineRoundCap* pCap = static_cast<PolylineRoundCap*>(cap.get());
            if (cap)
            {
                attr.radius = GetPortional(pCap->m_radius, lineWidth);
            }
            break;
        }
        default:
        {
            attr.startType = NbmApi::Line_Join;
            break;
        }
    }
}

/* See description in header file. */
void GeoPolylineTileGenerator::FeedPolylineWithCap(const GeoPolylineImpl& polyline)
{
    // NCDB supports add a list of polylines at once, but for now, only one polyline
    // is needed.

    m_hasColor = false;

    // 1. set road width.
    m_tileGenerator->SetRoadWidth(PixelToMercator(polyline.m_width));

    // 2. Generate and set attributes.
    map<unsigned int, NbmApi::PolylineAttr> polylineAttrs;
    NbmApi::PolylineAttr& attr = polylineAttrs[0];
    nsl_memset(&attr, 0, sizeof(NbmApi::PolylineAttr));

    attr.polylineAttrId = 1; // IDs assigned by user starts from 1.
    attr.interiorColor  = polyline.m_unhighlightedColor;
    attr.outlineColor   = POLYLINE_OUTLINE_COLOR;
    attr.outlineWidth   = POLYLINE_OUTLINE_WIDTH;

    // @bug: if start cap and end cap has same CapType, then width/height/radius of
    //       the start cap will be overridden by the end cap. We can't fix this
    //       unless updating existing NBM library.
    if (polyline.m_startCap)
    {
        SetupCapAttr(attr, polyline.m_startCap, true, polyline.m_width);
    }
    else
    {
        attr.startType = NbmApi::Line_Join;
    }

    if (polyline.m_endCap)
    {
        SetupCapAttr(attr, polyline.m_endCap, false, polyline.m_width);
    }
    else
    {
        attr.endType = NbmApi::Line_Join;
    }

    m_tileGenerator->SetPolylineAttr(polylineAttrs);

    // 3. Set real points ...
    const GeoPointsPtr& points = polyline.m_points;

    vector<NbmApi::PolylineElement> elements;
    elements.resize(1);
    struct NbmApi::PolylineElement& element = elements.front();
    element.polylineId     = *m_materialCategory;
    element.polylineAttrId = 1;
    element.vPoints.reserve(points->size());

    transform(points->begin(), points->end(), back_inserter(element.vPoints),
              ptr_fun(&GeoPointToNbmPoint));

    m_tileGenerator->SetPolylineList(elements);
}

/* See description in header file. */
GeoPolylineCap::GeoPolylineCap(PolylineCapType type)
        : m_type(type)
{
}

/* See description in header file. */
PolylineCapType GeoPolylineCap::GetCapType()
{
    return m_type;
}

/* See description in header file. */
shared_ptr<GeoPolylineCap>
GeoPolylineCap::CreateArrowCap(uint32 shapeWidth, uint32 shapeHeight)
{
    return GeoPolylineCapPtr(
        CCC_NEW PolylineArrowCap(shapeWidth, shapeHeight));
}

/* See description in header file. */
shared_ptr<GeoPolylineCap>
GeoPolylineCap::CreateRoundCap(uint32 radius)
{
    return GeoPolylineCapPtr(
        CCC_NEW PolylineRoundCap(radius));
}

/* See description in header file. */
PolylineRoundCap::PolylineRoundCap(uint32 radius)
        : GeoPolylineCap(PCT_Round),
          m_radius(radius)
{
}

/* See description in header file. */
PolylineRoundCap::~PolylineRoundCap()
{
}

/* See description in header file. */
PolylineArrowCap::PolylineArrowCap(uint32 shapeWidth, uint32 shapeHeight)
        : GeoPolylineCap(PCT_Arrow),
          m_width(shapeWidth),
          m_height(shapeHeight)
{
}

/* See description in header file. */
PolylineArrowCap::~PolylineArrowCap()
{
}

/*! @} */
