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

    @file     nbpointiteration.c
    @defgroup nbpointiteration Point Iteration
*/
/*
    See file description in header file.

    (C) Copyright 2008 - 2009 by Networks In Motion, Inc.                

    The information contained herein is confidential, proprietary 
    to Networks In Motion, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of Networks In Motion is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

/*! @{ */

#include "nbpointiteration.h"


NB_DEF void
NB_PointIterationInitializePackedPolylineForwardIteration(NB_PackedPolylineForwardIterationData* data, data_polyline_pack* polyline, int tx, int ty, int tz)
{
    data->packedPolyline = polyline;
    data->index = 0;
    data->tx = tx;
    data->ty = ty;
    data->tz = tz;
}

NB_DEF nb_boolean
NB_PackedPolylineForwardIteration(void* userData, NB_Vector* nextPoint)
{
	NB_PackedPolylineForwardIterationData*	data = userData;
	data_polyline_pack*					    packedPolyline = data->packedPolyline;

	while (data->index < data->packedPolyline->numsegments) {

		if (data->index > 0 && 
			packedPolyline->segments[data->index].x == packedPolyline->segments[data->index-1].x &&
			packedPolyline->segments[data->index].y == packedPolyline->segments[data->index-1].y) {

			data->index++;
			continue;
		}

        NB_SpatialConvertTileToMercator(data->tx, data->ty, data->tz, 
            packedPolyline->segments[data->index].x, packedPolyline->segments[data->index].y, 
			&nextPoint->x, &nextPoint->y);

        nextPoint->z = 0;

		data->index++;

		return TRUE;
	}

	return FALSE;
}

NB_DEF void
NB_PointIterationInitializePolylineForwardIteration(NB_PolylineForwardIterationData* data, data_util_state* dataState, data_polyline* polyline)
{
    data->dataState = dataState;
    data->polyline = polyline;
    data->index = 0;
}

NB_DEF void
NB_PointIterationInitializePolylineForwardLengthIteration(NB_PolylineForwardLengthIterationData* data, data_util_state* dataState, data_polyline* polyline, double length)
{
    data->dataState = dataState;
    data->polyline = polyline;
    data->index = 0;
    data->length = length;
    data->lastPoint.x = data->lastPoint.y = INVALID_LATLON;
    data->lastPoint.z = 0;
    data->lastLength = 0;
    data->lastHeading = INVALID_HEADING;
}

NB_DEF nb_boolean 
NB_PolylineForwardIteration(void* userData, NB_Vector* nextPoint)
{
	NB_PolylineForwardIterationData* data = userData;
	double lat, lon;

	if (data->index < (int32) data->polyline->numsegments) {

		data_polyline_get(data->dataState, data->polyline, data->index, &lat, &lon, NULL, NULL);
        NB_SpatialConvertLatLongToMercator(lat, lon, &nextPoint->x, &nextPoint->y);
		nextPoint->z = 0.0;

		data->index++;
		return TRUE;
	}
	
	return FALSE;
}

NB_DEF nb_boolean 
NB_PolylineReverseIteration(void* userData, NB_Vector* nextPoint)
{
	NB_PolylineForwardIterationData*	data = userData;
	double lat, lon;

	if (data->index >= 0) {

		data_polyline_get(data->dataState, data->polyline, data->index, &lat, &lon, NULL, NULL);
		NB_SpatialConvertLatLongToMercator(lat, lon, &nextPoint->x, &nextPoint->y);
		nextPoint->z = 0.0;

		data->index--;
		return TRUE;
	}

	return FALSE;
}

// Going forward seems less intuitive than going in reverse, seeing as the length-truncated endpoint relies
// on the information stored with the previous point.
NB_DEF nb_boolean 
NB_PolylineForwardLengthIteration(void* userData, NB_Vector* nextPoint)
{
	NB_PolylineForwardLengthIterationData*	data = userData;
	double lat, lon, length, mercatorMagnitude;
	NB_Vector v;

	if (data->index < (int32)data->polyline->numsegments && data->length > 0) {

		data_polyline_get(data->dataState, data->polyline, data->index, &lat, &lon, &length, NULL);

		NB_SpatialConvertLatLongToMercator(lat, lon, &nextPoint->x, &nextPoint->y);
		nextPoint->z = 0.0;

		// if first point, skip length measuring step
		if (data->lastPoint.x != INVALID_LATLON && data->lastPoint.y != INVALID_LATLON)
		{
			if (data->length > data->lastLength)
				data->length -= data->lastLength;
			else
			{
				// get the vector from the last point to this one
				vector_subtract(&v, nextPoint, &data->lastPoint);
				mercatorMagnitude = vector_magnitude(&v);
				// normalize to a fraction of the current length
				vector_normalize_to_length(&v, &v,  mercatorMagnitude * data->length / data->lastLength);
				vector_add(nextPoint, &data->lastPoint, &v);
				data->length = 0;
			}

			//to prevent direction from being set to 0, for the case 'arrow stem length' > polyline 
			if ((int32)data->polyline->numsegments - data->index == 1 && data->length > 0)
				data->length = 0;			
		}

		data->lastPoint = *nextPoint;
		data->lastLength = length;

		data->index++;
		return TRUE;
	}
	
	return FALSE;
}

