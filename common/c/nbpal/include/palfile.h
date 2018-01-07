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

@file palfile.h
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

#ifndef PALFILE_H
#define PALFILE_H

#include "paltypes.h"
#include "palstdlib.h"
#include "pal.h"


/*!
    @addtogroup nbpalfilefunc
    @{
*/

typedef struct PAL_File PAL_File;


/*! File access modes */
typedef enum
{
    PFM_Read,           /*!< Open for read access.  File must already exist */
    PFM_ReadWrite,      /*!< Open for read and write access.  File must already exist */
    PFM_Create,         /*!< Create file if it does not exist.  If the file already exists, open and destroy all contents */
    PFM_Append          /*!< Open file for append access.  If the file does not not exist, it will be created */
} PAL_FileMode;


/*! File seek starting points */
typedef enum{
    PFSO_Current,       /*!< Seek from current file pointer */
    PFSO_Start,         /*!< Seek from beginning of file */
    PFSO_End            /*!< Seek from end of file */
}PAL_FileSetOrigin;


#define PAL_FILE_CACHE_SIZE_DEFAULT     (-1)    /*!< Default file cache size defined by OEM */
#define PAL_FILE_CACHE_SIZE_BEST        (-2)    /*!< Cache entire file, if possible */
#define PAL_FILE_CACHE_SIZE_MINIMUM     (-3)    /*!< Minimum file cache size defined by OEM */
#define PAL_FILE_CACHE_SIZE_MAXIMUM     (-4)    /*!< Maximum file cache size defined by OEM */


#define PAL_FILE_ATTRIBUTE_NORMAL       (0x00)  /*!< Normal File */
#define PAL_FILE_ATTRIBUTE_SYSTEM       (0x01)  /*!< System File */
#define PAL_FILE_ATTRIBUTE_HIDDEN       (0x02)  /*!< Hidden File */
#define PAL_FILE_ATTRIBUTE_READONLY     (0x04)  /*!< Readonly File */
#define PAL_FILE_ATTRIBUTE_DIRECTORY    (0x08)  /*!< Directory */


/*! Open a file with the specified access mode

Opens the specified file with the given mode.  If the function succeeds, the palFile must
be closed using PAL_FileClose

@param pal The pal instance
@param filename The name of the file to be opened
@param mode The mode that the file should be opened with
@param palFile The file object created upon success.  This object must be closed with PAL_FileClose
@return PAL error code
@see PAL_FileClose
*/
PAL_DEC PAL_Error PAL_FileOpen(PAL_Instance* pal, const char *filename, PAL_FileMode mode, PAL_File** palFile);


/*! Close a previously opened file

Close a file and release all resources associated with it

@param palFile A file object previously opened with PAL_FileOpen
@return PAL error code
@see PAL_FileOpen
*/
PAL_DEC PAL_Error PAL_FileClose(PAL_File* palFile);

/*! Flush all open output streams

    This function forces a write of all buffered data for the given output or update stream
    via the stream's underlying write function.

    @return PAL_Ok if success,
            PAL_ErrFileNotOpen if file is not open,
            PAL_ErrFileFailed if an error of file operation occurs.
*/
PAL_DEC PAL_Error
PAL_FileFlush(PAL_File* palFile     /*!< A file object to flush */
              );

/*! Sets the cache size of an open file

Sets the file system cache size of the file.  If file cache size is not
supported, this function is a non-operation that returns a success error code

@param palFile The file object to set the cache size for
@param cacheSize The number of bytes to cache or a special cache size value
@return PAL error code
@see PAL_FILE_CACHE_SIZE_DEFAULT
@see PAL_FileOpen
*/
PAL_DEC PAL_Error PAL_FileSetCacheSize(PAL_File* palFile, uint32 cacheSize);


/*! Gets the file pointer of the file

Gets the current position of the file pointer

@return PAL error code
@see PAL_FileSetPosition
*/
PAL_DEC PAL_Error PAL_FileGetPosition(PAL_File* palFile, uint32* position);


