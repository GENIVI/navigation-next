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

/*****************************************************************/
/*                                                               */
/* (C) Copyright 2008 by Networks In Motion, Inc.                */
/*                                                               */
/* The information contained herein is confidential, proprietary */
/* to Networks In Motion, Inc., and considered a trade secret as */
/* defined in section 499C of the penal code of the State of     */
/* California. Use of this information by anyone other than      */
/* authorized employees of Networks In Motion is granted only    */
/* under a written non-disclosure agreement, expressly           */
/* prescribing the scope and manner of such use.                 */
/*                                                               */
/*****************************************************************/

#include "fileutil.h"
#include "sha1.h"
#include "paldebuglog.h"

#define TRUNCATEFILE_DEFAULT_CHUNK_SIZE (1024 * 50)

static nb_size   readfile2(PAL_Instance* pal, const char* filename, void **bufp);
static PAL_Error write_raw(PAL_File* fd, const void* pBuffer, uint32 dwCount, uint32* written);


nb_size
readfile(PAL_Instance* pal, const char* szfilename, void**bufp)
{
    // Why do we need two functions? They are the same.
	return readfile2(pal, szfilename, bufp);
}

CSL_DEF nb_size
readfile_obfuscate(PAL_Instance* pal, const char* szfilename, void** ppbuf, const byte* key_data, uint32 key_size)
{
	nb_size ret = readfile(pal, szfilename, ppbuf);

	if (ret > 0 && *ppbuf != NULL)
    {
		obfuscate_buffer(*ppbuf, (uint32)ret, key_data, key_size);
    }

	return ret;
}

/*! Read file.

    @return File size or '-1' on error
*/
nb_size
readfile2(PAL_Instance* pal, const char* filename, void **bufp)
{
    PAL_Error result = PAL_Ok;
	uint32 count = 0;
    uint32 filesize = 0;
	void* buf = NULL;
	PAL_File* file = NULL;
	int size = 0;

    result = PAL_FileGetSize(pal, filename, &filesize);

	if ((result == PAL_Ok) && (filesize > 0))
	{
		size=filesize + 1;
		buf = nsl_malloc(size);
		if (buf == NULL)
        {
			return (-1);
        }

        count = filesize;
		result = PAL_FileOpen(pal, filename, PFM_Read, &file);
		if (result)
		{
			nsl_free(buf);
			return (-1);
		}

        result = PAL_FileRead(file,  (uint8*) buf, size, &count);
        if (result || count != filesize)
		{
			result = PAL_FileClose(file);
			nsl_free(buf);
			return (-1);
		}

        ((byte*)(buf))[filesize] = 0; // make sure the buffer is NULL terminated (for convienience)
		*bufp = buf;
		result = PAL_FileClose(file);
		return (count);
	}
	else
	{
		*bufp = NULL;
		return (-1);
	}
}


nb_size
enumfile(PAL_Instance* pal, const char* szfilename, nb_size chunk, ENUMFILEFN cb, void* pUser)
{
	return enumfile_obfuscate(pal, szfilename, chunk, cb, pUser, NULL, 0);
}

CSL_DEF nb_size
enumfile_obfuscate(PAL_Instance *pal, const char* szfilename, nb_size chunk, ENUMFILEFN cb, void* pUser, const byte* key_data, uint32 key_size)
{
    PAL_Error result;
	PAL_File *fd = NULL;
	nb_size ret = -1;
	uint32 count = 0;
	void*	data = NULL;
	uint32 pos = 0;
	uint32 p = 0;

	if (cb == NULL)
		goto err;

    result = PAL_FileOpen(pal, szfilename, PFM_Read, &fd);

    if (result)
        return -1;

	data = nsl_malloc(chunk);

	if (data == NULL)
		goto err;

	ret = 0;

	while (1) {

		PAL_FileGetPosition(fd, &pos);

        result = PAL_FileRead(fd, (uint8*) data, (uint32)chunk, &count);

        if (result)
            break;

		if (key_data != NULL && key_size > 0)
		{
			for (p=0;p<(uint32) count;p++)
				((byte*) data)[p] ^= key_data[(pos+p) % key_size];
		}

		if (count > 0) {
			ret += count;
			cb(data, count, pUser);
		}

		if (count < (uint32) chunk)
			break;
	}

err:

	if (fd != NULL)
		PAL_FileClose(fd);
	if (data != NULL)
		nsl_free(data);

	return ret;
}