NB_DEF nb_boolean 
NB_PolylineReverseLengthIteration(void* userData, NB_Vector* nextPoint)
{
	NB_PolylineForwardLengthIterationData*	data = userData;
	double lat, lon, length, mercatorMagnitude, heading;
	NB_Vector v;

	if (data->index >= 0 && data->length > 0) {

		data_polyline_get(data->dataState, data->polyline, data->index, &lat, &lon, &length, &heading);

        NB_SpatialConvertLatLongToMercator(lat, lon, &nextPoint->x, &nextPoint->y);
		nextPoint->z = 0.0;

		// if first point, skip length measuring step
		if (data->lastPoint.x != INVALID_LATLON && data->lastPoint.y != INVALID_LATLON)
		{
			if (data->length > length)
				data->length -= length;
			else
			{
				// get the vector from the last point to this one
                vector_subtract(&v, nextPoint, &data->lastPoint);
				mercatorMagnitude = vector_magnitude(&v);
				// normalize to a fraction of the current length
				vector_normalize_to_length(&v, &v,  mercatorMagnitude * data->length / length);
				vector_add(nextPoint, &data->lastPoint, &v);
				data->length = 0;
			}
		}

		data->lastPoint = *nextPoint;
		data->lastHeading = heading;

		data->index--;
		return TRUE;
	}

	return FALSE;
}

NB_DEF void
NB_PointIterationInitializeMercatorPolylineIteration(NB_MercatorPolylineIterationData* data, NB_MercatorPolyline* polyline, nb_boolean forward, double mercatorLength)
{
    data->polyline = polyline;
    if (forward)
    {
        data->index = 0;
    }
    else
    {
        data->index = polyline->count - 1;
    }
    data->length = mercatorLength;
    data->lastPoint.x = INVALID_LATLON;
    data->lastPoint.y = INVALID_LATLON;
}

NB_DEF nb_boolean 
NB_MercatorPolylineForwardIteration(void* userData, NB_Vector* nextPoint)
{
	NB_MercatorPolylineIterationData* data = userData;

	if (data->index < data->polyline->count) {

        nextPoint->x = data->polyline->points[data->index].mx;
        nextPoint->y = data->polyline->points[data->index].my;
		nextPoint->z = 0.0;

		data->index++;
		return TRUE;
	}
	
	return FALSE;
}

NB_DEF nb_boolean 
NB_MercatorPolylineReverseIteration(void* userData, NB_Vector* nextPoint)
{
    NB_MercatorPolylineIterationData* data = userData;

    if (data->index >= 0) {

        nextPoint->x = data->polyline->points[data->index].mx;
        nextPoint->y = data->polyline->points[data->index].my;
        nextPoint->z = 0.0;

        data->index--;
        return TRUE;
    }

    return FALSE;
}

NB_DEF nb_boolean 
NB_MercatorPolylineForwardLengthIteration(void* userData, NB_Vector* nextPoint)
{
    NB_MercatorPolylineIterationData*	data = userData;
    double length;
    NB_Vector v;

    if (data->index < (int32)data->polyline->count && data->length > 0)
    {
        nextPoint->x = data->polyline->points[data->index].mx;
        nextPoint->y = data->polyline->points[data->index].my;
        nextPoint->z = 0.0;

        length = 0;

        // if first point, skip length measuring step
        if (data->lastPoint.x != INVALID_LATLON && data->lastPoint.y != INVALID_LATLON)
        {
            // get the vector from the last point to this one
            vector_subtract(&v, nextPoint, &data->lastPoint);
            length = vector_magnitude(&v);

            // If we are terminating before this new point
            if (length > data->length)
            {
                // normalize to remaining length
                vector_normalize_to_length(&v, &v, data->length);
                vector_add(nextPoint, &data->lastPoint, &v);
                data->length = 0;
            }
            else // Decrement the remaining length
            {
                data->length = data->length - length;
            }
        }

        data->lastPoint = *nextPoint;
        data->lastLength = length;
        data->index++;
        return TRUE;
    }

    return FALSE;
}

NB_DEF nb_boolean 
NB_MercatorPolylineReverseLengthIteration(void* userData, NB_Vector* nextPoint)
{
    NB_MercatorPolylineIterationData*	data = userData;
    double length;
    NB_Vector v;

    if (data->index >= 0 && data->length > 0)
    {
        nextPoint->x = data->polyline->points[data->index].mx;
        nextPoint->y = data->polyline->points[data->index].my;
        nextPoint->z = 0.0;

        length = 0;

        // if first point, skip length measuring step
        if (data->lastPoint.x != INVALID_LATLON && data->lastPoint.y != INVALID_LATLON)
        {
            // get the vector from the last point to this one
            vector_subtract(&v, nextPoint, &data->lastPoint);
            length = vector_magnitude(&v);

            // If we are terminating before this new point
            if (length > data->length)
            {
                // normalize to remaining length
                vector_normalize_to_length(&v, &v, data->length);
                vector_add(nextPoint, &data->lastPoint, &v);
                data->length = 0;
            }
            else // Decrement the remaining length
            {
                data->length = data->length - length;
            }
        }

        data->lastPoint = *nextPoint;
        data->lastLength = length;
        data->index--;
        return TRUE;
    }

    return FALSE;
}

/*! @} */
