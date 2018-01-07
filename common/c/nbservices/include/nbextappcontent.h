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

   @file     nbextappcontent.h
*/
/*
    (C) Copyright 2012 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems, Inc. is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#ifndef NBEXTAPPCONTENT_H
#define NBEXTAPPCONTENT_H

#include "nberror.h"
#include "paltypes.h"


/*! @{ */

/*! Enum to categorize the kind of 3rd party applications
*/

typedef enum
{
    NB_TPA_None = 0,                    /*! None */
    NB_TPA_Aisle411 = (1 << 0),         /*! aisle411 3rd paty app providing indoor map content */
    NB_TPA_HopStop = (1 << 1),          /*! HopStop 3rd party app providing public transit */
    NB_TPA_Fandango = (1 << 2),         /*! Fandango 3rd party app providing movie ticket purchasing options */
    NB_TPA_OpenTable = (1 << 3)         /*! OpenTable 3rd party app providing online reservation at different restaurant */

} NB_ThirdPartyApp;


/*! Describes content to be used for launch aisle411
*/
typedef struct
{
    int     retailerStoreID;            /*! Store id to be used for aisle411 launch */

} NB_Aisle411Content;


// TODO: other 3rd party apps content to be defined...

/*! Describes ext apps content
*/
typedef struct
{
    NB_ThirdPartyApp      tpa;               /*! Flags to indicate what 3rd party app content is available */

    NB_Aisle411Content    aisle411Content;   /*! aisle411 content */

    // TODO: extend with other 3rd party apps content

} NB_ExtAppContent;


/*! @} */

#endif //NBEXTAPPCONTENT_H