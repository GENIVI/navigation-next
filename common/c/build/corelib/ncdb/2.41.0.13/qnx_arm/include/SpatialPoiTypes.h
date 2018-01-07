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

@file     SpatialPoiTypes.h
@date     09/06/2011
@author   Sandeep Agrrawal

This file contains various data structures needed for Poi search. This is part of
API layer.

*/
/*
(C) Copyright 2009 by Networks In Motion, Inc.                

The information contained herein is confidential, proprietary 
to Networks In Motion, Inc., and considered a trade secret as 
defined in section 499C of the penal code of the State of     
California. Use of this information by anyone other than      
authorized employees of Networks In Motion is granted only    
under a written non-disclosure agreement, expressly           
prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/
/*! @{ */

#ifndef __NCDB_SPATIAL_POI_H__
#define __NCDB_SPATIAL_POI_H__

#include <vector>

#include "NcdbTypes.h"
#include "UtfString.h"
#include "GeocodeTypes.h"
#include "types.h"
#include "AutoSharedPtr.h"
#include "AutoArray.h"
#include "SingleSearchCommonTypes.h"

using namespace std;
//! Enum to describe house number type for POI
typedef enum address_type
{
	NO_HOUSE_NUMBER = 0,
	NUMERIC_HOUSE_NUMBER,
	ALPHA_HOUSE_NUMBER,
	FULL_ADDRESS
} ADDRESS_TYPE;

namespace Ncdb {

typedef AutoSharedPtr<TYPE_DATA>      TYPEDATAPtr;
typedef AutoArray<TYPEDATAPtr>      TYPEDATAPtrList;

#ifdef _MSC_VER
template class NCDB_API AutoArray<Ncdb::SpatialPoi*>;
template class NCDB_API AutoArray<AutoSharedPtr<SpatialPoi>>;
template class NCDB_API AutoArray<TYPE_INFO>;
template class NCDB_API AutoArray<TYPE_INFO*>;
template class NCDB_API AutoArray<AutoSharedPtr<TYPE_DATA>>;
#endif

class PoiSearchRequest
{
public:
	WorldPoint			searchLocation;
	float				radius;
	SEARCH_CRITERIA		criteria;
	SupportedLanguages	languageCode;
	int					NumResults;
	int					PageID;

	PoiSearchRequest()
		: searchLocation(-200,-100)
		, radius(10.0f)
		, languageCode(US_English)
		, NumResults(10)
		, PageID(1)
	{
	};
};


//! Defines a POI object which will be populated during search and retured as response
class NCDB_API SpatialPoi
{
	//! Display point for this poi
	WorldPoint			m_displayPoint;
	//! Routing point for this poi
	WorldPoint			m_routePoint;
	//! Name of POI
	UtfString			m_name;
	//! House number for this POI
	UtfString			m_houseNumber;
	//! List of phone numbers
	AutoArray<UtfString> m_telephoneNumbers;
	//! List of type information
	AutoArray<TYPE_INFO> m_typeInfos;
	//! Distance between display point and search point that was provided as input
	float				m_Distance;
	//! Address for this poi
	Address			m_Address;
    unsigned int    m_LfoPos;
    ADDR_SIDE       m_Side;
	UtfString			m_AddressStr;
    ADDRESS_TYPE        m_AddressType;

public:
	SpatialPoi()
	{
		m_Distance = 0.0;
	}
	float getDistance()
	{
		return m_Distance;
	}
	void setDistance(float dist)
	{
		this->m_Distance = dist;
	}
    ADDRESS_TYPE getAddressType()
	{
		return m_AddressType;
	}
	void setAddressType(ADDRESS_TYPE AddressType)
	{
		this->m_AddressType = AddressType;
	}

	void setDisplayPoint(WorldPoint& point)
	{
		this->m_displayPoint = point;
	}
	WorldPoint& getDisplayPoint(void)
	{
		return m_displayPoint;
	}

	void setRoutePoint(WorldPoint& point)
	{
		this->m_routePoint = point;
	}
	WorldPoint& getRoutePoint()
	{
		return m_routePoint;
	}

	UtfString& getName()
	{
		return m_name;
	}
	void setLfoPos(unsigned int lfopos)
	{
		this->m_LfoPos = lfopos;
	}
	unsigned int& getLfoPos()
	{
		return m_LfoPos;
	}
	void setSide(ADDR_SIDE side)
	{
		this->m_Side = side;
	}
    ADDR_SIDE& getSide()
	{
		return m_Side;
	}
	void setName(UtfString& m_name)
	{
		this->m_name = m_name;
	}
	UtfString& getHouseNumber()
	{
		return m_houseNumber;
	}
	void setHouseNumber(UtfString& hNum)
	{
		this->m_houseNumber = hNum;
	}

	UtfString& getAddressString()
	{
		return m_AddressStr;
	}
	void setAddressString(UtfString& addressStr)
	{
		this->m_AddressStr = addressStr;
	}
	void addTelephoneNumber(UtfString& number)
	{
		m_telephoneNumbers.push_back(number);
	}
	void addTypeInfo(TYPE_INFO& info)
	{
		m_typeInfos.push_back(info);
	}

	AutoArray<UtfString>& getTelephoneNumbers()
	{
		return m_telephoneNumbers;
	}
	AutoArray<TYPE_INFO>& getTypeInfo()
	{
		return m_typeInfos;
	}

	Address& getAddress()
	{
		return m_Address;
	}
	void setAddress(Address& address)
	{
		this->m_Address = address;
	}
};

typedef AutoSharedPtr<SpatialPoi>     SpatialPoiPtr;
typedef AutoArray<SpatialPoiPtr>      SpatialPoiPtrList;
};
#endif // __NCDB_SPATIAL_POI_H__
/*! @} */

