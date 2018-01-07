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

    @file     unzip.cpp
    @defgroup UNZIP_H Extracts ZIP archive
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

#ifdef WIN32
#pragma warning( disable : 4005 )
#pragma warning( disable : 4018 )
#pragma warning( disable : 4305 )
#pragma warning( disable : 4309 )
#endif

#include <unzip.h>
#include "crc32.h"
#include <quicksort.h>
#include <globals.h>

#define LOCAL_FILE_HEADER_SIGNATURE    0x04034b50


CUnzip::CUnzip()
{
    pZipBuffer = NULL;
    pZipOut	= NULL;
    m_returnCode = 1;
    m_sBufferSize = 0;
    m_counter = 0;
    m_sBuffer = m_sBufferPtr = m_sBufferEnd = NULL;
    m_bHold = 0;
    m_rSize = 0;
    m_totalSize = 0;
    m_totalcSize = 0;
    m_posCount = 0;
    m_selectFileCount = 0;
    m_endCount = 0;
    m_totalFiles = 0;
    m_bSize = 0;
    iFile = NULL;
    oFile = NULL;
    m_pInitPath = NULL;
    m_spin = 0;
    m_swTest = 0;
    m_swView = 0;
    m_swExtract = 0;
    m_swExclude = NULL;
    m_swFreshen = 0;
    m_swOverwrite = 0;
    m_swNew = 0;
    m_swDirs = 0;
    m_spin = 0;
}


CUnzip::~CUnzip()
{
    ReleaseBuffer();
}


uint32 CUnzip::GetCounter() const
{
    return m_counter;
}

void CUnzip::SetCounter(uint32 count)
{
    m_counter = count;
}


/*! Reades 1 file from ZIP archive and writes its contents into pZipBuffer

@return uint8* pZipBuffer

@param inputFile Pointer to IFILE
@param offset Stores uncompressed size of the file from archive, that is size of pZipBuffer

*/
uint8* CUnzip::Unzip(PAL_File* inputFile, int32& offset)
{
    LF	fileHeader;
    uint32 bytesRead = 0;
    ReleaseBuffer();
    offset = 0;
    uint8* tempFileHeader = NULL;
    iFile = inputFile;

    tempFileHeader = static_cast<uint8*>(nsl_malloc(sizeof(fileHeader)));
    if (NULL == tempFileHeader)
    {
        return NULL;
    }
    nsl_memset(tempFileHeader, 0, sizeof(fileHeader));

    PAL_FileRead(iFile, tempFileHeader, sizeof(fileHeader), &bytesRead);
    nsl_memcpy(&fileHeader, tempFileHeader, sizeof(fileHeader));
    nsl_free(tempFileHeader);

    PAL_FileSetPosition(iFile, PFSO_Current, fileHeader.lfFilenameLength + fileHeader.lfExtraFieldLength);
    offset = sizeof(fileHeader) + fileHeader.lfFilenameLength + fileHeader.lfExtraFieldLength + fileHeader.lfCompressedSize;
    SetCounter(fileHeader.lfUncompressedSize);

    if (fileHeader.lfSignature == LOCAL_FILE_HEADER_SIGNATURE)
    {
        ExtractZip(0, &fileHeader);
        ExtractZip(1, &fileHeader);
        ExtractZip(2, &fileHeader);
        return pZipBuffer;
    }
    else
    {
        // If signature != 0x04034b50 this means that there is no more unprocessed files left in the archive
        // Here pZipBuffer = NULL
        return pZipBuffer;
    }
}


/*! Allocates necessary amount of memory for pZipBuffer and pZipOut

@param pFileHeader Local file pointer

*/
int CUnzip::ExtractOpen(LF *pFileHeader)
{
    int	size = pFileHeader->lfUncompressedSize;
    pZipBuffer = pZipOut = static_cast<uint8*>(malloc_chk(size + 1));
    SetCounter(size + 1);  // Set number of bytes will be written to pZipBuffer
    return(0);             // ..and process this file
}


/*!
Stores character in output file
*/
void CUnzip::StoreChar(uint8 c)    /*!< Character to store */
{
    crc_32(c, (uint32 *) &m_crc);          // determine current crc 32
    *m_sBufferPtr = c;                   // a. yes .. put char in buffer

    if (++m_sBufferPtr >= m_sBufferEnd)  // q. past end of buffer?
    {									 // a. yes .. write buffer
        nsl_memcpy(pZipOut, m_sBuffer, m_sBufferSize);
        pZipOut += m_sBufferSize;
        m_sBufferPtr = m_sBuffer;        // ..then back to start again
    }
}


/*!
Write output buffer as needed
*/
void CUnzip::ExtractFlush(void)
{
    if (m_sBufferPtr != m_sBuffer)       // q. local buffer need flushing?
    {									 // a. yes .. flush local buffer
        nsl_memcpy( pZipOut, m_sBuffer, (uint32) (m_sBufferPtr - m_sBuffer) );
        pZipOut += (uint32) (m_sBufferPtr - m_sBuffer);
    }
}


/*!
Looks at the next code from input file
*/
int CUnzip::LookAtCode (int bits)           /*!< Number of bits */
{
    static int    bitMask[] =               // bit masks
    {
        0x0000,
        0x0001, 0x0003, 0x0007, 0x000f,
        0x001f, 0x003f, 0x007f, 0x00ff,
        0x01ff, 0x03ff, 0x07ff, 0x0fff,
        0x1fff, 0x3fff, 0x7fff, 0xffff
    };

    while (m_bSize < bits)                    // get enough bits
    {
        if (m_rSize > 0)                      // q. anything left to process?
        {
            uint8 temp = 0;
            uint32 bytesRead = 0;
            PAL_FileRead(iFile, &temp, 1, &bytesRead);
            m_bHold |= (uint32)temp << m_bSize;              // a. yes .. get a character
            m_bSize += 8;                      // ..and increment counter
            --m_rSize;                         // ..finally, decrement cnt
        }
        else if (!m_endCount)               // q. done messing around?
        {
            ExtractFlush();                    // a. yes .. flush output buffer
            return 0; //quit_with(data_error);              // ..then quit with an error msg
        }
        else if (m_endCount > 0)               // q. down counting?
        {
            m_endCount--;                      // a. yes .. decriment counter
            return(-1);                        // ..and return all done
        }
        else
        {
            m_endCount++;                       // else .. count up
            break;                              // ..and exit loop
        }
    }

    return((static_cast<int>(m_bHold)) & bitMask[bits]);    // return just enough bits
}


