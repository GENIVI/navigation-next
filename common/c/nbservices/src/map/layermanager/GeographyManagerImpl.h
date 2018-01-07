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

    @file       GeographyManagerImpl.h

    Class GeographyManagerImpl inherits from GeographyManager interface.

    @todo: Add description of this class.

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

#ifndef _GEOGRAPHYMANAGERIMPL_H
#define _GEOGRAPHYMANAGERIMPL_H

/*!
    @addtogroup nbmap
    @{
*/

extern "C"
{
}

#include "base.h"
#include "LayerManager.h"
#include "GeographyManager.h"
#include "GeographyLayerImpl.h"

namespace nbmap
{

// Types ........................................................................................

class LayerProvider;
class LayerIdGenerator;
class GeographyOperationImpl;

typedef vector< pair<double, string> > GEOGRAPHY_LAYERS;

/*! Configuration for all geography layers

    Modify this configuration if you want to add/remove/change the geography reference layers
*/
class GeographyLayerConfiguration
{
public:
       GeographyLayerConfiguration()
       {
              // Add all the layers (in ascending order)
              /*
                     Add/Remove/Modify the layers if necessary
              */
              m_layers.push_back( make_pair(1.0, "1"));
              m_layers.push_back( make_pair(4.0, "4"));
              m_layers.push_back( make_pair(8.0, "8"));
              m_layers.push_back( make_pair(12.0, "12"));
              m_layers.push_back( make_pair(16.0, "16"));
       }

       /*! Get all layers in ascending order of the reference zoom level */
       void GetLayers(GEOGRAPHY_LAYERS& layers) const
       {
              layers = m_layers;
       }

private:
       /*! Layers with reference zoom level and name-appending for the layers */
       GEOGRAPHY_LAYERS m_layers;
};


/*! */
class GeographyManagerImpl : public Base,
                             public GeographyManager
{
public:
    // Public functions .........................................................................

    /*! GeographyManagerImpl constructor
     *
     *  Will create and Initialize all variables.
     */
    GeographyManagerImpl(NB_Context* context,
                         shared_ptr<LayerProvider> layerProvider,
                         LayerManagerPtr layerManager,
                         shared_ptr<LayerIdGenerator> idGenerator);

    /*! Light weight constructor
     *
     *  Will create GeographyManagerImpl, but will not fully initialized it.
     */
    GeographyManagerImpl(NB_Context* context);

    /*! GeographyManagerImpl destructor */
    virtual ~GeographyManagerImpl();

    /* See description in GeographyManager.h */
    virtual shared_ptr<GeographyOperation> AddGeographyLayerSuit(const string& layerId);
    virtual void RemoveGeographyLayerSuit(const string& layerId);

    void Initialize(shared_ptr<LayerProvider> layerProvider,
                    LayerManagerPtr layerManager,
                    shared_ptr < LayerIdGenerator > idGenerator);

    void RefreshShapeTilesOfLayer(const vector<TileKeyPtr>& tileKeys,/*!< Tile keys to refresh */
                                  shared_ptr<string> layerID,        /*!< Shape layer to refresh tiles */
                                  GeographyType geographyType
                             );

    shared_ptr<GeographyLayerImpl> CreateGeographyLayer(const string& layerId);
    void RemoveGeographyLayer(shared_ptr<GeographyLayerImpl> geographyLayerPtr);
    void GetGeographyLayerConfiguration(GEOGRAPHY_LAYERS& layers);

private:

    // Private members ..........................................................................
    NB_Context*      m_pContext;                                /*!< NB_Context instance. */
    LayerManagerPtr  m_pLayerManager;
    shared_ptr<LayerProvider> m_layerProvider;                       /*!< A layer provider */
    shared_ptr<LayerIdGenerator> m_pLayerIdGenerator;
    GeographyLayerConfiguration m_config;
    /*!< A map contained all geography layers.
       Layer IDs are keys of this map.
    */
    map<string, shared_ptr<GeographyOperationImpl> > m_layerMap;
};

/*! */
class GeographyOperationImpl : public GeographyOperation
{
public:
    GeographyOperationImpl(GeographyManagerImpl* geographyManager);
    virtual ~GeographyOperationImpl();
    /* See description in GeographyOperation.h */
    virtual NB_Error AddGeographyShape(GeographyShapePtr shape);
    virtual NB_Error RemoveGeographyShape(shared_ptr<string> id);
    virtual NB_Error UpdateGeographyShape(GeographyShapePtr shape);

    void RemoveAllLayers();
    void InsertGeograpyLayer(string zoomLevelStr, shared_ptr<GeographyLayerImpl> geographyLayerPtr, bool isPolygon = true);
    shared_ptr<GeographyLayerImpl> GetGeographyLayer(int fitZoomLevel, GeographyType geographyType);

private:

    string GetFitZoomLevelForPolyshape(GeographyShapePtr shape);
    void CalcLatLonBoundaryEx(PolylinePtr polylinePtr, NB_LatitudeLongitude& leftTop, NB_LatitudeLongitude& rightBottom);
    void ConvertLatLongToTileEx(double latitude, double longitude, int& xTile, int& yTile, int zoomLevel);
    bool IsInOneTile(PolylinePtr polylinePtr, int zoomlevel);

    GeographyManagerImpl* m_geographyManager;
    map<string, shared_ptr<GeographyLayerImpl> > m_geographyLayers;
    map<string, shared_ptr<GeographyLayerImpl> > m_geopolylineLayers;

    /*! This map be used for removing shape fast.

        Use the shape id as the map key, the pair is <fitZoomLevel, isPolyline>.
        All of these will speed up the removing and avoid the linear search
     */
    map<string, pair<string, bool> > m_shapesInfo;
};

}  // namespace nbmap

/*! @} */

#endif  // _GEOGRAPHYMANAGERIMPL_H
