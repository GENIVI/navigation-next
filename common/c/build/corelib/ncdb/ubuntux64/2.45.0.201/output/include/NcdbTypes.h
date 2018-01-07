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

    @file     NcdbTypes.h
    @date     04/24/2009
    @defgroup MOBIUS_COMMON  Mobius Common API 

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
#ifndef NCDB_TYPES_H_ONCE
#define NCDB_TYPES_H_ONCE

// For MobiTest compatibility with other code branches only
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
+ Defines
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
#define NEW_FEATURE_SPECIAL_REGION
#define NEW_FEATURE_FIRST_MANEUVER_CROSSROAD
#define NEW_FEATURE_ROUTE_FEATURES          // New feature, support "want-route-features"
#define NEW_FEATURE_PARKING_LOT
#define MAP_MEM_OPTS_DEFINED                // Map can be opened with memory options
#define NO_MANEUVER_LIST_OPTION_DEFINED     // Routing options support no maneuver list generation
#define CO_MANEUVER_DEFINED                 // Support new "CO"(Continue On) maneuver
#define PRIORITY_MANEUVER_EXISTS            // The concept exists, doesn't mean the data has it
#define ELECTRONIC_TOLL_EXISTS              // The concept exists, doesn't mean the data has it
#define DESTINATION_ADDRESS_SUPPORT         // Support the destination in address feature
#define NBM_NEXGEN_FEATURES                 // support nexgen features
#define NEW_FEATURE_PARKING_LOT
#define ROUTE_TIME_LIMIT_DEFINED
#define DEST_STREET_NAME_VALIDATION_DEFINED
#define OPENLR_SUPPORT
#define FAST_ETA_DEFINED
#define FAST_ETA_DEFINED_2                  // Extended Fast ETA version with segment list 
#define FAST_ETA_DEFINED_3                  // Extended Fast ETA version with dist limit and return codes 
#define PEDESTRIAN_PREFERRED
#define NEW_NIGHT_MODE_API
#define SEGMENT_DTM_IS_APPLICABLE_API
#define TRAFFIC_FLOW_MAP_API
#define MULTIPLE_MAP_STYLES
#define HAS_DAWN_DUSK_DTM
#define SESSION_OVERRIDE_INI_API    2       // Session API INI override and InitOptions
#define EXTRA_TRAFFIC_SIGNS         1       // Additional traffic sign types are defined
#define ONBOARD_PHONEME_TEXT        1       // Support onboard phoneme data format
#define UTC_OFFSET_BY_POINT         1       // Support GetUTCOffsetForGivenPoint in RouteEngine

#if defined (_MSC_VER) || defined (NO_TRAFFIC_LIB)
    #define TRAFFIC_EMULATION       7
#else
#if defined (TRAFFIC_LIB_VER)
#if TRAFFIC_LIB_VER==0
    #define TRAFFIC_EMULATION       7
#endif
#endif
#endif

#ifdef _MSC_VER
    // Switching on Windows Memory leaks detection.
    #ifndef _CRTDBG_MAP_ALLOC
        #define _CRTDBG_MAP_ALLOC   
    #endif
    #include <stdlib.h>
    #include <crtdbg.h>
    #pragma warning(disable : 4996)
/*
    Disable warning C4512: "assignment operator could not be generated"
    This is due to the 'const'/'nonstatic reference' members of some of the classes.
    Assignment operators are not needed for these classes.
*/
    #pragma warning(disable:4512)

#ifdef NCDB_EXPORTS
	#ifndef NCDB_API
	#define NCDB_API  __declspec (dllexport)
	#endif
#elif NCDB_STATIC_IMPORT
	#ifndef NCDB_API
	#define NCDB_API
	#endif
#else
	#ifndef NCDB_API
	#define NCDB_API  __declspec (dllimport)
	#endif
#endif
#else
#ifdef NCDB_EXPORTS
	#ifndef NCDB_API
	#define NCDB_API  __attribute__ ((visibility("default")))
	#endif
#else
	#ifndef NCDB_API
	#define NCDB_API
	#endif
