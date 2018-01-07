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

/*
 * (C) Copyright 2012 by TeleCommunication Systems, Inc.
 *
 * The information contained herein is confidential, proprietary
 * to TeleCommunication Systems, Inc., and considered a trade secret
 * as defined in section 499C of the penal code of the State of
 * California. Use of this information by anyone other than
 * authorized employees of TeleCommunication Systems is granted only
 * under a written non-disclosure agreement, expressly prescribing
 * the scope and manner of such use.
 *
 */

/*===========================================================================

FILE: gpstest.c
===========================================================================*/


/*===============================================================================
INCLUDES AND VARIABLE DEFINITIONS
=============================================================================== */
#include "AEEModGen.h"          // Module interface definitions
#include "AEEAppGen.h"          // Applet interface definitions
#include "AEEShell.h"           // Shell interface definitions
#include "AEEStdLib.h"
#include "AEEMenu.h"
#include "AEEText.h"
#include "AEEPosDet.h"
#include "AEETelephone.h"
#include "AEEFile.h"
#include "abpalgps.h"
#include "nberror.h"

#include "abpal_gpstest.bid"
/*-------------------------------------------------------------------
Type Definitions
-------------------------------------------------------------------*/
#define GPSTEST_INFORECT_START_X    5
#define GPSTEST_INFORECT_START_Y   20
#define GPSTEST_LINE_HEIGHT        20
#define GPSTEST_INFORECT_HEIGHT   120
#define GPSTEST_EDIT_BOX_WIDTH    200

#define GPS_MAX_CONFIG 10
#define ABPAL_GPSTEST_LOG_FILE "abpal_gpstest_log.txt"

#define GPS_TRACKING_ID_1 0 //maybe this is a bug, but we can not use varios ID
#define GPS_TRACKING_ID_2 1

typedef enum _GpstestMenu
{
    MAIN_MENU_ID_GPSID_GET_LOCATION = 0,
    MAIN_MENU_ID_GPSID_TRACKING,
    MAIN_MENU_ID_GPSID_SUSPEND,
} GpstestMenu;

typedef enum _GpstestState
{
    GPSTEST_STATE_INIT = 0,
    GPSTEST_STATE_MENU,      
    GPSTEST_STATE_GET_LOCATION,    
    GPSTEST_STATE_TRACKING,
    GPSTEST_STATE_SUSPEND
} GpstestState;

/*-------------------------------------------------------------------
Applet structure. All variables in here are reference via "pMe->"
-------------------------------------------------------------------*/
// create an applet structure that's passed around. All variables in
// here will be able to be referenced as static.
typedef struct _gpstest {
    AEEApplet      a ;           // First element of this structure must be AEEApplet
    AEEDeviceInfo  DeviceInfo;
    IMenuCtl*      pIMenuCtl;
    GpstestState   state;

    PAL_Instance* pal;
    ABPAL_GpsContext* context;   
    ABPAL_GpsConfig gpsConfig[GPS_MAX_CONFIG];
    ABPAL_GpsCriteria criteria;
    int configCount;
    ABPAL_GpsLocation location;
    ABPAL_GpsState gpsState;
    boolean suspend;

    PAL_Error error;

} gpstest;

static PAL_Instance* 
PAL_CreateInstance(gpstest* pMe)
{
        PAL_Config* palConfig = (PAL_Config*)MALLOC(sizeof(PAL_Config));

        palConfig->shell = pMe->a.m_pIShell;

        return PAL_Create(palConfig);
}

/*-------------------------------------------------------------------
Function Prototypes
-------------------------------------------------------------------*/
static  boolean gpstest_HandleEvent(gpstest* pMe, AEEEvent eCode, uint16 wParam, uint32 dwParam);
boolean gpstest_InitAppData(gpstest* pMe);
void    gpstest_FreeAppData(gpstest* pMe);

