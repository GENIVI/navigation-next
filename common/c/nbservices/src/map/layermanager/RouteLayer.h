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
   @file        RouteLayer.h
   @defgroup    nbmap

   Description: RouteLayer is a special unified layer. It does not use the
   layer created by OffboardLayerProvider to download tiles, but use that
   layer as a template to generate route layer instances. Then fill that route
   layer instance with rid/rc. And use this generated layer to download
   tiles. Those generated route layers instances is bond to each instance of
   RouteManager, thus make it possible for each MapView to have its own
   RouteLayers.
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

#ifndef _ROUTELAYER_H_
#define _ROUTELAYER_H_


extern "C"
{
#include "nbrouteinformation.h"
}
#include "RefreshLayer.h"

namespace nbmap
{

/*! Encoded route Id and route Color. */
typedef struct _EncodedRouteInfo
{
    string encodedRouteId;              /*!< Encoded route Identifier */
    string encodedRouteColor;           /*!< Encoded route color */
} EncodedRouteInfo;

class RouteManager;

class RouteLayer : public RefreshLayer
{
public:
    /*! Default Constructor*/
    RouteLayer(TileManagerPtr tileManager, /*!< Tile Manager instance */
               uint32 layerIdDigital,      /*!< Identifier of assigned to this layer*/
               NB_Context* context         /*!< Pointer of  NB_context instance */
               );
    /*! Deconstructor */
    virtual ~RouteLayer();

    /*! Allocates new RouteLayer and assign new layerId to it.

        @return Pointer of newly created RouteLayer.
    */
    virtual UnifiedLayerPtr Clone();

    /*! Set route information for this route layer.

        @return NB_OK if succeeded.
    */
    NB_Error SetRouteInfo(const vector<EncodedRouteInfo>& encodedInfo);

    /*! Check whether this layer is created by RouteManager or not.

        RouteLayer created by OffboardLayerProvider should be used as a template to create
        new RouteLayers for RouteManager. This function can be used by RouteManager to find
        out whether a routeLayer can be used as template or not.

        @return true if created by RouteManager.
    */
    bool   IsCreatedByOffboardLayerManager();

    /*! Copy Constructor.*/
    RouteLayer(const RouteLayer& routeLayer);

    virtual std::string className() const { return "RouteLayer"; }

private:

    /*! Append tile/layer specific parameters into templateParameters.

        @return None.
    */
    virtual void
    AppendSpecialTemplateParamters(shared_ptr<map<string, string> > templateParameters);

    map<string, string> m_routeParameters; /*!< A map of received route parameters */
    bool m_createdByOffboardLayerProvider; /*!< Flag to indicate whether this instance is
                                                created by OffboardLayerProvider or not.
                                                Layers created by OffboardLayerProvider
                                                should be used as template to create new
                                                RouteLayer Instances. */
};

}

#endif /* _ROUTELAYER_H_ */

/*! @} */
