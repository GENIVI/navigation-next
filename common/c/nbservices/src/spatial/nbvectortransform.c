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

@file nbvectortransform.c
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

/*!
    @addtogroup nbspatial
    @{
*/

#include "navpublictypes.h"
#include "nbvectortransform.h"
#include "palmath.h"
#include "spatialvector.h"
#include "transformutility.h"

struct NB_VectorTransform
{
    double                              xCenter;
    double                              yCenter;
    // Y axis rotation (heading in 2D map)
    double                              yrotate;
    double                              rotysin;
    double                              rotycos;
    // X axis rotation
    double                              xrotate;
    double                              rotxsin;
    double                              rotxcos;
    double                              cosLat;
    double                              cosLatRadius;
    NB_Rectangle                        rect;
    double                              aspectRatio;            // aspect ratio
    double                              alpha;
    double                              beta;
    double                              hdist;                  // horizontal camera distance from user position
    double                              verticalFieldOfView;    // field of view (vertical, calculated based on fieldOfView and aspect ratio)
    double                              d;                      // 'd' viewing distance variable used in calculations
    double                              tanhfov;                // tangent of hfov/2 - used to clip sides
    double                              tanvfov;                // tangent of verticalFieldOfView/2 - used to clip sides
    double                              rysd;                   // precalculation for 3d transform
    double                              rycd;                   // precalculation for 3d transform
    NB_VectorTransformSettings    settings;
};

static void SetRectangle(NB_VectorTransform* context, NB_Rectangle* rect);
static void Set2D(NB_VectorTransform* pThis);
static void Set3D(NB_VectorTransform* pThis);

NB_DEF NB_Error
NB_VectorTransformCreate(NB_VectorTransform** transform)
{
    NB_VectorTransform* pThis = 0;

    if (!transform)
    {
        return NE_INVAL;
    }
    *transform = 0;

    pThis = nsl_malloc(sizeof(*pThis));
    if (!pThis)
    {
        return NE_NOMEM;
    }
    nsl_memset(pThis, 0, sizeof(*pThis));

    *transform = pThis;

    return NE_OK;
}

NB_DEF NB_Error
NB_VectorTransformDestroy(NB_VectorTransform* pThis)
{
    if (!pThis)
    {
        return NE_INVAL;
    }
    nsl_free(pThis);

    return NE_OK;
}

NB_DEF NB_Error
NB_VectorTransformSet(NB_VectorTransform* pThis, NB_VectorTransformSettings* settings, NB_Rectangle* rect, NB_LatitudeLongitude* center, double heading)
{
    if (!pThis || !settings || !rect || !center)
    {
        return NE_INVAL;
    }

    if (settings->type == NB_VTT_Unknown || settings->position == NB_VTP_Unknown)
    {
        return NE_INVAL;
    }

    pThis->settings = *settings;
    SetRectangle(pThis, rect);
    NB_SpatialConvertLatLongToMercator(center->latitude, center->longitude, &pThis->xCenter, &pThis->yCenter);
    pThis->cosLat = nsl_cos(TO_RAD(center->latitude));
    pThis->cosLatRadius = pThis->cosLat * RADIUS_EARTH_METERS;
    pThis->yrotate = -heading; // map rotation is opposite that of heading

    if (pThis->settings.type == NB_VTT_2D)
    {
        Set2D(pThis);
    }
    else
    {
        Set3D(pThis);
    }

    return NE_OK;
}