boolean
savefile_mode_obfuscate(PAL_Instance* pal,
                        const char* szfilename,
                        uint8* szdata,              /*!< Data to write */
                        int nsize,                  /*!< Size of data to write */
                        PAL_FileMode mode,
                        const byte* key_data,       /*!< Obfuscate key If set then obfuscate is used */
                        uint32 key_size             /*!< Size of obfuscate key */
                        )
{
    PAL_Error result = PAL_Ok;
    PAL_File* file = NULL;
    uint32 written = 0;

    PAL_SetLastError(pal, PAL_Ok);

    result = PAL_FileOpen(pal, szfilename, mode, &file);
    if (result == PAL_ErrFileNotExist)
    {
        result = PAL_FileOpen(pal, szfilename, PFM_Create, &file);
    }

    if (result != PAL_Ok)
    {
        goto errexit;
    }

    if (mode == PFM_Append)
    {
        result = PAL_FileSetPosition(file, PFSO_End, 0);
        if (result != PAL_Ok)
        {
            PAL_FileClose(file);
            goto errexit;
        }
    }

    if (key_data)
    {
        result = write_obfuscate(file, szdata, nsize, key_data, key_size, &written);
    }
    else
    {
        result = write_raw(file, szdata, nsize, &written);
    }

    if (result != PAL_Ok)
    {
        PAL_FileClose(file);
        goto errexit;
    }

    PAL_FileClose(file);
    return (boolean)(written == (uint32)nsize);

errexit:
    PAL_SetLastError(pal, result);
    return FALSE;
}

boolean
savepalfile_mode_obfuscate(PAL_Instance*   pal,
                           PAL_File*       file,
                           uint8*          szdata,
                           int             nsize,
                           const byte*     key_data,
                           uint32          key_size)
{
    PAL_Error result = PAL_Ok;
    uint32 written = 0;

    PAL_SetLastError(pal, PAL_Ok);
    if (!file)
    {
        result = PAL_ErrFileNotOpen;
        goto errexit;
    }

    if (key_data)
    {
        result = write_obfuscate(file, szdata, nsize, key_data, key_size, &written);
    }
    else
    {
        result = write_raw(file, szdata, nsize, &written);
    }

    if (result == PAL_Ok)
    {
        return (boolean)(written == (uint32)nsize);
    }

errexit:
    PAL_SetLastError(pal, result);
    return FALSE;
}


CSL_DEF boolean
fileexists(PAL_Instance* pal, const char* szfilename)
{
    boolean ret = (boolean)(PAL_FileExists(pal, szfilename) == PAL_Ok);
	return ret;
}

CSL_DEF uint32
filesize(PAL_Instance* pal, const char* szfilename)
{
    PAL_Error result;
	PAL_File* fd = NULL;
    uint32 filesize = 0;

	result = PAL_FileOpen(pal, szfilename, PFM_Read, &fd);

	if (result) {
        return 0;
    }

    PAL_FileGetSize(pal, szfilename, &filesize);

    PAL_FileClose(fd);

	return filesize;
}

CSL_DEF boolean
createdirectory(PAL_Instance* pal, const char* szdirname)
{
    PAL_Error result = PAL_FileCreateDirectory(pal, szdirname);
	if (result != PAL_Ok)
	{
		return 0;
	}
	return 1;
}

CSL_DEF boolean
isdirectory(PAL_Instance* pal, const char* szdirname)
{
    boolean ret = PAL_FileIsDirectory(pal, szdirname);
    return ret;
}


