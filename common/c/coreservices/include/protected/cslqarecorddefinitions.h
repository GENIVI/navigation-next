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

@file cslqarecorddefinitions.h
@defgroup cslqalog QA Logging
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

#ifndef CSLQARECORDDEFINITIONS_H
#define CSLQARECORDDEFINITIONS_H

#define RECORD_ID_NONE                      0xFFFF
#define RECORD_ID_FILE_HEADER               0
#define RECORD_ID_FILE_HASH                 1
#define RECORD_ID_GPS_CONFIG                2
#define RECORD_ID_GPS_FIX                   3
#define RECORD_ID_NAV_STATE                 4
#define RECORD_ID_NAV_POS                   5
#define RECORD_ID_NAV_STARTUP               6
#define RECORD_ID_POI_SEARCH                7
#define RECORD_ID_MAP_DOWNLOAD              8
#define RECORD_ID_ROUTE_ID                  9
#define RECORD_ID_NAV_ERROR                 10
#define RECORD_ID_NET_ERROR                 11
#define RECORD_ID_FATAL_ERROR               12
#define RECORD_ID_PLACE                     13
#define RECORD_ID_APP_STATE                 14
#define RECORD_ID_TAPI_STATUS               15
#define RECORD_ID_SEGMATCH                  16
#define RECORD_ID_DNS_RESULT                17
#define RECORD_ID_OFFROUTE                  18
#define RECORD_ID_QCP_BASE                  19
#define RECORD_ID_QCP_PRONUN                20
#define RECORD_ID_QCP_CACHE_LOOKUP          21
#define RECORD_ID_MEDIA_ERROR               22
#define RECORD_ID_KEY                       23
#define RECORD_ID_SEGMATCH2                 24
#define RECORD_ID_PLACE2                    25
#define RECORD_ID_NAV_STARTUP2              26
#define RECORD_ID_PLACE3                    27
#define RECORD_ID_PLACE4                    28
#define RECORD_ID_VOICECACHE                29
#define RECORD_ID_GPS_CONFIG2               30
#define RECORD_ID_GPS_FIX_REQUEST           31
#define RECORD_ID_APP_EXIT                  32
#define RECORD_ID_UPLOAD_FILE               33
#define RECORD_ID_VOICECACHE2               34
#define RECORD_ID_GPS_FIX_FILTERED          35
#define RECORD_ID_GPS_FILTER_STATE          36
#define RECORD_ID_EVENT                     37
#define RECORD_ID_GPS_FIX_ORIGIN            38
#define RECORD_ID_NET_EVENT                 39
#define RECORD_ID_NAV_POS2                  40
// Begin J2ME Only
#define RECORD_ID_GPS_BEGIN_TRACK           41
#define RECORD_ID_GPS_END_TRACK             42
#define RECORD_ID_GPS_ERROR                 43
// End J2ME Only
#define RECORD_ID_QCP_CACHE_LOOKUP2         44
#define RECORD_ID_SYNC_STATE                45
#define RECORD_ID_NOTIFY_STATE              46
#define RECORD_ID_TILE_REQUEST              47
#define RECORD_ID_TILE_RECEIVED             48
#define RECORD_ID_TILE_STATE                49
#define RECORD_ID_SMS                       50
#define RECORD_ID_TRAFFIC_INCIDENT          51
#define RECORD_ID_TRAFFIC_FLOW              52
#define RECORD_ID_TRAFFIC_SESSION           53
#define RECORD_ID_CONGESTION_WARNING        54
#define RECORD_ID_INCIDENT_WARNING          55
#define RECORD_ID_TRAFFIC_UPDATE            56
#define RECORD_ID_VECTOR_MAP_PROFILE        57
#define RECORD_ID_LOG_FUNCTION              58
#define RECORD_ID_DEV_GPSINIT               59
#define RECORD_ID_DEV_GPSFIXREQUEST         60
#define RECORD_ID_DEV_GPSGETCONFIG          61
#define RECORD_ID_DEV_GPSSETCONFIG          62
#define RECORD_ID_DEV_GPSPOSITION           63
#define RECORD_ID_DEV_GPSDEVICESTATE        64
#define RECORD_ID_PREFETCH_AREA             65
#define RECORD_ID_TILE_DISCARDED            66
#define RECORD_ID_PREFETCH_POINT            67

#define RECORD_ID_NET_QUERY                 69
#define RECORD_ID_NET_REPLY                 70
#define RECORD_ID_NAV_STATE2                71

#define RECORD_ID_NAV_STATE3                77
#define RECORD_ID_ANALYTICS_EVENT           78
#define RECORD_ID_ANALYTICS_CONFIG          79

