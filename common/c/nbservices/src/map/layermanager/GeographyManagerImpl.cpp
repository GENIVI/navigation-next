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

    @file       GeographyManagerImpl.cpp

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

#include "GeographyManagerImpl.h"
#include "GeographyOperation.h"
#include "LayerProvider.h"
#include "LocalLayerProvider.h"
#include "UnifiedLayerManager.h"
#include "LayerIdGenerator.h"
#include "palmath.h"
#include "nbspatial.h"
#include <vector>
#include <algorithm>

/*! @{ */

using namespace std;
using namespace nbmap;

// Public functions .............................................................................

/* See description in GeographyOperation.h */
GeographyOperationImpl::GeographyOperationImpl(GeographyManagerImpl* geographyManager)
{
    m_geographyManager = geographyManager;
}

GeographyOperationImpl::~GeographyOperationImpl()
{

}

string
GeographyOperationImpl::GetFitZoomLevelForPolyshape(GeographyShapePtr shape)
{
//    printf("---Enter function GeographyOperationImpl::GetFitZoomLevelForPolyshape()---\n"); fflush(stdout);
    string fitZoomLevel;
    if(!shape)
    {
        return fitZoomLevel;
    }

    PolylinePtr polylinePtr;
    if(shape->GetShapeType() == GEO_GRAPHIC_POLYLINE)
    {
        GeographyPolyline* geographyPolyline = static_cast<GeographyPolyline*>(shape.get());
        if(!geographyPolyline)
        {
            return fitZoomLevel;
        }

        polylinePtr = geographyPolyline->GetPolyline();
        if(!polylinePtr)
        {
            return fitZoomLevel;
        }
    }else if(shape->GetShapeType() == GEO_GRAPHIC_POLYGON)
    {
        GeographyPolygon* geographyPolygon = static_cast<GeographyPolygon*>(shape.get());
        if(!geographyPolygon)
        {
            return fitZoomLevel;
        }

        polylinePtr = geographyPolygon->GetOuterBoundary();
        if(!polylinePtr)
        {
            return fitZoomLevel;
        }
    }else if(shape->GetShapeType() == GEO_GRAPHIC_INVALID)
    {
        return fitZoomLevel;
    }

    if(!polylinePtr)
    {
        return fitZoomLevel;
    }

    if (polylinePtr->size() < 2)
    {
        return fitZoomLevel;
    }

    GEOGRAPHY_LAYERS configLayers;
    m_geographyManager->GetGeographyLayerConfiguration(configLayers);
    if(configLayers.empty())
    {
        return fitZoomLevel;
    }

    for (int i = configLayers.size() -1; i >= 0; --i)
    {
       if(IsInOneTile(polylinePtr, (int)(configLayers[i].first)))
       {
           fitZoomLevel = configLayers[i].second;
           break;
       }
    }

//    printf("The shape id: [%s], fit zoom level is: [%s]\n", shape->GetId()->c_str(), fitZoomLevel.c_str()); fflush(stdout);
    return fitZoomLevel;
}

