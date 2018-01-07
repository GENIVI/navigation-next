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
* Copyright (c) 2008 Networks In Motion, Inc.
* All rights reserved. This file and associated materials are the
* trade secrets, confidential information, and copyrighted works of
* Networks In Motion, Inc.
*
* This intellectual property is for the internal use only by Networks
* In Motion, Inc. This source code contains proprietary information
* of Networks In Motion, Inc., and shall not be used, copied by, or
* disclosed to, anyone other than a Networks In Motion, Inc.,
* certified employee that has written authorization to view or modify
* said source code.
*/

#include "paltypes.h"
#include "palstdlib.h"
#include "palimpl.h"
#import <Foundation/Foundation.h>

PAL_DEF PAL_Error
PAL_GenerateUUID(PAL_Instance* pal, uint8* buffer, uint32 bufferSize, uint32* bytesRead)
{
    if(buffer)
    {
        CFUUIDRef puuid = CFUUIDCreate(nil);
        CFUUIDBytes bytes = CFUUIDGetUUIDBytes(puuid);
        NSData* data = [NSData dataWithBytes:&bytes length:sizeof(bytes)];
        if(data.length > bufferSize)
        {
            nsl_memcpy(buffer, data.bytes, bufferSize);
        }
        else
        {
            nsl_memcpy(buffer, data.bytes, data.length);
        }

        CFRelease(puuid);
        *bytesRead = data.length;
        return PAL_Ok;
    }
    return PAL_Failed;
}