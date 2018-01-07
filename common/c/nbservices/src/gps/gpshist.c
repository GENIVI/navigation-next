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

    @file     gpshist.c
    @defgroup gphist GPS
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


/// @todo (BUG 56060) This file is obsolete


#include "gpshist.h"
#include "bsdqueue.h"
#include "cslerrorutil.h"
#include "gpsutil.h"

/*! @{ */

static void fix_history_clear(struct gpshistory* phist);

NB_DEF NB_Error    
fix_history_init(struct gpshistory* phist, PAL_Instance* pal, struct navprefs* pprefs)
{
    PAL_Error npe = PAL_Ok;
    fix_history_clear(phist);

    phist->pal = pal;
    phist->pprefs = pprefs;

    if (phist->pal == NULL)
    {
        return NE_INVAL;
    }

/*
    phist->track.cb = fix_history_gps_cb;
    phist->track.cbraw = NULL;
    phist->track.cbarg = phist;
    phist->track.period = -1;
    phist->track.req = AEEGPS_GETINFO_LOCATION;
    phist->track.acc = AEEGPS_ACCURACY_LOWEST;
#ifdef ENABLE_GPS_FILTER
    phist->track.flags = GTF_FILTER;
#else
    phist->track.flags = 0;
#endif

    MEMSET(&phist->loc, NULL, sizeof(phist->loc));
    gps_track(phist->pgpsctx, &phist->track);
*/
    
    // TODO -- Set up criteria properly

    // :TODO:
    // comment out for now to avoid crash in system test !
//    npe = pal_gps_begintracking(phist->pal, TRUE, &crit, fix_history_gps_cb, phist);
    
    return CSL_TranslatePalError(npe);
}

static void
fix_history_clear(struct gpshistory* phist)
{
    gpsfix_clear(&phist->fix);
    phist->fix_valid = FALSE;
    gpsfix_clear(&phist->last_heading_fix);
    phist->last_heading_fix_valid = FALSE;
    phist->last_heading_fix_same = FALSE;
}

NB_DEF void
fix_history_destroy(struct gpshistory* phist)
{
    //gps_untrack(phist->pgpsctx, &phist->track);    
    //TODO: Need to conform to the new gps API pal_gps_endtracking(phist->pal, fix_history_gps_cb, phist);

    fix_history_clear(phist);
}

NB_DEF NB_Error    
fix_history_add(struct gpshistory* phist, NB_GpsLocation* pfix)
{
    //assert(((pfix->status == PGE_None) && (pfix->valid & (NGV_Latitude | NGV_Longitude))), ("Invalid GPS point added to history"));
    
    phist->fix = *pfix;
    phist->fix_valid = TRUE;
    
    //@todo Check Heading
    /*
    if (heading_good(phist->pprefs->nav_min_speed_valid_heading, pfix)) {

        phist->last_heading_fix = *pfix;
        phist->last_heading_fix_valid = TRUE;
        phist->last_heading_fix_same = TRUE;
    }
    else
        phist->last_heading_fix_same = FALSE;
    */

    return NE_OK;
}

NB_DEF void
fix_history_foreach(struct gpshistory* phist, void (*cb)(void *arg, NB_GpsLocation *), void* cbarg)
{
    if (phist->fix_valid)
    {
        cb(cbarg, &phist->fix);
    }
    if (phist->last_heading_fix_valid && !phist->last_heading_fix_same)
    {
        cb(cbarg, &phist->last_heading_fix);
    }
}

NB_DEF NB_Error    
fix_history_latest(struct gpshistory* phist, NB_GpsLocation* pfix)
{
    if (!phist->fix_valid)
    {
        return NE_NOENT;
    }
    *pfix = phist->fix;

    return NE_OK;
}

/*! @} */
