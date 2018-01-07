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

 @file     nbspecialregioninformationprivate.h
 */
/*
 (C) Copyright 2014 by TeleCommunications Systems, Inc.

 The information contained herein is confidential, proprietary
 to TeleCommunication Systems, Inc., and considered a trade secret as
 defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of TeleCommunication Systems is granted only
 under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.

 ---------------------------------------------------------------------------*/

/*! @{ */
#ifndef NB_SPECIAL_REGION_INFORMATION_PRIVATE_H
#define NB_SPECIAL_REGION_INFORMATION_PRIVATE_H

#include "paltypes.h"
#include "nbenhancedcontentmanager.h"
#include "nbnavigation.h"
#include "data_nav_reply.h"
#include "vec.h"

typedef struct NB_SpecialRegion
{
    uint32        index;
    char*         regionType;
    char*         regionDescription;
    char*         signId;
    char*         version;
    char*         datasetid;
    uint32        startManeuver;
    double        startManeuverOffset;
    uint32        endManeuver;
    double        endManeuverOffset;
    uint32        nextOccurStartTime;
    uint32        nextOccurEndTime;
    nb_boolean    haveAlert;
} NB_SpecialRegion;

/*! struct of special region information. */
typedef struct NB_SpecialRegionInformation
{
    NB_Context* context;
    struct CSL_Vector* specialRegions;
} NB_SpecialRegionInformation;

/*! Create a special region information from tps

 @return NB_OK if success.
 @see NB_SpecialRegionInformationDestroy
 */
NB_Error
NB_SpecialRegionInformationCreateFromDataNavReply(NB_Context* context,                         /*!< Pointer to current context. */
                                                  data_nav_reply* reply,                       /*!< nav replay data */
                                                  NB_SpecialRegionInformation** information    /*!< return information. */
                                                  );


/*! Destroy special region information.

 @return NE_OK is success
 @see NB_SpecialRegionInformationCreateFromDataNavReply
 */
NB_Error
NB_SpecialRegionInformationDestroy(NB_SpecialRegionInformation* information    /*!< Destroy information. */
                                   );
#endif
/*! @} */
