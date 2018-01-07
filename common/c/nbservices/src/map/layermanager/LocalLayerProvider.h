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
  @file        LocalLayerProvider.h
  @defgroup    map

  Description: Provider of layers generated locally.
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

#ifndef _LOCALLAYERPROVIDER_H_
#define _LOCALLAYERPROVIDER_H_

extern "C"
{
#include "paltypes.h"
#include "pal.h"
#include "paltimer.h"
#include "palclock.h"
#include "nberror.h"
}
#include "LayerProvider.h"
#include "AggregateLayerProvider.h"
#include "nbcontext.h"

namespace nbmap
{

class AggregateLayerProvider;

class LocalLayerProvider : public LayerProvider,
                           public Base
{
public:
    LocalLayerProvider(NB_Context* context,                       /*!< Pointer to current context */
                     AggregateLayerProvider* parentLayerProvider/*!< Parent layer provider for updating layers */
                     );

    virtual ~LocalLayerProvider();

    /*See Description in LayerProvider.h */
    virtual void GetLayers(shared_ptr<AsyncCallback<const vector<LayerPtr>& > > callback,
                           shared_ptr<MetadataConfiguration> metadataConfig);
    virtual void ClearLayers();

    /*! See description in LayerManager.h */
    virtual const vector<LayerPtr>& GetCommonMaterialLayers() const;
    virtual const vector<LayerPtr>& GetLayerPtrVector() const;
    virtual const vector<LayerPtr>& GetOptionalLayerPtrVector() const;
    virtual const LayerPtr GetLayerAvailabilityMatrix() const;
    virtual const LayerPtr GetDAMLayer() const;

    virtual NB_Error SetCachePath(shared_ptr<string> cachePath);

    virtual void SetMetadataRetryTimes(int number);
    virtual NB_Error SetPersistentMetadataPath(shared_ptr<string> metadataPath);
    virtual NB_Error GetBackgroundRasterRange(uint32& minZoomLevel,
                                              uint32& maxZoomLevel);
    virtual void SetPreferredLanguageCode(uint8 languageCode);
    virtual void SetTheme(MapViewTheme theme);
    vector<LayerPtr> GetLayerPtrVectorFromParent();
    vector<LayerPtr> GetOptionalLayerPtrVectorFromParent();

    /*! Add a layer

        @return true if need to notify user to update layers, false otherwise.
    */
    bool
    AddLayer(LayerPtr layer     /*!< A layer to add */
             );

    /*! Remove a layer

        @return true if need to notify user to update layers, false otherwise.
    */
    bool
    RemoveLayer(LayerPtr layer  /*!< A layer to remove */
                );

    NB_Context* GetContext();

private:
    NB_Context* m_pContext;                        /*!< Pointer to current context */
    AggregateLayerProvider* m_parentLayerProvider; /*!< Parent layer provider for updating layers */
    vector<LayerPtr> m_pLayers;         /*!< cached layer list */
    vector<LayerPtr> m_pOptionalLayers; /*!< cached optional layer list */
    vector<LayerPtr> m_dummyCommonMaterialLayers;
    LayerPtr m_dummyLayer;
};

typedef bool (LocalLayerProvider::*LayerOperator)(LayerPtr) ;
typedef shared_ptr<LocalLayerProvider>   LocalLayerProviderPtr;
};

#endif /* _LOCALLAYERPROVIDER_H_ */
/*! @} */
