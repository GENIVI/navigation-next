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
@file     PinLayer.h
@defgroup nbmap

*/
/*
(C) Copyright 2011 by TeleCommunication Systems, Inc.

The information contained herein is confidential, proprietary
to TeleCommunication Systems, Inc., and considered a trade secret as
defined in section 499C of the penal code of the State of
California. Use of this information by anyone other than
authorized employees of TeleCommunication Systems, is granted only
under a written non-disclosure agreement, expressly
prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
/*! @{ */

#ifndef __PINLAYER__
#define __PINLAYER__

#include "paltypes.h"
#include "nbexp.h"
#include "navpublictypes.h"
#include "smartpointer.h"
#include "base.h"
#include "datastream.h"
#include "TileKey.h"
#include "Layer.h"
#include <vector>
#include <string>
#include "PinParameters.h"

namespace nbmap
{
class Pin;

/*! Provides a definition of a PinLayer within mapview. Allows a client of mapview to get displayable name
    and toggle the display of the PinLayer. */
class PinLayer
{
public:
    // Interface Functions ......................................................................

    /*! Remove pins by pin IDs

        @param pinIDs Specified pin IDs to remove
        @return None
    */
    virtual void RemovePins(const std::vector<shared_ptr<std::string> >& pinIDs) = 0;

    /*! Set file path of material config information

        This file is a XML file contained material config information.

        @return None
    */
    virtual void SetMaterialConfigPath(shared_ptr<std::string> materialConfigPath) = 0;

    virtual shared_ptr<Pin> GetPin(shared_ptr<std::string> pinID) = 0;

    /*! Remove all pins of the layer

        @return None
    */
    virtual void RemoveAllPins() = 0;

    /*! Get the layer ID

        @return The layer ID
    */
    virtual shared_ptr<std::string> GetLayerID() = 0;

    /*! Get the coordinates of all pins in the pin layer

        This function will not check if the pin layer is enabled internal. User should know
        if the pin layer is enabled before calling this function.

        @return A vector containing the coordinates of all pins in the layer
    */
    virtual shared_ptr<std::vector<NB_LatitudeLongitude> > GetAllPinCoordinates() = 0;


    // Public functions .........................................................................

    /*! PinLayer constructor */
    PinLayer() {}

    /*! PinLayer destructor */
    virtual ~PinLayer() {}

    /*! Add pins to the pin layer

        This function creates pin objects and returns pin IDs in parameter 'parametersVector'.
        If the pin ID of one PinParameters object in 'parametersVector' is not NULL of empty,
        this pin is added successfully.

        @todo: 1. Only original data of PNG format is supported for selected and unselected
                  images now.
               2. User should set same shared pointer of data stream for same image.
                  Because I only check pointers of data stream equal to avoid adding duplicated.
                  And I think checking original data of data stream is slow.

        @param parametersVector Vector of parameters to create a pin
        @return None
    */
    template <class T>
    void
    AddPins(std::vector<PinParameters<T> >& parametersVector);

    /*! Get bubble indicated by the pinID

        @todo: Add another parameter of 'type to type' to parse template
               automatically.

        Returns the pointer to the bubble object indicated by the pinID
        Will return a null pointer if the PinID doesnt exist

        @param pinID A string indicating the pinID
        @return Pointer to the bubble Object that the pinID belongs to. Null
                if the pinID doesnt exist.
    */
    template <class T>
    shared_ptr<T>
    GetBubble(shared_ptr<std::string> pinID);


protected:
    // Protected functions ......................................................................

    /*! Generate an unique pin ID

        This function is called in public function AddPin.

        @return Generated pin ID
    */
    virtual shared_ptr<std::string> GeneratePinID() = 0;

    /*! Add a created pin to the layer

        This function is called in public function AddPins.

        @param pin A shared pointer to the added pin
        @return If the pin is added successfully, return a shared pointer to
                the tile key contained this pin, empty pointer otherwise.
                This tile key is used to call function RefreshPinsOfTile.
    */
    virtual TileKeyPtr
    AddPinPtr(shared_ptr<Pin> pin) = 0;

    /*! Notify to refresh pins in the specified tiles

        @param tileKeys Specified the tiles to refresh
        @return None
    */
    virtual void
    RefreshPinsOfTiles(const std::vector<TileKeyPtr>& tileKeys) = 0;

    /*! Notify to refresh all pins

        @return None
    */
    virtual void
    RefreshAllPins() = 0;
};

typedef shared_ptr<PinLayer> PinLayerPtr;

}

#endif

/*! @} */
