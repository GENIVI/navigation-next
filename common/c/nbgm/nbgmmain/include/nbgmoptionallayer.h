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

    @file nbgmoptionallayer.h
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
#ifndef _NBGM_OPTIONAL_LAYER_H_
#define _NBGM_OPTIONAL_LAYER_H_
#include "paltypes.h"
#include <vector>
#include <string>
#include "nbgmbinarybuffer.h"
#include "smartpointer.h"

/*! \addtogroup NBRE_Service
*  @{
*/

/*! This struct will contain the content ID and the associated
    data stream with that content ID. 
*/
struct NBGM_OptionalLayerData
{
    shared_ptr<std::string> contentId;  // unique tile id
    NBGM_BinaryBuffer dataStream;       // tile data
};

/*! This class is used to inform NBGM that it needs to start running an animation.
    All the information about animation frames and animation times will be passed 
    in the interfaces mentioned below.
*/
class NBGM_OptionalLayer
{
public:
    virtual ~NBGM_OptionalLayer() {}
 
    /*! Use for load a group of optional tile data to NBGM.

        @param tiles        Tile data to load.
        @param baseDrawOrder Base draw order of this tile
        @param labelDrawOrder Label draw order of this tile
        @return PAL_Error
    */
    virtual PAL_Error LoadTiles(const std::vector<NBGM_OptionalLayerData>& tiles, uint8 baseDrawOrder, uint8 labelDrawOrder) = 0;

    /* Unload tiles.
 
        This call should not affect any running animation!

        @param tiles        tile data to unload.
        @return PAL_Error
    */
    virtual PAL_Error UnloadTiles(const std::vector< shared_ptr<std::string> >& contentIds) = 0;
 
    /*! Use for show one layer.

        @return none
    */
    virtual void Show() = 0;
 
    /*! Use for hide one layer but not unload any tiles which has already load in NBGM.
 
        @param frameIndex   Frame index of frame to play. Starting from 0.
        @return none
    */
    virtual void Hide() = 0;
};

/*! @} */

#endif
