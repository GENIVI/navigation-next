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

@file     nimfile.h

Windows Mobile file implementation

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

#ifndef NIMFILE_H__
#define NIMFILE_H__

#include <stdio.h>
#include "palerror.h"
#include "paltypes.h"
#include "windows.h"

namespace nimpal
{
    namespace file
    {
        typedef enum
        {
            NFAM_Invalid,
            NFAM_Read,
            NFAM_ReadWrite,
            NFAM_Create,
            NFAM_Append
        } NIMFileAccessMode;

        typedef enum
        {
            NFSO_Invalid,
            NFSO_Begin,
            NFSO_Current,
            NFSO_End
        } NIMFileSeekOrigin;

        class NIMFile
        {
        public:
            NIMFile(const wchar_t* filename, NIMFileAccessMode accessMode);
            ~NIMFile();

            PAL_Error Size(uint32* size);
            PAL_Error Close();
            PAL_Error Flush();
            PAL_Error Seek(int32 offset, NIMFileSeekOrigin origin);
            PAL_Error Tell(uint32 *position);
            PAL_Error Read(uint8* buffer, uint32 bufferSize, uint32* bytesRead);
            PAL_Error Write(const uint8 *buffer, uint32 bufferSize, uint32* bytesWritten);
            PAL_Error Truncate(uint32 position);

            /*!  Check to see if the file object is valid */
            uint8       IsValid();

        private:
            PAL_Error TranslateAccessMode(NIMFileAccessMode accessMode, uint32& accessFlags, uint32& createFlags);

            HANDLE m_handle;
            size_t m_size;
        };
    }
}

#endif 

/*! @} */
