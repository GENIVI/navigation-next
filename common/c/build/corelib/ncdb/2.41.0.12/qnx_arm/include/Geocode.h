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

    @file     Geocode.h
    @date     03/05/2009
    @defgroup MOBIUS_GEOCODE   Mobius Geocoding API

    Geocoding class declaration.
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
#ifndef NCDB_GEOCODE_H
#define NCDB_GEOCODE_H


#include "NcdbTypes.h"

#include "GeocodeTypes.h"
#include <vector>
#include <map>

namespace Ncdb {

class GeocodeControlManager;
class Session;

//!         Database Geocode Search Class
//!  Only one instance of Geocode per Session can be active at a time.
class NCDB_API Geocode
{
public:
	//! Constructor
	Geocode(Session&  session,bool LiteConfiguration=0);

	//! Destructor
	virtual ~Geocode();

    //!	  Check if class instance is properly initialized.
    bool IsValid();


// ############################################################
//      Functions perorming a search query.
// ############################################################

    /*!	    Find places closely matching the requested address.
        @param[in] address   - address data to search. Some address elements can be empty strings.
        @param[in] flags     - Geocoding flags
        
        @return NCDB_OK      - at least one place was found,
                NCDB_NOT_FOUND - no matching address is found.
                NCDB_INVALID - invalid search criterion.
    */
    ReturnCode  FindAddress(  Address&     address,
                              int           flags = GEOCODE_DEFAULT, 
							  float			timeout = GEOCODE_CALLER_TIMEOUT);


    /*!	Find places that closely match the requested freeform address string.
        @param[in] searchString      - input string with any address information to search.
        @param[in] flags             - Geocoding flags.
        
        @return NCDB_OK      - at least one place was found,
                NCDB_NOT_FOUND - no matching address is found.
                NCDB_INVALID - invalid search criterion.
    */
    ReturnCode  FindFreeformAddress( const char*   searchString,
                                     int           flags = GEOCODE_DEFAULT);
                                     


	// ############################################################
	//      Point address Geocode.
	// ############################################################
		/*!	Perform Point-address Geocode for a House number on a given PVID.
		@param[in]  PVID			 -  Navteq PVID as integer. 
		@param[in]  houseNumber		 -	House number. Can be alphanumeric.	
							
		@param[out] dispLocation	 -  Display Lat/Lon of the house number as retrieved from NT supplied Point Address data.
		@param[out] arrLocation		 -  Arrival Lat/Lon of the house number as retrieved from NT supplied Point Address data.
		@param[out]	corrHousenumber	 -  The house number matched in NCDB database. May or may not have same houseNumber in input.

		@return    NCDB_OK			 -  Point address location for the supplied House Number was retrieved.
				   NCDB_NOT_FOUND    -  There was no point address data associated with the house number/PVID/address combination.
				   <More granular error codes/scenarios to be defined during implementation.>
		*/
		ReturnCode  FindPointAddressPVID(int			PVID,
										UtfString       houseNumber,
								  						  						  
										WorldPoint&		dispLocation,
										WorldPoint&		arrLocation,
										UtfString&		corrHousenumber);


// ############################################################
//      Iterator function to get search results.
// ############################################################

    /*!	Get first/next search result. Iterator function through result list.
    @param[out] address				  -			Complete found place address.
    @param[in,out] SupportedLanguages -			see NcdbTypes.h
    @param[out] addrSegmentId  -				resulting Address Segment Id for found street segment.
												If it equals to 0, then parameter is skipped.

    @param[out] interpolatedLocation  -			resulting interpolated Lat/Long location.
												 If it equals to 0, then parapmeter is skipped.
										
	@param[out] arrLocation				-		Arrival Lat/Lon of the house number as retrieved from Point Address data.
	@param[out] dispLocation			-		Display Lat/Lon of the house number as retrieved from Point Address data.
	@param[out] PVIDAddressInfo		-		To be populated only in range geocoding queries
												Has nothing to do with normal geocoding.

        @return    NCDB_OK    -   next address is successfully obtained,
                   NCDB_END_OF_QUERY - there are no more found addresses.
    */
    ReturnCode  GetNext( Address&						address,
                         SupportedLanguages				languageCode = US_English,
                         AddressSegmentId*				addrSegmentId = 0,
                         WorldPoint*					interpolatedLocation = 0,
		 				 WorldPoint*					arrLocation = 0,	
						 WorldPoint*					dispLocation = 0,	
						 BoundingBox*					bBox = 0,					
						 PVIDAddressInfo*               PvidAddressInfo=0 , 
						 float							timeout = GEOCODE_CALLER_TIMEOUT,
						 BoundingBox*					vPort = 0);


// #####################################################################
//   Functions to get street segment data by its Address Segment Id.
// #####################################################################

