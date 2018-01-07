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
    @file     NavApiLaneInformation.h
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

#ifndef __NAVAPILANEINFORMATION_H__
#define __NAVAPILANEINFORMATION_H__

#include "NavApiTypes.h"

namespace nbnav
{

/*! Lane

    Lane data with TTF character arrays for highlighted lanes and non highlighted lanes
    Non thread safe class.
*/
class Lane
{
public:
    /*! Highlighted information for the lane

        @return string of highlighted information
    */
    std::string GetHighlighted() const;

    /*! Non highlighted information for the lane

        @return string of non highlighted information
    */
    std::string GetNonHighlighted() const;

    /*! Lane constructor.

       This constructor is for internal use only.
       @param laneItem
    */
    Lane(const void* laneItem);

private:
    std::string m_highlighted;
    std::string m_nonHighlighted;
};

/*! LaneInformation

    Provides the information about the lanes.
*/
class LaneInformation
{
public:
    /*! Position of the selected lane, 0-indexed from left.

        @return position
    */
    unsigned int SelectedLanePosition() const;

    /*! Number of selected lanes

        @return number of selected lanes
    */
    unsigned int SelectedLanes() const;

    /*! Array of the lanes

        @return vector of lanes
    */
    vector<Lane> Lanes() const;

    /*! LaneInformation constructor

        This constructor is for internal use only.

        @param impl
        @param manueverIndex
        @param laneCount
        @param laneItems
    */
    LaneInformation(const NavigationImpl& impl, unsigned long manueverIndex,
                    unsigned long laneCount, void* laneItems);
    LaneInformation();

private:
    unsigned int  m_selectedLanePosition;
    unsigned int  m_selectedLanes;
    vector <Lane> m_lanes;
};

}

#endif

/*! @} */