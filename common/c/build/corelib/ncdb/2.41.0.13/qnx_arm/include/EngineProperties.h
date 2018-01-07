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

    @file     EngineProperties.h
    @date     03/09/2009
    @defgroup MOBIUS_COMMON  Mobius Common API 

    DatabaseProperties API class.
*/
/*

(C) Copyright 2014 by TeleCommunication Systems, Inc.                

The information contained herein is confidential, proprietary 
to TeleCommunication Systems, Inc., and considered a trade secret as 
defined in section 499C of the penal code of the State of     
California. Use of this information by anyone other than      
authorized employees of TeleCommunication Systems is granted only    
under a written non-disclosure agreement, expressly           
prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

/*! @{ */
#ifndef NCDB_ENGINEPROPERTIES_H
#define NCDB_ENGINEPROPERTIES_H


#include "NcdbTypes.h"


namespace Ncdb
{
class EngineOptions;

class EnginePropertyData;

// ======================================================================================
//!     Class to access Engine Properties. 
// ======================================================================================
class NCDB_API EngineProperties
{
public:
	//! Constructor. 
    //  @param[in] chars - full path to engine config file.
	EngineProperties(const char *  engineConfigPath);      

	//! Destructor
	virtual ~EngineProperties();

// Engine Data Access functions

    //! Get Engine Options 
    const EngineOptions* GetEngineOptions() const;  
    EngineOptions* GetEngineOptions();  

    //! 0 - default traffic, 1 - OpenLR traffic
    int GetTrafficType() const;

private:

    EnginePropertyData*   m_EnginePropertyData;          // pointer to Engine Properties data container
};

} // namespace Ncdb

#endif // NCDB_ENGINEPROPERTIES_H
/*! @} */
