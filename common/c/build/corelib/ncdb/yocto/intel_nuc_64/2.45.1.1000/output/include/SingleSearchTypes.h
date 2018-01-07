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


#pragma once
#ifndef __SINGLE_SEARCH_TYPES_H__
#define __SINGLE_SEARCH_TYPES_H__

//#include "WorldPoint.h"
#include "NcdbTypes.h"
#include "UtfString.h"
#include "AutoSharedPtr.h"
#include "AutoArray.h"
#include "SingleSearchCommonTypes.h"
namespace Ncdb {
//! lists various possible search type current supported
typedef enum search_type
{
	SIMPLE_SUGGEST	= 0x0,						//suggestions for both POI and Streets
	SIMPLE_SEARCH	= 0x1,						//search for both POI and Streets
	GEOCODE_SEARCH	= 0x2,						//Geocode only search: can also include admin based search
	POI_SEARCH		= 0x3,						//POI only search:can also include airports
	AIRPORT_SEARCH	= 0x4,						//airport only search: Planned for future
    CSZ_SEARCH      = 0x5,		                //CSZ only search
	GEOCODE_SUGGEST = 0x7,						//Geocode only suggest: can also include admin based search
	POI_SUGGEST		= 0x8,						//POI only suggest:can also include airports
    GEOHASH_SEARCH  = 0x99                      //Dummy option to check PrefixQuery using Geohash4 value on Geohash9 works.
} SEARCH_TYPE_VALUES;

typedef int SearchReqType; // Use values from SEARCH_TYPE_VALUES

typedef enum GeoMatchType SearchMatchType;

//Flags
//ForceRequest in country		//applicable only if valid ISO country code is passed

class NCDB_API SearchRequest
{
public:
	SearchReqType		ReqType;
    UtfString           SearchString;		//!< String entered by user in serach box
    WorldPoint          SearchLocation;		//!< Map Center: used by proximity geocoding and poi
    WorldPoint          UserLocation;		//!< Current user Location. Not used for now.
	SupportedLanguages  InLanguageCode;     //!< Input Language code
    int                 SearchFlags;		//!< flags impacting search
	int					WantFlags;			//!< want flags in singleserachcommontypes.h.
	BoundingBox			BBox;				//!< Bounded rectangle for contained results. Not supported for now.
	UtfString			State;			    //!< full statename like "California". State is required for Geocode call.
	UtfString			Country;			//!< Country if known.
	int					NumResults;			//!< Results to return in 1 page. For POI multiple pages can be returned.
    int					PageID;				//!< Applicable for POI. If 1 then it's assumed to be fresh request
	SEARCH_CRITERIA     PoiSearchCriteria;  //!< Poi Search related input fields.
	AutoArray<UtfString> SearchStringTokenArray;
    float               Radius;

    // Constructor. Sets everything to a default
    SearchRequest():
        ReqType(SIMPLE_SEARCH), SearchLocation(-200,-100),
		UserLocation(-200,-100), InLanguageCode(US_English), SearchFlags(SAERCH_DEFAULT),
		WantFlags(WANT_SINGLELINE_ADDRESS), NumResults(10), PageID(1), Radius(25.0f)
    {
        SearchString = "";
        Country = "";
		SearchStringTokenArray.clear();
    };

};

#ifdef _MSC_VER
template class NCDB_API AutoArray<TYPE_INFO>;
template class NCDB_API AutoArray<TYPE_DATA>;
#endif

typedef struct NCDB_API SearchMatch		//applicable for suggest match also
{
    UtfString			    MatchName;					//!< Name of match, if available, like POI/Airport
    AutoArray<UtfString>    TelephoneNumbers;			//!< List of phone numbers for POI
    AutoArray<TYPE_INFO>    TypeInfos;					//!< List of type informations for POI
    AutoArray<TYPE_DATA>    TypeDatas;					//!< List of type datas for POI
    OutAddress      	    AddressOut;					//!< out address of POI or address, including Formatted elements
    float          	        MatchDistance;				//!< Distance of Match from MapCenter
    WorldPoint              GeocodedLocation;           //!< RP, otherwise IP
    WorldPoint              DisplayLocation;            //!< Display location for Match
    float          	        Confidence;                 //!< Confidence value from 0.0 to 1.0
    unsigned int            Score;                      //!< Component wise score breakup.

    SearchMatchType         MatchType;                  //!< How the match was retrieved. Values defined in enum GeoMatchType
                                                        //!< POI will be a match Type in addition to GC match types Address

    BoundingBox			    BBox;						//!< Bounding box in case of city/state/postal geocoding
    BoundingBox		        viewPort;					//!< Best Possible Viewport
    SupportedLanguages      OutLanguageCode;            //!< Future Use
    SearchMatch();
    SearchMatch(GeocodeMatch&);
    SearchMatch(SpatialPoi&);

} SearchMatch;

#ifdef _MSC_VER
template class NCDB_API AutoArray<AutoSharedPtr<SearchMatch>>;
#endif

};
#endif // __SINGLE_SEARCH_TYPES_H__
