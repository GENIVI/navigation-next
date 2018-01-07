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

    @file     DAMConfiguration.cpp
    See header file for description.

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

extern "C"
{
#include "nbcontextprotected.h"
}
#include "DAMConfiguration.h"
#include "contextbasedsingleton.h"
#include "DAMUtilityImpl.h"
#include "AggregateLayerProvider.h"
#include "MapView.h"
#include "UnifiedLayerProvider.h"
#include "Tuple.h"
#include "UnifiedLayerManager.h"

static const uint32 MAXIMUM_CACHING_TILE_COUNT          = 1000;
static const uint32 MAXIMUM_TILE_REQUEST_COUNT_PERLAYER = 64;

using namespace nbmap;

DAMUtilityPtr
DAMConfiguration::GetDAMUtility(NB_Context* context, shared_ptr<LayerManager> layerManager)
{
    // This has to be called in the context of the CCC thread!
    NB_ASSERT_CCC_THREAD(context);

    DAMUtilityPtr damUtility;

    do
    {
        if (!context)
        {
            break;
        }

        if (!layerManager)
        {
            layerManager = GetLayerManager(context, MAXIMUM_CACHING_TILE_COUNT, MAXIMUM_TILE_REQUEST_COUNT_PERLAYER);

            if (!layerManager)
            {
                break;
            }
        }

        // Get the instance of offboard layer provider.
        nbcommon::Tuple<TYPELIST_1(LayerManagerPtr)> otherParameters = nbcommon::MakeTuple(layerManager);

        damUtility = ContextBasedSingleton<DAMUtilityImpl>::getInstance(context, otherParameters);
    } while (0);

    return damUtility;
}

/* See header file for description */
LayerManagerPtr
DAMConfiguration::GetLayerManager(NB_Context* context,
                                  uint32 maximumCachingTileCount,
                                  uint32 maximumTileRequestCountPerLayer)
{
    if (!context)
    {
        return LayerManagerPtr();
    }

    LayerManagerPtr layerManager;

    // Get the instance of layer ID generator.
    LayerIdGeneratorPtr layerIdGenerator = ContextBasedSingleton<LayerIdGenerator>::getInstance(context);
    if (!layerIdGenerator)
    {
        return LayerManagerPtr();
    }

    // Get the version of the NBGM library.
    uint32 nbgmVersion = MapView::GetNBGMVersion();

    // Get the instance of unified layer provider.
    nbcommon::Tuple<TYPELIST_4(uint32, uint32, uint32, nbcommon::HybridMode)> otherParameters = nbcommon::MakeTuple(nbgmVersion,
                                                                                              maximumTileRequestCountPerLayer,
                                                                                              maximumCachingTileCount,
                                                                                              nbcommon::HBM_OFFBOARD_ONLY);
    UnifiedLayerProviderPtr layerProvider = ContextBasedSingleton<UnifiedLayerProvider>::getInstance(context,
                                                                                                      otherParameters);
    if (!layerProvider)
    {
        return LayerManagerPtr();
    }

    // Set the layer ID generator to the unified layer provider.
    layerProvider->SetLayerIdGenerator(layerIdGenerator);

    // Create and initialize an aggregate layer provider.
    shared_ptr<AggregateLayerProvider> aggregateLayerProvider(new AggregateLayerProvider(context,
                                                                                         nbgmVersion));
    if (!aggregateLayerProvider)
    {
        return LayerManagerPtr();
    }
    aggregateLayerProvider->Initialize();

    // Add the unified layer provider to the aggregate layer provider.
    aggregateLayerProvider->AddLayerProvider(layerProvider);

    // Create the layer manager.
    layerManager = LayerManagerPtr(new UnifiedLayerManager(aggregateLayerProvider,
                                                           context));
    if (!layerManager)
    {
        return LayerManagerPtr();
    }

    NB_ASSERT_CCC_THREAD(context);
    layerManager->GetLayers(shared_ptr<AsyncCallback<const vector<LayerPtr>& > >(),
                            shared_ptr<MetadataConfiguration>());

    return layerManager;
}

/*! @} */
