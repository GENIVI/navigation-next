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
    @file       GeographyLayerImpl.cpp

    See header file for description.
*/
/*
    (C) Copyright 2012 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

extern "C"
{
#include "csltypes.h"
#include "cslutil.h"
#include "transformutility.h"
#include "palclock.h"
}

#include "palmath.h"
#include "GeographyLayerImpl.h"
#include "GeographyManagerImpl.h"
#include "TileImpl.h"
#include "datastreamimplementation.h"
#include "StringUtility.h"
#include "LayerFunctors.h"
#include "UnifiedLayer.h" // Several useful macros are defined there.

// Disable "warning C4244: '=' : conversion from 'NBMDataPrecision' to 'unsigned short', possible loss of data"
// Disable "warning C4245: conversion from 'int' to 'unsigned int', signed/unsigned mismatch"
// (we don't own this code, the core team does!)
#pragma warning( push )
#pragma warning(disable:4244)
#pragma warning(disable:4245)
    #include "NBMCommon.h"
    #include "NBMTypes.h"
#pragma warning( pop )

#include <sstream>
#include <limits>
#include <map>
#include <set>
#include <iterator>
#include <algorithm>

/*! @{ */

using namespace nbcommon;
using namespace nbmap;

// Default draw order for geography layer
#define DEFAULT_GEOGRAPHY_LAYER_DRAW_ORDER 224

// Constants ....................................................................................

// Default reference tile grid level of geography layers
const uint32 GEOGRAPHY_LAYER_REFERENCE_TILE_GRID_LEVEL = 2;

// Used to generate checksum
const uint32 CRC_VALUE = 0xAEF0AEF0;

// Template to generate a content ID
const char PARAMETER_SHAPE_LAYER_ID[]   = "$layerid";
const char PARAMETER_SHAPE_TILE_X[]     = "$x";
const char PARAMETER_SHAPE_TILE_Y[]     = "$y";
const char PARAMETER_SHAPE_ZOOM_LEVEL[] = "$z";
const char PARAMETER_SHAPE_UNIFIED_ID[] = "$uid";
const char SHAPE_CONTENT_ID_TEMPLATE[]  = "$layerid_$x_$y_$z_$uid";

class ErrorNotifier
{
public:
    ErrorNotifier(AsyncTileRequestWithRequestPtr callback,
                  NB_Error error)
            : m_pCallback(callback),
              m_error(error)
    { }

    virtual ~ErrorNotifier()
    { }

    void operator() (const TileKeyPtr& key) const
    {
        if (m_pCallback && key)
        {
            m_pCallback->Error(key, m_error);
        }
    }

private:
    AsyncTileRequestWithRequestPtr m_pCallback;
    NB_Error m_error;
};

/*! Function used to convert a Geography coordinate to a NBM point */
static POINT2
TransformCoordinate(const GeographyCoordinate& coordinate)
{
	POINT2 point = {(float)coordinate.m_longitude, (float)coordinate.m_latitude};
	return point;
}


// Public functions .............................................................................

/* See header file for description */
GeographyLayerImpl::GeographyLayerImpl(shared_ptr<string> layerID,
                                       uint32 layerIdDigital,
                                       GeographyManager* geographyManager,
                                       NB_Context* context)
        : UnifiedLayer(TileManagerPtr(), layerIdDigital, context),
          m_layerID(layerID),
          m_geographyManager(geographyManager)
{
    // @note: After creating GeographyLayerImpl, use GeographyLayerImpl::IsValid() to check
    //        whether this object is valid. Do not use it if it is in invalid state.
    if (m_tileLayerInfo)
    {
        m_tileLayerInfo->drawOrder        = DEFAULT_GEOGRAPHY_LAYER_DRAW_ORDER;
        m_tileLayerInfo->tileDataType     = layerID;
        m_tileLayerInfo->refTileGridLevel = GEOGRAPHY_LAYER_REFERENCE_TILE_GRID_LEVEL;
    }
    m_isOverlay     = false;
    m_isRasterLayer = false;

    // Update characteristics.
    SetCharacteristics(TILE_ADDITIONAL_KEY_OPTIONAL, "");
    if (layerID)
    {
        SetCharacteristics(TILE_ADDITIONAL_KEY_NAME, *layerID);
    }
    ProcessCharacteristics();

    // Set this layer as enabled by default!
    SetEnabled(true);
}

/* See header file for description */
GeographyLayerImpl::~GeographyLayerImpl()
{
    // Nothing to do here.
}