    /*!	Get full address information for a given street segment.
        @param[in]  addrSegmentId    - Address Segment Id for a street segment.
        @param[out] address          - Complete street address of a given segment.
        @param[in,out] SupportedLanguages - see NcdbTypes.h
        @return    NCDB_OK           - next address is successfully obtained,
                   NCDB_END_OF_QUERY - there are no more found addresses.
    */
    ReturnCode  GetAddressById( AddressSegmentId& addrSegmentId,
                                Address&   address,
                                SupportedLanguages     languageCode = US_English);


    /*!	Get Lat/Lon location for a given street segment and house number.
        @details This operation requires additional access to Map Data Base.

        @param[in]  addrSegmentId  - Address Segment Id for a street segment.
        @param[in]  houseNumber - house number to resolve on a given Node Link.
        @param[out] location    - resulting Lat/Long location.
        @return    NCDB_OK    -   next address is successfully obtained,
                   NCDB_END_OF_QUERY - there are no more found addresses.
    */
    ReturnCode  GetLocationById( AddressSegmentId&  addrSegmentId,
                                 const char*        houseNumber,
                                 WorldPoint&        location);


    /*!	Get shape-point array for a given street segment and house number.
        @details This operation requires additional access to Map Data Base.

        @param[in]  addrSegmentId  - Address Segment Id for a street segment.
        @param[in]  houseNumber - house number to resolve on a given Node Link.
        @param[out] location    - resulting Lat/Long location.
        @param[out] rPolyline   - resulting shape-point array.
        @param[out] rShapeIdx   - index of nearest shape-point.
        @param[out] arrivalPolyline - resulting point address arrival link shape-point array.
        @return    NCDB_OK    -   next address is successfully obtained,
                   NCDB_END_OF_QUERY - there are no more found addresses.
    */
    ReturnCode  GetShapePointArray( AddressSegmentId&       addrSegmentId,
                                    const char*             houseNumber,
                                    WorldPoint&             location,
                                    WorldPointList&         rPolyline,
                                    int&                    rShapeIdx,
                                    WorldPointList*         arrivalPolyline = NULL);


    /*!	New Geocode API which returns all the results in one go
        @param[in]  req  - Address to be searched
        @param[out] matches  - The array of results
        @return     NCDB_OK    -   Addresses obtained successfully
                    NCDB_FAIL  -   Addresses not obtained successfully
    */
    ReturnCode  AddressGeocode(GeocodeRequest& req,
                               AutoArray<GeocodeMatch>& matches);

    ReturnCode  SetAddressFromLfoPos( OutAddress& outAddr, Address& Addr, unsigned int lfofpos, 
                                             UtfString housenum, int side, SupportedLanguages languageCode);

    void SetFormattedAddressFromLfoPos(Address& addr, GeoMatchType& foundAddrMatchType, SupportedLanguages languageCode,
                                                   UtfString &FormattedOut, AutoArray<UtfString> &FormattedOutCompact, AutoArray<UtfString> &FormattedOutExtended);


	//! Get Internal pointer (for internal test only)
    GeocodeControlManager*  GetGeocodeControlManager()
        { return m_GCManager; }

private:
    GeocodeControlManager* m_GCManager;     // for both single and multi maps
};

};
#endif // NCDB_GEOCODE_H
/*! @} */
