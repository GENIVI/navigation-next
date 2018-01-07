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

    @file     nblocation.h
*/
/*
    (C) Copyright 2005 - 2009 by Networks In Motion, Inc.

    The information contained herein is confidential, proprietary
    to Networks In Motion, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#ifndef NBLOCATION_H
#define NBLOCATION_H

#include "nberror.h"
#include "paltypes.h"

/*! @{ */

/*! An enum type to categorize the kind of location information available
    in a ::NB_Location struct.
 */
typedef enum {

	NB_Location_None = 0,		/*!< No location information available */
	NB_Location_Address,		/*!< Address as in Street number, Street, City, State, ZIP code */
	NB_Location_Intersection,	/*!< Intersection between Street 1, Street 2 at City, State, ZIP code */
	NB_Location_Airport,		/*!< Airport name */
	NB_Location_LatLon,         /*!< Latitude, Longitude */
	NB_Location_MyGPS,          /*!< GPS location */
	NB_Location_Route,          /*!< Location information along a route */
	NB_Location_RouteGPS,		/*!< GPS location, along a route */
	NB_Location_InMyDirection, 	/*!< To indicate a location in my direction */
    NB_Location_AddressFreeForm /*!< Address as in Street number, Street, City, State, ZIP code in free form*/
} NB_LocationType;

/*! \brief Maximum length for the areaname string in a ::NB_Location struct. */
#define NB_LOCATION_MAX_AREANAME_LEN        128

/*! \brief Maximum length for the street number string in a ::NB_Location struct. */
#define NB_LOCATION_MAX_STREETNUM_LEN       20

/*! \brief Maximum length for a Street string in a ::NB_Location struct. */
#define NB_LOCATION_MAX_STREET_LEN          128

/*! \brief Maximum length for the City string in a ::NB_Location struct. */
#define NB_LOCATION_MAX_CITY_LEN            150

/*! \brief Maximum length for the County string in a ::NB_Location struct. */
#define NB_LOCATION_MAX_COUNTY_LEN          64

/*! \brief Maximum length for the State string in a ::NB_Location struct. */
#define NB_LOCATION_MAX_STATE_LEN           64

/*! \brief Maximum length for the ZIP postal code string in a ::NB_Location struct. */
#define NB_LOCATION_MAX_POSTAL_LEN          10

/*! \brief Maximum length for the Country string in a ::NB_Location struct. */
#define NB_LOCATION_MAX_COUNTRY_LEN         32

/*! \brief Maximum length for the Airport string in a ::NB_Location struct. */
#define NB_LOCATION_MAX_AIRPORT_LEN         4

/*! \brief Maximum length for the Airport string in a ::NIM_Location struct. */
#define NB_LOCATION_MAX_FREE_FORM_LEN       516

/*! Maximum length for the address line */
#define MAX_ADDRESS_LINE_LEN                128

/*! Maximum length for the line separator */
#define MAX_LINE_SEPARATOR_LEN              4

/*! Maximum count of address lines in NB_ExtendedAddress */
#define NB_EXTENDED_ADDRESS_MAX_ADDRESS_LINE_COUNT      5

/*! Maximum count of address lines in NB_CompactAddress */
#define NB_COMPACT_ADDRESS_MAX_ADDRESS_LINE_COUNT       2

/*! Extended formatted address (max of 5 lines) for POI detail screen shall be provided. */
typedef struct
{
    int lineCount;                              /*!< Count of address lines, max value is 5 for extended
                                                     address. if its value is 0, there is no extended
                                                     address specified. */
    char addressLine1[MAX_ADDRESS_LINE_LEN];
    char addressLine2[MAX_ADDRESS_LINE_LEN];
    char addressLine3[MAX_ADDRESS_LINE_LEN];
    char addressLine4[MAX_ADDRESS_LINE_LEN];
    char addressLine5[MAX_ADDRESS_LINE_LEN];
} NB_ExtendedAddress;

/*! Compact formatted address (max of 2 lines) for list view shall be provided. */
typedef struct
{
    nb_boolean hasLineSeparator;                /*!< Is the line separator present? */
    int lineCount;                              /*!< Count of address lines, max value is 2 for compact
                                                     address. if its value is 0, there is no compact
                                                     address specified. */
    char lineSeparator[MAX_LINE_SEPARATOR_LEN]; /*!< If present, must insert between to line 1 and
                                                     line2 when address is shown in one line. If not
                                                     present for one line address, client shall put a
                                                     space between line1 and line2.*/
    char addressLine1[MAX_ADDRESS_LINE_LEN];
    char addressLine2[MAX_ADDRESS_LINE_LEN];
} NB_CompactAddress;

/*! \brief Main type describing a location.

    The value of the `type' field describes where to find the location
    information in the other fields.
 */
typedef struct {

    char    areaname[NB_LOCATION_MAX_AREANAME_LEN+1];
    char    streetnum[NB_LOCATION_MAX_STREETNUM_LEN+1];
    char    street1[NB_LOCATION_MAX_STREET_LEN+1];
    char    street2[NB_LOCATION_MAX_STREET_LEN+1];
    char    city[NB_LOCATION_MAX_CITY_LEN+1];
    char    county[NB_LOCATION_MAX_COUNTY_LEN+1];
    char    state[NB_LOCATION_MAX_STATE_LEN+1];
    char    postal[NB_LOCATION_MAX_POSTAL_LEN+1];
    char    country[NB_LOCATION_MAX_COUNTRY_LEN+1];         /*!< ISO country code, not country name. */
    char    airport[NB_LOCATION_MAX_AIRPORT_LEN+1];
    char    freeform[NB_LOCATION_MAX_FREE_FORM_LEN+1];
    char    country_name[NB_LOCATION_MAX_COUNTY_LEN+1];
    double  latitude;
    double  longitude;
    NB_LocationType type;
    NB_ExtendedAddress extendedAddress;     /*!< If present, an extended formatted address (max of 5 lines)
                                                 for POI detail screen shall be provided. */
    NB_CompactAddress compactAddress;       /*!< If present, a compact formatted address (max of 2 lines)
                                                 for list view shall be provided. */
} NB_Location;

/*! @} */

#endif