CSL_DEF boolean
savefile(PAL_Instance* pal, PAL_FileMode mode, const char* szfilename, uint8* szdata, int nsize)
{
	return savefile_mode_obfuscate(pal, szfilename, szdata, nsize, mode, NULL, 0);
}

CSL_DEF boolean savefile_obfuscate(PAL_Instance* pal, PAL_FileMode mode, const char* szfilename, uint8* szdata,
						           int nsize, const byte* key_data, uint32 key_size)
{
	return savefile_mode_obfuscate(pal, szfilename, szdata, nsize, mode, key_data, key_size);
}

CSL_DEF boolean
appendfile(PAL_Instance* pal, const char* szfilename, uint8* szdata, int nsize)
{
	return savefile_mode_obfuscate(pal, szfilename, szdata, nsize, PFM_Append, NULL, 0);
}

CSL_DEF boolean
appendfile_obfuscate(PAL_Instance* pal, const char* szfilename, uint8* szdata, int nsize,
					 const byte* key_data, uint32 key_size)
{
	return savefile_mode_obfuscate(pal, szfilename, szdata, nsize, PFM_Append, key_data, key_size);
}

CSL_DEF boolean
appendpalfile_obfuscate(PAL_Instance* pal, PAL_File* file, uint8* szdata, int nsize,
                        const byte* key_data, uint32 key_size)
{
	return savepalfile_mode_obfuscate(pal, file, szdata, nsize, key_data, key_size);
}


CSL_DEF boolean
removefile(PAL_Instance* pal, const char* szfilename)
{
    boolean ret = TRUE;

    // if file not exist then considered file removed
    if (PAL_FileExists(pal, szfilename) == PAL_Ok)
	{
        PAL_Error result = PAL_FileRemove(pal, szfilename);
        if (result != PAL_Ok)
		{
			ret = FALSE;
		}
    }

    return ret;
}

CSL_DEF boolean
renamefile(PAL_Instance* pal, const char* szoldname, const char* sznewname)
{
    PAL_Error result = PAL_FileRename(pal, szoldname, sznewname);
	if (result != PAL_Ok)
	{
		return 0;
	}
    return 1;
}

boolean
validatefile(PAL_Instance* pal, const char* szfilename, byte* hash)
{
	boolean valid = FALSE;
    PAL_Error result;
    PAL_File* fd = NULL;
	byte* buf = NULL;
	nb_size size;
	byte	filehash[SHS_DIGESTSIZE];

	nsl_memset(filehash,0,sizeof(filehash));

    result = PAL_FileOpen(pal, szfilename, PFM_Read, &fd);

	if (!result) {

		size = readfile2(pal, szfilename, (void**) &buf);

		if (size > 0 && buf != NULL && get_sha_hash(buf, (uint32)size, filehash) == NE_OK)
			valid = (boolean)(nsl_memcmp(hash, filehash, SHS_DIGESTSIZE) == 0);

		PAL_FileClose(fd);
	}

	if (buf != NULL)
		nsl_free(buf);

	return valid;
}

void
update_crc(void* pData, nb_size size, void* pUser)
{
	uint32* pCRC = pUser;

	*pCRC = crc32(*pCRC, pData, size);
}

CSL_DEF boolean
validatefile_crc(PAL_Instance* pal, const char* szfilename, uint32 crc)
{
	return validatefile_crc_obfuscate(pal, szfilename, crc, NULL, 0);
}

boolean
validatefile_crc_obfuscate(PAL_Instance* pal, const char* szfilename, uint32 crc, const byte* key_data, uint32 key_size)
{
	uint32 filecrc;
	boolean valid = FALSE;

	filecrc = 0;

	if (enumfile_obfuscate(pal, szfilename, 1024, update_crc, &filecrc, key_data, key_size) == -1)
		return FALSE;

	valid = (boolean)(filecrc == crc);

	return valid;
}

