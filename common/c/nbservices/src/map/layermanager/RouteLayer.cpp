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
   @file        RouteLayer.cpp
   @defgroup    nbmap

   Description: Implementation of RouteLayer.

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

#include "RouteLayer.h"
#include "nbcontextprotected.h"
#include "StringUtility.h"

using namespace nbmap;
using namespace std;

// Parameter keys to request a tile by the tile manager
static const string PARAMETER_KEY_ROUTE_WIDTH("$routewidth");
static const char DEFAULT_ROUTE_WIDTH[] = "routewidth";

// Default interval (in seconds) used to refresh route
// @todo: This is hard-coded at present, I think this should be returned from server, just
// as what happens for traffic layers.
static const uint32 DEFAULT_REFRESH_SECONDS = 300;

/* See description in header file. */
RouteLayer::RouteLayer(TileManagerPtr tileManager, uint32 layerIdDigital,
                       NB_Context* context)
        : RefreshLayer(tileManager, layerIdDigital, DEFAULT_REFRESH_SECONDS, context)
{
    m_createdByOffboardLayerProvider = true;
    m_isOverlay                      = true;
}

/* See description in header file. */
RouteLayer::RouteLayer(const RouteLayer& routeLayer)
        : RefreshLayer(routeLayer),
          m_createdByOffboardLayerProvider(false)
{
    m_isOverlay  = routeLayer.m_isOverlay;
}

/* See description in header file. */
RouteLayer::~RouteLayer()
{
}

/* See description in header file. */
UnifiedLayerPtr RouteLayer::Clone()
{
    return CloneWithTemplate<RouteLayer>(this);
}

/* See description in header file. */
NB_Error RouteLayer::SetRouteInfo(const vector<EncodedRouteInfo>& encodedInfo)
{
    NB_Error error = NE_INVAL;
    if (!encodedInfo.empty())
    {
        // Add encoded RouteInfo into mapping.
        for (size_t i = 0; i < encodedInfo.size(); ++i)
        {
            const EncodedRouteInfo& info = encodedInfo[i];
            string key;
            string strNumber = nbcommon::StringUtility::NumberToString(i+1);
            if (!info.encodedRouteId.empty())
            {
                key = CONTENTID_PARAMETER_ROUTE_ID + strNumber;
                m_routeParameters.insert(pair<string, string>(key, info.encodedRouteId));
            }
            if (!info.encodedRouteColor.empty())
            {
                key = CONTENTID_PARAMETER_ROUTE_COLOR + strNumber;
                m_routeParameters.insert(pair<string, string>(key, info.encodedRouteColor));
            }
        }
        error = NE_OK;
    }
    return error;
}

/* See description in header file. */
void RouteLayer::AppendSpecialTemplateParamters(shared_ptr<map<string, string> > templateParameters)
{
    if (!templateParameters)
    {
        return;
    }

    //@todo: here we set routewidth to a invalid string: "routewidth", when server detected
    //       this, it will set width of route line into their default value. This needs to
    //       be addressed, maybe we can let app_common to pass it in through RouteInfo.
    string routeWidth = nbcommon::StringUtility::NumberToString(DEFAULT_ROUTE_WIDTH);
    templateParameters->insert(pair<string, string>(PARAMETER_KEY_ROUTE_WIDTH, routeWidth));
    templateParameters->insert(m_routeParameters.begin(), m_routeParameters.end());
}

/* See description in header file. */
bool RouteLayer::IsCreatedByOffboardLayerManager()
{
    return m_createdByOffboardLayerProvider;
}

/*! @} */
