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

    @file     data_gps_probes_event.h

    Interface to create Gps-Probes-Event TPS element.

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
#ifndef DATA_GPS_PROBES_EVENT_
#define DATA_GPS_PROBES_EVENT_

#include "nbexp.h"
#include "datautil.h"
#include "dynbuf.h"
#include "data_gps.h"
#include "data_unpacked_gps.h"
#include "data_gps_offset.h"
#include "data_string.h"
#include "vec.h"

typedef struct data_gps_probes_event_ {
    /* Child Elements */
    /* data_gps_probes_event has children elements of gps and gps offset,
     * One of the Device Analytics requirements is that events are reported
     * in the reverse time order, i.e, newer events reported first, and older
     * event reported later. What this requirement means to gps-probes-event
     * is that we need pack a bunch of gps locations in reverse time order to a
     * gps-probes-event. Since gps-offset allows only positive time offset, we
     * need to make the oldest gps-probe the origin of this event, and pack all
     * other gps-probes as gps-offset, so that all the time-offset field are
     * positive value. To facilitate this, instead of storing data-gps and
     * data-gps-offset element here, a vector of gps locations are stored
     * and data-gps and data-gps-offset are created when they are needed to
     * create TPS elements.
     */
    struct CSL_Vector*  gps_locations;

    /* Attributes */
    uint32          nav_session_id;     /* Unique Id for this nav. session. */
    data_string     packing_version;    /* version of the protocol used for
                                           packing the GPS probes. */

    uint32 size;            /* estimate of the size of this element. */
} data_gps_probes_event;

NB_Error data_gps_probes_event_init(data_util_state* pds,
            data_gps_probes_event* pgpe);

void data_gps_probes_event_free(data_util_state* pds,
            data_gps_probes_event* pgpe);

tpselt data_gps_probes_event_to_tps(data_util_state* pds,
            data_gps_probes_event* pgpe);

boolean data_gps_probes_event_equal(data_util_state* pds,
            data_gps_probes_event* pgpe1, data_gps_probes_event* pgpe2);

NB_Error data_gps_probes_event_copy(data_util_state* pds,
            data_gps_probes_event* pgpe_dest, data_gps_probes_event* pgpe_src);

uint32 data_gps_probes_event_get_tps_size(data_util_state* pds,
            data_gps_probes_event* pgpe);

void data_gps_probes_event_to_buf(data_util_state* pds,
            data_gps_probes_event* pgpe,
            struct dynbuf* pdb);

NB_Error data_gps_probes_event_from_binary(data_util_state* pds,
            data_gps_probes_event* pgpe,
            byte** pdata, size_t* pdatalen);

/*  GPS should be addd in the reverse time order, i.e, newer gps probes added
 *  first, older gps probes added later */
NB_Error data_gps_probes_event_add_gps(data_util_state* pds,
            data_gps_probes_event* pgpe,
            uint32 nav_session_id,
            NB_GpsLocation* location);

/* get the number of gps fixes in this event */
uint32 data_gps_probes_event_get_gps_count(data_gps_probes_event* pgpe);

/* get the gps fix specified by index, 0 means the newest, count-1 means
 * the oldest.
 */
NB_GpsLocation* data_gps_probes_event_get_gps(data_gps_probes_event* pgpe,
        int index);

#endif
/*! @} */

