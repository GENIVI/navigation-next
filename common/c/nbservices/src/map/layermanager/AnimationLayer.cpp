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
    @file       AnimationLayer.cpp

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

/*! @{ */

extern "C"
{
#include "nbqalog.h"
}

#include "AnimationLayer.h"
#include "AnimationTileCallback.h"
#include "FrameListLayer.h"
#include "StringUtility.h"
#include "MapLegendInfoImpl.h"


using namespace nbcommon;
using namespace nbmap;
using namespace std;

// AnimationLayer Public functions ..............................................................

/* See header file for description */
AnimationLayer::AnimationLayer(NB_Context* context,
                               uint32 digitalId,
                               shared_ptr<TileManager> tileManager)
        : UnifiedLayer(tileManager, digitalId, context)
{
    m_isOverlay        = true;
    m_isAnimationLayer = true;
}

/* See header file for description */
AnimationLayer::~AnimationLayer()
{
    // Set the flag false to specify this AnimationLayer is destroyed.
    if (m_thisValid)
    {
        *m_thisValid = false;
    }

    // Clear the frame list layer and unregister the listener.
    SetFrameListLayer(FrameListLayerPtr());
}

/* See description in UnifiedLayer.h */
void
AnimationLayer::GetTiles(vector<TileKeyPtr> /*tileKeys*/,
                         AsyncTileRequestWithRequestPtr /*callback*/,
                         uint32 /*priority*/)
{
    // Do nothing here. All animation tiles should be downloaded through GetAnimationTiles()
}

/* See description in UnifiedLayer.h */
UnifiedLayerPtr
AnimationLayer::Clone()
{
    return CloneWithTemplate<AnimationLayer>(this);
}

/* See description in FrameListListener.h */
void
AnimationLayer::FrameListUpdated(shared_ptr<map<uint32, FrameListEntryPtr> > frameList)
{
    m_pendingRequests.clear();
    RemoveAllTiles();

    // Save the new frame list.
    m_frameList = frameList;
}

/* See description in FrameListListener.h */
void
AnimationLayer::FrameListFailed(NB_Error error)
{
    // Clear the current frame list.
    m_frameList.reset();

    if (IsEnabled())
    {
        // Return the error to the pending requests.
        vector<PendingRequestEntryPtr>::const_iterator entryIterator = m_pendingRequests.begin();
        vector<PendingRequestEntryPtr>::const_iterator entryEnd = m_pendingRequests.end();
        for (; entryIterator != entryEnd; ++entryIterator)
        {
            PendingRequestEntryPtr entry = *entryIterator;
            if (entry)
            {
                shared_ptr<AsyncCallback<TilePtr> > callback = entry->m_callback;
                if (callback)
                {
                    // Return the error for each tile key.
                    vector<TileKeyPtr>::const_iterator tileKeyIterator = entry->m_tileKeys.begin();
                    vector<TileKeyPtr>::const_iterator tileKeyEnd = entry->m_tileKeys.end();
                    for (; tileKeyIterator != tileKeyEnd; ++tileKeyIterator)
                    {
                        callback->Error(error);
                    }
                }
            }
        }
    }

    // Clear the pending requests.
    m_pendingRequests.clear();
    RemoveAllTiles();
}

