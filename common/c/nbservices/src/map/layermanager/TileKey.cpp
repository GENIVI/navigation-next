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
   @file        TileKey.cpp
   @defgroup    map

   Description: Implementation of TileKey.

*/
/*
   (C) Copyright 2012 by TeleCommunications Systems, Inc.

   The information contained herein is confidential, proprietary to
   TeleCommunication Systems, Inc., and considered a trade secret as defined
   in section 499C of the penal code of the State of California. Use of this
   information by anyone other than authorized employees of TeleCommunication
   Systems is granted only under a written non-disclosure agreement, expressly
   prescribing the scope and manner of such use.

 --------------------------------------------------------------------------*/

/*! @{ */

#include "TileKey.h"
#include "palmath.h"

using namespace nbmap;

/* See description in header file. */
TileKey::TileKey()
        :m_x(0), m_y(0), m_zoomLevel(0)
{
}

/* See description in header file. */
TileKey::TileKey(int x, int y, int zoomLevel)
        :m_x(x),m_y(y),m_zoomLevel(zoomLevel)
{
}

/* See description in header file. */
TileKey::TileKey(const TileKey& key)
{
    m_x         = key.m_x;
    m_y         = key.m_y;
    m_zoomLevel = key.m_zoomLevel;
}

/* See description in header file. */
TileKey::~TileKey()
{
}

/* See description in header file. */
bool TileKey::operator <(const TileKey& rightSide) const
{
    // zoom
    if (m_zoomLevel < rightSide.m_zoomLevel)
    {
        return true;
    }
    else if (m_zoomLevel > rightSide.m_zoomLevel)
    {
        return false;
    }

    // X
    if (m_x < rightSide.m_x)
    {
        return true;
    }
    else if (m_x > rightSide.m_x)
    {
        return false;
    }

    // Y
    if (m_y < rightSide.m_y)
    {
        return true;
    }
    else if (m_y > rightSide.m_y)
    {
        return false;
    }

    // Now they are considered equal
    return false;
};


/* See description in header file. */
bool TileKey::operator ==(const TileKey& rightSide) const
{
    return (m_x == rightSide.m_x) && (m_y == rightSide.m_y) &&
            (m_zoomLevel == rightSide.m_zoomLevel);
};

/* See description in header file. */
bool TileKey::IsValid()
{
    bool valid = true;
    int maxValue = (int)nsl_pow(2.0, m_zoomLevel) - 1;
    if (m_x > maxValue || m_y > maxValue || m_x < 0 || m_y < 0)
    {
        valid = false;
    }

    return valid;
}
/*! @} */
