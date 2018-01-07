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

///////////////////////////////////////////////////////////////////////////////
///  LpsPosDet.h
///  Secured Position determination APIs.
///  ILpsPosDet Interface exports the Secured basestation information and gpsOne
///  based position determination services.
///
///  @remarks None
///
///  @date 11/3/2004
///
/// Revision History:-
///-----------------------------------------------------------------
/// Sr No	Date				Note
///-----------------------------------------------------------------
///	1)		27-Apr-2005 		Changed the class ID for ILpsPosDet.
/// 2)		05-May-2005			Added three error code viz
///								EFAILED_LPSCONNECTION, 	EFAILED_EULADECLINED, EFAILED_FTTCANCELLED
///	3)		04-Jun-2005			Added error code EFAILED_AUTHENTICATION, for Authentication failed.
///
/// 4)		27-Apr-2005 		Changed the class ID for ILpsPosDet.
///
/// 5)		13-Oct-2005 		Updated documentation for ILPSPOSDET_Init().
///
/// 6)		29-Oct-2005 		Changed the class ID for ILpsPosDet.
///////////////////////////////////////////////////////////////////////////////

#ifndef _LPSPOSDET_H_
#define _LPSPOSDET_H_


/*********************************************************************/
/*                                                                   */
/* Copyright (c) 2006 Autodesk Inc.									 */
/*                                                                   */
/*********************************************************************/

#include <AEE.h>
#include <AEEShell.h>
#include <AEEDisp.h>
#include <AEEAppGen.h>
#include <AEEModGen.h>
#include <AEEStdLib.h>
#include <AEEClassIDs.h>
#include <AEEFile.h>
#include <AEEPosDet.h>
#include <BrewVersion.h>

#define ILPS_BASE_ERROR			-1
#define EFAILED_LPSCONNECTION	ILPS_BASE_ERROR - 1  // Unable to , connect to LPS,
													 // Unable to download (EULA)End User License Agreement
													 // Unable to save the privacy settings
#define EFAILED_EULADECLINED	ILPS_BASE_ERROR - 2  // EULA was declined
#define EFAILED_FTTCANCELLED	ILPS_BASE_ERROR - 3  // FTT(First Time through Provisioning) process was cancelled

#define EFAILED_AUTHENTICATION ILPS_BASE_ERROR	-4	// Authentication failed, because of invalid ClientID.

#define	EFAILED_LOCATIONDISABLED	ILPS_BASE_ERROR	-5  // Location disabled

//LPS R1.1 change
#define AEEGPS_METHOD_UNKNOWN  0
////////////////////

