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

    @file     nbvectormapprefetchprocessor.c
       
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

#include "paltypes.h"
#include "palmath.h"
#include "palclock.h"
#include "nbcontext.h"
#include "nbcontextprotected.h"
#include "nbvectormapprefetchprocessor.h"
#include "nbvectormapprefetchprocessorprivate.h"
#include "nbvectortilemanager.h"
#include "nbnavigation.h"
#include "nbnavigationprivate.h"
#include "nbnavigationstate.h"
#include "nbrouteinformationprivate.h"
#include "navtypes.h"
#include "nbspatial.h"
#include "nbutility.h"
#include "navutil.h"
#include "nbnavigationstateprivate.h"
#include "spatialvector.h"
#include "nbpointiteration.h"
#include "nbqalog.h"
#include "transformutility.h"

static NB_Error RequestAndLockTile(NB_VectorMapPrefetchState* vectorMapPrefetchState, NB_VectorTileIndex* index);
static NB_Error UnlockAllTiles(NB_VectorMapPrefetchState* vectorMapPrefetchState);

NB_DEF NB_Error
NB_VectorMapPrefetchProcessorStateCreate(NB_Context* context, NB_VectorMapPrefetchConfiguration* config, NB_VectorTileManager* manager, NB_VectorMapPrefetchState** vectorMapPrefetchState)
{
    NB_VectorMapPrefetchState* pThis = NULL;
    
    if (!context)
    {
        return NE_INVAL;
    }
    
    if ((pThis = nsl_malloc(sizeof(*pThis))) == NULL)
    {
        return NE_NOMEM;
    }

    nsl_memset(pThis, 0, sizeof(*pThis));

    pThis->context = context;
    pThis->lockedTiles = CSL_DictionaryAlloc(128);
    pThis->nextPrefetchGPSTime = 0;
    pThis->referenceCosLatitudeRadius = RADIUS_EARTH_METERS;
    pThis->manager = manager;

    if (pThis->lockedTiles == NULL)
    {
        NB_VectorMapPrefetchProcessorStateDestroy(pThis);
        return NE_NOMEM;
    }

    nsl_memcpy(&pThis->config, config, sizeof(*config));

    *vectorMapPrefetchState = pThis;

    return NE_OK;
}

NB_DEF NB_Error
NB_VectorMapPrefetchProcessorStateDestroy(NB_VectorMapPrefetchState* vectorMapPrefetchState)
{
    if (!vectorMapPrefetchState)
    {
        return NE_INVAL;
    }

    (void)UnlockAllTiles(vectorMapPrefetchState);

    if (vectorMapPrefetchState->lockedTiles)
    {
        CSL_DictionaryDealloc(vectorMapPrefetchState->lockedTiles);
        vectorMapPrefetchState->lockedTiles = NULL;
    }

    nsl_free(vectorMapPrefetchState);

    return NE_OK;
}

#ifdef ENABLE_QA_LOGGING
#ifdef ENABLE_QA_LOGGING_VERBOSE_DEBUG
static void
qalog_prefetch_point(struct vector_map_config* pcfg, double x, double y)
{
	double lat, lon;

	mercator_reverse(x, y, &lat, &lon);

	(void)dbglog_prefetch_point(gpstime(), lat, lon, dbgwriter_file, getdbglog(pcfg->pThis->vtm->pappdata));
#ifdef ENABLE_QA_NETWORK
	(void)dbglog_prefetch_point(gpstime(), lat, lon, dbgwriter_net, getdbgnet(pcfg->pThis->vtm->pappdata));
#endif
}
#endif
#endif

#define PREFETCH_TOLERANCE 1.2

