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

    @file     palunzip.cpp
    @defgroup PALUNZIP_H PAL UnZip Functions
*/
/*
    See file description in header file.

    (C) Copyright 2011 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

#include "palunzip.h"
#include "palfile.h"
#include "palstdlib.h"
#include "unzip.h"
#include <zlib.h>

#define ZIP_PATH_DELIMITER '/'
#define FILE_IN_ARCHIVE_MAX_NAME_LENGTH 256

/* See description below. */
static PAL_Error GetFileName(PAL_File* h, char* fileName, uint16& fileNameLength, const int32 adj);

/* See description below. */
static nb_boolean IsDirectory(const char* fileName);

/* See description below. */
static PAL_Error AddDestinationToPath(const char* destination, char* fileName, uint16& fileNameLength);

/* See header file for description. */
PAL_DEF PAL_Error
PAL_FileUnzip(PAL_Instance *pal, const char* zipFileName, const char* destination)
{
    CUnzip unzipInstance;
    PAL_Error err = PAL_Ok;
    PAL_File* palFileInput = NULL;
    PAL_File* palFileOutput = NULL;
    uint8* pBuffer = NULL;
    char fileName[FILE_IN_ARCHIVE_MAX_NAME_LENGTH] = {0};
    uint32 bytesWritten = 0;
    int32 offset = 0;
    uint16 fileNameLength = 0;

    if (NULL == pal || NULL == zipFileName || NULL == destination)
    {
        return PAL_ErrBadParam;
    }

    err = PAL_FileOpen(pal, zipFileName, PFM_Read, &palFileInput);
    if (err != PAL_Ok)
    {
        return err;
    }

    err = PAL_FileCreateDirectoryEx(pal, destination);
    if (err != PAL_Ok)
    {
        goto closeFile;
    }

    while ((pBuffer = unzipInstance.Unzip(palFileInput, offset)) != NULL)
    {
        nsl_memset(fileName, 0, FILE_IN_ARCHIVE_MAX_NAME_LENGTH); // to update fileName we initialize it with 0 first
        err = GetFileName(palFileInput, fileName, fileNameLength, offset);
        if (err != PAL_Ok)
        {
            goto closeFile;
        }
        err = AddDestinationToPath(destination, fileName, fileNameLength);
        if (err != PAL_Ok)
        {
            goto closeFile;
        }

        if (!IsDirectory(fileName))
        {
            // Find file' directory from zip member filename.
            // Some zip files (i.e.USA-VEG.zip does not contain directory name as first member in zip arch.
            // So we need to check whether target directory exists.
            char    *p = fileName;
            p = nsl_strrchr( fileName, '/' );
            if( p ) // filename contains directory
            {
                char tmpDir[FILE_IN_ARCHIVE_MAX_NAME_LENGTH];
                nsl_strlcpy( tmpDir, fileName, p-fileName+1 );
                tmpDir[p-fileName+1] = 0;
                err = PAL_FileCreateDirectoryEx( pal, tmpDir );
                if (err != PAL_Ok)
                {
                    goto closeFile;
                }
            }

            // If it is not a directory then we create a new file in the appropriate folder
            // and write to this file the contents of the pBuffer.
            err = PAL_FileOpen(pal, fileName, PFM_Create, &palFileOutput);
            if (err != PAL_Ok)
            {
                goto closeFile;
            }
            err = PAL_FileWrite(palFileOutput, pBuffer, unzipInstance.GetCounter(), &bytesWritten);
            if (err != PAL_Ok)
            {
                PAL_FileClose(palFileOutput);
                goto closeFile;
            }
            else
            {
                PAL_FileClose(palFileOutput);
            }
        }
        else // if (!IsDirectory(fileName))
        {
            // If fileName is the name of a directory then we create a new directory in the appropriate folder
            err = PAL_FileCreateDirectoryEx(pal, fileName);
            if (err != PAL_Ok)
            {
                goto closeFile;
            }
        }
    }

closeFile:
    if (palFileInput != NULL)
    {
        PAL_FileClose(palFileInput);
    }
    else
    {
        return PAL_Failed;
    }

    return err;
}


