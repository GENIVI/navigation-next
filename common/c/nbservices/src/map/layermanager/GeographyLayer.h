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
    @file       GeographyLayer.h

    Interface to add Geographic shapes to the map.
*/

/*
    (C) Copyright 2013 by TeleCommunications Systems, Inc.


    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#ifndef _GEOGRAPHYLAYER_H_
#define _GEOGRAPHYLAYER_H_

#include "smartpointer.h"
#include "GeographyOperation.h"

namespace nbmap
{

/*! Optional layer used to add geographic shapes.
    This layer should only be added if geographic shapes are added.
 */

class GeographyLayer
{
public:
    // Public functions .........................................................................
    GeographyLayer(){};

    virtual ~GeographyLayer(){};

    /*! Add a geographic shape to the layer
        @return NE_OK on success
     */
    virtual NB_Error AddGeographyShape(GeographyShapePtr shape, int fitZoomLevel) = 0;

    /*! Remove a geographic shape from the layer
        @return NE_NOENT if the entry doesn't exist, NE_OK if the item was removed.
     */
    virtual NB_Error RemoveGeographyShape(shared_ptr<std::string> id) = 0;
};

typedef shared_ptr<GeographyLayer> GeographyLayerPtr;

};  // namespace nbmap

/*! @} */
#endif /* _GEOGRAPHYLAYER_H_ */
