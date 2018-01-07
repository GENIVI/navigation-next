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

    @file patternsegment.h
    @date 09/16/2014
    @defgroup ltkcommon
*/
/*
 * (C) Copyright 2014 by TeleCommunication Systems, Inc.
 *
 * The information contained herein is confidential, proprietary
 * to TeleCommunication Systems, Inc., and considered a trade secret
 * as defined in section 499C of the penal code of the State of
 * California. Use of this information by anyone other than
 * authorized employees of TeleCommunication Systems is granted only
 * under a written non-disclosure agreement, expressly prescribing
 * the scope and manner of such use.
 *
 */

/*! @{ */
#ifndef __PATTERN_SEGMENT_H__
#define __PATTERN_SEGMENT_H__

#include "segmentattribute.h"
#include <QtGui/QBitmap>

namespace locationtoolkit
{
/*! Represents a Polyline PatternSegment */
class PatternSegment: public SegmentAttribute
{
public:
    /*! Creates a new ColorSegment object with the specified endPointIndex and color values.
     @param endPointIndex.
     @param bitmap.
     @param distance
     */
    PatternSegment(int endPointIndex, const QString& bitmapPath, float distance);

    /*! Returns bitmap data.
     @return Current the bitmap of PatternSegment object.
     */
    const QString& GetBitmapPath() const;

    /*! Sets bitmap data value, png format picture.
     @param Set the bitmap of PatternSegment object.
     */
    void SetBitmapPath(const QString& bitmap);

    /*! Returns distance value.
     @return Current the distance of PatternSegment object.
     */
    float GetDistance() const;

    /*! Sets distance value.
     @param Set the distance of PatternSegment object.
     */
    void SetDistance(float distance);

private:
    /*! The bitmap data of PatternSegment object, png format picture. */
    QString mBitmap;
    /*! percentage of bitmap width to describe gap between two segment. */
    float mDistance;
};
}
#endif // __PATTERN_SEGMENT_H__
