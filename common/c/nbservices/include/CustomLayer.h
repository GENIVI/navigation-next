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
@file     CustomLayer.h
@defgroup nbmap

*/
/*
(C) Copyright 2014 by TeleCommunication Systems, Inc.

The information contained herein is confidential, proprietary
to TeleCommunication Systems, Inc., and considered a trade secret as
defined in section 499C of the penal code of the State of
California. Use of this information by anyone other than
authorized employees of TeleCommunication Systems, is granted only
under a written non-disclosure agreement, expressly
prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
/*! @{ */

#ifndef __CUSTOMLAYER__
#define __CUSTOMLAYER__

#include "paltypes.h"
#include "nbexp.h"
#include "navpublictypes.h"
#include "smartpointer.h"
#include "base.h"
#include "datastream.h"
#include "TileKey.h"
#include "Layer.h"
#include <vector>
#include <string>
#include "PinLayer.h"
#include "UnifiedLayer.h"
#include "Pin.h"
#include "CustomLayerManager.h"
#include "PinMaterialParser.h"
#include "CustomNBMTile.h"

class NBMFileHeader;
class TileProvider;

namespace nbmap
{

/*! Provides a definition of a customLayer within mapview. Allows a client of mapview to get displayable name
    and toggle the display of the custom layer. */
class CustomLayer : public UnifiedLayer,
                    public PinLayer
{
public:
    // Public functions .........................................................................

    /*! CustomLayer constructor */
    CustomLayer(NB_Context* context,                    /*!< NB_Context instance */
                shared_ptr<string> layerID,             /*!< An ID to identify the layer */
                shared_ptr<string> materialConfigPath,  /*!< Path of a XML file contained material config information */
                uint32  layerIdDigital,                 /*!< Digital layer ID */
                CustomLayerManager* customLayerManager, /*!< A pointer to a custom layer manager */
                TileProviderPtr provider,               /*!< The pointer of tile provider.*/
                int mainOrder,                          /*!< The custom layer draw order.*/
                int subOrder,                           /*!< The custom layer sub draw order.*/
                int refZoom,                            /*!< Reference grid level.*/
                int minZoom,                            /*!< Min zoon level in this layer.*/
                int maxZoom,                            /*!< Max zoon level in this layer.*/
                bool visible                            /*!< Make the this layer visible or not.*/
                );

    /*! CustomLayer destructor */
    virtual ~CustomLayer();

    /* See description in Layer.h */
    virtual shared_ptr<string> GetTileDataType();

    virtual void GetTiles(vector<TileKeyPtr> tileKeys,
                         AsyncTileRequestWithRequestPtr callback,
                         uint32 priority);
    virtual NB_Error GetCachedTiles(const vector <TileKeyPtr>& tileKeys,
                                    AsyncTileRequestWithRequestPtr callback);
    virtual void RemoveAllTiles();

    /* See description in PinLayer.h */
    virtual shared_ptr<string> GetLayerID();
    virtual void RemovePins(const vector<shared_ptr<string> >& pinIDs);
    virtual void RemoveAllPins();
    virtual PinPtr GetPin(shared_ptr<string> pinID);
    virtual vector<PinPtr> GetAllPins();
    virtual void SetMaterialConfigPath(shared_ptr<string> materialConfigPath);
    virtual shared_ptr<vector<NB_LatitudeLongitude> > GetAllPinCoordinates();
    virtual uint32 GetID();

    /*! Check whether this CustomLayer is in valid state.

        This function should be called right after creating CustomLayer. It checks whether
        associated TileLayerInfo is valid. If it is invalid, user should not use this
        CustomLayer.

        @return true if valid.
    */
    bool IsValid();

    void LoadTile(CustomNBMTilePtr customNBMTilePtr);
    bool GetMarkerID(const std::string& internalMarkerID, int& userMarkerID);
    /**
     * Notifies the custom layer the tile in this specified location is invalidate.

     * @param x The x coordinate of the tile. This will be in the range [0, 2^zoom - 1] inclusive.
     * @param y The y coordinate of the tile. This will be in the range [0, 2^zoom - 1] inclusive.
     * @param zoom The zoom level of the tile. This will be in the range (2, 22] inclusive.
     * @return None.
     */
    void Invalidate(int x, int y, int zoom);

    /**
     * Notifies the custom layer the tiles of this specified zoom lever are invalidate.

     * @param zoom The zoom level of the tile. This will be in the range (2, 22] inclusive.
     * @return None.
     */
    void Invalidate(int zoom);

    /**
     * Notifies the custom layer all the custom tiles are invalidate.

     * @return None.
     */
    void Invalidate();

    virtual std::string className() const { return "CustomLayer"; }

private:
    // Private types ..........................................................................

    typedef map<TileKey, vector<PinPtr> > TILEKEY_PINVEC_MAP;

    // Private functions ......................................................................

    // Copy constructor and assignment operator are not supported.
    CustomLayer(const CustomLayer& layer);
    CustomLayer& operator=(const CustomLayer& layer);

    /* See description in PinLayer.h */
    virtual shared_ptr<string> GeneratePinID();
    virtual TileKeyPtr AddPinPtr(PinPtr pin);
    virtual void RefreshPinsOfTiles(const vector<TileKeyPtr>& tileKeys);
    virtual void RefreshAllPins();

    /* See source file for description */

    void NotifyPinsRemoved(shared_ptr<vector<PinPtr> > pins);
    TileKeyPtr ConvertTileKeyToLessZoomLevel(TileKeyPtr sourceTileKey,
                                             int targetZoomLevel);
    TileKeyPtr GetReferenceTileKeyByCoordinate(double latitude,
                                               double longitude);
    void GetTileByCoordinate(double latitude,
                             double longitude,
                             int zoomLevel,
                             int& tileX,
                             int& tileY);
    void GetCoordinateByTile(int tileX,
                             int tileY,
                             int zoomLevel,
                             double& latitude,
                             double& longitude);
    double GetBoundMercator(double mercator);

    shared_ptr<string> GetContentID(const TileKey& tileKey);

    void GetTile(TileKeyPtr tileKey, TileKeyPtr convertedTileKey,
                 AsyncTileRequestWithRequestPtr callback);

    // Refer to parent class for description.
    virtual void PostLayerToggled();

    static void UI_Interest(PAL_Instance* pal, void* userData);

    void AppendMarkerID(const std::map<std::string, int>& markerIDMap);

// Private members ........................................................................
    uint32 m_currentGeneratedID;               /*!< Current generated pin ID */
    uint32 m_layerIdDigital;
    string m_IdSeperator;                      /*!< Separator for layer id and pin Id.*/
    shared_ptr<string> m_layerID;              /*!< An ID to identify this layer */
    shared_ptr<string> m_projection;           /*!< Projection of this layer */
    shared_ptr<string> m_country;              /*!< Country of this layer */
    shared_ptr<string> m_materialConfigPath;   /*!< Path of a XML file contained material
                                                    config information */
    CustomLayerManager* m_customLayerManager;  /*!< Instance of custom layer manager that created the
                                                    custom Layer. CustomLayer will use this
                                                    CustomLayerManager to create PinIDs. */
    /*! Collection of added pins.

        All pins are organized by its TileKey. Although this makes it a little difficult to
        find pin by its id, it will speed up GetTile(). And GetTile() is called very
        frequently if this layer is enabled.
     */
    map<TileKey, vector<PinPtr> > m_Pins;

    PinMaterialParser           m_materialParser;    /*!< MaterialParser to parse PinMaterials  */
    shared_ptr<const string>  m_materialCategory;
    TileProviderPtr m_provider;
    int m_mainOrder;
    int m_subOrder;
    int m_refZoom;
    int m_minZoom;
    int m_maxZoom;
    bool m_visible;
    map<TileKey, nbcommon::DataStreamPtr> m_cache;
    PAL_Instance* m_pal;
    /*! This map store all marker data that belongs to itself

        The string KEY is meaning that the marker ID creating by TileFactory internal.
        The int VALUE is meaning that the marker ID creating by user.
    */
    std::map<std::string, int> m_markerIDMap;

    //Store the refresh tiles when user invoke the interface of 'invalidate(x, y, z)' or 'invalidate()'.
    std::set<TileKey> m_refreshTiles;

};

typedef shared_ptr<CustomLayer> CustomLayerPtr;

}

#endif

/*! @} */
