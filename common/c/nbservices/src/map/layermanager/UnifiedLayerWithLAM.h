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
   @file        UnifiedLayerWithLAM.h
   @defgroup    nbmap

   Description: UnifiedLayerWithLAM is layers that needs help from
   LayerAvailabilityMatrixLayer to decide if a tile should be downloaded from
   server.
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
#ifndef _UNIFIEDLAYERWITHLAM_H_
#define _UNIFIEDLAYERWITHLAM_H_

#include "UnifiedLayer.h"

namespace nbmap
{
class UnifiedLayerWithLAM : public UnifiedLayer
{
public:
    UnifiedLayerWithLAM(TileManagerPtr tileManager,
                        uint32 layerIdDigital,
                        NB_Context* context,
                        const shared_ptr<UnifiedLayer>* lamLayer);
    UnifiedLayerWithLAM(const UnifiedLayerWithLAM& layer);
    virtual ~UnifiedLayerWithLAM();


    virtual shared_ptr<UnifiedLayer> Clone();

    /*! Start the downloading from server.

        This function will only be called in the following cases:
        1. When LAM layer is not available, or
        2. When both LAM Layer is available and the LAM Layer indicates tile for this
           TileKey in this layer is available.

        @return None
    */
    void StartRealDownload(TileKeyPtr tileKey, TileKeyPtr convertedTileKey,
                           AsyncTileRequestWithRequestPtr callback,
                           uint32 priority);
    virtual std::string className() const { return "UnifiedLayerWithLAM"; }

private:

    /*! Start tile request for a tileKey.

        It will start download tiles with the help of LAM layer if LAM layer is available,
        or start download directly if LAM layer is not available.

        @return None
    */
    virtual void GetTile(TileKeyPtr tileKey, TileKeyPtr convertedTileKey,
                         AsyncTileRequestWithRequestPtr callback,
                         uint32 priority, bool cachedOnly = false);

    const shared_ptr<UnifiedLayer>* m_pLamLayer; /*!< Pointer of LAM Layer instance  */

    /*!< Flag to indicate if this layer is valid.

         This layer depends on LAM layer, and tile requests will be stored temporarily
         until LAM tile is downloaded. But sometimes, this layer may be destroyed before LAM
         tile is returned from server. This flag will be shared by tile requests to indicate
         if this layer is still valid. If it is not valid anymore, LAM Layer should not tell
         this layer to start download tiles.
    */
    shared_ptr<bool> m_isValid;
};

}

#endif /* _UNIFIEDLAYERWITHLAM_H_ */

/*! @} */