void TestGpsCancelTests(gpstest* pMe);
void TestGpsIdGetLocation(gpstest* pMe);
void TestGpsIdTracking(gpstest* pMe);
void TestGpsIdSuspend(gpstest* pMe);

void DrawGPSInfo   (gpstest* pMe, boolean completeRedraw, boolean showCallbackStatus);
void DrawSectorInfo(gpstest* pMe, boolean completeRedraw);
void DrawSSInfo(gpstest* pMe, boolean completeRedraw);
void DrawHeading (gpstest* pMe, AECHAR* text);
void DrawFootnotes (gpstest* pMe, boolean retryAvailable);
void DrawPosDetError(gpstest* pMe);
void DrawTelError(gpstest* pMe);
void DrawText(gpstest* pMe, AECHAR* text, int y);
void DrawOther(gpstest* pMe);
void DrawSettings (gpstest* pMe);

void RetrieveGPSInfo(gpstest* pMe);
void RetrieveSectorInfo(gpstest* pMe);
void RetrieveSSInfo(gpstest* pMe);
void GPSCallback(void* pData);
void gpstest_Log(gpstest* pMe);
void clearTestLog(gpstest* pMe);
void gpsTestLog(gpstest* pMe, char* logBuffer);
/*-------------------------------------------------------------------
Constant Data
-------------------------------------------------------------------*/
/*===============================================================================
FUNCTION DEFINITIONS
=============================================================================== */

/*===========================================================================
FUNCTION: AEEClsCreateInstance

DESCRIPTION
    This function is invoked while the app is being loaded. All Modules must provide this 
    function. Ensure to retain the same name and parameters for this function.
    In here, the module must verify the ClassID and then invoke the AEEApplet_New() function
    that has been provided in AEEAppGen.c. 

   After invoking AEEApplet_New(), this function can do app specific initialization. In this
   example, a generic structure is provided so that app developers need not change app specific
   initialization section every time except for a call to IDisplay_InitAppData(). 
   This is done as follows: InitAppData() is called to initialize AppletData 
   instance. It is app developers responsibility to fill-in app data initialization 
   code of InitAppData(). App developer is also responsible to release memory 
   allocated for data contained in AppletData -- this can be done in 
   IDisplay_FreeAppData().

PROTOTYPE:
   int AEEClsCreateInstance(AEECLSID ClsId,IShell * pIShell,IModule * po,void ** ppObj)

PARAMETERS:
    clsID: [in]: Specifies the ClassID of the applet which is being loaded

    pIShell: [in]: Contains pointer to the IShell object. 

    pIModule: pin]: Contains pointer to the IModule object to the current module to which
    this app belongs

    ppObj: [out]: On return, *ppObj must point to a valid IApplet structure. Allocation
    of memory for this structure and initializing the base data members is done by AEEApplet_New().

DEPENDENCIES
  none

RETURN VALUE
  AEE_SUCCESS: If the app needs to be loaded and if AEEApplet_New() invocation was
     successful
  EFAILED: If the app does not need to be loaded or if errors occurred in 
     AEEApplet_New(). If this function returns FALSE, the app will not be loaded.

SIDE EFFECTS
  none
===========================================================================*/
int AEEClsCreateInstance(AEECLSID ClsId, IShell *pIShell, IModule *po, void **ppObj)
{
    *ppObj = NULL;

    if( ClsId == AEECLSID_ABPAL_GPSTEST )
    {
        // Create the applet and make room for the applet structure
        if( AEEApplet_New(sizeof(gpstest),
                          ClsId,
                          pIShell,
                          po,
                          (IApplet**)ppObj,
                          (AEEHANDLER)gpstest_HandleEvent,
                          (PFNFREEAPPDATA)gpstest_FreeAppData) ) // the FreeAppData function is called after sending EVT_APP_STOP to the HandleEvent function
                          
        {
            //Initialize applet data, this is called before sending EVT_APP_START
            // to the HandleEvent function
            if(gpstest_InitAppData((gpstest*)*ppObj))
            {
                //Data initialized successfully
                return(AEE_SUCCESS);
            }
            else
            {
                //Release the applet. This will free the memory allocated for the applet when
                // AEEApplet_New was called.
                IAPPLET_Release((IApplet*)*ppObj);
                return EFAILED;
            }

        } // end AEEApplet_New

    }

    return(EFAILED);
}


