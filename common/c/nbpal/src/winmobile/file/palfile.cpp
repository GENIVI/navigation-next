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

/*--------------------------------------------------------------------------

(C) Copyright 2014 by TeleCommunication Systems, Inc.

The information contained herein is confidential, proprietary
to TeleCommunication Systems, Inc., and considered a trade secret as
defined in section 499C of the penal code of the State of
California. Use of this information by anyone other than
authorized employees of TeleCommunication Systems is granted only
under a written non-disclosure agreement, expressly
prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

#include "fileenum.h"
#include "fullfilename.h"
#include "palfile.h"
#include "NIMFile.h"
#include "paldebug.h"
#include <shlobj.h>
#include <Shlwapi.h>
#include <winbase.h>

const char* BACKDIR = "..";
const char* SUPPORTFILES_DIRECTORY= "supportfiles";
using namespace nimpal::file;


typedef struct PAL_File
{
    NIMFile*    file;
} PAL_File;


typedef struct PAL_FileEnum
{
    FileEnum*   fileEnum;
} PAL_FileEnum;

// File functions ................................................................................

/* See header for description */
PAL_Error PAL_FileLoadFile(PAL_Instance* pal, const char* filename, unsigned char** data, uint32* dataSize)
{
    if (pal == NULL || filename == NULL || data == NULL || dataSize == NULL)
    {
        return PAL_ErrBadParam;
    }

    FullFilename fullFilename(filename, 1);
    NIMFile file(fullFilename.GetWideName(), NFAM_Read);
    if (!file.IsValid())
    {
        return PAL_ErrFileNotExist;
    }

    PAL_Error err = file.Size(dataSize);
	unsigned char* buf = (unsigned char*)nsl_malloc(*dataSize);
	if (buf == NULL)
    {
		return PAL_ErrNoMem;
    }

    err = file.Read(buf, *dataSize, NULL);
    if (err == PAL_Ok)
    {
    	*data = buf;
    }
    else
    {
        *data = NULL;
        nsl_free(buf);
    }

	return err;
}

/* See header for description */
PAL_Error PAL_FileOpen(PAL_Instance* pal, const char *filename, PAL_FileMode mode, PAL_File** palFile)
{
	if (pal == NULL || filename == NULL || palFile == NULL)
    {
		return PAL_ErrBadParam;
    }
    *palFile = NULL;

	NIMFileAccessMode nimMode = NFAM_Invalid;
    switch (mode)
    {
    case PFM_Read:
		nimMode = NFAM_Read;
        break;

    case PFM_ReadWrite:
		nimMode = NFAM_ReadWrite;
        break;

    case PFM_Append:
		nimMode = NFAM_Append;
        break;

    case PFM_Create:
        nimMode = NFAM_Create;
        break;

    default:
		return PAL_ErrBadParam;
	}

	PAL_File* tempPalFile = new PAL_File;
    if (!tempPalFile)
    {
        return PAL_ErrNoMem;
    }

    FullFilename fullFilename(filename, 1);
    tempPalFile->file = new NIMFile(fullFilename.GetWideName(), nimMode);
    if (tempPalFile->file == NULL)
    {
        delete tempPalFile;
        return PAL_ErrNoMem;
    }

    if (!tempPalFile->file->IsValid())
    {
        delete tempPalFile->file;
        delete tempPalFile;
        return PAL_ErrFileNotExist;
    }

    *palFile = tempPalFile;
	return PAL_Ok;
}


/* See header for description */
PAL_Error PAL_FileClose(PAL_File* palFile)
{
    if (palFile == NULL)
    {
        return PAL_ErrBadParam;
    }

	palFile->file->Close();
    delete palFile->file;
    delete palFile;

    return PAL_Ok;
}


