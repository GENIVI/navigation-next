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

    @file     cslaes.h
*/
/*
    (C) Copyright 2011 by TeleCommunications Systems, Inc.
    (C) Copyright 2002 by George Anescu. Ms-PL license.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems, Inc. is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#ifndef __CSLAES_H__
#define __CSLAES_H__

#include "paltypes.h"
#include "pal.h"
#include "palstdlib.h"
#include "cslexp.h"
#include "nberror.h"

/* This is the Advanced Encrypting Standard (AES) implementation */

/*!
    @addtogroup cslaes
    @{
*/


// Public Functions ..............................................................................

/*! Encrypt passed data string by AES algorithm

    AES operates 16 byte blocks, so, if input string isn't multiply of 16, it will be expanded (internally)
    and lenght of result will be bigger - result length will be placed into 'n'.

    AES allow only 128/192/256 bit key.

    @returns NB_Error
    @see AES_Decrypt
*/
CSL_DEC NB_Error
CSL_AesEncrypt(char const* in,         /*!< Input string - data to encrypt. */
               char* result,           /*!< Output string - encrypted data. */
               int* n,                 /*!< In-out. Size of input data/size of result data (in bytes) */
               char const* key,        /*!< Public key. */
               int keyLength           /*!< Lenght of public key. */
               );

/*! Decrypt passed data string by AES algorithm

    @returns NB_Error
    @see AES_Decrypt
*/
CSL_DEC NB_Error
CSL_AesDecrypt(char const* in,         /*!< Input string - data to encrypt. */
               char* result,           /*!< Output string - encrypted data. */
               int n,                  /*!< Size of input data (in bytes). */
               char const* key,        /*!< Public key. */
               int keyLength           /*!< Lenght of public key. */
               );

/*! @} */

#endif //__CSLAES_H__