/*! Sets the file pointer of the file

Sets the file pointer to a position offset from the specified origin

@return PAL error code
@see PAL_FileGetPosition
*/
PAL_DEC PAL_Error PAL_FileSetPosition(PAL_File* palFile, PAL_FileSetOrigin origin, int32 offset);


/*! Read data from a file

Read byte data from a file

@param palFile The file to read from
@param buffer The buffer to read data into
@param bufferSize The number of bytes requested
@param bytesRead The number of bytes read into the buffer
@return PAL error code
*/
PAL_DEC PAL_Error PAL_FileRead(PAL_File* palFile, uint8* buffer, uint32 bufferSize, uint32* bytesRead);


/*! Write data to a file

Write byte data to a file at the current file position

@param palFile The file to write to
@param buffer The buffer to write data from
@param bufferSize The number of bytes to write
@param bytesWritten The number of bytes actually written
@return PAL error code
*/
PAL_DEC PAL_Error PAL_FileWrite(PAL_File* palFile, const uint8* buffer, uint32 bufferSize, uint32* bytesWritten);


/*! Trucate a file to the specified position

Truncate a file to the specified position.  On success, the file position will be at the end of the file

@param palFile The file to truncate
@param position The new length of the file
@return PAL error code
*/
PAL_DEC PAL_Error PAL_FileTruncate(PAL_File* palFile, uint32 position);


/*! Get file attributes

Get the attributes of a file

@param pal A pointer to the pal
@param filename The name of the file
@param attributes The attributes of the file
@return PAL error code
*/
PAL_DEC PAL_Error PAL_FileGetAttributes(PAL_Instance* pal, const char* filename, uint32* attributes);


/*!
    @}
    @addtogroup nbpalfilemisc
    @{
*/


/*! Delete a file from the filesystem

Delete a file from the filesystem

@param pal A pointer to the pal
@param filename The name of the file to delete
@return PAL error code
*/
PAL_DEC PAL_Error PAL_FileRemove(PAL_Instance* pal, const char *filename);


/*! Rename a file on the filesystem

Rename a file on the filesystem

@param pal A pointer to the pal
@param oldFilename The name of the file to rename
@param newFilename The new name of the file
@return PAL error code
*/
PAL_DEC PAL_Error PAL_FileRename(PAL_Instance* pal, const char *oldFilename, const char *newFilename);


/*! Check to see if a file exists

Check to see if a file or directory exists

@param pal A pointer to the pal
@param filename The name of the file to check for existence
@return PAL_Ok if file exists, PAL_ErrFileNotExist otherwise
*/
PAL_DEC PAL_Error PAL_FileExists(PAL_Instance* pal, const char *filename);


/*! Get the size of a file

Get the size of a file, in bytes

@param pal A pointer to the pal
@param filename The name of the file to get the size of
@param fileSize A pointer to a uint32 to receive the file size
@return PAL error code
*/
PAL_DEC PAL_Error PAL_FileGetSize(PAL_Instance* pal, const char* filename, uint32* fileSize);


/*! Load a file into memory

Read an entire file into memory in one operation

@param pal A pointer to the pal
@param filename The name of the file to load
@param data A pointer that receives the address of the data buffer.  The caller must free the memory with nsl_free
@param dataSize The number of bytes returned in the data buffer
@return PAL error code
*/
PAL_DEC PAL_Error PAL_FileLoadFile(PAL_Instance* pal, const char* filename, unsigned char** data, uint32* dataSize);


/*!
    @}
    @addtogroup nbpalfilesystem
    @{
*/


/*! Get the free space available on the file system

Get the free space available on the file system

@param pal A pointer to the pal
@return The amount of free space, in bytes
@see PAL_FileGetTotalSpace
*/
PAL_DEC uint32 PAL_FileGetFreeSpace(PAL_Instance* pal);