static NB_Error
NB_VectorMapPrefetchSlidingSegment(NB_VectorMapPrefetchState* prefetchState, const NB_Vector *p1, const NB_Vector *p2, double seg_length_meters)
{
	NB_Error error = NE_OK;
	NB_Vector segment, pstart, pend, p1l, p1r, p2l, p2r, temp;
	double seg_length = 0;		// Length of segment to be calculated in Mercator
	double seg_ext_length = 0;	// Distance beyond segment end to set end of range, in Mercator
	double seg_prefetch_window_width = 0;
	int n;
	int tz = prefetchState->config.tz;
	int tx, ty;
	NB_Vector pt_buffer1[20];
	NB_Vector pt_buffer2[20];
	int nvertices, retval;
	int mintx, ntile, maxtx, minty, maxty;
	struct ortho_clip_data ocdata;
	int num_unlocked_slots = NB_VectorTileManagerGetNumberOfUnlockedSlots(prefetchState->manager);
    int num_pending_slots = NB_VectorTileManagerGetNumberOfPendingSlots(prefetchState->manager);

	vector_subtract(&segment, p2, p1);

	// Can't prefetch if the segment is zero length
	if (vector_magnitude(&segment) == 0)
		return NE_OK;

	// If no length is passed in, do whole segment
	if (seg_length_meters == -1)
		seg_length_meters = vector_distance(p1, p2);

    seg_length = NB_SpatialMetersToMercator(seg_length_meters, prefetchState->referenceCosLatitudeRadius);
    seg_ext_length = NB_SpatialMetersToMercator(prefetchState->config.segmentExtensionLengthMeters, prefetchState->referenceCosLatitudeRadius);
    seg_prefetch_window_width = NB_SpatialMetersToMercator(prefetchState->config.prefetchWindowWidthMeters * PREFETCH_TOLERANCE, prefetchState->referenceCosLatitudeRadius);

	pstart = *p1;
	vector_normalize_to_length(&temp, &segment, seg_ext_length);
	vector_add(&pend, p2, &temp);

	// Get the box around the segment
	calculate_segment_normals(&pstart, &pend, seg_prefetch_window_width/2.0, &p1l, &p1r, &p2l, &p2r, NULL, NULL, NULL, 0, NULL, NULL, NULL, NULL);

    if (CSL_QaLogIsVerboseLoggingEnabled(NB_ContextGetQaLog(prefetchState->context)))
	{
		double seg_lat1, seg_lon1, seg_lat2, seg_lon2;
		double box_lat1, box_lon1, box_lat2, box_lon2, box_lat3, box_lon3, box_lat4, box_lon4;

		NB_SpatialConvertLatLongToMercator(pstart.x, pstart.y, &seg_lat1, &seg_lon1);
		NB_SpatialConvertLatLongToMercator(pend.x, pend.y, &seg_lat2, &seg_lon2);

		NB_SpatialConvertMercatorToLatLong(p1l.x, p1l.y, &box_lat1, &box_lon1);
		NB_SpatialConvertMercatorToLatLong(p2l.x, p2l.y, &box_lat2, &box_lon2);
		NB_SpatialConvertMercatorToLatLong(p2r.x, p2r.y, &box_lat3, &box_lon3);
		NB_SpatialConvertMercatorToLatLong(p1r.x, p1r.y, &box_lat4, &box_lon4);

        NB_QaLogPrefetchArea(prefetchState->context, seg_lat1, seg_lon1, seg_lat2, seg_lon2, 
			box_lat1, box_lon1, box_lat2, box_lon2, box_lat3, box_lon3, box_lat4, box_lon4);
	}

	// Set up array of vertices for clipping (being sure to use convex ordering)
	pt_buffer1[0] = p1l;
	pt_buffer1[1] = p1r;
	pt_buffer1[2] = p2r;
	pt_buffer1[3] = p2l;
	nvertices = 4;

	// Clip against two planes
	mintx = maxtx = -1;
	// Find the x tile extents
	for (n = 0; n < nvertices; n++)
	{
		NB_SpatialConvertMercatorToTile(pt_buffer1[n].x, pt_buffer1[n].y, tz, &tx, NULL);
		mintx = mintx == -1 ? tx : MIN(tx, mintx);
		maxtx = maxtx == -1 ? tx : MAX(tx, maxtx);
	}

	for (ntile = mintx; ntile <= maxtx && num_unlocked_slots > 0; ntile++)
	{
		// Set up array of vertices for clipping (being sure to use convex ordering)
		pt_buffer1[0] = p1l;
		pt_buffer1[1] = p1r;
		pt_buffer1[2] = p2r;
		pt_buffer1[3] = p2l;
		nvertices = 4;

		// clip against the minimum size
		ocdata.clipflags = ORTHO_CLIP_MIN | ORTHO_CLIP_X;
		NB_SpatialConvertTileToMercator(ntile, 0, tz, 0, 0, &ocdata.val, NULL); // ty is a don't-care
		retval = sutherland_hodgman_polygon_clip(pt_buffer1, nvertices, pt_buffer2, 20, ortho_clip, ortho_intersect, &ocdata);
		if (retval < 2)
			continue;
		nvertices = retval;

		ocdata.clipflags = ORTHO_CLIP_MAX | ORTHO_CLIP_X;
		NB_SpatialConvertTileToMercator(ntile, 0, tz, 1.0, 0, &ocdata.val, NULL); // ty is a don't-care
		retval = sutherland_hodgman_polygon_clip(pt_buffer2, nvertices, pt_buffer1, 20, ortho_clip, ortho_intersect, &ocdata);
		if (retval < 2)
			continue;
		nvertices = retval;

		// Get the min and max y tile values of the resulting polygon
		minty = maxty = -1;
		for (n = 0; n < nvertices; n++)
		{
            if (CSL_QaLogIsVerboseLoggingEnabled(NB_ContextGetQaLog(prefetchState->context)))
	        {
                double latitude;
                double longitude;

                NB_SpatialConvertMercatorToLatLong(pt_buffer1[n].x, pt_buffer1[n].y, &latitude, &longitude);
                NB_QaLogPrefetchPoint(prefetchState->context, latitude, longitude);
            }

			NB_SpatialConvertMercatorToTile(pt_buffer1[n].x, pt_buffer1[n].y, tz, NULL, &ty);
			minty = minty == -1 ? ty : MIN(ty, minty);
			maxty = maxty == -1 ? ty : MAX(ty, maxty);
		}

		tx = ntile;
		for (ty = minty; ty <= maxty && num_unlocked_slots > 0 && num_pending_slots > 0 && error == NE_OK; ty++)
		{
            NB_VectorTileIndex tileIndex;

            tileIndex.x = tx;
            tileIndex.y = ty;

            error = RequestAndLockTile(prefetchState, &tileIndex);
			num_unlocked_slots = NB_VectorTileManagerGetNumberOfUnlockedSlots(prefetchState->manager);
            num_pending_slots = NB_VectorTileManagerGetNumberOfPendingSlots(prefetchState->manager);
		}
	}

	return error;
}