NB_Error
GeographyOperationImpl::AddGeographyShape(GeographyShapePtr shape)
{
//    printf("---Enter function GeographyOperationImpl::AddGeographyShape()---\n"); fflush(stdout);

    if(!shape)
    {
        return NE_INVAL;
    }

    GeographyType geographyType = shape->GetShapeType();
    if(geographyType <= GEO_GRAPHIC_INVALID || geographyType > GEO_GRAPHIC_POLYGON)
    {
        return NE_INVAL;
    }

    shared_ptr<string> shapeId = shape->GetId();
    if(shapeId->empty())
    {
        return NE_INVAL;
    }

    string fitZoomLevelStr = GetFitZoomLevelForPolyshape(shape);
    if(fitZoomLevelStr.empty())
    {
//        printf("The fit zoom level < 1, this shape will be ignore!!!\n"); fflush(stdout);
        return NE_INVAL;
    }

    map<string, shared_ptr<GeographyLayerImpl> >::const_iterator layerIterator;
    if(geographyType == GEO_GRAPHIC_POLYGON)
    {
        layerIterator = m_geographyLayers.find(fitZoomLevelStr);
        if (layerIterator == m_geographyLayers.end())
        {
            return NE_INVAL;
        }
        pair<string, bool>& shapeInfoPair = m_shapesInfo[*shapeId];
        shapeInfoPair.first = fitZoomLevelStr;
        shapeInfoPair.second = false;
    }
    else
    {
        layerIterator = m_geopolylineLayers.find(fitZoomLevelStr);
        if (layerIterator == m_geopolylineLayers.end())
        {
            return NE_INVAL;
        }
        pair<string, bool>& shapeInfoPair = m_shapesInfo[*shapeId];
        shapeInfoPair.first = fitZoomLevelStr;
        shapeInfoPair.second = true;
    }

    shared_ptr<GeographyLayerImpl> geographyLayerPtr = layerIterator->second;
    if(!geographyLayerPtr)
    {
        return NE_NOENT;
    }

    return (geographyLayerPtr->AddGeographyShape(shape, atoi(fitZoomLevelStr.c_str())));
}

NB_Error
GeographyOperationImpl::RemoveGeographyShape(shared_ptr<string> id)
{
//    printf("---Enter function GeographyOperationImpl::RemoveGeographyShape()---\n");
//    printf("The remove shape id: %s\n", id->c_str());
//    fflush(stdout);

    if(id && id->empty())
    {
        return NE_INVAL;
    }

    map<string, pair<string, bool> >::iterator shapeInfoIter = m_shapesInfo.find(*id);
    if(shapeInfoIter == m_shapesInfo.end())
    {
        return NE_INVAL;
    }

    string fitZoomLevel = (shapeInfoIter->second).first;
    bool isPolyline = (shapeInfoIter->second).second;
    m_shapesInfo.erase(shapeInfoIter);
    if(fitZoomLevel.empty())
    {
        return NE_INVAL;
    }

    map<string, shared_ptr<GeographyLayerImpl> >::iterator geoLayerIter;
    if(isPolyline)
    {
        geoLayerIter = m_geopolylineLayers.find(fitZoomLevel);
        if(geoLayerIter == m_geopolylineLayers.end())
        {
            return NE_INVAL;
        }
    }
    else
    {
        geoLayerIter = m_geographyLayers.find(fitZoomLevel);
        if(geoLayerIter == m_geographyLayers.end())
        {
            return NE_INVAL;
        }
    }

    shared_ptr<GeographyLayerImpl> geographyLayerImpl = geoLayerIter->second;
    if(!geographyLayerImpl)
    {
        return NE_INVAL;
    }

//    printf("find this shape id[%s], isPolyline[%d], fitZoomLevel[%s]\n", (*id).c_str(), isPolyline?1:0, fitZoomLevel.c_str()); fflush(stdout);
    return geographyLayerImpl->RemoveGeographyShape(id);
}

NB_Error
GeographyOperationImpl::UpdateGeographyShape(GeographyShapePtr shape)
{
    NB_Error ret = RemoveGeographyShape(shape->GetId());
    if(ret != NE_OK)
    {
        return ret;
    }

    return AddGeographyShape(shape);
}

void
GeographyOperationImpl::RemoveAllLayers()
{
    //Delete all polygon layers
    map<string, shared_ptr<GeographyLayerImpl> >::iterator iter = m_geographyLayers.begin();
    map<string, shared_ptr<GeographyLayerImpl> >::iterator end = m_geographyLayers.end();
    for (; iter != end; ++iter)
    {
        shared_ptr<GeographyLayerImpl> geographyLayerPtr = (*iter).second;
        if(geographyLayerPtr)
        {
            m_geographyManager->RemoveGeographyLayer(geographyLayerPtr);
        }
    }

    //Delete all polyline layers
    iter = m_geopolylineLayers.begin();
    end = m_geopolylineLayers.end();
    for (; iter != end; ++iter)
    {
        shared_ptr<GeographyLayerImpl> geographyLayerPtr = (*iter).second;
        if(geographyLayerPtr)
        {
            m_geographyManager->RemoveGeographyLayer(geographyLayerPtr);
        }
    }

}

