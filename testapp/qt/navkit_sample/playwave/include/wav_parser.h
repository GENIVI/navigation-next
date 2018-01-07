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
    @file wav_parser.h
    @date 11/17/2014
    @addtogroup navigationkit
*/
/*
 * (C) Copyright 2014 by TeleCommunication Systems, Inc.
 *
 * The information contained herein is confidential, proprietary
 * to TeleCommunication Systems, Inc., and considered a trade secret
 * as defined in section 499C of the penal code of the State of
 * California. Use of this information by anyone other than
 * authorized employees of TeleCommunication Systems is granted only
 * under a written non-disclosure agreement, expressly prescribing
 * the scope and manner of such use.
---------------------------------------------------------------------------*/
 
#ifndef __WAV_PARSER_H 
#define __WAV_PARSER_H 
 
typedef unsigned char  uint8_t; 
typedef unsigned short uint16_t; 
typedef unsigned int   uint32_t; 
 
#if __BYTE_ORDER == __LITTLE_ENDIAN 
#define COMPOSE_ID(a,b,c,d) ((a) | ((b)<<8) | ((c)<<16) | ((d)<<24)) 
#define LE_SHORT(v)           (v) 
#define LE_INT(v)               (v) 
#define BE_SHORT(v)           bswap_16(v) 
#define BE_INT(v)               bswap_32(v) 
#elif __BYTE_ORDER == __BIG_ENDIAN 
#define COMPOSE_ID(a,b,c,d) ((d) | ((c)<<8) | ((b)<<16) | ((a)<<24)) 
#define LE_SHORT(v)           bswap_16(v) 
#define LE_INT(v)               bswap_32(v) 
#define BE_SHORT(v)           (v) 
#define BE_INT(v)               (v) 
#else 
#error "Wrong endian" 
#endif 
 
#define WAV_RIFF        COMPOSE_ID('R','I','F','F') 
#define WAV_WAVE        COMPOSE_ID('W','A','V','E') 
#define WAV_FMT         COMPOSE_ID('f','m','t',' ') 
#define WAV_DATA        COMPOSE_ID('d','a','t','a') 
 
/* WAVE fmt block constants from Microsoft mmreg.h header */ 
#define WAV_FMT_PCM             0x0001 
#define WAV_FMT_IEEE_FLOAT      0x0003 
#define WAV_FMT_DOLBY_AC3_SPDIF 0x0092 
#define WAV_FMT_EXTENSIBLE      0xfffe 
 
/* Used with WAV_FMT_EXTENSIBLE format */ 
#define WAV_GUID_TAG "/x00/x00/x00/x00/x10/x00/x80/x00/x00/xAA/x00/x38/x9B/x71" 
 
/* it's in chunks like .voc and AMIGA iff, but my source say there 
   are in only in this combination, so I combined them in one header; 
   it works on all WAVE-file I have 
 */ 
typedef struct WAVHeader { 
    uint32_t magic;     /* 'RIFF' */ 
    uint32_t length;        /* filelen */ 
    uint32_t type;      /* 'WAVE' */ 
} WAVHeader_t; 
 
typedef struct WAVFmt { 
    uint32_t magic;  /* 'FMT '*/ 
    uint32_t fmt_size; /* 16 or 18 */ 
    uint16_t format;        /* see WAV_FMT_* */ 
    uint16_t channels; 
    uint32_t sample_rate;   /* frequence of sample */ 
    uint32_t bytes_p_second; 
    uint16_t blocks_align;  /* samplesize; 1 or 2 bytes */ 
    uint16_t sample_length; /* 8, 12 or 16 bit */ 
} WAVFmt_t; 
 
typedef struct WAVFmtExtensible { 
    WAVFmt_t format; 
    uint16_t ext_size; 
    uint16_t bit_p_spl; 
    uint32_t channel_mask; 
    uint16_t guid_format;   /* WAV_FMT_* */ 
    uint8_t guid_tag[14];   /* WAV_GUID_TAG */ 
} WAVFmtExtensible_t; 
 
typedef struct WAVChunkHeader { 
    uint32_t type;      /* 'data' */ 
    uint32_t length;        /* samplecount */ 
} WAVChunkHeader_t; 
 
typedef struct WAVContainer { 
    WAVHeader_t header; 
    WAVFmt_t format; 
    WAVChunkHeader_t chunk; 
} WAVContainer_t; 
 
int WAV_ReadHeader(int fd, WAVContainer_t *container); 
 
int WAV_WriteHeader(int fd, WAVContainer_t *container); 
 
#endif /* #ifndef __WAV_PARSER_H */ 