/*!
Gets next code from input file
*/
uint32 CUnzip::GetCode(int bits)                  // nbr of bits in this code
{
    uint32    accumulator = 0;

    accumulator = LookAtCode(bits);             // prepare return value
    m_bHold >>= bits;                           // ..shift out requested bits
    m_bSize -= bits;                            // ..decrement remaining bit count
    return(accumulator);                        // ..and return value
}


/*!
Gets next byte from input file
*/
int CUnzip::GetByte(void)
{
    if (m_rSize > 0)                             // q. anything left?
    {
        uint32 bytesRead = 0;
        uint8 c = 0;
        --m_rSize;                               // a. yes .. decrement count
        PAL_FileRead(iFile, &c, 1, &bytesRead);
        return static_cast<int>(c);                           // ..and return w/character
    }
    else
    {
        return(-1);                              // else .. return error
    }
}


/*!
Copies stored data to output stream
*/
void CUnzip::ExtractCopy(uint32 len)             /*!< Length of copy */
{
    uint8  *pBuffer = NULL,                             // work buffer pointer
           *pWorkPtr = NULL;                            // work pointer
    uint32  currentReadSize = 0;                 // current read size

    pBuffer = static_cast<uint8*>(malloc_chk(static_cast<uint32>(COPY_BUFFER)));// get an file buffer

    while (len)                                  // loop copying file to output
    {
        currentReadSize = (len <= COPY_BUFFER) ? // determine next read size
            (int) len : (uint32) COPY_BUFFER;

        uint32 bytesRead = 0;
        PAL_FileRead(iFile, pBuffer, currentReadSize, &bytesRead);
        if(bytesRead != currentReadSize)          // q. read ok?
        {
            nsl_free(pBuffer);
            return;
        }
        m_rSize -= currentReadSize;               // decrement remaining length
        len -= currentReadSize;                   // ..and loop control length

        for (pWorkPtr = pBuffer; currentReadSize--;)         // for the whole block
        {
            StoreChar(*pWorkPtr++);               // ..write output data
        }
    }
    nsl_free(pBuffer);                            // ..and release file buffer
}


/*!
Loads a single S-F tree
*/
void CUnzip::ExpLoadTree(SFT *sf,            /*!< Base S-F tree pointer */
                      int n)                 /*!< number of entries */
{
    uint32  currentCharacter = 0,                     // current character read
            bitLength = 0,                            // bit length
            numberOfEntries = 0,                      // number of entries
            lastBitIncrement = 0,                     // last bit increment
            codeIncrement = 0,                        // code increment
            i = 0, j = 0,                                 // loop control
            bytesToRead = 0;                          // bytes to read
    SFT    *pWork = NULL;                                // work pointer


    if ((currentCharacter = GetByte()) == 0xffff)          // q. read length ok?
    {
        return;                                   // a. no .. just return
    }

    bytesToRead = currentCharacter + 1;           // get number of active elements

    for (pWork = sf, i = 0; bytesToRead--;)       // loop thru filling table
    {                                             // ..with lengths
        if ((currentCharacter = GetByte()) == 0xffff)       // q. get code ok?
        {
            return;                               // a. no .. just return
        }

        bitLength = (currentCharacter & 0x0f) + 1;// save bit length
        numberOfEntries = (currentCharacter >> 4) + 1;             // ..and nbr of values at bitLength

        for (j = 0; j < numberOfEntries; j++, i++, pWork++)    // loop filling this bit length
        {
            pWork->bitLength = static_cast<uint8>(bitLength);                // save bit length
            pWork->value = static_cast<uint8>(i);                    // ..and the relative entry nbr
        }
    }

    quicksort(static_cast<void*>(sf), n, sizeof(SFT), exp_cmp);// sort the bit lengths

    currentCharacter = lastBitIncrement = codeIncrement = 0;      // init code, last bit len ..
                                                    // ..and code increment
    for (pWork = &sf[n - 1]; pWork >= sf; pWork--)  // loop thru tree making codes
    {
        currentCharacter += codeIncrement;          // build current code
        pWork->code = currentCharacter;             // ..and store in tree

        if (pWork->bitLength != lastBitIncrement)   // q. bit lengths change?
        {
            lastBitIncrement = pWork->bitLength;    // a. yes .. save current value
            codeIncrement = (1 << (16 - lastBitIncrement));              // ..and bld new code increment
        }
    }

    for (pWork = sf, j = n; j--; pWork++)           // loop thru reversing bits
    {
        currentCharacter = pWork->code;             // get code entry

        for (i = 16, codeIncrement = 0; i--;)       // loop doing the bit work
        {
            codeIncrement = (codeIncrement << 1) | (currentCharacter & 1);           // building the reversed code
            currentCharacter >>= 1;                 // ..one bit at a time
        }

    pWork->code = codeIncrement;                    // put new code back
    }
}


/*!
Reads the input stream returning an S-F tree value
*/
int CUnzip::ExpRead(SFT *sf)          /*!< Base S-F tree pointer */
{
    uint32  current = 0,                                 // read code
            code = 0,                                    // working bit string
            bits = 0;                                    // current nbr of bits
    SFT     *p = sf;                                 // S-F pointer

    for (code = bits = 0;;)                          // loop to find the code
    {
        if ((current = GetCode(1)) == 0xffff)        // q. get a bit ok?
        {
            return(-1);                              // a. no .. return w/err code
        }

        current <<= bits++;                           // shift left a little
        code |= current;                              // save next bit

        for (;;)                                      // loop to find code
        {
            if (p->bitLength > bits)                  // q. read enough bits yet?
            {
                break;                                // a. no .. get another one
            }

            else if (p->bitLength == bits)           // q. read just enough?
            {                                   // a. yes .. check entries
                if (p->code == code)            // q. find proper entry?
                {
                    return (p->value);          // a. yes .. return w/code
                }
            }

            ++p;                                // bump index
        }
    }
}