/*! Get the total size of the file system

Get the total size of the file system

@param pal A pointer to the pal
@return The size of the filesystem, in bytes
@see PAL_FileGetFreeSpace
*/
PAL_DEC uint32 PAL_FileGetTotalSpace(PAL_Instance* pal);


/*!
    @}
    @addtogroup nbpaldirectory
    @{
*/


/*! Test to see if the specified name is a directory

Test to see if the specified name is a directory on the file system

@param pal A pointer to the pal
@param directoryName The name of the directory
@return non-zero if the directory exists, 0 otherwise
*/
PAL_DEC nb_boolean PAL_FileIsDirectory(PAL_Instance* pal, const char* directoryName);


/*! Create a directory

Create a new directory

@param pal A pointer to the pal
@param directoryName The name of the directory to create
@return PAL error code
*/
PAL_DEC PAL_Error PAL_FileCreateDirectory(PAL_Instance* pal, const char* directoryName);

/*! Creates all subdirectories in the given path.

This function ensures that all sub-path in the given directory are created. The given directory
can not contain a file-name (otherwise a directory with the file-name's name might be created)

The path has to be created using PAL_FileAppendPath()/PAL_FileRemovePath() to ensure that the platform 
specific delimiters are used.

@param pal              A pointer to the pal
@param directoryName    Path to the directories to create

@return PAL error code

@see PAL_FileAppendPath
@see PAL_FileRemovePath
*/
PAL_DEC PAL_Error PAL_FileCreateDirectoryEx(PAL_Instance* pal, const char* directoryName);

/*! Remove a directory or directory tree

Remove a directory or directory tree.  A directory may need to be empty to be
deleted, in which case the recursive flag should be set to a non-zero value to
recursively delete all of the child objects of the directory

@param pal A pointer to the pal
@param directoryName The name of the directory to create
@param recursive When 0, delete the directory only, otherwise delete the directory and all subdirectories

@return PAL error code
*/
PAL_DEC PAL_Error PAL_FileRemoveDirectory(PAL_Instance* pal, const char* directoryName, nb_boolean recursive);


/*! Appends a path or file to an existing path.

This function adds one subdirectory to the end of the existing directory string. This function can also be used
to add a file name to the end of a path.

It uses the correct system delimiter. Please use this function instead of "self-made" path modification.

@param pal              A pointer to the pal
@param path             In/Out: On input the existing path to which to add the new path, on output the combined path
@param maxPathLength    Buffer size for path to contain the combined path
@param additionalPath   The relative path to add

@return PAL error code

@see PAL_FileRemovePath
*/
PAL_DEC PAL_Error PAL_FileAppendPath(PAL_Instance* pal, char* path, uint32 maxPathLength, const char* additionalPath);


/*! Removes the last subdirectory or file from a given path.

This function can be used to remove a subdirectory or a file from a given path.

It uses the correct system delimiter. Please use this function instead of "self-made" path modification.

@param pal      A pointer to the pal
@param path     In/Out: Path to modify.

@return PAL error code

@see PAL_FileAppendPath
*/
PAL_DEC PAL_Error PAL_FileRemovePath(PAL_Instance* pal, char* path);


/*! Get last subdirectory or file from a given path.

This function can be used to get a subdirectory or a file from a given path.

It uses the correct system delimiter. Please use this function instead of "self-made" path modification.

@param pal      A pointer to the pal
@param fullPath In: Full path
@param lastPath Out: Last path or filename. It must be freed.

@return PAL error code

@see PAL_FileAppendPath
*/
PAL_DEC PAL_Error PAL_FileGetLastPath(PAL_Instance* pal, const char* fullPath, char** lastPath);


/*!
    @}
    @addtogroup nbpalfileenum
    @{
*/


//@note: In most of filesystems, there is limitation on length of file name,
//       but not on length of full path.

/*! Maximum length of a filename */
#define PAL_FILE_MAX_NAME_LENGTH        512

/*! Maximum length of fullpath. */
#define PAL_FILE_MAX_FULL_PATH_LENGTH  4096

