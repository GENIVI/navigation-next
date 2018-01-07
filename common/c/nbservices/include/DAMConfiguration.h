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

    @file     DAMConfiguration.h
    Class DAMConfiguration is used to create instance for
    DAM service, for example to check the map data.

*/
/*
    (C) Copyright 2013 by Networks In Motion, Inc.

    The information contained herein is confidential, proprietary
    to Networks In Motion, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

#ifndef DAMCONFIGURATION_H
#define DAMCONFIGURATION_H

/*!
    @addtogroup nbmap
    @{
*/

#include "DAMUtility.h"

namespace nbmap
{

class LayerManager;

class DAMConfiguration
{
public:
    virtual ~DAMConfiguration(){}

    /*! Get an instance of DAMUtility.

        @note: If parameter `layerManager` not specified, we will create LayerManager
        internally. If it is provided, DAM will use the LayerManager to retrieve DAM
        information, so it is user's responsibility to ensure the LayerManager is valid if
        user want to specify LayerManager of their own for some reason.

        @return A shared pointer to an instance of dam interface, empty pointer otherwise.
     */
    static DAMUtilityPtr
    GetDAMUtility(NB_Context* context,
                  shared_ptr<LayerManager> layerManager = shared_ptr<LayerManager>());
private:
    /*! Get an instance of layer manager

        @return A shared pointer to an instance of layer manager, empty pointer otherwise.
    */
    static shared_ptr<LayerManager>
    GetLayerManager(NB_Context* context,                    /*!< Pointer to current context */
                    uint32 maximumCachingTileCount,         /*!< Number of maximum count of saved tiles in cache */
                    uint32 maximumTileRequestCountPerLayer  /*!< Number of maximum count of tile requests for each layer */
                    );
};

}; //namespace nbmap

#endif /* DAMCONFIGURATION_H */

/*! @} */
