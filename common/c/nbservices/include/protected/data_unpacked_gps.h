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

 @file     data_unpacked_gps.h

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

#ifndef DATA_UNPACKED_GPS_H_
#define DATA_UNPACKED_GPS_H_

#include "nberror.h"
#include "nbgpstypes.h"
#include "datautil.h"
#include "data_point.h"

typedef struct data_unpacked_gps {

    data_point point;
    uint64 time;
    float heading;
    float speed;
    float altitude;
    float umag;
    float uang;
    float ua;
    float up;

} data_unpacked_gps;

NB_Error data_unpacked_gps_init(data_util_state* pds,
                                data_unpacked_gps* pdug);

void data_unpacked_gps_free(data_util_state* pds,
                            data_unpacked_gps* pdug);

tpselt data_unpacked_gps_to_tps(data_util_state* pds,
                                data_unpacked_gps* pdug);

boolean data_unpacked_gps_equal(data_util_state* pds,
                                data_unpacked_gps* pdug1, data_unpacked_gps* pdug2);

NB_Error data_unpacked_gps_copy(data_util_state* pds,
                                data_unpacked_gps* pdug_dest, data_unpacked_gps* pdug_src);

void data_unpacked_gps_from_gpsfix(data_util_state* pds, data_unpacked_gps* pg, NB_GpsLocation* pfix);

#endif /* DATA_UNPACKED_GPS_H_ */

/*! @} */
