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

@file nberror.h
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


#ifndef NBERROR_H
#define	NBERROR_H

/*
 * WARNING! Resource strings assume the order/values of these errors
 * DO NOT CHANGE without updating the resources.
 * These are now exposed as error codes within the error messages,
 * so new values can be added, but the existing values should not be
 * removed or reordered.
 */
typedef enum {
	NE_OK = 0				/* 0 No error */
	,NE_NOMEM				/* 1 Out of memory */
	,NE_BADDATA				/* 2 Received bad data */
	,NE_AGAIN				/* 3 Try again (should be trapped) */
	,NE_NOENT				/* 4 No such entity */
	,NE_INVAL				/* 5 Invalid argument */
	,NE_PERM				/* 6 Insufficient privileges */
	,NE_NET					/* 7 Network error */
	,NE_SOCK				/* 8 Socket error */
	,NE_NETSVC				/* 9 Network service error */
	,NE_INST				/* 10 Failed to instantiate class */
	,NE_EXIST				/* 11 Entity exists */
	,NE_FSYS				/* 12 File system operation error */
	,NE_RANGE				/* 13 Value out of range */
	,NE_RES					/* 14 Resource Error */
	,NE_GPS					/* 15 GPS Error */
	,NE_API					/* 16 API Error */
	,NE_NOTINIT				/* 17 Not properly initialized */
	,NE_NOSUPPORT			/* 18 This API is not currently supported */
	,NE_BUSY				/* 19 The subsystem is busy */
	,NE_UNEXPECTED			/* 20 This call is unexpected at this time */
	,NE_INTERRUPTED			/* 21 The operation was interrupted */
	,NE_GPS_PRIVACY			/* 22 GPS Privacy Refused */
	,NE_CORRUPTED			/* 23 Corrupt data detected. Records removed. */
	,NE_DECLINED			/* 24 The operation failed because the user declined permision. */
	,NE_CANCELLED			/* 25 The operation was cancelled. */
	,NE_GPS_TIMEOUT			/* 26 The GPS operation timed out. */
	,NE_GPS_NO_INFO			/* 27 The GPS operation did not return location information */
	,NE_GPS_BUSY			/* 28 The GPS operation failed because the receiver was busy */
	,NE_GPS_LOCATION_OFF	/* 29 The GPS operation failed because handset location is disabled. */
	,NE_DNS_LOOKUP_ERROR	/* 30 DNS lookup error */
    ,NE_TASK_QUEUE_FAILED   /* 31 Task queue operation failed */
	,NE_IGNORED             /* 32 Some task or function is ignored. */
    ,NE_NO_TRAFFIC_ROUTE_INFO/* 33 Server response when there is no traffic/route information for this tile. */

	,NEGPS_BASE = 1000
	,NEGPS_GENERAL
	,NEGPS_TIMEOUT
	,NEGPS_ACCURACY_UNAVAIL
	,NEGPS_INFO_UNAVAIL
	,NEGPS_PRIVACY_REFUSED
	,NEGPS_RESERVED1
	,NEGPS_RESERVED2
	,NEGPS_RESERVED3
	,NEGPS_RESERVED4
	,NEGPS_STALE_BS
	,NEGPS_RECEIVER_BUSY
    ,NEGPS_REJECTED
    ,NEGPS_TIMEOUT_START_NAV
	,NEGPS_UNKNOWN = 1099

	,NESERVER_UNKNOWN = 2000
	,NESERVER_FAILED
	,NESERVER_TIMEDOUT
	,NESERVER_UNKNOWNLOCALERROR
	,NESERVER_CONNECTIONNOTFOUND
	,NESERVER_UNSUPPORTEDLOCATION = 2009

	,NEROUTE_NONE = 3000
	,NEROUTE_TIMEOUT
	,NEROUTE_BADDEST
	,NEROUTE_BADORG
	,NEROUTE_CANNOTROUTE
	,NEROUTE_EMPTYROUTE
	,NEROUTE_NETERROR
	,NEROUTE_UNKNOWN
	,NEROUTE_NOMATCH
	,NEROUTE_SERVERERROR
	,NEROUTE_NODETOUR
	,NEROUTE_PEDROUTETOOLONG
	,NEROUTE_UNSUPPORTED_ORIGIN_COUNTRY = 3013
	,NEROUTE_UNSUPPORTED_DESTINATION_COUNTRY = 3014

	/* Extended Server Error Codes */
	,NESERVERX_LIBRARY_ERROR			= 4000
	,NESERVERX_PROTOCOL_ERROR			= 4001
	,NESERVERX_DOCUMENT_ERROR			= 4002
	,NESERVERX_INTERNAL_ERROR			= 4003
	,NESERVERX_TPS_CHAR_ENCODING_ERROR  = 4004
	,NESERVERX_UNSUPPORTED_ERROR		= 4010
	,NESERVERX_UNSUPPORTED_FILESET		= 4011
	,NESERVERX_UNSUPPORTED_PRONUN_STYLE	= 4012
	,NESERVERX_UNSUPPORTED_NAV_OPTIMIZE	= 4013
	,NESERVERX_UNSUPPORTED_NAV_VEHICLE	= 4014
	,NESERVERX_UNSUPPORTED_NAV_AVOID	= 4015

	,NESERVERX_PROCESSING_ERROR			= 4020
	,NESERVERX_SERVER_UNAVAILBLE_ERROR	= 4021
	,NESERVERX_EXTERNAL_PROCESSING_ERROR= 4022
	,NESERVERX_IMAGE_PROCESSING_ERROR	= 4023
    ,NESERVERX_DAILY_REQUEST_LIMIT_ERROR = 4026

	,NESERVERX_VALUE_ERROR				= 4030
	,NESERVERX_INVALID_COORD_ERROR		= 4031
	,NESERVERX_INVALID_IMAGE_ERROR		= 4032
	,NESERVERX_INVALID_MAP_COORD_ERROR	= 4033

    ,NESERVERX_INVALID_CREDENTIAL_ERROR	= 4040
    ,NESERVERX_DAILY_USER_LIMIT_ERROR   = 4046

    ,NESERVERX_UNAUTHORIZED_REGION      = 4277

    ,NESERVERX_UNAUTHORIZED_ACCESS      = 4299

    ,NE_FSYS_OPEN_ERROR                 = 6500
    ,NE_FSYS_PERMISSION_ERROR           = 6501
    ,NE_FSYS_WRITE_ERROR                = 6502
    ,NE_FSYS_NETWORK_READ_ERROR         = 6503
    ,NE_FSYS_UNZIP_READ_ERROR           = 6504
    ,NE_FSYS_CLOSE_ERROR                = 6505
    ,NE_FSYS_DELETE_ERROR               = 6506
    ,NE_FSYS_RENAME_ERROR               = 6507
    ,NE_FSYS_NOSPACE                    = 6508

    ,NESERVERX_FAILED_FACEBOOK          = 7001
    ,NESERVERX_INVALID_ACTIVATION       = 7002

    ,NESERVERX_EXTERNAL_CONNECTION_NOT_FOUND  = 8000  /* An error occurred trying to connect to the external search provider. */
    ,NESERVERX_EXTERNAL_REQUEST_FAILED        = 8001  /* An error response was returned from the external search provider. */
    ,NESERVERX_EXTERNAL_REQUEST_TIMEOUT       = 8002  /* A timeout occurred waiting on a response from the external search provider. */

    ,NE_ERROR_INVALID_LOCATION          = 9003    /*!< The requested center is not a valid coordinate pair */
    ,NE_ERROR_NO_LOCATION_AVAILABLE     = 9030    /*!< Location Kit failed to get a location from the server */

    /* HTTP error codes */
    ,NE_HTTP_NO_CONTENT = 10000         /*!< HTTP 204 - The server successfully processed the request,
                                             but is not returning any content. */
    ,NE_HTTP_NOT_MODIFIED               /*!< HTTP 304 - Indicates the resource has not been modified
                                             since last requested. */
    ,NE_HTTP_BAD_REQUEST                /*!< HTTP 400 - The request cannot be fulfilled due to bad syntax. */
    ,NE_HTTP_RESOURCE_NOT_FOUND         /*!< HTTP 404 - The requested resource could not be found but may
                                             be available again in the future. */
    ,NE_HTTP_INTERNAL_SERVER_ERROR      /*!< HTTP 500 - A generic server error message, given when no more
                                             specific message is suitable. */
    ,NE_HTTP_BAD_GATEWAY                /*!< HTTP 502 - The server was acting as a gateway or proxy and
                                             received an invalid response from the upstream server. */

} NB_Error;

#endif
