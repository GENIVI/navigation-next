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

/* =========================================================================

FILE: AEEAppGen.c

SERVICES: Sample IApplet implementation file

DESCRIPTION
  This file contains sample implementation of the IApplet interface

PUBLIC CLASSES:  
   N/A

INITIALIZATION AND SEQUENCING REQUIREMENTS:  N/A


           Copyright © 1999-2002 QUALCOMM Incorporated.
                     All Rights Reserved.
                   QUALCOMM Proprietary/GTDR
  ========================================================================= */


/* =========================================================================
                     INCLUDES AND VARIABLE DEFINITIONS
   ========================================================================= */

#include "AEE.h"        // Standard AEE definitions
#include "AEEShell.h"   // AEE Shell Services
#include "AEEStdLib.h"  // AEE stdlib stuff
#include "AEEAppGen.h"  // AEEApplet definitions
#include "AEEModGen.h"  // AEEMod definitions and utility function
#include "nmdef.h"


/* =========================================================================

                      PUBLIC DATA DECLARATIONS

   ========================================================================= */

/*-------------------------------------------------------------------
            Type Declarations
-------------------------------------------------------------------*/


/*-------------------------------------------------------------------
            Function Prototypes
-------------------------------------------------------------------*/
//Functions belonging to the AEEApplet VTBL

static uint32  AEEApplet_AddRef(IApplet * po);
static uint32  AEEApplet_Release(IApplet * po);
static boolean AEEApplet_HandleEvent(IApplet * po, AEEEvent evt, 
                                     uint16 wParam, uint32 dwParam);

/*-------------------------------------------------------------------
            Global Constant Definitions
-------------------------------------------------------------------*/


/*-------------------------------------------------------------------
            Global Data Definitions
-------------------------------------------------------------------*/


/*-------------------------------------------------------------------
            Static variable Definitions
-------------------------------------------------------------------*/


/* =========================================================================
                     FUNCTION DEFINITIONS
   ========================================================================= */

/* =========================================================================

Function:  AEEApplet_AddRef()

Description:
   This function increases the referecne count for the AEEApplet object

Prototype:
   uint32  AEEApplet_AddRef(IApplet * po)

Parameters:
   po: Pointer to IApplet interface whos reference count is to be incremented

Return Value:
   The updated reference count

Comments:  None

Side Effects: None

   ========================================================================= */
static uint32  AEEApplet_AddRef(IApplet * po)
{
   return ++(((AEEApplet *)po)->m_nRefs);
}

/* =========================================================================

Function:  AEEApplet_Release()

Description:
   This function decreases the referecne count for the IApplet object. If
   the reference count reaches zero, it does the required cleanup
   
   If the application developer has added data to the AEEApplet structure,
   then the corresponding cleanup operations (if any) must be done here for those
   data members

Prototype:
   uint32 AEEApplet_Release(IApplet * po)

Parameters:
   po: Pointer to the IApplet interface whose reference count needs to be
       decremented.

Return Value:
   The updated reference count

Comments:  None

Side Effects: None

   ========================================================================= */
static uint32 AEEApplet_Release(IApplet * po)
{
   IShell * pIShell = NULL;

   AEEApplet * pme = (AEEApplet *)po;

   if (--pme->m_nRefs)
      return(pme->m_nRefs);

   // Invoke the APP's FreeAppData function. This gives them a chance to 
   // free-up any data they may have allocated when the app was created.

   if(pme->pFreeAppData)
      pme->pFreeAppData(po);

   // Release the interfaces created in AEEApplet_New()
   if (pme->m_pIDisplay)
      IDISPLAY_Release(pme->m_pIDisplay);   


   IMODULE_Release(pme->m_pIModule);

   pIShell = pme->m_pIShell;

   // delete this AEEApplet object
   FREE_VTBL(pme, IApplet);
   FREE(pme);   
   
   ISHELL_Release(pIShell);   // Release the Shell

   return 0;
}


