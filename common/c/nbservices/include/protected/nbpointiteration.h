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

@file     nbpointiteration.h
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

/*!
@addtogroup nbpointiteration
@{
*/

#ifndef NBPOINTITERATION_H
#define NBPOINTITERATION_H

#include "paltypes.h"
#include "nbexp.h"
#include "nbspatial.h"
#include "data_polyline.h"
#include "data_polyline_pack.h"
#include "spatialvector.h"
#include "csltypes.h"
#include "nbgpstypes.h"
#include "nbvectortile.h"

typedef nb_boolean (NB_PointIterationNextPointFunction)(void* userData, NB_Vector* nextPoint);

typedef struct {

    data_polyline_pack*		packedPolyline;
	uint32					index;

	int						tx;
	int						ty;
	int						tz;
} NB_PackedPolylineForwardIterationData;

NB_DEC void NB_PointIterationInitializePackedPolylineForwardIteration(NB_PackedPolylineForwardIterationData* data, data_polyline_pack* polyline, int tx, int ty, int tz);

NB_DEC nb_boolean NB_PackedPolylineForwardIteration(void* userData, NB_Vector* nextPoint);

typedef struct  {

	data_util_state*            dataState;
	data_polyline*				polyline;
	int32						index;

} NB_PolylineForwardIterationData;

NB_DEC void NB_PointIterationInitializePolylineForwardIteration(NB_PolylineForwardIterationData* data, data_util_state* dataState, data_polyline* polyline);

typedef struct {

	data_util_state*            dataState;
	data_polyline*				polyline;
	int32						index;
	double						length;
	NB_Vector	    			lastPoint;
	double						lastLength; // for forward calculations
	double						lastHeading; // for external reference

} NB_PolylineForwardLengthIterationData;

NB_DEC void NB_PointIterationInitializePolylineForwardLengthIteration(NB_PolylineForwardLengthIterationData* data, data_util_state* dataState, data_polyline* polyline, double length);

NB_DEC nb_boolean NB_PolylineForwardIteration(void* userData, NB_Vector* nextPoint);

NB_DEC nb_boolean NB_PolylineReverseIteration(void* userData, NB_Vector* nextPoint);

NB_DEC nb_boolean NB_PolylineForwardLengthIteration(void* userData, NB_Vector* nextPoint);

NB_DEC nb_boolean NB_PolylineReverseLengthIteration(void* userData, NB_Vector* nextPoint);

typedef struct {

    NB_MercatorPolyline*        polyline;
    NB_Vector                   lastPoint;
    int                         index;
    double                      length;
    double                      lastLength;

} NB_MercatorPolylineIterationData;

NB_DEC void NB_PointIterationInitializeMercatorPolylineIteration(NB_MercatorPolylineIterationData* data, NB_MercatorPolyline* polyline, nb_boolean forward, double mercatorLength);

NB_DEC nb_boolean NB_MercatorPolylineForwardIteration(void* userData, NB_Vector* nextPoint);
NB_DEF nb_boolean NB_MercatorPolylineReverseIteration(void* userData, NB_Vector* nextPoint);
NB_DEF nb_boolean NB_MercatorPolylineForwardLengthIteration(void* userData, NB_Vector* nextPoint);
NB_DEF nb_boolean NB_MercatorPolylineReverseLengthIteration(void* userData, NB_Vector* nextPoint);
#endif

/*! @} */
