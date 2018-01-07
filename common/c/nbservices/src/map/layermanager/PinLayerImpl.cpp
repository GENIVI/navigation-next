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
    @file       PinLayerImpl.cpp

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
#include "PinLayerImpl.h"
#include "PinImpl.h"
#include "PinManagerImpl.h"
#include "TileImpl.h"
#include "datastreamimplementation.h"
#include "StringUtility.h"
#include "LayerFunctors.h"
#include "NBMFileHeader.h"
#include "NBMTextureImageChunk.h"
#include "NBMPinChunk.h"
#include "NBMTextChunk.h"
#include "PinManagerImpl.h"
#include "UnifiedLayer.h" // Several useful macros are defined there.
#include "PinMaterialParser.h"

#include <sstream>
#include <limits>
#include <map>
#include <set>
#include <iterator>

// Disable warning C4245: "conversion from 'int' to 'unsigned int', signed/unsigned mismatch"
// (we don't own this code, the core team does!)
#pragma warning( push )
#pragma warning(disable:4245)
    #include "NBMMaterialChunk.h"
#pragma warning( pop )


/*! @{ */

using namespace nbcommon;
using namespace nbmap;

// Default draw order for pin layer
#define DEFAULT_PIN_LAYER_DRAW_ORDER 209

// Constants ....................................................................................

// Used to generate checksum
const uint32 CRC_VALUE = 0xAEF0AEF0;

// Maximum time to generate a pin ID by hash
//const int MAX_HASH_TIME = 20;

// @todo: These value should be defined in NBM library.
const uint16 TEMP_NBM_LAYER_ID_POIS = 7;

// Default reference tile grid level of pin layers
const uint32 PIN_LAYER_REFERENCE_TILE_GRID_LEVEL = 12;

// Template to generate a content ID
const char PARAMETER_PIN_LAYER_ID[]   = "$layerid";
const char PARAMETER_PIN_TILE_X[]     = "$x";
const char PARAMETER_PIN_TILE_Y[]     = "$y";
const char PARAMETER_PIN_ZOOM_LEVEL[] = "$z";
const char PARAMETER_PIN_UNIFIED_ID[] = "$uid";
const char PIN_CONTENT_ID_TEMPLATE[]  = "PIN_$layerid_$x_$y_$z_$uid";

#define NBM_FILE_REFNAME_GPIN "GPIN"

#define RGBA_TO_BGRA(X) ((X & 0x00FF00FF) | ((X >> 16) & 0xFF00) | ((X & 0xFF00) << 16))

// Local structures.

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

class FindPinByIDFunctor
{
public:
    FindPinByIDFunctor(shared_ptr<string> pinId)
            : m_pinId(pinId) {}
    virtual ~FindPinByIDFunctor(){}

    bool operator() (const PinPtr& pin) const
    {
        return pin ? StringUtility::IsStringEqual(pin->GetPinID(), m_pinId) : false;
    }

private:
    shared_ptr<string> m_pinId;
};

class TileKeySetToPtrVector
{
public:
    TileKeySetToPtrVector(vector<TileKeyPtr>& container)
            : m_container(container)  {}
    virtual ~TileKeySetToPtrVector() {}

    void operator() (const TileKey& key)
    {
        m_container.push_back(TileKeyPtr(new TileKey(key)));
    }

    // Empty assignment operator to avoid warning C4512: "assignment operator could not be generated" (due to const member)
    TileKeySetToPtrVector& operator =(const TileKeySetToPtrVector& input) {return *this;};

private:
    vector<TileKeyPtr>& m_container;
};

class RecordPinCoordinateFunctor
{
public:
    RecordPinCoordinateFunctor(shared_ptr<vector<NB_LatitudeLongitude> > record)
            : m_records(record) {}
    virtual ~RecordPinCoordinateFunctor(){}

    void operator() (const PinPtr& pin)
    {
        if (pin && m_records)
        {
            NB_LatitudeLongitude coordinate = {0};
            coordinate.latitude = pin->GetLatitude();
            coordinate.longitude = pin->GetLongitude();
            m_records->push_back(coordinate);
        }
    }

private:
    shared_ptr<vector<NB_LatitudeLongitude> > m_records;
};


// Local functions ...........................................................

bool IsPinInformationEuqal(const CustomPinInformationPtr& left,
                           const CustomPinInformationPtr& right)
{
    if (!left && !right)
    {
        return true;
    }

    if (!left || !right)
    {
        return false;
    }

    return ((left->m_selectedImage.get()   == right->m_selectedImage.get())   &&
            (left->m_unselectedImage.get() == right->m_unselectedImage.get()) &&

            (left->m_unselectedCalloutOffsetX== right->m_unselectedCalloutOffsetX) &&
            (left->m_unselectedCalloutOffsetY== right->m_unselectedCalloutOffsetY) &&
            (left->m_unselectedBubbleOffsetX == right->m_unselectedBubbleOffsetX)  &&
            (left->m_unselectedBubbleOffsetY == right->m_unselectedBubbleOffsetY)  &&

            (left->m_selectedCalloutOffsetX== right->m_selectedCalloutOffsetX) &&
            (left->m_selectedCalloutOffsetY== right->m_selectedCalloutOffsetY) &&
            (left->m_selectedBubbleOffsetX == right->m_selectedBubbleOffsetX)  &&
            (left->m_selectedBubbleOffsetY == right->m_selectedBubbleOffsetY)  &&

            (left->m_unselectedCircleInteriorColor== right->m_unselectedCircleInteriorColor) &&
            (left->m_unselectedCircleOutLineColor == right->m_unselectedCircleOutLineColor)  &&

            (left->m_unselectedImageWidth== right->m_unselectedImageWidth) &&
            (left->m_unselectedImageHeight == right->m_unselectedImageHeight)  &&
            (left->m_selectedImageWidth== right->m_selectedImageWidth) &&
            (left->m_selectedImageHeight == right->m_selectedImageHeight)  &&

            (left->m_selectedCircleInteriorColor== right->m_selectedCircleInteriorColor) &&
            (left->m_selectedCircleOutLineColor == right->m_selectedCircleOutLineColor)  &&

            (left->m_circleOutlineWidth  == right->m_circleOutlineWidth));
}

