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
   @file        TileFactory.cpp
   @defgroup    nbmap

   Description: Implementation of TileFactory.

*/
/*
   (C) Copyright 2014 by TeleCommunications Systems, Inc.

   The information contained herein is confidential, proprietary to
   TeleCommunication Systems, Inc., and considered a trade secret as defined
   in section 499C of the penal code of the State of California. Use of this
   information by anyone other than authorized employees of TeleCommunication
   Systems is granted only under a written non-disclosure agreement, expressly
   prescribing the scope and manner of such use.

 --------------------------------------------------------------------------*/

/*! @{ */

extern "C"
{
#include "palclock.h"
}

#include "TileFactory.h"
#include "datastreamimplementation.h"
#include "NBMFileHeader.h"
#include "NBMTextureImageChunk.h"
#include "NBMPinChunk.h"
#include "NBMTextChunk.h"
#include "NBMMaterialChunk.h"
#include "StringUtility.h"
#include "nbgm.h"

using namespace nbcommon;

// @todo: These value should be defined in NBM library.
const uint16 TEMP_NBM_LAYER_ID_POIS = 7;

#define NBM_FILE_REFNAME_GPIN "GPIN"
#define RGBA_TO_BGRA(X) ((X & 0x00FF00FF) | ((X >> 16) & 0xFF00) | ((X & 0xFF00) << 16))

using namespace nbmap;
using namespace nbcommon;

// Local functions ...........................................................

