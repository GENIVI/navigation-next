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

#include <QFile>
#include <QDir>
#include "palfile.h"

char* g_workpath = NULL;

struct PAL_File
{
    QFile*  pFile;
};

struct PAL_FileEnum
{
    PAL_Instance*                   pPal;
    QDir*                           pDir;
    QStringList*                    pList;
    QStringList::const_iterator     it;
};

PAL_DEF PAL_Error PAL_FileOpen(PAL_Instance* pal, const char *filename, PAL_FileMode mode, PAL_File** palFile)
{
    if (pal == NULL || filename == NULL || palFile == NULL)
    {
        return PAL_ErrBadParam;
    }

    QIODevice::OpenMode openMode;
    switch (mode)
    {
        case PFM_Read:
            openMode = QIODevice::ReadOnly;
            break;
        case PFM_ReadWrite:
            openMode = QIODevice::ReadWrite;
            break;
        case PFM_Create:
            openMode = QIODevice::ReadWrite | QIODevice::Truncate;
            break;
        case PFM_Append:
            openMode = QIODevice::Append;
            break;
        default:
            return PAL_ErrUnsupported;
            break;
    }

    PAL_File* tempPalFile = (PAL_File *)nsl_malloc(sizeof(PAL_File));
    if (!tempPalFile)
    {
        return PAL_ErrNoMem;
    }

    tempPalFile->pFile = new QFile(filename);
    if (tempPalFile->pFile == NULL)
    {
        nsl_free(tempPalFile);
        return PAL_ErrNoMem;
    }

    if (!tempPalFile->pFile->open(openMode))
    {
        nsl_free(tempPalFile);
        return PAL_ErrFileFailed;
    }

    *palFile = tempPalFile;
    return PAL_Ok;
}

PAL_DEF PAL_Error PAL_FileClose(PAL_File* palFile)
{
    if (palFile == NULL)
    {
        return PAL_ErrBadParam;
    }

    if(palFile->pFile)
    {
        palFile->pFile->close();
        delete palFile->pFile;
    }

    nsl_free(palFile);
    return PAL_Ok;
}

PAL_DEF PAL_Error PAL_FileFlush(PAL_File* palFile)
{
    if (!palFile)
    {
        return PAL_ErrBadParam;
    }

    if (palFile->pFile == NULL)
    {
        return PAL_ErrFileNotOpen;
    }

    if (!palFile->pFile->flush())
    {
        return PAL_ErrFileFailed;
    }

    return PAL_Ok;
}

PAL_DEF PAL_Error PAL_FileSetCacheSize(PAL_File*, uint32)
{
    return PAL_Ok;
}

PAL_DEF PAL_Error PAL_FileGetPosition(PAL_File* palFile, uint32* position)
{
    long file_pos = 0;

    if (palFile == NULL || position == NULL)
    {
        return PAL_ErrBadParam;
    }

    if (palFile->pFile == NULL)
    {
        return PAL_ErrFileNotOpen;
    }

    file_pos = palFile->pFile->pos();
    if (file_pos < 0)
    {
        return PAL_ErrFileFailed;
    }

    *position = file_pos;
    return PAL_Ok;
}

PAL_DEF PAL_Error PAL_FileSetPosition(PAL_File* palFile, PAL_FileSetOrigin origin, int32 offset)
{
    if (palFile == NULL)
    {
        return PAL_ErrBadParam;
    }

    if (palFile->pFile == NULL)
    {
        return PAL_ErrFileNotOpen;
    }

    uint32 position = 0;
    switch (origin)
    {
        case PFSO_Current:
        {
            PAL_Error ret = PAL_FileGetPosition(palFile, &position);
            if (ret != PAL_Ok)
            {
                return ret;
            }

            position += offset;
            break;
        }
        case PFSO_End:
        {
            position = palFile->pFile->size() - offset;
            break;
        }
        case PFSO_Start:
        {
            position = offset;
            break;
        }
        default:
            return PAL_ErrBadParam;
    }

    if (!palFile->pFile->seek(position))
    {
        return PAL_ErrFileFailed;
    }

    return PAL_Ok;
}

PAL_DEF PAL_Error PAL_FileRead(PAL_File* palFile, uint8* buffer, uint32 bufferSize, uint32* bytesRead)
{
    if (palFile == NULL || buffer == NULL || bytesRead == NULL)
    {
        return PAL_ErrBadParam;
    }

    if (palFile->pFile == NULL)
    {
        return PAL_ErrFileNotOpen;
    }

    qint64 readed = palFile->pFile->read((char*)buffer, bufferSize);
    if (readed < 0)
    {
        return PAL_ErrFileFailed;
    }

    *bytesRead = readed;
    return PAL_Ok;
}