NB_Error
GeographyLayerImpl::AddGeographyShape(GeographyShapePtr shape, int fitZoomLevel)
{
    if(!shape)
    {
        return NE_INVAL;
    }

    m_tileLayerInfo->refTileGridLevel = fitZoomLevel;
    GeographyType shapeType = shape->GetShapeType();
    if(shapeType <= GEO_GRAPHIC_INVALID || shapeType > GEO_GRAPHIC_POLYGON)
    {
        return NE_INVAL;
    }
    else if(shapeType == GEO_GRAPHIC_POLYLINE)
    {
        ProcessAddPolylineShape(shape, fitZoomLevel);
    }
    else if(shapeType == GEO_GRAPHIC_POLYGON)
    {
        ProcessAddPolygonShape(shape, fitZoomLevel);
    }
    return NE_OK;
}

NB_Error
GeographyLayerImpl::RemoveGeographyShape(shared_ptr<string> id)
{
    if(!id || id->empty())
    {
        return NE_INVAL;
    }

    map<string, TileKeyPtr>::iterator tileKeyIter;
    tileKeyIter = m_tileKeyIndexByShapeID.find(*id);
    if(tileKeyIter == m_tileKeyIndexByShapeID.end())
    {
        return NE_INVAL;
    }

    //find the tile key
    TileKeyPtr tileKeyPtr = tileKeyIter->second;
    m_tileKeyIndexByShapeID.erase(tileKeyIter);

    map<TileKey, map<string, GeographyShapePtr> >::iterator iter = m_geographyShapes.find(*tileKeyPtr);
    if(iter != m_geographyShapes.end())
    {
        map<string, GeographyShapePtr>& mapShape = m_geographyShapes[*tileKeyPtr];
        map<string, GeographyShapePtr>::iterator shapeIter = mapShape.find(*id);
        if(shapeIter != mapShape.end())
        {
            GeographyShapePtr geographyShapePtr = shapeIter->second;
            GeographyType geographyType = geographyShapePtr->GetShapeType();
            mapShape.erase(shapeIter);
            if(mapShape.empty())
            {
                m_geographyShapes.erase(iter);
            }
            //printf("remove shape success, the id [%s]\n", (*id).c_str());
            //fflush(stdout);
            //refresh tile
            vector<TileKeyPtr> refreshTileKeys;
            TileKeyPtr tmpTileKeyPtr(new TileKey(*tileKeyPtr));
            refreshTileKeys.push_back(tmpTileKeyPtr);
            RefreshShapesOfTiles(refreshTileKeys, geographyType);
            return NE_OK;
        }
    }
//    printf("remove shape faile, the id [%s]\n", (*id).c_str());
//    fflush(stdout);

    return NE_INVAL;
}

void
GeographyLayerImpl::AddShape(TileKeyPtr tileKeyPtr, GeographyShapePtr shape)
{
    if(!shape || !tileKeyPtr)
    {
        return;
    }

    shared_ptr<string> shapeID = shape->GetId();
    if(shapeID)
    {
        m_tileKeyIndexByShapeID[*shapeID] = tileKeyPtr;
    }

    m_geographyShapes[*tileKeyPtr].insert( pair<string, GeographyShapePtr>(*shapeID, shape) );
    //printf("Insert shape tilekey(%d,%d,%d)\n", tileKeyPtr->m_x, tileKeyPtr->m_y, tileKeyPtr->m_zoomLevel); fflush(stdout);
}

void
GeographyLayerImpl::ProcessAddPolylineShape(GeographyShapePtr shape,  int fitZoomLevel)
{
    /*
    Split the shape(only one tile not hold this shape), return map<TileKey, vector<shape> >

    Maybe need this function in the further, but first stage of polyline/polygon need ignore this.
    SplitGeographyShape(shape);

    if this implement, we will invoke the function 'AddTileList'(need refactor) directly.
    ---------------------------------
    */

    GeographyPolyline* geographyPolyline = static_cast<GeographyPolyline*>(shape.get());
    if(!geographyPolyline)
    {
        return;
    }

    PolylinePtr polylinePtr = geographyPolyline->GetPolyline();
    if(!polylinePtr)
    {
        return;
    }

    TileKeyPtr tmpTileKeyPtr(new TileKey());
    tmpTileKeyPtr->m_zoomLevel = fitZoomLevel;
    bool ret = ConvertPolylineToTile(polylinePtr, tmpTileKeyPtr);
    if(!ret)
    {
        return;
    }

    //Use tile key and shape to insert map.
    AddShape(tmpTileKeyPtr, shape);

    //refresh tile
    vector<TileKeyPtr> refreshTileKeys;
    refreshTileKeys.push_back(tmpTileKeyPtr);
    RefreshShapesOfTiles(refreshTileKeys, shape->GetShapeType());
}

