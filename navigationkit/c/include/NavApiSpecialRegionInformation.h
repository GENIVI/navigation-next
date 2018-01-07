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
    @file     NavApiSpecialRegionInformation.h
*/
/*
    (C) Copyright 2014 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems, is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
/*! @{ */

#ifndef __NAVAPISPECIALREGIONINFORMATION_H__
#define __NAVAPISPECIALREGIONINFORMATION_H__

#include "NavApiTypes.h"

namespace nbnav
{
/*! Special region information

    Provides special region information.
*/
class SpecialRegionInformation
{
public:
    /*! Special region type.

        @return string.
     */
    std::string GetType() const;

    /*! Special region description.

        @return string
     */
    std::string GetDescription() const;

    /*! Special region image instance

        @return Image object
     */
    const Image& GetImage() const;

    /*! Index of the maneuver where the region starts

        @return index
     */
    int GetStartManeuverIndex() const;

    /*! Offset from the start of the maneuver to where the region starts

        @return offset
     */
    double GetStartManeuverOffset() const;

    /*! Index of the maneuver where the region ends

        @return index
     */
    int GetEndManeuverIndex() const;

    /*! Offset from the start of the maneuver to where the region ends

        @return offset
     */
    double GetEndManuverOffset() const;

    /*! Next time at which this region will be active within the next month

        @return value start from gps time
     */
    int GetNextOccurStartTime() const;

    /*! Next time at which this region will stop being active within the next month

        @return value start from gps time
     */
    int GetNextOccurEndTime() const;

    /*! Construction for internal use only;
    */
    SpecialRegionInformation(const void* data);
    SpecialRegionInformation();

private:
    std::string m_regionType;
    std::string m_regionDescription;
    Image       m_image;
    int         m_startManeuverIndex;
    double      m_startManeuverOffset;
    int         m_endManeuverIndex;
    double      m_endManeuverOffset;
    int         m_nextOccurStartTime;
    int         m_nextOccurEndTime;
};

}

#endif

/*! @} */