NB_DEF NB_Error 
NB_VectorMapPrefetchProcessorUpdate(NB_VectorMapPrefetchState* prefetchState, NB_NavigationState* navigationState, nb_boolean* tilesRequested)
{   
	NB_Error error = NE_OK;
    data_util_state* dataState = NB_ContextGetDataState(navigationState->context);
	uint32 valid = 0;
	uint32 pending = 0;
	uint32 downloading = 0;
	uint32 total = 0;
	uint32 discarded = 0;
	uint32 totaldata = 0;
	uint32 lastdata = 0;
	uint32 active = 0;
	data_nav_maneuver* pman = 0;
	double dist = 0.0;
	double prefetch_distance = prefetchState->config.minPrefetchDistance; // minimum # of meters of data to keep loaded

	uint32 start_man = navigationState->currentRoutePosition.closest_maneuver;
	uint32 start_seg = navigationState->currentRoutePosition.closest_segment;
	uint32 nman = NAV_MANEUVER_NONE;
	uint32 last_man = 0;

	NB_PolylineForwardLengthIterationData next_len_data;
	NB_Vector startpt = { 0 };
	NB_Vector endpt = { 0 };
    
    nb_gpsTime currentGPSTime = PAL_ClockGetGPSTime();

    if (prefetchState->manager == NULL)
    {
        return NE_OK; // no prefetching if there is no vector tile manager
    }

	if (start_man == NAV_MANEUVER_NONE)
    {
		return NE_OK; // no prefetching if we're not on the route
    }

    /* Schedule the first prefetch */
    if (prefetchState->nextPrefetchGPSTime == 0)
    {
        prefetchState->nextPrefetchGPSTime = currentGPSTime + prefetchState->config.secondsBeforeFirstPrefetch;
        return NE_OK;
    }
    else if (currentGPSTime < prefetchState->nextPrefetchGPSTime)
    {
        /* Not time to prefetch */
        return NE_OK;
    }
    else
    {
        prefetchState->nextPrefetchGPSTime = currentGPSTime + prefetchState->config.secondsBetweenPrefetch;
    }

    prefetchState->referenceCosLatitudeRadius = nsl_cos(TO_RAD(navigationState->currentRoutePosition.proj_lat)) * RADIUS_EARTH_METERS;

	// remove lock on tiles
    UnlockAllTiles(prefetchState);

    NB_VectorTileManagerGetStatistics(prefetchState->manager, &total, &valid, &pending, &downloading, &discarded, &totaldata, &lastdata, &active, NULL, NULL);
    NB_VectorTileManagerSetMaximumPendingTilesOverride(prefetchState->manager, prefetchState->config.numberOfPrefetchTiles);

	last_man = NB_RouteInformationGetManeuverCount(navigationState->navRoute);

	nman = start_man;

	do
	{
		pman = NB_RouteInformationGetManeuver(navigationState->navRoute, nman);

		// Can't prefetch anymore if we don't have a polyline for this maneuver
		if (!data_nav_maneuver_polyline_complete(dataState, pman))
		{
			return NE_OK;
        }
        
        NB_PointIterationInitializePolylineForwardLengthIteration(&next_len_data, dataState, &pman->polyline, pman->polyline.length);

        // If the current maneuver is the starting maneuver, then advance the the starting segment
        // otherwise start at the first point.
		if (nman == start_man)
		{
			while (NB_PolylineForwardIteration(&next_len_data, &startpt))
			{
				if (next_len_data.index >= (int32)start_seg)
				{
					break;
			    }
		    }
		}
		else
		{
			// Just get the start point
			NB_PolylineForwardIteration(&next_len_data, &startpt);
		}

		if (dist < prefetch_distance)
        {
			next_len_data.length = prefetch_distance - dist;
        }
        else
        {
            next_len_data.length = 0;
        }


		// Prefetch for the current maneuver's segments.
		while (NB_PolylineForwardLengthIteration(&next_len_data, &endpt) && pending < prefetchState->config.numberOfPrefetchTiles && NB_VectorTileManagerGetNumberOfUnlockedSlots(prefetchState->manager) > 0)
		{
			// Prefetch for this segment.
			error = NB_VectorMapPrefetchSlidingSegment(prefetchState, &startpt, &endpt, -1);
			
			// Update pending tile count.
			NB_VectorTileManagerGetStatistics(prefetchState->manager, &total, &valid, &pending, &downloading, &discarded, &totaldata, &lastdata, &active, NULL, NULL);
			
			// Add the length of the the segment
			dist = dist + next_len_data.lastLength;
			
			// Update start point with last end point
			startpt = endpt;
			
			// If pending, we no longer care about stopping at length; make sure we iterate through all the way
			if (pending)
			{
				next_len_data.length = pman->polyline.length;
            }
            
            if (error != NE_OK)
            {
                break;
            }
		}

		nman++;
		// Loop through remaining maneuvers until preset distance if none pending or until we have a full batch of prefetch tiles
	} while (
				(
					(pending > 0 && pending < prefetchState->config.numberOfPrefetchTiles) || 
					(pending == 0 && dist < prefetch_distance)
				) && 
				(
					(nman < last_man && error == NE_OK)
				) &&
				(
					(NB_VectorTileManagerGetNumberOfUnlockedSlots(prefetchState->manager) > 0)
				)
			);

    if (pending && error == NE_OK)
    {
        error = NB_VectorTileManagerDownloadPendingTiles(prefetchState->manager);

        // if no download handlers available, try again later
        if (error == NE_BUSY)
        {
            pending = 0;
            error = NE_OK;
            NB_QaLogAppState(navigationState->context, "Vector tile download handlers busy -- retry later");
        }
    }

    if (tilesRequested)
    {
        *tilesRequested = (nb_boolean)(pending > 0);
    }
    
	NB_VectorTileManagerSetMaximumPendingTilesOverride(prefetchState->manager, -1);

	return error;
}