//////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
extern "C" {
#endif

typedef struct _ILpsPosDet ILpsPosDet;

QINTERFACE(ILpsPosDet)
{
	INHERIT_IQueryInterface(ILpsPosDet);

	int	(*GetSectorInfo)(ILpsPosDet*, AEESectorInfo*);

	int	(*SetGPSConfig)(ILpsPosDet*, AEEGPSConfig*);

	int (*GetGPSConfig)(ILpsPosDet*, AEEGPSConfig*);

	int (*GetGPSInfo)(ILpsPosDet*, AEEGPSReq, AEEGPSAccuracy, AEEGPSInfo*,AEECallback*);

	int	(*GetOrientation)(ILpsPosDet*, AEEOrientationReq,void*,AEECallback*);

	int (*Init)(ILpsPosDet*, uint32, const char*, AEECallback*, int*);

#if MIN_BREW_VERSIONEx(3, 1, 3)
    int	(*ExtractPositionInfo)(ILpsPosDet*, AEEGPSInfo*, AEEPositionInfoEx*);
#endif
};


#define ILPSPOSDET_AddRef(pILpsPosDet)\
	GET_PVTBL(pILpsPosDet, ILpsPosDet)->AddRef(pILpsPosDet)

#define ILPSPOSDET_Release(pILpsPosDet)\
	GET_PVTBL(pILpsPosDet, ILpsPosDet)->Release(pILpsPosDet)

#define ILPSPOSDET_QueryInterface(pILpsPosDet, aeeCLSID, ppo)\
	GET_PVTBL(pILpsPosDet, ILpsPosDet)->QueryInterface(pILpsPosDet, aeeCLSID, ppo)

#define ILPSPOSDET_GetGPSInfo(pILpsPosDet, aeeGPSReq, aeeGPSAccuracy, pAEEGPSInfo, pAEECallback)\
	GET_PVTBL(pILpsPosDet, ILpsPosDet)->GetGPSInfo(pILpsPosDet, aeeGPSReq, aeeGPSAccuracy,\
														pAEEGPSInfo, pAEECallback)

#define ILPSPOSDET_GetOrientation(pILpsPosDet, aeeOrientationReq, pOrInfo, pAEECallback)\
	GET_PVTBL(pILpsPosDet, ILpsPosDet)->GetOrientation(pILpsPosDet, aeeOrientationReq,\
		pOrInfo, pAEECallback)

#define ILPSPOSDET_GetSectorInfo(pILpsPosDet, pAEESectorInfo)\
	GET_PVTBL(pILpsPosDet, ILpsPosDet)->GetSectorInfo(pILpsPosDet, pAEESectorInfo)

#define ILPSPOSDET_GetGPSConfig(pILpsPosDet, pAEEGPSConfig)\
	GET_PVTBL(pILpsPosDet, ILpsPosDet)->GetGPSConfig(pILpsPosDet, pAEEGPSConfig)

#define ILPSPOSDET_SetGPSConfig(pILpsPosDet, pAEEGPSConfig)\
	GET_PVTBL(pILpsPosDet, ILpsPosDet)->SetGPSConfig(pILpsPosDet, pAEEGPSConfig)


#define ILPSPOSDET_Init(pILpsPosDet, unClientID, pcszClientPwd, pAEECallback, pnStatus)\
	GET_PVTBL(pILpsPosDet, ILpsPosDet)->Init(pILpsPosDet, unClientID, pcszClientPwd, pAEECallback,\
	 pnStatus)

#if MIN_BREW_VERSIONEx(3, 1, 3)
#define ILPSPOSDET_ExtractPositionInfo(pILpsPosDet, piAEEGpsInfo,poAEEPositionInfoEx)\
	GET_PVTBL(pILpsPosDet, ILpsPosDet)->ExtractPositionInfo(pILpsPosDet, piAEEGpsInfo,poAEEPositionInfoEx)
#endif

#ifdef __cplusplus
}
#endif

