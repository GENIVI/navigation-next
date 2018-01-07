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

 @file     nbnaturallaneguidanceprocessor.h
 */
/*
 (C) Copyright 2012 by TeleCommunications Systems, Inc.

 The information contained herein is confidential, proprietary
 to TeleCommunication Systems, Inc., and considered a trade secret as
 defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of TeleCommunication Systems is granted only
 under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.

 ---------------------------------------------------------------------------*/

#ifndef NBNATURALLANEGUIDANCEPROCESSOR_H
#define NBNATURALLANEGUIDANCEPROCESSOR_H

#include "data_lane_guidance_item.h"
/* Remarks:
   NLG: Natural and Lane Guidance
*/
typedef struct natural_and_lane_guiance_state
{
    // TODO: Add members needed
    uint32 laneGuidancePlayingManeuverId;
}NB_NLGState;

typedef struct natural_and_lane_guiance_information
{
    // TODO: Add members needed
    char revert; // TODO: Disable error C2016 in win32, Delete after there are some members
}NB_NLGInformation;


NB_Error
NB_NLGProcessorUpdate(NB_Navigation * navigation,
                      NB_RouteInformation * route,
                      NB_PositionRoutePositionInformation * currentPosition,
                      NB_NLGInformation* information,
                      double speed
                      );

NB_Error
NB_NavigationVisualLaneGuidanceShowing(NB_Navigation* navigation,
                                       uint32 maneuverIndex,
                                       data_lane_info *laneInfo);
NB_Error
NB_NLGInformationCreate(NB_Context * context, NB_NLGInformation ** nlgInformation);

NB_Error
NB_NLGInformationDestroy(NB_NLGInformation * nlgInformation);

NB_Error
NB_NLGStateCreate(NB_Context * context, NB_NLGState ** nlgState);

NB_Error
NB_NLGStateDestroy(NB_NLGState * nlgState);

NB_Error
GetLaneCharacters(NB_RouteParameters * pRouteParmaters, NB_Lane * lane, data_lane_guidance_item * item);

#endif