void
GeographyLayerImpl::ProcessAddPolygonShape(GeographyShapePtr shape, int fitZoomLevel)
{
    /*
    Split the shape(only one tile not hold this shape), return map<TileKey, vector<shape> >

    Maybe need this function in the further, but first stage of polyline/polygon need ignore this.
    SplitGeographyShape(shape);

    if this implement, we will invoke the function 'AddTileList'(need refactor) directly.
    ---------------------------------
    */

    GeographyPolygon* geographyPolygon  = static_cast<GeographyPolygon*>(shape.get());
    if(!geographyPolygon)
    {
        return;
    }

    PolylinePtr outerBoundaryPtr = geographyPolygon->GetOuterBoundary();
    if(!outerBoundaryPtr)
    {
        return;
    }

    TileKeyPtr tmpTileKeyPtr(new TileKey());
    tmpTileKeyPtr->m_zoomLevel = fitZoomLevel;
    bool ret = ConvertPolylineToTile(outerBoundaryPtr, tmpTileKeyPtr);
    if(!ret)
    {
        return;
    }

    //Use tile key and shape to insert map.
    AddShape(tmpTileKeyPtr, shape);
    /*
     TODO: Add inner boundaries process in the further.
     PolylineListPtr innerBoundaries = geographyPolygon->GetInnerBoundaries();
     */

    //refresh tile
    vector<TileKeyPtr> refreshTileKeys;
    refreshTileKeys.push_back(tmpTileKeyPtr);
    RefreshShapesOfTiles(refreshTileKeys, shape->GetShapeType());
}

bool
GeographyLayerImpl::ConvertPolylineToTile(PolylinePtr polylinePtr, TileKeyPtr tileKeyPtr)
{
    if(!polylinePtr || !tileKeyPtr)
    {
        return false;
    }

    NB_LatitudeLongitude leftTop = {0};
    NB_LatitudeLongitude rightBottom = {0};
    CalcLatLonBoundary(polylinePtr, leftTop, rightBottom);
    if(!IsLatLogBoundaryInOneTile(leftTop, rightBottom, tileKeyPtr))
    {
        //this polyline more than one tile, ignore this shape. add log
        return false;
    }
    return true;
}

void GeographyLayerImpl::ConvertLatLongToTile(double latitude, double longitude, int& xTile, int& yTile)
{
    double xMercator = 0.0;
    double yMercator = 0.0;
    NB_SpatialConvertLatLongToMercator(latitude, longitude, &xMercator, &yMercator);
    NB_SpatialConvertMercatorToTile(xMercator, yMercator, m_tileLayerInfo->refTileGridLevel, &xTile, &yTile);
}

void
GeographyLayerImpl::RefreshShapesOfTiles(const vector<TileKeyPtr>& tileKeys, GeographyType geographyType)
{
    if (m_geographyManager)
    {
        GeographyManagerImpl* manager = static_cast<GeographyManagerImpl*>(m_geographyManager);
        if (manager)
        {
            manager->RefreshShapeTilesOfLayer(tileKeys, m_layerID, geographyType);
        }
    }
}

void
GeographyLayerImpl::CalcLatLonBoundary(PolylinePtr polylinePtr, NB_LatitudeLongitude& leftTop, NB_LatitudeLongitude& rightBottom)
{
    vector<double> latitudes;
    vector<double> longitudes;

    list<GeographyCoordinate>::iterator iter = polylinePtr->begin();
    list<GeographyCoordinate>::iterator end = polylinePtr->end();
    for (; iter != end; ++iter)
    {
        GeographyCoordinate& point = (*iter);
        latitudes.push_back(point.m_latitude);
        longitudes.push_back(point.m_longitude);
    }

    sort(latitudes.begin(), latitudes.end());
    sort(longitudes.begin(), longitudes.end());

    leftTop.latitude = latitudes[0];
    leftTop.longitude = longitudes[longitudes.size() - 1];
    rightBottom.latitude = latitudes[latitudes.size() - 1];
    rightBottom.longitude = longitudes[0];
}