/* See header for description */
PAL_Error PAL_FileRemove(PAL_Instance* pal, const char *filename)
{
    if (pal == NULL || filename == NULL)
    {
        return PAL_ErrBadParam;
    }

    FullFilename fullFilename(filename, 1);
    BOOL rc = DeleteFile(fullFilename.GetWideName());
    if (!rc && (GetLastError() != ERROR_FILE_NOT_FOUND))
    {
        return PAL_ErrFileFailed;
    }
    else
    {
        return PAL_Ok;
    }
}


/* See header for description */
PAL_Error PAL_FileRename(PAL_Instance* pal, const char *oldname, const char *newname)
{
    if (pal == NULL || oldname == NULL || newname == NULL)
    {
        return PAL_ErrBadParam;
    }

    FullFilename oldFullFilename(oldname, 1);
    FullFilename newFillFilename(newname, 1);
    BOOL rc = MoveFile(oldFullFilename.GetWideName(), newFillFilename.GetWideName());
    if (rc)
    {
        return PAL_Ok;
    }

    return PAL_ErrFileFailed;
}


/* See header for description */
PAL_Error PAL_FileExists(PAL_Instance* pal, const char *filename)
{
    if (pal == NULL || filename == NULL)
    {
        return PAL_ErrBadParam;
    }

    FullFilename fullFilename(filename, 1);
    DWORD rc = GetFileAttributes(fullFilename.GetWideName());

    return rc != -1 ? PAL_Ok : PAL_ErrFileNotExist;
}


/* See header for description */
PAL_Error PAL_FileGetSize(PAL_Instance* pal, const char* filename, uint32* fileSize)
{
    if (pal == NULL || filename == NULL || fileSize == NULL)
    {
        return PAL_ErrBadParam;
    }

    WIN32_FILE_ATTRIBUTE_DATA data;
    FullFilename fullFilename(filename, 1);
    BOOL rc = GetFileAttributesEx(fullFilename.GetWideName(), GetFileExInfoStandard, &data);
    if (!rc)
    {
        return PAL_ErrFileNotExist;
    }

    *fileSize = data.nFileSizeLow;
	return PAL_Ok;
}


/* See header for description */
uint32 PAL_FileGetFreeSpace(PAL_Instance* pal)
{
    ULARGE_INTEGER available = { 0 };
    BOOL rc = GetDiskFreeSpaceEx(NULL, &available, NULL, NULL);
    if (!rc)
    {
        return 0;
    }
    if (available.u.HighPart != 0)
    {
        return 0xffffffff;
    }

    return available.u.LowPart;
}


/* See header for description */
uint32 PAL_FileGetTotalSpace(PAL_Instance* pal)
{
    ULARGE_INTEGER total = { 0 };
    BOOL rc = GetDiskFreeSpaceEx(NULL, NULL, &total, NULL);
    if (!rc)
    {
        return 0;
    }
    if (total.u.HighPart != 0)
    {
        return 0xffffffff;
    }

    return total.u.LowPart;
}


/* See header for description */
PAL_Error PAL_FileGetPosition(PAL_File* palFile, uint32* position)
{
    if (palFile == NULL || position == NULL)
    {
        return PAL_ErrBadParam;
    }

    return palFile->file->Tell(position);
}


/* See header for description */
PAL_Error PAL_FileSetPosition(PAL_File* palFile, PAL_FileSetOrigin origin, int32 offset)
{
    if (palFile == NULL)
    {
        return PAL_ErrBadParam;
    }

	NIMFileSeekOrigin fileOrigin = NFSO_Invalid;
    switch (origin)
	{
		case PFSO_Current:
			fileOrigin = NFSO_Current;
			break;
		case PFSO_End:
			fileOrigin = NFSO_End;
			break;
		case PFSO_Start:
			fileOrigin = NFSO_Begin;
			break;
        default:
            return PAL_ErrBadParam;
	}

    palFile->file->Seek(offset, fileOrigin);
    return PAL_Ok;
}