/*===========================================================================
FUNCTION SampleAppWizard_HandleEvent

DESCRIPTION
    This is the EventHandler for this app. All events to this app are handled in this
    function. All APPs must supply an Event Handler.

PROTOTYPE:
    boolean SampleAppWizard_HandleEvent(IApplet * pi, AEEEvent eCode, uint16 wParam, uint32 dwParam)

PARAMETERS:
    pi: Pointer to the AEEApplet structure. This structure contains information specific
    to this applet. It was initialized during the AEEClsCreateInstance() function.

    ecode: Specifies the Event sent to this applet

   wParam, dwParam: Event specific data.

DEPENDENCIES
  none

RETURN VALUE
  TRUE: If the app has processed the event
  FALSE: If the app did not process the event

SIDE EFFECTS
  none
===========================================================================*/
static boolean gpstest_HandleEvent(gpstest* pMe, AEEEvent eCode, uint16 wParam, uint32 dwParam)
{  
    if (eCode == EVT_KEY && wParam == AVK_CLR && pMe->state == GPSTEST_STATE_MENU)
    {
        ISHELL_CloseApplet(pMe->a.m_pIShell, FALSE); // Close the applet without exit to Idle
    }

    if ((pMe->state == GPSTEST_STATE_MENU) && 
         IMENUCTL_IsActive(pMe->pIMenuCtl) && 
         IMENUCTL_HandleEvent(pMe->pIMenuCtl, eCode, wParam, dwParam))
        return TRUE;

    switch (eCode) 
    {
        // App is told it is starting up
        case EVT_APP_START:                        
            IMENUCTL_SetActive( pMe->pIMenuCtl, TRUE );
            pMe->state = GPSTEST_STATE_MENU;
            return(TRUE);


        // App is told it is exiting
        case EVT_APP_STOP:
            // Add your code here...

              return(TRUE);


        // App is being suspended 
        case EVT_APP_SUSPEND:
            IMENUCTL_SetActive( pMe->pIMenuCtl, FALSE);
              return(TRUE);


        // App is being resumed
        case EVT_APP_RESUME:
            switch(pMe->state)
            {
            case GPSTEST_STATE_GET_LOCATION:
                //todo update screen
                break;
            case GPSTEST_STATE_MENU:
            default:
                IMENUCTL_SetActive( pMe->pIMenuCtl, TRUE );
                IMENUCTL_Redraw(pMe->pIMenuCtl);
                break;
            }
              return(TRUE);

        // A key was pressed. Look at the wParam above to see which key was pressed. The key
        // codes are in AEEVCodes.h. Example "AVK_1" means that the "1" key was pressed.
        case EVT_KEY:
            switch (wParam)
            {
            case AVK_SELECT:
                if (pMe->state == GPSTEST_STATE_SUSPEND)
                {
                    ABPAL_GpsTrackingInfo trackingInfo = { 0 };
                    trackingInfo.Id = GPS_TRACKING_ID_1;
                    if(pMe->suspend)
                    {
                        pMe->error = ABPAL_GpsResume(pMe->context, &trackingInfo);
                        gpsTestLog(pMe,"ABPAL_GpsResume");
                    }
                    else
                    {
                        pMe->error = ABPAL_GpsSuspend(pMe->context, &trackingInfo);
                        gpsTestLog(pMe,"ABPAL_GpsSuspend");
                    }
                    pMe->suspend = !pMe->suspend;
                }
                break;
            case AVK_CLR:
                if (pMe->state != GPSTEST_STATE_MENU)
                {
                    //cancel test
                    TestGpsCancelTests(pMe);
                    pMe->state = GPSTEST_STATE_MENU;
                    IMENUCTL_SetActive(pMe->pIMenuCtl, TRUE);
                    IMENUCTL_Redraw(pMe->pIMenuCtl);
                }
                break;
            default:
                break;
            }

             return(TRUE);


        case EVT_COMMAND:  // Handle 'SELECT' button events
            switch(wParam)
            {
            case MAIN_MENU_ID_GPSID_GET_LOCATION:
                pMe->state = GPSTEST_STATE_GET_LOCATION;
                TestGpsIdGetLocation(pMe);
                break;
            case MAIN_MENU_ID_GPSID_TRACKING:
                pMe->state = GPSTEST_STATE_TRACKING;
                TestGpsIdTracking(pMe);
                break;
            case MAIN_MENU_ID_GPSID_SUSPEND:
                pMe->state = GPSTEST_STATE_SUSPEND;
                TestGpsIdSuspend(pMe);
                break;
            default:
                break;
            }
            break;

        // If nothing fits up to this point then we'll just break out
        default:
            break;
   }

   return FALSE;
}