shared_ptr<GeographyLayerImpl>
GeographyOperationImpl::GetGeographyLayer(int fitZoomLevel, GeographyType geographyType)
{
    if(fitZoomLevel <= 0)
    {
        return shared_ptr<GeographyLayerImpl>();
    }

    char buffer[10];
    nsl_memset(buffer, 0, 10 * sizeof(char));
    sprintf(buffer, "%d", fitZoomLevel);
    string zoomLevel(buffer);

    map<string, shared_ptr<GeographyLayerImpl> >::const_iterator layerIterator;
    if(geographyType == GEO_GRAPHIC_POLYGON)
    {
        layerIterator = m_geographyLayers.find(zoomLevel);
        if (layerIterator == m_geographyLayers.end())
        {
            return shared_ptr<GeographyLayerImpl>();
        }
    }
    else
    {
        layerIterator = m_geopolylineLayers.find(zoomLevel);
        if (layerIterator == m_geopolylineLayers.end())
        {
            return shared_ptr<GeographyLayerImpl>();
        }
    }

    return ((*layerIterator).second);
}

/* See declaration for description */
GeographyManagerImpl::GeographyManagerImpl(NB_Context* context,
                                           LayerProviderPtr layerProvider,
                                           LayerManagerPtr layerManager,
                                           shared_ptr<LayerIdGenerator> idGenerator)
        : m_pContext(context),
          m_pLayerManager(layerManager),
          m_layerProvider(layerProvider),
          m_pLayerIdGenerator(idGenerator)
{
}

/* See declaration for description */
GeographyManagerImpl::GeographyManagerImpl(NB_Context* context)
: m_pContext(context)
{
}

/* See declaration for description */
GeographyManagerImpl::~GeographyManagerImpl()
{
}

void GeographyManagerImpl::Initialize(LayerProviderPtr layerProvider,
                                      LayerManagerPtr layerManager,
                                      shared_ptr < LayerIdGenerator > idGenerator)
{
    NB_ASSERT_CCC_THREAD(m_pContext);

    m_layerProvider         = layerProvider;
    m_pLayerManager         = layerManager;
    m_pLayerIdGenerator     = idGenerator;
}

void
GeographyOperationImpl::InsertGeograpyLayer(string zoomLevelStr, shared_ptr<GeographyLayerImpl> geographyLayerPtr, bool isPolygon)
{
    if(isPolygon)
    {
        m_geographyLayers.insert(make_pair(zoomLevelStr,geographyLayerPtr));
    }
    else
    {
        m_geopolylineLayers.insert(make_pair(zoomLevelStr,geographyLayerPtr));
    }

}

shared_ptr<GeographyOperation>
GeographyManagerImpl::AddGeographyLayerSuit(const string& layerId)
{
    if (layerId.empty())
    {
        return shared_ptr<GeographyOperation>();
    }

    // Check if this layer ID exists.
    map<string, shared_ptr<GeographyOperationImpl> >::const_iterator layerIterator = m_layerMap.find(layerId);
    if (layerIterator != m_layerMap.end())
    {
        return layerIterator->second;
    }

    shared_ptr<GeographyOperationImpl> geographyOperationPtr(new GeographyOperationImpl(this));
    if(!geographyOperationPtr)
    {
        return shared_ptr<GeographyOperation>();
    }

    GEOGRAPHY_LAYERS configLayers;
    m_config.GetLayers(configLayers);
    if(configLayers.empty())
    {
        return shared_ptr<GeographyOperation>();
    }

    for (size_t i = 0; i < configLayers.size(); ++i)
    {
        //create polygon layers for zoom levels, based on the configuration
        shared_ptr<GeographyLayerImpl> polygonLayer = CreateGeographyLayer(string(layerId + configLayers[i].second + "Polygon"));
        if(!polygonLayer)
        {
            //TODO: need more think about how to handle the fail condition...
            return shared_ptr<GeographyOperation>();
        }
        geographyOperationPtr->InsertGeograpyLayer(configLayers[i].second, polygonLayer);

        //create polyline layers for zoom levels, based onthe configuration
        shared_ptr<GeographyLayerImpl> polylineLayer = CreateGeographyLayer(string(layerId + configLayers[i].second + "Polyline"));
        if(!polylineLayer)
        {
            //TODO: need more think about how to handle the fail condition...
            return shared_ptr<GeographyOperation>();
        }
        geographyOperationPtr->InsertGeograpyLayer(configLayers[i].second, polylineLayer, false);
    }

    m_layerMap.insert(make_pair(layerId, geographyOperationPtr));

    return geographyOperationPtr;
}

