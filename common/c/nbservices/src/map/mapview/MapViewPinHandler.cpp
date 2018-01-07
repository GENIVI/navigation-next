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
 @file     MapViewPinHandler.cpp
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

#include "MapViewPinHandler.h"
#include "transformutility.h"
#include "StringUtility.h"
#include "nbgmtypes.h"
#include "PinImpl.h"
#include "palclock.h"
#include <limits>

static const char CUSTOM_PIN_LAYER_ID[] = "custom_pin_layer";
const char ID_SEPERATOR[] = ":";
#define RGBA_TO_BGRA(X) ((X & 0x00FF00FF) | ((X >> 16) & 0xFF00) | ((X & 0xFF00) << 16))

using namespace nbmap;
using namespace nbcommon;

MapViewPinHandler::MapViewPinHandler(shared_ptr<string> materialConfigPath)
        :m_currentGeneratedID(0),
         m_IdSeperator(ID_SEPERATOR),
         m_materialParser(materialConfigPath ? materialConfigPath->c_str() : NULL)
{

}

MapViewPinHandler::~MapViewPinHandler()
{

}

/* See header for description */
void
MapViewPinHandler::AddPin(PinParameters<BubbleInterface>& pinParameters, NBGM_PinParameters& NBGMPinParameters)
{
    double locationX = 0.0;
    double locationY = 0.0;

    NBGMPinParameters.pinMaterialIndex = 0;

    if (pinParameters.m_type == PT_CUSTOM && pinParameters.m_customPinInformation)
    {
        CustomPinInformationPtr customPinInfo= pinParameters.m_customPinInformation;

        //UnSelected Pin Info
        NBGMPinParameters.unSelectedCalloutXCenterOffset = customPinInfo->m_unselectedCalloutOffsetX;
        NBGMPinParameters.unSelectedCalloutYCenterOffset = customPinInfo->m_unselectedCalloutOffsetY;
        NBGMPinParameters.unSelectedBubbleXCenterOffset = customPinInfo->m_unselectedBubbleOffsetX;
        NBGMPinParameters.unSelectedBubbleYCenterOffset = customPinInfo->m_unselectedBubbleOffsetY;
        NBGMPinParameters.unSelectedCircleInteriorColor = RGBA_TO_BGRA(customPinInfo->m_unselectedCircleInteriorColor);
        NBGMPinParameters.unSelectedCircleOutlineBitOnColor = RGBA_TO_BGRA(customPinInfo->m_unselectedCircleOutLineColor);
        NBGMPinParameters.unSelectedCircleOutlineBitOffColor = 0;
        NBGMPinParameters.displayunSelectedWidth = customPinInfo->m_unselectedImageWidth;
        NBGMPinParameters.dispalyunSelectedHeight = customPinInfo->m_unselectedImageHeight;
        NBGMPinParameters.unSelectedDataBinary.size = customPinInfo->m_unselectedImage->GetDataSize();
        if (NBGMPinParameters.unSelectedDataBinary.size > 0)
        {
            NBGMPinParameters.unSelectedDataBinary.addr = new uint8[NBGMPinParameters.unSelectedDataBinary.size];
            if (NBGMPinParameters.unSelectedDataBinary.addr)
            {
                nsl_memset(NBGMPinParameters.unSelectedDataBinary.addr, 0, NBGMPinParameters.unSelectedDataBinary.size);
                customPinInfo->m_unselectedImage->GetData(NBGMPinParameters.unSelectedDataBinary.addr, 0, NBGMPinParameters.unSelectedDataBinary.size);
            }
        }

        //Selected Pin Info
        NBGMPinParameters.selectedCalloutXCenterOffset = customPinInfo->m_selectedCalloutOffsetX;
        NBGMPinParameters.selectedCalloutYCenterOffset = customPinInfo->m_selectedCalloutOffsetY;
        NBGMPinParameters.selectedBubbleXCenterOffset = customPinInfo->m_selectedBubbleOffsetX;
        NBGMPinParameters.selectedBubbleYCenterOffset = customPinInfo->m_selectedBubbleOffsetY;
        NBGMPinParameters.selectedCircleInteriorColor = RGBA_TO_BGRA(customPinInfo->m_selectedCircleInteriorColor);
        NBGMPinParameters.selectedCircleOutlineBitOnColor = RGBA_TO_BGRA(customPinInfo->m_selectedCircleOutLineColor);
        NBGMPinParameters.selectedCircleOutlineBitOffColor = 0;
        NBGMPinParameters.displaySelectedWidth = customPinInfo->m_selectedImageWidth;
        NBGMPinParameters.dispalySelectedHeight = customPinInfo->m_selectedImageHeight;
        NBGMPinParameters.selectedDataBinary.size = customPinInfo->m_selectedImage->GetDataSize();
        if (NBGMPinParameters.selectedDataBinary.size > 0)
        {
            NBGMPinParameters.selectedDataBinary.addr = new uint8[NBGMPinParameters.selectedDataBinary.size];
            if (NBGMPinParameters.selectedDataBinary.addr)
            {
                nsl_memset(NBGMPinParameters.selectedDataBinary.addr, 0, NBGMPinParameters.selectedDataBinary.size);
                customPinInfo->m_selectedImage->GetData(NBGMPinParameters.selectedDataBinary.addr, 0, NBGMPinParameters.selectedDataBinary.size);
            }
        }

        NBGMPinParameters.circleOutlineWidth = customPinInfo->m_circleOutlineWidth;
    }
    else
    {
        // From pin material
        m_materialParser.GetPinMaterialIndex(pinParameters.m_type, NBGMPinParameters.pinMaterialIndex);
        NBGMPinParameters.pinMaterialIndex |= 0x8000;
    }

    //customPinInfo->m_customLayerPinID
    //customPinInfo->m_customLayerMargin
    NBGMPinParameters.linePattern = 0xFFFFFFFF;
    pinParameters.m_pinId = GeneratePinID();

    NBGMPinParameters.pinID = *(pinParameters.m_pinId);
    NBGMPinParameters.groupID = *(pinParameters.m_groupId);

    mercatorForward(pinParameters.m_latitude, pinParameters.m_longitude, &locationX, &locationY);
    NBGMPinParameters.locationX = locationX;
    NBGMPinParameters.locationY = locationY;
    NBGMPinParameters.radius = pinParameters.m_circleRadius;

    PinPtr pin(new PinImpl<BubbleInterface>(pinParameters.m_type,
                              pinParameters.m_pinId,
                              pinParameters.m_groupId,
                              pinParameters.m_bubble,
                              pinParameters.m_customPinInformation,
                              pinParameters.m_latitude,
                              pinParameters.m_longitude,
                              pinParameters.m_circleRadius));

    m_pins[*(pinParameters.m_pinId)] = pin;
}

