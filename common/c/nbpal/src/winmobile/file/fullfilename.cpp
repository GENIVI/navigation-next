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

@defgroup FULLFILENAME_H Helper class to construct a full filename

Helper class to construct and maintain a full filename relative
to the application directory

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

#include "fullfilename.h"
#include "palstdlib.h"

const wchar_t DELIMITER = L'\\';
const wchar_t DELIMITER_STRING[] = L"\\";


namespace nimpal
{
    namespace file
    {
        // Static variable
        wchar_t  FullFilename::m_moduleFilename[MAX_PATH] = {0};

        FullFilename::FullFilename(const char* filename, uint8 isFile)
        {
            m_fullFilename[0] = '\0';
            m_wideFullFilename[0] = '\0';

            // Convert to wide-char
            wchar_t wideFilename[MAX_PATH] = {0};
            MultiByteToWideChar(CP_ACP, 0, filename, -1, wideFilename, sizeof(wideFilename) / sizeof(wchar_t));

            // If the filename begins with a backslash or second char is ':'('c:\folder' construction),
            //consider it an absolute path. Otherwise, the path is relative.
            if (wideFilename[0] != DELIMITER && wideFilename[1] != L':')
            {
                // Only get the executable path once
                if (m_moduleFilename[0] == '\0')
                {
                    // get the executable path from the system and convert it to multibyte
                    DWORD filenameLength = GetModuleFileName(NULL, m_moduleFilename, (sizeof(m_moduleFilename) / sizeof(m_moduleFilename[0])));
                    if (filenameLength > 0)
                    {
                        // Remove the executable name from the path
                        wchar_t* delimiter = wcsrchr(m_moduleFilename, DELIMITER);
                        if (delimiter != 0)
                        {
                            *++delimiter = '\0';
                        }
                    }
                }

                wcscpy(m_wideFullFilename, m_moduleFilename);
            }

            // append the name of the file/directory
            wcscat(m_wideFullFilename, wideFilename);

            // append the trailing slash on a directory, if it doesn't exist
            if (isFile == 0 && m_wideFullFilename[wcslen(m_wideFullFilename)] != DELIMITER)
            {
                wcscat(m_wideFullFilename, DELIMITER_STRING);
            }

            // Don't convert string here to multi-byte version. The multibyte version is not used most of the time
            // and we don't want to constantly convert data which we don't use.
        }

        const char* FullFilename::GetName()
        {
            // Convert if not already converted
            if (strlen(m_fullFilename) == 0)
            {
                // convert to multi-byte version of the name too
                WideCharToMultiByte(CP_ACP, 0, m_wideFullFilename, -1, m_fullFilename, sizeof(m_fullFilename), NULL, NULL);
            }

            return m_fullFilename;
        }

        const wchar_t* FullFilename::GetWideName()
        {
            return m_wideFullFilename;
        }
    }
}

/*! @} */
