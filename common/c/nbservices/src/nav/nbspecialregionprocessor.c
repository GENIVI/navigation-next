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
 @file     nbspecialregionprocessor.c
 */
/*
 (C) Copyright 2014 by TeleCommunication Systems, Inc.

 The information contained herein is confidential, proprietary
 to TeleCommunication Systems, Inc., and considered a trade secret as
 defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of TeleCommunication Systems, is granted only
 under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.

 ---------------------------------------------------------------------------*/
/*! @{ */

#include "nbspecialregionprocessorprivate.h"
#include "nbenhancedcontentmanagerprotected.h"
#include "nbcontextaccess.h"
#include "nbrouteinformation.h"
#include "nbrouteinformationprivate.h"

/*! Verify if the specified special region with index has already in current state.

 @return TRUE for in, FALSE for not in
 */
static nb_boolean
HasAlreadyInCurrentState(NB_SpecialRegionState* specialRegionState, uint32 chechIndex, int32* foundIndex)
{
    int32 i = 0;
    int32 regionLength = 0;
    if (!specialRegionState || !specialRegionState->currentRegions)
    {
        return FALSE;
    }

    regionLength = CSL_VectorGetLength(specialRegionState->currentRegions);
    for (i = 0; i < regionLength; i++)
    {
        NB_SpecialRegionStateItem* item = (NB_SpecialRegionStateItem*)CSL_VectorGetPointer(specialRegionState->currentRegions, i);
        if (item)
        {
            if (item->index == chechIndex)
            {
                if (foundIndex)
                {
                    *foundIndex = i;
                }
                return TRUE;
            }
        }
    }
    return FALSE;
}

static void
AlertSpecialRegionStateChanged(NB_SpecialRegionState* specialRegionState, NB_SpecialRegionInformation* information, NB_NavigationSpecialRegionCallbackFunction callback, void* userData)
{
    int32 i = 0;
    int32 j = 0;
    int32 currentLength = 0;
    int32 regionLength = 0;
    NB_EnhancedContentManager* manager = NULL;

    if (!specialRegionState || !specialRegionState->context || !information || !callback)
    {
        return;
    }

    manager = NB_ContextGetEnhancedContentManager(specialRegionState->context);

    currentLength = CSL_VectorGetLength(specialRegionState->currentRegions);
    regionLength = CSL_VectorGetLength(information->specialRegions);
    for (i = 0; i < currentLength; i++)
    {
        NB_SpecialRegionStateItem* stateItem = (NB_SpecialRegionStateItem*)CSL_VectorGetPointer(specialRegionState->currentRegions, i);
        if (stateItem && !stateItem->isAlerted)
        {
            stateItem->isAlerted = TRUE;
            for (j = 0; j < regionLength; j++)
            {
                NB_SpecialRegion* region = (NB_SpecialRegion*)CSL_VectorGetPointer(information->specialRegions, j);
                if (region && region->index == stateItem->index)
                {
                    NB_SpecialRegionStateData stateData = {0};
                    if (region->regionType)
                    {
                        stateData.regionType = nsl_strdup(region->regionType);
                    }
                    if (region->regionDescription)
                    {
                        stateData.regionDescription = nsl_strdup(region->regionDescription);
                    }
                    stateData.showSpecialRegion = stateItem->ifIn;
                    stateData.startManeuverIndex = region->startManeuver;
                    stateData.startManeuverOffset = region->startManeuverOffset;
                    stateData.endManeuverIndex = region->endManeuver;
                    stateData.endManeuverOffset = region->endManeuverOffset;
                    stateData.nextOccurStartTime = region->nextOccurStartTime;
                    stateData.nextOccurEndTime = region->nextOccurEndTime;
                    if (stateItem->ifIn && manager)
                    {
                        NB_EnhancedContentManagerGetDataItem(manager, NB_ECDT_SpeedLimitsSign, region->datasetid,
                                                             region->signId, region->version, FALSE,
                                                             &stateData.imageDataSize, &stateData.imageData);
                    }
                    callback(&stateData, userData);
                    if (stateData.imageData)
                    {
                        nsl_free(stateData.imageData);
                    }
                    if (stateData.regionType)
                    {
                        nsl_free(stateData.regionType);
                    }
                    if (stateData.regionDescription)
                    {
                        nsl_free(stateData.regionDescription);
                    }
                    break;
                }
            }
        }
    }

    i = 0;
    while(currentLength = CSL_VectorGetLength(specialRegionState->currentRegions), i < currentLength)
    {
        for (i = 0; i < currentLength; i++)
        {
            NB_SpecialRegionStateItem* stateItem = (NB_SpecialRegionStateItem*)CSL_VectorGetPointer(specialRegionState->currentRegions, i);
            if (stateItem && !stateItem->ifIn)
            {
                CSL_VectorRemove(specialRegionState->currentRegions, i);
                break;
            }
        }
    }
}

NB_DEF NB_Error
NB_SpecialRegionStateCreate(NB_Context* context, NB_SpecialRegionState** specialRegionState)
{
    NB_SpecialRegionState* pThis = NULL;
    if (!context || !specialRegionState)
    {
        return NE_INVAL;
    }

    pThis = (NB_SpecialRegionState*)nsl_malloc(sizeof(NB_SpecialRegionState));
    if (!pThis)
    {
        return NE_NOMEM;
    }
    nsl_memset(pThis, 0, sizeof(NB_SpecialRegionState));
    pThis->context = context;

    pThis->currentRegions = CSL_VectorAlloc(sizeof(NB_SpecialRegionStateItem));
    if (!pThis->currentRegions)
    {
        nsl_free(pThis);
        return NE_NOMEM;
    }

    *specialRegionState = pThis;
    return NE_OK;
}