bool
GeographyLayerImpl::IsLatLogBoundaryInOneTile(NB_LatitudeLongitude& leftTop,
                                              NB_LatitudeLongitude& rightBottom,
                                              TileKeyPtr tileKeyPtr)
{
    if(!tileKeyPtr)
    {
        return false;
    }

    int xTileLeftTop = 0;
    int yTileLeftTop = 0;

    int xTileRightBottom = 0;
    int yTileRightBottom = 0;
    ConvertLatLongToTile(leftTop.latitude, leftTop.longitude, xTileLeftTop, yTileLeftTop);
    ConvertLatLongToTile(rightBottom.latitude, rightBottom.longitude, xTileRightBottom, yTileRightBottom);

    if((xTileLeftTop == xTileRightBottom) && (yTileLeftTop == yTileRightBottom))
    {
        tileKeyPtr->m_x = xTileLeftTop;
        tileKeyPtr->m_y = yTileLeftTop;
        return true;
    }
    else
    {
        return false;
    }
}

/* See description in Layer.h */
shared_ptr<string>
GeographyLayerImpl::GetTileDataType()
{
    // Layer ID is data type of this layer.
    return m_layerID;
}

/* See description in Layer.h */
void
GeographyLayerImpl::GetTiles(vector<TileKeyPtr> tileKeys,
                             AsyncTileRequestWithRequestPtr callback,
                             uint32 /*priority*/)
{
    // Tile key cannot be NULL. Also, geography tiles will not be cached, so just skip request if
    // no callback is provided, or we are wasting time.
    if (tileKeys.empty() || !callback)
    {
        return;
    }

    // Check 'm_tileLayerInfo' in function IsValid.
    if (!IsValid() && callback)
    {
        ErrorNotifier functor(callback, NE_NOTINIT);
        for_each (tileKeys.begin(), tileKeys.end(), functor);
        return;
    }

    if (m_geographyShapes.empty())
    {
        // There is no shapes, ignore it.
        ErrorNotifier functor(callback, NE_NOENT);
        for_each (tileKeys.begin(), tileKeys.end(), functor);
        return;
    }

    vector<TileKeyPtr>::const_iterator iter = tileKeys.begin();
    vector<TileKeyPtr>::const_iterator end  = tileKeys.end();

    set<TileKey> tileRequestFilter;

    for (; iter != end; ++iter)
    {
        const TileKeyPtr& tileKey = *iter;

        // Skip out of range requests.
        if (tileKey->m_zoomLevel < (int)m_tileLayerInfo->minZoom ||
            tileKey->m_zoomLevel > (int)m_tileLayerInfo->maxZoom)
        {
            continue;
        }

        /* If zoom level of tile key is greater than reference tile grid zoom level of
           this geography layer, convert this tile key to a tile key of reference tile grid
           zoom level. */
        TileKeyPtr tileKeyToRequest = tileKey;
        int refTileGridLevel = static_cast<int>(m_tileLayerInfo->refTileGridLevel);
        if (tileKeyToRequest->m_zoomLevel > refTileGridLevel)
        {
            tileKeyToRequest = ConvertTileKeyToLessZoomLevel(tileKeyToRequest, refTileGridLevel);
            if (!tileKeyToRequest)
            {
                if (callback)
                {
                    callback->Error(tileKey, NE_UNEXPECTED);
                }
                continue;
            }
        }

        pair<set<TileKey>::iterator, bool> filterResult =
                tileRequestFilter.insert(*tileKeyToRequest);
        if (!filterResult.second) // Skip duplicated requests.
        {
            continue;
        }

        GetTile(tileKey, tileKeyToRequest, callback);
    }
}

