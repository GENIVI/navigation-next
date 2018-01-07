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

(C) Copyright 2012 by TeleCommunication Systems

The information contained herein is confidential, proprietary
to Networks In Motion, Inc., and considered a trade secret as
defined in section 499C of the penal code of the State of
California. Use of this information by anyone other than
authorized employees of Networks In Motion is granted only
under a written non-disclosure agreement, expressly
prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#if defined (ANDROID_NDK)
#include <sys/statfs.h>
#endif
#if defined(__QNX__) || defined (IPHONE)
#include <sys/statvfs.h>
#endif
#include <limits.h>
#include "palfile.h"
#include <errno.h>

#define PATH_DELIMITER '/'

struct PAL_File
{
    FILE*    file;
};

struct PAL_FileEnum
{
    char dirName[PAL_FILE_MAX_NAME_LENGTH];
    uint32  filesOnly;
    DIR*   dir;
};

static PAL_Error
MapFileStatToAttributes(struct stat* file_stat, uint32* attributes)
{
    if (file_stat == NULL || attributes == NULL)
    {
        return PAL_ErrBadParam;
    }

    *attributes = 0;

    if (S_ISDIR(file_stat->st_mode))
    {
        *attributes |= PAL_FILE_ATTRIBUTE_DIRECTORY;
    }

    if (file_stat->st_uid == 0)  /* is root the owner of the file? */
    {
        *attributes |= PAL_FILE_ATTRIBUTE_SYSTEM;
    }

    return PAL_Ok;
}

static int
IsHiddenFile(const char *filename)
{
    if (filename == NULL)
    {
        return FALSE;
    }

    int len = strlen(filename);
    if (len == 0)
    {
        return FALSE;
    }

    const char *ptr = filename + len ;   /* end of string  at '\0' */
    while ( (*ptr != '/') && (ptr != filename))
    {
        ptr--;
    }

    if (*ptr == '/')
    {
        ptr++; /* move path '/' */
    }

    if (*ptr == '.')    /* starts with '.' */
    {
        return TRUE;
    }

    return FALSE;
}

static int
IsReadOnlyFile(const char *filename)
{
    /*! @todo Open file to write and if if it is writtable */
    return FALSE;
}

static PAL_Error
GetFileAttributes(const char* filename, uint32* attributes)
{
    if (filename == NULL || attributes == NULL)
    {
        return PAL_ErrBadParam;
    }

    struct stat file_stat;
    if (stat(filename, &file_stat))
    {
        return PAL_ErrFileFailed;
    }

    PAL_Error err = MapFileStatToAttributes(&file_stat, attributes);
    if (err)
    {
        return err;
    }

    if (IsHiddenFile(filename))
    {
        *attributes |= PAL_FILE_ATTRIBUTE_HIDDEN;
    }

    if (IsReadOnlyFile(filename))
    {
        *attributes |= PAL_FILE_ATTRIBUTE_READONLY;
    }
    return PAL_Ok;
}

static PAL_Error
GetFullPathFileName(const char* dirname,
        const char* filename, char* buf, uint32 bufSize)
{
    if ((dirname == NULL) || (filename == NULL) || (buf == NULL))
    {
        return PAL_Failed;
    }

    uint32 dirNameLen = strlen(dirname);
    uint32 fileNameLen = strlen(filename);

    uint32 fullPathSize = dirNameLen + fileNameLen;
    fullPathSize += 2; /*  '/' between dirname and filename, and '\0' */

    if (fullPathSize > bufSize)
    {
        return PAL_Failed;
    }

    *buf = '\0';
    if (strlen(dirname) > 0)
    {
        nsl_strcat(buf, dirname);
        if (dirname[dirNameLen -1 ] != '/')
        {
            /* append '/' */
            nsl_strcat(buf, "/");
        }
    }
    nsl_strcat(buf, filename);

    return PAL_Ok;
}