/* See header for description */
PAL_Error PAL_FileRead(PAL_File* palFile, byte* buffer, uint32 bufferSize, uint32* bytesRead)
{
    if (palFile == NULL || buffer == NULL)
    {
        return PAL_ErrBadParam;
    }

    return palFile->file->Read(buffer, bufferSize, bytesRead);
}


/* See header for description */
PAL_Error PAL_FileWrite(PAL_File* palFile, const byte* buffer, uint32 bufferSize, uint32* bytesWritten)
{
    if (palFile == NULL || buffer == NULL || bytesWritten == NULL)
    {
        return PAL_ErrBadParam;
    }

	return palFile->file->Write(buffer, bufferSize, bytesWritten);
}


/* See header for description */
PAL_Error PAL_FileTruncate(PAL_File* palFile, uint32 position)
{
    if (palFile == NULL)
    {
        return PAL_ErrBadParam;
    }

    return palFile->file->Truncate(position);
}


/* See header for description */
PAL_Error PAL_FileSetCacheSize(PAL_File* palFile, uint32 cacheSize)
{
    // Not supported, return success
    return PAL_Ok;
}


/* See header for description */
PAL_Error PAL_FileGetAttributes(PAL_Instance* pal, const char* filename, uint32* attributes)
{
    if (pal == NULL || filename == NULL || attributes == NULL)
    {
        return PAL_ErrBadParam;
    }
    *attributes = 0;

    FullFilename fullFilename(filename, 1);
    DWORD rc = GetFileAttributes(fullFilename.GetWideName());
    if (rc == -1)
    {
        return PAL_ErrFileNotExist;
    }

    *attributes = FileEnum::MapWinFileAttributesToPal(rc);

    return PAL_Ok;
}


// Directory/Path functions ......................................................................

/* See header for description */
uint8 PAL_FileIsDirectory(PAL_Instance* pal, const char* directoryName)
{
    if (pal == NULL || directoryName == NULL)
    {
        return 0;
    }

    FullFilename fullFilename(directoryName, 0);
    DWORD rc = GetFileAttributes(fullFilename.GetWideName());
    if (rc == -1)
    {
        return 0;
    }

    return (rc & FILE_ATTRIBUTE_DIRECTORY) != 0;
}


/* See header for description */
PAL_Error PAL_FileCreateDirectory(PAL_Instance* pal, const char* directoryName)
{
    if (pal == NULL || directoryName == NULL)
    {
        return PAL_ErrBadParam;
    }

    FullFilename fullFilename(directoryName, 0);
    BOOL rc = CreateDirectory(fullFilename.GetWideName(), 0);
    if (!rc)
    {
        return PAL_ErrFileFailed;
    }

	return PAL_Ok;
}


/* See header for description */
PAL_Error
PAL_FileCreateDirectoryEx(PAL_Instance* pal, const char* directoryName)
{
#ifndef WINCE
    if (!pal || !directoryName)
    {
        return PAL_ErrBadParam;
    }

    FullFilename fullFilename(directoryName, 0);

    // Create all directories included in the path
    int shellResult = SHCreateDirectoryExA(NULL, fullFilename.GetName(), NULL);

    // We consider it successful if all the directories already exist
    if ((shellResult == ERROR_ALREADY_EXISTS) ||
        (shellResult == ERROR_FILE_EXISTS) ||
        (shellResult == ERROR_SUCCESS))
    {
        return PAL_Ok;
    }
    else
    {
        return PAL_ErrFileFailed;
    }
#else // Win Mobile
    // SHCreateDirectoryExA() is not supported in WM.
    // todo: add implementation for WM
    return PAL_ErrUnsupported;
#endif
}


