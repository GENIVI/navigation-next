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
 @file     NavigatinoImplBuilder.h
 */
/*
 (C) Copyright 2014 by TeleCommunication Systems, Inc.

 The information contained herein is confidential, proprietary
 to TeleCommunication Systems, Inc., and considered a trade secret as
 defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of TeleCommunication Systems, is granted only
 under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.

 ---------------------------------------------------------------------------*/
/*! @{ */

#ifndef __NAVIGATIONIMPLBUILDER_H__
#define __NAVIGATIONIMPLBUILDER_H__

#include "NavApiTypes.h"
#include "NavApiPreferences.h"
#include "NavApiRouteOptions.h"
extern "C"
{
#include "nberror.h"
#include "nbnavigation.h"
#include "nbenhancedcontentmanager.h"
}

namespace nbnav
{
const int TRAFFIC_POLLING_INTERVAL      = 3;

void AddToneIntoVoiceCache(PAL_Instance* m_pal, NB_Context* m_context, const char* workpath);
NB_Error LoadFile(PAL_Instance* m_pal, std::string fullName, char** pbuf, nb_size* psize);
NB_EnhancedContentManager* CreateEnhancedContentManager(NB_Context* m_context,
                                                        NB_RouteConfiguration* routeConfiguration,
                                                        const char* workpath);
void SetupEnhancedContentMapConfiguration(NB_EnhancedContentMapConfiguration* enhancedConfiguration);
void SetupEnhancedContentManagerConfiguration(NB_Context* m_context,
                                              NB_EnhancedContentManagerConfiguration* config,
                                              const char* workpath);
void SetupLaneGuidanceConfiguration(NB_Context* m_context, NB_RouteParameters* pRouteParameters,
                                    const char* workpath);
void SetupRouteConfiguration(NB_RouteConfiguration* config, const Preferences* preference,
                            bool enhancedStartup);
void SetupStaticRouteConfiguration(NB_RouteConfiguration* config, const Preferences* preference);
void SetupRouteSummaryConfiguration(NB_RouteConfiguration* config);
void SetupRouteOptions(NB_RouteOptions* options, const RouteOptions* routeOptions,
                       const Preferences* preference, const char* voiceStyle);
void SetupNavigationConfiguration(NB_NavigationConfiguration* configuration,
                                  const Preferences* preference);
void SetupForPedestrianAndBike(NB_RouteOptions* options, NB_RouteConfiguration* config,
                               const RouteOptions* routeOption);
NB_GuidanceInformation* CreateGuidanceInformation(NB_Context* context, PAL_Instance* pal,
                                                  const char* workFolder, const char* baseVoicePath,
                                                  const char* voiceStyle, const char* locale,
                                                  bool isTTSEnabled);
void PlaceToNbPlace(const Place& place, NB_Place* nbPlace);
void ConfigNBNavigation(NB_Navigation* nbNavigation,
                        const Preferences& preference,
                        NB_NavigateAnnouncementUnits units);
}
#endif

/*! @} */
