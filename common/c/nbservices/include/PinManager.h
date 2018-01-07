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
@file     PinManager.h
@defgroup nbmap

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

#ifndef __PIN_MANAGER__
#define __PIN_MANAGER__

#include "paltypes.h"
#include "nbexp.h"
#include "navpublictypes.h"
#include "smartpointer.h"
#include <iostream>
#include <map>
#include <vector>
#include <string>

namespace nbmap
{

// Seperator between layer ID and pin ID
const char ID_SEPERATOR[] = ":";

class PinLayer;

/*! Provides a definition of a PinManager within mapview. Allows a user to
    add and remove Pin Layers.
*/
class PinManager
{
public:

    PinManager() {}

    virtual ~PinManager() {}

    /*! Add a Pin Layer

        This function will allow the user to add a Pin Layer to the Pin Manager.
        Creates a Pin Layer and returns a pointer the newly created layer.
        If the layerID already exists pointer to that pin layer is returned.

        @param layerID Layer ID to identify the Layer
        @return PinLayerPtr
    */
    virtual shared_ptr<PinLayer> AddPinLayer(shared_ptr<std::string> layerID) = 0;

    /*! Remove a Pin Layer

        Removes the layer to sepcified by the parameter.

        @param layer A pin layer to remove
        @return None
    */
    virtual void RemovePinLayer(shared_ptr<PinLayer > layer) = 0;

    /*! Set file path of material config information

        This file is a XML file contained material config information.

        @return None
    */
    virtual void SetMaterialConfigPath(shared_ptr<std::string> materialConfigPath) = 0;

    /*! Get file path of material config information

        This file is a XML file contained material config information.

        @return The material path
    */
    virtual shared_ptr<std::string> GetMaterialConfigPath() = 0;

    /*! Get the coordinates of all pins across all pin layers

        This function will check if the pin layers are enabled internal and only return
        the coordinates of the pin layers which are enabled.

        @return A vector containing the coordinates of all pins across all pin layers
    */
    virtual shared_ptr<std::vector<NB_LatitudeLongitude> > GetAllPinCoordinates() = 0;

    /*! Remove all the pins.
        @return None
    */
    virtual void RemoveAllPins() = 0;
};

typedef shared_ptr<PinManager> PinManagerPtr;

}

#endif
