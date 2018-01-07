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
   @file         MapLegendInfoImpl.h
   @defgroup     nbmap

   Description:  MapLegendInfoImpl inherits form MapLegendInfo interace, and
   is responsible for converting MapLegend returned from server into
   MapLegendInfo so that can be used through the SDK.
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
#ifndef _MAPLEGENDINFOIMPL_H_
#define _MAPLEGENDINFOIMPL_H_

#include "MapLegendInfo.h"

namespace protocol
{
class MapLegend;
}

namespace nbmap
{
class MapLegendInfoImpl : public MapLegendInfo
{
public:
    MapLegendInfoImpl();
    virtual ~MapLegendInfoImpl();

    // Refer to MapLegendInfo class for description.
    virtual uint32 GetTotalColors() const;
    virtual uint32 GetTotalColorBars() const;
    virtual const shared_ptr<ColorBar> GetColorBar(uint32 index) const;

    /*! Feed MapLegendInfo with protocol::MapLegend.

        This will clear all old info and fill it with new one.

        @return None.
    */
    void FeedWithMapLegend(shared_ptr<protocol::MapLegend> mapLegend);

private:

    /*! Reset this instance.

        @return None.
     */
    void Reset();

    uint32 m_totalColors;
    vector<shared_ptr<ColorBar> > m_colorBars;
};
}


#endif /* _MAPLEGENDINFOIMPL_H_ */








