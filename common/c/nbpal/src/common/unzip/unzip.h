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

    @file     unzip.h
    @defgroup UNZIP_H Extracts ZIP archive

    @brief Provides an unzip utility to extract ZIP archives
*/
/*
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

#ifndef UNZIP_H
#define UNZIP_H

#include "palfile.h"

#ifndef MAX_PATH
#define MAX_PATH    256
#endif

#define DICO_BUFFER_SIZE 32768L             // dictionary/output buffer size
#define COPY_BUFFER 16384                   // copy buffer size

#pragma pack (push, 1)


/*!
Local file header
*/
PACKED struct LocalFileHeader
{
    /*! Signature (0x04034b50) */
    uint32 lfSignature;
    /*! Vers needed to extract */
    uint16  lfExtract,
    /*! General purpose flag */
          lfFlag,
    /*! Compression method */
          lfCompressionMethod,
    /*! File time */
          lfTime,
    /*! File date */
          lfDate;
    /*! CRC-32 for file */
    uint32 lfCrc,
    /*! Compressed size */
          lfCompressedSize,
    /*! Uncompressed size */
          lfUncompressedSize;
    /*! File name length */
    uint16  lfFilenameLength,
    /*! Extra field length */
		  lfExtraFieldLength;
};

#pragma pack (pop)


/*!
Data descriptor
*/
struct DataDescriptor
{
    /*! CRC-32 for file */
    uint32 ddCrc,
    /*! Compressed size */
          ddCompressedSize,
    /*! Uncompressed size */
          ddUncompressedSize;
};


/*!
Central directory file header
*/
struct CentralDirectoryFileHeader
{
    /*! Signature (0x02014b50) */
    uint32 fhSignature;
    /*! Version made by */
    uint16  fhMade,
    /*! Vers needed to extract */
          fhExtract,
    /*! General purpose flag */
          fhFlag,
    /*! Compression method */
          fhCompressionMethod,
    /*! File time */
          fhTime,
    /*! File date */
          fhDate;
    /*! CRC-32 for file */
    uint32 fhCrc,
    /*! Compressed size */
          fhCompressedSize,
    /*! Uncompressed size */
          fhUncompressedSize;
    /*! File name length */
    uint16  fhFilenameLength,
    /*! Extra field length */
          fhExtraFieldLength,
    /*! File comment length */
          fhCommentLength,
    /*! Disk number */
          fhDiskNumber,
    /*! Internal file attrib */
          fhInternalFileAttrib;
    /*! External file attrib */
    uint32 fhExternalFileAttrib,
    /*! Offset of local header */
          fhLocalHeaderOffset;
};


/*!
End of central dir record
*/
struct EndOfCentralDirectory
{
    /*! Signature (0x06054b50) */
    uint32 edSignature;
    /*! This disk number */
    uint16  edDiskNumber,
    /*! Disk w/central dir */
          edCentralDisk,
    /*! Current disk's dir entries */
          edCurrent,
    /*! Total dir entries */
          edTotal;
    /*! Size of central dir */
    uint32 edSize,
    /*! Offset of central dir */
          edOffset;
    /*! Zip file comment length */
    uint16  edZipFileCommentLength;
};


typedef struct LocalFileHeader LF;
typedef struct CentralDirectoryFileHeader FH;
typedef struct DataDescriptor DD;
typedef struct EndOfCentralDirectory ED;


#define LF_SIG              0x0403          // local file header signature
#define FH_SIG              0x0201          // file header signature
#define ED_SIG              0x0605          // end of central dir signature
												// general purpose flag
#define LH_FLAG_ENCRYPT     0x01                // file is encrypted

												// for Method 6 - Imploding
#define LF_FLAG_8K          0x02                // use 8k dictionary vs 4k
#define LF_FLAG_3SF         0x04                // use 3 S-F trees vs 2

													// for Method 8 - Deflating
#define LF_FLAG_NORM        0x00                // normal compression
#define LF_FLAG_MAX         0x02                // maximum compression
#define LF_FLAG_FAST        0x04                // fast compression
#define LF_FLAG_SUPER       0x06                // super fast compression
#define LF_FLAG_DDREC       0x08                // use data descriptor record

												// compression method
#define LF_CM_STORED        0x00                // stored
#define LF_CM_SHRUNK        0x01                // shrunk
#define LF_CM_REDUCED1      0x02                // reduced with factor 1
#define LF_CM_REDUCED2      0x03                // reduced with factor 2
#define LF_CM_REDUCED3      0x04                // reduced with factor 3
#define LF_CM_REDUCED4      0x05                // reduced with factor 4
#define LF_CM_IMPLODED      0x06                // imploded
#define LF_CM_TOKENIZED     0x07                // tokenized (not used)
#define LF_CM_DEFLATED      0x08                // deflated


