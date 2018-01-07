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

    @file     GeocodeTypes.h
    @date     03/05/2009
    @defgroup MOBIUS_GEOCODE   Mobius Geocoding API

    Common types for geocoding API.
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

/*! @{ */
#ifndef NCDB_GEOCODE_TYPES_H
#define NCDB_GEOCODE_TYPES_H
#include <vector>
#include "NcdbTypes.h"
#include "UtfString.h"
#include "UniString.h"
#include "AddressSegmentId.h"
#include "SingleSearchCommonTypes.h"

namespace Ncdb {

#define     MAX_HOUSE_NUMBER_SIZE   12      // max size of house number string

#define STREETNAME_CHAR_TYPE_BEFORE '\x08'
#define STREETNAME_CHAR_TYPE        '\x09'
#define STREETNAME_CHAR_PREFIX      '\x10'
#define STREETNAME_CHAR_SUFFIX      '\x11'
#define STREETNAME_CHAR_NOBLANK     '\x15' // following STREETNAME_CHAR_TYPE_BEFORE or STREETNAME_CHAR_TYPE which means without blank between.
#if 0
#define STREETNAME_DELIMITER_TYPE_BEFORE    "\x08"
#define STREETNAME_DELIMITER_TYPE           "\x09"
#define STREETNAME_DELIMITER_PREFIX         "\x10"
#define STREETNAME_DELIMITER_SUFFIX         "\x11"
#endif

//Geocodind API Flags. 32 bits are available in the 4 byte flag.
#define GEOCODE_DEFAULT						0x00000000

#define SPELL_SKIP_STATE					0x00000001
#define SPELL_SKIP_CITY						0x00000002
#define SPELL_SKIP_STREET					0x00000004
#define SPELL_SKIP_CROSS_STREET				0x00000008

#define GEOCODE_USE_VARIANT_SPELLCHECKER		0x00000010
#define GEOCODE_USE_SPELLCHECKER				0x00000020		//Not usable. We use Geocode.ini to config it.
#define GEOCODE_USE_PATTERNMATCH			    0x00000040
											    //0x00000080

#define	GEOCODE_DONOTMATCH_HOUSENUM				0x00000100
#define	GEOCODE_USE_ROUTABLE					0x00000200
#define	GEOCODE_USE_POINT_ADDRESS				0x00000400		//Not usable. We use Geocode.ini to config it.
#define GEOCODE_CALCULATE_SCORE                 0x00000800

#define	GEOCODE_USE_METAPHONE					0x00001000
#define GEOCODE_USE_CTRYSEARCH                  0x00002000		//Not usable. We use Geocode.ini to config it.
#define GEOCODE_VERBOSE                         0x00004000
												//0x00008000

#define ONBOARD_GEOCODING					    0x00010000
											    //0x00020000
#define FPOS_GEOCODING					    	0x00080000				//can be azFpos,zipFpos etc...
																		//For FPOS_GEOCODING, AREA_GEOCODING/POSTAL_GEOCODING should also be ON
#define AREA_GEOCODING							0x00100000				//city/namedarea/zone
#define COUNTY_GEOCODING					    0x00040000
#define STATE_GEOCODING							0x00200000
#define POSTAL_GEOCODING						0x00400000
#define AIRPORT_GEOCODING						0x00800000

//All remaining bits are available for use
//End Geocoding API flags


#define MAX_SPELLCHECK1_LENGTH  15
#define MAX_SPELLCHECK2_LENGTH  10
#define MIN_PATTERNMATCH_LENGTH 3
#define MAX_PATTERNMATCH_LENGTH 6


// Geocode try identification flags
#define GEOCODE_TRY_EXACT                   0x0001
#define GEOCODE_TRY_RESTRICTIVE             0x0002
#define GEOCODE_TRY_PATTERN                 0x0004
#define GEOCODE_TRY_EXACT_MATCH_WORDS       0x0008
#define GEOCODE_TRY_SPELLCHECK1             0x0010
#define GEOCODE_TRY_SPELLCHECK2             0x0020
#define GEOCODE_TRY_HOUSE_NUM               0x0040
#define GEOCODE_TRY_PERFECT                 0x0080
#define GEOCODE_TRY_FABRICATED_WORDS        0x0100
#define GEOCODE_TRY_PHONETIC		        0x0200

// Street type flags to set the input start and end flags which is found in Geocode output ranges
#define LADDRTYPE_SHIFT             2
#define LADDRTYPESCH                0x0012
#define RADDRTYPESCH                0x0003
#define ADDRSTART                   0x0002
#define ADDREND                     0x0001

// Street side flags to set whether found range is on right or left or both the sides
#define SIDE_LEFT                   0x0001
#define SIDE_RIGHT                  0x0002
#define SIDE_BOTH                   0x0003

// Geocode timeout value to avoid infinite wait for caller.
#define GEOCODE_CALLER_TIMEOUT		0.0f

// Maximum results which caller wants NCDB to return
#define GEOCODE_MAX_RESULT_COUNT    30

// LITE MODE DEFINITIONS
#define LITEMODE_OFF        0x00000000
#define LITEMODE_MEDIUM     0x00000001
#define LITEMODE_EXTREME    0x00000002
#define LITEMODE_NAV        0x00000004

#define MAGNITUDE_MASK      0x00F0
#define CITY_TYPE_MASK      0x000F
#define CITY_TYPE_MAJORCITY_MASK 0x0001
#define CITY_TYPE_COUNTY_CAPITAL_MASK 0x0002
#define CITY_TYPE_STATE_CAPITAL_MASK 0x0004
#define CITY_TYPE_COUNTRY_CAPITAL_MASK 0x0008


/*
enum SPELLCHECKER_USAGE
{
    // 1st byte ( 0x--** ) reserved for algorithm selection
    DO_NOT_USE_SPELLCHECKER  = 0,
    USE_FIXED_SPELLCHECKER   = 0x0001,
    USE_VARIANT_SPELLCHECKER = 0x0002,
    // 2nd byte ( 0x**-- ) reserved for skipping fields of Address structure
    SKIP_STREET              = 0x0100,
    SKIP_CROSS_STREET        = 0x0200,
    SKIP_NAMED_AREA          = 0x0400,
    SKIP_CITY                = 0x0800,
    SKIP_ZONE                = 0x1000,
    SKIP_COUNTY              = 0x2000,
    SKIP_STATE               = 0x4000,
    SKIP_ZIP                 = 0x8000
};
*/

// -----------------------------------------------------------------------------------
//!   Structure containing street name parts
struct  StreetNameParts
{
    UtfString      Prefix;         //!<  Street name prefix (N,E, SW, etc.)
    UtfString      BaseName;       //!<  base name (examples: "156th", "Pine Tree" )
    UtfString      Suffix;         //!<  Street name suffix (NE, W,  etc.)
    UtfString      Type;           //!<  Street type  (Ave, Rd, Hwy, St, etc.)
    UtfString      TypeBefore;     //!<  Street type before name (Avenue De, Rue, Calle, etc.)
};


#define MAX_LINK_STREET_NAMES  7    // max number of street names in a link (street segment)

// -----------------------------------------------------------------------------------
//!   Set of bits, signifying error conditions, encountered during Geocode::FindAddress() run.
enum GeocodeErrorConditions
{
    GCErr_OK                = 0,            //!< NO ERROR
    GCErr_Country           = 0x0001,       //!< country  not  found
    GCErr_State             = 0x0002,       //!< state    not found
    GCErr_City              = 0x0004,       //!< city     not found
    GCErr_ZIP               = 0x0008,       //!< ZIP      not found
    GCErr_StreetInCity      = 0x0010,       //!< Street   not found
    GCErr_StreetInState     = 0x0020,       //!< Street   not found
    GCErr_NoHouseMatch      = 0x0040,       //!< Given exact house number not found
	GCErr_NoAirPortMatch	= 0x0080,
    GCErr_AddressInCity     = 0x8000,       //!< No address in a city(ies)
    GCErr_StreetFullName    = (0x0001 << 16),   //!< Street basename not found
    GCErr_StreetBaseName    = (0x0002 << 16),   //!< Street basename not found
    GCErr_StreetType        = (0x0004 << 16),   //!< Street type not found
    GCErr_StreetPrefix      = (0x0008 << 16),   //!< Street prefix not found
    GCErr_StreetSuffix      = (0x0010 << 16),   //!< Street suffix not found

};

// -----------------------------------------------------------------------------------
//! Enum detailing the geocode match merit/threshold
enum GeoMatchMerit
{
    GCMerit_BelowThreshold      = 0,
    GCMerit_Level2Threshold     = 1,
    GCMerit_Level1Threshold     = 2,
    GCMerit_WildcardThreshold   = 3,
    GCMerit_ExactThreshold      = 4,
};


struct GeocodeMatch;
#ifdef _MSC_VER
template class NCDB_API AutoArray<GeocodeMatch>;
#endif
#if !defined( __linux__ ) && !defined( __QNXNTO__ )
template class NCDB_API AutoArray< unsigned int >;
#endif

struct InAddress;
struct OutAddress;

//The airportInfo freeform needs
struct FoundAirportInfo
{
	UtfString airportCode;
	UtfString city;
	UtfString officialName;
	std::vector<UtfString> alternateNames;
	unsigned char altIndex;		//the altername's index
	unsigned char flag;			//low 4 bits:0x1    Airport Code   ;0x2    Official_Name ;0x3 Alternate_Names ;0x4 city name
								//high 4 bits:the word index +1  in the whole string. 0: the whole string
	void Clear()
	{
		airportCode.Clear();
		city.Clear();
		officialName.Clear();
		//alternateNames.clear();
        std::vector<UtfString>::iterator it = alternateNames.begin();
        for (; it != alternateNames.end(); ++it)
        {
            it->Clear();
        }
	}
	UtfString GetAirportMachedStr()
	{
		UtfString matchedStr = "";
		unsigned char tmpFlag = flag & 0xf;
		switch(tmpFlag)
		{
		case 1://airport code
			matchedStr = airportCode;
			break;
		case 2://
			matchedStr = officialName;
			break;
		case 3:
			matchedStr = alternateNames[altIndex];
			break;
		case 4:
			matchedStr = city;
			break;
		default:
			break;
		}

		return matchedStr;
	}
};

// -----------------------------------------------------------------------------------
//!   Structure containing full or partial address information
class NCDB_API Address
{
public:

    float          Displacement;						//!<  E911: How much to displac from street center line.
    UtfString      Number;                              //!<  House number (alphanumeric).
    UtfString      EndNumber;                           //!<  E911: End House number for range geocoding.
    UtfString      Street;                              //!<  Searched and found street name.
    int            NameCount;                           //!<  Number of street names.
    int            FoundNameIndex;                      //!<  Index of found street name.
    int            FFMatchedNameIndex;                  //!<  Index of Matched street name by Freeform Scoring.
    UtfString      Streets[MAX_LINK_STREET_NAMES];      //!<  Street names, including secondaty names (see NameCount).
    UtfString      CrossStreet;                         //!<  Searched and found crossing street name.
    int            CrossNameCount;                      //!<  Number of crossing street names.
    int            FoundCrossNameIndex;                 //!<  Index of found crossing street name.
    int            FFMatchedCrossNameIndex;             //!<  Index of Matched cross street name by Freeform Scoring.
    UtfString      CrossStreets[MAX_LINK_STREET_NAMES]; //!<  Crossing street names, including secondaty names (see CrossNameCount).
    AutoArray<unsigned int> infPoses;							//!<  List of predefined azFposes for city-Center Geocdoing
    UtfString      NamedArea;                           //!<  Named part of a city (Postal City, Neighborhood, District, Arrondissements  etc.)
    UtfString      City;                                //!<  City name
    UtfString      Zone;                                //!<  Zone name.
    UtfString      County;                              //!<  County name (Kreis in Germany, Commune in France, Rayon - in Russia)
    UtfString      State;                               //!<  State/Province abbr name (Department in Franse, Land in Germany, Oblast' in Russia)
	UtfString      StateFullName;                       //!<  State full name
    UtfString      Zip;                                 //!<  Postal code (ZIP in US)
    UtfString      ZipOtherSide;
    UtfString      Airport;                             //!<  Input airport code/name. In output it will be airport code
	UtfString      AirportName;                         //!<  Official Name of the airport for output.
	UtfString      FoundAirport;
	FoundAirportInfo	   AirportInfo;
	UtfString      Country;                             //!<  Country name
    WorldPoint     Location;							//!<  Center of proximity serach
    float	       Radius;								//!<  Radius of proximity serach
    float          Confidence;                          //!<  Confidence value from 0.0 to 1.0
    int            ErorConditions;                      //!<  Geocode Error conditions flags. See GeocodeErrorConditions.
    int            GeocodeTryFlags;                     //!<  Geocode try search flags.
    float          Distance;
    UtfString      FoundCity;
	UtfString      FoundZone;
	UtfString      FoundNamedArea;
    UtfString      FoundCityNor;
	UtfString      ZoneNor;
	UtfString      NamedAreaNor;
    UtfString      CityNor;