// this function is called when your application is starting up
boolean gpstest_InitAppData(gpstest* pMe)
{
    boolean ret_stat = FALSE;
    pMe->error = NE_OK;
    pMe->state = GPSTEST_STATE_INIT;

    clearTestLog(pMe);

    pMe->DeviceInfo.wStructSize = sizeof(pMe->DeviceInfo);
    ISHELL_GetDeviceInfo(pMe->a.m_pIShell,&pMe->DeviceInfo);

    DBGPRINTF("Initializing applet...");
    ISHELL_CreateInstance(pMe->a.m_pIShell, AEECLSID_MENUCTL, (void**)&pMe->pIMenuCtl);

    if (pMe->pIMenuCtl)
    {
        IMENUCTL_SetTitle(pMe->pIMenuCtl, NULL, 0, L"GPS Test");
        IMENUCTL_SetProperties(pMe->pIMenuCtl, MP_WRAPSCROLL);
        IMENUCTL_AddItem(pMe->pIMenuCtl, NULL, 0, MAIN_MENU_ID_GPSID_GET_LOCATION, L"TestGpsIdGetLocation", NULL);
        IMENUCTL_AddItem(pMe->pIMenuCtl, NULL, 0, MAIN_MENU_ID_GPSID_TRACKING,     L"TestGpsIdTracking", NULL);
        IMENUCTL_AddItem(pMe->pIMenuCtl, NULL, 0, MAIN_MENU_ID_GPSID_SUSPEND,      L"TestGpsIdSuspend", NULL);
        ret_stat = TRUE;
    }

    return ret_stat;
}

// this function is called when your application is exiting
void gpstest_FreeAppData(gpstest* pMe)
{
    if(pMe->pIMenuCtl) IMENUCTL_Release(pMe->pIMenuCtl);
}

//-----------------------utils----------------
void getTime(AECHAR* buf, int buf_len)
{
    JulianType time;
    GETJULIANDATE(0, &time);
    WSPRINTF(buf, buf_len,L"%02d:%02d:%02d", time.wHour, time.wMinute, time.wSecond);
}

void clearTestLog(gpstest* pMe)
{
    IFileMgr* pIFileMgr = NULL;
    IFile*    logFile = NULL;
    ISHELL_CreateInstance(pMe->a.m_pIShell, AEECLSID_FILEMGR, (void**)&pIFileMgr);
    if (pIFileMgr)
    {
        IFILEMGR_Remove(pIFileMgr, ABPAL_GPSTEST_LOG_FILE);
        IFILEMGR_Release(pIFileMgr);
    }
}

