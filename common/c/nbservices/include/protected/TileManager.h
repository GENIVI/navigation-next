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
    @file     TileManager.h
    @defgroup nbmap

*/
/*
    (C) Copyright 2011 by TeleCommunication Systems, Inc.                

    The information contained herein is confidential, proprietary 
    to TeleCommunication Systems, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of TeleCommunication Systems, is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/
/*! @{ */
#ifndef __TILE_MANAGER__
#define __TILE_MANAGER__

#include "smartpointer.h"
#include "TileInternal.h"
#include "AsyncCallback.h"
#include <map>
#include <string>

namespace nbmap
{
// Download priorities of tile requests. Lower number has higher priority.
#define HIGHEST_TILE_REQUEST_PRIORITY 0
    
// Default maximum count of requests to download tiles
#define DEFAULT_MAX_TILE_REQUEST_COUNT 64

/*!
	The Tile Manager component provides tiles to its clients. 
	Internally it will manage the complexity of working in offboard, 
	onboard, hybrid, NBI Service, etc modes of operation.
*/
class TileManager
{
public:

    TileManager() {}

    virtual ~TileManager() {}

    /*! Submit a tile request but do not care the returned tile

        This function can be used to prefetch tiles.

        @return None
    */
    virtual void
    GetTile(shared_ptr<std::map<std::string, std::string> > templateParameters, /*!< Parameter map of the tile request to get.
                                                                                     It contains key and value for both URL and
                                                                                     content ID templates. Key and value is like:
                                                                                     $x, 9090
                                                                                     $y, 13977

                                                                                     It can be NULL if there is no template
                                                                                     parameters. */
            uint32 priority                                                     /*!< Priority of tile request. Lower number has
                                                                                     higher priority. Zero is highest priority. */
            ) = 0;

    /*! Submit a tile request.

        When the tiles are available, invokes the callback to inform the client.
        If fastLoadOnly is true the tiles are fetched in the least expensive way, no network calls, no database calls,
        just the local cahce is queired.

        @return None
    */
    virtual void
    GetTile(shared_ptr<std::map<std::string, std::string> > templateParameters, /*!< Parameter map of the tile request to get */
            shared_ptr<AsyncCallbackWithRequest<shared_ptr<std::map<std::string, std::string> >, TilePtr> > callback, /*!< An AsyncCallbackWithRequest object with a parameter
                                                                                                                                 map for request and a Tile object for response */
            uint32 priority,                                                    /*!< Priority of tile request. Lower number has
                                                                                     higher priority. Zero is highest priority. */
            bool fastLoadOnly = false                                           /*!< Is the tile only loaded in cache? */
            ) = 0;

    /*! Cancel Pending tile requests.

        Indicate to tile manager that the client is no longer interested in any previously requested tiles
        Pending tile requests will be cancelled.

        @return None
    */
    virtual void
    RemoveAllTiles() = 0;


    /*! Set values that do not change for URL Args templates

        Values like tile version do not change from one request to
        another. This map will be used to set al such values
        that remain same across tile requests.

        @return None
    */
    virtual NB_Error
    SetCommonParameterMap(shared_ptr<std::map<std::string, std::string> > commonParameters   /*!< Map for the Args Template. Key and value is like:
                                                                                                     $fmt, PNG
                                                                                                     $vfmt, 30

                                                                                                  It can be NULL if there is no common
                                                                                                  parameters.
                                                                                                */
                          ) = 0;

    /*! Update the layer information

        @return None
     */
    virtual void UpdateTileLayerInfo(TileLayerInfoPtr info) = 0;   /*!< layer information. */


    virtual std::string str() const = 0;
};

typedef shared_ptr<TileManager > TileManagerPtr;

}
#endif
/*! @} */