#define RECORD_ID_MAPTILE_SOURCE_REQUEST    80
#define RECORD_ID_MAPTILE_SOURCE_REPLY      81
#define RECORD_ID_MAPTILE_CENTER_CHANGED    82
#define RECORD_ID_MAPTILE_ZOOM_CHANGED      83
#define RECORD_ID_MAPTILE_REQUEST           84
#define RECORD_ID_MAPTILE_RECEIVED          85
#define RECORD_ID_MAPTILE_CACHE_STATE       86
#define RECORD_ID_MAPTILE_EVENT             87
#define RECORD_ID_NETWORK_QUERY_EXTENDED    88
#define RECORD_ID_NETWORK_REPLY_EXTENDED    89
#define RECORD_ID_NETWORK_ERROR_EXTENDED    90
#define RECORD_ID_NETWORK_ERROR_EXTENDED2   91
#define RECORD_ID_ROUTE_QUERY               92
#define RECORD_ID_ROUTE_REPLY               93
#define RECORD_ID_NAV_SESSION_START         94
#define RECORD_ID_NAV_SESSION_ROUTE         95
#define RECORD_ID_NAV_SESSION_END           96
#define RECORD_ID_NAV_SESSION_RECALC        97
#define RECORD_ID_MAPTILE_SCREENSTATE       98
#define RECORD_ID_MAPTILE_TILESTATE         99
#define RECORD_ID_ASR_START_RECORDING       100
#define RECORD_ID_ASR_STOP_RECORDING        101
#define RECORD_ID_ASR_REQUEST               102
#define RECORD_ID_ASR_RECOGNIZED_REPLY      103
#define RECORD_ID_ASR_AMBIGUOUS_REPLY       104
#define RECORD_ID_SEARCH_ALONG_ROUTE        105
#define RECORD_ID_NAV_STARTUP_REGION        106
#define RECORD_ID_MAP_TILE_PERFORMANCE_EVENT 107
#define RECORD_ID_PRONUN_FILE_DOWNLOAD      108
#define RECORD_ID_GUIDANCE_MESSAGE_TRIGGERED 109
#define RECORD_ID_GUIDANCE_MESSAGE_RETRIEVED 110
#define RECORD_ID_GUIDANCE_MESSAGE_PLAYED   111
#define RECORD_ID_CELLID_REQUEST            112
#define RECORD_ID_CELLID_WIFI_REQUEST       113
#define RECORD_ID_CELLID_REPLY              114

#define RECORD_ID_NBI_API_CALL              120
#define RECORD_ID_NBI_LOCATION_EVENT        121
#define RECORD_ID_NBI_WIFI_PROBES_EVENT     124
#define RECORD_ID_NBI_NETWORK_LOCATION_REQUEST 122
#define RECORD_ID_NBI_NETWORK_LOCATION_REPLY 123

#define RECORD_ID_AUTHENTICATION_REQUEST    130
#define RECORD_ID_AUTHENTICATION_REPLY      131
#define RECORD_ID_LICENSE_REQUEST           132
#define RECORD_ID_LICENSE_REPLY             133
#define RECORD_ID_CAMERA                    134
#define RECORD_ID_CAMERA_WARNING            135
#define RECORD_ID_TRAFFIC_STATE             136
#define RECORD_ID_TRAFFIC_REGION            137
#define RECORD_ID_TRAFFIC_FLOW2             138
#define RECORD_ID_PROFILE_SETTING           139
#define RECORD_ID_TRAFFIC_EVENT             140
#define RECORD_ID_APP2APP_COMMAND           141
#define RECORD_ID_CLIENT_SESSION_ID         142

#define RECORD_ID_METADATA_SOURCE_REQUEST   144
#define RECORD_ID_WANTED_CONTENT            145
#define RECORD_ID_METADATA_SOURCE_RESPONSE  146
#define RECORD_ID_METADATA_SOURCE_TEMPLATE  147
#define RECORD_ID_MANIFEST_REQUEST          148
#define RECORD_ID_MANIFEST_RESPONSE         149
#define RECORD_ID_MANIFEST_RESPONSE_CONTENT 150
#define RECORD_ID_SAR_INFO                  151
#define RECORD_ID_SAR_STATE                 152
#define RECORD_ID_EC_INFO                   153
#define RECORD_ID_EC_DOWNLOAD2              158
#define RECORD_ID_EC_DOWNLOAD_RESULT2       159
#define RECORD_ID_EC_STATE                  165

#define RECORD_ID_SPEECH_STREAM_REQUEST     174
#define RECORD_ID_SPEECH_STREAM_REPLY       175
#define RECORD_ID_ONE_SHOT_ASR_RESULT       176
#define RECORD_ID_UNIFIED_MAP_CONFIG_INITIALIZATION_FAILURE 177
#define RECORD_ID_PREFETCH_WINDOW           178
#define RECORD_ID_TILE_DOWNLOAD_SUCCESS     179
#define RECORD_ID_TILE_DOWNLOAD_FAILURE     180
#define RECORD_ID_ONE_SHOT_ASR_EVENT        182
#define RECORD_ID_PERFORMANCE               183
#define RECORD_ID_KPI_SESSION_OPEN          184
#define RECORD_ID_KPI_SESSION_CLOSE         185
#define RECORD_ID_KPI_FREEFORM_EVENT        186
#define RECORD_ID_SPEED_LIMIT_STATE         188
#define RECORD_ID_SPEED_LIMIT_INFO          189
#define RECORD_ID_NAV_STATE4                190