bool IsPinInformationLess(const CustomPinInformationPtr& left,
                          const CustomPinInformationPtr& right)
{
    // Compare images
    /* @todo: User should set same shared pointer of data stream for same image. Because
       I only check pointers of data stream equal to avoid adding duplicated. And
       I think checking original data of data stream is slow.
    */
    if (left->m_selectedImage.get() < right->m_selectedImage.get())
    {
        return true;
    }
    else if (left->m_selectedImage.get() > right->m_selectedImage.get())
    {
        return false;
    }

    if (left->m_unselectedImage.get() < right->m_unselectedImage.get())
    {
        return true;
    }
    else if (left->m_unselectedImage.get() > right->m_unselectedImage.get())
    {
        return false;
    }

    // Compare CalloutOffset
    if (left->m_unselectedCalloutOffsetX < right->m_unselectedCalloutOffsetX)
    {
        return true;
    }
    else if (left->m_unselectedCalloutOffsetX > right->m_unselectedCalloutOffsetX)
    {
        return false;
    }

    if (left->m_unselectedCalloutOffsetY < right->m_unselectedCalloutOffsetY)
    {
        return true;
    }
    else if (left->m_unselectedCalloutOffsetY > right->m_unselectedCalloutOffsetY)
    {
        return false;
    }

    if (left->m_selectedCalloutOffsetX < right->m_selectedCalloutOffsetX)
    {
        return true;
    }
    else if (left->m_selectedCalloutOffsetX > right->m_selectedCalloutOffsetX)
    {
        return false;
    }

    if (left->m_selectedCalloutOffsetY < right->m_selectedCalloutOffsetY)
    {
        return true;
    }
    else if (left->m_selectedCalloutOffsetY > right->m_selectedCalloutOffsetY)
    {
        return false;
    }

    // Compare Bubble Offset
    if (left->m_unselectedBubbleOffsetX < right->m_unselectedBubbleOffsetX)
    {
        return true;
    }
    else if (left->m_unselectedBubbleOffsetX > right->m_unselectedBubbleOffsetX)
    {
        return false;
    }

    if (left->m_unselectedBubbleOffsetY < right->m_unselectedBubbleOffsetY)
    {
        return true;
    }
    else if (left->m_unselectedBubbleOffsetY > right->m_unselectedBubbleOffsetY)
    {
        return false;
    }

    if (left->m_selectedBubbleOffsetX < right->m_selectedBubbleOffsetX)
    {
        return true;
    }
    else if (left->m_selectedBubbleOffsetX > right->m_selectedBubbleOffsetX)
    {
        return false;
    }

    if (left->m_selectedBubbleOffsetY < right->m_selectedBubbleOffsetY)
    {
        return true;
    }
    else if (left->m_selectedBubbleOffsetY > right->m_selectedBubbleOffsetY)
    {
        return false;
    }

    // Compare Color
    if (left->m_unselectedCircleInteriorColor < right->m_unselectedCircleInteriorColor)
    {
        return true;
    }
    else if (left->m_unselectedCircleInteriorColor > right->m_unselectedCircleInteriorColor)
    {
        return false;
    }

    if (left->m_unselectedCircleOutLineColor < right->m_unselectedCircleOutLineColor)
    {
        return true;
    }
    else if (left->m_unselectedCircleOutLineColor > right->m_unselectedCircleOutLineColor)
    {
        return false;
    }

    if (left->m_selectedCircleInteriorColor < right->m_selectedCircleInteriorColor)
    {
        return true;
    }
    else if (left->m_selectedCircleInteriorColor > right->m_selectedCircleInteriorColor)
    {
        return false;
    }

    if (left->m_selectedCircleOutLineColor < right->m_selectedCircleOutLineColor)
    {
        return true;
    }
    else if (left->m_selectedCircleOutLineColor > right->m_selectedCircleOutLineColor)
    {
        return false;
    }

    // Compare width
    if (left->m_circleOutlineWidth < right->m_circleOutlineWidth)
    {
        return true;
    }
    else if (left->m_circleOutlineWidth > right->m_circleOutlineWidth)
    {
        return false;
    }

    if (left->m_unselectedImageWidth < right->m_unselectedImageWidth)
    {
        return true;
    }
    else if (left->m_unselectedImageWidth > right->m_unselectedImageWidth)
    {
        return false;
    }

    if (left->m_unselectedImageHeight < right->m_unselectedImageHeight)
    {
        return true;
    }
    else if (left->m_unselectedImageHeight > right->m_unselectedImageHeight)
    {
        return false;
    }

    if (left->m_selectedImageWidth < right->m_selectedImageWidth)
    {
        return true;
    }
    else if (left->m_selectedImageWidth > right->m_selectedImageWidth)
    {
        return false;
    }

    if (left->m_selectedImageHeight < right->m_selectedImageHeight)
    {
        return true;
    }
    else if (left->m_selectedImageHeight > right->m_selectedImageHeight)
    {
        return false;
    }

    return false;
}

// Public functions .............................................................................

/*! Check if given DataStream is empty. */
static inline bool
IsDataStreamEmpty(DataStreamPtr data)
{
    return !data || !data->GetDataSize();
}