NB_DEF NB_Error
NB_VectorTransformMercatorTo3D(NB_VectorTransform* pThis, double xMercator, double yMercator, NB_Vector* vector)
{
    double dx_merc = 0.0;
    double dy_merc = 0.0;
    double dx = 0.0;
    double dy = 0.0;
    double dz = 0.0;
    double rcz_ryx = 0.0;

    if (!pThis || !vector)
    {
        return NE_INVAL;
    }

    dx_merc = xMercator - pThis->xCenter;
    dy_merc = yMercator - pThis->yCenter;
    dx = (dx_merc * pThis->cosLatRadius ) + pThis->rysd;
    dy = - pThis->settings.height;
    dz = (dy_merc * pThis->cosLatRadius) + pThis->rycd;
    rcz_ryx = pThis->rotycos * dz +  pThis->rotysin * dx;

    // Dx = cos(roty) * (sin(rotz)*dy + cos(rotz)*dx) - sin(roty)*(dz)
    vector->x = pThis->rotycos * dx - pThis->rotysin * dz;

    // Dy = sin(rotx) * (cos(roty)*dz + sin(roty)*(sin(rotz)*dy + cos(rotz)*dx)) + cos(rotx)*(cos(rotz)*dy - sin(rotz)*dx)
    vector->y = pThis->rotxsin * rcz_ryx + pThis->rotxcos * dy;

    // Dz = cos(rotx) * (cos(roty)*dz + sin(roty)*(sin(rotz)*dy + cos(rotz)*dx)) - sin(rotx)*(cos(rotz)*dy - sin(rotz)*dx) 
    vector->z = pThis->rotxcos * rcz_ryx - pThis->rotxsin * dy;

    return NE_OK;
}

NB_DEF NB_Error
NB_VectorTransform3DToPixel(NB_VectorTransform* pThis, NB_Vector* vector, NB_Point* pixel)
{
    if (!pThis || !vector || !pixel)
    {
        return NE_INVAL;
    }

    if (pixel)
    {
        double xper = 0.0;
        double yper = 0.0;
        double screenx = 0.0;
        double screeny = 0.0;
        double z = vector->z;

        // prevent division by zero
        if (z == 0.0)
        {
            z = 1.0e-300; 
        }

        xper = vector->x / (z * pThis->d);
        yper = vector->y / (z * pThis->d * pThis->aspectRatio);

        // Projection -> screen transform
        screenx = pThis->alpha + xper * pThis->alpha;
        screeny = pThis->beta - yper * pThis->beta + pThis->rect.y;

        // prevent overflow of 16-bit int
        if (screenx > 32767)
        {
            screenx = 32767;
        }
        else if (screenx < -32767)
        {
            screenx = -32767;
        }

        if (screeny > 32767)
        {
            screeny = 32767;
        }
        else if (screeny < -32767)
        {
            screeny = -32767;
        }

        pixel->x = (int16)screenx;
        pixel->y = (int16)screeny;
    }

    return NE_OK;
}