#define RECORD_ID_DNS_IPV6_RESULT           191

#define RECORD_ID_DATA_ROAD_INFO            200
#define RECORD_ID_LANE_INFO                 201
#define RECORD_ID_LANE_ITEM_INFO            202
#define RECORD_ID_GUIDANCE_POINT            203

#define RECORD_ID_GUIDANCE_MESSAGE_RETRIEVED2 205
#define RECORD_ID_GUIDANCE_AUDIO_PLAYTIME  206
#define RECORD_ID_GUIDANCE_TIMING_DATA     207
#define RECORD_ID_GUIDANCE_MESSAGE_PLAYED2 208

#define RECORD_ID_LANE_INFO2                209
#define RECORD_ID_GUIDANCE_POINT2           210
#define RECORD_ID_LAYER_AVAILABLITY_MATRIX_CHECK     217
#define RECORD_ID_PLACE5                    218
#define RECORD_ID_PLACE6                    219
#define RECORD_ID_DAM_CHECK                 220
#define RECORD_ID_PLACE7                    221


#define FILE_HEADER_MAX_LEN                 20
#define FILE_HEADER_PRODUCT_NAME_LEN        10
#define FILE_HEADER_MDN_LEN                 20
#define FILE_HEADER_MIN_LEN                 20

#define FILE_HASH_MAX_LEN                   80
#define FILE_HASH_FILENAME_LEN              80
#define FILE_HASH_SHA1_LEN                  20

#define NAV_STATE_MAX_LEN                   50
#define NAV_STATE_STREET_LEN                50
#define NAV_STATE_CODE_LEN                  8

#define NAV_POS_MAX_LEN                     10
#define NAV_POS_ACTION_LEN                  10

#define NAV_STARTUP_MAX_LEN                 50
#define NAV_STARTUP_STREET_LEN              50

#define PLACE_LABEL_LEN                     16

#define APP_STATE_LEN                       32
#define HOST_NAME_LEN                       128

#define QCP_NAME_LEN                        32
#define QCP_ID_LEN                          32

#define MESSAGE_FROM_NAME_MAX_LEN           50

#define SMS_LOG_LEN                         200
#define TMC_LOC_LEN                         10

#define RASTER_TILE_URL_LENGTH              200
#define RASTER_TILE_FUNCTION_NAME_LENGTH    100

#define MAX_FILE_NAME_LEN                   64

#define SEARCH_NAME_LEN                     32
#define SEARCH_SCHEME_LEN                   32
#define SEARCH_MAX_CATEGORIES               32

#define SEARCH_CATEGORY_CODE_LENGTH         11

#define PRONUN_CODE_LENGTH                  32
#define QALOG_MAX_PRONUN_CODES              20

#define ASR_FIELD_LENGTH                    25
#define ASR_SESSION_LENGTH                  15
#define ASR_ENCODING_LENGTH                 5
#define ASR_RESULT_LENGTH                   50

#define LICENCE_REQUEST_ID_LENGTH           15
#define LICENSE_ACTION_LENGTH               10
#define LICENSE_KEY_LENGTH                  20
#define LICENSE_VENDOR_LENGTH               10
#define LICENCE_LOCALE_LENGTH               6
#define LICENSE_DATA_LENGTH                 8

#define AUTHENTICATION_PARAMETER_LENGTH     20
#define AUTHENTICATION_LOCALE_LENGTH        6

#define AUTHENTICATION_BUNDLE_NAME_LENGTH    8
#define AUTHENTICATION_BUNDLE_REGIONS_LENGTH 80
#define AUTHENTICATION_FEATURE_CODES_LENGTH  100
#define AUTHENTICATION_PRICE_OPTIONS_LENGTH  20

#define PROFILE_KEY_LENGTH                   25
#define PROFILE_VALUE_LENGTH                 40

#define ERROR_MESSAGE_LENGTH                 80

#define APP2APP_COMMAND_LENGTH               512

#define NB_MAP_LAYERID_LENGTH                10


#define GETTING_TPS_FAILED                   0
#define GETTING_MATERIAL_FAILED              1
#define PERFORMANCE_MSG_LENGTH               120

#define ONE_SHOT_ASR_SESSION_ID_LENGTH       36
#define ONE_SHOT_ASR_RESULT_TEXT_LENGTH      50

#if defined(AEE_SIMULATOR) || defined(_MSC_VER)
#define DBG_FUNCTION    __FUNCTION__
#else
#define DBG_FUNCTION    __func__
#endif


#endif

/*! @} */
