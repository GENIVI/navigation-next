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
    @file       FrameListLayer.cpp

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
#include "nbcontextprotected.h"
}

#include "FrameListLayerDecorator.h"
#include "datastream.h"
#include "FrameListListener.h"
#include "FrameListTileCallback.h"
#include "StringUtility.h"
#include "AnimationLayer.h"
#include "TileRefresher.h"
#include "TileImpl.h"

using namespace nbcommon;
using namespace nbmap;
using namespace std;

// Local Constants ..............................................................................

// Buffer size used to read the data parsed to the frame list
#define BUFFER_SIZE 256

// Separator of the timestamp and frame ID
//const char FRAME_LIST_SEPARATOR[] = ",";

/* Line break for each frame */
//const char FRAME_LIST_LINE_BREAK[] = "\r\n";

// 1 second equals to 1000 milliseconds
//const uint32 MILLISECONDS_PER_SECOND = 1000;
//const uint32 DEFAULT_REFRESH_SECONDS = 20;

static const TileKeyPtr FRAME_LIST_KEY(new TileKey(-1, -1, -1));

// Local Types ..................................................................................

// Local function prototype
/*! Functor used to compare two maps. */
class FrameListEntryCompareFunctor
{
public:
    FrameListEntryCompareFunctor()
    {
    }

    virtual ~FrameListEntryCompareFunctor()
    {
    }

    bool operator() (const pair<int, FrameListEntryPtr>& p1,
                     const pair<int, FrameListEntryPtr>& p2) const
    {
        bool result = false;
        if (p1.first == p2.first && p1.second && p2.second &&
            *(p1.second) == *(p2.second))
        {
            result = true;
        }
        return result;
    }
};

// Public functions .............................................................................

/* See header file for description */
FrameListLayer::FrameListLayer(NB_Context* context,
                               uint32 digitalId,
                               shared_ptr<TileManager> tileManager)
        : UnifiedLayer(tileManager, digitalId, context),
          TileExpireListener(),
          m_frameList(new FrameListMap),
          m_thisValid(new bool(true)),
          m_requestInProgress(false),
          m_tileRefresher(new TileRefresher(m_tileLayerInfo, context))
{
    m_isAnimationLayer = true;
    m_tileRefresher->RegisterListener(this);
}


/* See description in header file. */
FrameListLayer::FrameListLayer(const FrameListLayer& layer)
        : UnifiedLayer(layer),
          TileExpireListener(),
          m_frameList(layer.m_frameList),
          m_thisValid(layer.m_thisValid),
          m_requestInProgress(false),
          m_tileRefresher()
{
    m_isAnimationLayer = true;
    // Do not need to register to TileRefresher, there is only one FrameListLayer instance
    // that should be registered to it
}

/* See header file for description */
FrameListLayer::~FrameListLayer()
{
    if (!m_tileRefresher)
    {
        return;
    }

    m_tileRefresher->UnregisterListener(this);
    // Set the valid flag to false.
    if (m_thisValid)
    {
        *m_thisValid = false;
    }

    if (m_tileCallback)
    {
        // Set NULL to the callback.
        m_tileCallback->SetFrameListLayer(NULL);
    }
}

/* See description in UnifiedLayer.h */
void
FrameListLayer::GetTiles(vector<TileKeyPtr> /*tileKeys*/,
                         AsyncTileRequestWithRequestPtr /*callback*/,
                         uint32 /*priority*/)
{
    /* Nothing to do here. Because the frame list is not related with the tile
       keys. The AnimationLayer uses the function GetFrameList to request the
       latest frame list.
    */
}

/* See header file for description */
NB_Error
FrameListLayer::GetFrameList(FrameListMapPtr& frameList)
{
    if (m_frameList && !m_frameList->empty())
    {
        // Return the saved frame list if it is valid.
        frameList = m_frameList;
        return NE_OK;
    }

    // Request the latest frame list.
    NB_Error error = RequestFrameList();
    if (error == NE_OK)
    {
        /* Return NE_BUSY if the frame list has not been requested yet, or it is
           downloading in progress.
        */
        return NE_BUSY;
    }

    return error;
}