/*! Convert DataStream into TextureBitMapBody.

    @return NB_OK if succeeded.
*/
static inline NB_Error
DataStreamToTextureBitMapBody(TextureBitMapBody& bitmapBody,
                              DataStreamPtr imageData,
                              const char* name=NULL)
{
    uint32 dataSize = 0;
    if (!imageData || !(dataSize = imageData->GetDataSize()))
    {
        return NE_INVAL;
    }

    uint8* data = new uint8[dataSize];
    if (!data)
    {
        return NE_NOMEM;
    }

    // Copy image data to the buffer.
    uint32 gotDataSize = imageData->GetData(data, 0, dataSize);
    if (gotDataSize != dataSize)
    {
        delete [] data;
        data = NULL;
        return NE_UNEXPECTED;
    }

    // Create the bitmap body.
    bitmapBody.filesize = dataSize;
    bitmapBody.filebuf  = (char*) data;
    if (name)
    {
        bitmapBody.SetName(name);
    }


    return NE_OK;
}

    // Empty assignment operator to avoid warning C4512: "assignment operator could not be generated" (due to const member)
   // PinTileCallbackWithRequest& operator =(const PinTileCallbackWithRequest& input) {};


// Implementation of PinLayerImpl
/* See header file for description */
PinLayerImpl::PinLayerImpl(shared_ptr<string> layerID,
                           shared_ptr<string> materialConfigPath,
                           uint32 layerIdDigital,
                           PinManager* pinManager,
                           NB_Context* context)
        : UnifiedLayer(TileManagerPtr(), layerIdDigital, context),
          m_currentGeneratedID(0),
          m_IdSeperator(ID_SEPERATOR),
          m_layerID(layerID),
          m_materialConfigPath(materialConfigPath),
          m_pinManager(pinManager),
          m_materialParser(materialConfigPath ? m_materialConfigPath->c_str() : NULL)
{
    // @note: After creating PinLayerImpl, use PinLayerImpl::IsValid() to check whether this
    //        object is valid. Do not use it if it is in invalid state.
    if (m_tileLayerInfo)
    {
        m_tileLayerInfo->drawOrder        = DEFAULT_PIN_LAYER_DRAW_ORDER;
        m_tileLayerInfo->tileDataType     = layerID;
        m_tileLayerInfo->refTileGridLevel = PIN_LAYER_REFERENCE_TILE_GRID_LEVEL;

        //@todo: Hard-coded value, should be removed, refer to the TODOList of
        //       PinManagerImpl.cpp
        m_tileLayerInfo->materialCategory.reset(new string("pin"));
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
PinLayerImpl::~PinLayerImpl()
{
    // Nothing to do here.
}

/* See description in Layer.h */
shared_ptr<string>
PinLayerImpl::GetTileDataType()
{
    // Layer ID is data type of this layer.
    return m_layerID;
}

shared_ptr<string> PinLayerImpl::GetLayerID()
{
    return m_layerID;
}

/* See description in Layer.h */
void
PinLayerImpl::GetTiles(vector<TileKeyPtr> tileKeys,
                      AsyncTileRequestWithRequestPtr callback,
                      uint32 /*priority*/)
{
    // Tile key cannot be NULL. Also, pinTiles will not be cached, so just skip request if
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

    if (m_Pins.empty())
    {
        // There is no pins, ignore it.
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
           this pin layer, convert this tile key to a tile key of reference tile grid
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
PinLayerImpl::GetTile(TileKeyPtr tileKey, TileKeyPtr convertedTileKey,
                      AsyncTileRequestWithRequestPtr callback)
{
    if (!callback)
    {
        return;
    }

    // Loop for all pins to get pins existing in this tile.
    map<TileKey, vector<PinPtr> >  pinMaps;
    GetPinsInTile(convertedTileKey, pinMaps);
    if (pinMaps.empty())
    {
        callback->Error(tileKey, NE_NOENT);
        return;
    }

    NB_Error error = NE_OK;
    map<TileKey, vector<PinPtr> >::const_iterator iter = pinMaps.begin();
    map<TileKey, vector<PinPtr> >::const_iterator end  = pinMaps.end();
    for (; iter != end; ++iter)
    {
        TileImplPtr tile;
        const vector<PinPtr>& pinsInTile = iter->second;
        if (pinsInTile.empty())
        {
            continue;
        }

        const TileKey& refTileKey = iter->first;

        // Get content ID of the pin tile.
        shared_ptr<string> contentId = GetContentID(refTileKey,
                                                    pinsInTile);
        if ((!contentId) || (contentId->empty()))
        {
            error = NE_UNEXPECTED;
            continue;
        }

        // Create a tile object to return the data.
        tile.reset(new TileImpl(contentId, m_tileLayerInfo, false));
        TileKeyPtr refTileKeyPtr(new TileKey(refTileKey));
        if (!tile || !refTileKeyPtr)
        {
            error = NE_NOMEM;
            continue;
        }

        // Use NBM library to generate a pin tile.
        DataStreamPtr tileData;
        error = GenerateTileData(refTileKey, pinsInTile, tileData);
        if (error != NE_OK)
        {
            continue;
        }

        // Data of the pin tile must not be empty.
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


/* See description in Layer.h */
void
PinLayerImpl::RemoveAllTiles()
{

    NB_ASSERT_CCC_THREAD(m_pContext);

    // Nothing to do here.
}

/* See description in PinLayer.h */
uint32
PinLayerImpl::GetID()
{
    return m_tileLayerInfo->layerID;
}

/* See description in PinLayer.h */
void
PinLayerImpl::RemovePins(const vector<shared_ptr<string> >& pinIDs)
{
    NB_ASSERT_CCC_THREAD(m_pContext);

    // Check if the vector of pin IDs is empty.
    if (pinIDs.empty())
    {
        return;
    }

    // Find and erase pins by pin IDs.

    set<TileKey> uniqueTileKeys;
    shared_ptr<vector<PinPtr> > pinsToRemove(new vector<PinPtr>());

    vector<shared_ptr<string> >::const_iterator idIterator = pinIDs.begin();
    vector<shared_ptr<string> >::const_iterator idEnd = pinIDs.end();
    for (; idIterator != idEnd; ++idIterator)
    {
        // Get the pin ID.
        shared_ptr<string> pinId = *idIterator;
        if (!pinId)
        {
            continue;
        }

        // Find the pin information by pin ID.
        FindPinByIDFunctor functor(pinId);
        map<TileKey, vector<PinPtr> >::iterator iter = m_Pins.begin();
        map<TileKey, vector<PinPtr> >::iterator end  = m_Pins.end();
        for (; iter != end; ++iter)
        {
            vector<PinPtr>& pins = iter->second;
            vector<PinPtr>::iterator pinEnd = pins.end();
            vector<PinPtr>::iterator pinIter = find_if (pins.begin(), pinEnd, functor);
            if (pinIter == pinEnd)
            {
                continue; // pinId does not exist in this vector, try next.
            }

            // Pin is found in this vector, record TileKey and pin for later use
            uniqueTileKeys.insert(iter->first);
            PinPtr pin = *pinIter;

            // Clean up.
            pins.erase(pinIter);
            if (pins.empty())
            {
                m_Pins.erase(iter);
            }

            if (pinsToRemove && pin)
            {
                // Add the pin to remove.
                pinsToRemove->push_back(pin);
            }

            break;
        }
    }

    // Notify changed pin tiles to refresh if necessary.
    if (!uniqueTileKeys.empty())
    {
        vector<TileKeyPtr> tileKeysToRefresh;
        TileKeySetToPtrVector functor(tileKeysToRefresh);
        for_each (uniqueTileKeys.begin(), uniqueTileKeys.end(), functor);
        RefreshPinsOfTiles(tileKeysToRefresh);
    }

    // Notify that the pins are removed.
    NotifyPinsRemoved(pinsToRemove);
}

/* See description in PinLayer.h */
void
PinLayerImpl::RemoveAllPins()
{
    NB_ASSERT_CCC_THREAD(m_pContext);

    if (m_Pins.empty())
    {
        return;
    }

    shared_ptr<vector<PinPtr> > pinsToRemove(new vector<PinPtr>(GetAllPins()));

    vector<TileKeyPtr> tileKeysToRefresh;
    map<TileKey, vector<PinPtr> >::const_iterator iter = m_Pins.begin();
    map<TileKey, vector<PinPtr> >::const_iterator end = m_Pins.end();
    for(; iter != end; ++iter)
    {
        tileKeysToRefresh.push_back(TileKeyPtr(new TileKey(iter->first)));
    }

    m_Pins.clear();
    RefreshPinsOfTiles(tileKeysToRefresh);
    NotifyPinsRemoved(pinsToRemove);
}

/* See description in PinLayer.h */
vector<PinPtr>
PinLayerImpl::GetAllPins()
{
    vector<PinPtr> vecPins;
    map<TileKey, vector<PinPtr> >::const_iterator iter = m_Pins.begin();
    map<TileKey, vector<PinPtr> >::const_iterator end  = m_Pins.end();
    for (; iter != end; ++iter)
    {
        copy(iter->second.begin(), iter->second.end(), back_inserter(vecPins));
    }

    return vecPins;
}

/* See description in PinLayer.h */
void
PinLayerImpl::SetMaterialConfigPath(shared_ptr<string> materialConfigPath)
{

    NB_ASSERT_CCC_THREAD(m_pContext);

    m_materialConfigPath = materialConfigPath;
    if (m_materialConfigPath && (!(m_materialConfigPath->empty())))
    {
        m_materialParser.LoadFromFile(m_materialConfigPath->c_str());
    }
}

/* See description in PinLayer.h */
PinPtr
PinLayerImpl::GetPin(shared_ptr<string> pinID)
{

    NB_ASSERT_CCC_THREAD(m_pContext);

    PinPtr pin;
    if (pinID && !pinID->empty())
    {
        // Find the pin information by pin ID.
        FindPinByIDFunctor functor(pinID);
        map<TileKey, vector<PinPtr> >::const_iterator iter = m_Pins.begin();
        map<TileKey, vector<PinPtr> >::const_iterator end  = m_Pins.end();
        for (; iter != end; ++iter)
        {
            const vector<PinPtr>& pins = iter->second;
            vector<PinPtr>::const_iterator pinEnd = pins.end();
            vector<PinPtr>::const_iterator pinIter = find_if (pins.begin(), pinEnd, functor);
            if (pinIter != pinEnd)
            {
                pin = *pinIter;
                break;
            }
        }
    }
    return pin;
}

/* See description in PinLayer.h */
shared_ptr<vector<NB_LatitudeLongitude> >
PinLayerImpl::GetAllPinCoordinates()
{
    NB_ASSERT_CCC_THREAD(m_pContext);

    // Check if there are some pins in the layer.
    if (m_Pins.empty())
    {
        return shared_ptr<vector<NB_LatitudeLongitude> >();
    }

    // Create a vector to return the coordinates.
    shared_ptr<vector<NB_LatitudeLongitude> > vecCoordinates( new vector<NB_LatitudeLongitude>());
    if (!vecCoordinates)
    {
        return shared_ptr<vector<NB_LatitudeLongitude> >();
    }

    // Get the coordinates for all pins of the layer.
    map<TileKey, vector<PinPtr> >::const_iterator iter = m_Pins.begin();
    map<TileKey, vector<PinPtr> >::const_iterator end  = m_Pins.end();
    for (; iter != end; ++iter)
    {
        RecordPinCoordinateFunctor functor(vecCoordinates);
        for_each (iter->second.begin(), iter->second.end(), functor);
    }

    return vecCoordinates;
}

// Protected functions .....................................................................

/* See description in PinLayer.h */
shared_ptr<string>
PinLayerImpl::GeneratePinID()
{
    // Layer ID should not be empty.
    if ((!m_layerID) || (m_layerID->empty()))
    {
        return shared_ptr<string>();
    }

    // This segment is added for extra protection, but I don't think it will be used ...
    if (++m_currentGeneratedID == numeric_limits<uint32>::max())
    {
        m_IdSeperator += StringUtility::NumberToString(PAL_ClockGetUnixTime());
        m_currentGeneratedID = 0;
    }

    // Just increase m_currentGeneratedID and append it to form a unique id.
    shared_ptr<string> pinID(new string((*m_layerID) + m_IdSeperator +
                                        StringUtility::NumberToString(m_currentGeneratedID)));
    return pinID;
}

/* See description in PinLayer.h */
TileKeyPtr
PinLayerImpl::AddPinPtr(PinPtr pin)
{
    // Check if pin is empty.
    if (!pin)
    {
        return TileKeyPtr();
    }

    // Get pin ID.
    shared_ptr<string> pinID = pin->GetPinID();
    if ((!pinID) || pinID->empty())
    {
        return TileKeyPtr();
    }

    // Get latitude and longitude.
    double latitude  = pin->GetLatitude();
    double longitude = pin->GetLongitude();
    if ((latitude == INVALID_LATLON) ||
        (longitude == INVALID_LATLON))
    {
        return TileKeyPtr();
    }

    // Get a tile key of reference tile grid zoom level.
    TileKeyPtr referenceTileKey = GetReferenceTileKeyByCoordinate(latitude, longitude);
    if (!referenceTileKey)
    {
        return TileKeyPtr();
    }


    // Check if same pinId exists, return empty tileKey if so.
    vector<PinPtr>&          pins = m_Pins[*referenceTileKey];
    vector<PinPtr>::iterator end  = pins.end();
    FindPinByIDFunctor       functor(pinID);
    if (find_if (pins.begin(), end, functor) != end)
    {
        return TileKeyPtr();
    }

    pins.push_back(pin);

    return referenceTileKey;
}

/* See description in PinLayer.h */
void
PinLayerImpl::RefreshAllPins()
{
    if (m_pinManager)
    {
        PinManagerImpl* manager = static_cast<PinManagerImpl*>(m_pinManager);
        if (manager)
        {
            manager->RefreshAllPinsOfLayer(m_layerID);
        }
    }
}

/* See description in PinLayer.h */
void
PinLayerImpl::RefreshPinsOfTiles(const vector<TileKeyPtr>& tileKeys)
{
    if (m_pinManager)
    {
        PinManagerImpl* manager = static_cast<PinManagerImpl*>(m_pinManager);
        if (manager)
        {
            manager->RefreshTilesOfLayer(tileKeys, m_layerID);
        }
    }
}

/*! Notify that the pins are removed

    @return None
*/
void
PinLayerImpl::NotifyPinsRemoved(shared_ptr<vector<PinPtr> > pins)
{
    if (pins && (!(pins->empty())) && m_pinManager)
    {
        PinManagerImpl* manager = static_cast<PinManagerImpl*>(m_pinManager);
        if (manager)
        {
            manager->NotifyPinsRemoved(pins);
        }
    }
}

/*! Convert the tile key to a tile key of specified less zoom level

    @return Converted tile key of specified zoom level
*/
TileKeyPtr
PinLayerImpl::ConvertTileKeyToLessZoomLevel(TileKeyPtr sourceTileKey,   /*!< Source tile key */
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

/*! Convert coordinate to a tile key of reference tile grid zoom level

    @return Converted tile key of reference tile grid zoom level
*/
TileKeyPtr
PinLayerImpl::GetReferenceTileKeyByCoordinate(double latitude,  /*!< Latitude to convert */
                                              double longitude  /*!< Longitude to convert */
                                              )
{
    // Check 'm_tileLayerInfo' in function IsValid.
    if (!IsValid())
    {
        return TileKeyPtr();
    }

    int refTileGridLevel = static_cast<int>(m_tileLayerInfo->refTileGridLevel);
    int tileX = 0;
    int tileY = 0;
    GetTileByCoordinate(latitude,
                        longitude,
                        refTileGridLevel,
                        tileX,
                        tileY);

    return TileKeyPtr(new TileKey(tileX, tileY, refTileGridLevel));
}

/*! Convert coordinate to tile

    This function converts coordinate to mercator and then converts mercator to tile.

    @return None
*/
void
PinLayerImpl::GetTileByCoordinate(double latitude,      /*!< Latitude to convert */
                                  double longitude,     /*!< Longitude to convert */
                                  int zoomLevel,        /*!< Zoom level specified */
                                  int& tileX,           /*!< On return converted X position */
                                  int& tileY            /*!< On return converted Y position */
                                  )
{
    double mercatorX = 0.0;
    double mercatorY = 0.0;
    double halfScale = static_cast<double>(1 << (zoomLevel - 1));

    // Convert coordinate to mercator.
    mercatorForward(latitude, longitude, &mercatorX, &mercatorY);

    // Get bound mercator to avoid out of range.
    mercatorX = GetBoundMercator(mercatorX);
    mercatorY = GetBoundMercator(mercatorY);

    tileX = static_cast<int>((mercatorX / PI + 1.0) * halfScale);
    tileY = static_cast<int>((1.0 - mercatorY / PI) * halfScale);
}

/*! Convert tile to coordinate

    This function converts tile to mercator and then converts mercator to coordinate.

    @return None
*/
void
PinLayerImpl::GetCoordinateByTile(int tileX,            /*!< X position of tile */
                                  int tileY,            /*!< Y position of tile */
                                  int zoomLevel,        /*!< Zoom level of tile */
                                  double& latitude,     /*!< On return converted latitude */
                                  double& longitude     /*!< On return converted longitude */
                                  )
{
    double mercatorX = 0.0;
    double mercatorY = 0.0;
    double halfScale = static_cast<double>(1 << (zoomLevel - 1));

    // Convert tile to mercator.
    mercatorX = (static_cast<double>(tileX) / halfScale - 1.0) * PI;
    mercatorY = (1.0 - static_cast<double>(tileY) / halfScale) * PI;

    // Get bound mercator to avoid out of range.
    mercatorX = GetBoundMercator(mercatorX);
    mercatorY = GetBoundMercator(mercatorY);

    // Convert mercator to coordinate.
    mercatorReverse(mercatorX, mercatorY, &latitude, &longitude);
}

/*! Get bound mercator

    This function limits value of mercator to [-PI, PI).

    @return Bound mercator
*/
double
PinLayerImpl::GetBoundMercator(double mercator      /*!< Value of mercator to limit */
                               )
{
    // Limit value of mercator to [-PI, PI).
    while (mercator < -PI)
    {
        mercator += TWO_PI;
    }
    while (mercator >= PI)
    {
        mercator -= TWO_PI;
    }

    return mercator;
}

/*! Get content ID of a pin tile by a tile key

    @return Content ID of the pin tile if success, otherwise return empty shared pointer.
*/
shared_ptr<string>
PinLayerImpl::GetContentID(const TileKey& tileKey,    /*!< Tile key of the tile */
                           const vector<PinPtr>& pins /*!< Pins existing in the tile */
                           )
{
    if (pins.empty() || (!m_layerID) || (m_layerID->empty()))
    {
        return shared_ptr<string>();
    }

    shared_ptr<string> contentId(new string(PIN_CONTENT_ID_TEMPLATE));
    if (!contentId)
    {
        return shared_ptr<string>();
    }

    // Use CRC32 to get an unified ID by pin IDs existing in this tile.
    string stringToCrc;
    vector<PinPtr>::const_iterator pinIterator = pins.begin();
    vector<PinPtr>::const_iterator pinEnd      = pins.end();
    for (; pinIterator != pinEnd; ++pinIterator)
    {
        const PinPtr& pin = *pinIterator;
        if (!pin)
        {
            continue;
        }

        // Get pin ID.
        shared_ptr<string> pinId = pin->GetPinID();
        if ((!pinId) || pinId->empty())
        {
            continue;
        }

        stringToCrc += *pinId;
    }

    // If two tiles have same unified ID, both tiles contain same pins.
    uint32 unifiedId = crc32(CRC_VALUE, (const byte*) (stringToCrc.c_str()),
                             stringToCrc.size());

    // Format template with layer ID.
    StringUtility::ReplaceString(*contentId, PARAMETER_PIN_LAYER_ID, *m_layerID);

    // Format template with tile key.
    StringUtility::ReplaceString(*contentId, PARAMETER_PIN_TILE_X,
                                 StringUtility::NumberToString(tileKey.m_x));

    StringUtility::ReplaceString(*contentId, PARAMETER_PIN_TILE_Y,
                                 StringUtility::NumberToString(tileKey.m_y));

    StringUtility::ReplaceString(*contentId, PARAMETER_PIN_ZOOM_LEVEL,
                                 StringUtility::NumberToString(tileKey.m_zoomLevel));

    // Format template with unified ID.
    StringUtility:: ReplaceString(*contentId, PARAMETER_PIN_UNIFIED_ID,
                                  StringUtility::NumberToString(unifiedId));

    return contentId;
}

/*! Use NBM library to generate data of a pin tile

    @return NE_OK if success
*/
NB_Error
PinLayerImpl::GenerateTileData(const TileKey& tileKey,      /*!< Tile key of the tile */
                               const vector<PinPtr>& pins,  /*!< Pins existing in the tile */
                               DataStreamPtr& tileData      /*!< Return data of the pin tile */
                               )
{
    NB_Error error = NE_OK;
    m_pinMaterialFilter.clear();
    // Check if there are some pins.
    if (pins.empty())
    {
        return NE_INVAL;
    }

    // Create a data stream.
    DataStreamPtr dataStream(new DataStreamImplementation());
    if (!dataStream)
    {
        return NE_NOMEM;
    }

    // Initialize environment of NBM.
    NBMFileHeader nbmHeader;
    nbmHeader.setNBMVersion((24 << 8) | 0 );
    nbmHeader.SetIndex(static_cast<unsigned int>(tileKey.m_x),
                       static_cast<unsigned int>(tileKey.m_y),
                       static_cast<unsigned int>(tileKey.m_zoomLevel));

    nbmHeader.SetRefInfo(NBM_FILE_REFNAME_GPIN);
    nbmHeader.SetMaking();

    // Create an assiciated text chunk.
    NBMTextChunk* textChunk = static_cast<NBMTextChunk*>(
        nbmHeader.CreateChunk(NBM_CHUNCK_IDENTIFIER_TEXT, NULL));
    if (!textChunk)
    {
        return NE_UNEXPECTED;
    }

    // Create a GPIN chunk.
    NBMLayerInfo pinLayerInfo;
    pinLayerInfo.layerType = TEMP_NBM_LAYER_ID_POIS;
    pinLayerInfo.highScale = 0.0;
    pinLayerInfo.lowScale  = static_cast<float>(9.0e-8);
    NBMPinChunk* pinChunk  = static_cast<NBMPinChunk*>(
        nbmHeader.CreateChunk(NBM_CHUNCK_IDENTIFIER_GPIN, &pinLayerInfo));
    if (!pinChunk)
    {
        return NE_UNEXPECTED;
    }
    pinChunk->SetDataPrecision(NBMPrecision_float);

    /* This map is used to avoid adding duplicated custom pin materials to material chunk.
       The value of map is index of pin materials in material chunk. Check if the pin
       material is saved in this map after the same image has been used before (the data
       stream is found in the map of above addedImages). It is always used for the different
       callout offsets or bubble offsets (It is really a few for the same image).
    */
    map<PinMaterialKey, unsigned short> addedPinMaterials;

    // Add data of pin to chunks.
    vector<PinPtr>::const_iterator pinIterator = pins.begin();
    vector<PinPtr>::const_iterator pinEnd = pins.end();
    for (; pinIterator != pinEnd; ++pinIterator)
    {
        const PinPtr& pin = *pinIterator;
        if (!pin)
        {
            continue;
        }

        // Get pin ID.
        shared_ptr<string> pinId = pin->GetPinID();
        if ((!pinId) || pinId->empty())
        {
            continue;
        }

        // Get both unselected and selected indexes of the pin materials.
        uint16 pinMaterialIndex = NBM_INVALIDATE_INDEX;
        PinType type = pin->GetType();
        switch (type)
        {
            case PT_COMMON:
            case PT_START_FLAG:
            case PT_END_FLAG:
            case PT_SEVERE_INCIDENT:
            case PT_MAJOR_INCIDENT:
            case PT_MINOR_INCIDENT:
            {
                error = m_materialParser.GetPinMaterialIndex(type, pinMaterialIndex);
                break;
            }
            case PT_CUSTOM:
            {
                error = GetCustomPinMaterialChunkIndex(pin->GetCustomPinInformation(),
                                                       nbmHeader,
                                                       pinMaterialIndex);
                break;
            }
            case PT_INVALID:
            default:
            {
                // The type of pin is invalid.
                error = NE_INVAL;
                break;
            }
        }

        if (error != NE_OK)
        {
            m_pinMaterialFilter.clear();
            return error;
        }

        //@todo: NBM should not modify the content of string passed to them, and should accept a
        //       const char*, instead of `char*`.
        //@note: pinId are just identifier of pin, and will not be displayed on map, hence, set
        //       materialIndex to -1 bellow to skip font setting.
        NBMIndex textIndex = textChunk->SetData(static_cast<uint16>(-1), 0, (char*) (pinId->c_str()),
                                                static_cast<int>(pinId->size()));
        if (textIndex == NBM_INVALIDATE_INDEX)
        {
            m_pinMaterialFilter.clear();
            return NE_UNEXPECTED;
        }

        // Set data to point chunk.
        POINT2 point2 = {static_cast<float>(pin->GetLongitude()),
                         static_cast<float>(pin->GetLatitude())};

        NBMIndex pinIndex = pinChunk->SetData(pinMaterialIndex,
                                              point2,
                                              textIndex,
                                              pin->GetRadius());
        if (pinIndex == NBM_INVALIDATE_INDEX)
        {
            m_pinMaterialFilter.clear();
            return NE_UNEXPECTED;
        }
    }

    m_pinMaterialFilter.clear();

    // Get data from NBM.
    const char* buffer = NULL;
    unsigned int bufferSize = 0;
    if (!(nbmHeader.GetTileBuffer(buffer, bufferSize)))
    {
        return NE_UNEXPECTED;
    }
    error = dataStream->AppendData((const uint8*) buffer,
                                   static_cast<uint32>(bufferSize));
    NBMFileHeader::ReleaseBuffer(buffer);
    if (error != NE_OK)
    {
        return error;
    }

    tileData = dataStream;
    return NE_OK;
}

bool PinLayerImpl::IsValid()
{
    bool bValid = true;
    if (!m_tileLayerInfo)
    {
        bValid = false;
    }
    return bValid;
}

/* See description in header file. */
void PinLayerImpl::GetPinsInTile(TileKeyPtr tileKey, map<TileKey, vector<PinPtr> >& pinMaps)
{

    int tileX     = tileKey->m_x;
    int tileY     = tileKey->m_y;
    int zoomLevel = tileKey->m_zoomLevel;
    int distance  = m_tileLayerInfo->refTileGridLevel - zoomLevel;

    // When this functions is called, zoomLevel of input tileKey should not be greater than
    // ref-tile-grid-level of this layer.
    nsl_assert(distance >= 0);

    map<TileKey, vector<PinPtr> >::const_iterator iter = m_Pins.begin();
    map<TileKey, vector<PinPtr> >::const_iterator end  = m_Pins.end();
    for (; iter != end; ++iter)
    {
        const TileKey& key = iter->first;
        if (tileX != (key.m_x >> distance) || tileY != (key.m_y >> distance))
        {
            continue;
        }

        // If converted and requested tile keys are same, the pin exists in this tile.
        vector<PinPtr>& pins = pinMaps[key];
        copy(iter->second.begin(), iter->second.end(), back_inserter(pins));
    }
}

/* See description in header file. */
void PinLayerImpl::PostLayerToggled()
{
    RefreshAllPins();
}


// PinMaterialKey functions .....................................................................

/*! PinMaterialKey constructor */
PinLayerImpl::PinMaterialKey::PinMaterialKey(CustomPinInformationPtr pinInformation)
        : Base(),
          m_pinInformation(pinInformation)
{
}

/*! PinMaterialKey destructor */
PinLayerImpl::PinMaterialKey::~PinMaterialKey()
{
}

/*! PinMaterialKey copy constructor */
PinLayerImpl::PinMaterialKey::PinMaterialKey(const PinMaterialKey& key)
        : Base(),
          m_pinInformation(key.m_pinInformation)
{
}

/*! PinInformation assignment operator */
PinLayerImpl::PinMaterialKey&
PinLayerImpl::PinMaterialKey::operator=(const PinMaterialKey& key)
{
    if ((&key) != this)
    {
        this->m_pinInformation = key.m_pinInformation;
    }

    return *this;
}

/*! Equal operator */
bool
PinLayerImpl::PinMaterialKey::operator==(const PinMaterialKey& anotherKey) const
{
    return IsPinInformationEuqal(m_pinInformation, anotherKey.m_pinInformation);
}

/*! Less operator */
bool
PinLayerImpl::PinMaterialKey::operator<(const PinMaterialKey& anotherKey) const
{
    return IsPinInformationLess(m_pinInformation, anotherKey.m_pinInformation);
}

/* See description in header file. */
NB_Error PinLayerImpl::GetCachedTiles(const vector <TileKeyPtr>& /*tileKeys*/,
                                      AsyncTileRequestWithRequestPtr /*callback*/)
{
    return NE_OK;
}

/*! Get material chunk index for custom pin. */
NB_Error
PinLayerImpl::GetCustomPinMaterialChunkIndex(CustomPinInformationPtr pinInformation,
                                             NBMFileHeader& nbmHeader,
                                             uint16& pinMaterialIndex)
{
    if (!pinInformation || // No PinInformation
        (IsDataStreamEmpty(pinInformation->m_selectedImage) &&
         IsDataStreamEmpty(pinInformation->m_unselectedImage))) // both images are empty
    {
        return NE_INVAL;
    }

    DataStreamPtr selectedImage   = pinInformation->m_selectedImage;
    DataStreamPtr unselectedImage = pinInformation->m_unselectedImage;

    // Add the pin material to the material chunk if it is not empty and it is not
    // added to material chunk before.
    PinMaterialKey pinMaterialKey(pinInformation);
    map<PinMaterialKey, unsigned short>::iterator pinMaterialIterator
            = m_pinMaterialFilter.lower_bound(pinMaterialKey);
    if ((pinMaterialIterator != m_pinMaterialFilter.end()) &&
        (pinMaterialIterator->first == pinMaterialKey))
    {
        // The pin material has been added before.
        pinMaterialIndex = pinMaterialIterator->second;
        return NE_OK;
    }


    // 1. Create material chunk.
    NBMMaterialChunk* materialChunk = static_cast<NBMMaterialChunk*>(
        nbmHeader.GetChunk(NBM_CHUNCK_IDENTIFIER_MATERIALS));
    if (!materialChunk)
    {
        return NE_UNEXPECTED;
    }


    // 2. Create texture bitmap. If one of the images is empty, use the other one instead.
    if (IsDataStreamEmpty(selectedImage) && !IsDataStreamEmpty(unselectedImage))
    {
        selectedImage = unselectedImage;
    }
    if (IsDataStreamEmpty(unselectedImage) && !IsDataStreamEmpty(selectedImage))
    {
        unselectedImage = selectedImage;
    }

    TextureBitMapBody bitmapBody[2];
    // bitmap name is not necessary here, and not set.
    NB_Error error = DataStreamToTextureBitMapBody(bitmapBody[0], unselectedImage);
    error = error ? error : DataStreamToTextureBitMapBody(bitmapBody[1], selectedImage);
    if (error != NE_OK)
    {
        return error;
    }

    // 2. Create the pin body.
    // @note: a) MaterialNewPinBody is defined by NBM lib.
    //        b) unselectedBmp and selectedBmp of pinBody will be set by NBM library.
    MaterialNewPinBody pinBody;
    pinBody.unselectedCxOffset = pinMaterialKey.m_pinInformation->m_unselectedCalloutOffsetX;
    pinBody.unselectedCyOffset = pinMaterialKey.m_pinInformation->m_unselectedCalloutOffsetY;
    pinBody.unselectedBxOffset = pinMaterialKey.m_pinInformation->m_unselectedBubbleOffsetX;
    pinBody.unselectedByOffset = pinMaterialKey.m_pinInformation->m_unselectedBubbleOffsetY;

    pinBody.selectedCxOffset = pinMaterialKey.m_pinInformation->m_selectedCalloutOffsetX;
    pinBody.selectedCyOffset = pinMaterialKey.m_pinInformation->m_selectedCalloutOffsetY;
    pinBody.selectedBxOffset = pinMaterialKey.m_pinInformation->m_selectedBubbleOffsetX;
    pinBody.selectedByOffset = pinMaterialKey.m_pinInformation->m_selectedBubbleOffsetY;

    pinBody.selectedInteriorColor   = RGBA_TO_BGRA(pinMaterialKey.m_pinInformation->m_selectedCircleInteriorColor);
    pinBody.selectedBitOnColor = RGBA_TO_BGRA(pinMaterialKey.m_pinInformation->m_selectedCircleOutLineColor);
    pinBody.selectedBitOffColor = 0;

    pinBody.unselectedInteriorColor = RGBA_TO_BGRA(pinMaterialKey.m_pinInformation->m_unselectedCircleInteriorColor);
    pinBody.unselectedBitOnColor  = RGBA_TO_BGRA(pinMaterialKey.m_pinInformation->m_unselectedCircleOutLineColor);
    pinBody.unselectedBitOffColor  = 0;

    pinBody.pattern = 0xffffffff;

    pinBody.selectedImageHeight = pinMaterialKey.m_pinInformation->m_selectedImageHeight;
    pinBody.selectedImageWidth = pinMaterialKey.m_pinInformation->m_selectedImageWidth;
    pinBody.unselectedImageHeight = pinMaterialKey.m_pinInformation->m_unselectedImageHeight;
    pinBody.unselectedImageWidth = pinMaterialKey.m_pinInformation->m_unselectedImageWidth;

    pinBody.outlineWidth  = pinMaterialKey.m_pinInformation->m_circleOutlineWidth;

    // Add image data to material chunk.
    pinMaterialIndex = materialChunk->SetData(bitmapBody[0], bitmapBody[1], pinBody);
    if (pinMaterialIndex == NBM_INVALIDATE_INDEX)
    {
        return NE_UNEXPECTED;
    }

    m_pinMaterialFilter.insert(pinMaterialIterator, make_pair(pinMaterialKey, pinMaterialIndex));
    return NE_OK;
}


/*! @} */