#endif
#endif

//#include "WorldPoint.h"
#include "WorldRect.h"
#include "PixelPoint.h"
#include "PixelRect.h"

namespace Ncdb {

//! Error definition enum
typedef enum {
	//! succcessful operation 
	NCDB_OK = 0 ,

// File Error
    //! File could not be opened.
	NCDB_OPEN_ERR = 101,
	//! File read failure.
	NCDB_READ_ERR = 102,
	//! File write failure.
	NCDB_WRITE_ERR = 103,
	//! access denied 
	NCDB_ACCESS_DENIED = 104,
	//! Incompatible format
	NCDB_INVALID_FORMAT = 105,
	//! Incompatible format
	NCDB_INVALID_SUFFIX = 106,

// Query return codes
    //! for more records to be retrieved 
	NCDB_MORE_RECORDS    = 201,
    //! query end condition: no more records to read
    NCDB_END_OF_QUERY    = 202,
	//! no objects are found by query. 
    NCDB_NOT_FOUND       = 203,
	NCDB_TIME_OUT = 204, //Request Timedout
	//! retrieved data contain no information 
	NCDB_EMPTY_DATA      = 211,
	//! output buffer failure 
	NCDB_BUFFER_OVERFLOW = 212, 
	//! query handler is not valid 
	NCDB_INVALID_HANDLER = 221,
	//! feature not supported 
	NCDB_UNSUPPORTED     = 222,

//Router return codes
    //! origin link not accessible for vehicle type within search radius
    NCDB_ORIGIN_NOT_ACCESSIBLE = 301,
    //! way too long for pedestrian or bicycle
    NCDB_TOO_LONG       = 302,
    //! dest link not accessible for vehicle type within search radius
    NCDB_DEST_NOT_ACCESSIBLE = 303,
    //! route cost too high
    NCDB_COST_LIMIT = 304,
    //! no alternate route found
    NCDB_NO_ALTERNATE = 305,
    //! alternate route not acceptible
    NCDB_BAD_ALTERNATE = 306,

//Traffic Library Related error codes
    //!Traffic Reader could not be created or is null
	NCDB_TRAFFICREADER_NULL = 401,
    //!Unable to read in the historical data
	NCDB_HISTDATAREAD_ERR = 402,
	//!Unable to update live data
    NCDB_LIVEDATA_UPDATE_ERR = 403,
	//!Unable to update incident data
    NCDB_INCIDENTDATA_UPDATE_ERR = 404,
	//!Incorrent provider or path information
	NCDB_PROVIDER_PATH_ERR = 405,

// Map Display related error codes
	//!Invalid map center location.
	NCDB_INVALID_MAP_CENTER = 501,

// Exceptional conditions
	//! failed for unknown reason 
	NCDB_FAIL       = 1001,
	//! aborted due to unknown error 
	NCDB_ABORT      = 1002,
	//! invalid argument or combination
	NCDB_INVALID    = 1003,
	//! invalid pointer 
	NCDB_BAD_POINTER = 1006,

// System errors
	//! out of memory 
	NCDB_NO_MEMORY  = 2001,
	//! OS system API call failed
	NCDB_OS_ERROR   = 2002,
    //! third party component call failed
    NCDB_THIRD_PARTY_FAILURE    = 2003
} ReturnCode;


//! detailed map file extension
#define DETAILED_MAP_EXTENSION      ".biz"     
//! POI map file extension
#define POI_MAP_EXTENSION           ".bin"           
//! Maximum size of a name stored in NCDB
#define NCDB_MAX_AREA_NAME_SIZE     2048   // increased from 128 with exonyms support   2011.03.07 re-increased 
#define NCDB_MAX_NAME_SIZE          2048   // Edit by sevnsson ... 2010.10.13	Some of EU date need the longer memory 
										   // re-increased  2011.03.07/ increase it again, maybe it is still not enough for EEU


//! brief        different map types 
//! description  various map types being supported 

typedef enum
{
	//! bad/invalid map type
	Invalid_DatabaseType=0,  
	//! Basemap type
	Base_DatabaseType,
	//! POI map 
	Poi_DatabaseType, 
	//! street map type
	StreetNav_DatabaseType            
} DatabaseType;


//!  brief        different map object types
//!  description  various map object types being supported

typedef enum
{
	//! point type 
	Point_MapObj = 11,  
	//! polygon type 
	Polygon_MapObj = 12,              
	//! Link type 
	Link_MapObj = 13,                 
	//! label type 
	Label_MapObj = 15,             
	//! POI type 
	PoiPoint_MapObj = 16,             
	//! Segment with nodes defined 
	Segment_MapObj  = 17,             
	//! Hwy Shield 
	Shield_MapObj   = 18,             
	//! Sign board 
	Sign_MapObj     = 19,             
	//! number of object types 
	Num_MapObj = 19         
} MapObj;

//! brief        languages support
//! description  all languages being supported

typedef enum
{
	Invalid_Language	= 0,
	US_English	        = 1, 
	Australian_English  = 2,  
	Basque              = 3,
	Belgian_Dutch       = 4,
	Brazilian_Portuguese= 5,
	British_English     = 6, 
	Canadian_French     = 7, 
	Catalan             = 8,
	Czech               = 9,
	Danish              = 10, 
	Finnish             = 11,
	French              = 12,
	German              = 13, 
	Greek               = 14, 
	Hindi               = 15,
	Indian_English      = 16,
	Irish_English       = 17,
	Italian             = 18, 
	Mexican_Spanish     = 19,
	Norwegian           = 20, 
	Polish              = 21,
	Portuguese          = 22, 
	Russian             = 23,
	Scottish_English    = 24,
	Spanish             = 25, 
	Swedish             = 26, 
	Turkish             = 27,
	Chinese				= 28,
	Welsh               = 29,
	Arabic              = 30,
    Malay               = 31,
    Indonesian          = 32,
	Thai                = 33,
	Vietnamese          = 34,
    Hebrew              = 35,
	Chinese_Simplified	= 36,
    Unknown_Langauge            // max value and unknown language from data.
} SupportedLanguages;

#define  NCDB_DEFAULT_LANGUAGE_CODE   US_English

//Should be consistant with g_Langs (in file ExonymString.cpp) 
typedef enum
{
	Exonym_Invalid	                = 0,
    Exonym_Albanian	                = 1,
    Exonym_Arabic	                = 2,
    Exonym_Basque	                = 3,
    Exonym_Belarusian	            = 4,
    Exonym_BelarusianTransliterated = 5,
    Exonym_Bosnian                  = 6,
    Exonym_BosnianTransliterated    = 7,
    Exonym_Bulgarian                = 8,
    Exonym_BulgarianTranscribed     = 9,
    Exonym_BulgarianTransliterated  = 10,
    Exonym_Catalan                  = 11,
    Exonym_ChineseModern            = 12,
    Exonym_ChineseTraditional       = 13,
    Exonym_Croatian                 = 14,
    Exonym_CroatianTransliterated   = 15,
    Exonym_Czech                    = 16,
    Exonym_CzechTransliterated      = 17,
    Exonym_Danish                   = 18,
    Exonym_Dutch                    = 19,
    Exonym_English                  = 20,
    Exonym_Estonian                 = 21,
    Exonym_EstonianTransliterated   = 22,
    Exonym_Finnish                  = 23,
    Exonym_French                   = 24,
    Exonym_German                   = 25,
    Exonym_IrishGaelic              = 26,
    Exonym_Galician                 = 27,
    Exonym_Greek                    = 28,
    Exonym_ModernGreek              = 29,
    Exonym_GreekTransliterated      = 30,
    Exonym_Hungarian                = 31,
    Exonym_HungarianTransliterated  = 32,
    Exonym_Icelandic                = 33,
    Exonym_BahasaIndonesia          = 34,
    Exonym_Italian                  = 35,
    Exonym_Latvian                  = 36,
    Exonym_LatvianTransliterated    = 37,
    Exonym_Lithuanian               = 38,
    Exonym_LithuanianTransliterated = 39,
    Exonym_Macedonian               = 40,
    Exonym_MacedonianTranscribed    = 41,
    Exonym_Malaysian                = 42,
    Exonym_Moldovan                 = 43,
    Exonym_MoldovanTransliterated   = 44,
    Exonym_Montenegrin              = 45,
    Exonym_MontenegrinTransliterated= 46,
    Exonym_Norwegian                = 47,
    Exonym_Polish                   = 48,
    Exonym_PolishTransliterated     = 49,
    Exonym_Portuguese               = 50,
    Exonym_Pinyin                   = 51,
    Exonym_RomanianTransliterated   = 52,
    Exonym_Romanian                 = 53,
    Exonym_Russian                  = 54,
    Exonym_RussianTranscribed       = 55,
    Exonym_RussianTransliterated    = 56,
    Exonym_Serbian                  = 57,
    Exonym_SerbianTranscribed       = 58,
    Exonym_SerbianTransliterated    = 59,
    Exonym_Slovak                   = 60,
    Exonym_SlovakTransliterated     = 61,
    Exonym_Slovenian                = 62,
    Exonym_SlovenianTransliterated  = 63,
    Exonym_Spanish                  = 64,
    Exonym_Swedish                  = 65,
    Exonym_Thai                     = 66,
    Exonym_ThaiEnglish              = 67,
    Exonym_TurkishTranscribed       = 68,
    Exonym_Turkish                  = 69,
    Exonym_TurkishTransliterated    = 70,
    Exonym_Ukrainian                = 71,
    Exonym_UkrainianTransliterated  = 72,
    Exonym_Welsh                    = 73,
    Exonym_MaltaInIT5e              = 74,
    Exonym_Maltese                  = 75,
    Exonym_Kazakh                   = 76,
    Exonym_SCR                      = 77,
    Exonym_HIN                      = 78,
    Exonym_JPN                      = 79,
    Exonym_KOR                      = 80,
    Exonym_UKT                      = 81,
    Exonym_TRA                      = 82,
    Exonym_UND                      = 83,
    Exonym_WesternFrisian           = 84,
    Exonym_ARM                      = 85, //full name of languages below, in "ExonymString.cpp"
    Exonym_AUS                      = 86,
    Exonym_AZE                      = 87,
    Exonym_BAT                      = 88,
    Exonym_BET                      = 89,
    Exonym_BRE                      = 90,
    Exonym_BUN                      = 91,
    Exonym_BUR                      = 92,
    Exonym_CEL                      = 93,
    Exonym_CHL                      = 94,
    Exonym_EPO                      = 95,
    Exonym_FAO                      = 96,
    Exonym_GEM                      = 97,
    Exonym_GEO                      = 98,
    Exonym_GLA                      = 99,
    Exonym_GRC                      = 100,
    Exonym_GRL                      = 101,
    Exonym_GSW                      = 102,
    Exonym_HAW                      = 103,
    Exonym_HEB                      = 104,
    Exonym_HRV                      = 105,
    Exonym_KHM                      = 106,
    Exonym_KOS                      = 107,
    Exonym_LAO                      = 108,
    Exonym_LAT                      = 109,
    Exonym_LTZ                      = 110,
    Exonym_MAO                      = 111,
    Exonym_MAP                      = 112,
    Exonym_MLS                      = 113,
    Exonym_MON                      = 114,
    Exonym_MYN                      = 115,
    Exonym_NAI                      = 116,
    Exonym_NNO                      = 117,
    Exonym_NOB                      = 118,
    Exonym_OCI                      = 119,
    Exonym_PAA                      = 120,
    Exonym_PER                      = 121,
    Exonym_PUS                      = 122,
    Exonym_ROA                      = 123,
    Exonym_ROH                      = 124,
    Exonym_RUL                      = 125,
    Exonym_SCC                      = 126,
    Exonym_SCO                      = 127,
    Exonym_SCY                      = 128,
    Exonym_SLA                      = 129,
    Exonym_SMI                      = 130,
    Exonym_SMN                      = 131,
    Exonym_SRD                      = 132,
    Exonym_SRP                      = 133,
    Exonym_SWA                      = 134,
    Exonym_TAM                      = 135,
    Exonym_TGL                      = 136,
    Exonym_THL                      = 137,
    Exonym_TIB                      = 138,
    Exonym_UKL                      = 139,
    Exonym_URD                      = 140,
    Exonym_UZB                      = 141,
    Exonym_VAL                      = 142,
    Exonym_VIE                      = 143,
    Exonym_WEN                      = 144,
    Exonym_YID                      = 145,
    Exonym_ZUL                      = 146,
    Exonym_GRN                      = 147,
    Exonym_SMC                      = 148,
    Exonym_SML                      = 149,
    Exonym_HEL                      = 150,
    Exonym_CTN                      = 151,
    Exonym_Unknown            // max value and unknown language from data.
} ExonymLanguages;

typedef enum
{
    LinkDirectionUnknown = 0,
    LinkDirectionForward = 1,
    LinkDirectionBackward = 2,
    LinkDirectionBoth = 3
}
LinkDirection;

typedef enum 
{
    LabelModeNoLabel = -1,  // Not draw any labels
	LabelModeStatic=0,
	LabelModeDynamic = 1,
	LabelModeMax
}
LabelMode ;

typedef enum 
{
	ProjectionModeEquidistant=0,
	ProjectionModeMercator,
	ProjectionModeMercatorTile,
	ProjectionModeMercatorCenter,
	ProjectionModeNone,
	ProjectionModeMax
}
ProjectionMode ;

typedef enum
{
	ColorMode555=0,
	ColorMode565,
	ColorModeRGB24,
	ColorModeBGR24,
	ColorModeRGBA32,
	ColorModeBGRA32,
	ColorModeMax,
	ColorModePNG8
}
ColorMode;

typedef enum
{
	DisplayMode2D=0,
	DisplayMode3D,
	DisplayModeMax
}
DisplayMode;

//! Pan mode numeration.
typedef enum
{
	PanModeNormal,
	PanModeFastest,
	PanModeMax
}
PanMode;


// ======================================================================================
//!    Street side 
// ======================================================================================
typedef enum 
{
    StreetSide_Unknown = 0,
    StreetSide_Left,
    StreetSide_Right
}   StreetSide;

enum IconId {
	IconIdUnknown,
    IconIdNationalCapital,
    IconIdWorldCapital,
    IconIdCapital,
    IconIdMajorCity,
    IconIdCity,
    IconIdMinorCity,
    IconIdTown,
    IconIdOtherPlace,
    IconIdMapLocate,
	IconIdMax,
};

typedef int IniHashCode;

enum CountryId
{
	CountryIdUnknown			= 0,
	CountryIdUSA				= 1,
	CountryIdMexico				= 2,
	CountryIdBahamas			= 3,
	CountryIdCaymanIslands		= 4,
	CountryIdCanada				= 10,
	CountryIdUK					= 99,
	CountryIdUKEngland			= 100, // UK
	CountryIdUKScotland			= 101, // UK
	CountryIdUKWales			= 102, // UK
	CountryIdUKNireland			= 103, // UK
	CountryIdUKIsleofman		= 104, // UK
	CountryIdUKChannelIslands	= 105, // UK
	CountryIdIreland            = 106,
	CountryIdFrance             = 107,
	CountryIdMonaco             = 108,
	CountryIdAndorra            = 109,
	CountryIdSchweiz            = 110, // switzerland
	CountryIdEspana             = 111, // spain
	CountryIdDeutschland        = 112, // germany
	CountryIdBelgium			= 113,
	CountryIdAustria			= 114,
	CountryIdItalia			    = 115,
	CountryIdLuxembourg	        = 116,
	CountryIdNederland		    = 117,
	CountryIdPortugal		    = 118,
	CountryIdNorge              = 119,
	CountryIdSverige            = 120,
	CountryIdSuomi              = 121,
	CountryIdVaticano           = 122,
	CountryIdDanmark            = 123,
	CountryIdSanmarino          = 124,
	CountryIdLiechtenstein      = 125,
	CountryIdGibraltar          = 126,
	CountryIdBrazil             = 127,
	CountryIdAustralia          = 129,
	CountryIdMalta              = 130,
    CountryIdMalaysia           = 145,
	CountryIdUae                = 200,
	CountryIdOman               = 201,
    CountryIdGuadeloupe         = 202,
    CountryIdMartinique         = 203
};

enum StateId
{
	StateIdUnknown					= -1,
	StateIdAlabama					= 0,
	StateIdAlaska					= 1,
	StateIdArizona					= 2,
	StateIdArkansas					= 3,
	StateIdCalifornia				= 4,
	StateIdColorado					= 5,
	StateIdConnecticut				= 6,
	StateIdDelaware					= 7,
	StateIdDistrictOfColumbia		= 8,
	StateIdFlorida					= 9,
	StateIdGeorgia					= 10,
	StateIdHawaii					= 11,
	StateIdIdaho					= 12,
	StateIdIllinois					= 13,
	StateIdIndiana					= 14,
	StateIdIowa						= 15,
	StateIdKansas					= 16,
	StateIdKentucky					= 17,
	StateIdLouisiana				= 18,
	StateIdMaine					= 19,
	StateIdMaryland					= 20,
	StateIdMassachusetts			= 21,
	StateIdMichigan					= 22,
	StateIdMinnesota				= 23,
	StateIdStateIdMississippi		= 24,
	StateIdMissouri					= 25,
	StateIdMontana					= 26,
	StateIdNebraska					= 27,
	StateIdNevada					= 28,
	StateIdNewHampshire				= 29,
	StateIdNewJersey				= 30,
	StateIdNewMexico				= 31,
	StateIdNewYork					= 32,
	StateIdNorthCarolina			= 33,
	StateIdNorthDakota				= 34,
	StateIdOhio						= 35,
	StateIdOklahoma					= 36,
	StateIdOregon					= 37,
	StateIdPennsylvania				= 38,
	StateIdRhodeIsland				= 39,
	StateIdSouthCarolina			= 40,
	StateIdSouthDakota				= 41,
	StateIdTennessee				= 42,
	StateIdTexas					= 43,
	StateIdUtah						= 44,
	StateIdVermont					= 45,
	StateIdVirginia					= 46,
	StateIdWestVirginia				= 48,
	StateIdWisconsin				= 49,
	StateIdWyoming					= 50,
	StateIdOntario					= 60,
	StateIdQuebec					= 61,
	StateIdManitoba					= 62,
	StateIdSasakatchewan			= 63,
	StateIdAlberta					= 64,
	StateIdBritishColumbia			= 65,
	StateIdNewBrunswick				= 66,
	StateIdPrinceEdwardIsland		= 67,
	StateIdNovaScotia				= 68,
	StateIdNewfoundland				= 69,
	StateIdNorthwestTerritories		= 70,
	StateIdNunavut					= 71,
	StateIdYukonTerritory			= 72,
	StateIdPuertoRico				= 73,
	StateIdUSVirginIslands			= 74,
	StateIdUKEngland				= 200, // UK
	StateIdUKScotland			    = 201, // UK
	StateIdUKWales				    = 202, // UK
	StateIdUKNireland			    = 203, // UK
	StateIdUKIsleofman			    = 204, // UK
	StateIdUKChannelIslands			= 205, // UK
	StateIdIreland                  = 206,
	StateIdFrance				    = 207,
	StateIdMonaco				    = 208,
	StateIdAndorra				    = 209,
	StateIdSchweiz				    = 210, // switzerland
	StateIdEspana				    = 211, // spain
	StateIdDeutschland			    = 212, // germany
	StateIdBelgium				    = 213,
	StateIdAustria				    = 214,
	StateIdItalia				    = 215,
	StateIdLuxembourg			    = 216,
	StateIdNederland				= 217,
	StateIdPortugal				    = 218,
	StateIdNorge					= 219,
	StateIdSverige				    = 220,
	StateIdSuomi					= 221,
	StateIdVaticano				    = 222,
	StateIdDanmark				    = 223,
	StateIdSanmarino				= 224,
	StateIdLiechtenstein			= 225,
	StateIdGibraltar				= 226,
	StateIdMalta					= 227,
	StateIdFaroeIslands			    = 228,
	StateIdBrazil				    = 229,
	StateIdAustralia				= 230,
	// UAE
	StateIdUae					    = 300,
	StateIdOman					    = 301
};

enum RouteId
{
	RouteIdNone			= 0,
	RouteIdInterstate	= 1,
	RouteIdHighway		= 2,
	RouteIdState		= 3,
	RouteIdCounty		= 4,
	RouteIdMax,
};

typedef enum
{
	FontCaseAny=0,
	FontCaseUpper,
	FontCaseLower,
	FontCaseMixed,
	FontCaseMax
}
FontCase;

typedef enum
{
	FontAlignmentCenter=0,
	FontAlignmentLeft,
	FontAlignmentRight,
	FontAlignmentMax
}
FontAlignment;

typedef enum
{
	AreaStyleAny=0,
	AreaStyleWater,
	AreaStylePolitical,
	AreaStyleCampus,
	AreaStylePark,
	AreaStyleBuilding,
	AreaStyleInterior,
	AreaStyleIsland,
	AreaStyleHole,
	AreaStyleMax
}
AreaStyle;

typedef enum
{
	AreaUsageAny=0,
	AreaUsageOcean,
	AreaUsageGolfCourse,
	AreaUsageLocalPark,
	AreaUsageStatePark,
	AreaUsageNationalPark,
	AreaUsagePark,
	AreaUsageNationalForest,
	AreaUsageNationalOther,
	AreaUsageLake,
	AreaUsageAirport,
	AreaUsageWoodland,
	AreaUsageBuiltUp,
	AreaUsageIndustrial,
	AreaUsageMilitary,
	AreaUsageBayHarbour,
	AreaUsageRiver,
	AreaUsageRunway,
	AreaUsageIsland,
	AreaUsageCountry,
	AreaUsageCemetery,
	AreaUsageState,
	AreaUsageReservation,
	AreaUsageHospital,
	AreaUsageSports,
	AreaUsageShopping,
	AreaUsageUniversity,
	AreaUsageBusiness,
	AreaUsageConvention,
	AreaUsageCultural,
	AreaUsageEducation,
	AreaUsageEmergency,
	AreaUsageGovernment,
	AreaUsageHistorical,
	AreaUsageMedical,
	AreaUsageResidential,
	AreaUsageRetail,
	AreaUsageTourist,
	AreaUsageTransportation,
	AreaUsageUnclassified,
	AreaUsageInWater,
	AreaUsageMax
}
AreaUsage;

typedef enum
{
	LineStyleAny=0,
	LineStyleRoad,
    LineStyleTunnel,
	LineStyleLimited,
	LineStyleWater,
	LineStyleCartographic,
	LineStylePolitical,
	LineStyleMax
}
LineStyle;

typedef enum
{
	LineUsageAny=0,
	LineUsageRailroad,
	LineUsageRiver,
	LineUsageInterchange,
	LineUsageNotDrivable,
	LineUsageUnpaved,
	LineUsageCarpool,
	LineUsageBridge,
	LineUsageCountryBorder,
	LineUsageStateBorder,
	LineUsageTunnel,
	LineUsageFerryBoat,
	LineUsageRamp,
	LineUsageBussOnly,
	LineUsageTollRoad,
	LineUsageTollRamp,
	LineUsageLocal,
	LineUsageNormal,
	LineUsageRegional,
	LineUsageHighway,
	LineUsageNational,
	LineUsageVirtual,
	LineUsageWalkway,
	LineUsagePrivate,
	LineUsageDisputedCountryBorder,
	LineUsageDateEquatorLine,
    LineUsageShadow,
	LineUsageMax
}
LineUsage;

typedef enum
{
	PointTypeAny=0,
	PointTypeBasic,
	PointTypeCore,
	PointTypeStatic,
	PointTypeExtended,
	PointTypeMax
}
PointType;

typedef enum
{
	PointStyleAny=0,
	PointStyleAutomotive,
	PointStyleCommunity,
	PointStyleEducation,
	PointStyleEntertainment,
	PointStyleFoodDining,
	PointStyleHealthMedication,
	PointStyleHomeGarden,
	PointStyleLegalFinancial,
	PointStyleProfessionalService,
	PointStylePersonalCare,
	PointStyleRecreationSports,
	PointStyleRetail,
	PointStyleTravelTransportation,
	PointStyleNamedPlace,
	PointStyleMax
}
PointStyle;

typedef enum
{
	PointUsageAny=0,
	PointUsageNationalCapital,
	PointUsageCapital,
	PointUsageMajorCity,
	PointUsageCity,
	PointUsageMinorCity,
	PointUsageTown,
    PointUsageHamlet,
    PointUsageNeighborhood,
    PointUsageBorough,
	PointUsageTransit,
	PointUsageOther,
	PointUsageAirport,
	PointUsageState,
	PointUsageStateAbbreviation,
    PointUsageSuperCountry,
    PointUsageLargeCountry,
    PointUsageMediumCountry,
    PointUsageSmallCountry,
    PointUsageSpecialArea,
    PointUsageBigCountry,   //only for static label
	PointUsageCountryAbbreviation,
	PointUsageOcean,
    PointUsageContinent,
	PointUsageMax
}
PointUsage;

typedef enum
{
	KindOfLineSolid=0,
	KindOfLineDash,
	KindOfLineDot,
	KindOfLineDashDotDot,
	KindOfLineRailroad,
	KindOfLineNone,
	KindOfLineMax
}
KindOfLine;

typedef enum
{
	KindOfCircleNormal=0,
	KindOfCircleSolid,
	KindOfCircleMax
}
KindOfCircle;

typedef enum
{
	LabelLocationCenterline=0,
	LabelLocationOffset,
	LabelLocationMax
}
LabelLocation;

typedef enum
{
	ArrowHeadStyleNormal=0,
	ArrowHeadStyleContour,
	ArrowHeadStyleSolid,
	ArrowHeadStyleMax
}
ArrowHeadStyle;

typedef enum
{
    ArrowTypeNormal = 0,
    ArrowTypeRIM,
    ArrowTypeMax
}
ArrowType;

typedef enum
{
    LabelLetterTypeRegular = 0,
    LabelLetterTypeAllUpper,
    LabelLetterTypeAllLower
}
LabelLetterType;

typedef enum
{
    LabelOverlayNormal = 0,
    LabelOverlayWatermark
}
LabelOverlayType;

typedef enum
{
	LabelPriorityLowest = 100,
	LabelWeightLowest = 0
}LabelPropertyLowest;
typedef int LabelPriority;
typedef int LabelWeight;

//!  brief Memory management options for map data.
#define MAP_MEM_OPTS_VER    3
typedef enum 
{
    //! Current default behavior as defined by main ini file
    MM_Default,
    //! Use code default behavior, overrides other input
    MM_None,
    //! Allow some files to use memory mapped access
    MM_MMF,
    //! Allow Highway layer blocks to be cached after loading.
    MM_HwyCache,
    //! Use thin blocks only (for debugging use).
    MM_ThinBlocksOnly,
    //! Use hwy blocks only (for debugging use).
    MM_HwyBlocksOnly,
    //! Use local blocks only (for debugging use).
    MM_LocalBlocksOnly,
} MapMemOptions;

typedef enum
{
	localGlobal,
    localIndia,
} LocalId;

typedef enum
{
	CoreMap = 0,
	WorldMap,
	OuterMap,
} MapDataSource;

};

// Maximum number of maps that can be open in one session.
#define MAX_MAPS    16


#endif // NCDB_TYPES_H_ONCE

/*! @} */


