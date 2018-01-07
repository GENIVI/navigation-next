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

@file     finishfunctionmanager.h
*/
/*
(C) Copyright 2014 by TeleCommunication Systems, Inc.                

The information contained herein is confidential, proprietary 
to TeleCommunication Systems, Inc., and considered a trade secret as 
defined in section 499C of the penal code of the State of     
California. Use of this information by anyone other than      
authorized employees of TeleCommunication Systems is granted only    
under a written non-disclosure agreement, expressly           
prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/


#ifndef FINISHFUNCTIONMANAGER_H
#define FINISHFUNCTIONMANAGER_H

#include "finishfunctionbase.h"
#include "paltaskqueue.h"
#include <list>
#include "pallock.h"

/*! @{ */

class FinishFunctionManager
{
public:
    struct ListEntry
    {
        uint32                 ownerId;
        FinishFunctionBase*    function;
        FinishFunctionManager* manager;
        TaskId                 taskId;
    };

    FinishFunctionManager(PAL_Instance* pal);
    ~FinishFunctionManager();


    /*! Register a new message owner
    @return Owner identifier
    */
    uint32 RegisterOwner();


    /*! Schedule the execution of a finish function
    @param owner The owner id previously returned by RegisterOwner()
    @param function The finish function object to schedule
    @return PAL_Error
    */
    PAL_Error Schedule(uint32 owner, FinishFunctionBase* function);


    /*! Delete all scheduled funtions with a specific owner
    @param owner The owner id of the functions to delete
    */
    PAL_Error DeleteScheduledFunctions(uint32 owner);


    /*! Process callback in UI thread.
    @param entry Pointer to list entry
    */
    void ProcessCallback(ListEntry* entry);

private:
    void DeleteAllListEntries();

    PAL_Instance*           m_pal;
    uint32                  m_ownerId;
    std::list<ListEntry*>   m_list;
    PAL_Lock*				m_lock;
};


/*! @} */


#endif
