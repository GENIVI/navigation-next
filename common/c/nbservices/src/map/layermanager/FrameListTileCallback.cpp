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

    @file       FrameListTileCallback.cpp

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

#include "FrameListTileCallback.h"
#include "FrameListLayer.h"

/*! @{ */

using namespace nbmap;
using namespace std;

// Public functions .............................................................................

/* See header file for description */
FrameListTileCallback::FrameListTileCallback()
: AsyncCallbackWithRequest<shared_ptr<map<string, string> >, TilePtr>(),
  m_frameListLayer(NULL)
{
    // Nothing to do here.
}

/* See header file for description */
FrameListTileCallback::~FrameListTileCallback()
{
    // Nothing to do here.
}

/* See description in AsyncCallback.h */
void
FrameListTileCallback::Success(shared_ptr<map<string, string> > templateParameters,
                               TilePtr tile)
{
    if (m_frameListLayer)
    {
        // Notify that the frame list is updated successfully.
        m_frameListLayer->TileRequestSuccess(tile);
    }
}

/* See description in AsyncCallback.h */
void
FrameListTileCallback::Error(shared_ptr<map<string, string> > templateParameters,
                             NB_Error error)
{
    if (m_frameListLayer)
    {
        // Notify updating the frame list failed.
        m_frameListLayer->TileRequestError(error);
    }
}

/* See description in AsyncCallback.h */
bool
FrameListTileCallback::Progress(int percentage)
{
    // Nothing to do here.
    return false;
}

/* See header file for description */
void
FrameListTileCallback::SetFrameListLayer(FrameListLayer* frameListLayer)
{
    m_frameListLayer = frameListLayer;
}

/*! @} */