#define MAX_BITS    16                      // maximum bits in code
#define CODE_MAX    288                     // maximum nbr of codes in set


/*!
Builds a Huffman tree

@return TRUE if error building Huffman tree
        FALSE if tree built
*/
int CUnzip::InfBuildTree(uint32 *b,             /*!< Code lengths in bits */
                         uint32 n,              /*!< Number of codes */
                         uint32 s,              /*!< Number of simple codes */
                         uint32 *d,             /*!< Base values for non-simple */
                         uint32 *e,             /*!< List of extra bits */
                         HUFF **t,            /*!< Resulting table */
                         uint32 *m)             /*!< Maximum lookup bits */
{
    uint32  a = 0,                                  // code lengths of k
            c[MAX_BITS + 1] = {0},                    // bit length count table
            f = 0,                                  // i repeats every f entries
            i = 0, j = 0,                               // loop control
            *pWork = NULL,                            // work pointer
            v[CODE_MAX + 1] = {0},                        // values in order of bit length
            bitOffsets[MAX_BITS + 1] = {0},           // bit offsets
            *xp = NULL,                                 // pointer
            curEntries = 0;                         // entries in current table
    int     maxCodeLength = 0,                       // max code length
            tableLevel = 0,                         // table level
            k = 0,                                  // loop control
            maxBitsLength = 0,                      // max bits length
            bitsBeforeTable = 0,                      // bits before this table
            dummyEntriesNbr = 0;                    // number of dummy entries
    HUFF    workEntry,                                  // work entry
           *curTableEntry = NULL,                  // current table entry
           *tableStack[MAX_BITS] = {NULL};                        // table stack


    nsl_memset(c, 0, sizeof(c));                    // clear table to nulls


    for (pWork = b, i = n; i--;)                    // loop thru table to generate
    {
        c[*pWork++]++;                              // ..counts for each bit length
    }

    if (c[0] == n)                              // q. all zero lengths?
    {
        *t = 0;                                 // a. yes .. clear result..
        *m = 0;                                 // ..pointer and count
        return(FALSE);                          // ..and return all ok
    }

                                            // find min and max code lengths
    for (maxBitsLength = *m, j = 1; j <= MAX_BITS; j++)     // loop to find minimum code len
    {
        if (c[j])
        {                                     // q. find the min code length?
            break;
        }
    }                                  // a. yes .. exit the loop

    k = j;                                      // save minimum code length

    if ((uint32) maxBitsLength < j)                           // q. minimum greater than lookup?
    {
        maxBitsLength = j;                                  // a. yes .. set up new lookup
    }

    for (i = MAX_BITS; i; i--)                  // loop to find max code length
    {
        if (c[i])                               // q. find a used entry?
        {
            break;                              // a. yes .. exit loop
        }
    }

    maxCodeLength = i;                                      // save maximum code length

    if ((uint32) maxBitsLength > i)                           // q. lookup len greater than max?
    {
        maxBitsLength = i;                                  // a. yes .. set up new look len
    }

    *m = maxBitsLength;                                     // return new lookup to caller

    for (dummyEntriesNbr = 1 << j; j < i; j++, dummyEntriesNbr <<= 1)       // loop to adjust last length codes
    {
        if ((dummyEntriesNbr -= c[j]) < 0)                    // q. more codes than bits?
        {
            return(TRUE);                       // a. yes .. return w/error flag
        }
    }

    if ((dummyEntriesNbr -= c[i]) < 0)                        // q. more than max entry's count?
    {
        return(TRUE);                           // a. yes .. return w/error flag
    }

    c[i] += dummyEntriesNbr;                                  // adjust last length code


    bitOffsets[1] = j = 0;                               // initialize code stack
    for (pWork = c + 1, xp = &bitOffsets[2], i = maxCodeLength; i--;)    // loop thru generating offsets
    {
        *xp++ = (j += *pWork++);                    // ..into the entry for each length
    }


    for (pWork = b, i = 0; i < n; i++)              // make table of value in order ..
    {
        if ((j = *pWork++) != 0)                    // ..by bit lengths
        {
            v[bitOffsets[j]++] = i;
        }
    }


    bitOffsets[0] = i = 0;                               // first Huffman code is all zero
    pWork = v;                                      // use the values array
    tableLevel = -1;                                     // no tables yet, level = -1
    bitsBeforeTable = -maxBitsLength;                                     // bits decoded
    curTableEntry = tableStack[0] = (HUFF *) 0;                      // clear stack and pointer
    curEntries = 0;                                      // ..and number of entries

    for (; k <= maxCodeLength; k++)                         // loop from min to max bit lengths
    {
        for (a = c[k]; a--;)                    // process entries at this bit len
        {
            while (k > bitsBeforeTable + maxBitsLength)                   // build up tables to k length
            {
                tableLevel++;                            // increment table level
                bitsBeforeTable += maxBitsLength;                         // add current nbr of bits

                curEntries = (curEntries = maxCodeLength - bitsBeforeTable) > static_cast<uint32>(maxBitsLength)      // determine the number of entries
                    ? maxBitsLength : curEntries;                // ..in the current table

                if ((f = 1 << (j = k - bitsBeforeTable))      // q. k-bitsBeforeTable bit table contain enough?
                    > a + 1)
                {
                    f -= a + 1;                 // a. too few codes for k-bitsBeforeTable bit tbl
                    xp = c + k;                 // ..deduct codes from patterns left

                    while (++j < curEntries)             // loop to build upto z bits
                    {
                        if ((f <<= 1) <= *++xp) // q. large enough?
                        {
                            break;              // a. yes .. to use j bits
                        }

                        f -= *xp;               // else .. remove codes
                    }
                }

                curEntries = 1 << j;            // entries for j-bit table


                curTableEntry = static_cast<HUFF *>(malloc_chk(        // get memory for new table
                    (curEntries + 1) * sizeof(HUFF)));

                *t = curTableEntry + 1;          // link to main list
                *(t = &(curTableEntry->v.pTable)) = 0;       // clear ptr, save address
                tableStack[tableLevel] = ++curTableEntry;                     // table starts after link


                if (tableLevel)                 // q. is there a last table?
                {
                    bitOffsets[tableLevel] = i;                   // a. yes .. save pattern
                    workEntry.bitLength = static_cast<uint8>(maxBitsLength);          // bits to dump before this table
                    workEntry.extraBits = static_cast<uint8>(16 + j);     // bits in this table
                    workEntry.v.pTable = curTableEntry;              // pointer to this table
                    j = i >> (bitsBeforeTable - maxBitsLength);           // index to last table
                    tableStack[tableLevel-1][j] = workEntry;              // connect to last table
                }
            }


            workEntry.bitLength = static_cast<uint8>(k - bitsBeforeTable);            // set up table entry

            if (pWork >= v + n)                     // q. out of values?
            {
                workEntry.extraBits = 99;                      // a. yes .. set up invalid code
            }
            else if (*pWork < s)                   // q. need extra bits?
                 {
                    workEntry.extraBits = static_cast<uint8>(*pWork < 256         // a. yes .. set up extra bits
                        ? 16 : 15);             // 256 is end-of-block code
                    workEntry.v.code = *pWork++;                // simple code is just the value
                 }
                 else
                 {
                    workEntry.extraBits = static_cast<uint8>(e[*pWork - s]);        // non-simple--look up in lists
                    workEntry.v.code = d[*pWork++ - s];
                 }


            f = 1 << (k - bitsBeforeTable);        // fill code-like entries with workEntry

            for (j = i >> bitsBeforeTable; j < curEntries; j += f)
            {
                curTableEntry[j] = workEntry;
            }


            for (j = 1 << (k - 1); i & j; j >>= 1)  // backwards increment the
            {
                i ^= j;                             // ..k-bit code i
            }

            i ^= j;


            while ((i & ((1 << bitsBeforeTable) - 1)) != bitOffsets[tableLevel])
            {
                tableLevel--;                            // decrement table level
                bitsBeforeTable -= maxBitsLength;       // adjust bits before this table
            }
        }
    }

return(FALSE);                              // return FALSE, everything ok
}


