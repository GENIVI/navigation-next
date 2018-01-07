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
   @file         TileKeyUtils.h
   @defgroup     nbmap
*/
/*
    (C) Copyright 2013 by TeleCommunications Systems, Inc.

   The information contained herein is confidential, proprietary to
   TeleCommunication Systems, Inc., and considered a trade secret as defined
   in section 499C of the penal code of the State of California. Use of this
   information by anyone other than authorized employees of TeleCommunication
   Systems is granted only under a written non-disclosure agreement, expressly
   prescribing the scope and manner of such use.
--------------------------------------------------------------------------*/

/*! @{ */
#ifndef _TILEKEYUTILS_H_
#define _TILEKEYUTILS_H_

#include "TileKey.h"
#include "StringUtility.h"
#include <vector>

namespace nbmap
{

/*! Converts tile key to a serials of tile keys of other zoom level.

  For example, convert one tile key of zoom level 14 into 4 tile keys of zoom level
  15.

  @return void
*/
inline void
ConvertTileKeyToOtherZoomLevel(TileKeyPtr tileKey,  /*!< tile Key to be converted */
                               uint32 targetLevel,  /*!< Target zoom level */
                               vector<TileKeyPtr>& convertedKeys /*!< outputted results*/
                               )
{
    if (!tileKey)
    {
        return;
    }

    int distance = tileKey->m_zoomLevel - targetLevel ;
    if (distance > 0)        // Requested tile key is higher than target level.
    {
        TileKeyPtr convertedKey(new TileKey(tileKey->m_x >> distance,
                                            tileKey->m_y >> distance,
                                            targetLevel));
        if (convertedKey)
        {
            convertedKeys.push_back(convertedKey);
        }
    }
    else if (distance < 0)   // Requested tile key is smaller than target level.
    {
        nsl_assert(abs(distance) < 3);  // This should really not happen, just assert. If assert ouccred, you should find the cause.
        distance = abs(distance);
        int x0 = tileKey->m_x << distance;
        int x1 = (tileKey->m_x + 1) << distance;
        int y0 = tileKey->m_y << distance;
        int y1 = (tileKey->m_y+1) << distance;

        for (int x = x0; x < x1; ++x)
        {
            for (int y = y0; y < y1; ++y)
            {
                TileKeyPtr convertedKey(new TileKey(x, y, targetLevel));
                if (convertedKey)
                {
                    convertedKeys.push_back(convertedKey);
                }
                else
                {
                    // Should never happen.
                }
            }
        }
    }
    else
    {
        convertedKeys.push_back(tileKey);
    }
}

/*! Converts tile key to a serials of tile keys of other zoom level.

  For example, convert one tile key of zoom level 14 into 4 tile keys of zoom level
  15.

  @return void
*/
inline void
ConvertTileKeyToOtherZoomLevel(TileKeyPtr tileKey,  /*!< tile Key to be converted */
                               uint32 targetLevel,  /*!< Target zoom level */
                               set<TileKey>& convertedKeys /*!< outputted results*/
                               )
{
    if (!tileKey)
    {
        return;
    }

    int distance = tileKey->m_zoomLevel - targetLevel ;
    if (distance > 0)        // Requested tile key is higher than target level.
    {
        TileKeyPtr convertedKey(new TileKey(tileKey->m_x >> distance,
                                            tileKey->m_y >> distance,
                                            targetLevel));
        if (convertedKey)
        {
            convertedKeys.insert(*convertedKey);
        }
    }
    else if (distance < 0)   // Requested tile key is smaller than target level.
    {
        distance = abs(distance);
        int x0 = tileKey->m_x << distance;
        int x1 = (tileKey->m_x + 1) << distance;
        int y0 = tileKey->m_y << distance;
        int y1 = (tileKey->m_y+1) << distance;

        for (int x = x0; x < x1; ++x)
        {
            for (int y = y0; y < y1; ++y)
            {
                TileKeyPtr convertedKey(new TileKey(x, y, targetLevel));
                if (convertedKey)
                {
                    convertedKeys.insert(*convertedKey);
                }
                else
                {
                    // Should never happen.
                }
            }
        }
    }
    else
    {
        convertedKeys.insert(*tileKey);
    }
}

inline string StringifyTileKey(const TileKey& key)
{
    string str("TileKey(");
    str += nbcommon::StringUtility::NumberToString(key.m_x) + ", " +
           nbcommon::StringUtility::NumberToString(key.m_y) + ", " +
           nbcommon::StringUtility::NumberToString(key.m_zoomLevel) + ")";
    return str;
}

inline string StringifyTileKey(const TileKeyPtr& key)
{
    if (key)
    {
        return StringifyTileKey(*key);
    }
    return string("EmptyTileKey");
}
}

#endif /* _TILEKEYUTILS_H_ */
/*! @} */
