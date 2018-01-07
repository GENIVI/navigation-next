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
  @file        LayerProvider.cpp
  @defgroup    map

  Description: Implementation of LocalLayerProvider.
*/
/*
  (C) Copyright 2014 by TeleCommunications Systems, Inc.

  The information contained herein is confidential, proprietary to
  TeleCommunication Systems, Inc., and considered a trade secret as defined
  in section 499C of the penal code of the State of California. Use of this
  information by anyone other than authorized employees of TeleCommunication
  Systems is granted only under a written non-disclosure agreement, expressly
  prescribing the scope and manner of such use.

  --------------------------------------------------------------------------*/
/*! @{ */

#include "LayerProvider.h"
#include "UnifiedLayerListener.h"
#include <cmath>
#include <algorithm>

using namespace nbmap;

/*! Functor used to tell each listener that layers are updated. */
struct  UnifiedLayersUpdatedFunctor
{
    UnifiedLayersUpdatedFunctor()
    {
    }

    void operator () (UnifiedLayerListener* listener) const
    {
        if (listener)
        {
            listener->UnifiedLayerUpdated();
        }
    }

};

/*! Function to find specified UnifiedLayerListener. */
class UnifiedLayerListenerFinder
{
public:
    UnifiedLayerListenerFinder(UnifiedLayerListener* listener)
            : m_pListener(listener)
    {
    }

    virtual ~UnifiedLayerListenerFinder()
    {
    }

    bool operator() (UnifiedLayerListener* listener) const
    {
        return (m_pListener == listener);
    }

    UnifiedLayerListener* m_pListener;
};

void LayerProvider::CheckMetadataChanges(shared_ptr<AsyncCallback<bool> > /*callback*/, shared_ptr<MetadataConfiguration> /*config*/, bool /*skipTimeInterval*/, bool /*forceUpdate*/)
{
    //do nothing
}

void LayerProvider::RegisterUnifiedLayerListener(UnifiedLayerListener* listener,
                                                          bool higherPriority)
{
    if (listener)
    {
        list<UnifiedLayerListener*>::iterator end = m_pListeners.end();
        list<UnifiedLayerListener*>::iterator iter =
                find_if(m_pListeners.begin(), end, UnifiedLayerListenerFinder(listener));
        if (iter == end)
        {
            if (higherPriority)
            {
                m_pListeners.push_front(listener);
            }
            else
            {
                m_pListeners.push_back(listener);
            }
        }
        // No need to adjust its position.
    }
}

void LayerProvider::UnregisterUnifiedLayerListener(UnifiedLayerListener* listener)
{
    if (listener)
    {
        list<UnifiedLayerListener*>::iterator end = m_pListeners.end();
        list<UnifiedLayerListener*>::iterator iter =
                find_if(m_pListeners.begin(), end, UnifiedLayerListenerFinder(listener));
        if (iter != end)
        {
            m_pListeners.erase(iter);
        }
    }
}

void LayerProvider::NotifyLayersUpdated(const vector<LayerPtr>& /*newLayers*/)
{
    UnifiedLayersUpdatedFunctor functor;
    for_each (m_pListeners.begin(), m_pListeners.end(), functor);
}

/*! @} */