/*!
Frees malloc'd Huffman tables

@param t Base table to free
*/
void CUnzip::InfFree(HUFF *t)
{
    HUFF   *p = NULL;                                  // work pointer

    while (t)                                   // loop thru freeing memory
    {
        p = (--t)->v.pTable;                     // get next table address
        nsl_free(t);                                // free current table
        t = p;                                  // establish new base pointer
    }
}


/*!
Inflates the codes using the Huffman trees
*/
int CUnzip::InfCodes(HUFF *tl,                 /*!< Literal table */
                     HUFF *td,                 /*!< Distance table */
                     int  bl,                  /*!< Literal bit length */
                     int  bitLengthDistance)   /*!< Distance bit length */
{
    int current = 0,                           // current retrieved code
        extraBits = 0,                         // extra bits
        n = 0, d = 0;                          // length and distance
    uint8   *p = NULL;                          // work pointer
    HUFF   *t = NULL;                          // current Huffman tree

    for (;;)                                            // loop till end of block
    {
        current = LookAtCode(bl);                       // get some bits

        if ((extraBits = (t = tl + current)->extraBits) > 16)        // q. in this table?
            do
            {                                           // a. no .. loop reading codes
                if (extraBits == 99)                    // q. invalid entry?
                {
                    return(TRUE);                       // a. yes .. return an error
                }

                GetCode(t->bitLength);                  // read some bits
                extraBits -= 16;                        // nbr of bits to get
                current = LookAtCode(extraBits);        // get some bits
            } while ((extraBits = (t = t->v.pTable + current)->extraBits) > 16);

        GetCode(t->bitLength);                          // read some processed bits

        if (extraBits == 16)                            // q. literal code?
        {
            StoreChar(t->v.code);                       // a. yes .. output code
        }
        else
        {
            if (extraBits == 15)                        // q. end of block?
            {
                return(FALSE);                          // a. yes .. return all ok
            }

            n = GetCode(extraBits) + t->v.code;         // get length code
            current = LookAtCode(bitLengthDistance);                   // get some bits

            if ((extraBits = (t = td + current)->extraBits) > 16)    // q. in this table?
                do
                {                                        // a. no .. loop thru
                    if (extraBits == 99)                 // q. invalid entry?
                    {
                        return(TRUE);                    // a. yes .. just return
                    }

                    GetCode(t->bitLength);               // read some bits
                    extraBits -= 16;                     // number of bits to get
                    current = LookAtCode(extraBits);     // get some bits
                } while ((extraBits = (t = t->v.pTable + current)->extraBits) > 16);

            GetCode(t->bitLength);                       // read some processed bits
            d = t->v.code + GetCode(extraBits);          // get distance value

            if ((m_sBufferPtr - m_sBuffer) >= d)         // q. backward wrap?
            {
                p = m_sBufferPtr - d;                    // a. no .. just back up a bit
            }
            else
                p = m_sBufferSize - d + m_sBufferPtr;    // else .. find at end of buffer

            while (n--)                                  // copy previously outputed
            {                                            // ..strings from sliding buffer
                StoreChar(*p);                           // put out each character

                if (++p >= m_sBufferEnd)                 // q. hit the end of the buffer?
                {
                    p = m_sBuffer;                       // a. yes .. back to beginning
                }
            }
        }
    }
}


