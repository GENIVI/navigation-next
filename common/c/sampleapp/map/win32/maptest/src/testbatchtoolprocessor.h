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

    @file     testbatchtoolrocessor.h

    This file contains definition for TestBatchToolProcessor class.
*/
/*
    (C) Copyright 2012 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunications Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunications Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

#ifndef _TEST_BATCH_TOOL_PROCESSOR_H_
#define _TEST_BATCH_TOOL_PROCESSOR_H_

#include "testnavigation.h"

extern "C"
{
    #include "pal.h"
    #include "nbcontext.h"
    #include "nbgeocodehandler.h"
    #include "nbreversegeocodehandler.h"
    #include "nbsearchhandler.h"
    #include "nbpointsofinterestinformation.h"
    #include "nbrouteinformation.h"
    #include "nbroutehandler.h"
    #include "nbspatial.h"
    #include "nbtrafficparameters.h"
    #include "nbtraffichandler.h"
    #include "nbanalytics.h"
    #include "nbenhancedcontentmanager.h"
    #include "nbpointsofinterestinformation.h"
    #include "nbtrafficinformation.h"
    #include "nbcamerainformation.h"
    #include "navpublictypes.h"
    #include "nbnavigation.h"
    #include "nbtrafficprocessor.h"
    #include "nbdetourparameters.h"
}

#define BATCH_STRING_MAX_LEN 2048
#define BATCH_STRING_MAX_PARAMS 40

#define TEST_MAX_PATH_LENGTH                        256

#define TEST_SCREEN_WIDTH                           780
#define TEST_SCREEN_HEIGHT                          585

#define TEST_COUNTRY                                "USA"
#define TEST_NBGM_VERSION                           "1"

#define TEST_ECM_FILE_FORMAT                        "BIN"
#define TEST_MJO_FILE_FORMAT                        "BIN"
#define TEST_SAR_FILE_FORMAT                        "PNG"
#define TEST_TEXTURE_FILE_FORMAT                    "PNG"
#define TEST_COMPRESSED_TEXTURE_FILE_FORMAT         "zip"

// directories for test data
#define CITYMODEL_PATH                              "Citymodels"
#define MJO_PATH                                    "MJO"
#define SAR_PATH                                    "SAR"
#define TEXTURE_PATH                                "Textures"

// Parameters array from batch CSV file string.
//   buf - contain all parameters separated by '0'.
//   params - array of parameters start position in buf array.
//   outputFileName - file for results.
typedef struct {
    char                buf[BATCH_STRING_MAX_LEN+1];
    unsigned short int  params[BATCH_STRING_MAX_PARAMS];
    char                outputFileName[BATCH_STRING_MAX_LEN+1];
} BatchFileString;

typedef enum
{
    Route_CaseId = 0,
    Route_Type,
    Route_Mode,
    Route_Avoide,
    Route_Maneuvers,
    Route_Dest_Lat,
    Route_Dest_Lon,
    Route_Dest_Location,
    Route_Orig_Lat,
    Route_Orig_Lon,
    GPS_File,
    POI,
    Traffic,
    Enhanced,
    Camera
} BATCH_ROUTE_FILE_INDEX;

typedef enum
{
    PRINT_TYPE_DEFAULT = 0,
    PRINT_TYPE_ROUTE,
    PRINT_TYPE_ERROR
} PRINT_TYPE;

const char* GetDocumentsDirectoryPath();

class TestBatchToolGeocodeCallback;

class TestBatchToolProcessor
{
public:
    TestBatchToolProcessor(void* completeEvent, BatchFileString* batchParams);
    ~TestBatchToolProcessor(void);

    void Destroy(void);
    PAL_Instance* GetPal();
    bool IsInitialized();
    void ScheduleFinishQuery(void);
    void FinishQuery(void);
    void PrintPlaceToFile(NB_Place* place, double distance, char* fileName);

    // Public Tests
    void TestBatchToolGeocode(void);
    void TestBatchToolReverseGeocode(void);
    void TestBatchToolPOI(void);
    void TestBatchToolRoute(void);
    void TestBatchToolNavigationSession(void);

private:
    typedef void (TestBatchToolProcessor::*TestFunction)(void);

    void CallTestFunction(void);
    static void EventTaskCallback(PAL_Instance* pal, void* userData);

    // Tests - internal functions
    void BatchToolGeocode(void);
    void BatchToolReverseGeocode(void);
    void BatchToolPOI(void);
    void BatchToolRoute(void);
    void BatchToolNavigationSession(void);

    void UpdateGuidanceInformation();
    void PrintRouteToFile(NB_RouteInformation* route, char* str = NULL);
    void PrintNavigationToFile(int type, char* callbackName, char* buffer);
    void ParseCsvForRouteOptions(NB_RouteOptions* options, NB_RouteConfiguration* config);

    // Callbacks for tests
    static void Static_GeocodeCallback(void* handler, NB_NetworkRequestStatus status, NB_Error err, uint8 up, int percent, void* userData);
    void GeocodeCallback(NB_GeocodeHandler* handler, NB_NetworkRequestStatus status, NB_Error err, uint8 up, int percent);

    static void Static_ReverseGeocodeCallback(void* handler, NB_NetworkRequestStatus status, NB_Error err, uint8 up, int percent, void* userData);
    void ReverseGeocodeCallback(NB_ReverseGeocodeHandler* handler, NB_NetworkRequestStatus status, NB_Error err, uint8 up, int percent);

    static void Static_POISearchCallback(void* handler, NB_RequestStatus status, NB_Error err, uint8 up, int percent, void* userData);
    void POISearchCallback(NB_RequestStatus status, NB_Error err, uint8 up, int percent);

    static void Static_DownloadRouteCallback(void* handler, NB_NetworkRequestStatus status, NB_Error err, uint8 up, int percent, void* userData);
    void DownloadRouteCallback(void* handler, NB_NetworkRequestStatus status, NB_Error err, int percent);

    //for navigation
    static void Static_NavigationSessionStatusChange(NB_Navigation* navigation, NB_NavigateStatus status, void* userData);
    void NavigationSessionStatusChange();

    static void Static_NavigationCompleteRouteDownload(void* handler, NB_RequestStatus status, NB_Error err, nb_boolean up, int percent, void* userData);
    void NavigationCompleteRouteDownload(void* handler, NB_RequestStatus status, NB_Error err, nb_boolean up, int percent);

    static void Static_NavigationRouteDownload(void* handler, NB_RequestStatus status, NB_Error err, nb_boolean up, int percent, void* userData);
    void NavigationRouteDownload(void* handler, NB_RequestStatus status, NB_Error err, nb_boolean up, int percent);

    static void Static_NavigationPointsOfInterestNotifcationCallback(void* handler, NB_RequestStatus status, NB_Error err, nb_boolean up, int percent, void* userData);
    void NavigationPointsOfInterestNotifcationCallback(void* handler, NB_RequestStatus status, NB_Error err, nb_boolean up, int percent);

    static void Static_TrafficNotifcationCallback(void* handler, NB_RequestStatus status, NB_Error err, nb_boolean up, int percent, void* userData);
    void TrafficNotifcationCallback(void* handler, NB_RequestStatus status, NB_Error err, nb_boolean up, int percent);

    static void Static_CameraNotifcationCallback(void* handler, NB_RequestStatus status, NB_Error err, nb_boolean up, int percent, void* userData);
    void CameraNotifcationCallback(void* handler, NB_RequestStatus status, NB_Error err, nb_boolean up, int percent);

    static void Static_ManeuverPositionChangeCallback(NB_Navigation* navigation, uint32 maneuver, NB_NavigateManeuverPos maneuvPosition, void* userData);
    void ManeuverPositionChangeCallback(uint32 maneuver, NB_NavigateManeuverPos maneuvPosition);

    static void Static_EnhancedContentStateNotificationCallback(NB_Navigation* navigation, NB_EnhancedContentState* state, NB_EnhancedContentStateData available, void* userData);
    void EnhancedContentStateNotificationCallback(NB_EnhancedContentState* state, NB_EnhancedContentStateData available);

    static void Static_NavigationSessionGuidance(NB_Navigation* navigation, NB_GuidanceMessage* message, void* userData);
    void NavigationSessionGuidance(NB_GuidanceMessage* message);

    //Utility functions for navigation
    void SetupVectorMapPrefetchConfiguration(NB_VectorMapPrefetchConfiguration* config);
    void SetupNavigationConfiguration(NB_NavigationConfiguration* configuration);
    TestGPSFile* TestGPSFileLoad(const char* gpsFileName);
    NB_GpsLocation* TestGPSFileGetNextFix(int delayMilliseconds);
    void GPSInfo2GPSFix(GPSFileRecord* pos, NB_GpsLocation* cur);
    double UncDecode(uint8 unc);
    char* GetStatusString(NB_NavigateStatus status);
    void TestGPSFileDestroy(TestGPSFile* gps);
    char* GetManeuverPostionString(NB_NavigateManeuverPos position);
    void SetupEnhancedContentMapConfiguration(NB_EnhancedContentMapConfiguration* enhancedConfiguration);
    void CreateEnhancedContentManager(NB_RouteConfiguration* routeConfiguration);
    void SetupEnhancedContentManagerConfiguration(NB_EnhancedContentManagerConfiguration* config);

    // utility functions to log performance
    void PmPrintResultsReverseGeocode(NB_ReverseGeocodeInformation* information);
    void PmPrintInputParametersSearchBuilder(const char* poiname, NB_LatitudeLongitude * latlon, uint32 radius, const char* categories);
    void PmPrintResultsSearchBuilder(uint32 total, NB_Error error);
    void PmPrintInputParametersGeocode(const char* str);
    void PmPrintResultsGeocode(uint32 total, NB_Error error);
    void PmPrintInputParametersRoute(const char* str);
    void PmPrintResultsRoute( const char* err);
    void PmPrintInputParametersNavigationSession(const char* str, uint32 curFix);
    void PmPrintResultsNavigationSession( const char* callbackName, uint32 curFix, const char* err = NULL);

    // Helper functions
    bool DoGeocodeRequestWithParams(NB_GeocodeParameters* params);

private:
    PAL_Instance*               m_pal;
    NB_Context*                 m_context;
    NB_EnhancedContentManager*  m_ECManager;

    TestFunction                m_testFunction;

    void*                       m_completeEvent;
    bool                        m_initialized;
    bool                        m_getCoords;

    NB_GeocodeHandler*          m_GeocodeHandler;
    NB_ReverseGeocodeHandler*   m_ReverceGeocodeHandler;
    NB_SearchHandler*           m_searchHandler;

    NB_RouteHandler*            m_routeHandler;
    NB_GuidanceInformation*     m_guidanceInfo;

    bool                        m_navigationSession;
    TestGPSFile*                m_gpsSource;
    NB_GpsLocation*             m_gpsFix;
    NB_Navigation*              m_navigation;
    uint32                      m_statusCounter;
    PAL_File*                   m_fpOutFile;

    BatchFileString*            m_batchParams;

    NB_LatitudeLongitude       m_point;
};

#endif // _TEST_BATCH_TOOL_PROCESSOR_H_
/*! @} */
