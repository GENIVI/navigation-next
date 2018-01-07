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

    @file     ReverseGeocode.h
    @date     04/02/2009
    @defgroup MOBIUS_GEOCODE   Mobius Geocoding API   

    Reverse Geocoding class declaration.
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
#ifndef NCDB_REVERSE_GEOCODE_H
#define NCDB_REVERSE_GEOCODE_H


#include "NcdbTypes.h"

#include "GeocodeTypes.h"

namespace Ncdb {

class Session;
class ReverseGeocodeControl;

//!         Database Reverse Geocoding Search Class
class NCDB_API ReverseGeocode
{
public:
	//! Constructor
	ReverseGeocode(Session&  session);

	//! Destructor
	virtual ~ReverseGeocode();

    // ------------------  Flag constants  -----------------
    //!	Common Reverse Geocoding flags. 
    enum CommonFlags{
        SearchDefault           = 0x0000,       //!< default search flags
        SearchRoutablePlace     = 0x0001        //!< search for closest routable place    
    };

    //!	 Additional GetInstantName flags.  
    //!  If one or several of them are provided, then other object types are not searched. 
    //   By default all object types are searched in the following order: Address->Intersection->POI->Polygon->Airport.
    enum GetInstantNameFlags{
        SearchAddress      = 0x000100,       //!< find address on instant name search
        SearchPOIs         = 0x000200,       //!< find closest POIs on instant name search        (UNDER CONSTRUCTION)
        SearchPolygon      = 0x000400,       //!< find containing polygons on instant name search 
        SearchAirport      = 0x000800,       //!< find closest airport(s)                         (UNDER CONSTRUCTION)
        SearchIntersection = 0x001000,       //!< find intersections on instant name search       (UNDER CONSTRUCTION)
        InstantNameMask    = 0x0FFF00        //!< find address on instant name search
    };


    // ------------- Type of the returned Instant Name string -----------------
    //!	  Type of the found object, which name has been returned.
    typedef  enum  {
        NoObject       = 0,            //!< no object with a name is found within a given distance.
        AddressSegment,                //!< String with address of projected point onto the nearest street segment.    
        Polygon,                       //!< The name of containing polygon (a top one).  
        Intersection,                  //!< The nearest streets intersection (NOT YET IMPLEMENTED).
        Airport,                       //!< The nearest airport (NOT YET IMPLEMENTED).
        POIObject                      //!< The nearest POI name (NOT YET IMPLEMENTED).
    } FoundObjectType;

    //!	  Check if class instance is properly initialized.
    bool IsValid();

    /*!	Get address closely matching a given location.
         
        @param[in] location        - Lat/Long of search point.
        @param[in] distance        - max distance to search (in meters)
        @param[in] flags           - Set of search flags ( SearchDefault | SearchRoutablePlace ... ).
        @param[in] languageCode    - preferred language of names to return.
        @param[out] address        - Found address data. 
        @param[out] foundLocation   - Lat/Long of the exact location of found nearest address.
        @param[out] foundSide       - street side of found nearest address.
        @return    NCDB_OK      - an address is found,
                   NCDB_NOT_FOUND  - no address segment within a given distance is found.
                   NCDB_INVALID - invalid search input data. 
    */
    ReturnCode  FindAddress(WorldPoint&         location,
                            float               distance,
                            int                 flags, 
                            SupportedLanguages  languageCode,
                            Address&            address,
                            WorldPoint&         foundLocation,
                            StreetSide&         foundSide);                        

/*!   Search Point address by nearest DP on the nearest PVID
         
        @param[in] location        - Lat/Long of search point.
        @param[in] distance        - max distance to search (in meters)
        @param[in] flags           - Set of search flags ( will not play a role for point address. It is there as a placeholder).
        @param[in] languageCode    - preferred language of names to return. (for future use)
        @param[out] address        - Found address data. 
        @param[out] dispLocation   - Nearest DP on nearest PVID.
        @param[out] arrLocation    - RP of the above DP.
        @param[out] foundSide       - street side of found nearest address. Side relative to Address Link.
        @return    NCDB_OK      - an address is found,
                   NCDB_NOT_FOUND  - no address segment within a given distance is found.
                   NCDB_INVALID - invalid search input data. 
    */
    ReturnCode  FindPointAddress(WorldPoint&        location,
								float               distance,
								int                 flags, 
								SupportedLanguages  languageCode,
								Address&            address,
								WorldPoint&         dispLocation,
								WorldPoint&         arrLocation,
								StreetSide&         foundSide);    



