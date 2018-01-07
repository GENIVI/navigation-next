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
   @file         MapLegendInfo.h
   @defgroup     nbmap

   Description:  MapLegendInfo represents map legend provided by the server.
   This class can be used internally through the MapKit3D SDK.
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

/*! @} */
#ifndef _MAPLEGENDINFO_H_
#define _MAPLEGENDINFO_H_


extern "C"
{
#include "csltypes.h"
}
#include "smartpointer.h"
#include <vector>

namespace nbmap
{

/*! simple definition of ColorBar

  It consists of two parts: the first part is label to be shown, and second part is a
  serials of colors associated with this label.
*/
typedef pair<string, vector<nb_color> >  ColorBar;
typedef shared_ptr<ColorBar> ColorBarPtr;


class MapLegendInfo
{
public:
    virtual ~MapLegendInfo() {}

    /*! Get total colors of this MapLegendInfo.

        This value should be used to decide the width of each color, ie:
        widthOfColor =  widthOfView / numberOfColors

        @return number of total colors.
    */
    virtual uint32 GetTotalColors() const = 0;

    /*! Get total color bars.

        @return number of total color bars.
    */
    virtual uint32 GetTotalColorBars() const = 0;

    /*! Get color bar specified by index.

        @return shared pointer of color bar instance.
    */
    virtual const shared_ptr<ColorBar> GetColorBar(uint32 index) const = 0;
};

typedef shared_ptr<MapLegendInfo>  MapLegendInfoPtr;
}

#endif /* _MAPLEGENDINFO_H_ */