/*!
Inflates a fixed Huffman code block
*/
int CUnzip::InfFixed(void)
{
    uint32  i = 0,                                  // loop control
            bitLength = 7,                      // bit length for literal codes
            bitLengthDistance = 5,              // ..and distance codes
            l[288] = {0};                             // length list
    HUFF   *tl = NULL, *td = NULL;                            // literal and distance trees


    for (i = 0; i < 144; i++)                   // set up literal table
    {
        l[i] = 8;
    }

    for (; i < 256; i++)                        // ..fixing up ..
    {
        l[i] = 9;
    }

    for (; i < 280; i++)                        // ..all the
    {
        l[i] = 7;
    }

    for (; i < 288; i++)                        // ..entries
    {
        l[i] = 8;
    }

    if (InfBuildTree(l, 288, 257, cll,        // q. build literal table ok?
            cle, &tl, &bitLength))
    {
        return(TRUE);                           // a. no .. return with error
    }

    for (i = 0; i < 30; i++)                    // set up the distance list
    {
        l[i] = 5;                               // ..to 5 bits
    }

    if (InfBuildTree(l, 30, 0, cdo, cde,      // q. build distance table ok?
            &td, &bitLengthDistance))
    {
        return(TRUE);                           // a. no .. return with error
    }

    if (InfCodes(tl, td, bitLength, bitLengthDistance))              // q. inflate file ok?
    {
        return(TRUE);                           // a. no .. return with error
    }

    InfFree(tl);                               // free literal trees
    InfFree(td);                               // ..and distance trees
    return(FALSE);                              // ..and return all ok
}


/*!
Inflates a dynamic Huffman code block
*/
int CUnzip::InfDynamic(void)
{
    uint32  c = 0,                                  // code read from input stream
            i = 0, j = 0,                               // loop control
            lastLength = 0,                         // last length
            nbrOfLengths = 0,                       // nbr of lengths to get
            bl = 0, bitLengthDistance = 0,                             // literal and distance bit len
            nl = 0, nd = 0,                             // literal and distance codes
            nb = 0,                                 // nbr of bit length codes
            ll[286 + 30] = {0};                       // literal length and dist codes
    HUFF   *tl = NULL, *td = NULL;                            // literal and distance trees


    nl = GetCode(5) + 257;                     // get nbr literal len codes
    nd = GetCode(5) + 1;                       // ..and the nbr dist len codes
    nb = GetCode(4) + 4;                       // ..and nbr of of bit lengths

    for (j = 0; j < nb; j++)                    // read in bit length code
    {
        ll[bll[j]] = GetCode(3);               // set up bit lengths
    }

    for (; j < 19; j++)                         // loop thru clearing..
    {
        ll[bll[j]] = 0;                         // ..other lengths
    }

    bl = 7;                                     // set literal bit length

    if (InfBuildTree(ll, 19, 19,              // q. build decoding table for
            0, 0, &tl, &bl))                // ..trees using 7 bit lookup ok?
    {
        return(TRUE);                           // a. no .. return with error
    }


    nbrOfLengths = nl + nd;                                // number of lengths to get

    for (i = lastLength = 0; i < nbrOfLengths;)                     // get literal and dist code lengths
    {
        c = LookAtCode(bl);                    // get some bits
        j = (td = tl + c)->bitLength;                // get length code from table
        GetCode(j);                            // use those bits
        j = td->v.code;                         // ..then get code from table

        if (j < 16)                             // q. save length?
        {
            ll[i++] = lastLength = j;                    // a. yes .. also save last length
        }
        else if (j == 16)                      // q. repeat last length 3 to 6x?
             {
                j = GetCode(2) + 3;                // get repeat length code

                if (i + j > nbrOfLengths)                      // q. past end of array?
                {
                    return(TRUE);                   // a. yes .. return with error
                }

                while (j--)                         // else .. loop filling table
                {
                    ll[i++] = lastLength;                    // ..with last length
                }
             }
        else if (j == 17)                      // q. 3 to 10 zero length codes?
             {
                j = GetCode(3) + 3;                // a. yes .. get repeat code

                if (i + j > nbrOfLengths)                      // q. past end of array?
                {
                    return(TRUE);                   // a. yes .. return with error
                }

                while (j--)                         // else .. loop filling table
                {
                    ll[i++] = 0;                    // ..with zero length
                }

                lastLength = 0;                              // ..and save new last length
            }
            else                                   // else .. j == 18 and
            {                                   // ..generate 11 to 138 zero codes
                j = GetCode(7) + 11;               // get repeat code

                if (i + j > nbrOfLengths)                      // q. past end of array?
                {
                    return(TRUE);                   // a. yes .. return with error
                }

                while (j--)                         // else .. loop filling table
                {
                    ll[i++] = 0;                    // ..with zero length
                }
                lastLength = 0;                              // ..and save new last length
            }
    }

    InfFree(tl);                                // finally, free literal tree

    bl = 9;                                     // length of literal bit codes

    if (InfBuildTree(ll, nl, 257,               // q. build literal table ok?
            cll, cle, &tl, &bl))
    {
        return(TRUE);                           // a. no .. return with error
    }

    bitLengthDistance = 6;                                     // length of distance bit codes

    if (InfBuildTree(ll + nl, nd, 0,            // q. build distance table ok?
            cdo, cde, &td, &bitLengthDistance))
    {
        return(TRUE);                           // a. no .. return with error
    }

    if (InfCodes(tl, td, bl, bitLengthDistance))              // q. inflate block ok?
    {
        return(TRUE);                           // a. no .. return with error
    }

    InfFree(tl);                                // free literal trees
    InfFree(td);                                // ..and distance trees
    return(FALSE);                              // then finally, return all ok
}


