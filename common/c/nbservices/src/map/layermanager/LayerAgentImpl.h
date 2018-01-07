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
   @file        LayerAgentImpl.h
   @defgroup    nbmap

   Description: LayerAgenImpl inherits from LayerAgent.

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

#ifndef _LAYERAGENTIMPL_H_
#define _LAYERAGENTIMPL_H_

#include "LayerAgent.h"
#include "base.h"
#include <vector>
#include "UnifiedLayer.h"

namespace nbmap
{

class LayerManager;
class LocalLayerProvider;
class Layer;

class LayerAgentImpl : public LayerAgent,
                       public Base
{
public:
    virtual ~LayerAgentImpl();

    static LayerAgentPtr
    CreateLayerAgent(shared_ptr<LayerManager> layerManager,
                     shared_ptr<Layer> layer,
                     shared_ptr<LocalLayerProvider> layerProvider);

    // Refer to LayerAgent for description.
    virtual NB_Error SetEnabled(bool enabled);

    // Refer to LayerAgent for description.
    virtual shared_ptr<string> GetName();

    // Refer to LayerAgent for description.
    virtual bool IsEnabled();

    /*! Updates stored layer pointer to a new one.

      This should be called when metadata changes, it just replace the layer pointer with
      new one, caller should be responsible for notifying Listeners about this.

      @return None.
    */
    virtual void ResetLayer(shared_ptr<Layer> layer);

    shared_ptr<Layer> GetLayerPtr();
    void CCC_SetEnabled(bool enabled);

protected:
    /*! Constructor */
    LayerAgentImpl(shared_ptr<LayerManager> layerManager,
                   shared_ptr<Layer>        layer,
                   shared_ptr<LocalLayerProvider> layerProvider);

    /*! Do some clean up when layer is toggled.

      This is virtual function so that different layer can implement it differently.

      @return None.
    */
    virtual void PostLayerToggled(bool enabled);

    vector<UnifiedLayerPtr> GetChildrenLayers();

    // Protected member fields
    shared_ptr<LayerManager> m_pLayerManager; /*!< Pointer of Layer Manager instance */
    shared_ptr<Layer>        m_pLayer;        /*!< Pointer of real layer  */

private:
    shared_ptr<LocalLayerProvider> m_pLayerProvider; /*!< Pointer of layer provider.*/

};

}
#endif /* _LAYERAGENTIMPL_H_ */

/*! @} */