void gpsTestLog(gpstest* pMe, char* logBuffer)
{
    if(logBuffer)
    {
        IFileMgr* pIFileMgr = NULL;
        IFile*    logFile = NULL;
        JulianType time;
        char buffer[255] = {0};
        
        GETJULIANDATE(0, &time);
        SNPRINTF(buffer, sizeof(buffer),"%02d:%02d:%02d %s \terror code: %d\n",
            time.wHour, time.wMinute, time.wSecond, logBuffer,pMe->error);

        DBGPRINTF("%s", buffer);
        ISHELL_CreateInstance(pMe->a.m_pIShell, AEECLSID_FILEMGR, (void**)&pIFileMgr);
        if (pIFileMgr)
        {
            logFile = IFILEMGR_OpenFile(pIFileMgr, ABPAL_GPSTEST_LOG_FILE, _OFM_APPEND);
            if (!logFile && (IFILEMGR_GetLastError(pIFileMgr) == EFILENOEXISTS))
            {
                logFile = IFILEMGR_OpenFile(pIFileMgr, ABPAL_GPSTEST_LOG_FILE, _OFM_CREATE);
                IFILE_Release(logFile);
                logFile = IFILEMGR_OpenFile(pIFileMgr, ABPAL_GPSTEST_LOG_FILE, _OFM_APPEND);
            }
            if (logFile)
            {
                IFILE_Write(logFile, buffer, STRLEN(buffer));
                IFILE_Release(logFile);
            }
            IFILEMGR_Release(pIFileMgr);
            pIFileMgr = NULL;
        }
    }
}
void gpsTestLogW(gpstest* pMe, AECHAR* logBuffer)
{
    char buf[100] = {0};
    WSTRTOSTR(logBuffer, buf, sizeof(buf));
    gpsTestLog(pMe, buf);
}

void DrawText(gpstest* pMe, AECHAR* text, int y)
{
    IDISPLAY_DrawText(pMe->a.m_pIDisplay,    
                   AEE_FONT_NORMAL,     
                   text,              
                   -1,                  // -1 = Use full string length
                   GPSTEST_INFORECT_START_X,                   
                   y,                   
                   NULL,                // No clipping
                   0);                  // No flags
}
void DrawHeading (gpstest* pMe, AECHAR* text)
{
     DrawText(pMe, text, 0);
}

void DrawFootnotes (gpstest* pMe, boolean retryAvailable)
{
    DrawText(pMe, L"Back key: Menu, Susped/Resume: SELECT",   pMe->DeviceInfo.cyScreen - GPSTEST_LINE_HEIGHT);
}
void DrawError(gpstest* pMe)
{
    AECHAR buffer[50]={0};
    WSPRINTF(buffer, 50, L"Oops, we get an error: %d", pMe->error);
    DrawText(pMe, buffer, GPSTEST_INFORECT_START_Y);
}

void Draw_TestGpsIdGetLocation (gpstest* pMe)
{
    ABPAL_GpsGetState(pMe->context, &pMe->gpsState);  
    IDISPLAY_ClearScreen(pMe->a.m_pIDisplay);
    DrawHeading(pMe, L"TestGpsIdGetLocation");
    DrawFootnotes (pMe, TRUE);

    if (pMe->error == NE_OK)
    {
        AECHAR buffer[50] = {0};
        AECHAR float_buffer[20] = {0};

        double float_val = WGS84_TO_DEGREES(pMe->location.latitude);
        FLOATTOWSTR(float_val, float_buffer, 20);
        WSPRINTF(buffer, sizeof(buffer), L"Latitude: %s", float_buffer);
        DrawText(pMe, buffer, GPSTEST_INFORECT_START_Y);

        float_val = WGS84_TO_DEGREES(pMe->location.longitude);
        FLOATTOWSTR(float_val, float_buffer, 20);
        WSPRINTF(buffer, sizeof(buffer), L"Longitude: %s", float_buffer);
        DrawText(pMe, buffer, GPSTEST_INFORECT_START_Y + GPSTEST_LINE_HEIGHT);

        WSPRINTF(buffer, sizeof(buffer), L"Status: %d", pMe->gpsState);
        DrawText(pMe, buffer, GPSTEST_INFORECT_START_Y + GPSTEST_LINE_HEIGHT*2);

        getTime(float_buffer, sizeof(float_buffer));
        WSPRINTF(buffer, sizeof(buffer), L"Time: %s", float_buffer);
        DrawText(pMe, buffer, GPSTEST_INFORECT_START_Y + GPSTEST_LINE_HEIGHT*3);
    }
    else
    {
        DrawError(pMe);
    }

    IDISPLAY_Update(pMe->a.m_pIDisplay);
}
//----------------------------------------------------
//--------tests---------------------------------------
//----------------------------------------------------

