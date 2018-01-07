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

    @file palevent.h
    @date 10-17-10
    @defgroup palevent.h in nbpal

	The PAL event is used to synchronize multiple threads in the application.
	A thread can wait on an event which will block the thread until it is fired.
	The event can then be fired from another thread.
	
	The functionality is modeled after Windows system EVENT and WaitForSingleObject()
	API.
*/
/*
    (C) Copyright 2010 by TeleCommunication Systems

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret
    as defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly prescribing
    the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

#ifndef PALEVENT_H
#define PALEVENT_H

#include "pal.h"
#include "palexp.h"

typedef struct PAL_Event PAL_Event;

/*! Create and initialize a new PAL_Event object.

    @return PAL_Error.
*/
PAL_DEC PAL_Error
PAL_EventCreate(PAL_Instance* pal,			/*!< The pal instance. */
                PAL_Event** palevent		/*!< The PAL_Event object created upon success. This object must be destroied by PAL_EventDestroy. */
                );

/*! Destroy a previously created PAL_Event object.
 
    @return PAL_Error.
*/
PAL_DEC PAL_Error
PAL_EventDestroy(PAL_Event* palevent		/*!< A PAL_Event object created by PAL_EventCreate. */
                 );
				 
/*! Wait for an event.

	This function will block the thread until the thread is set/fired (or an error occurs).

    @return PAL_Error.
*/
PAL_DEC PAL_Error
PAL_EventWaitForEvent(PAL_Event* palevent	/*!< A PAL_Event object created by PAL_EventCreate. */
					  );
				 
/*! Set/Fire event.

	When an event is set/fired then any thread that waits on the event will unblock and continue operation.
	An event will automatically reset once a blocking thread received the event (and can therefor be waited on again).

    @return PAL_Error.
*/
PAL_DEC PAL_Error
PAL_EventSet(PAL_Event* palevent			/*!< A PAL_Event object created by PAL_EventCreate. */
			 );


#endif //PALEVENT_H

/*! @} */
