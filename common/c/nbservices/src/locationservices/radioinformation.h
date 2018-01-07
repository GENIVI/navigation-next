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

    @file     radioinformation.h
    @date     07/07/2011
    @defgroup LOCATIONSERVICES_LOCATIONREQUEST_GROUP

*/
/*
    radioInformation class declaration
        This class retrieves radio network data(cell ID and wifi).

    (C) Copyright 2011 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

#pragma once

#include "base.h"

extern "C" {
  #include "nberror.h"
  #include "palradio.h"
  #include "vec.h"
}

class RadioInformation;
class CellIDInformation;

typedef void (*RadioInformationReceivedCb)(PAL_Error status, RadioInformation* information, void* m_userData);

/* TODO:
   For future releases - add Wifi support.
*/
class RadioInformation : public Base
{
public:
    RadioInformation(PAL_RadioContext* radioContext, PAL_Instance* pal);
    ~RadioInformation(void);

    PAL_Error RequestInformation(void);
    void GetCellIdInformation(PAL_RadioSystemMode& sysMode, PAL_RadioCDMAInfo& cdmaInfo, PAL_RadioGSMInfo& gsmInfo) const;
    void SetListener(RadioInformationReceivedCb listener, void* userData);

private:
    static void Static_CellId_ReceivedCallback(PAL_Error status, CellIDInformation* information, void* userData);
    void CellIdReceived(PAL_Error status, CellIDInformation* information);

    CSL_Vector*                  m_listeners;

    CellIDInformation*           m_cellIdInfo;

    bool     m_cellIdInProgress;
};
/*! @} */
