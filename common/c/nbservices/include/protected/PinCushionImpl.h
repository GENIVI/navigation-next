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
@file     PinCushionImpl.h
@defgroup nbmap

*/
/*
(C) Copyright 2011 by TeleCommunication Systems, Inc.

The information contained heren is confidential, proprietary
to TeleCommunication Systems, Inc., and considered a trade secret as
defined in section 499C of the penal code of the State of
California. Use of this information by anyone other than
authorized employees of TeleCommunication Systems, is granted only
under a written non-disclosure agreement, expressly
prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
/*! @{ */

#ifndef __PIN_CUSHION_IMPL__
#define __PIN_CUSHION_IMPL__

#include "base.h"
#include "PinCushion.h"
#include "PinManager.h"
#include "nbcontext.h"
#include "PinBubbleResolver.h"
#include "Pin.h"
#include "PinLayer.h"

#define DEFAULT_LAYER_NAME "default"

namespace nbmap
{

/*! Provides a definition of a Pin within mapview. Allows a client of MapView to get Pin properties and
    interact with the pin.
*/

class PinCushionImpl : public Base,
                       public PinCushion
{
public:

    /*! PinCushionImpl constructor */
    PinCushionImpl(shared_ptr<PinManager> pinManagerPtr,
                   shared_ptr<PinBubbleResolver> pinBubbleResolver,
                   NB_Context* context);

    /*! PinCushionImpl destructor */
    virtual ~PinCushionImpl();

    /* See description in PinCushion.h */
    virtual shared_ptr<PinBubbleResolver> GetPinBubbleResolver();
    virtual NB_Error SetCustomPinBubbleResolver(shared_ptr<PinBubbleResolver> bubbleResolver);
    virtual shared_ptr<std::string> DropPin(double latitude, double longitude);
    virtual NB_Error RemovePin(shared_ptr<std::string> pinID);

private:

    // Copy constructor and assignment operator are not supported.
    PinCushionImpl(const PinCushionImpl& pin);
    PinCushionImpl& operator=(const PinCushionImpl& pin);

    bool                    m_IsDefaultLayerCreated;
    NB_Context*             m_pContext;
    shared_ptr<PinLayer>    m_pPinLayer;
    shared_ptr<PinManager>  m_pPinManager;
    shared_ptr<std::string> m_pCurrentPinID;
    shared_ptr<std::string> m_pLayerID;
    shared_ptr<PinBubbleResolver> m_pPinBubbleResolver;
};

}

#endif
/*! @} */