	short          CityMatchType;
	short          Magnitude;

	UtfString      Line1;                               //!<  Line 1 input for Geocoding (consisting of hno and street)
	UtfString	   ProxAdminStr;
    UtfString      Admin;
    UtfString      OutState;

	//population and other feature
	unsigned int Population;
	unsigned long long Surface;
	unsigned long PolyFPos;
	//
    StreetComponents StreetParts;
    StreetComponents StreetsParts[MAX_LINK_STREET_NAMES];
    StreetComponents CrossStreetParts;
    StreetComponents CrossStreetsParts[MAX_LINK_STREET_NAMES];
    UtfString        FoundStreet;
    StreetComponents FoundStreetParts;
    UtfString        FoundCrossStreet;
    StreetComponents FoundCrossStreetParts;

    //!<for Verbos purpose
    AutoArray<ExonymInfo> NamedAreaExonymList;  //!<  Exonym Names of NamedArea
    AutoArray<ExonymInfo> CityExonymList;      	//!<  Exonym Names of City
    AutoArray<ExonymInfo> ZoneExonymList;      	//!<  Exonym Names of Zone
    AutoArray<ExonymInfo> StateExonymList;      //!<  Exonym Names of State

    //!<For new scoring mechanism
    unsigned int    Score;

public:
    Address() :
        Displacement(0.0), NameCount(0), FoundNameIndex(-1), FFMatchedNameIndex(-1),
        CrossNameCount(0), FoundCrossNameIndex(-1), FFMatchedCrossNameIndex(-1), Country("USA"), Location(-200,-100),
        Radius(0.0), Confidence(0.0), ErorConditions(0),
        GeocodeTryFlags(0), Distance(0.0), CityMatchType(0), Magnitude(-1), Population(-1), Surface(-1), PolyFPos(-1), Score(0)
	{
	};

