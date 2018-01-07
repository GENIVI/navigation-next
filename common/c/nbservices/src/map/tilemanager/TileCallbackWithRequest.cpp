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

    @file       TileCallbackWithRequest.cpp

    See header file for description.
*/
/*
    (C) Copyright 2011 by TeleCommunications Systems, Inc.

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
#include "nbqalog.h"
#include "palclock.h"
#include "cslqarecorddefinitions.h"
#include "nbcontextprotected.h"
}

#include "TileCallbackWithRequest.h"
#include "TileImpl.h"
#include "StringUtility.h"
#include <algorithm>

/*! @{ */

using namespace std;
using namespace nbmap;

// Local Constants ..............................................................................

class TileRequestErrorNotifier
{
public:
    TileRequestErrorNotifier(NB_Context* context, NB_Error error)
            : m_pContext(context),
              m_error(error)
    {
    }

    virtual ~TileRequestErrorNotifier()
    {
    }

    void operator() (const TileRequestPtr& request) const
    {
        if (request)
        {
            TileCallbackWithRequest::WriteQaLogPerformance(m_pContext, request);

            if (request->m_callback)
            {
                request->m_callback->Error(request->m_requestedTileKey, m_error);
            }
        }
    }

private:
    NB_Context* m_pContext;
    NB_Error m_error;
};

class TileRequestSuccessNotifier
{
public:
    TileRequestSuccessNotifier(NB_Context* context, TilePtr tile)
            : m_pContext(context),
              m_pTile(tile)
    {
    }

    virtual ~TileRequestSuccessNotifier()
    {
    }

    void operator() (const TileRequestPtr& request) const
    {
        if (request)
        {
            TileCallbackWithRequest::WriteQaLogPerformance(m_pContext, request);

            if (request->m_callback)
            {
                request->m_callback->Success(request->m_requestedTileKey, m_pTile);
            }
        }
    }

private:
    NB_Context* m_pContext;
    TilePtr     m_pTile;
};

// Public functions .............................................................................

/* See header file for description */
TileCallbackWithRequest::TileCallbackWithRequest(NB_Context* context)
        : m_pContext(context),
          m_enabled(true)
{
    // Nothing to do here.
}

/* See header file for description */
TileCallbackWithRequest::~TileCallbackWithRequest()
{
    // Nothing to do here.
}

/* See description in AsyncCallback.h */
void
TileCallbackWithRequest::Success(shared_ptr<map<string, string> > templateParameters,
                                 TilePtr tile)
{
    TileKeyPtr dataTileKey = ParametersToTileKey(templateParameters);
    if (!dataTileKey)
    {
        return;
    }

    if (tile)
    {
        TileImpl* tileImpl = static_cast<TileImpl*>(tile.get());
        if (tileImpl && dataTileKey)
        {
            tileImpl->SetTileKey(dataTileKey);
        }
    }

    // QaLog: TileReceived.
    if (CSL_QaLogIsVerboseLoggingEnabled(NB_ContextGetQaLog(m_pContext)))
    {
        NB_QaLogTileReceived(m_pContext, NB_RECEIVE_TILE_INITIAL, dataTileKey->m_x,
                         dataTileKey->m_y, dataTileKey->m_zoomLevel);
    }

    NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelVerbose,"TileCallbackWithRequest::Success: X: %d Y: %d Z: %d",dataTileKey->m_x, dataTileKey->m_y, dataTileKey->m_zoomLevel);

    map<TileKey, vector<TileRequestPtr> >::iterator iter = m_stashedRequests.find(*dataTileKey);
    if (iter != m_stashedRequests.end())
    {
        vector<TileRequestPtr> requests;
        iter->second.swap(requests);
        m_stashedRequests.erase(*dataTileKey);

        if (m_enabled)
        {
            TileRequestSuccessNotifier notifier(m_pContext, tile);
            for_each (requests.begin(), requests.end(), notifier);
        }
    }

}

/* See description in AsyncCallback.h */
void
TileCallbackWithRequest::Error(shared_ptr<map<string, string> > templateParameters,
                               NB_Error error)
{
    TileKeyPtr dataTileKey = ParametersToTileKey(templateParameters);
    if (!dataTileKey)
    {
        return;
    }

    NotifyErrorForTileKey(dataTileKey, error);
}