/* See description in header file. */
void AnimationLayer::GetAnimationTiles(const vector <TileKeyPtr>& tileKeys,
                                       const vector <uint32>& timeStamps,
                                       shared_ptr <AsyncCallback <TilePtr> > callback,
                                       uint32 priority)
{
    if ((!IsEnabled()) || tileKeys.empty())
    {
        // Do nothing if this layer is disabled or there is no tile keys to request.
        return;
    }

    // Check if this layer is initialized properly.
    if ((!m_tileLayerInfo) || (!m_tileManager))
    {
        NotifyErrorForTileKeys(NE_NOTINIT, callback, tileKeys);
        return;
    }

    NB_Error error = NE_OK;

    // Check if the current frame list is valid.
    if ((!m_frameList || timeStamps.empty()) && m_frameListLayer)
    {
        // Request the frame list.
        shared_ptr<map<uint32, FrameListEntryPtr> > frameList;
        error = m_frameListLayer->GetFrameList(frameList);
        if (error == NE_OK)
        {
            // Save the frame list.
            m_frameList = frameList;
        }
        else if (error == NE_BUSY)
        {
            // Save the pending request to wait for the frame list updated.
            // @note: Pending requests are only used to report error, currently.
            PendingRequestEntryPtr pendingRequestEntry(new PendingRequestEntry(tileKeys,
                                                                               callback,
                                                                               priority));
            if (!pendingRequestEntry)
            {
                NotifyErrorForTileKeys(NE_NOMEM, callback, tileKeys);
                return;
            }
            m_pendingRequests.push_back(pendingRequestEntry);
            return;
        }
        else
        {
            // Notify the error.
            NotifyErrorForTileKeys(error, callback, tileKeys);
            return;
        }
    }

    if ((!m_frameList) || (m_frameList->empty()))
    {
        // Return NE_NOENT if the frame list is empty.
        NotifyErrorForTileKeys(NE_NOENT, callback, tileKeys);
        return;
    }

    // Request the tile for each frame.
    // @note: Frames shall be requested "breadth first" by the requirements.
    size_t frameCount = timeStamps.size();
    map<uint32, FrameListEntryPtr>::const_iterator end = m_frameList->end();

    for (size_t i = 0; i < frameCount; ++i)
    {
        // Request the tile for each tile key.
        vector<TileKeyPtr>::const_iterator tileKeyIterator = tileKeys.begin();
        vector<TileKeyPtr>::const_iterator tileKeyEnd = tileKeys.end();
        for (; tileKeyIterator != tileKeyEnd; ++tileKeyIterator)
        {
            // Get the requested tile key.
            TileKeyPtr requestedTileKey = *tileKeyIterator;
            if ((!requestedTileKey) ||
                (requestedTileKey->m_zoomLevel < (int) (m_tileLayerInfo->minZoom)) ||
                (requestedTileKey->m_zoomLevel > (int) (m_tileLayerInfo->maxZoom)))
            {
                if (callback)
                {
                    callback->Error(NE_NOENT);
                }
                continue;
            }

            // Create the template parameters by the tile key.
            shared_ptr<map<string, string> > templateParameters;
            error = CreateTemplateParametersByTileKey(requestedTileKey, templateParameters);
            if (error != NE_OK)
            {
                if (callback)
                {
                    callback->Error(error);
                }
                continue;
            }
            if (!templateParameters)
            {
                if (callback)
                {
                    callback->Error(NE_UNEXPECTED);
                }
                continue;
            }

            shared_ptr<string> frameId;
            shared_ptr<string> timeStampString;
            uint32 timeStamp = timeStamps.at(i);
            map<uint32, FrameListEntryPtr>::const_iterator result =
                    m_frameList->find(timeStamp);
            if (result != end && result->second)
            {
                frameId = result->second->GetFrameId();
                timeStampString = result->second->GetTimeStampString();
            }

            if (!frameId || !timeStampString)
            {
                if (callback)
                {
                    callback->Error(NE_UNEXPECTED);
                }
                continue;
            }

            // Copy the template parameters for the tile key.
            shared_ptr<map<string, string> > frameIdParameters(
                new map<string, string>(*templateParameters));
            if (!frameIdParameters)
            {
                if (callback)
                {
                    callback->Error(NE_NOMEM);
                }
                continue;
            }

            // Add the frame ID to the template parameters.
            frameIdParameters->insert(pair<string, string>(PARAMETER_KEY_FRAME_ID, *frameId));
            frameIdParameters->insert(pair<string, string>(PARAMETER_KEY_TIME_STAMP,
                                                           *timeStampString));

            // @todo: Should we add the frame ID to the QA log?
            // NB_QaLogTileRequest(m_pContext,
            //                     requestedTileKey->m_x,
            //                     requestedTileKey->m_y,
            //                     requestedTileKey->m_zoomLevel);

            /* Create the flag identified if the AnimationLayer is valid. It is set to
               false in the destructor of class AnimationLayer and used in the
               AnimationTileCallback.
            */
            if (!m_thisValid)
            {
                m_thisValid = shared_ptr<bool>(new bool(true));
                if (!m_thisValid)
                {
                    if (callback)
                    {
                        callback->Error(NE_NOMEM);
                    }
                    continue;
                }
            }

            // Create a callback for the request of the animation tile.
            shared_ptr<AnimationTileCallback> animationTileCallback(
                new AnimationTileCallback(this, m_thisValid, requestedTileKey, callback,
                                          timeStamp));
            if (!animationTileCallback)
            {
                if (callback)
                {
                    callback->Error(NE_NOMEM);
                }
                continue;
            }

            /* Request the animation tile of the frame. The same frame ID for different
               tile keys should have the same download priority.
            */
            m_tileManager->GetTile(frameIdParameters, animationTileCallback, priority);
        }
    }
}

