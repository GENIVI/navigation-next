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

    @file     nbpointsofinterestinformation.h
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

#ifndef POINTSOFINTERESTINFORMATION_H
#define POINTSOFINTERESTINFORMATION_H

#include "nbrouteinformation.h"
#include "nbsinglesearchinformation.h"
#include "nbsearchinformation.h"

/*!
@addtogroup nbpointsofinterestinformation
@{
*/

struct NB_Navigation;

/*! @struct NB_PointsOfInterestInformation
Information about the POIs on a route
*/
typedef struct NB_PointsOfInterestInformation NB_PointsOfInterestInformation;


/*! @struct NB_PointOfInterest
Information about a POI
*/
typedef struct
{
    NB_Place            place;                  /*!< place associated with the POI */
    NB_ExtendedPlace*   extendedPlace;          /*!< points to an allocated extended place information, which is destroyed by NB_PointsOfInterestInformationDestroy */
    double              routeDistanceRemaining; /*!< approximate route distance remaining at POI */
    
} NB_PointOfInterest;


/*! Create a POI information object

@param context NAVBuilder context instance
@param information On success, a newly created NB_PointsOfInterestInformation object; NULL otherwise.  A non-NULL object must be destroyed with NB_PointsOfInterestInformationDestroy
@returns NB_Error
*/
NB_DEC NB_Error NB_PointsOfInterestInformationCreate(NB_Context* context, NB_PointsOfInterestInformation** information);

/*! Update an existing POI information object with a previously returned search object

@param information A NB_PointsOfInterestInformation object
@param navigation Current navigation session (or NULL)
@param search The search object containing POI information
@returns NB_Error
*/
NB_DEC NB_Error NB_PointsOfInterestInformationUpdateWithSearch(NB_PointsOfInterestInformation* information, struct NB_Navigation* navigation, NB_SearchInformation* search);

/*! Update an existing POI information object with a previously returned search object
 
 @param information A NB_PointsOfInterestInformation object
 @param navigation Current navigation session (or NULL)
 @param search The search object containing POI information
 @returns NB_Error
 */
NB_DEC NB_Error NB_PointsOfInterestInformationUpdateWithSingleSearch(NB_PointsOfInterestInformation* information, struct NB_Navigation* navigation, NB_SingleSearchInformation* search);

/*! Get POI information

@param information A NB_PointsOfInterestInformation object
@param index Index of the POI place information
@param POI On success, receives the POI information
@returns NB_Error
*/
NB_DEC NB_Error NB_PointsOfInterestInformationGetPointOfInterest(NB_PointsOfInterestInformation* information, uint32 index, NB_PointOfInterest* poi);

/*! Get number of POIs

@param information A NB_PointsOfInterestInformation object
@param num the numbers of POI for a particular route.
@returns NB_Error
*/
NB_DEC NB_Error NB_PointsOfInterestInformationGetPointOfInterestCount(NB_PointsOfInterestInformation* information, int* count);

/*! Destroy a previously created POI information object

@param information A NB_PointsOfInterestInformation object created with NB_RouteHandlerGetPointOfInterestInformation()
@returns NB_Error
*/
NB_DEC NB_Error NB_PointsOfInterestInformationDestroy(NB_PointsOfInterestInformation* information);

/*! @} */

#endif
