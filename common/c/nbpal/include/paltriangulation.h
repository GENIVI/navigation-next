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

    @file       paltriangulation.h

    Arbitrary polygon with holes triangulation.
*/
/*
    (C) Copyright 2012 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#ifndef PALTRIANGULATION_H
#define PALTRIANGULATION_H

#include "paltypes.h"
#include "palerror.h"
#include "palconfig.h"
#include "pal.h"
/*!
    @addtogroup paltriangulation
    @{
*/

typedef struct
{
    float x;        /* x position */
    float y;        /* y position */
    float s;        /* s texture coordinates */
    float t;        /* t texture coordinates */
} PAL_TriangulationVertex;

typedef struct 
{
    PAL_TriangulationVertex*                vertices;          /* vertices */
    uint32                                  vertexCount;       /* vertex count */
} PAL_TriangulationVertexList;

/** Triangulate polygons. Input may contain multiple outer contours and inner holes.
@param outerPolygons
@param outerCount
@param innerPolygons
@param innerCount
@param resultVertices
@param resultVertexCount
@param resultIndices
@param resultIndexCount
@return TRUE if triangulation success, FALSE if failed
*/
PAL_DEC nb_boolean PAL_TriangulationPerform(const PAL_TriangulationVertexList* outerPolygons, uint32 outerCount, 
                                            const PAL_TriangulationVertexList* innerPolygons, uint32 innerCount,
                                            PAL_TriangulationVertex** resultVertices, uint32* resultVertexCount,
                                            uint16** resultIndices, uint32* resultIndexCount);

/** Release the memory allocated in triangulation.
@param vertices
@param indices
@return none
*/
PAL_DEC void PAL_TriangulationRelease(PAL_TriangulationVertex* vertices, uint16* indices);

/*! @} */

#endif