	/*!   Search Point address by nearest DP with or without house number
         
        @param[in] location        - Lat/Long of search point.
        @param[in] distance        - Max distance to search (in meters)
		@param[in]  houseNumber	   - House number. Can be alphanumeric.	
        @param[in] flags           - Set of search flags ( will not play a role for point address. It is there as a placeholder).
        @param[in] languageCode    - preferred language of names to return. (for future use)
        @param[out] address        - Found address data. 
        @param[out] dispLocation   - Nearest DP on nearest PVID.
        @param[out] arrLocation    - RP of the above DP.
        @param[out] foundSide       - street side of found nearest address. Side relative to Address Link.
        @return    NCDB_OK      - an address is found,
                   NCDB_NOT_FOUND  - no address segment within a given distance is found.
                   NCDB_INVALID - invalid search input data. 
    */
    ReturnCode  FindNearestDPByHouseandStreet(WorldPoint&      location,
								float               distance,
								UtfString           houseNumber,
								UtfString			streetName,		
								int                 flags, 
								SupportedLanguages  languageCode,
								Address&            address,
								WorldPoint&         dispLocation,
								WorldPoint&         arrLocation,
								StreetSide&         foundSide);

	/*!   Find nearest point address DP and RP within inRadius on the same side for the given "location"
         
        @param[in] location        - Lat/Long of search point.
        @param[in] distance        - Max distance to search (in meters)
		@param[in] languageCode    - preferred language of names to return.
		@param[out] nearestDP	   - Nearest DP info.	
        @param[out] nearestRP      - Nearest DP info.
        @return    NCDB_OK         - an address is found,
                   NCDB_NOT_FOUND  - no address segment within a given distance is found.
                   NCDB_INVALID    - invalid search input data. 
				   NCDB_UNSUPPORTED- ncdb not supported data. 
    */
    ReturnCode  FindNearestDPRP(const WorldPoint&	location,
								const float			distance,
								SupportedLanguages  languageCode,
								AddressData&		nearestDP,
								AddressData&		nearestRP);
								
    /*!	Get instant name (any closest object name) near a given location.
         
        @param[in] location        - Lat/Long of search point.
        @param[in] distance        - max distance to search (in meters)
        @param[in] flags           - Set of search flags ( SearchDefault | SearchPOIs ... ).
        @param[in] languageCode    - preferred language of names to return.
        @param[out] name           - Resulting instant name of closest map object 
        @param[out] foundType      - a type of found map object
        @return    NCDB_OK              - a close object is found and it's name is returned,
                   NCDB_NOT_FOUND       - no object is found inside a given bounding box.
                   NCDB_INVALID         - invalid search input data. 
    */
    ReturnCode  GetInstantName( WorldPoint&         location,
                                float               distance,
                                int                 flags, 
                                SupportedLanguages  languageCode,
                                UtfString&          name,               // Returned Instant name
                                FoundObjectType&    foundType );        // type of found object

    /*! Get the name of country which contains the given location.
        @param[in] location   - Lat/Long of search point.
        @param[out] name      - Resulting the country name for the location.
                                The country name is country code with three letters.
                                If the location does not belong to any country, no name will be returned that means the name is empty.
                                If the location belong to one country, one name will be returned.
        @return    NCDB_OK          - one country are found which contain the given location and its name is returned in name
                   NCDB_NOT_FOUND   - no country is found that means the location doesn't belong to any country and no name is returned in name.
                   NCDB_UNSUPPORTED - no country info supplied that means there is no world polygons data in map data.
    */
    ReturnCode  GetInstantCountry( WorldPoint&         location,
                                   UtfString&          name);  // Returned country name


private:
    ReverseGeocodeControl* m_RevGeoCtrls[MAX_MAPS];   // Subhandles for all maps

    friend class ReverseGeocodeControl;
};

};
#endif // NCDB_REVERSE_GEOCODE_H
/*! @} */