	Address(InAddress& address):
		Displacement(0.0), NameCount(0), FoundNameIndex(-1), FFMatchedNameIndex(-1),
		CrossNameCount(0), FoundCrossNameIndex(-1), FFMatchedCrossNameIndex(-1), Location(-200,-100),
		Radius(0.0), Confidence(0.0), ErorConditions(0),
		GeocodeTryFlags(0), Distance(0.0), CityMatchType(0), Magnitude(-1), Population(-1), Surface(-1), PolyFPos(-1), Score(0)
	{
		FromInAddress(address);
	};

	inline void FromInAddress(InAddress& address);
	inline void FromOutAddress(OutAddress& address);

    void Clear()
    {
        Displacement= 0.0f;
        Number.Clear();
        EndNumber.Clear();
        Street.Clear();
        NameCount= 0;
        FoundNameIndex = -1;
        FFMatchedNameIndex = -1;
        for(int nameIdx = 0; nameIdx < MAX_LINK_STREET_NAMES;nameIdx ++)
        {
            Streets[nameIdx].Clear();
            CrossStreets[nameIdx].Clear();
            StreetsParts[nameIdx].clear();
            CrossStreetsParts[nameIdx].clear();
        }
        CrossStreet.Clear();
        CrossNameCount= 0;
        FoundCrossNameIndex = -1;
        FFMatchedCrossNameIndex = -1;
        NamedArea.Clear();
        City.Clear();
        Zone.Clear();
        County.Clear();
        State.Clear();
        StateFullName.Clear();
        Zip.Clear();
        Airport.Clear();
        AirportName.Clear();
		FoundAirport.Clear();
		AirportInfo.Clear();
        Country.Clear();
        Location.x = -200;
        Location.y = -100;
        Radius = 0.0f;
        Confidence = 0.0f;
        ErorConditions = 0;
        GeocodeTryFlags = 0;
        Distance= 0.0f;
        FoundCity.Clear();

        FoundCityNor.Clear();
        CityNor.Clear();
        ZoneNor.Clear();
        NamedAreaNor.Clear();

		CityMatchType = 0;
		Magnitude = -1;
		Population = -1;
		Surface = -1;
		PolyFPos = -1;
        StreetParts.clear();
        CrossStreetParts.clear();
        FoundStreet.Clear();
        FoundStreetParts.clear();
        FoundCrossStreet.Clear();
        FoundCrossStreetParts.clear();

        NamedAreaExonymList.clear();
        CityExonymList.clear();
        ZoneExonymList.clear();
        StateExonymList.clear();

        Score= 0;
		FoundZone.Clear();
		FoundNamedArea.Clear();
        infPoses.clear();
    }
};

struct AddressData
{
	WorldPoint         dispLocation;
	WorldPoint         arrLocation;
	StreetSide         addrSide;
	Address            address;
	float              distFromLocation;
};

struct PVIDAddressInfo
{
    long long		SrcId;  			// Raw data provider source link ID
    int		        AddressType;		// Some link IDs have multiple address ranges per side based on address Type
    UtfString		LeftStartAddress;	// Start house number on left side
    UtfString		LeftEnAddress;		// End house number on left side
    UtfString		RightStartAddress;	// Start house number on right side
    UtfString		RightEnAddress;		// End house number on right side
};

struct NCDB_API InAddress
{
	UtfString	StartNumber;
	UtfString   EndNumber;					//only for range geocoding
	UtfString	Street;
	UtfString	Cross;
	UtfString	Airport;            		//airport code/name
	UtfString	Subcity1;					//can be namedarea/zone. Not used in search.
	UtfString	Subcity2;					//can be namedarea/zone. Not used in search.
	UtfString	City;						//used in serach in azst.ddx
	UtfString	County;						//Not used in search.
	UtfString	State;						//Can be state code/name
	UtfString	Postal;
	UtfString	Line1;						//House num & street name
	UtfString	Line2;						//administrative info
	UtfString	Freeform;					//Contain freeform string.
	UtfString   Country;					//Can be country code/name