NB_DEF NB_Error
NB_SpecialRegionStateDestroy(NB_SpecialRegionState* specialRegionState)
{
    if (!specialRegionState || !specialRegionState->context)
    {
        return NE_INVAL;
    }

    if (specialRegionState->currentRegions)
    {
        CSL_VectorDealloc(specialRegionState->currentRegions);
    }

    nsl_free(specialRegionState);
    return NE_OK;
}

NB_DEF NB_Error
NB_SpecialRegionProcessorDownloadPicture(NB_RouteInformation* route, NB_SpecialRegionState* specialRegionState, NB_SpecialRegionInformation* information)
{
    NB_EnhancedContentManager* manager = NULL;
    int32 i = 0;
    int32 regionLength = 0;

    if (!route || !specialRegionState || !specialRegionState->context || !information)
    {
        return NE_INVAL;
    }

    manager = NB_ContextGetEnhancedContentManager(specialRegionState->context);
    if (!manager)
    {
        return NE_OK;
    }

    regionLength = CSL_VectorGetLength(information->specialRegions);
    for (i = 0; i < regionLength; i++)
    {
        NB_SpecialRegion* specialRegion = (NB_SpecialRegion*)CSL_VectorGetPointer(information->specialRegions, i);
        if (!specialRegion)
        {
            continue;
        }

        if (!NB_EnhancedContentManagerIsDataAvailable(manager, NB_ECDT_SpecialRegions, specialRegion->datasetid,
                                                      specialRegion->signId) ||
            NB_EnhancedContentManagerCheckFileVersion(manager, NB_ECDT_SpecialRegions, specialRegion->datasetid,
                                                      specialRegion->signId, specialRegion->version) != NE_OK)
        {
            NB_EnhancedContentManagerRequestDataDownload(manager, NB_ECDT_SpecialRegions, specialRegion->datasetid,
                                                         specialRegion->signId, specialRegion->version);
        }
    }
    return NE_OK;
}

NB_DEF NB_Error
NB_SpecialRegionProcessorUpdate(NB_SpecialRegionState* specialRegionState, NB_RouteInformation* route, NB_SpecialRegionInformation* information,
                                NB_PositionRoutePositionInformation* currentPosition, NB_NavigationSpecialRegionCallbackFunction callback, void* userData)
{
    int32 i = 0;
    int32 regionLength = 0;
    int32 foundIndex = 0;
    double offsetInManeuver = 0;
    data_nav_maneuver* maneuver = NULL;
    nb_boolean isInRegion = FALSE;
    uint32 maneuverIndex = 0;

    if (!specialRegionState || !specialRegionState->context || !route || !information || !currentPosition)
    {
        return NE_INVAL;
    }

    if (currentPosition->closestManeuver == NAV_MANEUVER_NONE || currentPosition->closestManeuver == NAV_MANEUVER_NONE)
    {
        return NE_OK;
    }

    maneuverIndex = currentPosition->closestManeuver;

    maneuver = NB_RouteInformationGetManeuver(route, maneuverIndex);
    if (!maneuver)
    {
        return NE_NOENT;
    }

    offsetInManeuver = maneuver->distance - currentPosition->maneuverDistanceRemaining;

    regionLength = CSL_VectorGetLength(information->specialRegions);
    for (i = 0; i < regionLength; i++)
    {
        NB_SpecialRegion* specialRegion = (NB_SpecialRegion*)CSL_VectorGetPointer(information->specialRegions, i);
        isInRegion = FALSE;
        if (!specialRegion)
        {
            continue;
        }

        if (maneuverIndex > specialRegion->startManeuver&&
            maneuverIndex < specialRegion->endManeuver)
        {
            isInRegion = TRUE;
        }
        else if (maneuverIndex == specialRegion->startManeuver &&
                 maneuverIndex == specialRegion->endManeuver)
        {
            if (offsetInManeuver >= specialRegion->startManeuverOffset &&
                offsetInManeuver <= specialRegion->endManeuverOffset)
            {
                isInRegion = TRUE;
            }
        }
        else if (maneuverIndex == specialRegion->startManeuver)
        {
            if (offsetInManeuver >= specialRegion->startManeuverOffset)
            {
                isInRegion = TRUE;
            }
        }
        else if (maneuverIndex == specialRegion->endManeuver)
        {
            if (offsetInManeuver <= specialRegion->endManeuverOffset)
            {
                isInRegion = TRUE;
            }
        }
        if (HasAlreadyInCurrentState(specialRegionState, i, &foundIndex))
        {
            // remove region from state
            if (!isInRegion)
            {
                NB_SpecialRegionStateItem* stateItem = (NB_SpecialRegionStateItem*)CSL_VectorGetPointer(specialRegionState->currentRegions, foundIndex);
                if (stateItem)
                {
                    stateItem->ifIn = FALSE;
                    stateItem->isAlerted = FALSE;
                }
            }
        }
        else
        {
            // add region to state
            if (isInRegion)
            {
                NB_SpecialRegionStateItem stateItem = {0};
                stateItem.index = i;
                stateItem.ifIn = TRUE;
                stateItem.isAlerted = FALSE;
                if (!CSL_VectorAppend(specialRegionState->currentRegions, &stateItem))
                {
                    return NE_NOMEM;
                }
            }
        }
    }

    AlertSpecialRegionStateChanged(specialRegionState, information, callback, userData);
    return NE_OK;
}

/*! @} */