/* ===========================================================================

FUNCTION AEEApplet_HandleEvent

DESCRIPTION
   This is the EventHandler for this app. All events to this app are handled 
   in this function. This function passes the event to the HandleEvent() 
   function that has been registered by the app during AEE_AppNew() function

PROTOTYPE:
   boolean AEEApplet_HandleEvent(IApplet * pi, AEEEvent eCode, uint16 wParam, 
                                 uint32 dwParam)

PARAMETERS:
   pi: Pointer to the AEEApplet structure. This structure contains information
       specific to this applet. It was initialized during the 
       AppCreateInstance() function.

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
boolean  AEEApplet_HandleEvent(IApplet * po, AEEEvent evt, uint16 wParam, 
                               uint32 dwParam)
{
   return ((AEEApplet *)po)->pAppHandleEvent(po, evt,wParam,dwParam);
}


/* =========================================================================

FUNCTION AEEApplet_New

DESCRIPTION
   This function is invoked when the app is being loaded. It creates a new 
   instance of the AEEApplet structure for this applet. This function is 
   invoked by the app from within the AEEClsCreateInstance() function

PROTOTYPE:
   boolean AEEApplet_New(int16 nSize,
                         AEECLSID clsID,
                         IShell * pIShell, 
                         IModule * pIModule, 
                         IApplet **ppobj, 
                         AEEHANDLER pAppHandleEvent,
                         PFNFREEAPPDATA pFreeAppData)


PARAMETERS:
   nSize: [in]: Specifies the size of the structure that needs to be allocated
           for the applet. Typically,applets will define their struct (say, 
           CSocketApp) that contains AEEApplet as the first element. This 
           structure will then contain app specific data. The size of the 
           entire struct (CSocketApp) needs to be passed here.

   clsID: [in]: Specifies the ClassID of this applet

   pIShell: [in]: Contains a valid pointer to the AEE IShell interface

   pIModule: [in]: contains a vlaid pointer to the module to which this app 
            belongs

   ppobj: [out]: On return, *ppobj points to a valid AEEApplet structure. This
        allocation is done by this function

   pAppHandleEvent: [in]: Functipn Pointer to the App's HandleEvent function.
                 It is used from within AEEAppet_HandleEvent()

   pFreeAppdata: [in]: Function pointer to the App's function to free the app
               data.  This function will be invoked when the reference count 
               of the app reaches zero. The app must free all its dynamically
               allocated data inside this function.

DEPENDENCIES
  none

RETURN VALUE
  TRUE: If successful
  FALSE: If failed

SIDE EFFECTS
  none
  ===========================================================================*/
boolean AEEApplet_New(int16 nIn,
                      AEECLSID clsID,
                      IShell * pIShell, 
                      IModule * pIModule, 
                      IApplet **ppobj, 
                      AEEHANDLER pAppHandleEvent,
                      PFNFREEAPPDATA pFreeAppData)
{
   AEEApplet *       pme = NULL;
   VTBL(IApplet) *   appFuncs;
   int               nSize;

   if(nIn < 0)
      return(FALSE);

   nSize = (int)nIn;

   if(!ppobj)
      return(FALSE);

   *ppobj = NULL;

   //Validate parameters
   if (!pIShell || !pIModule)
      return FALSE;

   if(nSize < sizeof(AEEApplet))
      nSize += sizeof(AEEApplet);

   // Create an Applet object

   if (NULL == (pme = (AEEApplet*)MALLOC(nSize + sizeof(IAppletVtbl))))
      return FALSE;

   appFuncs = (IAppletVtbl *)((byte *)pme + nSize);

   //Initialize the individual entries in the VTBL
   appFuncs->AddRef      = AEEApplet_AddRef;
   appFuncs->Release     = AEEApplet_Release;
   appFuncs->HandleEvent = AEEApplet_HandleEvent;


   INIT_VTBL(pme, IApplet, *appFuncs);   // Initialize the VTBL

   //Initialize the data members
   pme->m_nRefs      = 1;
   pme->m_pIShell    = pIShell;
   pme->m_pIModule   = pIModule;
   pme->m_pIDisplay  = NULL;

   pme->clsID        = clsID;

   //Store the function pointers to APP's HandleEvent and FreeAppData functions
   pme->pAppHandleEvent = pAppHandleEvent;
   pme->pFreeAppData = pFreeAppData;

   ISHELL_CreateInstance(pIShell, AEECLSID_DISPLAY, 
                         (void **)&pme->m_pIDisplay);

   if (!pme->m_pIDisplay) {
      //Cleanup
      FREE_VTBL(pme, IApplet);
      FREE(pme);
      return FALSE;
   }
   ISHELL_AddRef(pIShell);
   IMODULE_AddRef(pIModule);


   *ppobj = (IApplet*)pme;
  
   return TRUE;
}


/* ============= End of File =============================================== */