void
GeographyLayerImpl::GetTile(TileKeyPtr tileKey, TileKeyPtr convertedTileKey,
                      AsyncTileRequestWithRequestPtr callback)
{
    if (!callback)
    {
        return;
    }

//    printf("=============Enter function GeographyLayerImpl::GetTile()---\n");
//    fflush(stdout);
    // Loop for all shapes to get shapes existing in this tile.
    map<TileKey, vector<GeographyShapePtr> >  shapeMaps;
    GetShapesInTile(convertedTileKey, shapeMaps);
    if (shapeMaps.empty())
    {
        callback->Error(tileKey, NE_NOENT);
        return;
    }

    NB_Error error = NE_OK;
    map<TileKey, vector<GeographyShapePtr> >::const_iterator iter = shapeMaps.begin();
    map<TileKey, vector<GeographyShapePtr> >::const_iterator end  = shapeMaps.end();
    for (; iter != end; ++iter)
    {
        TileImplPtr tile;
        const vector<GeographyShapePtr>& shapesInTile = iter->second;
        if (shapesInTile.empty())
        {
            continue;
        }

        const TileKey& refTileKey = iter->first;

        // Get content ID of the shape tile.
        shared_ptr<string> contentId = GetContentID(refTileKey, shapesInTile);
        if ((!contentId) || (contentId->empty()))
        {
            error = NE_UNEXPECTED;
            continue;
        }

        // Create a tile object to return the data.
        tile.reset(new TileImpl(contentId, m_tileLayerInfo));
        TileKeyPtr refTileKeyPtr(new TileKey(refTileKey));
        if (!tile || !refTileKeyPtr)
        {
            error = NE_NOMEM;
            continue;
        }

        // Use NBM library to generate a shape tile.
        DataStreamPtr tileData = DataStream::Create();
//        printf("!!!!!!!!!!!!!!!!!!!ready to enter GenerateGeographyTileData()!!!!!!!!!!!!!!!!!!!!\n");
//        fflush(stdout);
        error = GenerateGeographyTileData(refTileKey, shapesInTile, tileData);
        if (error != NE_OK)
        {
//            printf("!!!!!!!!!!!!!!!!!!!Faile to exec GenerateGeographyTileData()!!!!!!!!!!!!!!!!!!!!\n");
//            fflush(stdout);
            continue;
        }
//        printf("!!!!!!!!!!!!!!!!!!!Success to exec GenerateGeographyTileData()!!!!!!!!!!!!!!!!!!!!\n");
//        fflush(stdout);
        // Data of the shape tile must not be empty.
        if ((!tileData) || (tileData->GetDataSize() == 0))
        {
            error = NE_UNEXPECTED;
            continue;
        }

        // Set the tile data.
        tile->SetData(tileData);
        tile->SetTileKey(refTileKeyPtr);

        // Set error code to NE_OK.
        error = NE_OK;
        callback->Success(tileKey, tile);
    }

    if (error)
    {
        callback->Error(tileKey, error);
    }
}

NB_Error
GeographyLayerImpl::GenerateGeographyTileData(const TileKey& tileKey,
                                              const vector<GeographyShapePtr>& shapes,
                                              nbcommon::DataStreamPtr& tileData)
{
	// I'm assuming the call is always made with at least one shape/polygon
	if (shapes.empty())
	{
		return NE_INVAL;
	}

    //create nbm tile object
    NBMFileHeader nbmTile;
    nbmTile.SetMaking();
    nbmTile.SetIndex(tileKey.m_x, tileKey.m_y, tileKey.m_zoomLevel);

    NB_Error result = NE_INVAL;

	// All the shapes in the layer have to be of the same type. (either all polygons or all polylines). We report
	// an error if it is not consistent (see below)
	GeographyType geographyType = shapes[0]->GetShapeType();

	switch (geographyType)
	{
	case GEO_GRAPHIC_POLYLINE:
		result = GeneratePolylineTile(nbmTile, shapes);
		break;

	case GEO_GRAPHIC_POLYGON:
		result = GeneratePolygonTile(nbmTile, shapes);
		break;

	default:
		break;
	}

	if (result != NE_OK)
	{
		return result;
	}

    const char* pBuffer = NULL;
    unsigned int bufferSize = 0;

    if (nbmTile.GetTileBuffer(pBuffer, bufferSize))
    {
    	return tileData->AppendData((uint8*)pBuffer, bufferSize);
    }
    else
    {
    	return NE_UNEXPECTED;
    }
}