/*! Data structure used to enumerate files */
typedef struct PAL_FileEnum PAL_FileEnum;


/*! File enumeration information
@see PAL_FileGetAttributes
*/
typedef struct PAL_FileEnumInfo
{
    uint32      attributes;                         /*!< File attributes */
    uint32      size;                               /*!< File size, in bytes */
    char        filename[PAL_FILE_MAX_NAME_LENGTH]; /*!< Filename */
} PAL_FileEnumInfo;


/*! Start a file system enumeration

Start a file system enumeration.  All files, including hidden and system files, are returned.  If filesOnly is not
specified, all directories are returned, except for the special current (.) and parent (..) directories.  There is
not guarantee in the order that files (and directories) are returned.

@param pal A pointer to the pal
@param directoryName The directory to enumerate
@param filesOnly Whether or not to return directories as well as files.  Pass 0 to include directories, non-zero otherwise
@param fileEnum A pointer to an enumeration object.  This object must be freed using PAL_FileEnumerateDestroy
@return PAL error code
@see PAL_FileEnumerateNext
@see PAL_FileEnumerateDestroy
*/
PAL_DEC PAL_Error PAL_FileEnumerateCreate(PAL_Instance* pal, const char* directoryName, nb_boolean filesOnly, PAL_FileEnum** fileEnum);


/*! Get the next file from an enumeration

Gets the first/next file from an enumeration.  If all files have been enumerated, the return value will be PAL_ErrNoData

@param fileEnum A pointer to an enumeration object
@param fileInformation A pointer to an information object to receive data about the next file in the enumeration
@return PAL error code
@see PAL_FileEnumerateCreate
*/
PAL_DEC PAL_Error PAL_FileEnumerateNext(PAL_FileEnum* fileEnum, PAL_FileEnumInfo* fileInformation);


/*! End a file system enumeration

End a file system enumeration and release any resources held

@param fileEnum A pointer to an enumeration object
@return PAL error code
@see PAL_FileEnumerateCreate
*/
PAL_DEC PAL_Error PAL_FileEnumerateDestroy(PAL_FileEnum* fileEnum);

/*! Get home directory. (deprecated) The same as PAL_FileGetWorkPath

 @return work directory (full access folder)
 */

PAL_DEC const char* PAL_FileGetHomeDirectory();

/*! Get application directory. (deprecated) The same as PAL_FileGetApplicationPath

 @return application directory.
 */

PAL_DEC const char* PAL_FileGetBundleDirectory();

/*! Get application directory path. The files are read-only in this directory.
    This directory contains resources.
 iOS   - <Application_Home>/appname/appname.app
 BB10  - <Application_Home>/app/native/
 QNX   - TODO
 Win32 - <Application_Home>/
 WP8   - TODO

 @return application directory path
 */
PAL_DEC const char* PAL_FileGetApplicationPath();

/*! Get work directory path.
 iOS   - <Application_Home>/Documents
 BB10  - <Application_Home>/data/
 QNX   - TODO
 Win32 - <Application_Home>/
 WP8   - TODO

 @return work directory path
 */
PAL_DEC const char* PAL_FileGetWorkPath();

/*! Get cache directory path. The files can be deleted by system.
 iOS   - <Application_Home>/Library/Caches
 BB10  - <Application_Home>/tmp/
 QNX   - TODO
 Win32 - <Application_Home>/
 WP8   - TODO

 @return cache directory path
 */
PAL_DEC const char* PAL_FileGetCachePath();

/*! Flush data to file

 @param palFile The file object
 @return PAL error code
 */
PAL_DEC PAL_Error PAL_FileFlush(PAL_File* palFile);

/*! Make file full path
 
 @param fullpath. return full path.
 @param maxlen. The length of received buffer.
 @param filename. file name.
 */
PAL_DEC void PAL_FileFullPath(char* fullpath, int maxlen, const char* filename);

/*! @} */

#endif
