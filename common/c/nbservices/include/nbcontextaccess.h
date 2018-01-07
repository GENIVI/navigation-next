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

    @file     nbcontext.h
    @defgroup nbcontext Context

    This API is used to manage the NAVBuilder context
*/
/*
    (C) Copyright 2014 by TeleCommunication Systems, Inc.                

    The information contained herein is confidential, proprietary 
    to TeleCommunication Systems, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of TeleCommunication Systems is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

#ifndef NBCONTEXTACCESS_H
#define NBCONTEXTACCESS_H

#include "nbcontext.h"
#include "nbanalytics.h"
#include "nbwifiprobes.h"
#include "nbgpshistory.h"
#include "cslcache.h"
#include "nbvectortilemanager.h"
#include "nbenhancedcontentmanager.h"
#include "nbenhancedvectormapmanager.h"
#include "nbguidanceprocessor.h"

/*! @{ */

#define MAX_TARGET_LENGTH 64

typedef struct
{
    char    key[MAX_TARGET_LENGTH];
    char    value[MAX_TARGET_LENGTH];
} NB_TargetMapping;

/*! Associate a GPS History instance with the context

Associate a GPS History instance with the context. Ownership remains with the
client and it's the client's responsibility to clean up using
NB_ContextRegisterCallback. To disable GPS History, pass a NULL GPS History.

@param context The context to set the GPS History for
@param history The GPS History to set, NULL to disable
@returns NB_Error
*/
NB_DEC NB_Error NB_ContextSetGpsHistoryNoOwnershipTransfer(NB_Context* context, NB_GpsHistory* history);


/*! Get the GPS History from the context

@param context The context to get the GPS History from
@returns The GPS History associated with this context
*/
NB_DEC NB_GpsHistory* NB_ContextGetGpsHistory(NB_Context* context);

/*! Associate a Vector Tile Manager instance with the context

Associate a Vector Tile Manager instance with the context. Ownership remains with the
client and it's the client's responsibility to clean up using
NB_ContextRegisterCallback. To disable Vector Tile Manager, pass a NULL Vector Tile Manager.

@param context The context to set the Vector Tile Manager for
@param manager The Vector Tile Manager to set, NULL to disable
@returns NB_Error
*/
NB_DEC NB_Error NB_ContextSetVectorTileManagerNoOwnershipTransfer(NB_Context* context, NB_VectorTileManager* manager);


/*! Get the Vector Tile Manager from the context

@param context The context to get the Vector Tile Manager from
@returns The Vector Tile Manager associated with this context
*/
NB_DEC NB_VectorTileManager* NB_ContextGetVectorTileManager(NB_Context* context);


/*! Associate a Enhanced Content Manager instance with the context. Ownership remains with the
client and it's the client's responsibility to clean up using
NB_ContextRegisterCallback. To disable Enhanced Content Manager, pass a NULL Enhanced Content Manager.

@param context The context to set the Enhanced Content Manager for
@param enhancedContentManager The enhanced content manager to set
@returns The enhanced content manager associated with the content, NULL if none
*/
NB_DEC NB_Error NB_ContextSetEnhancedContentManagerNoOwnershipTransfer(NB_Context* context, NB_EnhancedContentManager* enhancedContentManager);


/*! Get the enhanced content manager from the context

@param context The context to get the enhanced content manager instance from
@returns NB_Error
*/
NB_DEC NB_EnhancedContentManager* NB_ContextGetEnhancedContentManager(NB_Context* context);

/*! Get the voice cache from the context

@param context The context to get the voice cache from
@returns The voice cache associated with this context
*/
NB_DEC CSL_Cache* NB_ContextGetVoiceCache(NB_Context* context);


/*! Get the raster tile manager cache from the context

@param context The context to get the raster tile manager cache from
@returns The raster tile manger cache associated with this context
*/
NB_DEC CSL_Cache* NB_ContextGetRasterTileManagerCache(NB_Context* context);


/*! Get the persistent data object from the context

@param context The context to get the persistent data from
@returns The persistent data object associated with this context
*/
NB_DEC NB_PersistentData* NB_ContextGetPersistentData(NB_Context* context);


/*! Set target mappings

@param context The context to set target mappings for
@param mapping An array of NB_TargetMapping
@param mappingCount The number of elements in the mapping array
@returns The persistent data object associated with this context
*/
NB_DEC NB_Error NB_ContextSetTargetMappings(NB_Context* context, NB_TargetMapping* mapping, nb_size mappingCount);

/*! Associate a Enhanced Vector Map Manager instance with the context. Ownership remains with the
	client and it's the client's responsibility to clean up using
	NB_ContextRegisterCallback. To disable Enhanced Vector Map Manager, pass a NULL Enhanced Vector Map Manager.

    @param context The context to set the enhanced vector map manager for
    @param enhancedVectorMapManager The enhanced vector map manager to set
    @returns The enhanced content manager associated with the content, NULL if none
*/
NB_DEC NB_Error NB_ContextSetEnhancedVectorMapManagerNoOwnershipTransfer(NB_Context* context, NB_EnhancedVectorMapManager* enhancedVectorMapManager);


/*! Get the enhanced vector map manager from the context

    @param context The context to get the enhanced vector map manager instance from
    @returns NB_Error
*/
NB_DEC NB_EnhancedVectorMapManager* NB_ContextGetEnhancedVectorMapManager(NB_Context* context);

/*! Associate an pointer to snippet length callback with the context

    @param 'context' The context to set the enhanced vector map manager for
    @param 'callback' callback pointer to set
    @param 'callbackData' callback data pointer to set
    @returns NB_Error
*/
NB_DEC NB_Error NB_ContextSetSnippetLengthCallback(NB_Context* context, NB_GetAnnounceSnippetLength callback, void* userData);


/*! Get the enhanced vector map manager from the context

    @param 'context' The context to get pointer to callback from
    @param 'callback' callback pointer to get
    @param 'callbackData' callback data pointer to get
    @returns NB_Error
*/
NB_DEC NB_Error NB_ContextGetSnippetLengthCallback(NB_Context* context, NB_GetAnnounceSnippetLength* callback, void** userData);

/*! Associate an Analytics instance with the context

Associate a Analytics instance with the context. Ownership remains with the
client and it's the client's responsibility to clean up using
NB_ContextRegisterCallback. To disable Analytics, pass a NULL.

@param context The context to set the Device Analytics for
@param analytics Optional. The Analytics
@returns NB_Error
*/
NB_DEC NB_Error NB_ContextSetAnalyticsNoOwnershipTransfer(NB_Context* context, NB_Analytics* analytics);

/*! Get the QA Log from the context

@param context The context to get the QA Log from
@returns The QA Log associated with this context
*/
NB_DEC NB_Analytics* NB_ContextGetAnalytics(NB_Context* context);

/*! Associate an WifiProbes instance with the context

Associate a WifiProbes instance with the context. Ownership remains with the
client and it's the client's responsibility to clean up using
NB_ContextRegisterCallback. To disable WifiProbes, pass a NULL.

@param context The context to set the WifiProbes for
@param wifiProbes Optional. The WifiProbes
@returns NB_Error
*/
NB_DEC NB_Error NB_ContextSetWifiProbesNoOwnershipTransfer(NB_Context* context, NB_WifiProbes* wifiProbes);


/*! Get the WifiProbes from the context

@param context The context to get the WifiProbes from
@returns The WifiProbes associated with this context
*/
NB_DEC NB_WifiProbes* NB_ContextGetWifiProbes(NB_Context* context);
/*! @} */


#endif
