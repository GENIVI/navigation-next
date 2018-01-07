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
  @file        TemplatePinCushion.cpp
  @defgroup

  Description: Implementation of PinCushionImpl

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

#include "PinCushionImpl.h"

using namespace nbcommon;
using namespace nbmap;


/* See header file for description */

PinCushionImpl::PinCushionImpl(shared_ptr<PinManager> pinManagerPtr,
                                  shared_ptr<PinBubbleResolver > pinBubbleResolver,
                                  NB_Context* context)
        : Base(),
          PinCushion(),
          m_IsDefaultLayerCreated(false),
          m_pContext(context),
          m_pPinManager(pinManagerPtr),
          m_pLayerID(new string(DEFAULT_LAYER_NAME)),
          m_pPinBubbleResolver(pinBubbleResolver)
{
}

/* See header file for description */

PinCushionImpl::~PinCushionImpl()
{
    if (m_pPinLayer)
    {
        m_pPinLayer->RemoveAllPins();
    }
}

/* See description in PinCushion.h */

shared_ptr<std::string>
PinCushionImpl::DropPin(double latitude,
                           double longitude)
{
    shared_ptr<std::string> pinID;
    if (!m_pPinLayer)
    {
        m_pPinLayer = m_pPinManager->AddPinLayer(m_pLayerID);
    }
    if (!m_pPinLayer)
    {
        return pinID;
    }

    // Remove previously created Dropped Pin (Dropped pin should be unique.).
    if (m_pCurrentPinID)
    {
        m_pPinLayer->RemovePins(vector<shared_ptr<string> >(1, m_pCurrentPinID));
    }

    shared_ptr<BubbleInterface> bubble;
    if (m_pPinBubbleResolver)
    {
        bubble = m_pPinBubbleResolver->GetPinBubble(latitude, longitude);
    }

    // Add a new pin.
    vector<PinParameters<BubbleInterface> > parametersVector;
    parametersVector.push_back(PinParameters<BubbleInterface>(PT_COMMON,
                                                bubble,
                                                CustomPinInformationPtr(),
                                                latitude,
                                                longitude));
    m_pPinLayer->AddPins(parametersVector);

    if (!(parametersVector.empty()))
    {
        pinID = parametersVector.back().m_pinId;
        m_pCurrentPinID = pinID;
    }

    return pinID;
}

/* See description in PinCushion.h */
NB_Error
PinCushionImpl::RemovePin(shared_ptr<std::string> pinID)
{
    NB_Error error = NE_INVAL;
    if (pinID && m_pPinLayer)
    {
        m_pPinLayer->RemovePins(vector<shared_ptr<string> >(1, pinID));
        error = NE_OK;
    }
    //@todo: refresh Pins.
    return error;
}

shared_ptr<PinBubbleResolver>
PinCushionImpl::GetPinBubbleResolver()
{
    return m_pPinBubbleResolver;
}

NB_Error
PinCushionImpl::SetCustomPinBubbleResolver(shared_ptr<nbmap::PinBubbleResolver> resolver)
{
    NB_Error error = NE_INVAL;
    if (resolver)
    {
        m_pPinBubbleResolver = resolver;
    }
    //@todo: refresh Pins.
    return error;
}

/*! @} */