/*! Gets the name of the processed file from ZIP archive

This function gets the name of the currently processed file from ZIP archive.

@param fileName Name of the file from ZIP archive
@param fileNameLength Length of the fileName
@param offset Number of bytes we've read with CUnzip::Unzip() function.
              We need this because to read the name of the file
              we have to move the file-position indicator in a stream
              back to the beginning of our file. To learn more see ZIP specification.

@see CUnzip::Unzip()

*/
static PAL_Error GetFileName(PAL_File* h, char* fileName, uint16& fileNameLength, const int32 offset)
{
    LF fileHeader;
    uint32 bytesRead = 0;
    uint8* tempFileHeader = NULL;
    uint8* tempFileName = NULL;

    tempFileHeader = static_cast<uint8*>(nsl_malloc(sizeof(fileHeader)));
    if (NULL == tempFileHeader)
    {
        return PAL_Failed;
    }
    nsl_memset(tempFileHeader, 0, sizeof(fileHeader));

    PAL_FileSetPosition(h, PFSO_Current, -offset);

    PAL_FileRead(h, tempFileHeader, sizeof(fileHeader), &bytesRead);
    nsl_memcpy(&fileHeader, tempFileHeader, sizeof(fileHeader));
    nsl_free(tempFileHeader);

    tempFileName = static_cast<uint8*>(nsl_malloc(fileHeader.lfFilenameLength));
    if (NULL == tempFileName)
    {
        return PAL_Failed;
    }
    nsl_memset(tempFileName, 0, fileHeader.lfFilenameLength);

    PAL_FileRead(h, tempFileName, fileHeader.lfFilenameLength, &bytesRead);
    nsl_memcpy(fileName, tempFileName, fileHeader.lfFilenameLength);
    nsl_free(tempFileName);

    PAL_FileSetPosition(h, PFSO_Current, offset - fileHeader.lfFilenameLength - sizeof(fileHeader));

    fileNameLength = fileHeader.lfFilenameLength;

    return PAL_Ok;
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
    uLongf  tempSize = (uLongf)maxDestinationSize;

    if ((!source) || (sourceSize == 0) || (!destination) || (!decompressedSize))
    {
        return PAL_ErrBadParam;
    }

    // Decompress the data using zlib.
    decompressedError = uncompress((Bytef*) destination,
                                   &tempSize,
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


/*! Checks if the object specified in const char* fileName is directory.

If the file is a directory (folder) then it has '/' at the end of its name.
So IsDirectory() basically checks if this symbol is present or not.

@return true If it is a directory and false if it is not

@param fileName Name of the file which has to be checked

@see GetFileName()

*/
static nb_boolean IsDirectory(const char* fileName)
{
    int length = 0;

    if (fileName != NULL)
    {
        length = nsl_strlen(fileName);
    }
    else
    {
        return FALSE;
    }

    if (*(fileName + length - 1) == ZIP_PATH_DELIMITER)
    {
        return TRUE;
    }
    return FALSE;
}


/*! Adds the name of the destination folder to the beginning of the fileName.

To be able to write all our extracted files to the destination folder we should
add its name at the beginning of the name of the file.

@param destination Name of the destination folder
@param fileName Name of the file
@param fileNameLength Length of the fileName

*/
static PAL_Error AddDestinationToPath(const char* destination, char* fileName, uint16& fileNameLength)
{
    int length = 0;
    char temp[FILE_IN_ARCHIVE_MAX_NAME_LENGTH] = {0};

    if (NULL == destination || NULL == fileName)
    {
        return PAL_ErrBadParam;
    }

    length  = nsl_strlen(destination);
    nsl_strlcpy(temp, destination, FILE_IN_ARCHIVE_MAX_NAME_LENGTH);
    nsl_strlcat(temp, "/", FILE_IN_ARCHIVE_MAX_NAME_LENGTH);
    nsl_strlcat(temp, fileName, FILE_IN_ARCHIVE_MAX_NAME_LENGTH);
    nsl_strlcpy(fileName, temp, FILE_IN_ARCHIVE_MAX_NAME_LENGTH);
    fileNameLength += length + 1;

    return PAL_Ok;
}


/*! @} */
