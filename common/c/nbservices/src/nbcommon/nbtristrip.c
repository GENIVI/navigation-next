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

@file     nbtristrip.c
@defgroup nbtristrip nbtristrip

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


#include "nbcontextprotected.h"
#include "paltypes.h"
#include "palstdlib.h"
#include "palmath.h"
#include "spatialvector.h"
#include "nbspatial.h"
#include "data_polyline_pack.h"
#include "data_road.h"
#include "nbtristrip.h"
#include "nbtristripprotected.h"
#include "transformutility.h"
#include "nbvectortile.h"
#include "nbpointiteration.h"

/*! @{ */


// Local Functions .................................................................................

static int polyline_to_merc_multi_tristrip(NB_PointIterationNextPointFunction nextfunc, void* nextuser, NB_Vector* ptristripbg, 
NB_Vector* ptristripfg, int ntristrip, double bgwidth,double fgwidth, double maxlength, NB_Vector* last_pt, NB_Vector* last_dir);

static int polyline_to_merc_multi_tristrip_beveled( NB_PointIterationNextPointFunction nextfunc, void* nextuser, NB_Vector* ptristrip,
        int ntristrip, double lineWidth, NB_Vector* last_pt, NB_Vector* last_dir);

static int8 GetRoadWidth(NB_VectorTileRoadType roadType);
static NB_Error MercatorPolylineToPolylinePack(NB_MercatorPolyline* polyline,NB_VectorTileIndex tileIndex,int zoomLevel, data_polyline_pack** ppack);
static NB_Error GetRoadWidthByTileIndex(NB_VectorTileRoadType roadType,NB_VectorTileIndex tileIndex,int zoomLevel, double* fgRoadWidth, double* bgRoadWidth);


// Public functions ..............................................................................

//@todo (BUG 55872) rename this function to something that matches what it does.
NB_DEF NB_Error
NB_TriStripPolylineToMercatorCreate(
    NB_VectorTileRoadType roadType,
    NB_MercatorPolyline* polyline,
    NB_VectorTileIndex tileIndex,
    int zoomLevel,
    nb_boolean forward,
    double maxLength,
    NB_TriStrip** triStrip,
    NB_MercatorPoint* lastPoint,
    NB_Vector* lastHeading)
{
    NB_Error error = NE_OK;
    int i = 0;
    NB_MercatorPolylineIterationData next_data = {0};
    struct tristrip ts_bg = {0}; 
    struct tristrip ts_fg = {0};
    NB_MercatorTriStrip* fgStrip = 0;
    NB_MercatorTriStrip* bgStrip = 0;
    byte* ptr = 0;
    int size = 0;
    double fgRoadWidth = 0;
    double bgRoadWidth = 0;
    NB_TriStrip* newTriStrip = 0;
    NB_Vector endHeading = {0};
    NB_Vector endPoint = {0};
    NB_PointIterationNextPointFunction* pIterFunction = NULL;

    if (polyline == NULL ||
        triStrip == NULL)
    {
        return NE_INVAL;
    }

    newTriStrip = nsl_malloc(sizeof(NB_TriStrip));
    if (newTriStrip == NULL)
    {
        return NE_NOMEM;
    }
    nsl_memset(newTriStrip, 0, sizeof(NB_TriStrip));

    NB_PointIterationInitializeMercatorPolylineIteration(&next_data, polyline, forward, maxLength);
    if (forward)
    {
        if (maxLength == -1)
        {
            pIterFunction = NB_MercatorPolylineForwardIteration;
        }
        else
        {
            pIterFunction = NB_MercatorPolylineForwardLengthIteration;
        }
    }
    else
    {
        if (maxLength == -1)
        {
            pIterFunction = NB_MercatorPolylineReverseIteration;
        }
        else
        {
            pIterFunction = NB_MercatorPolylineReverseLengthIteration;
        }
    }

    // Get width of road
    GetRoadWidthByTileIndex(roadType, tileIndex, zoomLevel, &fgRoadWidth, &bgRoadWidth);   
 
    size = next_data.polyline->count * NB_TRISTRIP_POINTS_PER_SEGMENT + NB_TRISTRIP_POINTS_ENDCAP;
    ts_bg.npoints = size;
    ts_bg.fg = FALSE;
    ts_bg.points = nsl_malloc(sizeof(NB_Vector)*ts_bg.npoints);

    ts_fg.npoints = size;
    ts_fg.fg = TRUE;
    ts_fg.points = nsl_malloc(sizeof(NB_Vector)*ts_fg.npoints);

    if (ts_bg.points == NULL || ts_fg.points == NULL) 
    {
        error = NE_NOMEM;
        goto error;
    }

    nsl_memset(ts_bg.points, 0, sizeof(NB_Vector)*ts_bg.npoints);
    nsl_memset(ts_fg.points, 0, sizeof(NB_Vector)*ts_fg.npoints);

    ts_fg.npoints = ts_bg.npoints = polyline_to_merc_multi_tristrip(pIterFunction, &next_data,
        ts_bg.points, ts_fg.points, ts_bg.npoints, bgRoadWidth, fgRoadWidth, maxLength, &endPoint, &endHeading);

    // Setup foreground/background tristrip pointer address
    fgStrip = nsl_malloc(sizeof(NB_MercatorTriStrip) + (sizeof(NB_MercatorPoint) * size));
    if (fgStrip == NULL)
    {
        error = NE_NOMEM;
        goto error;
    }

    fgStrip->count = ts_fg.npoints;

    bgStrip = nsl_malloc(sizeof(NB_MercatorTriStrip) + (sizeof(NB_MercatorPoint) * size));
    if (bgStrip == NULL)
    {
        error = NE_NOMEM;
        goto error;
    }

    bgStrip->count = ts_bg.npoints;

    // Setup foreground tristrip points array
    ptr = (byte*)fgStrip;
    ptr += sizeof(NB_MercatorTriStrip);
    fgStrip->points = (NB_MercatorPoint*)ptr;

    // Setup background tristrip points array
    ptr = (byte*)bgStrip;
    ptr += sizeof(NB_MercatorTriStrip);
    bgStrip->points = (NB_MercatorPoint*)ptr;

    for (i = 0; i < ts_fg.npoints; i++)
    {
        fgStrip->points[i].mx = ts_fg.points[i].x;
        fgStrip->points[i].my = ts_fg.points[i].y;

        bgStrip->points[i].mx = ts_bg.points[i].x;
        bgStrip->points[i].my = ts_bg.points[i].y;
    }

    nsl_free(ts_fg.points);
    nsl_free(ts_bg.points);

    newTriStrip->triStripForeground = fgStrip;
    newTriStrip->triStripBackground = bgStrip;
    *triStrip = newTriStrip;

    if (lastPoint != NULL)
    {
        lastPoint->mx = endPoint.x;
        lastPoint->my = endPoint.y;
    }

    if (lastHeading != NULL)
    {
        *lastHeading = endHeading;
    }

    return NE_OK;

error:
    if (ts_fg.points != NULL)
    {
        nsl_free(ts_fg.points);
    }

    if (ts_bg.points != NULL)
    {
        nsl_free(ts_bg.points);
    }

    if (fgStrip != NULL)
    {
        nsl_free(fgStrip);
    }

    if (bgStrip != NULL)
    {
        nsl_free(bgStrip);
    }

    if (newTriStrip)
    {
        nsl_free(newTriStrip);
    }

    return error;
}

