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

    @file       TileImpl.cpp

    See header file for description.
*/
/*
    (C) Copyright 2011 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#include "TileImpl.h"
#include "datastreamimplementation.h"

#define TILE_INFO_INVALID_VALUE ((uint32)-1)
#define BUFFER_SIZE             4096


/*! @{ */

using namespace std;
using namespace nbcommon;
using namespace nbmap;

// Local Constants ..............................................................................
//static const uint32 INVALID_FRAME_INDEX = (uint32)(-1);

// TileImpl Public functions ................................................................

/* See header file for description */
TileImpl::TileImpl(shared_ptr<string> contentId,
                   TileLayerInfoPtr info,
                   bool duplicated,
                   shared_ptr<const string> categoryName)
        : m_tileLayerInfo(info),
          m_contentId(contentId),
          m_timeStamp(0),
          m_isDuplicate(duplicated)
{
    if (m_contentId)
    {
        // Make a copy of cachingName, cachingName should not be changed during life cycle
        // of this tile, but contentId may get changed.
        m_cachingName.reset(new string(*m_contentId));
    }
    // Nothing to do here.
}

/* See header file for description */
TileImpl::~TileImpl()
{
    // Nothing to do here.
}

/* See description in Tile.h */
DataStreamPtr
TileImpl::GetData()
{
    return m_dataStream;
}

/* See description in Tile.h */
void
TileImpl::SetData(DataStreamPtr dataStream)
{
    m_dataStream = dataStream;
}

/* See description in Tile.h */
uint32
TileImpl::GetSize()
{
    uint32 size = 0;
    if (m_dataStream)
    {
        size = m_dataStream->GetDataSize();
    }

    return size;
}

/* See description in Tile.h */
shared_ptr<string>
TileImpl::GetDataType()
{
    return m_tileLayerInfo->tileDataType;
}

/* See description in Tile.h */
shared_ptr<string>
TileImpl::GetContentID()
{
    return m_contentId;
}

/* See description in Tile.h */
TileKeyPtr
TileImpl::GetTileKey()
{
    return m_tileKey;
}

/* See description in Tile.h */
void
TileImpl::SetTileKey(TileKeyPtr tileKey)
{
    m_tileKey = tileKey;
}

/* See description in Tile.h */
shared_ptr<string>
TileImpl::GetCachingPath()
{
    return m_cachingPath;
}

/* See description in Tile.h */
void
TileImpl::SetCachingPath(shared_ptr<string> cahingPath)
{
    m_cachingPath = cahingPath;
}

/* See description in Tile.h */
shared_ptr<map<string, shared_ptr<string> > >
TileImpl::GetAdditionalData()
{
    return m_additionalData;
}

/* See description in Tile.h */
void
TileImpl::SetAdditionalData(shared_ptr<map<string, shared_ptr<string> > > additionalData)
{
    m_additionalData = additionalData;
}

/* See description in Tile.h */
uint32
TileImpl::GetDrawOrder()
{
    return m_tileLayerInfo->drawOrder;
}

/* See description in Tile.h */
uint32 TileImpl::GetLayerID()
{
    return m_tileLayerInfo->layerID;
}

/* See description in header file. */
uint32 TileImpl::GetLabelDrawOrder()
{
    return m_tileLayerInfo->labelDrawOrder;
}

/* See description in header file. */
uint32 TileImpl::GetTimeStamp() const
{
    return m_timeStamp;
}

/* See description in header file. */
void TileImpl::SetTimeStamp(uint32 timeStamp)
{
    m_timeStamp = timeStamp;
}

/* See description in Tile.h */
bool TileImpl::IsDuplicatedTile()
{
    return m_isDuplicate;
}
/* See description in header file. */
shared_ptr <string> TileImpl::GetCachingName() const
{
    return m_cachingName;
}

/* See description in header file. */
shared_ptr<const string> TileImpl::GetMaterialCategory() const
{
    return m_tileLayerInfo->materialCategory;
}

/* See description in header file. */
shared_ptr<const string> TileImpl::GetMaterialBaseType() const
{
    return m_tileLayerInfo->materialBaseType;
}

/* See description in header file. */
shared_ptr<const string> TileImpl::GetMaterialThemeType() const
{
    return m_tileLayerInfo->materialThemeType;
}

/* See description in header file. */
DrawOrder TileImpl::GetDrawOrders() const
{
    DrawOrder orders;
    orders.baseDrawOrder  = (uint8)m_tileLayerInfo->drawOrder;
    orders.labelDrawOrder = (uint8)m_tileLayerInfo->labelDrawOrder;
    orders.subDrawOrder   = m_tileLayerInfo->subDrawOrder;
    return orders;
}

/*! @} */
