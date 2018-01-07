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

@file     namebitmapmask.h
@defgroup Test and group filenames and masks

Function declarations for test filenames used for system test.
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

#ifndef _NAMEBITMASKMAP_H_
#define _NAMEBITMASKMAP_H_

#include "paltypes.h"

//#define MAX_SUITES          19
//#define MAX_GROUPS          5

struct NameBitmaskMapping
{
    const char*     name;
    uint64          bitmask;
};

#define SUITE_NETWORK           0x0000000000000001ULL
#define SUITE_RASTERMAP         0x0000000000000002ULL
#define SUITE_RASTERTILE        0x0000000000000004ULL
#define SUITE_VECTORMAP         0x0000000000000008ULL
#define SUITE_GEOCODE           0x0000000000000010ULL
#define SUITE_SEARCHBUILDER     0x0000000000000020ULL
#define SUITE_POI               0x0000000000000040ULL
#define SUITE_NAVIGATION        0x0000000000000080ULL
#define SUITE_DIRECTIONS        0x0000000000000100ULL
#define SUITE_SPATIAL           0x0000000000000200ULL
#define SUITE_SPEECH            0x0000000000000400ULL
#define SUITE_FILESET           0x0000000000000800ULL
#define SUITE_MOTD              0x0000000000001000ULL
#define SUITE_PLACEMESSAGE      0x0000000000002000ULL
#define SUITE_SYNC              0x0000000000004000ULL
#define SUITE_PROFILE           0x0000000000008000ULL
#define SUITE_DATASTORE         0x0000000000010000ULL
#define SUITE_SMS               0x0000000000020000ULL
#define SUITE_REVERSEGEOCODE    0x0000000000040000ULL
#define SUITE_ROUTE             0x0000000000080000ULL
#define SUITE_QALOG             0x0000000000100000ULL
#define SUITE_SUBSCRIPTION      0x0000000000200000ULL
#define SUITE_GPSTYPES          0x0000000000400000ULL
#define SUITE_TRISTRIP          0x0000000000800000ULL
#define SUITE_ANALYTICS         0x0000000001000000ULL
#define SUITE_ERS               0x0000000002000000ULL
#define SUITE_LICENSE           0x0000000004000000ULL
#define SUITE_LOCATION          0x0000000008000000ULL
#define SUITE_ENHANCEDCONTENT   0x0000000010000000ULL
#define SUITE_LOCATIONSERVICES  0x0000000020000000ULL
#define SUITE_SINGLESEARCH      0x0000000040000000ULL
#define SUITE_TILESERVICE       0x0000000080000000ULL
#define SUITE_SPEEDCAMERAS      0x0000000100000000ULL
#define SUITE_PUBLICTRANSIT     0x0000000200000000ULL
#define SUITE_POIIMAGEMANAGER        0x0000000400000000ULL
#define SUITE_MAPVIEW                0x0000000800000000ULL
#define SUITE_BATCHTOOL              0x0000001000000000ULL
#define SUITE_SINGLESEARCHSERVICES   0x0000002000000000ULL
#define SUITE_NAVAPI            0x0000004000000000ULL
#define SUITE_ALL               0xFFFFFFFFFFFFFFFFULL
#define SUITE_LAYERMANAGER           0x0000004000000000ULL

#define GROUP_ALL               0xFFFFFFFFFFFFFFFFULL

#define GROUP_MAP               SUITE_RASTERMAP | SUITE_RASTERTILE | SUITE_VECTORMAP | SUITE_SPATIAL | SUITE_TRISTRIP | SUITE_TILESERVICE
#define GROUP_GEOCODE           SUITE_GEOCODE | SUITE_REVERSEGEOCODE
#define GROUP_NAVIGATION        SUITE_NAVIGATION | SUITE_DIRECTIONS | SUITE_ROUTE | SUITE_GPSTYPES | SUITE_ENHANCEDCONTENT | SUITE_SPEEDCAMERAS | SUITE_NAVAPI
#define GROUP_NBSERVICES        SUITE_NETWORK | SUITE_SEARCHBUILDER | SUITE_GPSTYPES | GROUP_GEOCODE | GROUP_MAP | GROUP_NAVIGATION | SUITE_ANALYTICS | SUITE_LOCATION | SUITE_LOCATIONSERVICES | SUITE_TILESERVICE
#define GROUP_ABSERVICES        SUITE_NETWORK | SUITE_SPEECH | SUITE_FILESET | SUITE_MOTD | SUITE_PLACEMESSAGE | SUITE_SYNC | SUITE_PROFILE | SUITE_DATASTORE | SUITE_SMS | SUITE_QALOG | SUITE_SUBSCRIPTION | SUITE_ERS | SUITE_LICENSE

#ifdef __cplusplus 
extern "C"
{
#endif

    extern const int suiteCount;
    extern const int groupCount;
    extern struct NameBitmaskMapping suiteMap[];
    extern struct NameBitmaskMapping groupMap[];

#ifdef __cplusplus 

};
#endif

#endif //_NAMEBITMASKMAP_H_

/*! @} */
