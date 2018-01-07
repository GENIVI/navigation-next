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

@file     nbvectortileinformationprivate.c
@defgroup nbvectortileinformationprivate nbvectortileinformationprivate.

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
#include "nbvectortileinformationprivate.h"
#include "nbspatial.h"
#include "nbvectortile.h"
#include "nbtristrip.h"
#include "nbtristripprotected.h"
#include "nbvectortilemanager.h"
#include "nbcontextaccess.h"

/*! @{ */

// Local Types ...................................................................................

typedef struct s_cursor 
{
    byte* sptr;  /* points to start of buffer, equals to vtm_buf */
    byte* cptr;  /* where the data is copied to, initally is the same as vtm_buf */
    byte* eptr;  /* points to end of buffer, equals to vtm_buf + vtm_size, this has to hold sptr <= cptr < eptr */

}s_cursor_t;


// Local Functions ...............................................................................

static NB_Error    s_vectile_alloc(VecTile_t* vt, int size);
static void        s_vectile_free(VecTile_t* vt);
static boolean     s_vectile_exist(VecTile_t* vt);

static int         s_vectile_size(VecTile_t* vt, data_vector_tile* dvt);
static int         s_vectile_road_size(VecTile_t* vt, struct CSL_Vector* roads);
static int         s_vectile_area_size(VecTile_t* vt, struct CSL_Vector* areas);
static int         s_vectile_strlen_with_padding(VecTile_t* vt, data_string* ds);

static NB_Error   s_vectile_copy(VecTile_t* vt, data_vector_tile* dvt);
static NB_Error   s_vectile_road_copy(VecTile_t* vt, struct CSL_Vector* roads, s_cursor_t* cursor);
static NB_Error   s_vectile_area_copy(VecTile_t* vt, struct CSL_Vector* areas, s_cursor_t* cursor);
static char*      s_vectile_strcpy(VecTile_t* vt, char* dst, data_string* src);




// Public functions ..............................................................................

VecTile_t* 
VectorTileCreate(NB_Context* context, int zoomLevel, data_vector_tile* pdvt, boolean takeover)
{
    NB_Error err = NE_OK;

    VecTile_t *vt = (VecTile_t*)nsl_malloc(sizeof(VecTile_t));
    if (!vt)
    {
        return NULL;
    }

    nsl_memset(vt, 0, sizeof(VecTile_t));
    vt->context = context;
    vt->ref_count = 1;
    vt->vectorTile.x = pdvt->tx;
    vt->vectorTile.y = pdvt->ty;
    vt->vectorTile.zoomLevel = zoomLevel;

    if (takeover) 
    {
        nsl_memcpy(&vt->data, pdvt, sizeof(data_vector_tile));
    } 
    else 
    {
        err = err ? err : data_vector_tile_init(NB_ContextGetDataState(context), &vt->data);
        err = err ? err : data_vector_tile_copy(NB_ContextGetDataState(context), &vt->data, pdvt);        
    }

    if (err) 
    {
        DEBUGLOG(LOG_SS_NB_MAP, LOG_SEV_MAJOR, ("Error during cache_add (%d)", err));
        nsl_free(vt);
        return NULL;
    }

    return vt;
}

NB_VectorTile*
VectorTileDecode(VecTile_t *vt)
{
    int size;

    if (vt == NULL)
    {
        return NULL;
    }

    if (s_vectile_exist(vt))
    {
        return &vt->vectorTile;
    }

    size = s_vectile_size(vt, &vt->data);

    DEBUGLOG(LOG_SS_NB_MAP, LOG_SEV_INFO, ("tile[%d,%d,%d] size = %d.", vt->vectorTile.x, vt->vectorTile.y, vt->vectorTile.zoomLevel, size));

    if (size == 0) // We have an empty tile
    {
      vt->vmt_buf = NULL;
      vt->vmt_size = 0;
    }
    else
    {
      if (s_vectile_alloc(vt, size) != NE_OK) 
      {
          DEBUGLOG(LOG_SS_NB_MAP, LOG_SEV_MAJOR, ("No memory available."));
          return NULL;        
      }

      if (s_vectile_copy(vt, &vt->data) != NE_OK) 
      {
          DEBUGLOG(LOG_SS_NB_MAP, LOG_SEV_MAJOR, ("Failed to copy tile data to NB_VectorTile."));
          s_vectile_free(vt);
          return NULL;
      }
    }
    return &vt->vectorTile;
}