NB_DEF nb_boolean
NB_VectorTransformIsTileVisible(NB_VectorTransform* pThis, int xTileIndex, int yTileIndex, int zoomLevel, double minClipZ, double maxClipZ)
{
    NB_Vector vertex_in[4] = { {0} };
    struct zclip_data zmindata = { 0 };
    struct zclip_data zmaxdata = { 0 };
    struct tanclip_data ptandata_xz = { 0 };
    struct tanclip_data ntandata_xz = { 0 };
    struct tanclip_data ptandata_yz = { 0 };
    struct tanclip_data ntandata_yz = { 0 };
    double mx = 0.0;
    double my = 0.0;    

    // set up the quad
    NB_SpatialConvertTileToMercator(xTileIndex, yTileIndex, zoomLevel, 0, 0, &mx, &my);
    NB_VectorTransformMercatorTo3D(pThis, mx, my, &vertex_in[0]);

    NB_SpatialConvertTileToMercator(xTileIndex, yTileIndex, zoomLevel, 1, 0, &mx, &my);
    NB_VectorTransformMercatorTo3D(pThis, mx, my, &vertex_in[1]);

    NB_SpatialConvertTileToMercator(xTileIndex, yTileIndex, zoomLevel, 1, 1, &mx, &my);
    NB_VectorTransformMercatorTo3D(pThis, mx, my, &vertex_in[2]);

    NB_SpatialConvertTileToMercator(xTileIndex, yTileIndex, zoomLevel, 0, 1, &mx, &my);
    NB_VectorTransformMercatorTo3D(pThis, mx, my, &vertex_in[3]);

    zmindata.zlim = minClipZ;
    zmaxdata.zlim = maxClipZ;

    ptandata_xz.tan = pThis->tanhfov;
    ntandata_xz.tan = -pThis->tanhfov;
    ptandata_yz.tan = pThis->tanvfov;
    ntandata_yz.tan = -pThis->tanvfov;

    // if the clip returns any points at all, the tile is visible
    return ((sutherland_hodgman_polygon_clip(vertex_in, 4, NULL, 0, zmin_clip, z_intersect, &zmindata) > 0)
        && (sutherland_hodgman_polygon_clip(vertex_in, 4, NULL, 0, zmax_clip, z_intersect, &zmaxdata) > 0)
        && (sutherland_hodgman_polygon_clip(vertex_in, 4, NULL, 0, xzmin_clip, xz_intersect, &ntandata_xz) > 0)
        && (sutherland_hodgman_polygon_clip(vertex_in, 4, NULL, 0, xzmax_clip, xz_intersect, &ptandata_xz) > 0)
        && (sutherland_hodgman_polygon_clip(vertex_in, 4, NULL, 0, yzmin_clip, yz_intersect, &ntandata_yz) > 0)
        && (sutherland_hodgman_polygon_clip(vertex_in, 4, NULL, 0, yzmax_clip, yz_intersect, &ptandata_yz) > 0)
        ) ? TRUE : FALSE;
}

NB_DEF NB_Error
NB_VectorTransformPixelDistanceToMercator(NB_VectorTransform* pThis, int pixelDistance, double* mercatorDistance)
{
    if (!mercatorDistance)
    {
    return NE_INVAL;
    }

    *mercatorDistance = (pixelDistance * pThis->settings.metersPerPixel) / (pThis->cosLatRadius);
    return NE_OK;
}

void
SetRectangle(NB_VectorTransform* pThis, NB_Rectangle* rect)
{
    pThis->rect = *rect;
    pThis->alpha = 0.5 * pThis->rect.dx - 0.5;
    pThis->beta = 0.5 * pThis->rect.dy - 0.5;
    pThis->aspectRatio = (double)pThis->rect.dy / (double)pThis->rect.dx;
}

void SetFieldOfView(NB_VectorTransform* pThis)
{
    pThis->verticalFieldOfView = TO_DEG(2.0 * (nsl_atan(pThis->aspectRatio * nsl_tan(TO_RAD(pThis->settings.fieldOfView / 2.0)))));
    pThis->tanhfov = nsl_tan(TO_RAD(pThis->settings.fieldOfView / 2.0));
    pThis->tanvfov = nsl_tan(TO_RAD(pThis->verticalFieldOfView / 2.0));
    pThis->d = pThis->tanhfov;
}

void Set2D(NB_VectorTransform* pThis)
{
    SetFieldOfView(pThis);

    pThis->xrotate = -90.0; // top down
    pThis->rotxsin = nsl_sin(TO_RAD(-pThis->xrotate));
    pThis->rotxcos = nsl_cos(TO_RAD(-pThis->xrotate));

    // calculate hdist
    if (pThis->settings.position == NB_VTP_Mid)
    {
        pThis->hdist = 0;    // camera directly above the point
    }
    else
    {
        // translate the camera ahead of the point so that it is at the bottom of the screen
        pThis->hdist = -pThis->settings.height * nsl_tan(TO_RAD(pThis->verticalFieldOfView / 2.0)); // negative because camera is in front of the user position

        switch (pThis->settings.position)
        {
        case NB_VTP_Custom:
            pThis->hdist = pThis->hdist * pThis->settings.positionMultiplier;
            break;
        case NB_VTP_Lower:
            // shorten the distance by 1/6 screen
            pThis->hdist = pThis->hdist * 2.0 / 3.0;
            pThis->settings.positionMultiplier = 2.0 / 3.0;
            break;
        case NB_VTP_Upper:
            pThis->hdist = pThis->hdist * 1.0 / 3.0;
            pThis->settings.positionMultiplier = 1.0 / 3.0;
            break;
        case NB_VTP_Unknown:
            case NB_VTP_Mid:
            break;
        }
    }

    pThis->rotysin = nsl_sin(TO_RAD(-pThis->yrotate));
    pThis->rotycos = nsl_cos(TO_RAD(-pThis->yrotate));
    pThis->rysd = pThis->rotysin * pThis->hdist;
    pThis->rycd = pThis->rotycos * pThis->hdist;
}