PAL_DEF PAL_Error
PAL_FileLoadFile(PAL_Instance* pal, const char* filename,
        unsigned char** data, uint32* dataSize)
{

    if (pal == NULL || filename == NULL || data == NULL || dataSize == NULL)
    {
        return PAL_ErrBadParam;
    }

    uint32 fileSize = 0;
    PAL_Error err = PAL_FileGetSize(pal, filename, &fileSize);
    if (err)
    {
        return err;
    }

    unsigned char* buf = NULL;
    uint32 totalBytesRead = 0;
    if (fileSize != 0)  /* non-empty file */
    {
        PAL_File *palFile = NULL;
        buf = (unsigned char*)nsl_malloc(fileSize);
        if (buf == NULL)
        {
            return PAL_ErrNoMem;
        }

        err = PAL_FileOpen(pal, filename, PFM_Read, &palFile);
        if (err)
        {
            nsl_free(buf);
            return err;
        }

        uint32 bytesRead = 0;
        do {
            err = PAL_FileRead(palFile, buf+totalBytesRead,
                    fileSize - totalBytesRead, &bytesRead);
            if (err)
            {
                break;
            }
            else
            {
                totalBytesRead += bytesRead;
            }
        } while ( (bytesRead > 0) && (totalBytesRead < fileSize));

        PAL_FileClose(palFile);

        if (err || (totalBytesRead < fileSize))
        {
            nsl_free(buf);
            return PAL_ErrFileFailed;
        }
    }

    *data = buf;
    *dataSize = totalBytesRead;
    return PAL_Ok;
}

PAL_DEF PAL_Error
PAL_FileOpen(PAL_Instance* pal, const char *filename,
        PAL_FileMode mode, PAL_File** palFile)
{
    if (pal == NULL || filename == NULL || palFile == NULL)
    {
        return PAL_ErrBadParam;
    }

    const char* modeString = NULL;
    switch (mode)
    {
        case PFM_Read:
            modeString = "rb";
            break;

        case PFM_ReadWrite:
            modeString = "rb+";
            break;

        case PFM_Append:
            modeString = "ab+";
            break;

        case PFM_Create:
            modeString = "wb+";
            break;

        default:
            return PAL_ErrBadParam;
    }

    PAL_File* tempPalFile = (PAL_File *)nsl_malloc(sizeof(PAL_File));
    if (!tempPalFile)
    {
        return PAL_ErrNoMem;
    }

    tempPalFile->file = fopen(filename, modeString);
    if (tempPalFile->file == NULL)
    {
        nsl_free(tempPalFile);
        return PAL_ErrFileFailed;
    }

    *palFile = tempPalFile;
    return PAL_Ok;
}


PAL_DEF PAL_Error
PAL_FileClose(PAL_File* palFile)
{
    if (palFile == NULL)
    {
        return PAL_ErrBadParam;
    }

    if(palFile->file)
    {
        fclose(palFile->file);
    }

    nsl_free(palFile);
    return PAL_Ok;
}

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

    if (fflush(palFile->file) != 0)
    {
        return PAL_ErrFileFailed;
    }

    return PAL_Ok;
}    

PAL_DEF PAL_Error
PAL_FileRemove(PAL_Instance* pal, const char *filename)
{
    if (pal == NULL || filename == NULL)
    {
        return PAL_ErrBadParam;
    }

    if (remove(filename))
    {
        return PAL_ErrFileFailed;
    }

    return PAL_Ok;
}


PAL_DEF PAL_Error
PAL_FileRename(PAL_Instance* pal, const char *oldname,
        const char *newname)
{
    if (pal == NULL || oldname == NULL || newname == NULL)
    {
        return PAL_ErrBadParam;
    }

    if (rename(oldname, newname))
    {
        return PAL_ErrFileFailed;
    }

    return PAL_Ok;
}

PAL_DEF PAL_Error
PAL_FileExists(PAL_Instance* pal, const char *filename)
{
    if ((pal == NULL) || (filename == NULL))
    {
        return PAL_ErrBadParam;
    }

    struct stat file_stat;
    if (stat(filename, &file_stat) == 0)
    {
        return PAL_Ok;
    }
    else
    {
        return PAL_ErrFileNotExist;
    }
}

PAL_DEF PAL_Error
PAL_FileGetSize(PAL_Instance* pal, const char* filename,
        uint32* fileSize)
{
    if (pal == NULL || filename == NULL || fileSize == NULL)
    {
        return PAL_ErrBadParam;
    }

    struct stat file_stat;
    if (stat(filename, &file_stat))
    {
        return PAL_ErrFileNotExist;
    }

    *fileSize = file_stat.st_size;
    return PAL_Ok;
}


PAL_DEF uint32
PAL_FileGetFreeSpace(PAL_Instance* pal)
{
#if defined(ANDROID_NDK)
    struct statfs stat;
    if (statfs("/", &stat))
#endif
#if defined(__QNX__) || defined(IPHONE)
    struct statvfs stat;
    if (statvfs("/", &stat))
#endif
    {
        return 0;
    }
    else
    {
        uint64 freeSpace = (uint64)stat.f_bsize * stat.f_bavail;
        if (freeSpace > UINT_MAX)
        {
            freeSpace = UINT_MAX;
        }
        return (uint32)freeSpace;
    }
}


