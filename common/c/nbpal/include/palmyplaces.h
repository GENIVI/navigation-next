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
    @file     palmyplaces.h
*/
/*
    (C) Copyright 2012 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems, is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
/*! @{ */

#ifndef PALMYPLACES_H
#define PALMYPLACES_H


#include "pal.h"
#include "paltypes.h"
#include "palstdlib.h"

#define PAL_MYPLACES_STRLEN                    100

#define PAL_MYPLACES_CATEGORY_CODE_STRLEN      10
#define PAL_MYPLACES_CATEGORY_NAME_STRLEN      50

#define PAL_MYPLACES_PHONE_COUNTRY_LEN          6
#define PAL_MYPLACES_PHONE_AREA_LEN             6
#define PAL_MYPLACES_PHONE_NUMBER_LEN          12

#define PAL_MYPLACES_MAX_FREE_FORM_STRLEN     400
#define PAL_MYPLACES_MAX_POSTAL_STRLEN         10
#define PAL_MYPLACES_MAX_COUNTRY_STRLEN         3
#define PAL_MYPLACES_MAX_AIRPORT_STRLEN         4

#define PAL_MYPLACES_URL_STRLEN               200

typedef enum
{
    PAL_ST_ByNone = 0,
    PAL_ST_ByName,
    PAL_ST_ByLastUsed,
    PAL_ST_ByLastUpdatedTime,
    PAL_ST_ByDistance,
    PAL_ST_ByRelevance
}PAL_SortType;

typedef enum
{
   PAL_MPT_None = 0,
   PAL_MPT_Recent,
   PAL_MPT_Favorite,
   PAL_MPT_Contact,
   PAL_MPT_MyPlacesMax
} PAL_MyPlacesType;

typedef struct
{
    char code[PAL_MYPLACES_CATEGORY_CODE_STRLEN+1];
    char name[PAL_MYPLACES_CATEGORY_NAME_STRLEN+1];
} PAL_MyPlacesCategory;

typedef enum
{
    PAL_MPPT_Primary = 0,
    PAL_MPPT_Secondary,
    PAL_MPPT_National,
    PAL_MPPT_Mobile,
    PAL_MPPT_Fax
} PAL_MyPlacesPhoneType;

/*! This type describes a phone contact */
typedef struct
{
    PAL_MyPlacesPhoneType type;                                         /*!< Kind of phone contact */
    char                  country[PAL_MYPLACES_PHONE_COUNTRY_LEN+1];    /*!< Country code */
    char                  area[PAL_MYPLACES_PHONE_AREA_LEN+1];          /*!< Area code */
    char                  number[PAL_MYPLACES_PHONE_NUMBER_LEN+1];      /*!< Actual phone number */
} PAL_MyPlacesPhone;

typedef struct
{
    char    areaname[PAL_MYPLACES_STRLEN+1];
    char    streetnum[PAL_MYPLACES_STRLEN+1];
    char    street1[PAL_MYPLACES_STRLEN+1];
    char    street2[PAL_MYPLACES_STRLEN+1];
    char    city[PAL_MYPLACES_STRLEN+1];
    char    county[PAL_MYPLACES_STRLEN+1];
    char    state[PAL_MYPLACES_STRLEN+1];
    char    postal[PAL_MYPLACES_MAX_POSTAL_STRLEN+1];
    char    country[PAL_MYPLACES_MAX_COUNTRY_STRLEN+1];         /*!< ISO country code, not country name. */
    char    airport[PAL_MYPLACES_MAX_AIRPORT_STRLEN+1];
    char    freeform[PAL_MYPLACES_MAX_FREE_FORM_STRLEN+1];
    double  latitude;
    double  longitude;
} PAL_MyPlacesLocation;

typedef struct
{
    char                 name[PAL_MYPLACES_STRLEN+1];
    PAL_MyPlacesLocation location;
    uint32               myPlaceID;
    PAL_MyPlacesType     type;
    PAL_MyPlacesCategory category;
    PAL_MyPlacesPhone    phone;
    char                 url[PAL_MYPLACES_URL_STRLEN+1];
    char                 imageUrl[PAL_MYPLACES_URL_STRLEN+1];
    void*                data;
} PAL_MyPlace;

#endif
/*! @{ */
