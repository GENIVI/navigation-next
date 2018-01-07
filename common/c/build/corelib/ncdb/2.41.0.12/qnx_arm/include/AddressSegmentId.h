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

    @file     AddressSegmentId.h
    @date     03/05/2009
    @defgroup MOBIUS_GEOCODE   Mobius Geocoding API  

    Address Segment Id for geocoding API.
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
#ifndef NCDB_ADDRESSSEGMENTID_H
#define NCDB_ADDRESSSEGMENTID_H


#include "NcdbTypes.h"

namespace Ncdb {

//!    Type definition for Address Segment Id, refering to a unique MapLink in data base.
class AddressSegmentId  
{
public:
    //! default initialization with zeros as invalid values.
    AddressSegmentId() : m_mapId(0),
       m_AddressLFOpos(0),m_AddressSegmentID(0), m_Side(StreetSide_Unknown), m_CrossSegmentId(0), m_SrcId(0), m_NameIndex(-1), m_CrossNameIndex(-1), m_ArrivalLFOpos(0),m_ArrivalSegmentId(0), m_ArrivalSide(StreetSide_Unknown)
       { }    

       bool    IsValid() { return ( m_AddressLFOpos != 0 && m_AddressLFOpos != -1);  }

    //! Get Street info of a segment.
    //long GetSegmentID() { return m_SegmentId; }
    StreetSide   Side()  { return m_Side; }
    long long GetSrcId() { return m_SrcId;}

    //! Get name index (from 0 to 6) in the street name list of a segment.
    short        NameIndex() { return   m_NameIndex; }

	void GetAddressLink (long& addressLFOpos, unsigned long long& addressSegmentId,  StreetSide& addressSide)
	{
		addressLFOpos = m_AddressLFOpos;
		addressSegmentId = m_AddressSegmentID;
		addressSide = m_Side;
	};

	void GetArrivalLink (long& arrivalLFOpos, unsigned long long& arrivalSegmentId, StreetSide& arrivalSide)
	{
		arrivalLFOpos = m_ArrivalLFOpos;
		arrivalSegmentId = m_ArrivalSegmentId;
		arrivalSide = m_ArrivalSide;
	};

private:
    int					m_mapId;
    //long				m_SegmentId;        // internal  segment data id (LinkInfo pos)
	long				m_AddressLFOpos;
	unsigned long long	m_AddressSegmentID;	//64 bit identifier corresponding to NodeLinkID
	StreetSide			m_Side;             // side of street on Address link
    long				m_CrossSegmentId;   // internal crossing segment data id (LinkInfo pos)
	long long			m_SrcId;				// Raw data provider source link ID. Required for E911 and Point Address lookup
    short				m_NameIndex;        // the index of street name in the street name list (0 .. 6)
    short				m_CrossNameIndex;   // the index of crossing street name in the street name list (0 .. 6)

	long				m_ArrivalLFOpos;
    unsigned long long	m_ArrivalSegmentId;   // for arrival link (if different from address link)
    StreetSide			m_ArrivalSide;        // for arrival link (if different from address side)

    friend class    GeocodeControlManager;
	friend class    GeocodeControl;
    friend class    GeocodeManager;
	friend class	Geocode;
};


};
#endif // NCDB_ADDRESSSEGMENTID_H
/*! @} */
