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

    @file     nbrastermapinformation.h
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

#ifndef NBRASTERMAPINFORMATION_H
#define NBRASTERMAPINFORMATION_H

#include "paltypes.h"
#include "nbrastermapparameters.h"


/*!
    @addtogroup nbrastermapinformation
    @{
*/


/*! @struct NB_RasterMapInformation
A RasterMap maintains information about a raster map
*/
typedef struct NB_RasterMapInformation NB_RasterMapInformation;


/*! Destroy a previously created RasterMap object

@param map A NB_RasterMapInformation object created with NB_RasterMapInformationHandlerGetMap()
@returns NB_Error
*/
NB_DEC NB_Error NB_RasterMapInformationDestroy(NB_RasterMapInformation* map);


/*! Get the latitude, longitude, scale and rotation of the map

Information about the map.  These values may differ slightly from those used to retrieve the map from
the server, but always exactly match the map data

@param map A NB_RasterMapInformation object
@param latlong On success, receives the latitude and longitude of the center of the map
@param scale On success, receives the scale of the map
@param rotation On success, receives the heading that corresponds to "up" on the map
@returns NB_Error
@see NB_RasterMapInformationCreateParameters
*/
NB_DEC NB_Error NB_RasterMapInformationGetMapInformation(NB_RasterMapInformation* map, NB_LatitudeLongitude* latlong, double* scale, double* rotation);


/*! Get the map data

@param map A NB_RasterMapInformation object
@param format On success, receives the format of the map data
@param data On success, receives a pointer to the map data.  The map data continues to be owned by the NB_RasterMapInformation object and will be freed when NB_RasterMapInformationDestroy is called
@param dataLength On success, receives the count, in bytes, of the map data
@returns NB_Error
*/
NB_DEC NB_Error NB_RasterMapInformationGetImageData(NB_RasterMapInformation* map, NB_ImageFormat* format, uint8** data, uint32* dataLength);


/*! Get the location at the center of the map

Get the location at the center of the map, if available.  This address is only available if the NB_RasterMapInformationParameters object
was created specifying that center be reverse geocoded.

@param map A NB_RasterMapInformation object
@param location On success, receives the address at the center of the map
@returns NB_Error
@see NB_RasterMapInformationCreateParameters
*/
NB_DEC NB_Error NB_RasterMapInformationGetCenterLocation(NB_RasterMapInformation* map, NB_Location* location);


/*! Translates a latitude and longitude to the corresponding coordinates within the rendered map
 
@param map A NB_RasterMapInformation object
@param rectangle A rectangle representing the map rendering area.  The size specified must match the size of the rendered map
@param latlong The latitude and longitude to convert to map coordinates
@param point On success, receives the coordinates of the point within the map rectangle
@return NB_Error
*/
NB_DEC NB_Error NB_RasterMapInformationTransformLLToMap(NB_RasterMapInformation* map, NB_Rectangle* rectangle, NB_LatitudeLongitude latlong, NB_Point* point);


/*! Translates a point within the map rectangle to the corresponding latitude and longitude. 
@param map A NB_RasterMapInformation object
@param rectangle A rectangle representing the map rendering area.  The size specified must match the size of the rendered map
@param point The point to convert into latitude and longitude
@param latlong On success, receives the latitude and longitude of the point within the map rectangle
@return NB_Error
*/
NB_DEC NB_Error NB_RasterMapInformationTransformMapToLL(NB_RasterMapInformation* map, NB_Rectangle* rectangle, NB_Point* point, NB_LatitudeLongitude* latlong);


/*! @} */

#endif
