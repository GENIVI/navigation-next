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
  @file        TemplateInstantiation.cpp
  @defgroup    nbmap

  Description: Explicitly instantiation of used templates.
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

#include "TemplatePinLayer.cpp"
#include "TemplatePinImpl.cpp"
#include "TemplatePinParameters.cpp"
#include "BubbleInterface.h"
#include <string>
#include <vector>


using namespace nbmap;

template class PinImpl<void>;
template class PinImpl<BubbleInterface>;

template class PinParameters<void>;
template class PinParameters<BubbleInterface>;

template void
PinLayer::AddPins<BubbleInterface>(std::vector<PinParameters<BubbleInterface> >&);
template shared_ptr<BubbleInterface>
PinLayer::GetBubble<BubbleInterface>(shared_ptr<string>);

template void
PinLayer::AddPins<void>(std::vector<PinParameters<void> >&);
template shared_ptr<void>
PinLayer::GetBubble<void>(shared_ptr<string>);

/*! @} */
