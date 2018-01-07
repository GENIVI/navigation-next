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
@file         NKUIState.h
@defgroup     nkui
*/
/*
(C) Copyright 2014 by TeleCommunications Systems, Inc.

The information contained herein is confidential, proprietary to
TeleCommunication Systems, Inc., and considered a trade secret as defined
in section 499C of the penal code of the State of California. Use of this
information by anyone other than authorized employees of TeleCommunication
Systems is granted only under a written non-disclosure agreement, expressly
prescribing the scope and manner of such use.
--------------------------------------------------------------------------*/

/*! @{ */

#ifndef __NKUIState__
#define __NKUIState__

#include "NKUIStateID.h"
#include "WidgetID.h"
#include "nkui_macros.h"
#include "NKUIContext.h"
#include "NKUIEvent.h"
#include <list>

namespace nkui
{
class NKUIContext;

/** Basic class to represent NKUI state.
 *
 *   All states should be derived from this class.
 **/
class NKUIState
{
public:
    NKUIState(NKUIContext* pContext,
              NKUIState* formerState,
              NKUIStateID stateID,
              const MapSetting& mapSetting);
    virtual ~NKUIState();

    enum LocationFeedType
    {
        LFT_None = 0,                   // No one wants location
        LFT_Map  = 1,                   // map wants location
        LFT_Nav  = 1 << 1,              // nav wants location
    };


    /** implement NKUIStateInterfaces
        see @NKUIStateInterfaces for details
    */
    virtual NKUIStateID GetStateID() const
    {
        return m_stateID;
    }

    virtual NKUIState* GetFormerState() const
    {
        return m_pFormerState;
    }

    virtual NKUIState* DoTransition(NKEventID event);
    virtual std::list<WidgetID> GetActiveWidget();
    virtual const MapSetting& GetMapSetting();

    /**
     *  this function can retun true if can update location for navkit. at request route and
     *  rts state should not update location
     *
     *  @return ture update location for navkit false not
     */
    virtual uint32 GetLocationFeedType()
    {
        return m_locationFeedType;
    }

    virtual bool WantAlternativeRoute()
    {
        return m_wantAlternativeRoute;
    }

protected:
    NKUIContext*        m_pNKUIContext;
    std::list<WidgetID> m_widgetList;
    MapSetting          m_mapSetting; // this field should only be modified in constructor!
    NKUIState*          m_pFormerState;
    NKUIStateID         m_stateID;
    bool                m_wantAlternativeRoute;
    uint32              m_locationFeedType;
};
}

#endif /* defined(__NKUIState__) */


/*! @} */
