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

    @file     commoncomponentconfiguration.h
*/
/*
    (C) Copyright 2011 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
#ifndef COMMONCOMPONENTCONFIGURATION_H
#define COMMONCOMPONENTCONFIGURATION_H

//@todo: Integrate #include from Hybrid Branch
#include "workerqueuemanager.h"
#include "ncdbobjectsmanager.h"

#include "contextbasedsingleton.h"


using namespace nbcommon;

/*
* Returns AutoSharedPtrs to various services. Encapsulates the knowledge of ContextBasedSingleton vs Prototype.
*/
class CommonComponentConfiguration
{
public:
    static shared_ptr<WorkerQueueManager> GetWorkerQueueManagerComponent(NB_Context* context)
    {
        //@todo: Integrate #include from Hybrid Branch
        return  ContextBasedSingleton<WorkerQueueManager>::getInstance(context);
    }

    static NB_Error GetCommonNBMTile(NB_Context* context, shared_ptr<Ncdb::NBMTile>& nbmTile)
    {
        return ContextBasedSingleton<nbcommon::NcdbObjectsManager>::getInstance(context)->GetNBMTile(nbmTile);
    }

private:
    // this class cannot be instantiated
    CommonComponentConfiguration(){}
    virtual ~CommonComponentConfiguration()
    {
    }
};

#endif   // COMMONCOMPONENTCONFIGURATION_H