NB_DEF NB_Error
NB_TriStripPolylineToMercatorCreateBeveled(
    NB_VectorTileRoadType roadType,
    NB_MercatorPolyline* polyline,
    NB_VectorTileIndex tileIndex,
    int zoomLevel,
    nb_boolean forward,
    double maxLength,
    NB_TriStrip** triStrip,
    NB_MercatorPoint* lastPoint,
    NB_Vector* lastHeading)
{
    NB_Error error = NE_OK;
    int i = 0;
    NB_MercatorPolylineIterationData next_data = {0};
    struct tristrip ts_bg = {0};
    struct tristrip ts_fg = {0};
    NB_MercatorTriStrip* fgStrip = 0;
    NB_MercatorTriStrip* bgStrip = 0;
    byte* ptr = 0;
    int size = 0;
    double fgRoadWidth = 0;
    double bgRoadWidth = 0;
    NB_TriStrip* newTriStrip = 0;
    NB_Vector endHeading = {0};
    NB_Vector endPoint = {0};
    NB_PointIterationNextPointFunction* pIterFunction = NULL;

    if (polyline == NULL || triStrip == NULL)
    {
        return NE_INVAL;
    }

    newTriStrip = (NB_TriStrip*)nsl_malloc(sizeof(NB_TriStrip));
    if (newTriStrip == NULL)
    {
        return NE_NOMEM;
    }
    nsl_memset(newTriStrip, 0, sizeof(NB_TriStrip));

    // Get width of road
    GetRoadWidthByTileIndex(roadType, tileIndex, zoomLevel, &fgRoadWidth, &bgRoadWidth);

    // Allocate tristrip data
    nsl_memset( &next_data, 0, sizeof( next_data ) );
    NB_PointIterationInitializeMercatorPolylineIteration(&next_data, polyline, forward, maxLength);
    if (forward){
        if (maxLength == -1)
            pIterFunction = NB_MercatorPolylineForwardIteration;
        else
            pIterFunction = NB_MercatorPolylineForwardLengthIteration;
    } else {
        if (maxLength == -1)
            pIterFunction = NB_MercatorPolylineReverseIteration;
        else
            pIterFunction = NB_MercatorPolylineReverseLengthIteration;
    }
    size = next_data.polyline->count * NB_TRISTRIP_POINTS_PER_SEGMENT_BEVELED + NB_TRISTRIP_POINTS_ENDCAP_BEVELED;
    ts_bg.npoints = size;
    ts_bg.fg = FALSE;
    ts_bg.points = (NB_Vector*)nsl_malloc(sizeof(NB_Vector)*ts_bg.npoints);

    ts_fg.npoints = size;
    ts_fg.fg = TRUE;
    ts_fg.points = (NB_Vector*)nsl_malloc(sizeof(NB_Vector)*ts_fg.npoints);

    if (ts_bg.points == NULL || ts_fg.points == NULL){
        error = NE_NOMEM;
        goto error;
    }
    nsl_memset(ts_bg.points, 0, sizeof(NB_Vector)*ts_bg.npoints);
    nsl_memset(ts_fg.points, 0, sizeof(NB_Vector)*ts_fg.npoints);

    // Background
    ts_bg.npoints = polyline_to_merc_multi_tristrip_beveled( pIterFunction, &next_data, ts_bg.points,
            ts_bg.npoints, bgRoadWidth, &endPoint, &endHeading);

    // Foreground
    nsl_memset( &next_data, 0, sizeof( next_data ) );
    NB_PointIterationInitializeMercatorPolylineIteration(&next_data, polyline, forward, maxLength);
    if (forward){
        if (maxLength == -1)
            pIterFunction = NB_MercatorPolylineForwardIteration;
        else
            pIterFunction = NB_MercatorPolylineForwardLengthIteration;
    } else {
        if (maxLength == -1)
            pIterFunction = NB_MercatorPolylineReverseIteration;
        else
            pIterFunction = NB_MercatorPolylineReverseLengthIteration;
    }
    ts_fg.npoints = polyline_to_merc_multi_tristrip_beveled( pIterFunction, &next_data, ts_fg.points,
            ts_fg.npoints, fgRoadWidth, &endPoint, &endHeading);

    // Setup foreground/background tristrip pointer address
    fgStrip = (NB_MercatorTriStrip*)nsl_malloc(sizeof(NB_MercatorTriStrip) + (sizeof(NB_MercatorPoint) * size));
    if (fgStrip == NULL)
    {
        error = NE_NOMEM;
        goto error;
    }

    fgStrip->count = ts_fg.npoints;

    bgStrip = (NB_MercatorTriStrip*)nsl_malloc(sizeof(NB_MercatorTriStrip) + (sizeof(NB_MercatorPoint) * size));
    if (bgStrip == NULL)
    {
        error = NE_NOMEM;
        goto error;
    }

    bgStrip->count = ts_bg.npoints;

    // Setup foreground tristrip points array
    ptr = (byte*)fgStrip;
    ptr += sizeof(NB_MercatorTriStrip);
    fgStrip->points = (NB_MercatorPoint*)ptr;

    // Setup background tristrip points array
    ptr = (byte*)bgStrip;
    ptr += sizeof(NB_MercatorTriStrip);
    bgStrip->points = (NB_MercatorPoint*)ptr;

    for (i = 0; i < ts_fg.npoints; i++)
    {
        fgStrip->points[i].mx = ts_fg.points[i].x;
        fgStrip->points[i].my = ts_fg.points[i].y;

        bgStrip->points[i].mx = ts_bg.points[i].x;
        bgStrip->points[i].my = ts_bg.points[i].y;
    }

    nsl_free(ts_fg.points);
    nsl_free(ts_bg.points);

    newTriStrip->triStripForeground = fgStrip;
    newTriStrip->triStripBackground = bgStrip;
    *triStrip = newTriStrip;

    if (lastPoint != NULL)
    {
        lastPoint->mx = endPoint.x;
        lastPoint->my = endPoint.y;
    }

    if (lastHeading != NULL)
    {
        *lastHeading = endHeading;
    }

    return NE_OK;

error:
    if (ts_fg.points != NULL)
    {
        nsl_free(ts_fg.points);
    }

    if (ts_bg.points != NULL)
    {
        nsl_free(ts_bg.points);
    }

    if (fgStrip != NULL)
    {
        nsl_free(fgStrip);
    }

    if (bgStrip != NULL)
    {
        nsl_free(bgStrip);
    }

    if (newTriStrip)
    {
        nsl_free(newTriStrip);
    }

    return error;
}

