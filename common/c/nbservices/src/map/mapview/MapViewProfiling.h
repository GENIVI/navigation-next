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
    @file     MapViewProfiling.h
*/
/*
 
    Profiling class for Map View. Should only be used for debugging purpose and should
    be disabled for production code!
 
 
 
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

#ifndef _MAPVIEW_PROFILING_
#define _MAPVIEW_PROFILING_

#include "palclock.h"
#include <vector>
#include <list>

using namespace std;

namespace nbmap 
{
    
class MapViewProfiling
{
public:
    
    MapViewProfiling(bool enabled);
    
    /*! Records the current time in milliseconds.

        Does nothing if profiling is not enabled.
     */
    void RecordTick(uint32 index    /*!< Index of value to record */
                    );
    
    bool IsEnabled() const;
    
    // Print profiling output for the given value. Does nothing if profiling is not enabled */
    void Print(uint32 index         /*!< Index value to output. */
               ) const;
    
    /*! Return the current system time in milliseconds */
    static uint32 GetTickCount();
    
private:
    
    // indicates if profiling is enabled or not. Has to be set to 'false' for production code!
    bool m_enabled;
    
    class Value
    {
    public:
        Value() : m_accumulative(0), m_count(0), m_current(0), m_tickCount(0), m_min(0xFFFFFFFF), m_max(0){};
        
        // Accumulitive value (used for averages). (in milliseconds)
        uint32 m_accumulative;
        
        // The number each value has been recorded
        uint32 m_count;
        
        // Current value (in milliseconds)
        uint32 m_current;
        
        // tick count value (in milliseconds)
        uint32 m_tickCount;
        
        // Last values, used to calculate the average of the last n elements.
        list<uint32> m_lastValues;
        
        uint32 m_min;
        uint32 m_max;
    };
    
    // Profiling values
    vector<Value> m_values;
};
    
}
    
#endif

/*! @} */ 
