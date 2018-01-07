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
    @file       PinLayerImpl.h

    Class PinLayerImpl inherits from PinLayer interface.

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

#ifndef PINLAYERIMPL_H
#define PINLAYERIMPL_H

/*!
    @addtogroup nbmap
    @{
*/

#include "base.h"
#include "smartpointer.h"
#include "Pin.h"
#include "PinLayer.h"
#include "UnifiedLayer.h"
#include "TileInternal.h"
#include <string>
#include <vector>
#include <map>
#include "PinMaterialParser.h"

extern "C"
{
#include "nbcontextprotected.h"
}

class NBMFileHeader;
class VxCfgInfoMgt;

namespace nbmap
{
// Types ........................................................................................

class PinManager;

/*! */
class PinLayerImpl : public UnifiedLayer,
                     public PinLayer
{
public:
    // Public functions .........................................................................

    /*! PinLayerImpl constructor */
    PinLayerImpl(shared_ptr<string> layerID,           /*!< An ID to identify the layer */
                 shared_ptr<string> materialConfigPath,/*!< Path of a XML file contained material config information */
                 uint32  layerIdDigital,                    /*!< Digital layer ID */
                 PinManager* pinManager,                    /*!< A pointer to a pin manager */
                 NB_Context* context                        /*!< NB_Context instance */
                 );

    /*! PinLayerImpl destructor */
    virtual ~PinLayerImpl();

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

    /*! Check whether this PinLayer is in valid state.

        This function should be called right after creating PinLayerImpl. It checks whether
        associated TileLayerInfo is valid. If it is invalid, user should not use this
        PinLayerImpl.

        @return true if valid.
    */
    bool IsValid();

    virtual std::string className() const { return "PinLayerImpl"; }


private:
    // Private types ..........................................................................

    /*! Key used to distinguish two pin materials */
    struct PinMaterialKey : public Base
    {
        /* See source file for description */

        PinMaterialKey(CustomPinInformationPtr pinInformation);
        virtual ~PinMaterialKey();
        PinMaterialKey(const PinMaterialKey& key);
        PinMaterialKey& operator=(const PinMaterialKey& key);
        bool operator==(const PinMaterialKey& anotherKey) const;
        bool operator<(const PinMaterialKey& anotherKey) const;

        CustomPinInformationPtr m_pinInformation;
    };

    typedef map<TileKey, vector<PinPtr> > TILEKEY_PINVEC_MAP;

    // Private functions ......................................................................

    // Copy constructor and assignment operator are not supported.
    PinLayerImpl(const PinLayerImpl& layer);
    PinLayerImpl& operator=(const PinLayerImpl& layer);

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

    shared_ptr<string> GetContentID(const TileKey& tileKey,
                                    const vector<PinPtr>& pins);
    NB_Error GenerateTileData(const TileKey& tileKey,
                              const vector<PinPtr>& pins,
                              nbcommon::DataStreamPtr& tileData);

    NB_Error GetCustomPinMaterialChunkIndex(CustomPinInformationPtr customPinInformation,
                                            NBMFileHeader& nbmHeader,
                                            uint16& pinMaterialIndex);

    void GetTile(TileKeyPtr tileKey, TileKeyPtr convertedTileKey,
                 AsyncTileRequestWithRequestPtr callback);

    void GetPinsInTile(TileKeyPtr tileKey, map<TileKey, vector<PinPtr> >& pins);

    // Refer to parent class for description.
    virtual void PostLayerToggled();

// Private members ........................................................................
    uint32 m_currentGeneratedID;               /*!< Current generated pin ID */
    string m_IdSeperator;                      /*!< Separator for layer id and pin Id.*/
    shared_ptr<string> m_layerID;              /*!< An ID to identify this layer */
    shared_ptr<string> m_projection;           /*!< Projection of this layer */
    shared_ptr<string> m_country;              /*!< Country of this layer */
    shared_ptr<string> m_materialConfigPath;   /*!< Path of a XML file contained material
                                                    config information */
    PinManager* m_pinManager;                  /*!< Instance of Pin Manager that created the
                                                    pin Layer. PinLayer will use this
                                                    PinManager to create PinIDs. */
    /*! Collection of added pins.

        All pins are organized by its TileKey. Although this makes it a little difficult to
        find pin by its id, it will speed up GetTile(). And GetTile() is called very
        frequently if this layer is enabled.
     */
    map<TileKey, vector<PinPtr> > m_Pins;

    PinMaterialParser           m_materialParser;    /*!< MaterialParser to parse PinMaterials  */
    map<PinMaterialKey, uint16> m_pinMaterialFilter; /*!< Used to filter out duplicated pin materials. */
    shared_ptr<const string>  m_materialCategory;
};

};  // namespace nbmap

/*! @} */

#endif  // PINLAYERIMPL_H
