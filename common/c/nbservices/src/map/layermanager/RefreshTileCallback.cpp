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

    @file       RefreshTileCallback.cpp

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

#include "RefreshTileCallback.h"
#include "RefreshLayer.h"

/*! @{ */

using namespace std;
using namespace nbmap;

// Public functions .............................................................................

/* See header file for description */
RefreshTileCallback::RefreshTileCallback(AsyncTileRequestWithRequestPtr callback,
                                         shared_ptr<bool> layerValid,
                                         RefreshLayer* layer)
: AsyncCallbackWithRequest<TileKeyPtr, TilePtr>(),
  m_callback(callback),
  m_layerValid(layerValid),
  m_layer(layer)
{
    // Nothing to do here.
}

/* See header file for description */
RefreshTileCallback::~RefreshTileCallback()
{
    // Nothing to do here.
}

/* See description in AsyncCallback.h */
void
RefreshTileCallback::Success(TileKeyPtr tileKey,
                             TilePtr tile)
{
    // Check if the layer is valid.
    if (m_layerValid && m_layer)
    {
        NB_Error error = m_layer->AddValidTile(tile);
        if (error != NE_OK)
        {
            Error(tileKey, error);
            return;
        }
    }

    if (m_callback)
    {
        m_callback->Success(tileKey, tile);
    }
}

/* See description in AsyncCallback.h */
void
RefreshTileCallback::Error(TileKeyPtr tileKey,
                           NB_Error error)
{
    if (m_callback)
    {
        m_callback->Error(tileKey, error);
    }
}

/* See description in AsyncCallback.h */
bool
RefreshTileCallback::Progress(int percentage)
{
    if (m_callback)
    {
        return m_callback->Progress(percentage);
    }

    return false;
}

/*! @} */