static void 
GetTileKey(int tx, int ty, int tz, char* key)
{
    nsl_sprintf(key, "%d,%d,%d", tx, ty, tz);
}

static nb_boolean
IsTileInLockedList(NB_VectorMapPrefetchState* vectorMapPrefetchState, int tx, int ty, int tz)
{
    char key[20];
    size_t valuesize = 0;

    GetTileKey(tx, ty, tz, key);

    return CSL_DictionaryGet(vectorMapPrefetchState->lockedTiles, key, &valuesize) != NULL ? TRUE : FALSE;
}

static NB_Error
AddTileToLockedList(NB_VectorMapPrefetchState* vectorMapPrefetchState, NB_VectorTile* tile)
{
    char key[20];

    GetTileKey(tile->x, tile->y, tile->zoomLevel, key);

    if (!CSL_DictionarySet(vectorMapPrefetchState->lockedTiles, key, (const char*) &tile, sizeof(tile)))
    {
        return NE_NOMEM;
    }

    return NE_OK;
}

static NB_Error 
RequestAndLockTile(NB_VectorMapPrefetchState* vectorMapPrefetchState, NB_VectorTileIndex* index)
{
    NB_Error error = NE_OK;
    NB_VectorTile* tile = NULL;

    if (IsTileInLockedList(vectorMapPrefetchState, index->x, index->y, vectorMapPrefetchState->config.tz))
    {
        /* We have already locked this tile */
        return NE_OK;
    }

    error = NB_VectorTileManagerGetTile(vectorMapPrefetchState->manager, index, &tile,  TRUE);

    if (error == NE_NOENT)
    {
        error = NE_OK;
    }

    if (tile != NULL)
    {
        error = AddTileToLockedList(vectorMapPrefetchState, tile);

        if (error != NE_OK)
        {
            /* We couldn't add the tile to our list, so release our lock */
            NB_VectorTileManagerUnlockTile(vectorMapPrefetchState->manager, tile);
        }
    }

    return error;
}

static int 
UnlockTileFunction(void* userdata, const char* key, const char *value, size_t valueLength)
{
    NB_VectorMapPrefetchState* vectorMapPrefetchState = userdata;
    NB_VectorTile* tile = 0;

    nsl_memcpy(&tile, value, sizeof(tile));

    NB_VectorTileManagerUnlockTile(vectorMapPrefetchState->manager, tile);

    return 0;
}

static NB_Error 
UnlockAllTiles(NB_VectorMapPrefetchState* vectorMapPrefetchState)
{
    if (vectorMapPrefetchState->lockedTiles == NULL)
    {
        return NE_INVAL;
    }

    CSL_DictionaryIteration(vectorMapPrefetchState->lockedTiles, UnlockTileFunction, vectorMapPrefetchState);

    return NE_OK;
}
