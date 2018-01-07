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

    @file     cellinfolistener.h
    @date     4/28/2010
    @defgroup PAL_RADIO PAL Radio Functions

    @brief    Classes for PAL Cell information listeners.

    Windows implementation for PAL Cell information listeners.
*/
/*
(C) Copyright 2010 by TeleCommunication Systems, Inc.

The information contained herein is confidential, proprietary 
to TeleCommunication Systems, Inc., and considered a trade secret as 
defined in section 499C of the penal code of the State of     
California. Use of this information by anyone other than      
authorized employees of TeleCommunication Systems is granted only    
under a written non-disclosure agreement, expressly           
prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

#ifndef CELLINFOLISTENER_H
#define CELLINFOLISTENER_H

#include "riltypes.h"
#include "palradio.h"
#include "palradiolistener.h"

namespace nimpal
{
    namespace radio
    {
        /*! Base class for Cell information listeners */
        class CellInfoListener : public PalRadioListener
        {
        public:
            CellInfoListener(void* userData);
            virtual ~CellInfoListener() {}
            /*! This function calls user callback. Abstract prototype. */
            virtual void Callback(PAL_Error status, const RILCELLTOWERINFO* cellInfo) = 0;

        protected:
            void* m_userData;
            PAL_Error m_status;
        };


        /*! Class for GSM Cell information listener */
        class GSMCellInfoListener : public CellInfoListener
        {
        public:
            GSMCellInfoListener(PAL_RadioGSMNetworkInfoCallback userCallback, void* userData);
            virtual ~GSMCellInfoListener() {}
            /*! This function schedules calling user callback from the PAL thread. */
            virtual void Callback(PAL_Error status, const RILCELLTOWERINFO* cellInfo);

        protected:
            /*! PAL thread caller. User callback actually is called here. */
            virtual void PalThreadCallerFunc();

        private:
            PAL_RadioGSMNetworkInfoCallback m_userCallback;
            PAL_RadioGSMInfo m_gsmInfo;
        };

        /*! Class for CDMA Cell information listener */
        class CDMACellInfoListener : public CellInfoListener
        {
        public:
            CDMACellInfoListener(PAL_RadioCDMANetworkInfoCallback userCallback, void* userData);
            virtual ~CDMACellInfoListener() {}
            /*! This function schedules calling user callback from the PAL thread. */
            virtual void Callback(PAL_Error status, const RILCELLTOWERINFO* cellInfo);

        protected:
            /*! PAL thread caller. User callback actually is called here. */
            virtual void PalThreadCallerFunc();

        private:
            PAL_RadioCDMANetworkInfoCallback m_userCallback;
            PAL_RadioCDMAInfo m_cdmaInfo;
        };
    }
}

#endif