/*!
Extracts a deflated file
*/
void CUnzip::ExtractInflate(LF *pFileHeader)            // local file pointer
{
    uint32  current = 0,                            // current read character
            eoj = 0;                            // end of job flag

    if (pFileHeader->lfFlag & LF_FLAG_DDREC)            // q. need data descriptor rec?
    {
		return; //quit_with(not_supported);     // a. yes .. quit w/error msg
    }

    m_endCount = -m_endCount;                   // set end count to negative

    while (!eoj)                             // loop till end of job
    {
        eoj = GetCode(1);                       // get the eoj bit
        current = GetCode(2);                   // ..then get block type
        uint8 c = 0;
        uint32 bytesRead = 0;

        switch (current)                        // depending on block type
        {
            case 0:                             // 0: stored block
                m_bSize = 0;                    // flush remaining bits
                current = GetCode(16);          // get block length ok

                PAL_FileRead(iFile, &c, 1, &bytesRead);
                PAL_FileRead(iFile, &c, 1, &bytesRead);
                m_rSize -= 2;                   // ..and its count

                ExtractCopy(current);           // copy bytes to output stream
                break;                          // ..and get next block

            case 1:                             // 1: fixed Huffman codes
                eoj |= InfFixed();              // process a fixed block
                break;                          // ..then get another block

            case 2:                             // 2: dynamic Huffman codes
                eoj |= InfDynamic();            // process the dynamic block
                break;                          // ..then get next block

            case 3:                             // 3: unknown type
				//quit_with(data_error);        // quit with an error message
                break;

            default:
                break;
        }
    }
}


/*!
Extracts an imploded file
*/
void CUnzip::ExtractExplode(LF *pFileHeader)            // local file pointer
{
    int     current = 0,                                  // current read character
            ltf = 0,                                // literal S-F tree available
            db = 0,                                 // dictionary read bits
            distance = 0,                                  // distance
            len = 0,                                // ..and length to go back
            mml = 0;                                // minimum match length 3
    uint8   *p = NULL;                                  // work dictionary pointer
    SFT    *sft = NULL, *sft2 = NULL, *sft3 = NULL; // S-F trees pointers

    sft = static_cast<SFT *>(malloc_chk(                   // get memory for S-F trees
                (256 + 64 + 64) * sizeof(SFT)));

    sft2 = &sft[256];                           // ..and set up ..
    sft3 = &sft[320];                           // ..the base pointers

    db = (pFileHeader->lfFlag & LF_FLAG_8K) ? 7 : 6;    // ..and dictionary read in bits

    mml = ((ltf = pFileHeader->lfFlag & LF_FLAG_3SF)    // set literal S-F tree available
				!= 0) ? 3 : 2;                  // ..and minimum match lengths

    if (ltf)                                    // q. literal tree available?
    {
        ExpLoadTree(sft, 256);                  // a. yes .. load literal tree
    }

    ExpLoadTree(sft2, 64);                      // ..then load length trees
    ExpLoadTree(sft3, 64);                      // ..and finally, distance trees

    for (;;)                                    // loop processing compressed data
    {
        if ((current = GetCode(1)) == 0xffff)        // q. get a bit ok?
            break;                              // a. no .. exit loop

        if (current)                                  // q. encoded literal data?
        {                                   // a. yes .. continue processing
            if (ltf)                            // q. literal S-F tree available?
            {                               // a. yes .. get char from tree
                if ((current = ExpRead(sft)) == 0xffff)  // q. get char from tree ok?
                {
                    break;                          // a. no .. exit loop
                }
            }
            else if ((current = GetCode(8)) == 0xffff)  // q. get next character ok?
            {
                break;                              // a. no .. exit loop
            }

            StoreChar(current);                      // ..and put char to output stream
        }
        else                                   // else .. use sliding dictionary
        {
            if ((distance = GetCode(db)) == 0xffff)   // q. get distance code ok?
            {
                break;                          // a. no .. exit loop
            }

            if ((current = ExpRead(sft3)) == 0xffff) // q. get distance S-F code ok?
            {
                break;                          // a. no .. exit loop
            }

            distance = (distance | (current << db)) + 1;   // update distance

            if ((len = ExpRead(sft2)) == 0xffff)   // q. get length S-F code ok?
            {
            break;                              // a. no .. exit loop
            }

            if (len == 63)                      // q. get max amount?
            {                               // a. yes .. get another byte
                if ((current = GetCode(8)) == 0xffff)// q. get additional len ok?
                {
                    break;                      // a. no .. exit loop
                }

                len += current;                 // ..then add to overall length
            }

            len += mml;                         // add in minimum match length

            if ((m_sBufferPtr - m_sBuffer) >= distance)      // q. backward wrap?
            {
                p = m_sBufferPtr - distance;                  // a. no .. just back up a bit
            }
            else
            {
                p = m_sBufferSize - distance + m_sBufferPtr;  // else .. find at end of buffer
            }

            while (len--)                       // copy previously outputed
            {                               // ..strings from sliding buffer
                StoreChar(*p);                 // put out each character

                if (++p >= m_sBufferEnd)                 // q. hit the end of the buffer?
                {
                    p = m_sBuffer;                     // a. yes .. back to beginning
                }
            }
        }
    }

    nsl_free(sft);                                  // free S-F trees
}