boolean
validatefile_obfuscate(PAL_Instance* pal, const char* szfilename, byte* hash, const byte* key_data, uint32 key_size)
{
	byte* buf = NULL;
	nb_size size;
	boolean valid = FALSE;
	byte	filehash[SHS_DIGESTSIZE];

	size = readfile_obfuscate(pal, szfilename, (void**) &buf, key_data, key_size);

	if (size > 0 && buf != NULL && get_sha_hash(buf, (uint32)size, filehash) == NE_OK)
		valid = (boolean)(nsl_memcmp(hash, filehash, SHS_DIGESTSIZE) == 0);

	if (buf != NULL)
		nsl_free(buf);

	return valid;
}

int32
read_obfuscate(PAL_File* fd, void * pBuffer, uint32 dwCount, const byte* key_data, uint32 key_size)
{
    PAL_Error result = PAL_Ok;
    uint32 pos = 0;
    uint32 ret = 0;
    uint32 p = 0;

    PAL_FileSetPosition(fd, PFSO_Current, 0);
    PAL_FileGetPosition(fd, &pos);

    ret = dwCount;
    result = PAL_FileRead(fd, pBuffer, dwCount, &ret);

	if (result)
		return 0;

	for (p=0;p<(uint32) ret;p++)
		((byte*) pBuffer)[p] ^= key_data[(pos+p) % key_size];

	return ret;
}

CSL_DEF nb_size
readfilechunk_obfuscate(PAL_Instance* pal, const char* szfilename, int32 offset, uint32 len, void* data, const byte* key_data, uint32 key_size)
{
	nb_size ret;
    PAL_Error result;
	PAL_File* fd = NULL;

	result = PAL_FileOpen(pal, szfilename, PFM_Read, &fd);

	if (result)
		return -1;

	if (PAL_FileSetPosition(fd, PFSO_Start, offset) != PAL_Ok) {
		ret = 0;
		goto errexit;
	}

	ret = read_obfuscate(fd, data, len, key_data, key_size);

errexit:

    PAL_FileClose(fd);

	return ret;
}

PAL_Error
write_obfuscate(PAL_File* fd, uint8* pBuffer, uint32 dwCount, const byte* key_data, uint32 key_size, uint32* written)
{
    PAL_Error result = PAL_Ok;
    uint32 pos = 0;
    uint8*  tmp = NULL;
    uint32 p = 0;

//    DEBUGLOG(LOG_SS_FILE, LOG_SEV_DEBUG, ("write_obfuscate: pos=%u", pos));

    // Get current file position
    PAL_FileGetPosition(fd, &pos);

    if (dwCount == 0)
        return 0;

    tmp = nsl_malloc(dwCount);

    if (tmp == NULL)
    {
        return PAL_ErrNoMem;
    }

    nsl_memcpy(tmp, pBuffer, dwCount);

    for (p = 0; p < dwCount; p++)
    {
        tmp[p] ^= key_data[(pos+p) % key_size];
    }

    *written = 0;
    result = PAL_FileWrite(fd, tmp, dwCount, written);

    nsl_free(tmp);

    return result;
}

static PAL_Error
write_raw(PAL_File* fd, const void* pBuffer, uint32 dwCount, uint32* written)
{
    *written = 0;
    return PAL_FileWrite(fd, (uint8*) pBuffer, dwCount, written);
}

NB_Error
getfreespace(PAL_Instance* pal, uint32* bytes_free, uint32* bytes_total)
{
    if (bytes_free)
        *bytes_free = PAL_FileGetFreeSpace(pal);

    if (bytes_total)
        *bytes_total = PAL_FileGetTotalSpace(pal);

	return NE_OK;
}