bool IsCustomLayerPinInformationEuqal(const CustomPinInformationPtr& left,
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

bool IsCustomLayerPinInformationLess(const CustomPinInformationPtr& left,
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

TileFactory::TileFactory(NB_Context* context)
{
    m_pContext  = context;
    m_currentID = 0;
}

TileFactory::~TileFactory()
{
}

NB_Context*  TileFactory::GetContext()
{
    return m_pContext;
}

shared_ptr<string>
TileFactory::GeneratePinID()
{
    shared_ptr<string> pinID(new string(StringUtility::NumberToString(PAL_ClockGetUnixTime()) + "_" +
                                        StringUtility::NumberToString(m_currentID++)));
    return pinID;
}

NB_Error TileFactory::GenerateCustomLayerTileData(const TileKey& tileKey,
                                                  const vector<PinParameters<nbmap::BubbleInterface> >& parametersVector,
                                                  CustomNBMTile*& customNBMTile)
{
    NB_Error error = NE_OK;
    m_pinMaterialFilter.clear();
    // Check if there are some pins.
    if (parametersVector.empty())
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
    map<string, int> markerIDMap;

    // Add data of pin to chunks.
    vector<PinParameters<nbmap::BubbleInterface> >::const_iterator pinParaIterator = parametersVector.begin();
    vector<PinParameters<nbmap::BubbleInterface> >::const_iterator pinParaEnd = parametersVector.end();
    for (; pinParaIterator != pinParaEnd; ++pinParaIterator)
    {
        const PinParameters<nbmap::BubbleInterface>& pinPara = *pinParaIterator;

        // Get pin ID.
        shared_ptr<string> pinId = GeneratePinID();
        if ((!pinId) || pinId->empty())
        {
            continue;
        }

        // Get both unselected and selected indexes of the pin materials.
        uint16 pinMaterialIndex = NBM_INVALIDATE_INDEX;
        PinType type = pinPara.m_type;
        switch (type)
        {
            //TODO: Later, here maybe need handle the non-custom condition.
            /*
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
            */
            case PT_CUSTOM:
            {
                error = GetCustomPinMaterialChunkIndex(pinPara.m_customPinInformation,
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
        POINT2 point2 = {static_cast<float>(pinPara.m_longitude),
                         static_cast<float>(pinPara.m_latitude)};

        NBMIndex pinIndex = pinChunk->SetData(pinMaterialIndex,
                                              point2,
                                              textIndex,
                                              pinPara.m_circleRadius);
        if (pinIndex == NBM_INVALIDATE_INDEX)
        {
            m_pinMaterialFilter.clear();
            return NE_UNEXPECTED;
        }

        markerIDMap.insert(std::pair<string, int>(*pinId, pinPara.m_customPinInformation->m_customLayerPinID));
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

    //TODO: 'customNBMTile' be wrapped with shared_ptr by invoke side.
    //so here will cause misunderstanding of memory leak.
    customNBMTile = new CustomNBMTile(tileKey, dataStream, markerIDMap);
    markerIDMap.clear();
    return NE_OK;
}

// PinMaterialKey functions .....................................................................

/*! PinMaterialKey constructor */
TileFactory::PinMaterialKey::PinMaterialKey(CustomPinInformationPtr pinInformation)
        : Base(),
          m_pinInformation(pinInformation)
{
}

/*! PinMaterialKey destructor */
TileFactory::PinMaterialKey::~PinMaterialKey()
{
}

/*! PinMaterialKey copy constructor */
TileFactory::PinMaterialKey::PinMaterialKey(const PinMaterialKey& key)
        : Base(),
          m_pinInformation(key.m_pinInformation)
{
}

/*! PinInformation assignment operator */
TileFactory::PinMaterialKey&
TileFactory::PinMaterialKey::operator=(const PinMaterialKey& key)
{
    if ((&key) != this)
    {
        this->m_pinInformation = key.m_pinInformation;
    }

    return *this;
}

/*! Equal operator */
bool
TileFactory::PinMaterialKey::operator==(const PinMaterialKey& anotherKey) const
{
    return IsCustomLayerPinInformationEuqal(m_pinInformation, anotherKey.m_pinInformation);
}

/*! Less operator */
bool
TileFactory::PinMaterialKey::operator<(const PinMaterialKey& anotherKey) const
{
    return IsCustomLayerPinInformationLess(m_pinInformation, anotherKey.m_pinInformation);
}

/*! Get material chunk index for custom pin. */
NB_Error
TileFactory::GetCustomPinMaterialChunkIndex(CustomPinInformationPtr pinInformation,
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

    // 2. Create the poi body.
    MaterialPOIBody poiBody;
    poiBody.icon = 0;
    poiBody.selectedPin = 0;
    poiBody.unselectedPin = 0;
    poiBody.cxOffset = pinMaterialKey.m_pinInformation->m_unselectedCalloutOffsetX;
    poiBody.cyOffset = pinMaterialKey.m_pinInformation->m_unselectedCalloutOffsetY;
    poiBody.bxOffset = pinMaterialKey.m_pinInformation->m_unselectedBubbleOffsetX;
    poiBody.byOffset = pinMaterialKey.m_pinInformation->m_unselectedBubbleOffsetY;
    poiBody.distanceToObj = (signed char)(((pinMaterialKey.m_pinInformation->m_customLayerMargin)*160.0f) / (NBGM_GetConfig()->dpi));
    poiBody.distanceToLab = (signed char)(((pinMaterialKey.m_pinInformation->m_customLayerMargin)*160.0f) / (NBGM_GetConfig()->dpi));
    poiBody.distanceToPoi = (signed char)(((pinMaterialKey.m_pinInformation->m_customLayerMargin)*160.0f) / (NBGM_GetConfig()->dpi));
    poiBody.width = (signed char)(((pinMaterialKey.m_pinInformation->m_selectedImageWidth)*160.0f) / (NBGM_GetConfig()->dpi));
    poiBody.height = (signed char)(((pinMaterialKey.m_pinInformation->m_selectedImageHeight)*160.0f) / (NBGM_GetConfig()->dpi));

    // Add image data to material chunk.
    pinMaterialIndex = materialChunk->SetData(bitmapBody[0], bitmapBody[0], bitmapBody[0], poiBody);
    if (pinMaterialIndex == NBM_INVALIDATE_INDEX)
    {
        return NE_UNEXPECTED;
    }

    m_pinMaterialFilter.insert(pinMaterialIterator, make_pair(pinMaterialKey, pinMaterialIndex));
    return NE_OK;
}

/*! @} */
