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

/*****************************************************************/
/*                                                               */
/* (C) Copyright 2014 by TeleCommunication Systems, Inc.                */
/*                                                               */
/* The information contained herein is confidential, proprietary */
/* to TeleCommunication Systems, Inc., and considered a trade secret as */
/* defined in section 499C of the penal code of the State of     */
/* California. Use of this information by anyone other than      */
/* authorized employees of TeleCommunication Systems is granted only    */
/* under a written non-disclosure agreement, expressly           */
/* prescribing the scope and manner of such use.                 */
/*                                                               */
/*****************************************************************/
#ifndef NCDB_SPECIALREGION_H
#define NCDB_SPECIALREGION_H

#include "NcdbTypes.h"
#include "UtfString.h"
#include "AutoArray.h"   
#include "AutoSharedPtr.h"

namespace Ncdb
{
    enum NCDB_API SpecialRegionType
    {
        SPECIALREGION_CLOSURES = 1,
        SPECIALREGION_SEASONAL_CLOSURES,
        SPECIALREGION_TOLL_BOOTHS,
        SPECIALREGION_TIME_DEPENDENCIES,
        SPECIALREGION_RESTRICTED_TURN,
        SPECIALREGION_ENTERING_STATE,
        SPECIALREGION_GATE
    };

    class NCDB_API SpecialRegion
    {
    public:
        SpecialRegion();
        ~SpecialRegion();
        unsigned int                  GetStartManeuverIndex() const;
        unsigned int                  GetEndManeuverIndex() const;
        double                        GetStartManeuverOffset() const;
        double                        GetEndManeuverOffset() const;
        unsigned int                  GetNextStartTime() const;
        unsigned int                  GetNextEndTime() const;
        SpecialRegionType             GetSpecialRegionType() const; 
        const UtfString&              GetCountryCode()const;

    private:
        unsigned int        m_startManIndex;
        unsigned int        m_endManIndex;
        double              m_startManOffset;
        double              m_endManOffset;
        unsigned int        m_nextStartTime;
        unsigned int        m_nextEndTime;
        UtfString	        m_CCode;
        SpecialRegionType   m_SpecialRegionType;
        friend class RouteEngine;
    };

    typedef AutoArray< SpecialRegion > SpecialRegionList;
#ifdef _MSC_VER
    template class NCDB_API AutoArray<SpecialRegion>;
    template class NCDB_API AutoSharedPtr<SpecialRegionList>;
#endif
}

#endif