PAL_DEF PAL_Error PAL_FileWrite(PAL_File* palFile, const uint8* buffer, uint32 bufferSize, uint32* bytesWritten)
{
    if (palFile == NULL || buffer == NULL || bytesWritten == NULL)
    {
        return PAL_ErrBadParam;
    }

    if (palFile->pFile == NULL)
    {
        return PAL_ErrFileNotOpen;
    }

    qint64 writed = palFile->pFile->write((const char*)buffer, bufferSize);
    if (writed < 0)
    {
        return PAL_ErrFileFailed;
    }

    *bytesWritten = writed;
    return PAL_Ok;
}

PAL_DEF PAL_Error PAL_FileTruncate(PAL_File* palFile, uint32 position)
{
    if (palFile == NULL)
    {
        return PAL_ErrBadParam;
    }

    if (palFile->pFile == NULL)
    {
        return PAL_ErrFileNotOpen;
    }

    if (!palFile->pFile->resize(position))
    {
        return PAL_ErrFileFailed;
    }

    return PAL_Ok;
}

PAL_DEF PAL_Error PAL_FileGetAttributes(PAL_Instance* pal, const char* filename, uint32* attributes)
{
    if (pal == NULL || filename == NULL || attributes == NULL)
    {
        return PAL_ErrBadParam;
    }

    if (PAL_FileIsDirectory(pal, filename))
    {
        *attributes |= PAL_FILE_ATTRIBUTE_DIRECTORY;
    }
    else
    {
        QFile file(filename);
        if (file.exists())
        {
            QString fn(filename);
            QStringList list = fn.split(QDir::separator(), QString::SkipEmptyParts);
            if (list.last().startsWith("."))
            {
                *attributes |= PAL_FILE_ATTRIBUTE_HIDDEN;
            }
            else
            {
                *attributes |= PAL_FILE_ATTRIBUTE_NORMAL;
            }

            if (file.open(QIODevice::Append))
            {
                file.close();
            }
            else
            {
                *attributes |= PAL_FILE_ATTRIBUTE_READONLY;
            }
        }
    }

    return PAL_Ok;
}

PAL_DEF PAL_Error PAL_FileRemove(PAL_Instance* pal, const char *filename)
{
    if (pal == NULL || filename == NULL)
    {
        return PAL_ErrBadParam;
    }

    QFile file(filename);
    if (!file.remove())
    {
        return PAL_ErrFileFailed;
    }

    return PAL_Ok;
}

PAL_DEF PAL_Error PAL_FileRename(PAL_Instance* pal, const char *oldFilename, const char *newFilename)
{
    if (pal == NULL || oldFilename == NULL || newFilename == NULL)
    {
        return PAL_ErrBadParam;
    }

    QFile file(oldFilename);
    if (!file.rename(newFilename))
    {
        return PAL_ErrFileFailed;
    }

    return PAL_Ok;
}

PAL_DEF PAL_Error PAL_FileExists(PAL_Instance* pal, const char *filename)
{
    if ((pal == NULL) || (filename == NULL))
    {
        return PAL_ErrBadParam;
    }

    QFile file(filename);
    if (file.exists())
    {
        return PAL_Ok;
    }
    else
    {
        return PAL_ErrFileNotExist;
    }
}

PAL_DEF PAL_Error PAL_FileGetSize(PAL_Instance* pal, const char* filename, uint32* fileSize)
{
    if (pal == NULL || filename == NULL || fileSize == NULL)
    {
        return PAL_ErrBadParam;
    }

    QFile file(filename);
    *fileSize = file.size();

    return PAL_Ok;
}

PAL_DEF PAL_Error PAL_FileLoadFile(PAL_Instance* pal, const char* filename, unsigned char** data, uint32* dataSize)
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

PAL_DEF uint32 PAL_FileGetFreeSpace(PAL_Instance*)
{
    // TODO: Not support
    return -1;
}

PAL_DEF uint32 PAL_FileGetTotalSpace(PAL_Instance*)
{
    // TODO: Not support
    return -1;
}