NB_Error
copylargefile_obfuscate(PAL_Instance* pal, const char* src, const char* dest, boolean append, const byte* key_data, uint32 key_size)
{
#define COPYBLOCKSIZE	(1024 * 500)
#define ERREXIT(e)		{ result = e; goto errexit; }
    NB_Error result = NE_OK;
    PAL_Error palerr;
    PAL_File* srcfile = NULL;
    PAL_File* destfile = NULL;
    uint8* block = nsl_malloc(COPYBLOCKSIZE);
    uint32 n, bytes_read = 0, written = 0;
    uint32 srcsize = filesize(pal, src);
    uint32 origdestsize = 0;

    if (!block)
        return NE_NOMEM;

    palerr = PAL_FileOpen(pal, src, PFM_Read, &srcfile);
    if (result)
        ERREXIT(NE_FSYS);

    if (!append && !removefile(pal, dest))
        ERREXIT(NE_FSYS);

    if (append)
        origdestsize = filesize(pal, dest);

    palerr = PAL_FileOpen(pal, dest, fileexists(pal, dest) ? PFM_Append : PFM_Create, &destfile);

    if (result)
        ERREXIT(NE_FSYS);

    while ((n = readfilechunk_obfuscate(pal, src, bytes_read, COPYBLOCKSIZE, block, key_data, key_size)) > 0) {

        bytes_read += n;
        result = write_obfuscate(destfile, block, n, key_data, key_size, &written);
        if (result)
        {
            ERREXIT(result);
        }
    }

    // make sure we read all the data
    if (bytes_read != srcsize)
        ERREXIT(NE_FSYS);

errexit:
    nsl_free(block);

    if (srcfile != NULL)
        PAL_FileClose(srcfile);

    if (destfile != NULL) {

        if (result != NE_OK)
            (void)PAL_FileTruncate(destfile, origdestsize);

        PAL_FileClose(destfile);
    }

    return result;
}



CSL_DEF boolean
removedir(PAL_Instance* pal, const char* dirname)
{
    boolean ret = PAL_FileRemoveDirectory(pal, dirname, TRUE);
    return ret;
}

NB_Error
truncatefile(PAL_Instance* pal, const char* filename, uint32 head_bytes, uint32 tail_bytes)
{
	NB_Error result = NE_OK;
    PAL_Error palerr;
	PAL_File* fd = NULL;
	uint32 size = 0;
	byte* buf = NULL;
	uint32 bytes_read = 0;
    uint32 bytes_written = 0;
	uint32 read_pos = 0;
	uint32 chunk_size = TRUNCATEFILE_DEFAULT_CHUNK_SIZE;

	if (!fileexists(pal, filename))
		return NE_OK;

	size = filesize(pal, filename);

	palerr = PAL_FileOpen(pal, filename, PFM_ReadWrite, &fd);

	if (palerr) {
		result = NE_FSYS;
		goto errexit;
	}

	if (head_bytes + tail_bytes >= size) {
		if (PAL_FileTruncate(fd, 0) != PAL_Ok) {
			result = NE_FSYS;
			goto errexit;
		}
		// all done
		goto done;
	}

	if (tail_bytes > 0) {

		if (PAL_FileTruncate(fd, size - tail_bytes) != PAL_Ok) {
			result = NE_FSYS;
			goto errexit;
		}
	}

	if (head_bytes > 0) {

		// read in chunks
		buf = (byte*) nsl_malloc(chunk_size);
		if (!buf) {
			result = NE_NOMEM;
			goto errexit;
		}

		// copy file in chunks, in place
		for (read_pos = head_bytes; TRUE; read_pos += bytes_read) {

			if (PAL_FileGetPosition(fd, &read_pos) != PAL_Ok) {
				result = NE_FSYS;
				goto errexit;
			}

            bytes_read = chunk_size;
			palerr = PAL_FileRead(fd, buf, sizeof(*buf), &bytes_read);

			// exit loop when there's nothing left to read
			if (palerr || bytes_read == 0)
				break;

            bytes_written = bytes_read;
            palerr = PAL_FileWrite(fd, buf, sizeof(*buf), &bytes_written);
            if (palerr || bytes_written != bytes_read) {
				result = NE_FSYS;
				goto errexit;
			}

			read_pos += bytes_read;
		}

		// set new end of file
		if (PAL_FileTruncate(fd, size - (head_bytes + tail_bytes)) != PAL_Ok) {
			result = NE_FSYS;
			goto errexit;
		}
	}

done:
errexit:
	if (buf != NULL)
		nsl_free(buf);
	if (fd != NULL)
		PAL_FileClose(fd);

	return result;
}