/* See header for description */
PAL_Error PAL_FileRemoveDirectory(PAL_Instance* pal, const char* directoryName, uint8 recursive)
{
    if (pal == NULL || directoryName == NULL)
    {
        return PAL_ErrBadParam;
    }

    FullFilename fullFilename(directoryName, 0);

    if (recursive)
    {
        FileEnum files(fullFilename.GetWideName(), 0);
        PAL_FileEnumInfo fileInfo;
        PAL_Error err = files.Next(&fileInfo);
        while (!err)
        {
            char filename[PAL_FILE_MAX_NAME_LENGTH];
            nsl_strlcpy(filename, directoryName, sizeof(filename));
            PAL_FileAppendPath(pal, filename, PAL_FILE_MAX_NAME_LENGTH, fileInfo.filename);

            if ((fileInfo.attributes & PAL_FILE_ATTRIBUTE_DIRECTORY) != 0)
            {
                PAL_FileRemoveDirectory(pal, filename, recursive);
            }
            else
            {
                err = PAL_FileRemove(pal, filename);
            }

            if (!err)
            {
                err = files.Next(&fileInfo);
            }
        }

        if (err != PAL_Ok && err != PAL_ErrNoData)
        {
            return err;
        }
    }

    BOOL rc = RemoveDirectory(fullFilename.GetWideName());
    if (!rc)
    {
        return PAL_ErrFileFailed;
    }

	return PAL_Ok;
}


/* See header for description */
PAL_Error
PAL_FileAppendPath(PAL_Instance* pal, char* path, uint32 maxPathLength, const char* additionalPath)
{
#ifndef WINCE
    if (!pal || !path || !additionalPath)
    {
        return PAL_ErrBadParam;
    }

    // Check char buffer size. Add 2 for delimiter and NULL termination
    if ((nsl_strlen(path) + nsl_strlen(additionalPath) + 2) > maxPathLength)
    {
        return PAL_ErrRange;
    }

    // Use OS function to do the path manipulation. Add path or file
    return (PathAppendA(path, additionalPath) ? PAL_Ok : PAL_Failed);
#else // Win Mobile
    // PathAppendA() is not supported in WM.
    // todo: add implementation for WM
    return PAL_ErrUnsupported;
#endif
}


/* See header for description */
PAL_Error
PAL_FileRemovePath(PAL_Instance* pal, char* path)
{
#ifndef WINCE
    if (!pal || !path)
    {
        return PAL_ErrBadParam;
    }

    // Use OS function to do the path manipulation. Remove last subdirectory or file
    return (PathRemoveFileSpecA(path) ? PAL_Ok : PAL_Failed);
#else // Win Mobile
    // PathRemoveFileSpecA() is not supported in WM.
    // todo: add implementation for WM
    return PAL_ErrUnsupported;
#endif
}


/* See header for description */
PAL_Error
PAL_FileGetLastPath(PAL_Instance* pal, const char* fullPath, char** lastPath)
{
    PAL_Error error = PAL_Ok;
    char* pathWithoutLastDelimiter = NULL;
    const char* last = NULL;
    const char* current = NULL;

    if (!pal || !fullPath || (nsl_strlen(fullPath) == 0) || !lastPath)
    {
        return PAL_ErrBadParam;
    }

    // Find the last PATH_DELIMITER
    current = nsl_strrchr(fullPath, PATH_DELIMITER);

    if (current)
    {
        // PATH_DELIMITER isn't the last character
        if (nsl_strlen(current) > 1)
        {
            ++current;
        }
        // Argument fullPath equal to PATH_DELIMITER
        else if (current == fullPath)
        {
            return PAL_ErrBadParam;
        }
        // PATH_DELIMITER is the last character
        else
        {
            pathWithoutLastDelimiter = nsl_strdup(fullPath);
            if (!pathWithoutLastDelimiter)
            {
                return PAL_ErrNoMem;
            }
            // Remove the last character PATH_DELIMITER
            pathWithoutLastDelimiter[nsl_strlen(pathWithoutLastDelimiter) - 1] = '\0';
            // Find the last PATH_DELIMITER
            last = nsl_strrchr(pathWithoutLastDelimiter, PATH_DELIMITER);
            if (last)
            {
                // PATH_DELIMITER is the last character. Argument fullPath is end of "//".
                if (nsl_strlen(last) <= 1)
                {
                    nsl_free(pathWithoutLastDelimiter);
                    return PAL_ErrBadParam;
                }

                current = last + 1;
            }
            else
            {
                current = pathWithoutLastDelimiter;
            }
        }
    }
    else
    {
        current = fullPath;
    }

    *lastPath = nsl_strdup(current);
    if (!(*lastPath))
    {
        error = PAL_ErrNoMem;
    }

    if (pathWithoutLastDelimiter)
    {
        nsl_free(pathWithoutLastDelimiter);
    }

    return error;
}