/* See header file for description. */
NB_DEF NB_Error
NB_TriStripDestroy(
    NB_TriStrip* triStrip)
{
    if (triStrip == NULL)
    {
        return NE_INVAL;
    }

    if (triStrip->triStripBackground)
    {
        nsl_free(triStrip->triStripBackground);
        triStrip->triStripBackground = NULL;
    }

    if (triStrip->triStripForeground)
    {
        nsl_free(triStrip->triStripForeground);
        triStrip->triStripForeground = NULL;
    }

    nsl_free(triStrip);
    triStrip = NULL;

    return NE_OK;
}



// Private functions .............................................................................

NB_DEF NB_Error
NB_RoadToMercatorMultiTriStrip(
    int xTileIndex,
    int yTileIndex,
    int tileZoomLevel,
    data_road* proad,
    NB_MercatorTriStrip* fgStripStart,
    NB_MercatorTriStrip* bgStripStart)
{
    NB_Error error = NE_OK;
    int i = 0;
    int totalLines = 0;
    int line = 0;
    NB_PackedPolylineForwardIterationData next_data = {0};
    int8 bgw = 0;
    int8 fgw = 0;
    double realbgw = 0;
    double realfgw = 0;
    struct tristrip ts_bg = {0}; 
    struct tristrip ts_fg = {0};
    NB_MercatorTriStrip* fgStrip = 0;
    NB_MercatorTriStrip* bgStrip = 0;
    byte* ptrFG = 0;
    byte* ptrBG = 0;
    NB_MercatorTriStrip* tsFG = 0;
    NB_MercatorTriStrip* tsBG = 0;
    nb_boolean foregroundOnly = FALSE;

#ifdef COLLECT_RENDER_STATS
    uint32 starttime;
#endif

#ifdef COLLECT_RENDER_STATS
    starttime = GETUPTIMEMS();
#endif

    if (fgStripStart == NULL ||
        proad == NULL)
    {
        return NE_INVAL;
    }
    if (bgStripStart == NULL)
    {
        foregroundOnly = TRUE;
    }
    // Convert the polyline_packs into foreground/background tristrips if not done yet
    if (proad->vec_polyline_pack != NULL)
    {
        if (!foregroundOnly)
        {
            bgw		= GetRoadWidth((NB_VectorTileRoadType)proad->type); // :TRICKY: The enumerations are assumed to be identical!
            fgw		= bgw - 2;
        }
        else
        {
            //single pass vector tile
            //only the foreground data of the road is needed
            fgw = GetRoadWidth((NB_VectorTileRoadType)proad->type);   // :TRICKY: The enumerations are assumed to be identical!
        }

        NB_SpatialMeterLengthToMercatorProjectionByTileIndex(xTileIndex, yTileIndex, tileZoomLevel, fgw, &realfgw);
        NB_SpatialMeterLengthToMercatorProjectionByTileIndex(xTileIndex, yTileIndex, tileZoomLevel, bgw, &realbgw);

        totalLines = CSL_VectorGetLength(proad->vec_polyline_pack);

        // Setup tristrip pointers
        ptrFG = (byte*)fgStripStart;
        fgStrip = (NB_MercatorTriStrip*)ptrFG;
        ptrFG += sizeof(NB_MercatorTriStrip) * totalLines;

        if (!foregroundOnly)
        {
            ptrBG = (byte*)bgStripStart;
            bgStrip = (NB_MercatorTriStrip*)ptrBG;
            ptrBG += sizeof(NB_MercatorTriStrip) * totalLines;
        }

        for (line = 0; line < totalLines; line++) 
        {
            NB_PointIterationInitializePackedPolylineForwardIteration(&next_data, CSL_VectorGetPointer(proad->vec_polyline_pack, line), xTileIndex, yTileIndex, tileZoomLevel);

            if (!foregroundOnly)
            {
                ts_bg.npoints = next_data.packedPolyline->numsegments * NB_TRISTRIP_POINTS_PER_SEGMENT + NB_TRISTRIP_POINTS_ENDCAP;
                ts_bg.points = nsl_malloc(sizeof(NB_Vector)*ts_bg.npoints);
                ts_bg.fg = FALSE;
                if(ts_bg.points == NULL)
                {
                    error = NE_NOMEM;
                    goto error;
                }
                nsl_memset(ts_bg.points, 0, sizeof(NB_Vector)*ts_bg.npoints);
            }

            ts_fg.npoints = next_data.packedPolyline->numsegments * NB_TRISTRIP_POINTS_PER_SEGMENT + NB_TRISTRIP_POINTS_ENDCAP;
            ts_fg.points = nsl_malloc(sizeof(NB_Vector)*ts_fg.npoints);
            ts_fg.fg = TRUE;

            if (ts_fg.points == NULL) 
            {
                error = NE_NOMEM;
                goto error;
            }
            nsl_memset(ts_fg.points, 0, sizeof(NB_Vector)*ts_fg.npoints);

            ts_fg.npoints = polyline_to_merc_multi_tristrip(NB_PackedPolylineForwardIteration, &next_data,
                ts_bg.points, ts_fg.points, ts_fg.npoints, realbgw, realfgw, -1, NULL, NULL);

            tsFG = fgStrip + line;
            tsFG->count = ts_fg.npoints;
            tsFG->points = (NB_MercatorPoint*)ptrFG;

            for (i = 0; i < ts_fg.npoints; i++)
            {
                tsFG->points[i].mx = ts_fg.points[i].x;
                tsFG->points[i].my = ts_fg.points[i].y;
            }

            ptrFG += sizeof(NB_MercatorPoint) * (tsFG->count+1);
            nsl_free(ts_fg.points);

            if (!foregroundOnly)
            {
                ts_bg.npoints = ts_fg.npoints;

                tsBG = bgStrip + line;
                tsBG->count = ts_bg.npoints;
                tsBG->points = (NB_MercatorPoint*)ptrBG;

                for (i = 0; i < ts_bg.npoints; i++)
                {
                    tsBG->points[i].mx = ts_bg.points[i].x;
                    tsBG->points[i].my = ts_bg.points[i].y;
                }
                ptrBG += sizeof(NB_MercatorPoint) * (tsBG->count+1);
                nsl_free(ts_bg.points);
            }

#ifdef ENABLE_QA_LOGGING
            //pdata->pcfg->profiledata.num_polyline_points_to_tristrip += next_data.npoly;
#endif
        }
    }

#ifdef COLLECT_RENDER_STATS
    //pdata->pcfg->polyline_to_tristrip_time = pdata->pcfg->polyline_to_tristrip_time + (GETUPTIMEMS() - starttime);
#endif

    return NE_OK;

error:
    if (ts_fg.points != NULL)
    {
        nsl_free(ts_fg.points);
    }

    if (ts_bg.points != NULL)
    {
        nsl_free(ts_bg.points);
    }

    return error;
}



