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
   @file         MapLegendInfoImpl.cpp
   @defgroup     nbmap

   Description:  Implementation of MapLegendInfoImpl
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
#include "MapLegendInfoImpl.h"
#include "NBProtocolMapLegend.h"
#include <set>
#include <string>

using namespace nbmap;

/* See description in header file. */
MapLegendInfoImpl::MapLegendInfoImpl()
        : m_totalColors(0)
{
}

/* See description in header file. */
MapLegendInfoImpl::~MapLegendInfoImpl()
{
}

/* See description in header file. */
uint32
MapLegendInfoImpl::GetTotalColors() const
{
    return m_totalColors;
}

/* See description in header file. */
uint32
MapLegendInfoImpl::GetTotalColorBars() const
{
    return (uint32)m_colorBars.size();
}

/* See description in header file. */
const shared_ptr<ColorBar>
MapLegendInfoImpl::GetColorBar(uint32 index) const
{
    shared_ptr<ColorBar> colorBar;
    if (index < m_colorBars.size())
    {
        colorBar = m_colorBars.at(index);
    }
    return colorBar;
}

/* See description in header file. */
void
MapLegendInfoImpl::FeedWithMapLegend(shared_ptr <protocol::MapLegend> mapLegend)
{
    Reset();

    shared_ptr<vector<shared_ptr<protocol::ColorBar> > > colorBars;
    if (mapLegend && (colorBars = mapLegend->GetColorBarArray()) && !colorBars->empty())
    {
        set<string> legendFilter; // Used to filter out duplicated ids.
        vector<shared_ptr<protocol::ColorBar> >::const_iterator iter = colorBars->begin();
        vector<shared_ptr<protocol::ColorBar> >::const_iterator end  = colorBars->end();
        for (; iter != end; ++iter)
        {
            const shared_ptr<protocol::ColorBar> colorBar = *iter;
            shared_ptr<string> id = colorBar->GetId();
            shared_ptr<string> label = colorBar->GetLabel();
            shared_ptr<vector<shared_ptr<protocol::Color> > > colorArray =
                    colorBar->GetColorArray();
            if (!id || !label || !colorArray || colorArray->empty())
            {
                continue;
            }

            pair<set<string>::iterator, bool> result = legendFilter.insert(*id);
            if (!result.second)
            {
                continue;
            }

            vector<nb_color> nbColorArray;
            vector<shared_ptr<protocol::Color> >::const_iterator colorIter = colorArray->begin();
            vector<shared_ptr<protocol::Color> >::const_iterator colorEnd  = colorArray->end();
            for (; colorIter != colorEnd; ++colorIter)
            {
                if (*colorIter)
                {
                    nbColorArray.push_back(MAKE_NB_COLOR((*colorIter)->GetRed(),
                                                         (*colorIter)->GetGreen(),
                                                         (*colorIter)->GetBlue()));
                    ++m_totalColors;
                }
            }
            m_colorBars.push_back(shared_ptr<ColorBar> (new ColorBar(*label, nbColorArray)));
        }
    }
}

/* See description in header file. */
void
MapLegendInfoImpl::Reset()
{
    m_colorBars.clear();
    m_totalColors = 0;
}