void
VectorTileDestroy(NB_VectorTile* tile)
{
    VecTile_t* vt = (VecTile_t*)tile;
    s_vectile_free(vt);
    data_vector_tile_free(NB_ContextGetDataState(vt->context), &vt->data);
    nsl_free(vt);
}


// Private functions .............................................................................

static NB_Error    
s_vectile_alloc(VecTile_t* vt, int size) 
{
    vt->vmt_buf = (byte *) nsl_malloc(size);
    vt->vmt_size = size;

    if (vt->vmt_buf != NULL)
        nsl_memset(vt->vmt_buf, 0, size);

    return vt->vmt_buf ? NE_OK : NE_NOMEM;
}

static void
s_vectile_free(VecTile_t* vt)
{
    nsl_free(vt->vmt_buf);
    vt->vmt_buf = NULL;
    vt->vmt_size = 0;
}

static boolean
s_vectile_exist(VecTile_t* vt)
{
    return (vt->vmt_buf != NULL) ? TRUE : FALSE;
}

static int         
s_vectile_size(VecTile_t* vt, data_vector_tile* dvt)
{
    int totalsize = 0;

    totalsize += s_vectile_area_size(vt, dvt->vec_areas);

    totalsize += s_vectile_road_size(vt, dvt->vec_roads);

    return totalsize;
}

static int         
s_vectile_road_size(VecTile_t* vt, struct CSL_Vector* roads)
{
    int size = 0;
    int len = 0;
    int i, j;
    NB_VectorTileManagerConfiguration * pConfig = NULL;

    if (roads == NULL || (len = CSL_VectorGetLength(roads)) == 0)
        return 0;

    size += len * sizeof(NB_VectorTileRoad);

    for (i = 0; i < len; i++) 
    {
        data_road* r = (data_road*)CSL_VectorGetPointer(roads, i);
        int lines = CSL_VectorGetLength(r->vec_polyline_pack);

        size += s_vectile_strlen_with_padding(vt, &r->label);

        /* polylines */
        size += sizeof(NB_MercatorPolyline) * lines;

        for (j = 0; j < lines; j++ ) 
        {
            data_polyline_pack* pp = (data_polyline_pack*) CSL_VectorGetPointer(r->vec_polyline_pack, j);
            size += sizeof(NB_MercatorPoint) * pp->numsegments;
        }

        /* foreground tri-strip*/
        size += sizeof(NB_MercatorTriStrip) * lines;

        for (j = 0; j < lines; j++ ) 
        {
            data_polyline_pack* pp = (data_polyline_pack*) CSL_VectorGetPointer(r->vec_polyline_pack, j);
            size += sizeof(NB_MercatorPoint) * (pp->numsegments * NB_TRISTRIP_POINTS_PER_SEGMENT + NB_TRISTRIP_POINTS_ENDCAP);
        }

        pConfig = NB_VectorTileManagerGetConfiguration(NB_ContextGetVectorTileManager(vt->context));
        if (!pConfig->foregroundOnly)
        {
            /* background tri-strip*/
            size += sizeof(NB_MercatorTriStrip) * lines;

            for (j = 0; j < lines; j++ ) 
            {
                data_polyline_pack* pp = (data_polyline_pack*) CSL_VectorGetPointer(r->vec_polyline_pack, j);
                size += sizeof(NB_MercatorPoint) * (pp->numsegments * NB_TRISTRIP_POINTS_PER_SEGMENT + NB_TRISTRIP_POINTS_ENDCAP);
            }
        }
    }

    return size;   
}

static int         
s_vectile_area_size(VecTile_t* vt, struct CSL_Vector* areas)
{
    int size = 0;
    int len = 0;
    int i;

    if (areas == NULL || (len = CSL_VectorGetLength(areas)) == 0)
        return 0;

    size += len * sizeof(NB_VectorTileArea);

    for (i = 0; i < len; i++ ) 
    {
        data_area* a = (data_area*)CSL_VectorGetPointer(areas, i);

        size += s_vectile_strlen_with_padding(vt, &a->label);

        size += sizeof(NB_MercatorPolygon);

        size += sizeof(NB_MercatorPoint) * a->polyline_pack.numsegments;
    }

    return size;
}