void Set3D(NB_VectorTransform* pThis)
{
    double length = 0.0;
    double alpha = 0.0;
    double beta = 0.0;

    SetFieldOfView(pThis);

    // calculate rotx and hdist.  These values are determined by the spec that the horizon should be exactly at the top of the screen
    // and the user should be 1/6 of the way up from the bottom of the screen, given a height value.
    length = nsl_sqrt((pThis->settings.zDistance * pThis->settings.zDistance) - (pThis->settings.height * pThis->settings.height));
    alpha = nsl_atan(pThis->settings.height / length);
    beta = PI_OVER_TWO - alpha - TO_RAD(pThis->verticalFieldOfView);

    switch (pThis->settings.position)
    {
    case NB_VTP_Custom:
        pThis->hdist = pThis->settings.height * nsl_tan(beta + TO_RAD(pThis->verticalFieldOfView * pThis->settings.positionMultiplier));
        break;
    case NB_VTP_Lower:
        pThis->hdist = pThis->settings.height * nsl_tan(beta + TO_RAD(pThis->verticalFieldOfView / 4.0));
        pThis->settings.positionMultiplier = 0.25;
        break;
    case NB_VTP_Mid:
        pThis->hdist = pThis->settings.height * nsl_tan(beta + TO_RAD(pThis->verticalFieldOfView / 2.0));
        pThis->settings.positionMultiplier = 0.5;
        break;
    case NB_VTP_Upper:
    default:
        pThis->hdist = pThis->settings.height * nsl_tan(beta + TO_RAD(2.0 * pThis->verticalFieldOfView / 3.0));
        pThis->settings.positionMultiplier = 2.0/3.0;
    }

    pThis->xrotate = -TO_DEG(alpha + TO_RAD(pThis->verticalFieldOfView / 2.0));

    pThis->rotxsin = nsl_sin(TO_RAD(-pThis->xrotate));
    pThis->rotxcos = nsl_cos(TO_RAD(-pThis->xrotate));
    pThis->rotysin = nsl_sin(TO_RAD(-pThis->yrotate));
    pThis->rotycos = nsl_cos(TO_RAD(-pThis->yrotate));
    pThis->rysd = pThis->rotysin * pThis->hdist;
    pThis->rycd = pThis->rotycos * pThis->hdist;
}

NB_Error
NB_VectorTransformGetNearPointDistanceForCamera(NB_VectorTransform* pThis, double* distanceOut)
{
    double subjectDistance = 0;
    double bottomDistance = 0;

    if (pThis->settings.fieldOfView == 0) // Transform is not initialized
    {
        if (distanceOut != NULL)
        {
            *distanceOut = 0;
        }
        return NE_NOTINIT;
    }

    subjectDistance = pThis->settings.height / nsl_tan(TO_RAD(nsl_fabs(pThis->xrotate) - pThis->verticalFieldOfView * (pThis->settings.positionMultiplier - 0.5)));
    bottomDistance = pThis->settings.height / nsl_tan(TO_RAD(nsl_fabs(pThis->xrotate) + pThis->verticalFieldOfView / 2.0));
    
    if (distanceOut != NULL)
    {
        *distanceOut = subjectDistance - bottomDistance;
    }

    return NE_OK;
}

/*! @} */