    InAddress()
    {
    }

	inline void FromAddress(Address& address)
	{
		StartNumber = address.Number;
		EndNumber = address.EndNumber;
		Street = address.Street;
		Cross = address.CrossStreet;
		Airport = address.Airport;
		Subcity1 = address.NamedArea;
		Subcity2 = address.Zone;
		City = address.City;
		County = address.County;
		State = address.State;
		Postal = address.Zip;
		Country = address.Country;
	}
};

typedef enum GeoReqType
{
    GeoReqComponent = 0,
    GeoReqLine1_2   = 1,
    GeoReqFreeform  = 2,
	GeoReqAirport	= 4					//input assumed to be airport code/name
}GeoReqType;

class GeocodeRequest
{
public:
    InAddress				AddressIn;
    SupportedLanguages 		InLanguageCode;
    unsigned int			LiteMode;
    WorldPoint 				ProxLocation;			//Map Center: used by proximity geocoding
	WorldPoint 				ProxUserLocation;		//Current user Location, if available
    int 					Flags;
	int						WantFlags;				//want flags in singleserachtypes.h.
    GeoReqType              ReqType;

    // Constructor. Set everything to a default value.
    GeocodeRequest():
        InLanguageCode(Invalid_Language), LiteMode(LITEMODE_OFF),
        ProxLocation(-200,-100), Flags(GEOCODE_DEFAULT), WantFlags(0),
        ReqType(GeoReqComponent)
    {
    }

        // Functions to determine address type

        // proximity takes 1st precedence. Valid lat/lon plus street name for now
        bool IsProximityAddress()
        {
            return ( (ProxLocation.x != -200 && ProxLocation.y != -100) &&
                     (!AddressIn.Street.IsEmpty()
                      /* -- FUTURE-- || !Address.Line1.IsEmpty()
                         !Address.Freeform.IsEmpty()*/) );
        }

        // Component Address is defined as a street name plus one admin level
        bool IsComponentAddress()
        {
            return ReqType == GeoReqComponent;
        }

        // Line1 Line2 address is defined as not component and line1 and line2 being non
        // empty
        bool IsLine1Line2Address()
        {
            return ReqType == GeoReqLine1_2;
        }

        // Freeform address is defined as not component, not line1 line2 and freeform being
        // non empty
        bool IsFreeformAddress()
        {
            return (ReqType & GeoReqFreeform) == GeoReqFreeform;
        }

		// Airport address. Can be given in freeform as well as InAddress.Airport.
        bool IsAirport()
        {
            return ReqType == GeoReqAirport;		//it is not necessary that caller will set this. In that case parser to determine it.
        }

		bool IsCityGeocoding()
        {
            //Implementation to follow
            return false;
        }