static int
s_vectile_strlen_with_padding(VecTile_t* vt, data_string* ds)
{
    const char *s = data_string_get(NB_ContextGetDataState(vt->context), ds);

    // We need to add one for NULL termination. Especially if it is exactly at a 4 byte boundary. (e.g. string length 16)
    int size = nsl_strlen(s) + 1;

    // align at 4 bytes boundery (who knows why?)
    int r = size % 4; 

    size += (r == 0) ? (0) : (4 - r);

    return size;
}

static NB_Error   
s_vectile_copy(VecTile_t* vt, data_vector_tile* dvt)
{
    NB_Error err = NE_OK;
    s_cursor_t cursor;

    cursor.sptr = vt->vmt_buf;
    cursor.cptr = vt->vmt_buf;
    cursor.eptr = vt->vmt_buf + vt->vmt_size;

    err = s_vectile_area_copy(vt, dvt->vec_areas, &cursor);

    err = err ? err : s_vectile_road_copy(vt, dvt->vec_roads, &cursor);

    return err;
}

static NB_Error   
s_vectile_road_copy(VecTile_t* vt, struct CSL_Vector* roads, s_cursor_t* cursor)
{
    NB_Error err = NE_OK;
    int len = CSL_VectorGetLength(roads);
    NB_VectorTile* vectorTile = (NB_VectorTile*)vt;
    byte *ptr = cursor->cptr;
    int i, j, k;
    NB_VectorTileManagerConfiguration* pConfig = NULL;
    pConfig = NB_VectorTileManagerGetConfiguration(NB_ContextGetVectorTileManager(vt->context));

    vectorTile->roads = (NB_VectorTileRoad*) ptr;
    vectorTile->roadCount = len;
    ptr += sizeof(NB_VectorTileRoad) * vectorTile->roadCount;  /* ptr is now points to end of array of VectorTileRoad */

    for (i = 0; i < len; i++) 
    {
        data_road* dr = (data_road*)CSL_VectorGetPointer(roads, i);
        NB_VectorTileRoad* vr = (NB_VectorTileRoad*)vectorTile->roads + i;

        vr->priority = dr->priority;
        
        /* TRICKY: The two types are different types and it is assumed that they match! */
        vr->roadType = (NB_VectorTileRoadType) dr->type;
        vr->lanes = dr->lanes;

        vr->label = (char*) ptr;
        s_vectile_strcpy(vt, vr->label, &dr->label);
        ptr += s_vectile_strlen_with_padding(vt, &vr->label);

        /* polylines */
        vr->polylineCount = CSL_VectorGetLength(dr->vec_polyline_pack);
        vr->polylines = (NB_MercatorPolyline*) ptr;
        ptr += sizeof(NB_MercatorPolyline) * vr->polylineCount;
        for (j = 0; j < vr->polylineCount; j++) 
        {
            data_polyline_pack* dpp = (data_polyline_pack*)CSL_VectorGetPointer(dr->vec_polyline_pack, j);
            NB_MercatorPolyline* mpp = vr->polylines + j;

            mpp->count = dpp->numsegments;
            mpp->points = (NB_MercatorPoint*)ptr;
            ptr += sizeof(NB_MercatorPoint) * mpp->count;
            for (k = 0; k < mpp->count; k++) 
            {
                plp_segment* seg = dpp->segments + k;
                NB_MercatorPoint* mp = mpp->points + k;

                NB_SpatialConvertTileToMercator(vectorTile->x, vectorTile->y, vectorTile->zoomLevel, seg->x, seg->y, &mp->mx, &mp->my);
            }
        }

        /* foreground */
        vr->foregroundCount = CSL_VectorGetLength(dr->vec_polyline_pack);
        vr->foregroundTriStrip = (NB_MercatorTriStrip*) ptr;
        ptr += sizeof(NB_MercatorTriStrip) * vr->foregroundCount;
        for (j = 0; j < vr->foregroundCount; j++) 
        {
            data_polyline_pack* pp = (data_polyline_pack*)CSL_VectorGetPointer(dr->vec_polyline_pack, j);
            NB_MercatorTriStrip* ts = vr->foregroundTriStrip + j;

            ts->count = pp->numsegments * NB_TRISTRIP_POINTS_PER_SEGMENT + NB_TRISTRIP_POINTS_ENDCAP;
            ts->points = (NB_MercatorPoint*)ptr;
            ptr += sizeof(NB_MercatorPoint) * ts->count;
        }

        if (!pConfig->foregroundOnly)
        {
            /* background */
            vr->backgroundCount = CSL_VectorGetLength(dr->vec_polyline_pack);
            vr->backgroundTriStrip = (NB_MercatorTriStrip*) ptr;
            ptr += sizeof(NB_MercatorTriStrip) * vr->backgroundCount;
            for (j = 0; j < vr->backgroundCount; j++) 
            {
                data_polyline_pack* pp = (data_polyline_pack*)CSL_VectorGetPointer(dr->vec_polyline_pack, j);
                NB_MercatorTriStrip* ts = vr->backgroundTriStrip + j;

                ts->count = pp->numsegments * NB_TRISTRIP_POINTS_PER_SEGMENT + NB_TRISTRIP_POINTS_ENDCAP;
                ts->points = (NB_MercatorPoint*)ptr;
                ptr += sizeof(NB_MercatorPoint) * ts->count;
            }        
        }
        else
        {
            vr->backgroundCount = 0;
            vr->backgroundTriStrip = NULL;
        }

        // Foreground and Background tristrips
        err = NB_RoadToMercatorMultiTriStrip(vt->vectorTile.x, vt->vectorTile.y, vt->vectorTile.zoomLevel, dr, vr->foregroundTriStrip, vr->backgroundTriStrip);
        if (err != NE_OK)
        {
            break;
        }
    }

    return err;
}

