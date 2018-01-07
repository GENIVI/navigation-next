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
  @file        LayerFunctors.h
  @defgroup    map

  Description: Set of LayerFunctors used in map.

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


#ifndef _FUNCTORS_H_
#define _FUNCTORS_H_

#include <algorithm>
#include "Layer.h"
#include "base.h"
#include "StringUtility.h"

namespace nbmap
{

/*! Functor to find a layer by layer pointer. */
class LayerFindByPointerFunctor : public Base
{
public:
    LayerFindByPointerFunctor(LayerPtr pLayer)
    {
        m_pLayer = pLayer;
    }

    virtual ~LayerFindByPointerFunctor()
    {
    }

    bool operator()(const LayerPtr& layer) const
    {
        bool ret = false;

        if (layer && m_pLayer)
        {
            shared_ptr<string> lhs = layer->GetLayerName();
            shared_ptr<string> rhs = m_pLayer->GetLayerName();
            if (lhs && rhs) ret = (*lhs == *rhs);
        }

        return ret;
    }

private:
    LayerPtr m_pLayer;
};

/*! Functor to find a layer by layer ID. */
class LayerFindByDataTypeFunctor : public Base
{
public:
    LayerFindByDataTypeFunctor(shared_ptr<string> dataType)
            : m_sDataType(dataType),
              m_cDataType(dataType ? dataType->c_str() : NULL)
    {
    }

    LayerFindByDataTypeFunctor(const char* dataType)
            : m_cDataType(dataType)
    {
    }

    virtual ~LayerFindByDataTypeFunctor()
    {
    }

    bool operator() (const LayerPtr& layer) const
    {
        bool found = false;
        if (layer)
        {
            shared_ptr<string> tmpId = layer->GetTileDataType();
            if (tmpId && m_cDataType)
            {
                found = !tmpId->compare(m_cDataType);
            }
        }
        return found;
    }
private:
    shared_ptr<string> m_sDataType;
    const char*        m_cDataType;
};

/*! Find a layer by its ID (digital ID which is assigned to layer when it was created). */
class LayerFindByIdFunctor
{
public:
    LayerFindByIdFunctor(uint32 id)
            : m_layerId(id)
    {
    }

    virtual ~LayerFindByIdFunctor()
    {
    }

    bool operator() (const LayerPtr& layer) const
    {
        return layer ? layer->GetID() == m_layerId : false;
    }

private:
    uint32 m_layerId;
};

/*! Similar to LayerFindByDataTypeFunctor, but more strict. */
struct LayerFindStrictFunctor
{
    LayerFindStrictFunctor(const LayerPtr& layer)
            : m_layer(layer)
    {
    }

    bool operator() (const LayerPtr& layer) const
    {
        bool result = false;
        if (layer && m_layer)
        {
            if (layer == m_layer &&
                nbcommon::StringUtility::IsStringEqual(layer->GetTileDataType(),
                                                       m_layer->GetTileDataType()) &&
                layer->GetID() == m_layer->GetID())
            {
                result = true;
            }
        }
        return result;
    }

    LayerPtr m_layer;
};

class LayerViewerFunctor
{
public:
    LayerViewerFunctor(){}
    virtual ~LayerViewerFunctor(){}
    void operator()(LayerPtr layer) const
    {
        fprintf(stderr, "Layer: %p, type: %s, ID: %d, zoom: (%d, %d), lam-index: %d\n",
                layer.get(), layer->GetTileDataType()->c_str(), layer->GetID(),
                layer->GetMinZoom(), layer->GetMaxZoom(),
                layer->GetLayerAvailabilityMatrixIndex());
    }
};


};

#endif /* _FUNCTORS_H_ */
/*! @} */
