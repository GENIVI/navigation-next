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
 
 @file nbgmmapviewconfig.h
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

#ifndef _NBGM_MAP_VIEW_CONFIG_H_
#define _NBGM_MAP_VIEW_CONFIG_H_
#include "pal.h"
#include "nbgmrendersystemtype.h"
#include "nbtaskqueue.h"

class NBGM_RenderContext;
/*! NBGM configuration parameters
 
 This struct specifies NBGM global configuration used to construct the namager instance
 */
typedef struct NBGM_MapViewConfig
{
    shared_ptr<TaskQueue>               renderTaskQueue;        /*!< The task queue used by renderring NBGM map view. */
    NBGM_RenderSystemType               renderSystemType;       /*!< Specify the render system type */
    shared_ptr<NBGM_RenderContext>      renderContext;          /*!< The Render context used by this view*/
    nb_boolean                          drawSky;                /*!< Draw sky or not */
    nb_boolean                          drawFlag;               /*!< Draw flag or not */
    nb_boolean                          drawAvatar;             /*!< Draw avatar or not */
    uint32                              renderInterval;         /*!< Interval for render */
    NBGM_MapViewConfig():
        renderSystemType(NBGM_RS_GLES),
        drawSky(FALSE),
        drawFlag(FALSE),
        drawAvatar(FALSE),
        renderInterval(18)
    {
    }
}NBGM_MapViewConfig;

#endif