// LZW Support Structures
#define TABLE_SIZE          0x2001          // dictionary table size
#define ZFREE               0xffff          // free entry value


/*!
Dictionary entry
*/
struct Dictionary
{
    /*! Parent's code */
    uint16 parentCode;
    /*! Replacement character */
    char c;
};

typedef struct Dictionary SD;


// Expanded Structures
#define EXPLODE_DLE         144             // escape character
#define EXPAND_BUFF         4096            // sliding buffer size


/*!
Follower set
*/
struct FollowerSet
{
    uint8 setLength,
         set[32];
};


typedef struct FollowerSet FS;


/*!
Shannon-Fano Tree Structure
*/
struct ShannonFanoTree
{
    /*! Bit length */
    uint8  bitLength,
    /*! Value */
          value;
    /*! Tree bit code */
    uint16 code;
};


typedef struct ShannonFanoTree SFT;


/*!
Huffman Tree Structure
*/
struct  HuffmanTree
{
    /*! Extra bits */
    uint8 extraBits,
    /*! Bit length */
         bitLength;
    union
    {
        /*! Literal, len or distance */
        int	   code;
        /*! Chain pointer */
        struct HuffmanTree *pTable;
    } v;
};


typedef struct HuffmanTree HUFF;


/*!
Unzip class
*/
class CUnzip
{
public:

    CUnzip();
    ~CUnzip();
    uint32 GetCounter() const;
    void SetCounter(uint32 count);
    uint8* Unzip(PAL_File* inputFile, int32& offset);

private:

    long m_crc;

    /*! Bits hold area */
    long m_bHold,
    /*! Remaining size to process */
         m_rSize,
    /*! Keep alive message time */
         m_lastKpal,
    /*! Uncompressed sum */
         m_totalSize,
    /*! Compressed sum */
         m_totalcSize;
    /*! Errorlevel return code */
	int  m_returnCode,
    /*! Positional parms count */
         m_posCount,
    /*! Select file parms count */
         m_selectFileCount,
    /*! End of data counter/flag */
         m_endCount,
    /*! Files processed */
         m_totalFiles;

    /*! Sliding buffer size */
    uint32 m_sBufferSize;
    /*! Bits left to process */
    uint8 m_bSize,
    /*! Sliding buffer, pointer, and end */
	     *m_sBuffer, *m_sBufferPtr, *m_sBufferEnd;

    /*! Start up drive and path */
    char *m_pInitPath,
    /*! Positional parms array */
         **m_pPosParms,
    /*! Keep alive character index */
         m_spin,
    /*! Zip filename and path */
         m_zipFileName[MAX_PATH],
    /*! Base output path */
         m_outputPath[MAX_PATH],
    /*! Create directory switch */
         m_swDirs,
    /*! Freshen target files switch */
         m_swFreshen,
    /*! Retrieve newer files switch */
         m_swNew,
    /*! Overwrite files switch */
         m_swOverwrite,
    /*! Test file integrity switch */
         m_swTest,
    /*! View ZIP directory option */
         m_swView,
    /*! Extract file option */
         m_swExtract,
    /*! Exclude files option */
         *m_swExclude;

    /*! Stores number of bytes written to pZipBuffer */
    uint32 m_counter;

    /*! Input file */
    PAL_File *iFile,
    /*! Output file */
             *oFile;

	uint8* pZipBuffer;
	uint8* pZipOut;


// See source file for function descriptions
    int	    ExtractOpen(LF *pFileHeader);
    void    StoreChar(uint8 c);
    void    ExtractFlush(void);
    int	    LookAtCode (int bits);
    uint32  GetCode(int bits);
    int	    GetByte(void);
    void    ExtractCopy(uint32 len);
    void    ExpLoadTree(SFT *sf, int n);
    int	    ExpRead(SFT *sf);
    int	    InfBuildTree(uint32 *b, uint32 n, uint32 s, uint32 *d, uint32 *e, HUFF **t, uint32 *m);
    void    InfFree(HUFF *t);
    int	    InfCodes(HUFF *tl, HUFF *td, int  bl, int  bd);
    int	    InfFixed(void);
    int	    InfDynamic(void);
    void    ExtractInflate(LF *pFileHeader);
    void    ExtractExplode(LF *pFileHeader);
    void    ExtractExpand(LF *pFileHeader);
    void    ExtractShrunk(void);
    void    ExtractStored(void);
    void    ExtractZip(int fnc, LF *pFileHeader);
    void    ReleaseBuffer();
};

#endif

/*! @} */
