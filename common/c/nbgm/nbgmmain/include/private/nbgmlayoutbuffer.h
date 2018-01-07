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

    @file nbgmlayoutbuffer.h
*/
/*
    (C) Copyright 2012 by Networks In Motion, Inc.

    The information contained herein is confidential, proprietary
    to Networks In Motion, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
#ifndef _NBGM_LAYOUT_BUFFER_H_
#define _NBGM_LAYOUT_BUFFER_H_
#include "nbretypes.h"
#include "nbrevector3.h"
#include "nbrenode.h"
#include "palerror.h"
#include "nbgmlayoutpolyline.h"
#include "nbreaxisalignedbox2.h"
#include "nbgmrasterize.h"
#include "nbgmcontext.h"
#include "nbgmlayoutelement.h"

/*! \addtogroup NBGM_Service
*  @{
*/

/** Trace the available screen space.
This class use a 2d buffer to record used region in screen.(But has less pixels)
Before adding any element, check this buffer first.
*/
class NBGM_LayoutBuffer
{
public:
    NBGM_LayoutBuffer(NBGM_Context& nbgmContext, float width, float height, float cellSize);
    ~NBGM_LayoutBuffer();

public:
    /// Clear buffer with zero
    void Clear();
    /// Clear mask bits with zero
    void Clear(uint32 mask);
    /// Resize layout buffer
    void Resize(float width, float height);
    /// Is there any collision in rect
    nb_boolean IsRegionAvailable(const NBRE_AxisAlignedBox2d& rect, uint32 value);
    /// Is there any collision in polyline
    nb_boolean IsRegionAvailable(const NBRE_Polyline2d& polyline, int32 lineWidth, uint32 mask);
    /// Update rect region with value
    void UpdateRegion(const NBRE_AxisAlignedBox2d& rect, uint32 mask);
    /// Update rect region with value with expand
    void UpdateRegion(const NBRE_AxisAlignedBox2d& rect, uint32 value, int32 expand);
    /// Update polyline region with value
    void UpdateRegion(const NBRE_Polyline2d& polyline, int32 lineWidth, uint32 mask);
    /// Update line region with value
    void UpdateRegion(const NBRE_Vector2d& p0, const NBRE_Vector2d& p1, int32 lineWidth, uint32 mask);
    /// Get polyline parts without any collision
    NBGM_LayoutPolylineList GetRegionAvailableParts(const NBGM_LayoutPolyline& polyline, int32 lineWidth, uint32 mask);
    /// Get polyline parts without any collision
    nb_boolean GetAvailablePolylineParts(const NBRE_Polyline2d& polyline, int32 lineWidth, uint32 mask, NBRE_Vector<NBRE_Polyline2d>& result);

    const uint32* Buffer() const { return mBuffer; }
    uint32* Buffer() { return mBuffer; }
    const NBRE_Vector2f& Size() const { return mSize; }
    const NBRE_Vector2i& CellCount() const { return mCellCount; }
    float CellSize() const { return mCellSize; }

    nb_boolean TransformScreenRectToBufferRect(const NBRE_AxisAlignedBox2d& rect, NBRE_AxisAlignedBox2i& result) const;

private:
    /// Transform polyline from screen space to buffer space
    NBGM_Point2iList TransformPolyline(const NBRE_Polyline2d& polyline);
    /// Convert from buffer coordinates to world coordinates
    NBRE_Vector2d BufferToWorld(const NBRE_Vector2i& position);
    /// Clip rect by buffer boundary, return FALSE if out of buffer
    nb_boolean ClipRect(NBRE_AxisAlignedBox2i& rect);
    /// Check is rect free
    nb_boolean CheckRect(const NBRE_AxisAlignedBox2i& rect, uint32 mask);
    /// Update rect region
    void UpdateRect(const NBRE_AxisAlignedBox2i& rect, uint32 mask);

private:
    DISABLE_COPY_AND_ASSIGN(NBGM_LayoutBuffer);

private:
    NBGM_Context& mNBGMContext;
    uint32* mBuffer;
    NBRE_Vector2f mSize;
    NBRE_Vector2i mCellCount;
    float mCellSize;
};

/*! @} */
#endif