/*!
Extracts a reduced file
*/
void CUnzip::ExtractExpand(LF *pFileHeader)             // local file pointer
{
    int     i = 0, j = 0, k = 0,                            // loop variables
            c = 0,                                  // current character
            reduceCharacter = 0,                    // reduction character
            reduceMask = 0,                           // mask
            reduceShift = 0,                           // shift value
            expandLength = 0;                        // expand length
    uint8    lastCharacter = 0,                         // last character
            state = 0,                          // state machine indicator
		    *p = NULL;                                 // work pointer
    FS      *fsp = NULL,                               // follower sets pointer
            *fse = NULL;                               // ..and entry
    static
    uint8    reduce[4][2] =                      // reduction mask and lengths
                {
                { 0x7f, 7 },
                { 0x3f, 6 },
                { 0x1f, 5 },
                { 0x0f, 4 }
                },
            len_codes[33] =                     // bit lengths for numbers
                { 1, 1, 1, 2, 2, 3, 3, 3, 3, 4, // this table maps the minimum
                4, 4, 4, 4, 4, 4, 4, 5, 5, 5, // ..number of bits to represent
                5, 5, 5, 5, 5, 5, 5, 5, 5, 5, // ..a value
                5, 5, 5
                };

    fsp = static_cast<FS *>(malloc_chk(256 * sizeof(FS)));  // allocate memory for sets

    i = pFileHeader->lfCompressionMethod - LF_CM_REDUCED1;            // get index into array
    reduceMask = reduce[i][0];                    // ..copy over mask
    reduceShift = reduce[i][1];                    // ..and shift amount

    for (i = 256, fse = &fsp[255]; i--; fse--)  // build follower sets
    {
        if ((j = GetCode(6)) == 0xffff)        // q. get a length code ok?
        {
            break;                              // a. no .. exit loop
        }

        fse->setLength = j;                       // save length of set data

        for (p = fse->set; j--; p++)            // set up length in set
        {
            if ((k = GetCode(8)) == 0xffff)    // q. get a data code ok?
            {
                break;                          // a. no .. exit loop
            }

            *p = (char) k;                      // save set data
        }
    }

    for (;;)                                    // loop till file processed
    {
        fse = &fsp[lastCharacter];                     // current follower set

        if (!(fse->setLength))                   // q. empty set?
        {										// a. yes .. get more input
            if ((c = GetCode(8)) == 0xffff)    // q. get a code ok?
            {
                break;                          // a. no .. exit loop
            }
        }
        else
        {
            if ((c = GetCode(1)) == 0xffff)    // q. get a code ok?
            {
                break;                          // a. no .. exit loop
            }

            if (c)                              // q. need to get another byte?
            {									// a. yes .. get another
                if ((c = GetCode(8)) == 0xffff)    // q. get a code ok?
                {
                    break;                          // a. no .. exit loop
                }
            }
            else
            {
                i = len_codes[fse->setLength];    // get next read bit length

                if ((c = GetCode(i)) == 0xffff)// q. get next code ok?
                {
					break;                      // a. no .. exit loop
                }

                c = fse->set[c];                // get encoded character
            }
        }

        lastCharacter = c;                             // set up new last character

        switch (state)                          // based on current state
        {
            case 0:                             // 0: output character
                if (c == EXPLODE_DLE)           // q. DLE character?
                {
                    state = 1;                  // a. yes .. change states
                }
                else
                {
                    StoreChar(c);              // else .. output character
                }
                break;                          // ..then process next character


            case 1:                             // 1: store length
                if (!c)                      // q. null character?
                {
                    StoreChar(EXPLODE_DLE);    // a. yes .. output a DLE char
                    state = 0;                  // ..and change states
                }
                else
                {
                    reduceCharacter = c;            // save character being reduced
                    c &= reduceMask;              // clear unused bits
                    expandLength = c;                // save length to expand
                    state = (c == reduceMask)     // select next state
                            ? 2 : 3;            // ..if char is special flag
                }
                break;                          // ..then process next character

            case 2:                             // 2: store length
                expandLength += c;                   // save length to expand
                state = 3;                      // select next state
                break;                          // ..then get next character

            case 3:                             // 3: expand string
                c = ((reduceCharacter >> reduceShift)  // compute offset backwards
                        << 8) + c + 1;
                expandLength += 3;              // set up expansion length

                if ((m_sBufferPtr - m_sBuffer) >= c)  // q. backward wrap?
                {
                    p = m_sBufferPtr - c;         // a. no .. just back up a bit
                }
                else
                {
                    p = m_sBufferSize - c + m_sBufferPtr;      // else .. find at end of buffer
                }

                while (expandLength--)           // copy previously outputed
                    {                            // ..strings from sliding buffer
                    StoreChar(*p);               // put out each character

                    if (++p >= m_sBufferEnd)     // q. hit the end of the buffer?
                    {
                        p = m_sBuffer;           // a. yes .. back to beginning
                    }
                }

                state = 0;                       // change state back
                break;                           // ..and process next character
            default:
                break;
        }
    }

    nsl_free(fsp);                              // free follower sets
}


