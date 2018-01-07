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
   @file         FrameListLayerDecorator.h
   @defgroup     nbmap

   Description:  FrameListLayerDecorator is a decorator of FrameListLayer.

   FrameList is returned from server and can be shared by multiple
   FrameListLayerDecorators. This class inherits from FrameListLayer and
   overwrite the public functions to forward all operations of FrameList to
   the FrameListLayer created by OffboardLayerProvider. This will ensure all
   decorators of FrameListLayer have the same frame list.
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

#ifndef _FRAMELISTLAYERDECORATOR_H_
#define _FRAMELISTLAYERDECORATOR_H_

#include "FrameListLayer.h"
#include "FrameListListener.h"

namespace nbmap
{
class FrameListLayerDecorator : public FrameListLayer,
                                public FrameListListener
{
public:
    FrameListLayerDecorator(FrameListLayer& instance, uint32 layerId);
    virtual ~FrameListLayerDecorator();

    // Refer FrameListListener for description.
    virtual void FrameListUpdated(FrameListMapPtr frameList);
    virtual void FrameListFailed(NB_Error error);

    // Refer UnifiedLayer for description.
    virtual UnifiedLayerPtr Clone();
    virtual void PostLayerToggled();

    // Refer FrameListLayer for description.
    virtual NB_Error GetFrameList(FrameListMapPtr& frameList);

    virtual std::string className() const { return "FrameListLayerDecorator"; }

private:
    FrameListLayer& m_instance; /*!< Reference of instance which will retrieve FR from
                                  server. */
};
}

#endif /* _FRAMELISTLAYERDECORATOR_H_ */

/*! @} */