/* See header file for description */
void
AnimationLayer::SetFrameListLayer(FrameListLayerPtr frameListLayer)
{
    if (m_frameListLayer.get() == frameListLayer.get())
    {
        // Do nothing if the frame list layers are same.
        return;
    }

    // Save the new frame list layer.
    m_frameListLayer = frameListLayer;
}

void
AnimationLayer::SetMapLegend(shared_ptr<protocol::MapLegend> mapLegend)
{
    m_pMapLegend = mapLegend;
    if (!m_pMapLegendInfo)
    {
        m_pMapLegendInfo.reset(new MapLegendInfoImpl);
        if (!m_pMapLegendInfo)
        {
            return;
        }
    }

    m_pMapLegendInfo->FeedWithMapLegend(mapLegend);
}

/* See description in header file. */
shared_ptr <MapLegendInfo> AnimationLayer::GetMapLegendInfo()
{
    return m_pMapLegendInfo;
}


void
AnimationLayer::SetAnimationInfo(shared_ptr<protocol::AnimationInfo> animationInfo)
{
    m_pAnimationInfo = animationInfo;
}

shared_ptr<string>
AnimationLayer::GetFrameListLayerID()
{
    shared_ptr<string> frameListLayerId;
    if (m_pAnimationInfo)
    {
        frameListLayerId = m_pAnimationInfo->GetFrameList();
    }
    return frameListLayerId;
}

// Private functions ............................................................................

/* See header file for description */
AnimationLayer::AnimationLayer(const AnimationLayer& layer)
        : UnifiedLayer(layer),
          m_frameListLayer(layer.m_frameListLayer), // All animation layers share the same frame list layer.
          m_pMapLegend(layer.m_pMapLegend),
          m_pMapLegendInfo(layer.m_pMapLegendInfo),
          m_pAnimationInfo(layer.m_pAnimationInfo)
{
    m_isOverlay        = layer.m_isOverlay ;
    m_isAnimationLayer = layer.m_isAnimationLayer;
}

/*! Notify an error for the tile keys through a callback

    @return None
*/
void
AnimationLayer::NotifyErrorForTileKeys(NB_Error error,                                                      /*!< Error to notify */
                                       shared_ptr<AsyncCallback <TilePtr> > callback, /*!< Callback used to notify the error */
                                       vector<TileKeyPtr> tileKeys                                          /*!< Tile keys returned the error for */
                                       )
{
    if ((error == NE_OK) || (!callback))
    {
        // Do nothing if there is no error or the callback is empty.
        return;
    }

    // Notify the error for each tile key.
    vector<TileKeyPtr>::const_iterator tileKeyIterator = tileKeys.begin();
    vector<TileKeyPtr>::const_iterator tileKeyEnd = tileKeys.end();
    for (; tileKeyIterator != tileKeyEnd; ++tileKeyIterator)
    {
        callback->Error(error);
    }
}


// PendingRequestEntry functions ................................................................

/* See header file for description */
AnimationLayer::PendingRequestEntry::PendingRequestEntry(const vector<TileKeyPtr>& tileKeys,
                                                         shared_ptr<AsyncCallback<TilePtr> > callback,
                                                         uint32 priority)
: Base(),
  m_tileKeys(tileKeys),
  m_callback(callback),
  m_priority(priority)
{
    // Nothing to do here.
}

/* See header file for description */
AnimationLayer::PendingRequestEntry::~PendingRequestEntry()
{
    // Nothing to do here.
}


/*! @} */