PAL_DEF nb_boolean PAL_FileIsDirectory(PAL_Instance* pal, const char* directoryName)
{
    if (pal == NULL || directoryName == NULL)
    {
        return 0;
    }

    QDir dir(directoryName);
    if (!dir.exists())
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

PAL_DEF PAL_Error PAL_FileCreateDirectory(PAL_Instance* pal, const char* directoryName)
{
    if (pal == NULL || directoryName == NULL)
    {
        return PAL_ErrBadParam;
    }

    QDir dir;
    if (!dir.mkdir(directoryName))
    {
        return PAL_ErrFileFailed;
    }

    return PAL_Ok;
}

PAL_DEF PAL_Error PAL_FileCreateDirectoryEx(PAL_Instance* pal, const char* directoryName)
{
    if (pal == NULL || directoryName == NULL)
    {
        return PAL_Ok;
    }

    QDir dir;
    if (!dir.mkpath(directoryName))
    {
        return PAL_ErrFileFailed;
    }

    return PAL_Ok;
}

PAL_DEF PAL_Error PAL_FileRemoveDirectory(PAL_Instance* pal, const char* directoryName, nb_boolean recursive)
{
    if (pal == NULL || directoryName == NULL)
    {
        return PAL_ErrBadParam;
    }

    if (recursive)
    {
        /* recursive remove */
        PAL_FileEnum *fileEnum = NULL;
        PAL_FileEnumInfo fileInfo;

        PAL_Error err = PAL_FileEnumerateCreate(pal, directoryName, FALSE, &fileEnum);
        if (err)
        {
            return err;
        }

        while (PAL_FileEnumerateNext(fileEnum, &fileInfo) == PAL_Ok)
        {
            char fullPath[PAL_FILE_MAX_NAME_LENGTH];
            nsl_strcpy(fullPath, directoryName);
            err = PAL_FileAppendPath(pal, fullPath, PAL_FILE_MAX_NAME_LENGTH, fileInfo.filename);
            if (err)
            {
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

            if (err)
            {
                break;
            }
        }

        PAL_FileEnumerateDestroy(fileEnum);

        if (err)
        {
            return err;
        }
    }

    QDir dir;
    if (!dir.rmdir(directoryName))
    {
        return PAL_ErrFileFailed;
    }

    return PAL_Ok;
}

PAL_DEF PAL_Error PAL_FileAppendPath(PAL_Instance* pal, char* path, uint32 maxPathLength, const char* additionalPath)
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

PAL_DEF PAL_Error PAL_FileRemovePath(PAL_Instance* pal, char* path)
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

PAL_DEF PAL_Error PAL_FileGetLastPath(PAL_Instance* pal, const char* fullPath, char** lastPath)
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

PAL_DEF PAL_Error PAL_FileEnumerateCreate(PAL_Instance* pal, const char* directoryName, nb_boolean filesOnly, PAL_FileEnum** fileEnum)
{
    if (pal == NULL || directoryName == NULL || fileEnum == NULL)
    {
        return PAL_ErrBadParam;
    }

    PAL_FileEnum* temp = (PAL_FileEnum *) nsl_malloc(sizeof(PAL_FileEnum));
    if (temp == NULL)
    {
        return PAL_ErrNoMem;
    }

    temp->pPal = pal;
    temp->pDir = new QDir(directoryName);
    if (filesOnly)
    {
        temp->pList = new QStringList(temp->pDir->entryList(QDir::Files | QDir::NoDotAndDotDot, QDir::Name));
    }
    else
    {
        temp->pList = new QStringList(temp->pDir->entryList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot, QDir::Name));
    }
    temp->it = temp->pList->begin();

    *fileEnum = temp;
    return PAL_Ok;
}

PAL_DEF PAL_Error PAL_FileEnumerateNext(PAL_FileEnum* fileEnum, PAL_FileEnumInfo* fileInformation)
{
    if (fileEnum == NULL || fileInformation == NULL)
    {
        return PAL_ErrBadParam;
    }

    PAL_Error result = PAL_ErrNoData;
    if ((fileEnum->it) != fileEnum->pList->end())
    {
        QString filename = *(fileEnum->it);
        nsl_strcpy(fileInformation->filename, filename.toUtf8().constData());

        QString path = fileEnum->pDir->path();
        QString fullpath = QString("%1%2%3").arg(path).arg(QDir::separator()).arg(filename);
        std::string fp = fullpath.toStdString();
        const char* pFullPath = fp.c_str();
        result = PAL_FileGetAttributes(fileEnum->pPal, pFullPath, &(fileInformation->attributes));
        result = (result == PAL_Ok) ? PAL_FileGetSize(fileEnum->pPal, pFullPath, &(fileInformation->size)) : result;
        ++(fileEnum->it);
    }

    return result;
}

PAL_DEF PAL_Error PAL_FileEnumerateDestroy(PAL_FileEnum* fileEnum)
{
    if (fileEnum == NULL)
    {
        return PAL_ErrBadParam;
    }

    if (fileEnum->pDir != NULL)
    {
        delete fileEnum->pDir;
        fileEnum->pDir = NULL;
    }

    if (fileEnum->pList != NULL)
    {
        delete fileEnum->pList;
        fileEnum->pList = NULL;
    }

    nsl_free(fileEnum);
    return PAL_Ok;
}

PAL_DEF const char* PAL_FileGetHomeDirectory()
{
    return NULL;
}

PAL_DEF const char* PAL_FileGetBundleDirectory()
{
    return NULL;
}

PAL_DEF const char* PAL_FileGetApplicationPath()
{
    return NULL;
}

PAL_DEF const char* PAL_FileGetWorkPath()
{
    // @todo This should be changed in the future.
    //       Currently we just use this method to get work path.
    // copy navkit resource to data folde
    return g_workpath;
}

PAL_DEF const char* PAL_FileGetCachePath()
{
    return g_workpath;
}

PAL_DEF void PAL_FileFullPath(char* fullpath, int maxlen, const char* filename)
{
    // TODO: This should be changed in the future basing on different platforms.
    // Currently, because Android and W32 platform do not use this function, so just copy filename to fullpath.
    nsl_strncpy(fullpath, filename, maxlen);
}
