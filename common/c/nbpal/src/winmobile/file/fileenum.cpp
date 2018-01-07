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

@defgroup FILEENUM_H Helper class to enumerate files

Helper class to enumerate files in a directory

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

#include "fileenum.h"
#include "palstdlib.h"

namespace
{
    static const int ATTR_MAP_WIN32_IDX = 0;
    static const int ATTR_MAP_PAL_IDX = 1;

    int attributeMapping[][2] = {
        { FILE_ATTRIBUTE_DIRECTORY, PAL_FILE_ATTRIBUTE_DIRECTORY },
        { FILE_ATTRIBUTE_HIDDEN, PAL_FILE_ATTRIBUTE_HIDDEN },
        { FILE_ATTRIBUTE_NORMAL, PAL_FILE_ATTRIBUTE_NORMAL },
        { FILE_ATTRIBUTE_READONLY, PAL_FILE_ATTRIBUTE_READONLY },
        { FILE_ATTRIBUTE_SYSTEM, PAL_FILE_ATTRIBUTE_SYSTEM },
        { 0, 0}
    };
}

namespace nimpal
{
    namespace file
    {
        FileEnum::FileEnum(const wchar_t* dirname, uint8 filesOnly) :
            m_filesOnly(filesOnly)
        {
            memset(&m_findData, 0, sizeof(WIN32_FIND_DATA));

            // Append a wildcard to the end of the directory name
            wchar_t pattern[PAL_FILE_MAX_NAME_LENGTH + 1];
            wcsncpy(pattern, dirname, PAL_FILE_MAX_NAME_LENGTH);
            if (pattern[wcslen(pattern)] != '\\')
            {
                wcscat(pattern, L"\\*");
            }
            else
            {
                wcscat(pattern, L"*");
            }

            m_handle = FindFirstFile(pattern, &m_findData);
            m_haveNext = (m_handle != INVALID_HANDLE_VALUE);
            if (!NextIsValidMatch())
            {
                AdvanceEnum();
            }
        }

        FileEnum::~FileEnum()
        {
            if (m_handle != INVALID_HANDLE_VALUE)
            {
                FindClose(m_handle);
            }
        }


        PAL_Error FileEnum::Next(PAL_FileEnumInfo* info)
        {
            if (info == NULL)
            {
                return PAL_ErrBadParam;
            }
            if (!m_haveNext)
            {
                return PAL_ErrNoData;
            }

            WideCharToMultiByte(CP_UTF8, 0, m_findData.cFileName, -1, info->filename, sizeof(info->filename), NULL, NULL);
            info->size = m_findData.nFileSizeLow;
            info->attributes = MapWinFileAttributesToPal(m_findData.dwFileAttributes);

            AdvanceEnum();

            return PAL_Ok;
        }


        /*! Advance to next valid match */
        void FileEnum::AdvanceEnum()
        {
            do
            {
                m_haveNext = FindNextFile(m_handle, &m_findData) != 0;
            }
            while (!NextIsValidMatch());
        }


        /*! See if the next info matches the request

        Returns true if 1) there are no more elements, 2) not filesOnly and
        not the current or parent directory, or 3) filesOnly and current is
        not a directory
        */
        bool FileEnum::NextIsValidMatch()
        {
            if (!m_haveNext)
            {
                return true;
            }
            if (!m_filesOnly)
            {
                return wcscmp(m_findData.cFileName, L".") != 0 && wcscmp(m_findData.cFileName, L"..") != 0;
            }

            return (m_findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
        }


        int FileEnum::MapWinFileAttributesToPal(int win32Attributes)
        {
            int palAttributes = 0;

            for (int i = 0; ; i++)
            {
                if (attributeMapping[i][ATTR_MAP_WIN32_IDX] == 0)
                {
                    break;
                }

                if (win32Attributes & attributeMapping[i][ATTR_MAP_WIN32_IDX])
                {
                    palAttributes |= attributeMapping[i][ATTR_MAP_PAL_IDX];
                }
            }

            return palAttributes;
        }
    }
}

/*! @} */
