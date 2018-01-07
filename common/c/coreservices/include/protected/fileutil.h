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

/*-
 * fileutil.h: created 2004/07/07 by Mark Goddard.
 *
 * Copyright (c) 2004 Networks In Motion, Inc.
 * All rights reserved.  This file and associated materials are the
 * trade secrets, confidential information, and copyrighted works of
 * Networks In Motion, Inc.
 *
 * This intellectual property is for the internal use only by Networks
 * In Motion, Inc.  This source code contains proprietary information
 * of Networks In Motion, Inc., and shall not be used, copied by, or
 * disclosed to, anyone other than a Networks In Motion, Inc.,
 * certified employee that has written authorization to view or modify
 * said source code.
 *
 */

#ifndef FILE_UTIL_H
#define	FILE_UTIL_H

#include "nberror.h"
#include "paltypes.h"
#include "palfile.h"
#include "pal.h"
#include "cslexp.h"
#include "cslutil.h"

typedef void (*ENUMFILEFN)(void* pData, nb_size size, void* pUser);

nb_size	readfile(PAL_Instance* pal, const char* szfilename, void**);
CSL_DEC nb_size	readfile_obfuscate(PAL_Instance* pal, const char* szfilename, void** ppbuf, const byte* key_data, uint32 key_size);


nb_size enumfile(PAL_Instance* pal, const char* szfilename, nb_size chunk, ENUMFILEFN cb, void* pUser);

CSL_DEC nb_size enumfile_obfuscate(PAL_Instance* pal, const char* szfilename, nb_size chunk, ENUMFILEFN cb, void* pUser, const byte* key_data, uint32 key_size);

CSL_DEC boolean savefile(PAL_Instance* pal, PAL_FileMode mode, const char* szfilename, uint8* szdata, int nsize);

CSL_DEC boolean savefile_obfuscate(PAL_Instance* pal, PAL_FileMode mode, const char* szfilename, uint8* szdata, 
						           int nsize, const byte* key_data, uint32 key_size);
						   
CSL_DEC boolean removefile(PAL_Instance* pal, const char* szfilename);
CSL_DEC boolean renamefile(PAL_Instance* pal, const char* szoldname, const char* sznewname);

CSL_DEC boolean appendfile(PAL_Instance* pal, const char* szfilename, uint8* szdata, int nsize);
CSL_DEC boolean appendfile_obfuscate(PAL_Instance* pal, const char* szfilename, uint8* szdata,
                                     int nsize, const byte* key_data, uint32 key_size);
CSL_DEC boolean appendpalfile_obfuscate(PAL_Instance* pal, PAL_File* file, uint8* szdata,
                                        int nsize, const byte* key_data, uint32 key_size);

CSL_DEC boolean fileexists(PAL_Instance* pal, const char* szfilename);
CSL_DEC uint32	filesize(PAL_Instance* pal, const char* szfilename);

CSL_DEC boolean createdirectory(PAL_Instance* pal, const char* szdirname);
CSL_DEC boolean isdirectory(PAL_Instance* pal, const char* szdirname);
boolean	validatefile(PAL_Instance* pal, const char* szfilename, byte* hash /* 20 bytes */);
boolean	validatefile_obfuscate(PAL_Instance* pal, const char* szfilename, byte* hash /* 20 bytes */, const byte* key_data, uint32 key_size);

CSL_DEC boolean validatefile_crc(PAL_Instance* pal, const char* szfilename, uint32 crc);

boolean validatefile_crc_obfuscate(PAL_Instance* pal, const char* szfilename, uint32 crc, const byte* key_data, uint32 key_size);

CSL_DEC nb_size	readfilechunk_obfuscate(PAL_Instance* pal, const char* szfilename, int32 offset, 
						uint32 len, void* data, const byte* key_data, uint32 key_size);

NB_Error getfreespace(PAL_Instance* pal, uint32* bytes_free, uint32* bytes_total);
NB_Error copylargefile_obfuscate(PAL_Instance* pal, const char* src, const char* dest, boolean append, const byte* key_data, uint32 key_size);
CSL_DEC boolean	 removedir(PAL_Instance* pal, const char* dirname);
NB_Error truncatefile(PAL_Instance* pal, const char* filename, uint32 head_bytes, uint32 tail_bytes);
CSL_DEC int32     read_obfuscate(PAL_File* fd, void * pBuffer, uint32 dwCount, const byte* key_data, uint32 key_size);
CSL_DEC PAL_Error write_obfuscate(PAL_File* fd, uint8* pBuffer, uint32 dwCount, const byte* key_data, uint32 key_size, uint32* written);


#endif