/* See description in header file */
NB_Error
GeographyLayerImpl::GeneratePolygonTile(NBMFileHeader& nbmTile, const vector<GeographyShapePtr>& shapes)
{
	/*
	 * This function generates a DVA tile from the polygons
	 */

    nbmTile.SetRefInfo((char*)NBM_FILE_REFNAME_AREA);

    // Get material chunk, this chunk is a static chunk and already exists
    NBMMaterialChunk* materialChunk = static_cast<NBMMaterialChunk*>(nbmTile.GetChunk(NBM_CHUNCK_IDENTIFIER_MATERIALS));
    if (!materialChunk)
    {
        // Cannot get material chunk.
        return NE_UNEXPECTED;
    }

    NBMLayerInfo layerInfo;
    layerInfo.layerType = NBM_LAYER_ID_AREA;

    // This is the near/far visibility. We just need to make sure that it is always visible
    layerInfo.highScale = 0;
    layerInfo.lowScale = (float)(7.0e-8);	// We got this from sample code, not sure what the proper value should be

    NBMLayerInfo idLayerInfo;
    idLayerInfo.layerType = NBM_LAYER_ID_ASSOCIATED;
    idLayerInfo.highScale = 0;
    idLayerInfo.lowScale = (float)(7.0e-8);

    NBMIdChunk* idChunk = (NBMIdChunk*)nbmTile.CreateChunk(NBM_CHUNCK_IDENTIFIER_EIDS,
                                                           &idLayerInfo);
    idChunk->SetAvoidDuplication(false);

    // Create area chunk
    NBMAreaChunk* areaChunk = static_cast<NBMAreaChunk*>(nbmTile.CreateChunk(NBM_CHUNCK_IDENTIFIER_AREA, &layerInfo,idChunk));

    // For all polygones in this tile
    vector<GeographyShapePtr>::const_iterator endShape = shapes.end();
    for (vector<GeographyShapePtr>::const_iterator shape = shapes.begin(); shape != endShape; ++shape)
    {
    	// Verify that the shape is a polygon
    	if ((*shape)->GetShapeType() != GEO_GRAPHIC_POLYGON)
    	{
    		return NE_INVAL;
    	}

		const GeographyPolygon& polygon = dynamic_cast<const GeographyPolygon&>(*(*shape));

		/*
			 convert output boundary to polyline chunk
		 */
		PolylinePtr polyline = polygon.GetOuterBoundary();
		if (polyline->empty())
		{
			continue;
		}

		// Convert Geography coordinates to POINTS.
		vector<POINT2> points;
		transform(polyline->begin(), polyline->end(), back_inserter(points), TransformCoordinate);

		// the polyline has to be closed (first and last point are the same) for area tiles. If it's not then we close it
		if ((points.front().x != points.back().x) || (points.front().y != points.back().y))
		{
			points.push_back(points.front());
		}

		// Create polyline chunk
		NBMPolyLineChunk* polylineChunk = static_cast<NBMPolyLineChunk*>(nbmTile.CreateChunk(NBM_CHUNCK_IDENTIFIER_POLYLINE));
        polylineChunk->SetDataPrecision(NBMPrecision_float);
		NBMIndex polylineIndex = polylineChunk->SetData(static_cast<unsigned short>(points.size()), &(points.front()), 1);

		MaterialPtr polygonMaterial = polygon.GetMaterial();

		/*
		 	@todo:
		 	The value MaterialAreaOutlinedBody::outlineWidth has currently no effect! This need to be fixed in NBGM!
		 	See bugs: 164530, 164527
		 */

		// Material for this polygon
		MaterialAreaOutlinedBody material;
		material.interiorColor = polygonMaterial->m_fillColor;
		material.outlineColor  = polygonMaterial->m_edgeColor;
		material.outlineWidth  = polygonMaterial->m_edgeSize;


		// @todo: Edge styles "dash" and "dots" are currently not supported!

		NBMIndex materialIndex = materialChunk->SetData(material);

		// Set material and polyline to area chunk
		areaChunk->SetData(materialIndex, NBM_INVALIDATE_INDEX, polylineIndex);
        shared_ptr<string> id = polygon.GetId();
        if (!id || id->empty())
        {
            id.reset(new string("Unnamed."));
        }

        idChunk->SetData((unsigned char)id->size(), (unsigned char*)id->c_str());
    }

    return NE_OK;
}

