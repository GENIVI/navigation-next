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

#include "NIMFile.h"
#include "palstdlib.h"
#include "windows.h"

namespace nimpal
{
    namespace file
    {
        NIMFile::NIMFile(const wchar_t* filename, NIMFileAccessMode accessMode) :
            m_handle(INVALID_HANDLE_VALUE),
            m_size(0)
        {
            uint32 accessFlags = 0;
            uint32 createFlags = 0;

            PAL_Error err = TranslateAccessMode(accessMode, accessFlags, createFlags);
            if (!err)
            {
                m_handle = CreateFile(filename, accessFlags, 0, NULL, createFlags, FILE_ATTRIBUTE_NORMAL, NULL);
	            if (m_handle != INVALID_HANDLE_VALUE)
	            {
                    m_size = GetFileSize(m_handle, NULL);
                    if (accessMode == NFAM_Append)
                    {
                        SetFilePointer(m_handle, 0, 0, FILE_END);
                    }
	            }
            }
        }

        NIMFile::~NIMFile()
        {
	        Close();
        }


        uint8 NIMFile::IsValid()
        {
            return m_handle == INVALID_HANDLE_VALUE ? 0 : 1;
        }


        PAL_Error NIMFile::Close()
        {
	        if (m_handle != INVALID_HANDLE_VALUE)
            {
		        CloseHandle(m_handle);
                m_handle = INVALID_HANDLE_VALUE;
            }

            return PAL_Ok;
        }

        PAL_Error NIMFile::Flush()
        {
            if (m_handle == INVALID_HANDLE_VALUE)
            {
                return PAL_ErrFileNotOpen;
            }

            BOOL result = FlushFileBuffers(m_handle);

            return (result ? PAL_Ok : PAL_ErrFileFailed);
        }

        PAL_Error NIMFile::Size(uint32* size)
        {
            if (size == NULL)
            {
                return PAL_ErrBadParam;
            }
            if (m_handle == INVALID_HANDLE_VALUE)
            {
                return PAL_ErrFileNotOpen;
            }

	        *size = m_size;
            return PAL_Ok;
        }


        PAL_Error NIMFile::Seek(int32 offset, NIMFileSeekOrigin origin)
        {
            uint32 originFlag;
            switch (origin)
            {
            case NFSO_Begin:
                originFlag = FILE_BEGIN;
                break;
            case NFSO_Current:
                originFlag = FILE_CURRENT;
                break;
            case NFSO_End:
                originFlag = FILE_END;
                break;
            default:
                return PAL_ErrBadParam;
            }

            if (m_handle == INVALID_HANDLE_VALUE)
            {
                return PAL_ErrFileNotOpen;
            }

            DWORD rc = SetFilePointer(m_handle, offset, NULL, originFlag);
            return rc == INVALID_SET_FILE_POINTER ? PAL_ErrFileFailed : PAL_Ok;
        }


        PAL_Error NIMFile::Tell(uint32 *position)
        {
            if (position == NULL)
            {
                return PAL_ErrBadParam;
            }
            if (m_handle == INVALID_HANDLE_VALUE)
            {
                return PAL_ErrFileNotOpen;
            }

            *position = SetFilePointer(m_handle, 0, NULL, FILE_CURRENT);
            return PAL_Ok;
        }


        PAL_Error NIMFile::Read(uint8* buffer, uint32 bufferSize, uint32* bytesRead)
        {
            if (buffer == NULL)
            {
                return PAL_ErrBadParam;
            }
            if (m_handle == INVALID_HANDLE_VALUE)
            {
                return PAL_ErrFileNotOpen;
            }

            DWORD read = 0;
            BOOL rc = ReadFile(m_handle, buffer, bufferSize, &read, NULL);
            if (bytesRead != NULL)
            {
                *bytesRead = read;
            }

            return rc ? PAL_Ok : PAL_ErrFileFailed;
        }


        PAL_Error NIMFile::Write(const uint8* buffer, uint32 bufferSize, uint32* bytesWritten)
        {
            if (buffer == NULL)
            {
                return PAL_ErrBadParam;
            }
            if (m_handle == INVALID_HANDLE_VALUE)
            {
                return PAL_ErrFileNotOpen;
            }

            DWORD written = 0;
            BOOL rc = WriteFile(m_handle, buffer, bufferSize, &written, NULL);
            if (bytesWritten)
            {
                *bytesWritten = written;
            }

            return rc ? PAL_Ok : PAL_ErrFileFailed;
        }


        PAL_Error NIMFile::Truncate(uint32 position)
        {
            if (m_handle == INVALID_HANDLE_VALUE)
            {
                return PAL_ErrFileNotOpen;
            }

            PAL_Error err = Seek(position, NFSO_Begin);
            if (!err)
            {
                BOOL rc = SetEndOfFile(m_handle);
                if (!rc)
                {
                    err = PAL_ErrFileFailed;
                }
            }

	        return err;
        }

        //PAL_Error NIMFile::Flush()
        //{
        //    if (m_handle == INVALID_HANDLE_VALUE)
        //    {
        //        return PAL_ErrFileNotOpen;
        //    }

        //    BOOL rc = FlushFileBuffers(m_handle);
        //    if (!rc)
        //    {
        //        return PAL_ErrFileFailed;
        //    }

        //    return PAL_Ok;
        //}


        PAL_Error NIMFile::TranslateAccessMode(NIMFileAccessMode accessMode, uint32& accessFlags, uint32& createFlags)
        {
            PAL_Error err = PAL_Ok;

            switch (accessMode)
            {
            case NFAM_Read:
                accessFlags = GENERIC_READ;
                createFlags = OPEN_EXISTING;
                break;

            case NFAM_ReadWrite:
                accessFlags = GENERIC_READ | GENERIC_WRITE;
                createFlags = OPEN_EXISTING;
                break;

            case NFAM_Append:
                accessFlags = GENERIC_READ | GENERIC_WRITE;
                createFlags = OPEN_ALWAYS;
                break;

            case NFAM_Create:
                accessFlags = GENERIC_READ | GENERIC_WRITE;
                createFlags = CREATE_ALWAYS;
                break;

            default:
	            accessFlags = 0;
                createFlags = 0;
		        err = PAL_ErrBadParam;
	        }

            return err;
        }
    }
}

/*! @} */