// File enumeration functions ....................................................................

/* See header for description */
PAL_Error PAL_FileEnumerateCreate(PAL_Instance* pal, const char* directoryName, uint8 filesOnly, PAL_FileEnum** fileEnum)
{
    if (pal == NULL || directoryName == NULL || fileEnum == NULL)
    {
        return PAL_ErrBadParam;
    }

    PAL_FileEnum* temp = new PAL_FileEnum;
    if (temp == NULL)
    {
        return PAL_ErrNoMem;
    }

    FullFilename fullFilename(directoryName, 0);
    temp->fileEnum = new FileEnum(fullFilename.GetWideName(), filesOnly);
    if (temp->fileEnum == NULL)
    {
        delete temp;
        return PAL_ErrNoMem;
    }

    *fileEnum = temp;

	return PAL_Ok;
}


/* See header for description */
PAL_Error PAL_FileEnumerateNext(PAL_FileEnum* fileEnum, PAL_FileEnumInfo* fileInformation)
{
    if (fileEnum == NULL || fileInformation == NULL)
    {
        return PAL_ErrBadParam;
    }

	return fileEnum->fileEnum->Next(fileInformation);
}


/* See header for description */
PAL_Error PAL_FileEnumerateDestroy(PAL_FileEnum* fileEnum)
{
    if (fileEnum == NULL)
    {
        return PAL_ErrBadParam;
    }

    delete fileEnum->fileEnum;
    delete fileEnum;

    return PAL_Ok;
}

/* See header file for description */
PAL_DEF PAL_Error
PAL_FileFlush(PAL_File* palFile)
{
    if (!palFile)
    {
        return PAL_ErrBadParam;
    }

    if (!(palFile->file))
    {
        return PAL_ErrFileNotOpen;
    }

    return palFile->file->Flush();
}

/* See header for description */
const char* PAL_FileGetHomeDirectory()
{
    return PAL_FileGetWorkPath();
}

/* See header for description */
const char* PAL_FileGetBundleDirectory()
{
    return PAL_FileGetApplicationPath();
}

/* See header for description */
const char* PAL_FileGetApplicationPath()
{
    static char currentPath[FILENAME_MAX];

   ::GetModuleFileNameA(NULL, currentPath, FILENAME_MAX);
    {
        // Remove the executable name from the path
        char* delimiter = strrchr(currentPath, PATH_DELIMITER);
        if (delimiter != 0)
        {
            *++delimiter = '\0';
        }
    }

   return currentPath;
}

/* See header for description */
const char* PAL_FileGetWorkPath()
{
   // all paths are the same for win32
   return PAL_FileGetApplicationPath();
}

/* See header for description */
const char* PAL_FileGetCachePath()
{
   // all paths are the same for win32
   return PAL_FileGetApplicationPath();
}

void PAL_FileFullPath(char* fullpath, int maxlen, const char* filename)
{
    // TODO: This should be changed in the future basing on different platforms.
    // Currently, because Android and W32 platform do not use this function, so just copy filenae to fullpath
    nsl_strncpy(fullpath, filename, maxlen);
}
    
#ifdef __cplusplus
 }
#endif