/* See description in header file */
NB_Error
GeographyLayerImpl::GeneratePolylineTile(NBMFileHeader& nbmTile, const vector<GeographyShapePtr>& shapes)
{
	/*
	 * This function generates a DVR tile from the polylines
	 */

    nbmTile.SetRefInfo((char*)NBM_FILE_REFNAME_ROAD);

    // Get material chunk, this chunk is a static chunk and already exists
    NBMMaterialChunk* materialChunk = static_cast<NBMMaterialChunk*>(nbmTile.GetChunk(NBM_CHUNCK_IDENTIFIER_MATERIALS));
    if (!materialChunk)
    {
        // Cannot get material chunk.
        return NE_UNEXPECTED;
    }

    NBMLayerInfo layerInfo;
    layerInfo.layerType = NBM_LAYER_ID_ROAD;

    // This is the near/far visibility. We just need to make sure that it is always visible
    layerInfo.highScale = 0;
    layerInfo.lowScale = (float)(7.0e-8);	// We got this from sample code, not sure what the proper value should be

    // Create road chunk
    NBMRoadChunk* roadChunk = static_cast<NBMRoadChunk*>(nbmTile.CreateChunk(NBM_CHUNCK_IDENTIFIER_ROAD, &layerInfo));

    // For all polylines in this tile
    vector<GeographyShapePtr>::const_iterator endShape = shapes.end();
    for (vector<GeographyShapePtr>::const_iterator shape = shapes.begin(); shape != endShape; ++shape)
    {
    	// Verify that the shape is a polyline
    	if ((*shape)->GetShapeType() != GEO_GRAPHIC_POLYLINE)
    	{
    		return NE_INVAL;
    	}

		const GeographyPolyline& geoPolyline = dynamic_cast<const GeographyPolyline&>(*(*shape));
		PolylinePtr polyline = geoPolyline.GetPolyline();
		if (polyline->empty())
		{
			continue;
		}

		// Convert Geography coordinates to POINTS.
		vector<POINT2> points;
		transform(polyline->begin(), polyline->end(), back_inserter(points), TransformCoordinate);

		// Create polyline chunk
		NBMPolyLineChunk* polylineChunk = static_cast<NBMPolyLineChunk*>(nbmTile.CreateChunk(NBM_CHUNCK_IDENTIFIER_POLYLINE));
        polylineChunk->SetDataPrecision(NBMPrecision_float);
		NBMIndex polylineIndex = polylineChunk->SetData(static_cast<unsigned short>(points.size()), &(points.front()), 1);

		MaterialPtr polygonMaterial = geoPolyline.GetMaterial();

		// Material for this polyline
		MaterialOutlinedBody material;
		material.interiorColor = polygonMaterial->m_fillColor;
		material.outlineColor  = polygonMaterial->m_edgeColor;
		material.outlineWidth  = 0;		// This is not needed for polylines, the width is set in the SetData() call below

		// @todo: Edge style is currently not supported!

		NBMIndex materialIndex = materialChunk->SetData(material);

		/*
		 	@todo:
		 	We have to figure out how to handle the polyline width (edge size). The polylines look really bad close up
		 	with the XLarge width. But are still too thin for very far polylines (e.g. from Alaska to New York).

		 	I think the best would be to do the polyline width variable based on the zoom level. NBGM would have to do that.

		 	@todo: The render engine currently just casts the BlackBerry "EdgeStyle" enumeration to "m_edgeSize". This has
		 		   to be changed. We have to either create our own enumeration. Or convert them in absolute values (meters?)
		 */
		double width = 0.0;
		switch (polygonMaterial->m_edgeSize)
		{
			case 1: width = 0.0; 		break;	// None!
			case 2: width = 0.0000005; 	break;	// XSmall
			case 3: width = 0.000002; 	break;	// Small
			case 4: width = 0.000008; 	break;	// Medium
			case 5: width = 0.000032; 	break;	// Large
			case 6: width = 0.000128; 	break;	// XLarge
		}

		// Set material and polyline to road chunk
		roadChunk->SetData(materialIndex, static_cast<float>(width), polylineIndex);	// Second parameter is the polyline width!
    }

    return NE_OK;

}

/* See description in header file. */
void GeographyLayerImpl::GetShapesInTile(TileKeyPtr tileKey, map<TileKey, vector<GeographyShapePtr> >& shapeMaps)
{

    int tileX     = tileKey->m_x;
    int tileY     = tileKey->m_y;
    int zoomLevel = tileKey->m_zoomLevel;
    int distance  = m_tileLayerInfo->refTileGridLevel - zoomLevel;

    // When this functions is called, zoomLevel of input tileKey should not be greater than
    // ref-tile-grid-level of this layer.
    nsl_assert(distance >= 0);

    map<TileKey, map<string, GeographyShapePtr> >::const_iterator iter = m_geographyShapes.begin();
    map<TileKey, map<string, GeographyShapePtr> >::const_iterator end  = m_geographyShapes.end();
    for (; iter != end; ++iter)
    {
        const TileKey& key = iter->first;
        if (tileX != (key.m_x >> distance) || tileY != (key.m_y >> distance))
        {
            continue;
        }

        // If converted and requested tile keys are same, the shape exists in this tile.
//        printf("get tilekey(%d,%d,%d)\n", key.m_x, key.m_y, key.m_zoomLevel);
//        fflush(stdout);

        // If converted and requested tile keys are same, the pin exists in this tile.
        vector<GeographyShapePtr>& shapes = shapeMaps[key];
        map<string, GeographyShapePtr>::const_iterator mapIter = (iter->second).begin();
        map<string, GeographyShapePtr>::const_iterator mapEnd  = (iter->second).end();
        for (; mapIter != mapEnd; ++mapIter)
        {
            shapes.push_back(mapIter->second);
        }
//        printf("In this tile(%d, %d, %d), have [%d] shapes\n", tileKey->m_x, tileKey->m_y, tileKey->m_zoomLevel, shapes.size());
//        fflush(stdout);
    }
}

