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

@file pal.h
@defgroup PAL_H PAL

@brief Provides the PAL interface

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

#ifndef PAL_H
#define PAL_H

#include "paltypes.h"
#include "palerror.h"
#include "palconfig.h"
#include "paltestlog.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! External reference for a PAL instance structure

This typedef provides an opaque reference to a created PAL instance.
*/
typedef struct PAL_Instance PAL_Instance;


/*! External reference for a PAL config

This typedef a reference to the PAL config.
*/
typedef struct PAL_Config PAL_Config;


/*! Creates an instance of the PAL struct.

This function creates an instance of the PAL struct using the config parameters supplied by the PAL_Config param

@param config The pal configuration parameters
@return PAL_Instance reference to PAL struct instance
*/
PAL_DEC PAL_Instance* PAL_Create(PAL_Config* config);


/*! Destroys the PAL struct instance.

This function destroys the pal instance and calls the destructor for the PAL_Config

@param pal The pal instance
*/
PAL_DEC void PAL_Destroy(PAL_Instance* pal);


/*! Check to see if running on PAL/Event thread

@param pal The PAL instance
@return Non-zero if the call to this routine is made on the PAL thread; zero, otherwise
*/
PAL_DEC nb_boolean PAL_IsPalThread(PAL_Instance* pal);

/*! Get the pal configuration parameters in use.

@param pal The PAL instance
@param config The pal configuration parameters
@return PAL error code
 */
PAL_DEC PAL_Error PAL_ConfigGet(PAL_Instance* pal, PAL_Config* config);

/*! Set the pal configuration parameters.

This function change the pal configuration parameters in use to the new one

@param pal The PAL instance
@param config The new pal configuration parameters to use
@return PAL error code
 */
PAL_DEC PAL_Error PAL_ConfigSet(PAL_Instance* pal, PAL_Config* config);

/*! Assert if called on non-PAL thread.

This function asserts when called on non-PAL thread.

@param pal The PAL instance
@return Non-zero if the call to this routine is made on the PAL thread; zero, otherwise
 */
PAL_DEC nb_boolean PAL_AssertPalThread(PAL_Instance* pal);

/*! Check to see if running on simulator

@param pal The PAL instance
@return Non-zero if client starts on simulator; zero, otherwise
*/
PAL_DEC nb_boolean PAL_IsSimulator(PAL_Instance* pal);

/*! Check to see if network is blocked

 @param pal The PAL instance
 @return Non-zero if network is blocked; zero, otherwise
 */
PAL_DEC nb_boolean PAL_IsNetworkBlocked(PAL_Instance* pal);

/*! Used to get error code from some functions which can't return error code in standart way

 @param pal The PAL instance
 @return error code from functions which supports this feature
 */
PAL_DEC PAL_Error PAL_GetLastError(PAL_Instance* pal);

/*! Sets error code, called by some functions which can't return error code in standart way

 @param pal The PAL instance
 */
PAL_DEC void PAL_SetLastError(PAL_Instance* pal, PAL_Error err);

void PAL_ScheduleDestroy(PAL_Instance* pal);

/*! Get default locale in System

 @param pal The PAL instance
 @return locale from device
 */
PAL_DEC const char* PAL_GetLocale(PAL_Instance* pal);

/*! Get country code by locale

 @param pal The PAL instance
 @param locale A locale
 @return country code by locale
 */
PAL_DEF const char* PAL_GetISO3CountryCode(PAL_Instance* pal, const char* locale);

/*! Used to set isBackground flag when app is moved to background

 @param pal The PAL instance
 @param isBackground The flag for background state
 */
PAL_DEC void PAL_SetIsBackground(PAL_Instance* pal, nb_boolean isBackground);

/*! Check if app is in background

 @param pal The PAL instance
 @return Non-zero if app is in background; zero if app is in foreground
 */
PAL_DEC nb_boolean PAL_IsBackground(PAL_Instance* pal);

/* Generate a binary UUID(Universally Unique IDentifier) as specified by RFC 4122.

An UUID is 128 bits long, and can guarantee across space and time.

@param pal The PAL instance
@param buffer The buffer to generate data into, this buffer size must bigger than 128 bits
@return PAL_Error error code when generates uuid.
*/
PAL_DEC PAL_Error PAL_GenerateUUID(PAL_Instance* pal, uint8* buffer, uint32 bufferSize, uint32* bytesRead);

/*! Check to see if PAL was created in single-threaded mode

@param pal The PAL instance
@return Non-zero if PAL is in single-threaded mode; zero, otherwise
*/
PAL_DEC nb_boolean PAL_IsSingleThreaded(PAL_Instance* pal);

#ifdef __cplusplus
}
#endif

#endif

/*! @} */
