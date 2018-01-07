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

    @file     nbdoxygengroups.h
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

/*!

@defgroup nbgeocode Geocode
Functions necessary to geocode, or get the latitude and longitude of, addresses
or places

@defgroup nbgeocodehandler Geocode Handler
@ingroup  nbgeocode
The GeocodeHandler object sends a geocode request to a server, receives the reply
and provides a way to retrieve the results

@defgroup nbgeocodeinformation Geocode Information
@ingroup  nbgeocode
The GeocodeInformation object is the result of a geocode reqeuest

@defgroup nbgeocodeparameters Geocode Parameters
@ingroup  nbgeocode
The GeocodeParameters object defines the information to be geocoded

@defgroup nbreversegeocode Reverse Geocode
Functions necessary to reverse geocode, or convert a latitude and longitude into,
an address or place

@defgroup nbreversegeocodehandler Reverse Geocode Handler
@ingroup  nbreversegeocode
The ReverseGeocodeHandler object sends a reverse geocode request to a server, receives the reply
and provides a way to retrieve the results

@defgroup nbreversegeocodeinformation Reverse Geocode Information
@ingroup  nbreversegeocode
The ReverseGeocodeInformation object is the result of a reverse geocode reqeuest

@defgroup nbreversegeocodeparameters Reverse Geocode Parameters
@ingroup  nbreversegeocode
The ReverseGeocodeParameters object defines how a latitude and longitude are to be reverse geocoded

@defgroup nbsearch Search
Functions necessary to search for Businesses, Places, Events and Venues

@defgroup nbsearchhandler Seach Handler
@ingroup nbsearch
The SearchHandler object sends a search request to a server, receives the reply
and provides a way to retrieve the results

@defgroup nbsearchinformation Search Information
@ingroup nbsearch
The SearchInformation object is the results of a seach

@defgroup nbsearchparameters Search Parameters
@ingroup nbsearch
The SearchParameters object defines what to search for

@defgroup nbrastermap Raster Map
Functions necessary to generate and retrieve raster maps and overlays

@defgroup nbrastermapparameters Raster Map Parameters
@ingroup nbrastermap
Describes the input parameters to retrieve a raster map

@defgroup nbrastermaphandler Raster Map Handler
@ingroup nbrastermap
The RasterMapHandler object sends a raster map request to a server, receives the reply
and provides a way to retrieve the map

@defgroup nbrastermapinformation Raster Map Information
@ingroup nbrastermap
The RasterMapInformation object is the result of a raster map request

@defgroup nbrastermapinformation Raster Map Information
@ingroup nbrastermap
The RasterMapInformation object is the result of a raster map request

@defgroup nbvectortile Vector Tile
Functions to generate and retrieve vector tiles

@defgroup nbvectortilehandler Vector Tile Handler
@ingroup nbvectortile
The VectorTileHandler object sends a vector tile request to a server, receives the reply
and provides a way to retrieve the tiles

@defgroup nbvectortileparameters Vector Tile Parameters
@ingroup nbvectortile
Describes the input parameters to retrieve a vector tile

@defgroup nbvectortileinformation Vector Tile Information
@ingroup nbvectortile
The VectorTileInformation object is the result of a vector tile request

@defgroup nbvectortilemanager Vector Tile Manager
@ingroup nbvectortile
The VectorTileManager object retrieves and caches requested vector tiles

@defgroup nbrastertile Raster Tile
Functions to generate and retrieve raster tiles

@defgroup nbrastertilemanager Raster Tile Manager
@ingroup nbrastertile
The RasterTileManager object retrieves and caches requested raster tiles

@defgroup nbrastertiledatasource Raster Tile Data Source
@ingroup nbrastertile
The RasterTileManagerDataSource object describes the data source for raster tile requests

@defgroup nbnavigation Navigation
Routing and navigation API

@defgroup nbcamera Camera
@ingroup nbnavigation
Functions necessary to interact with cameras on route

@defgroup nbcamerainformation Camera Information
@ingroup nbcamera
The CameraInformation object is the result of a route request

@defgroup nbcameraprocessor Camera Processor
@ingroup nbcamera
The CameraState Processor updates the CameraState object

@defgroup nbgpshistory GPS History
@ingroup nbnavigation
Functions used to create, destroy, and interact with a GPS History object

@defgroup nbnavigationstate Navigation State
@ingroup nbnavigation
Functions used provide information about navigation states

@defgroup nbguidance Guidance
@ingroup nbnavigation
Functions necessary to create, destroy, and interact with route objects

@defgroup nbguidanceprocessor Guidance Processor
@ingroup nbguidance
Functions used to create, destroy, and interact with a Guidance Information object

@defgroup nbguidanceinformation Guidance Information
@ingroup nbguidance
The Guidance Processor updates the GuidanceState object

@defgroup nbguidancemessage Guidance Message
@ingroup nbguidance
The Guidance Processor updates the GuidanceState object

@defgroup nbroute Route
@ingroup nbnavigation
Functions necessary to create, destroy, and interact with route objects

@defgroup nbroutehandler Route Handler
@ingroup nbroute
The RouteHandler object sends a route map request to a server, receives the reply
and provides a way to retrieve the route, traffic, and camera information

@defgroup nbrouteparameters Route Parameters
@ingroup nbroute
Describes the input parameters to retrieve a route

@defgroup nbrouteinformation Route Information Object
@ingroup nbroute
Functions used to create, destroy, and interact with a Guidance Information object

@defgroup nbdetourparameters Detour Parameter
@ingroup nbroute
Functions necessary to create detour parameters used to create route

@defgroup nbtraffic Traffic
@ingroup nbnavigation
Functions necessary to create, destroy, and interact with traffic objects

@defgroup nbtraffichandler Traffic Handler
@ingroup nbtraffic
The trafficHandler object sends a route map request to a server, receives the reply
and provides a way to retrieve the traffic notify information

@defgroup nbtrafficinformation Traffic Information
@ingroup nbtraffic
Functions used to destroy and interact with a Traffic Information object

@defgroup nbtrafficparameters Traffic Parameters
@ingroup nbtraffic
Describes the input parameters to retrieve a traffic notify information

@defgroup nbtrafficprocessor Traffic Processor
@ingroup nbtraffic
The TrafficState Processor updates the TrafficState object

@defgroup nbpointsofinterestinformation POI Information
@ingroup nbnavigation
This interface provides access to POIs prefetched during navigation

@defgroup nbcontext Context
Functions necessary to create, destroy and interact with NAVBuilder context

@defgroup nbspatial Spatial
This API contains conversion functions to convert coordinates from/to mercator projection.

@defgroup nbtristrip TriStrip
This API contains utility functions for TriStrip objects.

@defgroup nbversion Version
Provides version and copyright information about the component.

@defgroup nimlocation Location Types
Provides the information about the Location and Places

@defgroup nbiterationcommand Iteration Command
Provides the information about the Iteration Command for seacrh and geocode

@defgroup nbnetwork Network
Functions related to Network

@defgroup nbnetwork Network
@ingroup  nbnetwork
This interface provides the ability to establish a connection to the NIM server for use by other NAVBuilder APIs.

@defgroup nbnetconfig NetworkConfig
@ingroup  nbnetwork
This interface defines the configuration to create a NB network object

@defgroup nbgps GPS
Platform-independent GPS location API

@defgroup gpscriteriaset GPS Criteria
@ingroup  nbgps
Functions defining the methods related with GPS Criteria

@defgroup gpshist GPS History
@ingroup  nbgps
This interface defines the methods related with GPS history

@defgroup gputil GPS Utility
@ingroup  nbgps
This interface defines the methods for GPS Utilities

@defgroup nbgpsfilter GPS Filter
@ingroup nbgps
This interface defines the methods for GPS filtering

@defgroup nbanalytics Analytics
This interface defines the methods for Analytics

@defgroup nbqalog QA Log
This interface defines the methods for QA Logging

@defgroup nblocation Location
Functions necessary to query location based on cell ID.

@defgroup nblocationhandler Location Handler
@ingroup  nblocation
The Location Handler object sends a request to a server, receives the reply

@defgroup nblocationinformation Location Information
@ingroup  nblocation
The Location Information object is a result of a Location request

@defgroup nblocationparameters Location Parameters
@ingroup  nblocation
The Location Parameters object defines the request information for Location Handler

@defgroup nbmobilecoupons Mobile Coupons
Functions necessary to operate with Mobile Coupons.

@defgroup nbstorerequesthandler Store Request Handler
@ingroup  nbmobilecoupons
The Store Request Handler object sends a request to a server, receives the reply.

@defgroup nbstorerequestinformation Store Request Information
@ingroup  nbmobilecoupons
The Store Request Information object is a result of a Store Request request.

@defgroup nbstorerequestparameters Store Request Parameters
@ingroup  nbmobilecoupons
The Store Request Parameters object defines the request information for Store Request Handler.

@defgroup nbcouponrequesthandler Coupon Request Handler
@ingroup  nbmobilecoupons
The Coupon Request Handler object sends a request to a server, receives the reply.

@defgroup nbcouponrequestinformation Coupon Request Information
@ingroup  nbmobilecoupons
The Coupon Request Information object is a result of a Coupon request.

@defgroup nbcouponrequestparameters Coupon Request Parameters
@ingroup  nbmobilecoupons
The Coupon Request Parameters object defines the request information for Coupon Request Handler.

@defgroup nbcouponloadimagehandler Load Image Handler
@ingroup  nbmobilecoupons
The Load Image Handler object sends a request to a server, receives the reply.

@defgroup nbcouponloadimageinformation Load Image Information
@ingroup  nbmobilecoupons
The Load Image Information object is a result of a Load Image request.

@defgroup nbcouponloadimageparameters Load Image Parameters
@ingroup  nbmobilecoupons
The Load Image Parameters object defines the request information for Load Image Handler.

@defgroup nbcategorylisthandler Category List Handler
@ingroup  nbmobilecoupons
The Category List Handler object sends a request to a server, receives the reply.

@defgroup nbcategorylistinformation Category List Information
@ingroup  nbmobilecoupons
The Category List Information object is a result of a Category List request.

@defgroup nbcategorylistparameters Category List Parameters
@ingroup  nbmobilecoupons
The Category List Parameters object defines the request information for Category List Handler.

@defgroup nbsearchfilter Search Filter functions
@ingroup  nbmobilecoupons
Search Filter functions are the utility function to operate with search filter data structure.

@defgroup nbsolarcalculator Calculate time of sunrise and sunset for the date and location
Functions to calculate and represent time of sunrise and sunset for the date and location

*/