static int
append_mercator_end_cap(
NB_Vector* p1, 
NB_Vector* p2, 
NB_Vector* n1l, 
NB_Vector* n1r, 
    double offset, 
    double fraction, 
NB_Vector* ptristrip)
{
    NB_Vector d, nl, nr;
    int ts_size = 0;

    vector_subtract(&d, p1, p2);
    vector_normalize_to_length(&d, &d, offset);

    vector_subtract(&nl, n1l, p1);
    vector_multiply(&nl, &nl, 0.5);
    vector_add(&nl, &d, &nl);
    vector_add(&nl, p1, &nl);

    vector_subtract(&nr, n1r, p1);
    vector_multiply(&nr, &nr, fraction);
    vector_add(&nr, &d, &nr);
    vector_add(&nr, p1, &nr);

    // right normal
    //mercator_to_3d(&pcfg->ctx, nr.x, nr.y, &ptristrip[ts_size].x, &ptristrip[ts_size].y, &ptristrip[ts_size].z);
    ptristrip[ts_size] = nr;
    ts_size++;

    // left normal
    //mercator_to_3d(&pcfg->ctx, nl.x, nl.y, &ptristrip[ts_size].x, &ptristrip[ts_size].y, &ptristrip[ts_size].z);
    ptristrip[ts_size] = nl;
    ts_size++;

    return ts_size;
}

