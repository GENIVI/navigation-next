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

    @file     abserverversionparameters.h
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

#ifndef ABSERVERVERSIONPARAMETERS_H
#define ABSERVERVERSIONPARAMETERS_H


#include "abexp.h"


/*!
    @addtogroup abserverversionparameters
    @{
*/


/*! @struct AB_ServerVersionParameters
    Opaque data structure that defines the parameters of a ServerVersion operation
*/
typedef struct AB_ServerVersionParameters AB_ServerVersionParameters;


/*! Server version query types */
typedef enum
{
    ASVQT_VersionDetailed = 0,          /*!< Detailed server version information */
} AB_ServerVersionQueryType;



/*! Create parameters for a server version query request

@param context context instance
@param queryType Desired AB_ServerVersionQueryType
@param The target of the version query
@param parameters On success, a newly created AB_ServerVersionParameters object; NULL otherwise.  A valid object must be destroyed using AB_ServerVersionParametersDestroy().
@returns NB_Error
*/
AB_DEC NB_Error AB_ServerVersionParametersCreate(NB_Context* context, AB_ServerVersionQueryType queryType, const char* target, AB_ServerVersionParameters** parameters);


/*! Destroy a previously created ServerVersionParameters object

@param parameters An AB_ServerVersionParameters object created with AB_ServerVersionParametersCreate()
@returns NB_Error
*/
AB_DEC NB_Error AB_ServerVersionParametersDestroy(AB_ServerVersionParameters* parameters);


/*! @} */

#endif
