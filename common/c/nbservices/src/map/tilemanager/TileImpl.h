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

    @file       TileImpl.h

    Class TileImpl inherits from Tile interface. An TileImpl
    object is used to return the data of tile when user requests a tile
    by the tile manager.
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

#ifndef TILEIMPL_H
#define TILEIMPL_H

/*!
    @addtogroup nbmap
    @{
*/

extern "C"
{
#include "nberror.h"
}

#include "TileInternal.h"
#include "smartpointer.h"
#include <string>
#include "base.h"

namespace nbmap
{

// Constants ....................................................................................

// Types ........................................................................................

/*! An TileImpl object is used to return the data of tile */
class TileImpl : public Tile,
                 public Base
{
public:
    // Public functions .........................................................................

    /*! TileImpl constructor */
    TileImpl(shared_ptr<std::string> contentId,
             TileLayerInfoPtr info,
             bool duplicated = false,
             shared_ptr<const string> categoryName = shared_ptr<const string>());

    /*! TileImpl destructor */
    virtual ~TileImpl();

    /* See description in Tile.h */
    virtual nbcommon::DataStreamPtr GetData();
    virtual uint32 GetSize();
    virtual shared_ptr<string> GetDataType();
    virtual shared_ptr<string> GetContentID();
    virtual TileKeyPtr GetTileKey();
    virtual shared_ptr<string> GetCachingPath();
    virtual shared_ptr<map<string, shared_ptr<string> > > GetAdditionalData();
    virtual uint32 GetDrawOrder();
    virtual uint32 GetLabelDrawOrder();
    virtual uint32 GetLayerID();
    virtual uint32 GetTimeStamp() const;
    virtual shared_ptr<const string> GetMaterialCategory() const;
    virtual shared_ptr<const string> GetMaterialBaseType() const;
    virtual shared_ptr<const string> GetMaterialThemeType() const;
    virtual DrawOrder GetDrawOrders() const;

    /*! Set tile key of data

      @return None
    */
    void SetTileKey(TileKeyPtr tileKey);

    /*! Set the tile data

      @return None
    */
    void SetData(nbcommon::DataStreamPtr dataStream);

    /*! Set the additional data of key and value

        The keys of the additional data are TILE_ADDITIONAL_KEY_*.

        @return None
        @see GetAdditionalData
    */
    void SetAdditionalData(shared_ptr<map<string,
                                              shared_ptr<string> > > additionalData);

    /*! Set the full path of the caching tile

      @return None
      @see GetCachingPath
    */
    void SetCachingPath(shared_ptr<string> cahingPath);

    /*! Set time stamp of this tile.

      @return None.
    */
    void SetTimeStamp(uint32 timeStamp);


    /*! Check if this tile is duplicated.

       @return true if so.
    */
    bool IsDuplicatedTile();

    /*! Return caching name of this tile.

      @return shared pointer of cached tile.
    */
    shared_ptr<string> GetCachingName() const;

private:
    // Private functions ........................................................................

    // Copy constructor and assignment operator are not supported.
    TileImpl(const TileImpl& tile);
    TileImpl& operator=(const TileImpl& tile);

    // Private members ..........................................................................

    TileLayerInfoPtr m_tileLayerInfo;   /*!< Tile info shared by this layer */
    TileKeyPtr       m_tileKey;         /*!< Tile key of data */
    nbcommon::DataStreamPtr m_dataStream;   /*!< Data of the tile */
    shared_ptr<string> m_dataType;     /*!< Data type of the tile */
    shared_ptr<string> m_contentId;    /*!< Content ID of the tile, used to identify the
                                            tile (mainly for components out of CCC, for
                                            example, NBGM). This ID may get changed after tile
                                            generated to ensure ID is unique. */
    shared_ptr<string> m_cachingName;  /*!< Caching name of this tile, used to get content
                                            from cache. Different from m_contentId, this
                                            cachingName will not change during tile's life
                                            cycle, and only used internally in CCC. */
    shared_ptr<string> m_cachingPath;  /*!< Full path of the caching tile */
    shared_ptr<map<string, shared_ptr<string> > > m_additionalData;  /*!< Additional data of key and value */
    uint32 m_timeStamp;

    bool   m_isDuplicate;               /*!< Flag to indicate if this tile is duplicated tile.
                                             If a tile is duplicated, it does not need to save
                                             to cache.
                                        */
};

typedef shared_ptr<TileImpl> TileImplPtr;

};  // namespace nbmap

/*! @} */

#endif  // TILEIMPL_H
