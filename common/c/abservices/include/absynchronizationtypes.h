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

    @file     absynchronizationtypes.h
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

/*! @{ */

#ifndef ABSYNCHRONIZATIONTYPES_H
#define	ABSYNCHRONIZATIONTYPES_H

#include "paltypes.h"


/*! Place message modification flags */
typedef enum
{
    AB_PLACE_FLAGS_NONE					= 0,        /*!< No modification */
    AB_PLACE_FLAGS_MODIFIED				= 1 << 0,   /*!< Modified */
    AB_PLACE_FLAGS_TS_MODIFIED			= 1 << 1,   /*!< Timestamp modified */
    AB_PLACE_FLAGS_MSG_MODIFIED			= 1 << 2    /*!< Message modified */

} AB_PlaceFlags;

typedef enum
{
    AB_PLACE_MESSAGE_FLAGS_NONE             = 0,
    AB_PLACE_MESSAGE_FLAGS_SENT             = 1 << 0,
    AB_PLACE_MESSAGE_FLAGS_READ             = 1 << 1,
    AB_PLACE_MESSAGE_FLAGS_REPLIED          = 1 << 2,
    AB_PLACE_MESSAGE_FLAGS_SEND_FAILED      = 1 << 3,
    AB_PLACE_MESSAGE_FLAGS_REPORT_ERROR     = 1 << 4,
    AB_PLACE_MESSAGE_FLAGS_INVALID_NUMBER   = 1 << 5,
    AB_PLACE_MESSAGE_FLAGS_INBOX_FULL       = 1 << 6

} AB_PlaceMessageFlags;

typedef int64 AB_PlaceID;

#define PLACE_ID_NONE	-1

#define AB_PLACE_FROM_LEN               49
#define AB_PLACE_TO_LEN                 49
#define AB_PLACE_MESSAGE_LEN            160
#define AB_PLACE_SIGNATURE_LEN          100
#define AB_PLACE_MESSAGE_BANNER_ID_LEN  10
#define AB_PLACE_MESSAGE_ID_LEN	        80

typedef struct
{
    char            from[AB_PLACE_FROM_LEN+1];
    char            from_name[AB_PLACE_FROM_LEN+1];
    char            to[AB_PLACE_TO_LEN+1];
    char            message[AB_PLACE_MESSAGE_LEN+1];
    char            signature[AB_PLACE_SIGNATURE_LEN+1];

    nb_gpsTime      stime;
    uint32          flags;

    char            banner_id[AB_PLACE_MESSAGE_BANNER_ID_LEN+1];
    char            message_id[AB_PLACE_MESSAGE_ID_LEN+1];

} AB_PlaceMessage;


#endif // ABSYNCHRONIZATIONTYPES_H


/*! @} */