		bool IsStateGeocoding()
        {
            //Implementation to follow
            return false;
        }

        // Validate the request. For e,g if proximity is set, valid parameter for proximity should be set (at the moment street and/or houseno)
        bool IsValid()
        {
            return IsProximityAddress() || IsComponentAddress() ||
                IsLine1Line2Address() || IsFreeformAddress();
            //return !Address.Street.IsEmpty() && (!Address.City.IsEmpty() || !Address.Zone.IsEmpty() ||
            //                                     !Address.NamedArea.IsEmpty() || !Address.Zip.IsEmpty() ||
            //                                     !Address.State.IsEmpty() || IsProximityAddress());
        }
};

inline void Address::FromInAddress(InAddress& address)
{
	Number = address.StartNumber;
	EndNumber = address.EndNumber;
	Street = address.Street;
	CrossStreet = address.Cross;
	Airport = address.Airport;
	NamedArea = address.Subcity1;
	Zone =  address.Subcity2;
	City = address.City;
	County = address.County;
	State = address.State;
	Zip = address.Postal;
	Country = address.Country;
};

inline void Address::FromOutAddress(OutAddress& address)
{
	Number = address.StartNumber;
	EndNumber = address.EndNumber;

	Street = address.StreetName;
	NameCount = address.StreetList.size()-1;
	int j;
	for(j = 0; j < NameCount; j++)
	{
		Streets[j] = address.StreetList[j+1].fullName;
	}
	for(j = 0; j < NameCount; j++)
	{
		StreetsParts[j] = address.StreetList[j+1];
	}

	CrossStreet = address.CrossStreet;
	CrossNameCount = address.CrossStreetList.size()-1;
	for(j = 0; j < CrossNameCount; j++)
	{
		CrossStreets[j] = address.CrossStreetList[j+1].fullName;
	}
	for(j = 0; j < CrossNameCount; j++)
	{
		CrossStreetsParts[j] = address.CrossStreetList[j+1];
	}

	NamedArea = address.Subcity1;
	City = address.City;
	Zone = address.Subcity2;
	County = address.County;
	State = address.State;
	StateFullName = address.StateFullName;
	Zip = address.Postal;
	Country = address.Country;

	NamedAreaExonymList = address.NamedAreaExonymList;
	CityExonymList = address.CityExonymList;
	ZoneExonymList = address.ZoneExonymList;
	StateExonymList = address.StateExonymList;

	Airport = address.AirportCode;
	AirportName = address.AirportName;

	Admin = address.OutAdmin;
	OutState = address.OutState;
};

typedef struct GeocodeMatch
{
    OutAddress      	AddressOut;						//!< option matched address
    float          	    Confidence;                   	//!< Confidence value from 0.0 to 1.0
    unsigned int        Score;                          //!< Component wise score breakup.
    GeoMatchType        MatchType;                      //!< How the match was retrieved. Values defined in enum GeoMatchType
    GeoMatchMerit       MatchMerit;                     //! <Merit of match. Values defined in enum GeoMatchMerit
    AddressSegmentId    addrSegmentId;					//!< Mainly for E911. useful for Nav use case as well.
    float          	    Distance;						//!< Proximity use. The distance in units from the location
    int            	    ErorConditions;                 //!< Geocode Error conditions flags. See GeocodeErrorConditions.
    WorldPoint          GeocodedLocation;               //!< RP in case of point address, otherwise intrepolated
    WorldPoint          DisplayLocation;                //!< only for point address, otherwise default
    WorldPoint          InterpolatedLocation;           //!< interpolated point (always)
	BoundingBox			BBox;							//!< Bounding box in case of city/state/postal geocoding
	BoundingBox		    viewPort;						//!< Best Possible Viewport
    SupportedLanguages  OutLanguageCode;                //!< Future Use

    // Constructor. Sets everything to a default
    GeocodeMatch():
        Confidence(0.0f), Score(0), MatchType(GCMatch_Undefined),
        MatchMerit(GCMerit_BelowThreshold), Distance(0.0f), ErorConditions (0),
        GeocodedLocation(-200,-100), DisplayLocation(-200,-100),
        InterpolatedLocation(-200,-100), OutLanguageCode(Invalid_Language)
    {
    };
}GeocodeMatch;

}; //end namespace NCDB
#endif // NCDB_GEOCODE_TYPES_H
/*! @} */
