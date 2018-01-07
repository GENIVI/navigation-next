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

    @file     data_location_query.h
    
    Interface for TPS queries to the Location servlet.
*/
/*
    (C) Copyright 2010 by Networks In Motion, Inc.

    The information contained herein is confidential, proprietary
    to Networks In Motion, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.
---------------------------------------------------------------------------*/

/*! @{ */

#ifndef DATA_LOCATION_QUERY_H
#define DATA_LOCATION_QUERY_H

#include "nbexp.h"
#include "datautil.h"
#include "data_gsm.h"
#include "data_cdma.h"
#include "data_wifi.h"
#include "data_location_moved_point.h"
#include "data_proxy_api_key.h"

typedef struct data_location_query_
{
    /* Child Elements */
    struct CSL_Vector*  vec_gsm;
    struct CSL_Vector*  vec_cdma;
    struct CSL_Vector*  vec_wifi;
    struct CSL_Vector*  vec_moving_wifi;

    boolean             want_location_tiles;    /*!< If this element is present, the server will return location-tile elements when available in the reply.*/
    boolean             want_location_setting;  /*!< If this element is present, the location reply might include an optional use-location-setting element.*/
    data_proxy_api_key  proxy_api_key;          /*!< If this element is present, it contains an API Key. */

    /* Attributes */

} data_location_query;

NB_Error    data_location_query_init(data_util_state* pds, data_location_query* plq);
void        data_location_query_free(data_util_state* pds, data_location_query* plq);

tpselt      data_location_query_to_tps(data_util_state* pds, data_location_query* plq);

boolean     data_location_query_add_gsm(data_util_state* pds, data_location_query* plq, uint32 mcc, uint32 mnc, uint32 lac, uint32 cellid, int16 signalStrength);
boolean     data_location_query_add_cdma(data_util_state* pds, data_location_query* plq, uint32 sid, uint32 nid, uint32 cellid, int16 signalStrength);
boolean     data_location_query_add_wifi(data_util_state* pds, data_location_query* plq, const char* mac_address, int16 signalStrength);
/* 
  See data_location_moved_point.h for cause codes.
*/
boolean     data_location_query_add_location_moved_point( data_util_state* pds,
                                                          data_location_query* plq,
                                                          const char* macAddress,
                                                          uint8 cause,
                                                          uint32 timeStamp
                                                         );

/*! @} */

#endif // DATA_LOCATION_QUERY_H