PAL_DEF uint32
PAL_FileGetTotalSpace(PAL_Instance* pal)
{
#if defined(ANDROID_NDK)
    struct statfs stat;
    if (statfs("/", &stat))
#endif
#if defined(__QNX__) || defined(IPHONE)
        struct statvfs stat;
    if (statvfs("/", &stat))
#endif
    {
        return 0;
    }
    else
    {
        uint64 freeSpace = (uint64)stat.f_bsize * stat.f_blocks;
        if (freeSpace > UINT_MAX)
        {
            freeSpace = UINT_MAX;
        }
        return (uint32)freeSpace;
    }
}

PAL_DEF PAL_Error
PAL_FileGetPosition(PAL_File* palFile, uint32* position)
{
    long file_pos = 0;

    if (palFile == NULL || position == NULL)
    {
        return PAL_ErrBadParam;
    }

    if (palFile->file == NULL)
    {
        return PAL_ErrFileNotOpen;
    }

    file_pos = ftell(palFile->file);
    if (file_pos < 0)
    {
        return PAL_ErrFileFailed;
    }

    *position = file_pos;
    return PAL_Ok;
}

PAL_DEF PAL_Error
PAL_FileSetPosition(PAL_File* palFile, PAL_FileSetOrigin origin,
        int32 offset)
{
    if (palFile == NULL)
    {
        return PAL_ErrBadParam;
    }

    if (palFile->file == NULL)
    {
        return PAL_ErrFileNotOpen;
    }

    int fileOrigin = SEEK_CUR;
    switch (origin)
    {
        case PFSO_Current:
            fileOrigin = SEEK_CUR;
            break;
        case PFSO_End:
            fileOrigin = SEEK_END;
            break;
        case PFSO_Start:
            fileOrigin = SEEK_SET;
            break;
        default:
            return PAL_ErrBadParam;
    }

    if (fseek(palFile->file, offset, fileOrigin))
    {
        return PAL_ErrFileFailed;
    }

    return PAL_Ok;
}


PAL_DEF PAL_Error
PAL_FileRead(PAL_File* palFile, byte* buffer, uint32 bufferSize,
        uint32* bytesRead)
{
    if (palFile == NULL || buffer == NULL || bytesRead == NULL)
    {
        return PAL_ErrBadParam;
    }

    if (palFile->file == NULL)
    {
        return PAL_ErrFileNotOpen;
    }

    if (feof(palFile->file) || ferror(palFile->file))
    {
        return PAL_ErrFileFailed;
    }

    *bytesRead = fread(buffer, 1, bufferSize, palFile->file);
    if (ferror(palFile->file)) /* error encountered*/
    {
        return PAL_ErrFileFailed;
    }

    return PAL_Ok;
}


PAL_DEF PAL_Error
PAL_FileWrite(PAL_File* palFile, const byte* buffer, uint32 bufferSize,
        uint32* bytesWritten)
{
    if (palFile == NULL || buffer == NULL || bytesWritten == NULL)
    {
        return PAL_ErrBadParam;
    }

    if (palFile->file == NULL)
    {
        return PAL_ErrFileNotOpen;
    }

    *bytesWritten = fwrite(buffer, 1, bufferSize, palFile->file);
    if (*bytesWritten == 0) /* end of file or write failed */
    {
        return PAL_ErrFileFailed;
    }

    return PAL_Ok;
}


PAL_DEF PAL_Error
PAL_FileTruncate(PAL_File* palFile, uint32 position)
{
    if (palFile == NULL)
    {
        return PAL_ErrBadParam;
    }

    if (palFile->file == NULL)
    {
        return PAL_ErrFileNotOpen;
    }

    if (ftruncate(fileno(palFile->file), position))
    {
        return PAL_ErrFileFailed;
    }

    return PAL_Ok;
}


PAL_DEF PAL_Error
PAL_FileSetCacheSize(PAL_File* palFile, uint32 cacheSize)
{
    if (palFile == NULL)
    {
        return PAL_ErrBadParam;
    }

    if (palFile->file == NULL)
    {
        return PAL_ErrFileNotOpen;
    }

    int buffer_mode;
    if (cacheSize)
    {
        buffer_mode = _IOFBF;
    }
    else
    {
        buffer_mode = _IONBF;
    }

    if (setvbuf(palFile->file, NULL, buffer_mode, cacheSize))
    {
        return PAL_ErrFileFailed;
    }

    return PAL_Ok;
}