/* See description in AsyncCallback.h */
bool
TileCallbackWithRequest::Progress(int percentage)
{
    return true;
}

/*! Writes Performance QaLog.

    @todo: It may be more helpful to log TileDataType here.

    @return void
*/
void TileCallbackWithRequest::WriteQaLogPerformance(const NB_Context* context,
                                                    const TileRequestPtr& request)
{
    // Check if QA logging is enabled
    if (request && context && (NB_ContextGetQaLog(context)) && (CSL_QaLogIsVerboseLoggingEnabled(NB_ContextGetQaLog(context))))
    {
        uint32 duration = PAL_ClockGetTimeMs() - request->m_startTimeStamp;
        char msg[PERFORMANCE_MSG_LENGTH] = {0};
        nsl_sprintf(msg, "Download Tile(%d, %d, %d) finished.",
                    request->m_requestedTileKey->m_x, request->m_requestedTileKey->m_y,
                    request->m_requestedTileKey->m_zoomLevel);
        NB_QaLogPerformance(context, msg, duration);
    }
}

/* See description in header file. */
TileRequest::TileRequest(TileKeyPtr requestedTileKey,
                       TileKeyPtr dataTileKey,
                       shared_ptr <AsyncCallbackWithRequest<TileKeyPtr,TilePtr> > callback)
        : m_requestedTileKey(requestedTileKey),
          m_dataTileKey(dataTileKey),
          m_callback(callback),
          m_startTimeStamp(PAL_ClockGetTimeMs())
{
}

/* See description in header file. */
TileRequest::~TileRequest()
{
}

/* See description in header file. */
void TileCallbackWithRequest::StoreTileRequest(TileRequestPtr request)
{
    if (request && request->m_dataTileKey)
    {
        map<TileKey, vector<TileRequestPtr> >::iterator iter =
                m_stashedRequests.insert(m_stashedRequests.begin(),
                                         make_pair(*(request->m_dataTileKey),
                                                   vector<TileRequestPtr>()));
        if (iter != m_stashedRequests.end())
        {
            iter->second.push_back(request);
        }
    }
}


/* See description in header file. */
TileKeyPtr
TileCallbackWithRequest::ParametersToTileKey(shared_ptr<map <string,string> > parameters)
{
    TileKeyPtr key;
    if (parameters)
    {
        int x = 0;
        int y = 0;
        int z = 0;

        map<string, string>::iterator iter = parameters->find(PARAMETER_KEY_TILE_X);
        map<string, string>::iterator end  = parameters->end();
        if (iter != end)
        {
            nbcommon::StringUtility::StringToNumber(iter->second, x);
        }
        iter = parameters->find(PARAMETER_KEY_TILE_Y);
        if (iter != end)
        {
            nbcommon::StringUtility::StringToNumber(iter->second, y);
        }
        iter = parameters->find(PARAMETER_KEY_ZOOM_LEVEL);
        if (iter != end)
        {
            nbcommon::StringUtility::StringToNumber(iter->second, z);
        }

        key.reset(new TileKey(x, y, z));
    }
    return key;
}

/* See description in header file. */
void TileCallbackWithRequest::NotifyErrorForTileKey(const TileKeyPtr& tileKey, NB_Error error)
{
    // Some requests may be ignored by HttpDownloadManager because the same url is under
    // processing. Do not report error in this case, because the request will finally be
    // processed.
    if (error == NE_IGNORED)
    {
        return;
    }

    map<TileKey, vector<TileRequestPtr> >::iterator iter = m_stashedRequests.find(*tileKey);
    if (iter != m_stashedRequests.end())
    {
        vector<TileRequestPtr> requests;
        iter->second.swap(requests);
        m_stashedRequests.erase(*tileKey);

        if (m_enabled)
        {
            TileRequestErrorNotifier notifier(m_pContext, error);
            for_each (requests.begin(), requests.end(), notifier);
        }
    }
}

/* See description in header file. */
void TileCallbackWithRequest::SetEnabled(bool enabled)
{
    m_enabled = enabled;
}

/*! @} */
