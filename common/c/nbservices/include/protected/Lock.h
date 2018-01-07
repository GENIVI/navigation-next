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

	@file     Lock.h
	@date     07/29/2010

    Simple wrapper class to encapsulate PAL lock. The constructor gets the
    lock, the destructor releases the lock. This ensures that the lock
    is always released in every code path (since the desctructor is 
    always called).
*/
/*
	(C) Copyright 2010 by TeleCommunication Systems                

	The information contained herein is confidential, proprietary 
	to Networks In Motion, Inc., and considered a trade secret as 
	defined in section 499C of the penal code of the State of     
	California. Use of this information by anyone other than      
	authorized employees of Networks In Motion is granted only    
	under a written non-disclosure agreement, expressly           
	prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

/*! @{ */

#ifndef _NB_LOCK_H_
#define _NB_LOCK_H_

#include "pallock.h"

namespace nb 
{

class Lock
{
public:

	/*! The contructor aquires the lock. 
     
        The lock has to be initialized.
	*/
    Lock(PAL_Lock* lock) : m_lock(lock)
    {
        PAL_LockLock(lock);
    };

	/*! The destructor releases the lock. 
	*/
    ~Lock()
    {
        PAL_LockUnlock(m_lock);
    };
	
private:
	PAL_Lock* m_lock;
};

}
    
#endif // _NB_LOCK_H_

/*! @} */





