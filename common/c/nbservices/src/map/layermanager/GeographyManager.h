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
  @file        GeographyManager.h
  @defgroup    nbmap
  Description: Interface class for GeographyManager, which manage geography layers.
*/
/*
   (C) Copyright 2013 by TeleCommunications Systems, Inc.

   The information contained herein is confidential, proprietary to
   TeleCommunication Systems, Inc., and considered a trade secret as defined
   in section 499C of the penal code of the State of California. Use of this
   information by anyone other than authorized employees of TeleCommunication
   Systems is granted only under a written non-disclosure agreement, expressly
   prescribing the scope and manner of such use.

 --------------------------------------------------------------------------*/
/*! @{ */

#ifndef _GEOGRAPHYMANAGER_H_
#define _GEOGRAPHYMANAGER_H_

#include "smartpointer.h"
#include "GeographyOperation.h"
#include <vector>

namespace nbmap
{


/*! Provides a definition of a GeographyManager within mapview. Allows a user to
    add and remove geography Layers.
*/
class GeographyManager
{
public:
    /*! Default constructor. */
    GeographyManager(){};

    /*! Default destructor. */
    virtual ~GeographyManager(){};

    /*! Add a geography layer suit

        This function will create 4 geography layers(zoom level 2/8/12/16) as layer suit.
        if create layers success and return pointer of GeographyOperation.

        @param layerId Layer ID to identify the Layer suit
        @return GeographyOperationPtr
    */
    virtual shared_ptr<GeographyOperation> AddGeographyLayerSuit(const string& layerId) = 0;

    /*! Remove a geography layer suit

        Removes the layer to sepcified by the parameter.

        @param layer A geography layer suit to remove
        @return None
    */
    virtual void RemoveGeographyLayerSuit(const string& layerId) = 0;



};

typedef shared_ptr<GeographyManager> GeographyManagerPtr;

};


/*! @} */
#endif /* _GEOGRAPHYMANAGER_H_ */