/* See header for description */
void
MapViewPinHandler::RemovePins(const vector<shared_ptr<string> >& pinIDs)
{
    for (uint32 i = 0; i < pinIDs.size(); ++i)
    {
        m_pins.erase(*(pinIDs[i]));
    }
}

/* See header for description */
void
MapViewPinHandler::RemoveAllPins()
{
    m_pins.clear();
}

/* See header for description */
void
MapViewPinHandler::UpdatePinPosition(shared_ptr<string> pinID, double lat, double lon)
{
    if (m_pins.find(*pinID) != m_pins.end())
    {
        PinImpl<BubbleInterface>* pinIml = static_cast<PinImpl<BubbleInterface>*>(m_pins[*pinID].get());
        pinIml->SetLatitude(lat);
        pinIml->SetLongitude(lon);
    }

}

/* See header for description */
PinPtr
MapViewPinHandler::GetPin(shared_ptr<string> pinID)
{
    if (m_pins.find(*pinID) != m_pins.end())
    {
        return m_pins[*pinID];
    }

    return PinPtr();
}

/* See header for description */
shared_ptr<BubbleInterface>
MapViewPinHandler::GetBubble(shared_ptr<string> pinID)
{
    if(m_pins.find(*pinID) != m_pins.end())
    {
        PinImpl<BubbleInterface>* pinIml = static_cast<PinImpl<BubbleInterface>*>(m_pins[*pinID].get());

        return pinIml->GetBubble();
    }

    return shared_ptr<BubbleInterface>();
}

/* See header for description */
shared_ptr<string>
MapViewPinHandler::GeneratePinID()
{
    // This segment is added for extra protection, but I don't think it will be used ...
    if (++m_currentGeneratedID == numeric_limits<uint32>::max())
    {
        m_IdSeperator += StringUtility::NumberToString(PAL_ClockGetUnixTime());
        m_currentGeneratedID = 0;
    }

    // Just increase m_currentGeneratedID and append it to form a unique id.
    shared_ptr<string> pinID(new string(CUSTOM_PIN_LAYER_ID + m_IdSeperator +
                                        StringUtility::NumberToString(m_currentGeneratedID)));
    return pinID;
}


