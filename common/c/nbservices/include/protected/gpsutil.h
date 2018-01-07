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

    @file     gpsutil.h
   
*/
/*
    (C) Copyright 2005 - 2009 by Networks In Motion, Inc.

    The information contained herein is confidential, proprietary
    to Networks In Motion, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#ifndef GPSUTIL_H
#define GPSUTIL_H

#include "paltypes.h"
#include "nbgpstypes.h"
#include "nbexp.h"
#include "cslutil.h"

/*!
    @addtogroup nbgps
    @{
*/

#define GPS_PACKED_LEN 19

NB_DEC boolean gpsfix_pack(NB_GpsLocation* gpsfix, byte* buf, size_t size);
NB_DEC void gpsfix_clear(NB_GpsLocation* gpsfix);

NB_DEC void gpsfix_setpoint(NB_GpsLocation* gpsfix, double lat, double lon);
NB_DEC boolean gpsfix_validloc(NB_GpsLocation* pfix);

NB_DEC NB_Error translate_gps_error(uint32 status, boolean ignore_minor);
NB_DEC boolean is_gps_error(NB_Error err);

NB_DEC boolean heading_good(double min_valid_speed, NB_GpsLocation* pfix);
NB_DEC int get_gps_qual(uint32 gps_warn, NB_GpsLocation* pfix, uint32 gps_time);
NB_DEC uint8 uncencode(double v);

NB_DEC boolean gpsfix_identical(NB_GpsLocation* gpsfix1, NB_GpsLocation* gpsfix2);

/*! @} */
#endif