/*=======================================================================
INTERFACES   DOCUMENTATION
=======================================================================
Interface Name: ILpsPosdet

Description:
This interface provides services for position determination using sector
information or GPS information for self handset.
In order to use the sector-based postion determination methods such as
ILPSPOSDET_GetSectorInfo, SectorInfo privileges are required.
Similarily, for GPS based position determination methods such as
ILPSPOSDET_SetGPSConfig, ILPSPOSDET_GetGPSConfig, and ILPSPOSDET_GetGPSInfo,
Position Determination privileges are required.

Methods such as ILPSPOSDET_GetGPSInfo, are asynchronous methods which use AEECallbacks.
Care must be taken to ensure that the callbacks and information structures passed
to these methods by reference remain in scope till the callback returns. If multiple
requests for GPS information are made without waiting for the previous
request callbacks to return, the request will return an EITEMBUSY error code.

BREW SDK users can set the GPS emulation in the Tools->GPS Emulation menu to use a pre-recorded
NMEA file as GPS input, or connect an NMEA-output capable GPS device, this is only
useful in case of Sector or GPS info, request for self.An offline utlity called
NMEALogger.exe can be used to record an NMEA file from data coming from a GPS device
connected to the serial port of the desktop/laptop. This NMEA file can be used later
as GPS input. See SDK User's Guide and SDK Utilities Guide for details.

The following methods are supported in this interface:

ILPSPOSDET_AddRef
ILPSPOSDET_Release
ILPSPOSDET_QueryInterface
ILPSPOSDET_GetSectorInfo
ILPSPOSDET_SetGPSConfig
ILPSPOSDET_GetGPSInfo
ILPSPOSDET_GetOrientation
ILPSPOSDET_Init
ILPSPOSDET_ExtractPositionInfo(for BREW 3.1.3+)

=======================================================================
Function: ILPSPOSDET_AddRef()

Description:
   This function increments the reference count of the ILpsPosDet
   Interface object, allowing the object to be shared by multiple callers.
   The object is freed when the reference  count reaches 0 (zero).

Prototype:

   uint32 ILPSPOSDET_AddRef(ILpsPosDet * pILpsPosDet)

Parameters:
   pILpsPosDet: [in]. Pointer to the ILpsPosDet Interface object.

Return Value:

   Incremented reference count for the object.

Comments:
   A valid object returns a positive reference count.

Side Effects:
   None

See Also:
   ILPSPOSDET_Release()

=======================================================================
Function: ILPSPOSDET_Release()

Description:
   This function decrements the reference count of the ILpsPosDet Interface
   object. The object is freed from memory and is no longer valid when the
   reference count reaches 0 (zero).

Prototype:

   uint32 ILPSPOSDET_Release(ILpsPosDet * pILpsPosDet)

Parameters:

   pILpsPosDet: [in]. Pointer to the ILpsPosDet Interface object.

Return Value:

   Decremented reference count for the object.
   0 (zero), If the object has been freed and is no longer valid.

Comments:
   None

Side Effects:
   None

See Also:
   ILPSPOSDET_AddRef()
=======================================================================

Function: ILPSPOSDET_QueryInterface()

Description:
   This function asks an object for another API contract from the object in question.

Prototype:

   int ILPSPOSDET_QueryInterface
   (
   ILpsPosDet * pILpsPosDet,
   AEECLSID aeeCLSID,
   void ** ppo
   )

Parameters:
   pILpsPosDet		: [in]. Pointer to the ILpsPosDet Interface object.
   aeeCLSID			:  [in]. Requested class ID exposed by the object
   ppo				: [out]. Returned object.  Filled by this method

Return Value:
   SUCCESS			: Interface found
   ENOMEMORY		: Insufficient memory
   ECLASSNOTSUPPORT	: Requested interface is unsupported

Comments:
   The pointer in *ppo is set to the new interface (with refcount positive), or NULL if the
   ClassID is not supported by the object.
   ppo MUST not be NULL.

Side Effects:
   None

See Also:
   None
=======================================================================
Function: ILPSPOSDET_GetSectorInfo

Description:

    	In current version this API will return EUNSUPPORTED
Prototype:

   int   ILPSPOSDET_GetSectorInfo(ILpsPosDet *pILpsPosDet, AEESectorInfo *pAEESecInfo)

Parameters:
   pILpsPosDet	: [in]. The interface pointer
   pAEESSecInfo		: [out]. This must be a value ptr to the AEESectorInfo structure.
						On return, the members of this struct contain sector information.

Return Value:
   SUCCESS		: if the function succeeded
   EPRIVLEVEL	: If the caller does not have sufficient privilege levels (PL_SECTORINFO)
   EBADPARM		: if pAEESecInfo is NULL
   EUNSUPPORTED	: if this function is not supported.
   ENOMEMORY	: Out of memory
   EFAILED		: General Failure
   EITEMBUSY	: If previous request is in progress and another request is made then this code is returned


Comments:

	SUCCESS:	When The operation completed successfully and the  pAEESectorInfo contains
				valid data.
	EPRIVLEVEL	: 1) The requester application is not authorized to get Sector Information,or
				  2) The user denied permission on the consent dialog box.
	ENOMEMORY	: Out of memory
	EFAILED		: General Failure

Side Effects:
   None

See Also:
   None
=======================================================================

Function: ILPSPOSDET_SetGPSConfig

Description:

    This function sets the GPS configuration to be used by the GPS engine.

Prototype:

   int ILPSPOSDET_SetGPSConfig(ILpsPosDet *pILpsPosDet, AEEGPSConfig *pAEEGPSConfig);


Parameters:
   pILpsPosDet	: [in]. The interface pointer.
   pConfig		: [in]. Pointer to GPS configuration. See AEEGPSConfig for details.


Return Value:
   SUCCESS   : if the function succeeded
   EPRIVLEVEL: if the caller does not have sufficient privilege levels
               (PL_POS_LOCATION) to invoke this function
   EBADPARM  : if pConfig is NULL or the data passed in the the *pConfig is invalid.
               (checks validity of mode, server.svrType, optim)
   EUNSUPPORTED: if this configuration is not supported on the system.


Comments:
   Until this function is called, the position determination engine will be
   configured with default settings. Only the position determination requests following
   a call to ILPSPOSDET_SetGPSConfig() will use the new configuration.

Side Effects:
   None

See Also:

=======================================================================
Function: ILPSPOSDET_GetGPSInfo

Description:

    This function returns information for GPS based position
    location. It returns latitude, longitude, altitude information, as well as
    vector information such as horizontal and vertical velocity, heading,
    and the uncertainity of the horizontal information. This is an asynchronous
    call, and the callback specified by pAEECallback is called on completion. If the
    position request is not satisfactorily answered, the status member of AEEGPSInfo
    will be non-zero values.

    This function enforces the privacy policies recommended by the network operator.
    Privacy implementation on the device may prompt the user using a dialog.
    Applications invoking this function must be prepared to surrender the screen for
    the dialog prompted to user. Application after invoking this function must suspend
    any paint to the screen on the event EVT_DIALOG_START and must redraw on the event
    EVT_DIALOG_END.


Prototype:

   int ILPSPOSDET_GetGPSInfo(ILpsPosDet *pILpsPosDet,
                                 AEEGPSReq aeeGPSReq,
                                 AEEGPSAccuracy aeeGPSAccuracy,
                                 AEEGPSInfo *pAEEGPSInfo,
                                 AEECallback *pAEECallback,
                                );

Parameters:
   pILpsPosDet	: [in]. The interface pointer.
   aeeGPSReq		: [in]. Request type (AEEGPS_GETINFO_LOCATION, AEEGPS_GETINFO_VELOCITY,
						AEEGPS_GETINFO_ALTITUDE). The flags can be combined to get more
						than one type of information.
   aeeGPSAccuracy	: [in]. Desired level of accuracy for this request.
   pAEEGPSInfo		: [out]. On input, this must be a valid ptr to the
					  AEEGPSInfo structure. On callback, the members of this
					  struct contain GPS information. The caller must ensure
					  that this structure is valid till the callback
					  specified by pcb gets called.
   pAEECallback    : [in]. Callback function which gets called on completion of position
						determination.


Return Value:
   SUCCESS		: if the function succeeded
   EPRIVLEVEL	: If the caller does not have sufficient privilege levels (PL_POS_LOCATION)
   EBADPARM		: if pAEEGPSInfo or pAEECallback is NULL
   EUNSUPPORTED	: if this function is not supported.
   ENOMEMORY	: Out of memory
   EFAILED		: General Failure
   EITEMBUSY	: If previous request is in progress and another request is made then this code is returned



Comments:
   The result of the operation is indicated in the "status" member of AEEGPSInfo when callback is invoked.
	The possible values status are:
	AEEGPS_ERR_NO_ERR :	Indicates that request is completely answered.
	AEEGPS_ERR_GENERAL_FAILURE : Indicates that reason for failure is a result of either device is low on resources or device is busy with other operations to handle this request or device is shutting down.

	AEEGPS_ERR_TIMEOUT : Request timedout.

	AEEGPS_ERR_INFO_UNAVAIL : Indicates that either partial or no information is available for the request. Check the fValid field to retrieve any partial information available.

	AEEGPS_ERR_PRIVACY_REFUSED : Indicates that the request is refused to protect the device's privacy. This occurs when the Privacy enforced on the device decided to reject the position requests of the application.

	EPRIVLEVEL	: 1) The requester application is not authorized to get GPS Information,or
				  2) The user denied permission on the consent dialog box.
	ENOMEMORY	: Out of memory
	EFAILED		: General Failure.

Side Effects:
   None

See Also:
   None


=======================================================================


Function: ILPSPOSDET_GetOrientation

Description:

    This function returns device's orientation in the horizontal plane.
    This is an asynchronous call, and the callback specified by pcb is
    called on completion.

Prototype:

   int ILPSPOSDET_GetOrientation(ILpsPosDet *pILpsPosDet,
                                 AEEOrientationReq aeeOrientationReq,
                                 void *pOrInfo,
                                 AEECallback *pAEECallback,
                                );

Parameters:
   pILpsPosDet		: [in]. The interface pointer.
   aeeOrientationReq    : [in]. The requested information. (Possible values include
							AEEORIENTATION_REQ_AZIMUTH)
   pOrInfo				: [out]. On input, this must be a valid ptr to the
						  valid memory with the first two bytes (wSize) indicating
						  the space available in bytes. The space should be a minimum
						  of that required to place the response corresponding the request.
						  On callback, the members of this struct contain Orientation
						  information corresponding to the request. The caller must ensure
						  that this memory is valid till the callback specified by pcb
						  gets called.
   pAEECallback			: [in]. Callback function which gets called on completion of the
							request.


Return Value:
   SUCCESS        : if the function succeeded
   EPRIVLEVEL     : if the caller does not have sufficient privilege levels
                    (PL_POS_LOCATION) to invoke this function
   EBADPARM       : if pGPSInfo or pcb is NULL
   EUNSUPPORTED   : if this function is not supported.
   EFAILED        : general failure



Comments:
   None

Side Effects:
   None

See Also:
   None


=======================================================================


Function: ILPSPOSDET_Init


Description :	This function is used to initialize an instance of the
			  	ILpsPosDet Extension library for use by a client application.
				This function registers the client application with the server, if the client
				is not registered. During registration process, the extension will display a
				dialog box.  Applications using this function must be prepared to surrender
				the screen for the dialog prompted to user. After invoking this function
				applications must suspend any paint to the screen on the event EVT_DIALOG_START
				and must redraw on the event EVT_DIALOG_END.

Prototype   int LpsPosDet_Init( ILpsPosDet* pILpsPosDet, uint32 unClientID,
								const char* pszClientPwd, AEECallback* pAEECallback,
								int* pnStatus)
Parameters:
pILpsPosDet		[in]: Pointer to  ILpsPosDet interface.

unClientID		[in]: Client ID.  This identifies the client application.

pwszClientPwd	[in]: Password.  Also known as a secret key.

pAEECallback	[in]: Callback function which gets called on completion of initialization.
					  It is always invoked if ILPSPOSDET_Init() returns SUCCESS, regardless of
					  whether the application is already registered or not.
					  If ILPSPOSDET_Init() returns an error, the callback function will not be called.

pnStatus		[out]: Pointer to a global variable that will contain the status of the initialization
                       and registration process when the callback is called.  The caller must ensure
                       that this member is valid until the callback specified by pAEECallback gets called.
                       It contains valid values only when the callback is called.

					   The possible values for the status are:
					   SUCCESS : The registration completed successfully
					   EFSFULL : File system full
					   EFAILED_LPSCONNECTION : Connection attempt to LPS Server failed
					   EFAILED_EULADECLINED : User declined the End User License Agreement (EULA) for the application
					   EFAILED_FTTCANCELLED : User cancelled the registration dialog (by pressing CLR key or by closing clamshell )
					   EFAILED_AUTHENTICATION : Authentication failed due to invalid client ID.
                       EFAILED_LOCATIONDISABLED : Position determination is reserved for emergency call only
					   ENOMEMORY : Out of memory
					   EFAILED : General Failure

Return Value:
	SUCCESS        : if the function succeeded
	EBADPARM       : if pszClientPwd or pAEECallback or nStatus is NULL
	EFSFULL		   : File system full
	EFAILED        : general failure

Remarks :
					This method should be the first function called after the extension is loaded
					and before any other functions are call.  If not, the other functions will return
					EUNSUPPORTED.  If an invalid clientID is passed, then status member will contain
					EFAILED_AUTHENTICATION error code when control returns in callback.

					Sample code for ILPSPOSDET_Init() invocation is given below.

					//Declarations

					//Application Data Structure:
					typedef struct _MyApp
					{
						AEEApplet a;
						//application specific parameters
						int nInitializationStatus; //The address to this member is passed as "pnStatus" to
												   //ILPSPOSDET_Init(). This need not be a member of application data
												   //structure but it should be global enough to be valid until
												   //the callback specified by "pAEECallback" gets called.

					    AEECallback	initCallback; //The address to this member is passed as "pAEECallback" to
												   //ILPSPOSDET_Init(). This need not be a member of application data
												   //structure but it should be global enough to be valid until
												   //the registration process is complete.

					   ILpsPosDet*	pILpsPosDet; //Pointer to ILpsPosDet Interface
					}MyApp;

					//Callback function declaration
					void InitCB( void* pUserData );

					//Functions

					//Function calling ILPSPOSDET_Init()
					void InitILpsPosdet(MyApp *pme)
					{
						//Assuming that pme->pILpsPosdet points to an already created instance of ILpsPosDet extension
						int nRet = SUCCESS;

						CALLBACK_Init( &pme->initCallback, (PFNNOTIFY)InitCB, (void*) pme );
						nRet = ILPSPOSDET_Init( pme->pILpsPosdet, 12345678, "Password",
												&pme->initCallback, &pme->nInitializationStatus );
						if( nRet == SUCCESS )
						{
							//ILPSPOSDET_Init() succeeded. Callback will be called
							//action code here
						}
						else
						{
							//ILPSPOSDET_Init() failed. nRet indicates the error code. Callback will not be called
							//action code here
						}
					}

					//Callback function
					void InitCB( void* pUserData )
					{
						ILpsPosDet* pme = (ILpsPosDet*) pUserData;

						if( pme->nInitializationStatus == SUCCESS )
						{
							//Successful registration
							//action code here
						}
						else
						{
							//Registration failed. pme->nInitializationStatus indicates the error code.
							//action code here
						}
					}

see  None

=====================================================================
// This function is supported for BREW version 3.1.3 and above
Function: ILPSPOSDET_ExtractPositionInfo

Description:

   This function generates the position information in the format
   specified by AEEPositionInfoEx.


Prototype:

   int ILPSPOSDET_ExtractPositionInfo(ILpsPosDet *pILpsPosDet,
                                      AEEGPSInfo *piAEEGPSInfo,
									  AEEPositionInfoEx
									  *poAEEPositionInfoEx );


Parameters:
   pILpsPosDet	       : [in]. The interface pointer.
   piAEEGPSInfo	       : [in]. Pointer to AEEGPSInfo data.
   poAEEPositionInfoEx : [out].Output buffer for position information.


Return Value:
   SUCCESS   : if the function succeeded

   EBADPARM  : if pILpsPosDet or piAEEGPSInfo or poAEEPositionInfoEx is
               NULL. or the buffer provided by poAEEPositionInfoEx is not
			   sufficient or if the data or inside piAEEGPSInfo is not
			   valid.
   EUNSUPPORTED : if this function is not supported.
   EFAILED :   general failure


Comments:
   None.

Side Effects:
   None

See Also:

=======================================================================*/


#endif	// #ifndef _LPSPOSDET_H_