static boolean createABPALGps(gpstest* pMe)
{
    pMe->pal = PAL_CreateInstance(pMe);
    if(pMe->pal)
    {
        gpsTestLog(pMe,"PAL_CreateInstance");
        pMe->error = ABPAL_GpsCreate(pMe->pal, &pMe->context);
        gpsTestLog(pMe,"ABPAL_GpsCreate");
        return (pMe->error == NE_OK);
    }
    else
    {
        pMe->error = NE_API;
        gpsTestLog(pMe,"PAL_CreateInstance");
        return FALSE;
    }
}

static void deleteABPALGps(gpstest* pMe)
{
    PAL_Error err = PAL_Ok;
    if(pMe->context)
    {
        err = ABPAL_GpsDestroy(pMe->context);
        pMe->context = NULL;
        gpsTestLog(pMe,"ABPAL_GpsDestroy");
    }
    if(pMe->pal)
    {
        PAL_Destroy(pMe->pal);
        pMe->pal = NULL;
        gpsTestLog(pMe,"PAL_Destroy");
    }

}
//----------------------------
#define pdeHost       "PDEIS801.vzwtest.com"
#define pdePort       "8888"
#define pdeIp         "15.178.174.131"
#define nfix           "14401"
#define nint           "1"
#define nopt           "1"
#define nqos           "16"


NB_Error gpsConfig(gpstest* pMe)
{
    pMe->gpsConfig[0].name = "providertype";
    pMe->gpsConfig[0].value = "pos";
    pMe->gpsConfig[1].name = "host";
    pMe->gpsConfig[1].value = MALLOC( (STRLEN(pdeHost)*sizeof(char)+1) );
    STRCPY( pMe->gpsConfig[1].value, pdeHost );
    pMe->gpsConfig[2].name = "ip";
    pMe->gpsConfig[2].value = MALLOC( (STRLEN(pdeIp)*sizeof(char)+1) );
    STRCPY( pMe->gpsConfig[2].value, pdeIp );

    pMe->gpsConfig[3].name = "port";        
    pMe->gpsConfig[3].value = MALLOC( (STRLEN(pdePort)*sizeof(char)+1) );
    STRCPY( pMe->gpsConfig[3].value, pdePort);      
             
    pMe->gpsConfig[4].name = "nfix";        
    pMe->gpsConfig[4].value = MALLOC( (STRLEN(nfix)*sizeof(char)+1) );
    STRCPY( pMe->gpsConfig[4].value, nfix );      

    pMe->gpsConfig[5].name = "nint";        
    pMe->gpsConfig[5].value = MALLOC( (STRLEN(nint )*sizeof(char)+1) );
    STRCPY( pMe->gpsConfig[5].value, nint );      

    pMe->gpsConfig[6].name = "nopt";        
    pMe->gpsConfig[6].value = MALLOC( (STRLEN(nopt)*sizeof(char)+1) );
    STRCPY( pMe->gpsConfig[6].value, nopt );      

    pMe->gpsConfig[7].name = "nqos";        
    pMe->gpsConfig[7].value = MALLOC( (STRLEN(nqos)*sizeof(char)+1) );
    STRCPY( pMe->gpsConfig[7].value, nqos );      
                
    pMe->configCount = GPS_MAX_CONFIG;     

    pMe->error =  ABPAL_GpsInitialize( pMe->context, pMe->gpsConfig, pMe->configCount, NULL, NULL, NULL);
    gpsTestLog(pMe,"ABPAL_GpsInitialize");
    return pMe->error;
}
void setCriteria(gpstest* pMe)
{
    pMe->criteria.desiredAccuracy = AEEGPS_ACCURACY_LOWEST;
    pMe->criteria.desiredInformation = AEEGPS_GETINFO_LOCATION;
    pMe->criteria.desiredInterval = 1 * 1000;
    pMe->criteria.desiredMaxAge = 0;
    pMe->criteria.desiredNetworkAssist = TRUE;
    pMe->criteria.desiredPerformance = 16;
    pMe->criteria.desiredSetting = PGC_Interval; // PGC_Accuracy | PGC_Performance | PGC_Interval | PGC_Timeout | PGC_MaxAge;
    pMe->criteria.desiredTimeout = 0;
    pMe->criteria.desiredTotalFixes = 14401;
}