static NB_Error   
s_vectile_area_copy(VecTile_t* vt, struct CSL_Vector* areas, s_cursor_t* cursor)
{
    NB_Error err = NE_OK;
    int len = CSL_VectorGetLength(areas);
    NB_VectorTile* vectorTile = (NB_VectorTile*)vt;
    byte *ptr = cursor->cptr;
    int i, j;

    vectorTile->areas = (NB_VectorTileArea*) ptr;
    vectorTile->areaCount = len;
    ptr += sizeof(NB_VectorTileArea) * vectorTile->areaCount;  /* ptr is now points to end of array of NB_VectorTileArea */

    for (i = 0; i < len; i++) 
    {
        data_area* da = (data_area*)CSL_VectorGetPointer(areas, i);
        NB_VectorTileArea* va = vectorTile->areas + i;

        va->priority = da->priority;

        /* TRICKY: The two types are different types and it is assumed that they match! */
        va->areaType = (NB_VectorTileAreaType)da->type;

        va->label = (char*) ptr;
        s_vectile_strcpy(vt, va->label, &da->label);
        ptr += s_vectile_strlen_with_padding(vt, &da->label);

        va->polygonCount = 1;
        va->polygons = (NB_MercatorPolygon*) ptr;
        ptr += sizeof(NB_MercatorPolygon) * va->polygonCount;

        va->polygons->count = da->polyline_pack.numsegments;
        va->polygons->points = (NB_MercatorPoint*) ptr;
        ptr += sizeof(NB_MercatorPoint) * va->polygons->count;

        for (j = 0; j < va->polygons->count; j++) 
        {
            NB_MercatorPoint* mp = va->polygons->points + j;
            plp_segment*   seg = da->polyline_pack.segments + j;

            NB_SpatialConvertTileToMercator(vectorTile->x, vectorTile->y, vectorTile->zoomLevel, seg->x, seg->y, &mp->mx, &mp->my);
        }
    }

    cursor->cptr = ptr;

    return err;
}

static char*
s_vectile_strcpy(VecTile_t* vt, char *dst, data_string* src)
{
    if (dst != NULL) {
        nsl_strcpy(dst, data_string_get(NB_ContextGetDataState(vt->context), src));
    }

    return dst;
}

/*! @} */


