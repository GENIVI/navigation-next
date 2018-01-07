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

    @file       TileCallbackWithRequest.h

    Class TileCallbackWithRequest inherits from AsyncCallbackWithRequest
    interface. A TileCallbackWithRequest object is a callback of requesting
    a tile by the tile manager. It returns a TileKey object (contained x,
    y and zoom level) and a Tile object (contained content ID and data)
    to user if the request succeeds.
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

#ifndef TILECALLBACKWITHREQUEST_H
#define TILECALLBACKWITHREQUEST_H

/*!
    @addtogroup nbmap
    @{
*/


extern "C"
{
#include "nbcontext.h"
}

#include "AsyncCallback.h"
#include "TileInternal.h"
#include "TileKey.h"
#include <string>
#include <map>

namespace nbmap
{

// Constants ....................................................................................

// Types ...................................................................................

class TileRequest
{
public:
    TileRequest(TileKeyPtr requestedTileKey,
               TileKeyPtr dataTileKey,
               shared_ptr <AsyncCallbackWithRequest <TileKeyPtr,TilePtr> > callback);

    virtual ~TileRequest();

    TileKeyPtr m_requestedTileKey;
    TileKeyPtr m_dataTileKey;
    shared_ptr <AsyncCallbackWithRequest <TileKeyPtr,TilePtr> > m_callback;
    uint32 m_startTimeStamp;
};

typedef shared_ptr<TileRequest> TileRequestPtr;


/*! A TileCallbackWithRequest object is a callback of requesting a tile by the tile manager */
class TileCallbackWithRequest
        : public AsyncCallbackWithRequest<shared_ptr<map<string, string> >, TilePtr>,
          public Base
{
public:
    // Public functions .........................................................................

    /*! TileCallbackWithRequest constructor */
    TileCallbackWithRequest(NB_Context* context  /*!< NB_Context instance */
                            );

    /*! TileCallbackWithRequest destructor */
    virtual ~TileCallbackWithRequest();

    /* See description in AsyncCallback.h */
    virtual void Success(shared_ptr<map<string, string> > templateParameters,
                         TilePtr tile);
    virtual void Error(shared_ptr<map<string, string> > templateParameters,
                       NB_Error error);
    virtual bool Progress(int percentage);

    /*! Store a tile request into this callback.

      Layer from DTS server is context-based singleton, this means it is possible that multiple
      MapView instances requesting the same tile. This function stores these requests, and
      requests will get notified after tile returns.

      @return None.
    */
    void StoreTileRequest(TileRequestPtr request);

    /*! Write QaLog for record performance.

      @return None.
    */
    static void WriteQaLogPerformance(const NB_Context* context,
                                      const TileRequestPtr& request);

    /*! Enable or disable this callback.

      @return void
    */
    void SetEnabled(bool enabled);

protected:

    /*! Helper function to notify error to all requests for specified TileKey.

      @return None
    */
    void NotifyErrorForTileKey(const TileKeyPtr& tileKey, NB_Error error);

    /*! Helper function to convert parameters into TileKey.

      @return Shared pointer to converted TileKey.
    */
    TileKeyPtr ParametersToTileKey(shared_ptr<map<string, string> > parameters);

    NB_Context* m_pContext;       /*!< NB_Context instance */

private:
    // Private functions ........................................................................

    // Copy constructor and assignment operator are not supported.
    TileCallbackWithRequest(const TileCallbackWithRequest& callback);
    TileCallbackWithRequest& operator=(const TileCallbackWithRequest& callback);

    // Private members .....................................................................
    map<TileKey, vector<TileRequestPtr> > m_stashedRequests; /*!<  Stashed Requests  */
    bool m_enabled;
};

};  // namespace nbmap

/*! @} */

#endif  // TILECALLBACKWITHREQUEST_H