/*!
Extracts a LZW shrunk file
*/
void CUnzip::ExtractShrunk(void)                // local file pointer
{
    int     baseCode = 0,                           // base code
            previousCode = 0,                       // previous code
            savedCode = 0,                          // saved code
            currentCode = 0;                        // current code
    uint32  cnt = 0;                                // decode stack counter
    char    codeSize = 9;                       // code size
    SD      *dict = NULL,                              // main dictionary
            *d = NULL,                                 // dictionary entry pointer
            *fd = NULL,                                // next free pointer
            *ld = NULL;                                // last entry
    char    *decode_stack = NULL;                      // decode stack


    dict = static_cast<SD *>(malloc_chk(             // allocate dictionary trees
            sizeof(SD) * TABLE_SIZE));
    decode_stack = static_cast<char *> (malloc_chk(   // ..and allocate decode stack
            TABLE_SIZE));

    ld = &dict[TABLE_SIZE];                     // get address of last entry

    for (fd = d = &dict[257]; d < ld; d++)      // loop thru dictionary
    {
        d->parentCode = ZFREE;                 // ..and make each one free
    }

    StoreChar(previousCode = baseCode = GetCode(codeSize));       // get and store a code

    for (;;)                                    // inner loop
    {
        cnt = 0;                                // reset decode stack

        if ((savedCode = currentCode = GetCode(codeSize)) == -1)     // q. end of data?
        {
            break;                              // a. yes .. exit loop
        }

        if (currentCode == 256)                           // q. special code?
        {                                   // a. yes .. get next code
            if ((currentCode = GetCode(codeSize)) == -1)       // q. get next code ok?
            {
                nsl_free(decode_stack);                      // free decode stack
                nsl_free(dict);                              // ..and dictionary
                return;
            }

            if (currentCode == 1)                // q. 256,1 sequence?
            {
                codeSize++;                           // a. yes .. increase code size
                continue;                       // ..and get next character
            }
            else if (currentCode == 2)                   // q. clear tree (256,2)?
                {                               // a. yes .. partially clear nodes
                    for (d = &dict[257];            // loop thru dictionary..
                        d < fd; d++)        // ..starting past literals
                    {
                        d->parentCode |= 0x8000;      // ..and mark as unused
                    }

                    for (d = &dict[257];            // loop again thru dictionary..
                        d < fd; d++)        // ..checking each used node
                    {
                        currentCode = d->parentCode & 0x7fff;       // get node's next pointer

                        if (currentCode >= 257)                   // q. uses another node?
                        {
                            dict[currentCode].parentCode &= 0x7fff; // a. yes .. clear target
                        }
                    }

                    for (d = &dict[257];            // loop once more, this time
                        d < fd; d++)        // ..release unneeded entries
                    {
                        if (d->parentCode & 0x8000)   // q. need to be cleared?
                        {
                            d->parentCode = ZFREE;     // a. yes .. set it to free
                        }
                    }

                    for (d = &dict[257];            // loop thru dictionary to..
                        d < ld; d++)        // ..find the first free node
                    {
                        if (d->parentCode == ZFREE)    // q. find a free entry?
                        {
                            break;                  // a. yes .. exit loop
                        }
                    }

                    fd = d;                         // save next free dict node
                    continue;                       // ..continue with inner loop
                }
            }

        if (currentCode < 256)                            // q. literal code?
        {
            StoreChar(baseCode = currentCode);                // a. yes .. put out literal
        }

        else                                   // else .. must be .gt. 256
        {
            if(dict[currentCode].parentCode == ZFREE)        // q. using new code?
            {
                decode_stack[cnt++] = baseCode;      // a. yes .. store old character
                currentCode = previousCode;                        // set up search criteria
            }

            while (currentCode > 255)                     // loop finding entries to use
            {
                d = &dict[currentCode];                   // point to current entry
                decode_stack[cnt++] = d->c;     // put character into stack
                currentCode = d->parentCode;                // get parent's code
            }

            StoreChar(baseCode = currentCode);                // put out first character

            while (cnt)                         // loop outputing from ..
            {
                StoreChar(decode_stack[--cnt]);// ..decode stack
            }
        }

        fd->parentCode = previousCode;          // store parent's code
        fd->c = baseCode;                       // ..and its character
        previousCode = savedCode;                              // set up new parent code

        while (++fd < ld)                       // loop thru dictionary
        {
            if (fd->parentCode == ZFREE)           // q. entry free?
            {
                break;                          // a. yes .. done looping
            }
        }
    }

    nsl_free(decode_stack);                      // free decode stack
    nsl_free(dict);                              // ..and dictionary
}


/*!
Extracts a stored file
*/
void CUnzip::ExtractStored(void)                // local file pointer
{
    ExtractCopy(m_rSize);                        // copy stored data to file
}


/*!
Extracts files from a ZIP file

@param fnc = 0 - initialization call
           = 1 - extracting file
           = 2 - completion call
*/
void CUnzip::ExtractZip(int fnc,                // function
                    LF *pFileHeader)                    // local file header pointer
{

switch (fnc)                                    // based on function number
    {
    case 0:                                     // initialization call
        m_sBufferSize = static_cast<uint32>(DICO_BUFFER_SIZE);  // set up the dictionary size
		if (NULL == m_sBuffer) m_sBuffer = static_cast<uint8 *>(malloc_chk(m_sBufferSize)); // get memory for output buffer
        m_sBufferEnd = &m_sBuffer[m_sBufferSize]; // set up end of buffer address
        break;                                   // ..and return to caller

    case 1:
        if (pFileHeader->lfFlag & LH_FLAG_ENCRYPT)
        {
			//Error(" No se admiten ficheros encriptados.");
			//ASSERT( FALSE );
            break;
        }

        if ( ExtractOpen(pFileHeader) )
        {
            break;
        }

        m_rSize = pFileHeader->lfCompressedSize;      // remaining filesize to process
        m_bSize = 0;                          // ..bits in byte to process
        m_endCount = 2;                        // end of data counter/flag
        m_crc = -1;                           // clear current crc
        m_sBufferPtr = m_sBuffer;             // reset next output pointer
        m_lastKpal = 0;                      // clear keep alive timer
        m_spin = 0;                           // ..next kpal character index
        nsl_memset(m_sBuffer, 0, m_sBufferSize);  // ..and dictionary/output buffer

        switch (pFileHeader->lfCompressionMethod)                 // based on compression method
		{
            case LF_CM_STORED:              // stored file
                ExtractStored();           // process stored file
                break;                      // ..then do next file

            case LF_CM_SHRUNK:              // LZW shrunk file
                ExtractShrunk();           // process shrunk file
                break;                      // ..then do next file

            case LF_CM_REDUCED1:            // reduced file #1
            case LF_CM_REDUCED2:            // reduced file #2
            case LF_CM_REDUCED3:            // reduced file #3
            case LF_CM_REDUCED4:            // reduced file #4
                ExtractExpand(pFileHeader);        // process reduced file
                break;                      // ..then do next file

            case LF_CM_IMPLODED:            // imploded file
                ExtractExplode(pFileHeader);       // process imploded file
                break;                      // ..then do next file

            case LF_CM_DEFLATED:            // deflated file
                ExtractInflate(pFileHeader);       // process deflated file
                break;                      // ..then do next file

            case LF_CM_TOKENIZED:           // tokenized file
            default:
                return;                     // ..and return to caller
        }

        ExtractFlush();                    // else .. flush our output buffer

        m_crc ^= 0xffffffffL;                 // finalize crc value

        break;                              // ..and return to caller

    case 2:                                 // completion call
        if(m_sBuffer != NULL)
        {
            nsl_free(m_sBuffer);
            m_sBuffer = NULL;
        }
        break;
                            // then return to caller
    default:
        break;
    }
}


/*!
Releases allocated memory
*/
void CUnzip::ReleaseBuffer()
{
    if(pZipBuffer != NULL)
    {
        nsl_free(pZipBuffer);
        pZipBuffer = pZipOut = NULL;
    }
}

#ifdef WIN32
	#pragma warning( default : 4005 )
	#pragma warning( default : 4018 )
	#pragma warning( default : 4305 )
	#pragma warning( default : 4309 )
#endif

/*! @} */