/* See description in GeographyManager.h */
shared_ptr<GeographyLayerImpl>
GeographyManagerImpl::CreateGeographyLayer(const string& layerId)
{
    NB_ASSERT_CCC_THREAD(m_pContext);

    // If layer ID is not valid return an empty pointer.
    if (layerId.empty() || !m_pLayerIdGenerator)
    {
        return shared_ptr<GeographyLayerImpl>();
    }

    //Create a layer add it to map and return it.
    uint32 layerIdDigital = m_pLayerIdGenerator->GenerateId();
    //Create a new Shared pointer for the Layer
    shared_ptr<string> layerIdPtr = shared_ptr<string>(new string(layerId));
    shared_ptr<GeographyLayerImpl> geographyLayerPtr(new GeographyLayerImpl(layerIdPtr, layerIdDigital, this, m_pContext));
    //Check if memory allocation fails.
    if(!geographyLayerPtr)
    {
        //return a null pointer if memory allocation fails
        return shared_ptr<GeographyLayerImpl>();
    }

    //Add the Layer to the Layer Provider
    LocalLayerProvider* geographyLayerProvider = static_cast<LocalLayerProvider*>(m_layerProvider.get());
    if (geographyLayerProvider)
    {
        bool needToUpdate = geographyLayerProvider->AddLayer(geographyLayerPtr);
        if (needToUpdate)
        {
            UnifiedLayerManager* layerManager =
                    static_cast<UnifiedLayerManager*>(m_pLayerManager.get());
            if (layerManager)
            {
                vector<LayerPtr> layers;
                layers.push_back(geographyLayerPtr);
                layerManager->NotifyLayersAdded(layers);
            }
        }
    }
    else
    {
        return shared_ptr<GeographyLayerImpl>();
    }
    return geographyLayerPtr;
}

void
GeographyManagerImpl::RemoveGeographyLayerSuit(const string& layerId)
{
    // Find geography layer suit to remove.
    if(layerId.empty())
    {
        return;
    }

    map<string, shared_ptr<GeographyOperationImpl> >::iterator operationIterator = m_layerMap.find(layerId);
    if (operationIterator == m_layerMap.end())
    {
        return;
    }

    shared_ptr<GeographyOperationImpl> removedOperation = operationIterator->second;
    removedOperation->RemoveAllLayers();

    // Remove this geography operation from manager.
    m_layerMap.erase(operationIterator);
}

/* See description in GeographyManager.h */
void
GeographyManagerImpl::RemoveGeographyLayer(shared_ptr<GeographyLayerImpl> geographyLayerPtr)
{
    NB_ASSERT_CCC_THREAD(m_pContext);

    if (!geographyLayerPtr)
    {
        return;
    }

    // Remove this geography layer from layer provider.
    LocalLayerProvider* layerProvider = dynamic_cast<LocalLayerProvider*>(m_layerProvider.get());
    if (layerProvider)
    {
        bool needToUpdate = layerProvider->RemoveLayer(geographyLayerPtr);
        if (needToUpdate)
        {
            UnifiedLayerManager* layerManager =
                    static_cast<UnifiedLayerManager*>(m_pLayerManager.get());
            if (layerManager)
            {
                vector<LayerPtr> layers;
                layers.push_back(geographyLayerPtr);
                layerManager->NotifyLayersRemoved(layers);
            }
        }
    }
}

