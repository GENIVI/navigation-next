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
    @file     MapViewTileCallback.h
*/
/*
    (C) Copyright 2012 by TeleCommunication Systems, Inc.                

    The information contained herein is confidential, proprietary 
    to TeleCommunication Systems, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of TeleCommunication Systems, is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/
/*! @{ */

#ifndef _MAP_VIEW_TILE_CALLBACK_
#define _MAP_VIEW_TILE_CALLBACK_

#include "AsyncCallback.h"
#include "Tile.h"
#include "TileKey.h"

/*
    Disable warning C4512: "assignment operator could not be generated"
    This is due to the 'const' members of some of the classes. Assignment operators are not needed for these classes.
*/
#pragma warning(disable:4512)

namespace nbmap 
{
class MapView;
    
/*! Callback class used to handle tile requests.
 
    All tile requests get forwarded to the MapView.
 */
class MapViewTileCallback : public AsyncCallback<TilePtr>
{
public:
    MapViewTileCallback() : m_pMapView(NULL){};
    virtual ~MapViewTileCallback(){};
    
    /*! Has to be called before using the class */
    void SetMapView(MapView* pMapView);
    
private:
    
    // AsyncCallback functions ..............................................................................
    
    /* See AsyncCallback.h for description */
    virtual void Success(TilePtr response);
    virtual void Error(NB_Error error);
    
    MapView* m_pMapView;    
};
    
    
/*! Callback class used to handle common material update requests.
     
    All tile requests get forwarded to the MapView.
*/
class MapViewCommonMaterialRequestCallback : public AsyncCallbackWithRequest<TileKeyPtr, TilePtr>
{
public:
    MapViewCommonMaterialRequestCallback() : m_pMapView(NULL){};
    virtual ~MapViewCommonMaterialRequestCallback(){};

    /*! Has to be called before using the class */
    void SetMapView(MapView* pMapView);

private:
        
    // AsyncCallbackWithRequest functions ..............................................................................
        
    /* See AsyncCallback.h for description */
    virtual void Success(TileKeyPtr request, TilePtr response);
    virtual void Error(TileKeyPtr request, NB_Error error);
        
    MapView* m_pMapView;    
};    
    
/*! Callback class used to handle Async-GenerateMapImage requests

    All tile requests get forwarded to the MapView.
 */
class AsyncGenerateMapImageTileCallback : public AsyncCallback<TilePtr>
{
public:
    AsyncGenerateMapImageTileCallback(MapView* pMapView, shared_ptr<bool> screenshotInProgress, const shared_ptr<AsyncCallback<shared_ptr<nbcommon::DataStream> > >  callback
                                      );
    virtual ~AsyncGenerateMapImageTileCallback();

    // AsyncCallback functions ..............................................................................

    /* See AsyncCallback.h for description */
    virtual void Success(TilePtr response);
    virtual void Error(NB_Error error);

    const shared_ptr<AsyncCallback<shared_ptr<nbcommon::DataStream> > > m_callback;
    shared_ptr<bool> m_pScreenshotInProgress;  /*!< Flag to indicate if Current the Callback is Running. */
    MapView* m_pMapView;
};

}

#endif

/*! @} */ 
