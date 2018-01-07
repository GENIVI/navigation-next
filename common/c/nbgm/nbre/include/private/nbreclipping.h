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

    @file nbreclipping.h
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

#ifndef _NBRE_CLIPPING_H_
#define _NBRE_CLIPPING_H_
#include "nbreaxisalignedbox2.h"
#include "nbrevector2.h"
#include "nbretypes.h"
#include "nbrepolyline2.h"

/** \addtogroup NBRE_Core
*  @{
*/

/** Clip result
*/
enum NBRE_LineClipResult
{
    ///Both points out of clipping region
    NBRE_LCR_OUTSIDE,
    /// Both end points in clipping region
    NBRE_LCR_INSIDE,
    /// Start point out of clipping region and replaced
    NBRE_LCR_CLIP_START,
    /// End point out of clipping region and replaced
    NBRE_LCR_CLIP_END,
    /// Both points out of clipping region and replaced
    NBRE_LCR_CLIP_BOTH
};

//! Clipping algorithms
/** 
*/
template <typename T>
class NBRE_Clipping
{
    typedef NBRE_Vector< NBRE_Polyline2<T> > Polyline2List;
    typedef NBRE_Vector< NBRE_Polyline3<T> > Polyline3List;
public:
    /// Cohen-Sutherland line clipping
    static NBRE_LineClipResult ClipLineByRect(const NBRE_AxisAlignedBox2<T>& rect, NBRE_Vector2<T>& np1, NBRE_Vector2<T>& np2);
    /// Clip polyline by rect
    static Polyline2List ClipByRect(const NBRE_AxisAlignedBox2<T>& rect, const NBRE_Polyline2<T>& screenPolyline);
    /// Clip by plane
    static NBRE_LineClipResult ClipByPlane(const NBRE_Plane<T>& plane, NBRE_Vector3<T>& np1, NBRE_Vector3<T>& np2);
    /// Clip polyline by plane
    static void ClipByPlane(const NBRE_Plane<T>& plane, const NBRE_Polyline3<T>& polyline, NBRE_Vector< NBRE_Polyline3<T> >& result);
};

#include "nbreclipping.inl"

typedef NBRE_Clipping<float> NBRE_Clippingf;
typedef NBRE_Clipping<double> NBRE_Clippingd;
typedef NBRE_Clipping<int32> NBRE_Clippingi;
/** @} */
#endif