PAL_DEF uint8
PAL_FileIsDirectory(PAL_Instance* pal, const char* dirname)
{
    if (pal == NULL || dirname == NULL)
    {
        return 0;
    }

    struct stat file_stat;
    if (stat(dirname, &file_stat))
    {
        return FALSE;
    }

    return S_ISDIR(file_stat.st_mode);
}


PAL_DEF PAL_Error
PAL_FileCreateDirectory(PAL_Instance* pal, const char* dirname)
{
    if (pal == NULL || dirname == NULL)
    {
        return PAL_ErrBadParam;
    }

    if (mkdir(dirname, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH))
    {
    	int k = errno;
        return PAL_ErrFileFailed;
    }

    return PAL_Ok;
}

PAL_DEF PAL_Error
PAL_FileRemoveDirectory(PAL_Instance* pal, const char* dirname,
        uint8 recursive)
{
    if (pal == NULL || dirname == NULL)
    {
        return PAL_ErrBadParam;
    }

    if (recursive)
    {
        /* recursive remove */
        PAL_FileEnum *fileEnum = NULL;
        PAL_FileEnumInfo fileInfo;

        PAL_Error err = PAL_FileEnumerateCreate(pal, dirname, FALSE,
                &fileEnum);
        if (err) {
            return err;
        }

        while (PAL_FileEnumerateNext(fileEnum, &fileInfo) == PAL_Ok)
        {
            char fullPath[PAL_FILE_MAX_NAME_LENGTH];
            err = GetFullPathFileName(dirname, fileInfo.filename, fullPath,
                    sizeof(fullPath));
            if (err) {
                break;
            }

            if (PAL_FileIsDirectory(pal, fullPath))
            {
                err = PAL_FileRemoveDirectory(pal, fullPath, TRUE);
            }
            else
            {
                err = PAL_FileRemove(pal, fullPath);
            }

            if (err) {
                break;
            }
        }

        PAL_FileEnumerateDestroy(fileEnum);

        if (err) {
            return err;
        }
    }

    if (rmdir(dirname))
    {
        return PAL_ErrFileFailed;
    }
    return PAL_Ok;
}

PAL_DEF PAL_Error
PAL_FileGetAttributes(PAL_Instance* pal, const char* filename,
        uint32* attributes)
{
    if (pal == NULL)
    {
        return PAL_ErrBadParam;
    }

    return GetFileAttributes(filename, attributes);
}

PAL_DEF PAL_Error
PAL_FileEnumerateCreate(PAL_Instance* pal, const char* dirname,
        uint8 filesOnly, PAL_FileEnum** fileEnum)
{
    if (pal == NULL || dirname == NULL || fileEnum == NULL)
    {
        return PAL_ErrBadParam;
    }

    PAL_FileEnum* temp = (PAL_FileEnum *) nsl_malloc(sizeof(PAL_FileEnum));
    if (temp == NULL)
    {
        return PAL_ErrNoMem;
    }

    nsl_strncpy(temp->dirName, dirname, sizeof(temp->dirName));
    temp->dirName[sizeof(temp->dirName) - 1] = '\0';
    temp->filesOnly = filesOnly;
    temp->dir = opendir(dirname);
    if (temp->dir == NULL)
    {
        nsl_free(temp);
        return PAL_ErrFileFailed;
    }

    *fileEnum = temp;
    return PAL_Ok;
}

PAL_DEF PAL_Error
PAL_FileEnumerateNext(PAL_FileEnum* fileEnum,
        PAL_FileEnumInfo* fileInformation)
{
    if (fileEnum == NULL || fileInformation == NULL)
    {
        return PAL_ErrBadParam;
    }

    if (fileEnum->dir == NULL)
    {
        return PAL_ErrFileNotOpen;
    }

    char fullPath[PAL_FILE_MAX_NAME_LENGTH];
    struct dirent* entry = NULL;
    uint32 attributes = 0;

    while ((entry = readdir(fileEnum->dir)) != NULL)
    {
        if ((nsl_strcmp(entry->d_name, ".") == 0) ||
            (nsl_strcmp(entry->d_name, "..") == 0))
        {
            /* skip "." or ".." */
            continue;
        }

        if (GetFullPathFileName(fileEnum->dirName, entry->d_name, fullPath,
                    sizeof(fullPath)) != PAL_Ok)
        {
            return PAL_ErrFileFailed;
        }

        if (GetFileAttributes(fullPath, &attributes) != PAL_Ok)
        {
            return PAL_ErrFileFailed;
        }

        if (fileEnum->filesOnly && (attributes & PAL_FILE_ATTRIBUTE_DIRECTORY))
        {
            /* skip directory if filesOnly is true */
            continue;
        }
        else
        {
            break;
        }
    }

    if (entry == NULL)
    {
        return PAL_ErrNoData;
    }

    struct stat file_stat;
    if (stat(fullPath, &file_stat))
    {
        return PAL_ErrFileFailed;
    }

    nsl_strcpy(fileInformation->filename, entry->d_name);
    fileInformation->size = file_stat.st_size;
    fileInformation->attributes = attributes;
    return PAL_Ok;
}