void
GeographyManagerImpl::RefreshShapeTilesOfLayer(const vector<TileKeyPtr>& tileKeys, shared_ptr<string> layerID, GeographyType geographyType)
{
    NB_ASSERT_CCC_THREAD(m_pContext);

    // Check if layer ID is empty.
    if ((tileKeys.empty()) || (!layerID))
    {
        return;
    }

    map<string, shared_ptr<GeographyOperationImpl> >::iterator iter = m_layerMap.begin();
    map<string, shared_ptr<GeographyOperationImpl> >::iterator end = m_layerMap.end();
    for (; iter != end; ++iter)
    {
        const string& tmpLayerId = (*iter).first;
        size_t found = layerID->find(tmpLayerId);
        if(found != string::npos)
        {
            shared_ptr<GeographyOperationImpl> geographyOperationPtr = (*iter).second;
            if(!geographyOperationPtr)
            {
                return;
            }
            TileKeyPtr tmpTileKey = tileKeys[0];
            shared_ptr<GeographyLayerImpl> geographyLayer = geographyOperationPtr->GetGeographyLayer(tmpTileKey->m_zoomLevel, geographyType);

            // Notify layer manager to update a tile of the layer.
            if (m_pLayerManager && geographyLayer)
            {
                UnifiedLayerManager* layerManager = static_cast<UnifiedLayerManager*>(m_pLayerManager.get());
                if (layerManager)
                {
                    layerManager->RefreshTiles(tileKeys, geographyLayer);
//                    printf("RefreshTilesOfLayer, layerId:%s, tileKeys.x: %d, tileKeys.y: %d, tileKeys.zoomLevel: %d\n",
//                            layerID->c_str(), tileKeys[0]->m_x, tileKeys[0]->m_y, tileKeys[0]->m_zoomLevel);
//                    fflush(stdout);
                    return;
                }
            }
        }
    }
}

void
GeographyManagerImpl::GetGeographyLayerConfiguration(GEOGRAPHY_LAYERS& layers)
{
    m_config.GetLayers(layers);
}

bool
GeographyOperationImpl::IsInOneTile(PolylinePtr polylinePtr, int zoomlevel)
{
    if(!polylinePtr)
    {
        return false;
    }

    NB_LatitudeLongitude leftTop = {0};
    NB_LatitudeLongitude rightBottom = {0};
    CalcLatLonBoundaryEx(polylinePtr, leftTop, rightBottom);

    int xTileLeftTop = 0;
    int yTileLeftTop = 0;

    int xTileRightBottom = 0;
    int yTileRightBottom = 0;
    ConvertLatLongToTileEx(leftTop.latitude, leftTop.longitude, xTileLeftTop, yTileLeftTop, zoomlevel);
    ConvertLatLongToTileEx(rightBottom.latitude, rightBottom.longitude, xTileRightBottom, yTileRightBottom, zoomlevel);

    if((xTileLeftTop == xTileRightBottom) && (yTileLeftTop == yTileRightBottom))
    {
        return true;
    }
    else
    {
        return false;
    }
}

void
GeographyOperationImpl::ConvertLatLongToTileEx(double latitude, double longitude, int& xTile, int& yTile, int zoomLevel)
{
    double xMercator = 0.0;
    double yMercator = 0.0;
    NB_SpatialConvertLatLongToMercator(latitude, longitude, &xMercator, &yMercator);
    NB_SpatialConvertMercatorToTile(xMercator, yMercator, zoomLevel, &xTile, &yTile);
}

void
GeographyOperationImpl::CalcLatLonBoundaryEx(PolylinePtr polylinePtr, NB_LatitudeLongitude& leftTop, NB_LatitudeLongitude& rightBottom)
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

/*! @} */
