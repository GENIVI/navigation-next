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

    @file     networkutil.h
*/
/*
    (C) Copyright 2011 by TeleCommunications Systems, Inc.             

    The information contained herein is confidential, proprietary 
    to TeleCommunication Systems, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of Networks In Motion is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

/*! @{ */

#ifndef _NETWORK_UTIL_H_
#define _NETWORK_UTIL_H_

#include "nbcontext.h"
#include "nbnetwork.h"


/*! Create a PAL and NB Context with specific persistent data

@param pal On success, receives a pointer to a PAL_Instance.  A valid object must be destroyed with PAL_DestroyInstance
@param context On success, receives a pointer to a PAL_Instance.  A valid object must be destroyed with NB_ContextDestroy
@returns Non-zero on success; zero on failure
*/
uint8 CreatePalAndContext(PAL_Instance**pal, NB_Context** context);

/*! Create a PAL instance and a context with a count of HTTP download connection

    @returns Non-zero on success; zero on failure
*/
uint8
CreatePalAndContextWithHttpConnectionCount(uint32 httpConnectionCount,  /*!< Count of connections for the generic HTTP download manager */
                                           PAL_Instance** pal,          /*!< On success, receives a pointer to a PAL_Instance */
                                           NB_Context** context         /*!< On success, receives a pointer to a NB_Context */
                                           );


//----------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------
// copied over from abservices to prevent forward reference for the nbservices unit test

typedef struct AB_NetworkOptionsDetailed
{
    uint64              mdn;                /*!< Mobile Directory Number */
    uint64              min;                /*!< Mobile Identification Number */
    const char*	        adsAppId;           /*!< */
    const char*	        credential;         /*!< Credential to send in iden */
    const char*	        device;             /*!< Device name */
    const char*         hostname;           /*!< Optional hostname; if not specified, hostname is generated from credential */
    const char*         domain;             /*!< Domain name to use with specified or generated hostname */
    const char*	        firmwareVersion;    /*!< Firmware version */
    const char*	        language;           /*!< Language */
    const char*         tpslibData;         /*!< TPS definition to communicate with server */
    const char*         clientGuidData;     /*!< Client GUID assigned by server.  Pass NULL to request a GUID */
    const char*         gwsubid;            /*!< Optional gwsubid to pass in iden */
    const char*         carrier;            /*!< Name of the carrier, e.g. Verizon */
    uint32              classId;            /*!< Class ID */
    uint32              platformId;         /*!< Platform ID */
    uint32              priceType;          /*!< Price Type */
    uint32              tpslibLength;       /*!< Length of the TPS definition data */
    uint32              clientGuidLength;   /*<! Length of the client GUID */
    NB_NetworkProtocol  protocol;           /*!< Protocol used to communicate with server */
    uint16              port;               /*!< Server port to connect to */
    nb_boolean          isEmulator;         /*!< Non-zero if this session is running in an emulator; zero otherwise */
} AB_NetworkOptionsDetailed;

/*! Create a new instance of a NB_NetworkConfiguration object
 
Use this function to create a new instance of a NB_NetworkConfiguration object. Call
AB_NetworkConfigurationDestroy to destroy the object.

@param options A pointer to a network option structure
@param configuration On succeess, the new network configuration object
@return NE_OK for success, NE_NOMEM if memory allocation failed
@see AB_NetworkConfigurationDestroy
*/
NB_Error AB_NetworkConfigurationCreate(AB_NetworkOptionsDetailed* options, NB_NetworkConfiguration** configuration);


/*! Destroy an instance of a NB_NetworkConfiguration object
 
Use this function to destroy an existing instance of a NBI_NetworkConfigurtation object created by
AB_NetworkConfigurationCreate

@param configuration The configuration object to be destroyed
@return NE_OK for success
@see AB_NetworkConficurationCreate
*/
NB_Error AB_NetworkConfigurationDestroy(NB_NetworkConfiguration* configuration);


#endif // _NETWORK_UTIL_H_


/*! @} */
