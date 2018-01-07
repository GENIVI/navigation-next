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

    @file     data_location_reply.h

    Interface for TPS replies from the Location servlet.
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

#ifndef DATA_LOCATION_REPLY_H
#define DATA_LOCATION_REPLY_H

#include "nbexp.h"
#include "datautil.h"
#include "data_location_position.h"
#include "data_location_error.h"
#include "data_location_tile.h"

typedef struct data_location_reply_ {

    /* Child Elements */
    data_location_position    cellid_position;
    data_location_error       cellid_error;           /*!< Error element for location request. */
    struct CSL_Vector*        location_tiles;         /*!< list of data_location_tile */
    boolean                   use_location_setting;   /*!< This element will only be present if want-device-setting is present in the query and the configuration setting for the device's platform is to associate LK with the location setting on the device. */

    /* Attributes */

} data_location_reply;

NB_Error        data_location_reply_init(data_util_state* pds, data_location_reply* plr);
void            data_location_reply_free(data_util_state* pds, data_location_reply* plr);

NB_Error        data_location_reply_from_tps(data_util_state* pds, data_location_reply* plr, tpselt te);

/*! @} */

#endif // DATA_LOCATION_REPLY_H
