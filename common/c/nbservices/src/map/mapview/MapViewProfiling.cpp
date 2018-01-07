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
    @file     MapViewProfiling.cpp
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

#include "MapViewProfiling.h"
#include "palstdlib.h"

using namespace nbmap;

/*! Number of profilings data to calculate the second average value */
const uint32 COUNT_FOR_AVERAGE = 10;

    
MapViewProfiling::MapViewProfiling(bool enabled)
: m_enabled(enabled)
{
    
}
    
/* See header for description */
void
MapViewProfiling::RecordTick(uint32 index)
{
    if (! m_enabled)
    {
        return;
    }
    
    uint32 tickCount = GetTickCount();
    uint32 newValue = 0;
    
    // We compare each value to the previous entry (the first entry doesn't have a value since we have nothing to compare to)
    if (index > 0)
    {
        newValue = tickCount - m_values[index - 1].m_tickCount;
    }
    
    if (index >= m_values.size())
    {
        // Make sure vector is big enought and add element
        m_values.resize(index + 1);
    }
        
    // Get reference to existing value
    Value& value = m_values[index];
    
    // Update the value
    value.m_count++;
    value.m_accumulative    += newValue;
    value.m_current         = newValue;
    value.m_tickCount       = tickCount;
    
    value.m_min = std::min(value.m_min, newValue);
    value.m_max = std::max(value.m_max, newValue);
    
    // Add value to our last value list. Only keep last n values
    value.m_lastValues.push_back(newValue);
    if (value.m_lastValues.size() > COUNT_FOR_AVERAGE)
    {
        value.m_lastValues.pop_front();
    }
}
    
bool
MapViewProfiling::IsEnabled() const
{
    return m_enabled;
}
    
void
MapViewProfiling::Print(uint32 index) const
{
    if ((! m_enabled) || (index >= m_values.size()))
    {
        return;
    }

    // Calculate average of last n values
    uint32 averageOfN = 0;
    const list<uint32>& lastValues = m_values[index].m_lastValues;
    
    if (! lastValues.empty())
    {
        list<uint32>::const_iterator listValue = lastValues.begin();
        list<uint32>::const_iterator listValueEnd = lastValues.end();
        
        for (; listValue != listValueEnd; ++listValue)
        {
            averageOfN += *listValue;
        }
        averageOfN /= lastValues.size();
    }
    
    printf("[%d]Value[%d]: Current: %d,\tAverage: %d,\tAverage(last %d): %d,\tMin: %d,\tMax: %d\r\n", 
           m_values[index].m_count, 
           index, 
           m_values[index].m_current,
           m_values[index].m_accumulative / m_values[index].m_count, 
           COUNT_FOR_AVERAGE,
           averageOfN,
           m_values[index].m_min,
           m_values[index].m_max);
}
    
uint32
MapViewProfiling::GetTickCount()
{
    PAL_ClockDateTime dateTime = {0};
    PAL_ClockGetDateTime(&dateTime);
    return (dateTime.hour * 60 * 60 * 1000 + dateTime.minute * 60 * 1000 + dateTime.second * 1000 + dateTime.milliseconds);
}



/*! @} */ 
