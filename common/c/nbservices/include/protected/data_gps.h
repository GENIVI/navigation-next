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

/*****************************************************************/
/*                                                               */
/* (C) Copyright 2004 by Networks In Motion, Inc.                */
/*                                                               */
/* The information contained herein is confidential, proprietary */
/* to Networks In Motion, Inc., and considered a trade secret as */
/* defined in section 499C of the penal code of the State of     */
/* California. Use of this information by anyone other than      */
/* authorized employees of Networks In Motion is granted only    */
/* under a written non-disclosure agreement, expressly           */
/* prescribing the scope and manner of such use.                 */
/*                                                               */
/*****************************************************************/

/*-
 * data_image.h: created 2004/12/09 by Mark Goddard.
 */

#ifndef DATA_GPS_
#define DATA_GPS_

#include "datautil.h"
#include "data_point.h"
#include "nbgpstypes.h"
#include "gpsutil.h"
#include "nbexp.h"

typedef struct data_gps_ {

    byte packed[GPS_PACKED_LEN];

} data_gps;

NB_DEC NB_Error data_gps_init(data_util_state* pds, data_gps* pg);
NB_DEC void     data_gps_free(data_util_state* pds, data_gps* pg);

NB_DEC tpselt       data_gps_to_tps(data_util_state* pds, data_gps* pg);
NB_DEC NB_Error data_gps_from_tps(data_util_state* pds, data_gps* pg, tpselt te);

NB_DEC boolean      data_gps_equal(data_util_state* pds, data_gps* pg1, data_gps* pg2);
NB_DEC NB_Error data_gps_copy(data_util_state* pds, data_gps* pg_dest, data_gps* pg_src);

NB_DEC void     data_gps_from_gpsfix(data_util_state* pds, data_gps* pg, NB_GpsLocation* pfix);

NB_DEC uint32   data_gps_get_tps_size(data_util_state* pds, data_gps* pg);

NB_DEC void     data_gps_to_buf(data_util_state* pds, data_gps* pg, struct dynbuf* pdb);

NB_DEC NB_Error data_gps_from_binary(data_util_state* pds, data_gps* pg, byte** pdata, size_t* pdatalen);

#endif