static int
polyline_to_merc_multi_tristrip(
    NB_PointIterationNextPointFunction nextfunc, 
    void* nextuser, 
    NB_Vector* ptristripbg, 
    NB_Vector* ptristripfg, 
    int ntristrip, 
    double bgwidth,
    double fgwidth, 
    double maxlength, 
    NB_Vector* last_pt, 
    NB_Vector* last_dir)
{
    NB_Vector pt = {0};
    NB_Vector seg_window[2] = { { 0 } };
    NB_Vector bg_n1l, bg_n1r, bg_n2l, bg_n2r, bg_lnr = {0}, bg_lnl = {0}, lp;
    NB_Vector fg_n1l, fg_n1r, fg_n2l, fg_n2r, fg_lnr = {0}, fg_lnl = {0};

    int seg_window_size = 0;
    int seg_processed = 0;
    int ts_size = 0;
    double length_remain = maxlength;

    double bg_normal_len = bgwidth/2.0;
    double fg_normal_len = fgwidth/2.0;

    nb_boolean foregroundOnly = FALSE;

    if (nextfunc == NULL)
        goto error;
    if (ptristripbg == NULL)
    {
        foregroundOnly = TRUE;
    }
    // for each segment of the road's polyline
    while (nextfunc(nextuser, &pt))
    {
        // advance the segment window
        seg_window[1] = seg_window[0];
        seg_window[0] = pt;
        seg_window_size++;

        // we need a history of at least two points to proceed.
        if (seg_window_size < 2)
            continue;

        calculate_segment_normals(&seg_window[1], &seg_window[0], fg_normal_len, &fg_n1l, &fg_n1r, &fg_n2l, &fg_n2r, 
            NULL, &lp, last_dir, bg_normal_len, &bg_n1l, &bg_n1r, &bg_n2l, &bg_n2r); 

        if (last_pt)
            *last_pt = lp;

        if (seg_processed > 0) {

            vector_add(&bg_n1r, &bg_lnr, &bg_n1r);
            vector_multiply(&bg_n1r, &bg_n1r, 0.5);

            vector_add(&bg_n1l, &bg_lnl, &bg_n1l);
            vector_multiply(&bg_n1l, &bg_n1l, 0.5);

            vector_add(&fg_n1r, &fg_lnr, &fg_n1r);
            vector_multiply(&fg_n1r, &fg_n1r, 0.5);

            vector_add(&fg_n1l, &fg_lnl, &fg_n1l);
            vector_multiply(&fg_n1l, &fg_n1l, 0.5);
        }

        if (seg_processed == 0) {

            if ((ts_size+2) >= ntristrip) {

                // Insufficient room in triangle strip buffer
                nsl_assert((FALSE));
                goto error;
            }

            if (!foregroundOnly)
            {
                append_mercator_end_cap(&seg_window[1], &seg_window[0], &bg_n1l, &bg_n1r, bg_normal_len, 0.5, ptristripbg);
            }
            ts_size += append_mercator_end_cap(&seg_window[1], &seg_window[0], &fg_n1l, &fg_n1r, fg_normal_len, 0.5, ptristripfg);
        }

        if ((ts_size+2) >= ntristrip) {

            // Insufficient room in triangle strip buffer
            nsl_assert((FALSE));
            goto error;
        }

        // right normal
        if (!foregroundOnly)
        {
            ptristripbg[ts_size] = bg_n1r;
        }
        ptristripfg[ts_size] = fg_n1r;
        ts_size++;

        // left normal
        if (!foregroundOnly)
        {
            ptristripbg[ts_size] = bg_n1l;
        }
        ptristripfg[ts_size] = fg_n1l;
        ts_size++;

        seg_processed++;
        bg_lnl = bg_n2l;
        bg_lnr = bg_n2r;
        fg_lnl = fg_n2l;
        fg_lnr = fg_n2r;

        if (maxlength >= 0 && length_remain <= 0)
            break;
    }

    if (seg_processed > 0) {

        if ((ts_size+2) >= ntristrip)
            goto error;

        // right normal
        if (!foregroundOnly)
        {
            ptristripbg[ts_size] = bg_lnr;
        }
        ptristripfg[ts_size] = fg_lnr;
        ts_size++;

        // left normal
        if (!foregroundOnly)
        {
            ptristripbg[ts_size] = bg_lnl;
        }
        ptristripfg[ts_size] = fg_lnl;
        ts_size++;

        if ((ts_size+2) >= ntristrip)
            goto error;

        if (!foregroundOnly)
        {
            append_mercator_end_cap(&lp, &seg_window[1], &bg_lnl, &bg_lnr, bg_normal_len, 0.5, ptristripbg+ts_size);
        }
        ts_size += append_mercator_end_cap(&lp, &seg_window[1], &fg_lnl, &fg_lnr, fg_normal_len, 0.5, ptristripfg+ts_size);
    }

    return ts_size;

error:

    return -1;
}


