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

    @file     nbguidanceinformationprivate.h
    @defgroup nbguidanceinformationprivate

    This API is used to create Guidance Information objects.  Guidance Parameters objects can be used to retrieve
    guidance information.

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

#ifndef GUIDANCEINFORMATIONPRIVATE_H
#define GUIDANCEINFORMATIONPRIVATE_H

#include "nbguidanceinformation.h"
#include "nbguidancemessage.h"
#include "instructset.h"

/*! Get Guidance information voice instructset

@param information A NB_GuidanceInformation objects
@param voice On success, the newly created instructset object; NULL otherwise.  This is a pointer to an internal object which will only be valid during the lifetime of this object.
@returns NB_Error
*/
NB_Error
NB_GuidanceInformationGetVoices(NB_GuidanceInformation* information, instructset** voice);

/*! Get Guidance information steps instructset

@param information A NB_GuidanceInformation objects
@param steps On success, the newly created instructset object; NULL otherwise.  This is a pointer to an internal object which will only be valid during the lifetime of this object.
@returns NB_Error
*/
NB_Error
NB_GuidanceInformationGetSteps(NB_GuidanceInformation* information, instructset** steps);

/*! Get Guidance information image instructset

@param information A NB_GuidanceInformation objects
@param images On success, the newly created instructset object; NULL otherwise.  This is a pointer to an internal object which will only be valid during the lifetime of this object.
@returns NB_Error
*/
NB_Error
NB_GuidanceInformationGetImages(NB_GuidanceInformation* information, instructset** images);

/*! Get Guidance information voice  style string

@param information A NB_GuidanceInformation objects
@param voice On success, copies voice string which must be deleted using nsl_free
@returns NB_Error
*/
NB_Error
NB_GuidanceInformationGetVoiceStyle(NB_GuidanceInformation* information, char** voice);

/*! Get Guidance information steps  style string

@param information A NB_GuidanceInformation objects
@param steps On success, copies steps string
@returns NB_Error
*/
NB_Error
NB_GuidanceInformationGetStepStyle(NB_GuidanceInformation* information, char* steps);

/*! Get Guidance information image style string

@param information A NB_GuidanceInformation objects
@param images On success, copies images string
@returns NB_Error
*/
NB_Error
NB_GuidanceInformationGetImageStyle(NB_GuidanceInformation* information, char* images);

/*! Get Guidance information set data source function

@param images On success, receives a pointer to a read-only string containing the images style name
@param datasourceFunc The DataSource function
@param userData Callback data for Data Source function
@returns NB_Error
*/
NB_Error
NB_GuidanceInformationSetDataSourceFunction(NB_GuidanceInformation* information, NB_GuidanceDataSourceFunction datasourceFunc, void* userData);


NB_DEC NB_Error NB_GuidanceInformationGetTurnImage(NB_GuidanceInformation* information,
                    NB_GuidanceDataSourceFunction dataSource, void* dataSourceUserData, uint32 index,
                    NB_NavigateManeuverPos maneuverPosition, NB_NavigateAnnouncementUnits units, char* imageCodeBuffer,
                    uint32 imageCodeBufferSize);

NB_DEC NB_Error NB_GuidanceInformationGetTurnCharacter(NB_GuidanceInformation* information,
                                            NB_GuidanceDataSourceFunction dataSource, void* dataSourceUserData, uint32 index,
                                            NB_NavigateManeuverPos maneuverPosition, NB_NavigateAnnouncementUnits units, char* characterBuffer,
                                            uint32 characterBufferSize);


NB_DEC NB_Error NB_GuidanceInformationGetTurnAnnouncement(NB_GuidanceInformation* information,
                    NB_GuidanceDataSourceFunction dataSource, void* dataSourceUserData, uint32 index,
                    NB_NavigateManeuverPos maneuverPosition, NB_NavigateAnnouncementSource source,
                    NB_NavigateAnnouncementType type, NB_NavigateAnnouncementUnits units, NB_GuidanceMessage** message, uint32* soundTypes);

NB_DEC NB_Error NB_GuidanceInformationGetRouteCalculationAnnouncement(NB_GuidanceInformation* information,
                    NB_GuidanceDataSourceFunction dataSource, void* dataSourceUserData,
                    NB_NavigateRouteCalculationType type, NB_NavigateAnnouncementType announceType, NB_NavigateAnnouncementUnits units,
                    NB_GuidanceMessage** message);

NB_DEC NB_Error NB_GuidanceInformationEnumerateManeuverText(NB_GuidanceInformation* information,
                    NB_GuidanceDataSourceFunction dataSource, void* dataSourceUserData,
                    NB_RouteTextType textType, NB_NavigateAnnouncementUnits units, uint32 index,
                    NB_GuidanceInformationTextCallback callback, void* userData);

NB_DEC NB_Error NB_GuidanceInformationEnumerateStackedTurnText(NB_GuidanceInformation* information,
                    NB_GuidanceDataSourceFunction dataSource, void* dataSourceUserData,
                    NB_RouteTextType textType, NB_NavigateAnnouncementUnits units, uint32 index,
                    NB_GuidanceInformationTextCallback callback, void* userData);

NB_DEC NB_Error NB_GuidanceInformationEnumerateArrivalText(NB_GuidanceInformation* information,
                    NB_GuidanceDataSourceFunction dataSource, void* dataSourceUserData,
                    NB_RouteTextType textType, NB_NavigateAnnouncementUnits units, uint32 index,
                    NB_GuidanceInformationTextCallback callback, void* userData);

NB_DEC NB_Error NB_GuidanceInformationGetNavigationModeSwitchAnnouncement(NB_GuidanceInformation* information,
                    NB_GuidanceDataSourceFunction dataSource, void* dataSourceUserData,
                    NB_NavigateAnnouncementType announceType, NB_NavigateAnnouncementUnits units,
                    NB_GuidanceMessage** message);

NB_DEC NB_Error NB_GuidanceInformationGetNavigationModeSwitchToStaticAnnouncement(NB_GuidanceInformation* information,
                    NB_GuidanceDataSourceFunction dataSource, void* dataSourceUserData,
                    NB_NavigateAnnouncementType announceType, NB_NavigateAnnouncementUnits units,
                    NB_GuidanceMessage** message);

NB_NavigateManeuverPos NB_GuidanceInformationGetRouteCalculationManeuver(NB_NavigateRouteCalculationType routeCalculationType);

/*! @} */

#endif // GUIDANCEINFORMATIONPRIVATE_H