/* See header file for description */
void
FrameListLayer::TileRequestSuccess(TilePtr tile)
{
    // Reset the flag to identify that the request of the frame list is returned.
    m_requestInProgress = false;

    TileImpl* tileImpl = static_cast<TileImpl*>(tile.get());
    // Check if the tile is valid.
    if (!tile || !m_tileRefresher || !tileImpl)
    {
        TileRequestError(NE_UNEXPECTED);
        return;
    }

    tileImpl->SetTileKey(FRAME_LIST_KEY);

    // Create the new frame list.
    FrameListMap newFrameList;

    // Check if dataStream is OK to read, and alloc memory for reading data.
    DataStreamPtr dataStream = tile->GetData();
    uint32 dataSize = 0;
    uint8* buffer   = NULL;

    if (dataStream)
    {
        dataSize = dataStream->GetDataSize();
        if (dataSize > 0)
        {
            buffer = (uint8*)nsl_malloc(dataSize);
            if (buffer)
            {
                memset(buffer, 0, dataSize);
            }
        }
    }

    if (!buffer || dataSize == 0)
    {
        return;
    }

    // Now read data into allocated buffer.
    uint32 dataSizeGot  = 0;
    uint32 remainedSize = dataSize;

    while (dataSizeGot != dataSize && remainedSize > 0)
    {
        dataSizeGot += dataStream->GetData(buffer + dataSizeGot, dataSizeGot, remainedSize);
        remainedSize = dataSize - dataSizeGot;
    }

    char* ptr = (char*)buffer;
    uint32 dataParsed  = 0;
    vector<const char*> ptrList;
    for (; dataParsed != dataSize; ++dataParsed)
    {
        char* pos = (char*)buffer + dataParsed;
        switch (*pos)
        {
            case ',': // Comma found, treat as separator.
            {
                *pos = '\0';
                ptrList.push_back(ptr);
                ptr = ++ pos;
                break;
            }
            case '\r':
            {
                if (*(pos+1) == '\n') // CRLF found, treat as end of line.
                {
                    *pos = '\0';
                    *(pos+1) = '\0';
                    ptrList.push_back(ptr);
                    if (dataParsed != dataSize)
                    {
                        ptr = pos + 2;
                    }

                    // According to SDS, a frame must ends with CRLF.
                    FrameListEntryPtr entry = FrameListEntry::FeedWithCharArray(ptrList);
                    if (entry)
                    {
                        newFrameList.insert(make_pair(entry->GetTimeStamp(), entry));
                    }
                    ptrList.clear();
                    continue;
                }
                break;
            }
            default: // bypass this character.
            {
                break;
            }
        }
    }

    if (buffer)
    {
        nsl_free(buffer);
    }

    shared_ptr<map<string, shared_ptr<string> > > additionalData = tile->GetAdditionalData();
    if (additionalData)
    {
        shared_ptr<string> refreshString;
        map<string, shared_ptr<string> > :: iterator result =
                additionalData->find(HTTP_HEADER_TILE_REFRESH);
        if (result != additionalData->end() && (refreshString = result->second) &&
            !refreshString->empty())
        {
            uint32 secondsInterval = 0;
            StringUtility::StringToNumber(*refreshString, secondsInterval);
            if (secondsInterval)
            {
                m_tileRefresher->SetRefreshInterval(secondsInterval);
            }
        }
    }

    m_tileRefresher->AddTileToWatch(tile);

    // Update the new frame list.
    UpdateFrameList(newFrameList);
}

/* See header file for description */
void
FrameListLayer::TileRequestError(NB_Error error)
{
    // Reset the flag to identify that the request of the frame list is returned.
    m_requestInProgress = false;

    // Clear the current frame list.
    m_frameList.reset();

    // Notify the error to the listeners.
    NotifyFrameListError(error);
}

/* See header file for description */
void
FrameListLayer::RegisterListener(FrameListListener* listener)
{
    if (listener)
    {
        m_listeners.insert(listener);
    }
}

/* See header file for description */
void
FrameListLayer::UnregisterListener(FrameListListener* listener)
{
    if (listener)
    {
        m_listeners.erase(listener);
    }
}


// Private functions ............................................................................


/*! Request the latest frame list by the tile manager

    @return NE_OK if success
*/
NB_Error
FrameListLayer::RequestFrameList()
{
    if (m_requestInProgress)
    {
        // The frame list is requesting in progress.
        return NE_OK;
    }

    // Check if the tile manager is valid.
    if (!m_tileManager)
    {
        return NE_NOTINIT;
    }

    // Create the callback for the request of the frame list if it is empty.
    if (!m_tileCallback)
    {
        m_tileCallback = shared_ptr<FrameListTileCallback>(new FrameListTileCallback());
        if (!m_tileCallback)
        {
            return NE_NOMEM;
        }

        // Set the frame list layer to the callback.
        m_tileCallback->SetFrameListLayer(this);
    }

    // Set the flag to identify that the frame list is requesting in progress.
    m_requestInProgress = true;

    if (!m_frameList)
    {
        m_frameList.reset(new FrameListMap);
    }

    /* Request the frame list from the tile manager.

       @todo: Check if the priority to request the frame list is in accord with
              the current logic of the download priority.
    */
    m_tileManager->GetTile(shared_ptr<map<string, string> >(),
                           m_tileCallback,
                           m_downloadPriority);
    return NE_OK;
}

/*! Update the new frame list

    This function sets the new frame list and notifies the updated frame list to
    the listeners.

    @return None
*/
void
FrameListLayer::UpdateFrameList(FrameListMap& newFrameList)
{
    // Compare the current frame list with the new one.
    bool frameListChanged = !IsFrameListEqual(*m_frameList, newFrameList);

    // Update the frame list.
    if (frameListChanged)
    {
        m_frameList->swap(newFrameList);
        NotifyFrameListUpdated();
    }
}

