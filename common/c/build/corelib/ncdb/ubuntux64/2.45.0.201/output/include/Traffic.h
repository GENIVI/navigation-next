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

/*
 * (C) Copyright 2014 by TeleCommunication Systems, Inc.
 *
 * The information contained herein is confidential, proprietary
 * to TeleCommunication Systems, Inc., and considered a trade secret
 * as defined in section 499C of the penal code of the State of
 * California. Use of this information by anyone other than
 * authorized employees of TeleCommunication Systems is granted only
 * under a written non-disclosure agreement, expressly prescribing
 * the scope and manner of such use.
 *
 */

#ifndef TRAFFIC_H__
#define TRAFFIC_H__

#include "NcdbTypes.h"
#include "UtfString.h"

#if defined (TRAFFIC_EMULATION)

#include <stdio.h>
#include <time.h>

namespace Ncdb
{

class NCDB_API Traffic
{
public:
    Traffic(void){};
    ~Traffic(void){};

    int GetTrafficSpeedByTMC (const char* tmc_loc, double* pspeed, double* pflow = NULL, double* pconf = NULL);
    int GetHistoricSpeedByTMC (const char* tmc, time_t time, double* pspeed);
    int GetIncidentsByTMC (const char* tmc_loc);
    int GetClosedByTMC (const char* tmc_loc);

    void SetTrafficSpeed (const char* tmc, double speed, double flow);
    void SetIncident (const char* tmc);
    void SetClosed (const char* tmc);
    void ClearTraffic (const char* tmc);
    void SetHistoricSpeed (const char* tmc, double speed);

    void TranslateTmc (bool enable);

    void ResetTrafficData();
    void EmptyTrafficBuffers();
    bool SetTrafficData(const char* provider, bool useRealTime = true,
        bool useHistoric = true, bool useClosures = true);
    bool SetTrafficData(const Ncdb::UtfString & provider, bool useRealTime = true,
        bool useHistoric = true, bool useClosures = true);

    bool AddHistoricData(const char* histFile);
    bool AddHistoricData(const Ncdb::UtfString & histFile);

    bool SetClosureData(const char* closureFile);
    bool SetClosureData(const Ncdb::UtfString & closureFile);

    bool GetTrafficState (Ncdb::UtfString * provider = NULL, bool * useRealTime = NULL,
        bool * useHistoric = NULL, bool * useClosures = NULL, bool * transTmc = NULL) const;
    void UpdateTrafficData();
};

}

#endif
#endif

