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

    @file nbgmnbmchunk.h
*/
/*
    (C) Copyright 2010 by Networks In Motion, Inc.

    The information contained herein is confidential, proprietary
    to Networks In Motion, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
#ifndef _NBGM_NBM_CHUNK_H_
#define _NBGM_NBM_CHUNK_H_
#include "nbretypes.h"
#include "nbrecommon.h"

class NBGM_NBMArchive;
struct NBGM_NBMField;

/*! \addtogroup NBRE_Mapdata
*  @{
*/
/*! \addtogroup NBGM_NBMFile
*  @{
*/

#define MAKE_NBM_CHUNK_ID(A, B, C, D) ( ((D)<<24) + ((C)<<16) + ((B)<<8) + (A) )

#define NBM_ID_NBMF MAKE_NBM_CHUNK_ID ('N', 'B', 'M', 'F')
#define NBM_ID_LAYS MAKE_NBM_CHUNK_ID ('L', 'A', 'Y', 'S')
#define NBM_ID_SEEN MAKE_NBM_CHUNK_ID ('S', 'E', 'E', 'N')
#define NBM_ID_OBJS MAKE_NBM_CHUNK_ID ('O', 'B', 'J', 'S')
#define NBM_ID_OBJX MAKE_NBM_CHUNK_ID ('O', 'B', 'J', 'X')
#define NBM_ID_MESH MAKE_NBM_CHUNK_ID ('M', 'E', 'S', 'H')
#define NBM_ID_GEOM MAKE_NBM_CHUNK_ID ('G', 'E', 'O', 'M')
#define NBM_ID_MTRL MAKE_NBM_CHUNK_ID ('M', 'T', 'R', 'L')
#define NBM_ID_BMAP MAKE_NBM_CHUNK_ID ('B', 'M', 'A', 'P')
#define NBM_ID_IMGS MAKE_NBM_CHUNK_ID ('I', 'M', 'G', 'S')
#define NBM_ID_AREA MAKE_NBM_CHUNK_ID ('A', 'R', 'E', 'A')
#define NBM_ID_PNTS MAKE_NBM_CHUNK_ID ('P', 'N', 'T', 'S')
#define NBM_ID_TEXT MAKE_NBM_CHUNK_ID ('T', 'E', 'X', 'T')
#define NBM_ID_PLIN MAKE_NBM_CHUNK_ID ('P', 'L', 'I', 'N')
#define NBM_ID_SPLN MAKE_NBM_CHUNK_ID ('S', 'P', 'L', 'N')
#define NBM_ID_TPSH MAKE_NBM_CHUNK_ID ('T', 'P', 'S', 'H')
#define NBM_ID_TPAR MAKE_NBM_CHUNK_ID ('T', 'P', 'A', 'R')
#define NBM_ID_TPTH MAKE_NBM_CHUNK_ID ('T', 'P', 'T', 'H')
#define NBM_ID_LPTH MAKE_NBM_CHUNK_ID ('L', 'P', 'T', 'H')

#define NBM_NAME_NBMF "NBMF"
#define NBM_NAME_LAYS "LAYS"
#define NBM_NAME_SEEN "SEEN"
#define NBM_NAME_OBJS "OBJS"
#define NBM_NAME_OBJX "OBJX"
#define NBM_NAME_MESH "MESH"
#define NBM_NAME_GEOM "GEOM"
#define NBM_NAME_MTRL "MTRL"
#define NBM_NAME_BMAP "BMAP"
#define NBM_NAME_IMGS "IMGS"
#define NBM_NAME_AREA "AREA"
#define NBM_NAME_PNTS "PNTS"
#define NBM_NAME_TEXT "TEXT"
#define NBM_NAME_PLIN "PLIN"
#define NBM_NAME_SPLN "SPLN"
#define NBM_NAME_TPSH "TPSH"
#define NBM_NAME_TPAR "TPAR"
#define NBM_NAME_TPTH "TPTH"
#define NBM_NAME_LPTH "LPTH"

struct NBGM_NBMChunkInfo
{
    uint32      mIdentifier;
    NBRE_String mName;
    uint32      mSize;
    uint16      mFlag;
    uint32      mOffset;

    void Serialize(NBGM_NBMArchive& ar);
};

class NBGM_NBMChunk
{
    friend class NBGM_NBMObject;
    friend class NBGM_NBMChunkFactory;
public:
    virtual ~NBGM_NBMChunk();

public:
    const NBGM_NBMChunkInfo& ChunkInfo() const;

protected:
    explicit NBGM_NBMChunk(const NBGM_NBMChunkInfo& chunkInfo);

private:
    void Serialize(NBGM_NBMArchive& ar);

private:
    virtual void ReadImpl(NBGM_NBMField& field) = 0;
    virtual void WriteImpl(NBGM_NBMField& field) const = 0;

protected:
    NBGM_NBMChunkInfo  mChunkInfo;
};

/** @} */
/** @} */
#endif