/*! Check if the two shared pointers to the frame lists are equal

    @return true if the two frame lists are equal, false otherwise.
*/
bool
FrameListLayer::IsFrameListEqual(const FrameListMap& frameList1,
                                 const FrameListMap& frameList2) const
{
    if (frameList1.empty() && frameList2.empty())
    {
        return true;
    }

    /* Consider they are different when:
       1. One of the shared pointers is empty, but not both.
       2. Or two frame lists have different count of frames.
    */
    if (frameList1.size() != frameList2.size())
    {
        return false;
    }

    // Else, we need to check the FrameListEntry one by one using equal()
    FrameListEntryCompareFunctor functor;
    return equal(frameList1.begin(), frameList1.end(), frameList2.begin(), functor);
}

/*! Clone a FrameListLayer.

  This function behaves differently with Clone() of other layers.
  FrameListLayer is responsible for getting FrameList from server, and it is inefficient and
  non-flexible to have multiple copies of this layer. So in this function, it creates a
  FrameListLayerDecorator, and forward operations into the FrameListLayer created by
  UnifiedLayerProvider.
*/
UnifiedLayerPtr FrameListLayer::Clone()
{
    // Caller should make sure T is a subclass of UnifiedLayer!
    shared_ptr<UnifiedLayer> newLayer;
    uint32 layerId = LayerIdGenerator::InvalidId;
    LayerIdGeneratorPtr idGenerator =
            ContextBasedSingleton<LayerIdGenerator>::getInstance(m_pContext);
    if (idGenerator &&
        ((layerId = idGenerator->GenerateId()) != LayerIdGenerator::InvalidId))
    {
        newLayer.reset(new FrameListLayerDecorator(*this, layerId));
    }

    return newLayer;
}

/* See description in header file. */
void FrameListLayer::NotifyFrameListUpdated()
{
    // Notify the frame list updated.
    set<FrameListListener*>::iterator listenerIterator = m_listeners.begin();
    set<FrameListListener*>::const_iterator listenerEnd = m_listeners.end();
    for (; listenerIterator != listenerEnd; ++listenerIterator)
    {
        FrameListListener* listener = *listenerIterator;
        if (listener)
        {
            listener->FrameListUpdated(m_frameList);
        }
    }
}

/* See description in header file. */
void FrameListLayer::NotifyFrameListError(NB_Error error)
{
    set<FrameListListener*>::iterator listenerIterator = m_listeners.begin();
    set<FrameListListener*>::const_iterator listenerEnd = m_listeners.end();
    for (; listenerIterator != listenerEnd; ++listenerIterator)
    {
        FrameListListener* listener = *listenerIterator;
        if (listener)
        {
            listener->FrameListFailed(error);
        }
    }
}

/* See description in header file. */
void FrameListLayer::TilesExpired(const vector <TileKeyPtr>& tileKeys)
{
    if (!(tileKeys.empty()) && !m_listeners.empty())
    {
        NB_Error error = RequestFrameList();
        if (error != NE_OK)
        {
            NotifyFrameListError(error);
        }
    }
}

// Implementation of FrameListEntry
static const size_t FRAME_LIST_ENTRY_SIZE = 2;

/* See description in header file. */
shared_ptr <FrameListEntry>
FrameListEntry::FeedWithCharArray(const vector<const char*>& charArray)
{
    shared_ptr<FrameListEntry> entry;
    if (charArray.size() >= FRAME_LIST_ENTRY_SIZE)
    {
        uint32 timeStamp = 0;
        shared_ptr<string> frameId;
        shared_ptr<string> timeStampString;

        const char* ptr = charArray.at(0);
        if (ptr)
        {
            timeStamp = static_cast<uint32>(nsl_atoi(ptr));
            timeStampString.reset(new string(ptr));
        }

        ptr = charArray.at(1);
        if (ptr)
        {
            frameId.reset(new string(ptr));
        }

        //@note: Update this if more items are added to frameList.
        if (timeStamp && frameId && timeStampString)
        {
            entry.reset(new FrameListEntry);
            if (entry)
            {
                entry->m_timeStamp       = timeStamp;
                entry->m_timeStampString = timeStampString;
                entry->m_frameID         = frameId;
            }
        }
    }

    return entry;
}

/* See description in header file. */
FrameListEntry::FrameListEntry()
        : m_timeStamp(0)
{
}

/* See description in header file. */
FrameListEntry::~FrameListEntry()
{
}

/* See description in header file. */
uint32 FrameListEntry::GetTimeStamp()
{
    return m_timeStamp;
}

/* See description in header file. */
shared_ptr <string> FrameListEntry::GetTimeStampString()
{
    return m_timeStampString;
}

/* See description in header file. */
shared_ptr <string> FrameListEntry::GetFrameId()
{
    return m_frameID;
}

bool FrameListEntry::operator == (const FrameListEntry& entry) const
{
    return m_timeStamp == entry.m_timeStamp &&
            !StringUtility::CompareStdString(m_frameID, entry.m_frameID);
}

/*! @} */