static int
polyline_to_merc_multi_tristrip_beveled(
        NB_PointIterationNextPointFunction nextfunc,
        void* nextuser,
        NB_Vector* ptristrip,
        int ntristrip,
        double lineWidth,
        NB_Vector* last_pt,
        NB_Vector* last_dir)
{
    NB_Vector	p1, p2, p3, normal, line, normalWidth, normalHalfWidth, pt, endPt, normalEnd, unitZ,
    nextNormal, nextNormalWidth, nextNormalHalfWidth,
    left1, left2, right1, right2, u, l, ulnormal, ulnormalWidth;
    double		epsilon = 1.E-9, limitMiter = 2*lineWidth;
    int			ts_size = 0;

    unitZ.x = 0.;
    unitZ.y = 0.;
    unitZ.z = 1.;

    if( !nextfunc(nextuser, &p1) ||
            !nextfunc(nextuser, &p2) )
        goto error;
    vector_subtract( &line, &p2, &p1 );
    vector_cross_product( &normal, &line, &unitZ );
    vector_normalize_to_length( &normalWidth, &normal, lineWidth );
    vector_multiply( &normalHalfWidth, &normalWidth, 0.5 );

    // Starting endcaps
    if ((ts_size+NB_TRISTRIP_POINTS_ENDCAP_BEVELED) >= ntristrip) {
        // Insufficient room in triangle strip buffer
        nsl_assert((FALSE));
        goto error;
    }
    vector_normalize_to_length( &line, &line, lineWidth*0.5 );
    vector_subtract( &endPt, &p1, &line );
    vector_normalize_to_length( &normalEnd, &normal, lineWidth*0.25 );

    vector_subtract( &pt, &endPt, &normalEnd );
    ptristrip[ts_size] = pt;
    ts_size++;

    vector_add( &pt, &endPt, &normalEnd );
    ptristrip[ts_size] = pt;
    ts_size++;

    vector_subtract( &pt, &p1, &normalHalfWidth );
    ptristrip[ts_size] = pt;
    ts_size++;

    vector_add( &pt, &p1, &normalHalfWidth );
    ptristrip[ts_size] = pt;
    ts_size++;

    // Beveled line
    p3 = p2;
    p2 = p1;
    nextNormal = normal;
    nextNormalWidth = normalWidth;
    nextNormalHalfWidth = normalHalfWidth;

    while (nextfunc(nextuser, &pt)){

        // Discard duplicate Mercator points
        if (pt.x == p3.x && pt.y == p3.y) {
            continue;
        }
        p1 = p2;
        p2 = p3;
        p3 = pt;
        normal = nextNormal;
        normalWidth = nextNormalWidth;
        normalHalfWidth = nextNormalHalfWidth;
        vector_subtract( &line, &p3, &p2 );
        vector_cross_product( &nextNormal, &line, &unitZ );
        vector_normalize_to_length( &nextNormalWidth, &nextNormal, lineWidth );
        vector_multiply( &nextNormalHalfWidth, &nextNormalWidth, 0.5 );
        if ((ts_size+2) >= ntristrip) {
            // Insufficient room in triangle strip buffer
            nsl_assert((FALSE));
            goto error;
        }

        // Get intersection points for bevel (u == upper, l = lower)
        vector_add( &left1, &p1, &normalHalfWidth );
        vector_add( &left2, &p2, &normalHalfWidth );
        vector_add( &right1, &p2, &nextNormalHalfWidth );
        vector_add( &right2, &p3, &nextNormalHalfWidth );
        if( !vector_intercept( &u, &left1, &left2, &right1, &right2 ) ||
                !vector_points_within( &u, &left2, limitMiter ) ){
            // Parallel lines, so no bevel needed
            vector_subtract( &pt, &p2, &normalHalfWidth );
            //AddVertexP2Ck( vdata, &entry->dispVertexes.vtxCount, pt.x, pt.y, sizeOfvdata, tileRect );
            ptristrip[ts_size] = pt;
            ts_size++;
            vector_add( &pt, &p2, &normalHalfWidth );
            //AddVertexP2Ck( vdata, &entry->dispVertexes.vtxCount, pt.x, pt.y, sizeOfvdata, tileRect );
            ptristrip[ts_size] = pt;
            ts_size++;
            continue;
        }
        vector_subtract( &left1, &p1, &normalHalfWidth );
        vector_subtract( &left2, &p2, &normalHalfWidth );
        vector_subtract( &right1, &p2, &nextNormalHalfWidth );
        vector_subtract( &right2, &p3, &nextNormalHalfWidth );
        if( !vector_intercept( &l, &left1, &left2, &right1, &right2 ) ||
                !vector_points_within( &u, &left2, limitMiter )  ){
            // Parallel lines, so no miter needed
            vector_subtract( &pt, &p2, &normalHalfWidth );
            //AddVertexP2Ck( vdata, &entry->dispVertexes.vtxCount, pt.x, pt.y, sizeOfvdata, tileRect );
            ptristrip[ts_size] = pt;
            ts_size++;
            vector_add( &pt, &p2, &normalHalfWidth );
            //AddVertexP2Ck( vdata, &entry->dispVertexes.vtxCount, pt.x, pt.y, sizeOfvdata, tileRect );
            ptristrip[ts_size] = pt;
            ts_size++;
            continue;
        }
        vector_subtract( &ulnormal, &u, &l );
        vector_normalize_to_length( &ulnormalWidth, &ulnormal, lineWidth );

        if ((ts_size+6) >= ntristrip) {
            // Insufficient room in triangle strip buffer
            nsl_assert((FALSE));
            goto error;
        }
        if( vector_point_in_segment( &l, &left1, &left2, epsilon ) ){
            // Bevel is on the upper side because the intercept point for the lower edges is inside the edges
            //AddVertexP2Ck( vdata, &entry->dispVertexes.vtxCount, l.x, l.y, sizeOfvdata, tileRect );	// lower
            ptristrip[ts_size] = l;
            ts_size++;
            vector_add( &pt, &l, &normalWidth );
            //AddVertexP2Ck( vdata, &entry->dispVertexes.vtxCount, pt.x, pt.y, sizeOfvdata, tileRect );	// lower + normalWidth
            ptristrip[ts_size] = pt;
            ts_size++;
            //AddVertexP2Ck( vdata, &entry->dispVertexes.vtxCount, l.x, l.y, sizeOfvdata, tileRect );	// lower
            ptristrip[ts_size] = l;
            ts_size++;
            vector_add( &pt, &l, &ulnormalWidth );
            //AddVertexP2Ck( vdata, &entry->dispVertexes.vtxCount, pt.x, pt.y, sizeOfvdata, tileRect );	// lower + ulnormalWidth
            ptristrip[ts_size] = pt;
            ts_size++;
            //AddVertexP2Ck( vdata, &entry->dispVertexes.vtxCount, l.x, l.y, sizeOfvdata, tileRect );	// lower
            ptristrip[ts_size] = l;
            ts_size++;
            vector_add( &pt, &l, &nextNormalWidth );
            //AddVertexP2Ck( vdata, &entry->dispVertexes.vtxCount, pt.x, pt.y, sizeOfvdata, tileRect );	// lower + nextNormalWidth
            ptristrip[ts_size] = pt;
            ts_size++;

        } else {
            // Bevel is on the lower side
            vector_subtract( &pt, &u, &normalWidth );
            //AddVertexP2Ck( vdata, &entry->dispVertexes.vtxCount, pt.x, pt.y, sizeOfvdata, tileRect );	// upper - normalWidth
            ptristrip[ts_size] = pt;
            ts_size++;
            //AddVertexP2Ck( vdata, &entry->dispVertexes.vtxCount, u.x, u.y, sizeOfvdata, tileRect );	// upper
            ptristrip[ts_size] = u;
            ts_size++;
            vector_subtract( &pt, &u, &ulnormalWidth );
            //AddVertexP2Ck( vdata, &entry->dispVertexes.vtxCount, pt.x, pt.y, sizeOfvdata, tileRect );	// upper - ulnormalWidth
            ptristrip[ts_size] = pt;
            ts_size++;
            //AddVertexP2Ck( vdata, &entry->dispVertexes.vtxCount, u.x, u.y, sizeOfvdata, tileRect );	// upper
            ptristrip[ts_size] = u;
            ts_size++;
            vector_subtract( &pt, &u, &nextNormalWidth );
            //AddVertexP2Ck( vdata, &entry->dispVertexes.vtxCount, pt.x, pt.y, sizeOfvdata, tileRect );	// upper - nextNormalWidth
            ptristrip[ts_size] = pt;
            ts_size++;
            //AddVertexP2Ck( vdata, &entry->dispVertexes.vtxCount, u.x, u.y, sizeOfvdata, tileRect );	// upper
            ptristrip[ts_size] = u;
            ts_size++;
        }
    }

    // Ending endcaps
    if ((ts_size+NB_TRISTRIP_POINTS_ENDCAP_BEVELED) >= ntristrip) {
        // Insufficient room in triangle strip buffer
        nsl_assert((FALSE));
        goto error;
    }
    vector_normalize_to_length( &line, &line, lineWidth*0.5 );
    vector_add( &endPt, &p3, &line );
    vector_normalize_to_length( &normalEnd, &nextNormal, lineWidth*0.25 );

    vector_subtract( &pt, &p3, &nextNormalHalfWidth );
    ptristrip[ts_size] = pt;
    ts_size++;

    vector_add( &pt, &p3, &nextNormalHalfWidth );
    ptristrip[ts_size] = pt;
    ts_size++;

    vector_subtract( &pt, &endPt, &normalEnd );
    ptristrip[ts_size] = pt;
    ts_size++;

    vector_add( &pt, &endPt, &normalEnd );
    ptristrip[ts_size] = pt;
    ts_size++;

    // Set lastPoint, lastHeading
    if( last_pt )
        *last_pt = p3;
    if( last_dir ){
        vector_normalize( &line, &line );
        *last_dir = line;
    }

    return ts_size;

error:
    return -1;
}



