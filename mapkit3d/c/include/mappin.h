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
 @file     mappin.h
 */
/*
 (C) Copyright 2013 by TeleCommunication Systems, Inc.

 The information contained herein is confidential, proprietary
 to TeleCommunication Systems, Inc., and considered a trade secret as
 defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of TeleCommunication Systems, is granted only
 under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.

 ---------------------------------------------------------------------------*/
/*! @{ */

#ifndef MAPPIN_H_
#define MAPPIN_H_

#include "smartpointer.h"
#include "mapcontroller.h"

namespace nbmap
{

/*! a simple wrapper class to control pins
 */
class MapPin
{
public:
    /**
    * @name SetVisible - show/hide this pin object.
    * @param visible - true:visible, false: invisible
    * @return None.
    */
    void SetVisible(bool visible);

    /**
    * @name SetSelected - select this pin object.
    * @param selected - set true to select this pin.
    * @return None.
    */
    void SetSelected(bool selected);

    /**
     * @name GetSelected - Indicate if the pin is selected.
     * @return true means selected, false means unselected.
     */
    bool GetSelected();

protected:
    MapPin(PAL_Instance& pal, MapController& controller);
    virtual ~MapPin();

private:
    static void SelectPinFunc(PAL_Instance* pal, void* userData);

private:
    shared_ptr<string> m_id;
    PAL_Instance &m_pal;
    MapController &m_mapController;
    bool m_selected;

friend class MapController;

};

}

#endif /* MAPPIN_H_ */