static void
GpsGetLocationCallback( const void* userData, PAL_Error error, const ABPAL_GpsLocation* plocation, const char* errorInfoXml )
{
    gpstest *pMe = (gpstest*) userData;
    pMe->error = error;
    gpsTestLog(pMe,"GpsGetLocationCallback");

    if(error !=  PAL_Ok )
    {
        DrawError(pMe);
    }
    else
    {
        pMe->location = *plocation;
        Draw_TestGpsIdGetLocation(pMe);
    }
}

static void
GpsBeginTrackingCallback( const void* userData, PAL_Error error, const ABPAL_GpsLocation* plocation, const char* errorInfoXml )
{
    gpstest *pMe = (gpstest*) userData;
    pMe->error = error;
    gpsTestLog(pMe,"GpsBeginTrackingCallback");

    if(error !=  PAL_Ok )
    {
        DrawError(pMe);
    }
    else
    {
        pMe->location = *plocation;
        Draw_TestGpsIdGetLocation(pMe);
    }
}
//----------------------------
void TestGpsCancelTests(gpstest* pMe)
{
    //cancel tracking
    ABPAL_GpsTrackingInfo trackingInfo = { 0 };
    trackingInfo.Id = GPS_TRACKING_ID_1;
    pMe->error = ABPAL_GpsEndTracking(pMe->context, &trackingInfo);
    gpsTestLog(pMe,"ABPAL_GpsEndTracking");
    //cancel cb
    pMe->error = (pMe->error) ? pMe->error : ABPAL_GpsCancelGetLocation(pMe->context);
    gpsTestLog(pMe,"ABPAL_GpsCancelGetLocation");

    //delete ABPAL
    deleteABPALGps(pMe);
}

void TestGpsIdGetLocation(gpstest* pMe)
{
    pMe->error = NE_OK;
    deleteABPALGps(pMe);
    if(createABPALGps(pMe))
    {
        pMe->error = gpsConfig(pMe);
        if(pMe->error == NE_OK)
        {
            setCriteria(pMe);
            pMe->error = ABPAL_GpsGetLocation(pMe->context, &pMe->criteria, GpsGetLocationCallback, pMe);
            gpsTestLog(pMe,"ABPAL_GpsGetLocation");
        }
    }
}

void TestGpsIdTracking(gpstest* pMe)
{
    ABPAL_GpsTrackingInfo trackingInfo = { 0 };
    trackingInfo.Id = GPS_TRACKING_ID_1;
    deleteABPALGps(pMe);
    if(createABPALGps(pMe))
    {
        pMe->error = gpsConfig(pMe);
        if(pMe->error == NE_OK)
        {
            setCriteria(pMe);
            pMe->error = ABPAL_GpsBeginTracking(pMe->context, FALSE, &pMe->criteria, GpsBeginTrackingCallback, pMe, &trackingInfo);
            gpsTestLog(pMe,"ABPAL_GpsBeginTracking");
        }
    }
}

void TestGpsIdSuspend(gpstest* pMe)
{
    pMe->suspend = FALSE;
    TestGpsIdTracking(pMe);
}
