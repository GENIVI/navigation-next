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

@file palunzip.c
@defgroup PALUNZIP_H PAL UnZip Functions

@brief Provides a unzip utility to extract ZIP archive
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

#ifdef __cplusplus
extern "C" {
#endif

#include "palunzip.h"
#include "palfile.h"
#include "unzip.h"
#include "fullfilename.h"

#define FILE_IN_ZIP_MAX_NAME_LENGTH  1024
#define BUFFER_SIZE                 16384
#define PATH_DELIMITER_UNZ            '/'
#define PATH_DELIMITER_CODE          '\\'

using namespace nimpal::file;

PAL_DEF PAL_Error
PAL_FileUnzip(PAL_Instance *pal, const char* zipFileName, const char* destination)
{
    unzFile pFile = NULL;
    PAL_Error err = PAL_Ok;
    int unzipResult = UNZ_OK;
    char fullPathNameInZip[FILE_IN_ZIP_MAX_NAME_LENGTH] = {0};
    char destinationPathNameInZip[FILE_IN_ZIP_MAX_NAME_LENGTH] = {0};
    char dirInZip[FILE_IN_ZIP_MAX_NAME_LENGTH] = {0};
    unz_file_info fileInfo = {0};
    PAL_File * palFile = NULL;
    uint8 *buffer = 0;
    uint32 bytesWritten = 0;
    char *pChar = NULL;


    if (pal == NULL || zipFileName == NULL || destination == NULL)
    {
        return PAL_ErrBadParam;
    }

    FullFilename fullFilename(zipFileName, 1);

    pFile = unzOpen(fullFilename.GetName());
    if (pFile == NULL)
    {
        return PAL_Failed;
    }

    unzipResult = unzGoToFirstFile(pFile);
    if (unzipResult != UNZ_OK)
    {
        err = PAL_Failed;
        goto closeFile;
    }

    buffer = (uint8 *)nsl_malloc(BUFFER_SIZE);

    if (buffer == NULL)
    {
        err = PAL_ErrNoMem;
        goto closeFile;
    }

    do
    {
        if (unzipResult == UNZ_OK)
        {
            unzipResult = unzGetCurrentFileInfo(pFile, &fileInfo, fullPathNameInZip, sizeof(fullPathNameInZip), NULL, 0, NULL, 0);
            if (unzipResult != UNZ_OK)
            {
                err = PAL_Failed;
                goto closeFile;
            }

            unzipResult = unzOpenCurrentFilePassword(pFile, NULL);
            if (unzipResult != UNZ_OK)
            {
                err = PAL_Failed;
                goto closeFile;
            }
            nsl_strlcpy(destinationPathNameInZip, destination, sizeof(destinationPathNameInZip));
            PAL_FileAppendPath(pal, destinationPathNameInZip, sizeof(destinationPathNameInZip), fullPathNameInZip);

            //update path delimiters

            pChar = destinationPathNameInZip;
            while(*pChar)
            {
                if(*pChar == PATH_DELIMITER_UNZ)
                {
                    *pChar = PATH_DELIMITER_CODE;
                }
                ++pChar;
            }

            nsl_strlcpy(dirInZip, destinationPathNameInZip, sizeof(dirInZip));
            PAL_FileRemovePath(pal, dirInZip);

            if (nsl_strlen(dirInZip) > 0)
            {
                err = PAL_FileCreateDirectoryEx(pal, dirInZip);

                if (err != PAL_Ok)
                {
                    goto closeFile;
                }
            }

            if (!PAL_FileIsDirectory(pal, destinationPathNameInZip))
            {
                err = PAL_FileOpen(pal, destinationPathNameInZip, PFM_Create, &palFile);

                if (err != PAL_Ok)
                {
                   goto closeFile;
                }

                int readSize = 0;
                while ((readSize = unzReadCurrentFile(pFile, buffer, BUFFER_SIZE)) > 0)
                {
                    err = PAL_FileWrite(palFile, buffer, readSize, &bytesWritten);
                    if (err != PAL_Ok)
                    {
                        PAL_FileClose(palFile);
                        goto closeFile;
                    }
                }

                err = PAL_FileClose(palFile);
                if (err != PAL_Ok)
                {
                    goto closeFile;
                }

                unzipResult =  unzCloseCurrentFile(pFile);
                if (unzipResult != UNZ_OK)
                {
                    err = PAL_Failed;
                    goto closeFile;
                }
            }
        }

        unzipResult = unzGoToNextFile(pFile);
    }
    // exit from loop if unz == UNZ_END_OF_LIST_OF_FILE
    while (unzipResult == UNZ_OK);

closeFile:
    if (buffer)
    {
        nsl_free(buffer);
    }

    if (pFile != NULL)
    {
        unzipResult = unzClose(pFile);
        if (unzipResult != UNZ_OK)
        {
            err = PAL_Failed;
        }
    }

return err;
}

/* See header file for description */
PAL_DEF PAL_Error
PAL_DecompressData(const uint8* source,
                   uint32 sourceSize,
                   uint8* destination,
                   uint32 maxDestinationSize,
                   uint32* decompressedSize)
{
    PAL_Error palError = PAL_Ok;
    int decompressedError = Z_OK;
    uint32 tempSize = maxDestinationSize;

    if ((!source) || (sourceSize == 0) || (!destination) || (!decompressedSize))
    {
        return PAL_ErrBadParam;
    }

    // Decompress the data using zlib.
    decompressedError = uncompress((Bytef*) destination,
                                   (uLongf*)&tempSize,
                                   (const Bytef*) source,
                                   (uLong)sourceSize);
    switch (decompressedError)
    {
        case Z_OK:
        {
            palError = PAL_Ok;
            *decompressedSize = tempSize;
            break;
        }
        case Z_MEM_ERROR:
        {
            palError = PAL_ErrNoMem;
            break;
        }
        case Z_BUF_ERROR:
        {
            palError = PAL_ErrRange;
            break;
        }
        case Z_DATA_ERROR:
        default:
        {
            palError = PAL_ErrNoData;
            break;
        }
    }

    return palError;
}

#ifdef __cplusplus
 }
#endif