PAL_DEF PAL_Error
PAL_FileEnumerateDestroy(PAL_FileEnum* fileEnum)
{
    if (fileEnum == NULL)
    {
        return PAL_ErrBadParam;
    }

    if (fileEnum->dir == NULL)
    {
        return PAL_ErrFileNotOpen;
    }

    closedir(fileEnum->dir);
    nsl_free(fileEnum);
    return PAL_Ok;
}

/* See header for description */
PAL_Error
PAL_FileAppendPath(PAL_Instance* pal, char* path, uint32 maxPathLength, const char* additionalPath)
{
    if (pal == NULL || path == NULL || additionalPath == NULL)
    {
        return PAL_ErrBadParam;
    }

    int length = nsl_strlen(path);

    // Check char buffer size. Add 2 for delimiter and NULL termination
    if ((length + nsl_strlen(additionalPath) + 2) > maxPathLength)
    {
        return PAL_ErrRange;
    }

    // Add delimiter if not there, yet
    if (path[length - 1] != PATH_DELIMITER)
    {
        path[length] = PATH_DELIMITER;
        path[length + 1] = '\0';
    }

    nsl_strcat(path, additionalPath);
    return PAL_Ok;
}


/*
 * Code from http://stackoverflow.com/questions/675039/how-can-i-create-directory-tree-in-c-linux
 */

static int do_mkdir(const char *path, mode_t mode)
{
    struct stat     st;
    int             status = 0;

    if (stat(path, &st) != 0)
    {
        /* Directory does not exist */
        if (mkdir(path, mode) != 0)
            status = -1;
    }
    else if (!S_ISDIR(st.st_mode))
    {
        errno = ENOTDIR;
        status = -1;
    }

    return(status);
}

/**
** mkpath - ensure all directories in path exist
** Algorithm takes the pessimistic view and works top-down to ensure
** each directory in path exists, rather than optimistically creating
** the last element and working backwards.
*/
static int mkpath(const char *path, mode_t mode)
{
    char           *pp;
    char           *sp;
    int             status;
    char           *copypath = nsl_strdup(path);

    status = 0;
    pp = copypath;
    while (status == 0 && (sp = strchr(pp, '/')) != 0)
    {
        if (sp != pp)
        {
            /* Neither root nor double slash in path */
            *sp = '\0';
            status = do_mkdir(copypath, mode);
            *sp = '/';
        }
        pp = sp + 1;
    }
    if (status == 0)
        status = do_mkdir(path, mode);
    nsl_free(copypath);
    return (status);
}



/* See header for description */
PAL_Error
PAL_FileCreateDirectoryEx(PAL_Instance* pal, const char* directoryName)
{
	if (pal == NULL || directoryName == NULL)
	{
		return PAL_Ok;
	}

	if (mkpath(directoryName, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) != 0)
	{
		return PAL_Failed;
	}

	return PAL_Ok;
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


/* See header for description */
PAL_Error
PAL_FileRemovePath(PAL_Instance* pal, char* path)
{
	char* current = NULL;

    if (pal == NULL || path == NULL)
    {
        return PAL_ErrBadParam;
    }

    // Find last path delimiter
    current = nsl_strrchr(path, PATH_DELIMITER);
    if (current)
    {
        // Just set the last delimiter to zero will terminate the string. We don't need to delete any characters following it
        *current = '\0';
    }
    // Not sure if we should return an error if we didn't remove a path
    return PAL_Ok;
}

PAL_DEF void
PAL_FileFullPath(char* fullpath, int maxlen, const char* filename)
{
    // TODO: This should be changed in the future basing on different platforms.
    // Currently, because Android and W32 platform do not use this function, so just copy filenae to fullpath.
    nsl_strncpy(fullpath, filename, maxlen);
}

#ifdef __cplusplus
 }
#endif
