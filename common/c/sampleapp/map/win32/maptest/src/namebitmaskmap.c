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

@file     namebitmaskmap.c
@defgroup Test and group names and masks

Test and group names used for system test.
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

#include "namebitmaskmap.h"

struct NameBitmaskMapping suiteMap[] = {
    { "network",            SUITE_NETWORK },
    { "rastermap",          SUITE_RASTERMAP },
    { "rastertile",         SUITE_RASTERTILE },
    { "vectormap",          SUITE_VECTORMAP },
    { "geocode",            SUITE_GEOCODE },
    { "searchbuilder",      SUITE_SEARCHBUILDER },
    { "navigation",         SUITE_NAVIGATION },
    { "directions",         SUITE_DIRECTIONS },
    { "spatial",            SUITE_SPATIAL },
    { "speech",             SUITE_SPEECH },
    { "fileset",            SUITE_FILESET },
    { "motd",               SUITE_MOTD },
    { "placemessage",       SUITE_PLACEMESSAGE },
    { "sync",               SUITE_SYNC },
    { "profile",            SUITE_PROFILE },
    { "datastore",          SUITE_DATASTORE },
    { "sms",                SUITE_SMS },
    { "reversegeocode",     SUITE_REVERSEGEOCODE },
    { "route",              SUITE_ROUTE },
    { "qalog",              SUITE_QALOG },
    { "subscription",       SUITE_SUBSCRIPTION },
    { "tristrip",           SUITE_TRISTRIP },
    { "analytics",          SUITE_ANALYTICS },
    { "ers",                SUITE_ERS },
    { "license",            SUITE_LICENSE },
    { "location",           SUITE_LOCATION },
    { "enhancedcontent",    SUITE_ENHANCEDCONTENT },
    { "locationservices",   SUITE_LOCATIONSERVICES },
    { "tileservice",        SUITE_TILESERVICE },
    { "speedcameras",       SUITE_SPEEDCAMERAS },
    { "publictransit",      SUITE_PUBLICTRANSIT },
    { "singlesearch",       SUITE_SINGLESEARCH },
    { "gpstypes",           SUITE_GPSTYPES },
    { "poiimagemanager",        SUITE_POIIMAGEMANAGER },
    { "batchtool",              SUITE_BATCHTOOL },
    { "singlesearchservices",   SUITE_SINGLESEARCHSERVICES },
    { "navapi",             SUITE_NAVAPI},
    { "MapView",            SUITE_MAPVIEW },
    { "Map",                SUITE_LAYERMANAGER }
};

const int suiteCount = sizeof(suiteMap) / sizeof(suiteMap[0]);

struct NameBitmaskMapping groupMap[] = {
    { "nbservices",         GROUP_NBSERVICES },
    { "abservices",         GROUP_ABSERVICES },
    { "map",                GROUP_MAP },
    { "geocode",            GROUP_GEOCODE },
    { "navigation",         GROUP_NAVIGATION }
};

const int groupCount = sizeof(groupMap) / sizeof(groupMap[0]);


/*! @} */