/*! Convert the tile key to a tile key of specified less zoom level

    @return Converted tile key of specified zoom level
*/
TileKeyPtr
GeographyLayerImpl::ConvertTileKeyToLessZoomLevel(TileKeyPtr sourceTileKey,   /*!< Source tile key */
                                                    int targetZoomLevel         /*!< Specified zoom level to convert */
                                            )
{
    // Check if source tile key is empty.
    if (!sourceTileKey)
    {
        return TileKeyPtr();
    }

    /* If zoom level of source tile key is not greater than target zoom level,
       return source tile key. */
    int sourceZoomLevel = sourceTileKey->m_zoomLevel;
    if (sourceZoomLevel <= targetZoomLevel)
    {
        return sourceTileKey;
    }

    // Create a tile key of target zoom level.
    TileKeyPtr targetTileKey(new TileKey());
    if (!targetTileKey)
    {
        return TileKeyPtr();
    }
    targetTileKey->m_zoomLevel = targetZoomLevel;

    // Convert coordinate x and y with target zoom level.
    int zoomLevelDiffer = sourceZoomLevel - targetZoomLevel;
    targetTileKey->m_x = (sourceTileKey->m_x) >> zoomLevelDiffer;
    targetTileKey->m_y = (sourceTileKey->m_y) >> zoomLevelDiffer;

    return targetTileKey;
}

/*! Get content ID of a shape tile by a tile key

    @return Content ID of the shape tile if success, otherwise return empty shared pointer.
*/
shared_ptr<string>
GeographyLayerImpl::GetContentID(const TileKey& tileKey,    /*!< Tile key of the tile */
                           const vector<GeographyShapePtr>& shapes /*!< Shapes existing in the tile */
                           )
{
    if (shapes.empty() || (!m_layerID) || (m_layerID->empty()))
    {
        return shared_ptr<string>();
    }

    shared_ptr<string> contentId(new string(SHAPE_CONTENT_ID_TEMPLATE));
    if (!contentId)
    {
        return shared_ptr<string>();
    }

    // Use CRC32 to get an unified ID by shape IDs existing in this tile.
    string stringToCrc;
    vector<GeographyShapePtr>::const_iterator shapeIterator = shapes.begin();
    vector<GeographyShapePtr>::const_iterator shapeEnd      = shapes.end();
    for (; shapeIterator != shapeEnd; ++shapeIterator)
    {
        const GeographyShapePtr& shape = *shapeIterator;
        if (!shape)
        {
            continue;
        }

        // Get shape ID.
        shared_ptr<string> shapeId = shape->GetId();
        if ((!shapeId) || shapeId->empty())
        {
            continue;
        }

        stringToCrc += *shapeId;
    }

    // If two tiles have same unified ID, both tiles contain same shapes.
    uint32 unifiedId = crc32(CRC_VALUE, (const byte*) (stringToCrc.c_str()),
                             stringToCrc.size());

    // Format template with layer ID.
    StringUtility::ReplaceString(*contentId, PARAMETER_SHAPE_LAYER_ID, *m_layerID);

    // Format template with tile key.
    StringUtility::ReplaceString(*contentId, PARAMETER_SHAPE_TILE_X,
                                 StringUtility::NumberToString(tileKey.m_x));

    StringUtility::ReplaceString(*contentId, PARAMETER_SHAPE_TILE_Y,
                                 StringUtility::NumberToString(tileKey.m_y));

    StringUtility::ReplaceString(*contentId, PARAMETER_SHAPE_ZOOM_LEVEL,
                                 StringUtility::NumberToString(tileKey.m_zoomLevel));

    // Format template with unified ID.
    StringUtility:: ReplaceString(*contentId, PARAMETER_SHAPE_UNIFIED_ID,
                                  StringUtility::NumberToString(unifiedId));

    return contentId;
}

bool GeographyLayerImpl::IsValid()
{
    bool bValid = true;
    if (!m_tileLayerInfo)
    {
        bValid = false;
    }
    return bValid;
}

/* See description in header file. */
NB_Error GeographyLayerImpl::GetCachedTiles(const vector <TileKeyPtr>& /*tileKeys*/,
                                                    AsyncTileRequestWithRequestPtr /*callback*/)
{
    //@todo: Implement this if necessary
    return NE_OK;
}

/* See description in Layer.h */
void
GeographyLayerImpl::RemoveAllTiles()
{

    NB_ASSERT_CCC_THREAD(m_pContext);

    // Nothing to do here.
}

/*! @} */
