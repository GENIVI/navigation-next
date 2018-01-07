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
  @file        GenericTileCallback.cpp

  Implementation of GenericTileCallback, refer to header file more
  description.
*/
/*
   (C) Copyright 2012 by TeleCommunications Systems, Inc.

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
#include "nbqalog.h"
#include "cslqarecorddefinitions.h"
#include "nbcontextprotected.h"
}
#include "GenericTileCallback.h"
#include "UnifiedLayerManager.h"
#include "TileInternal.h"

using namespace std;
using namespace nbmap;

// Implementation of RetryGetLayerCallback, refer to header file for descriptions.
RetryGetLayerCallback::RetryGetLayerCallback(LayerManagerWrapperForCallbackPtr manager)
{
    m_layerManager = manager;
}

RetryGetLayerCallback::~RetryGetLayerCallback()
{
}

void RetryGetLayerCallback::Success(const vector<LayerPtr>& layers)
{
    if (m_layerManager->m_pManager)
    {
        m_layerManager->m_pManager->UpdateLayers(layers);
    }
}

void RetryGetLayerCallback::Error(NB_Error /*error*/)
{
    if (m_layerManager->m_pManager)
    {
        vector<LayerPtr> layers;
        m_layerManager->m_pManager->UpdateLayers(layers);
    }
}


// Implementation of GenericLayerCallback, refer to header file for descriptions.
GenericLayerCallback::GenericLayerCallback(LayerManagerWrapperForCallbackPtr manager,
                                           NB_Context* context,
                                           shared_ptr<AsyncCallback<const vector<LayerPtr>& > > callback)
{
    m_pUnifiedLayerManager = manager;
    m_callback             = callback;
    m_pContext             = context;
    m_startTimeStamp       = PAL_ClockGetTimeMs();
}

GenericLayerCallback::~GenericLayerCallback()
{
}

void GenericLayerCallback::Success(const vector<LayerPtr>& layers)
{
    WriteQaLogPerformance();
    if(m_pUnifiedLayerManager && m_pUnifiedLayerManager->m_pManager)
    {
        m_pUnifiedLayerManager->m_pManager->LayerRequestSuccess(layers);
    }
}

void GenericLayerCallback::Error(NB_Error error)
{
    WriteQaLogPerformance(false);
    if(m_pUnifiedLayerManager && m_pUnifiedLayerManager->m_pManager)
    {
        m_pUnifiedLayerManager->m_pManager->LayerRequestError(error);
    }
}

/*! Writes Performance QaLog.

    @return None.
*/
void GenericLayerCallback::WriteQaLogPerformance(bool succeeded)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(m_pContext) == NULL)
    {
        return;
    }

    uint32 duration = PAL_ClockGetTimeMs() - m_startTimeStamp;
    string message("Request metadata finished, result: ");
    if (succeeded)
    {
        message += "Succeeded.";
    }
    else
    {
        message += "Failed.";
    }
    if (CSL_QaLogIsVerboseLoggingEnabled(NB_ContextGetQaLog(m_pContext)))
    {
        NB_QaLogPerformance(m_pContext, message.c_str(), duration);
    }
}

/* See description in header file. */
LayerManagerWrapperForCallback::LayerManagerWrapperForCallback(UnifiedLayerManager* manager)
        : m_pManager(manager)
{
}

/* See description in header file. */
LayerManagerWrapperForCallback::~LayerManagerWrapperForCallback()
{
}

// Implementation of TileRequestCallback, refer to header file for descriptions.

/* See description in header file. */
TileRequestCallback::TileRequestCallback(LayerManagerWrapperForCallbackPtr layerManager,
                                         shared_ptr < AsyncCallbackWithRequest < TileKeyPtr, TilePtr > > callback)
        : m_pLayerManager(layerManager),
          m_pCallbackWithRequest(callback),
          m_pCallbackWithoutRequest()
{
}

/* See description in header file. */
TileRequestCallback::TileRequestCallback(LayerManagerWrapperForCallbackPtr layerManager,
                                         shared_ptr < AsyncCallback < TilePtr > > callback)
        : m_pLayerManager(layerManager),
          m_pCallbackWithRequest(),
          m_pCallbackWithoutRequest(callback)
{
}


/* See description in header file. */
TileRequestCallback::~TileRequestCallback()
{
}

/* See description in header file. */
void TileRequestCallback::Success(TileKeyPtr request, TilePtr response)
{
    if (m_pCallbackWithRequest)
    {
        if (request && response)
        {
            m_pCallbackWithRequest->Success(request, response);
        }
        else
        {
            m_pCallbackWithRequest->Error(request, NE_INVAL);
        }
    }
    else if (m_pCallbackWithoutRequest)
    {
        m_pCallbackWithoutRequest->Success(response);
    }
}

/* See description in header file. */
void TileRequestCallback::Error(TileKeyPtr request, NB_Error error)
{
    // Notify UnifiedLayerManager to record errors, this is needed for handling 404 errors.
    if (m_pLayerManager->m_pManager)
    {
        m_pLayerManager->m_pManager->TileRequestError(error);
    }

    if (m_pCallbackWithRequest)
    {
        m_pCallbackWithRequest->Error(request, error);
    }
    else if (m_pCallbackWithoutRequest)
    {
        m_pCallbackWithoutRequest->Error(error);
    }
}

/*! @} */
