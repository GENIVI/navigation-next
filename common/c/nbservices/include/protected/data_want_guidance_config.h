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

 @file     data_want_guidance_config.h
 */
/*
 (C) Copyright 2012 by TeleCommunications Systems, Inc.

 The information contained herein is confidential, proprietary
 to TeleCommunication Systems, Inc., and considered a trade secret as
 defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of TeleCommunications Systems, Inc. is granted only
 under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.
 ---------------------------------------------------------------------------*/

/*! @{ */

#ifndef DATA_WANT_GUIDANCE_CONFIG_H
#define DATA_WANT_GUIDANCE_CONFIG_H

#include "datautil.h"
#include "data_string.h"


typedef struct data_want_guidance_config_
{

    /* Child Elements */
    boolean want_enchanced_natural_guidance;
    boolean want_natural_guidance;
    boolean want_lane_guidance;
    boolean want_play_times;
    boolean want_guidance_config;

    /* Attributes */

} data_want_guidance_config;

NB_Error
data_want_guidance_config_init(data_util_state* state,
                               data_want_guidance_config* items
                               );
void
data_want_guidance_config_free(data_util_state* state,
                               data_want_guidance_config* items
                               );
NB_Error
data_want_guidance_config_from_tps(data_util_state* state,
                                   data_want_guidance_config* wantGuidanceConfig,
                                   tpselt tpsElement
                                   );

boolean
data_want_guidance_config_equal(data_util_state* state,
                                data_want_guidance_config* wantGuidanceConfig1,
                                data_want_guidance_config* wantGuidanceConfig2
                                );
NB_Error
data_want_guidance_config_copy(data_util_state* state,
                               data_want_guidance_config* destinationWantGuidanceConfig,
                               data_want_guidance_config* sourceWantGuidanceConfig
                               );
tpselt
data_want_guidance_config_to_tps(data_util_state* state,
                                 data_want_guidance_config* items
                                 );

#endif
/*! @} */