int8 GetRoadWidth(
                  NB_VectorTileRoadType roadType)
{
    switch (roadType)
    {
        /* Unassigned Road Format */
    case NB_VMRT_None:
        return 4;
        break;

        /* Arterial Roads */
    case NB_VMRT_Arterial:
        return 6;
        break;

        /* Limited Access Roads */
    case NB_VMRT_Limited_access:
        return 10;
        break;

        /* Priority 2 Roads and all others */
    case NB_VMRT_Local:
    case NB_VMRT_Ramp:
    case NB_VMRT_Rotary:
    case NB_VMRT_Terminal:
    case NB_VMRT_Bridge:
    case NB_VMRT_Tunnel:
    default:
        return 6;
        break;
    }
}

/*! @} */


NB_Error
MercatorPolylineToPolylinePack(
    NB_MercatorPolyline* polyline,
    NB_VectorTileIndex tileIndex,
    int zoomLevel, 
    data_polyline_pack** ppack)
{
    data_polyline_pack* dpp = 0;
    uint32 i = 0;
    int alloc_size = 0;
    byte* ptr = 0;
    plp_segment* pseg = 0;

    if (ppack == NULL)
    {
        return NE_NOMEM;
    }

    alloc_size = sizeof(data_polyline_pack) + (sizeof(plp_segment) * polyline->count);
    ptr = nsl_malloc(alloc_size);
    dpp = (data_polyline_pack*)ptr;
    if (dpp == NULL)
    {
        return NE_NOMEM;
    }
    nsl_memset(dpp, 0, alloc_size);

    dpp->numsegments = polyline->count;

    ptr += sizeof(data_polyline_pack);
    dpp->segments = (plp_segment*)ptr;
    pseg = dpp->segments;
    for (i = 0; i < dpp->numsegments; i++, pseg++)
    {       
        NB_SpatialConvertMercatorToTileOffset(
                tileIndex.x, 
                tileIndex.y, 
                zoomLevel, 
                polyline->points[i].mx, 
                polyline->points[i].my, 
                &pseg->x, 
                &pseg->y);
    }

    *ppack = dpp;
   
    return NE_OK;
}

NB_Error
GetRoadWidthByTileIndex(
    NB_VectorTileRoadType roadType,
    NB_VectorTileIndex tileIndex,
    int zoomLevel,
    double* fgRoadWidth, 
    double* bgRoadWidth)
{
    int8 fgWidth = 0;
    int8 bgWidth = 0;

    if (fgRoadWidth == NULL ||
        bgRoadWidth == NULL)
    {
        return NE_INVAL;
    }

    bgWidth = GetRoadWidth(roadType);
    fgWidth = bgWidth - 2;

    NB_SpatialMeterLengthToMercatorProjectionByTileIndex(tileIndex.x, tileIndex.y, zoomLevel, bgWidth, bgRoadWidth);  
    NB_SpatialMeterLengthToMercatorProjectionByTileIndex(tileIndex.x, tileIndex.y, zoomLevel, fgWidth, fgRoadWidth);

    return NE_OK;
}


