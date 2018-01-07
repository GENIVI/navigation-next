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

(C) Copyright 2010 by Networks In Motion, Inc.

The information contained herein is confidential, proprietary
to Networks In Motion, Inc., and considered a trade secret as
defined in section 499C of the penal code of the State of
California. Use of this information by anyone other than
authorized employees of Networks In Motion is granted only
under a written non-disclosure agreement, expressly
prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
#include "nbgmdom.h"
#include "nbrelog.h"
#include "palerror.h"
#include "nbgmcommon.h"

typedef enum DOM_ChunkIndex
{
    NBMF_CHUNK_INDEX = 0,
    LAYS_CHUNK_INDEX,
    SEEN_CHUNK_INDEX,
    OBJS_CHUNK_INDEX,
    OBJX_CHUNK_INDEX,
    MESH_CHUNK_INDEX,
    GEOM_CHUNK_INDEX,
    MTRL_CHUNK_INDEX,
    BMAP_CHUNK_INDEX,
    IMGS_CHUNK_INDEX,
    AREA_CHUNK_INDEX,
    PNTS_CHUNK_INDEX,
    TEXT_CHUNK_INDEX,
    PLIN_CHUNK_INDEX,
    SPLN_CHUNK_INDEX,
    TPSH_CHUNK_INDEX,
    THAR_CHUNK_INDEX,
    TPTH_CHUNK_INDEX,
    LTPH_CHUNK_INDEX,
    POIS_CHUNK_INDEX,
    GPIN_CHUNK_INDEX,
    EIDS_CHUNK_INDEX,
    DROD_CHUNK_INDEX,
    LPDR_CHUNK_INDEX,
    UNSUPPORTED_CHUNK_INDEX
}DOM_ChunkIndex;

typedef enum DOM_FloatType
{
    FLOAT_TYPE1 = 1,
    FLOAT_TYPE2 = 2,
    FLOAT_TYPE3 = 3
}DOM_FloatType;

struct NBRE_DOM
{
    NBRE_Array*     arrayNBMChunk;
    char*           domName;
};

typedef struct DOM_Chunk
{
    NBRE_DOM_ChunkInfo          chunkInfo;
    uint16                      flag;
    uint16                      count;
    void*                       chunkDetail;
}DOM_Chunk;

typedef struct DomReadStatus
{
    NBRE_DOM*               dom;
    NBRE_IOStream*          stream;
    nb_boolean              isFileEnd;
    nb_boolean              isNBMFile;
    uint32                  absNBMFileOffset;
}DomReadStatus;

typedef PAL_Error (*ChunkReadFunc)(DomReadStatus* readStatus, NBRE_DOM_ChunkInfo* chunkInfo, void** chunk);

static float ConvertFloat16ToFloat32(uint16 src);

static PAL_Error ReadUint32(NBRE_IOStream* stream, uint32* buffer, uint32 count);
static PAL_Error ReadUint24(NBRE_IOStream* stream, uint32* buffer, uint32 count);
static PAL_Error ReadUint16(NBRE_IOStream* stream, uint16* buffer, uint32 count);
static PAL_Error ReadUint8(NBRE_IOStream* stream, uint8* buffer, uint32 count);
static PAL_Error ReadInt8(NBRE_IOStream* stream, int8* buffer, uint32 count);
static PAL_Error ReadChar(NBRE_IOStream* stream, char* buffer, uint32 count);
static PAL_Error ReadFloat(NBRE_IOStream* stream, float* buffer, uint32 count);
static PAL_Error ReadDouble(NBRE_IOStream* stream, double* buffer, uint32 count);
static PAL_Error ReadFloat16(NBRE_IOStream* stream, float* buffer, uint32 count);
static PAL_Error ReadFloat16v2(NBRE_IOStream* stream, double* buffer, uint32 count, double tileSizeX, double tileSizeY);
static PAL_Error ReadFloat16v2(NBRE_IOStream* stream, float* buffer, uint32 count, double tileSizeX, double tileSizeY);
static PAL_Error ReadFloat16v2(NBRE_IOStream* stream, float* buffer, uint32 count, float absRange);

// Range values defined in NBM SDS "Data Format" section
static const float HALF2_Z_RANGE = 1000.0f;
static const float HALF2_NORMAL_RANGE = 1.0f;
static const float HALF2_TEXCOORD_RANGE = 1.0f;

static PAL_Error ReadFileHeader(DomReadStatus* readStruct, NBRE_DOM_ChunkInfo* info, void** chunk);
static PAL_Error ReadImages(DomReadStatus* readStruct, NBRE_DOM_ChunkInfo* info, void** chunk);
static PAL_Error ReadMaterials(DomReadStatus* readStruct, NBRE_DOM_ChunkInfo* info, void** chunk);
static PAL_Error ReadMeshes(DomReadStatus* readStruct, NBRE_DOM_ChunkInfo* info, void** chunk);
static PAL_Error ReadTracks(DomReadStatus* readStruct, NBRE_DOM_ChunkInfo* info, void** chunk);
static PAL_Error ReadTexts(DomReadStatus* readStruct, NBRE_DOM_ChunkInfo* info, void** chunk);
static PAL_Error ReadExternalObjects(DomReadStatus* readStruct, NBRE_DOM_ChunkInfo* info, void** chunk);
static PAL_Error ReadScenes(DomReadStatus* readStruct, NBRE_DOM_ChunkInfo* info, void** chunk);
static PAL_Error ReadLayers(DomReadStatus* readStruct, NBRE_DOM_ChunkInfo* info, void** chunk);
static PAL_Error ReadGeometries(DomReadStatus* readStruct, NBRE_DOM_ChunkInfo* info, void** chunk);
static PAL_Error ReadBitmaps(DomReadStatus* readStruct, NBRE_DOM_ChunkInfo* info, void** chunk);
static PAL_Error ReadPathShields(DomReadStatus* readStruct, NBRE_DOM_ChunkInfo* info, void** chunk);
static PAL_Error ReadRoads(DomReadStatus* readStruct, NBRE_DOM_ChunkInfo* info, void** chunk);
static PAL_Error ReadPolylines(DomReadStatus* readStruct, NBRE_DOM_ChunkInfo* info, void** chunk);
static PAL_Error ReadAreas(DomReadStatus* readStruct, NBRE_DOM_ChunkInfo* info, void** chunk);
static PAL_Error ReadPoints(DomReadStatus* readStruct, NBRE_DOM_ChunkInfo* info, void** chunk);
static PAL_Error ReadObjects(DomReadStatus* readStruct, NBRE_DOM_ChunkInfo* info, void** chunk);
static PAL_Error ReadSplines(DomReadStatus* readStruct, NBRE_DOM_ChunkInfo* info, void** chunk);
static PAL_Error ReadPathArrows(DomReadStatus* readStruct, NBRE_DOM_ChunkInfo* info, void** chunk);
static PAL_Error ReadPois(DomReadStatus* readStruct, NBRE_DOM_ChunkInfo* info, void** chunk);
static PAL_Error ReadGPins(DomReadStatus* readStruct, NBRE_DOM_ChunkInfo* info, void** chunk);
static PAL_Error ReadEids(DomReadStatus* readStruct, NBRE_DOM_ChunkInfo* info, void** chunk);
static PAL_Error ReadDrod(DomReadStatus* readStruct, NBRE_DOM_ChunkInfo* info, void** chunk);
static PAL_Error ReadRoadWithDrawOrder(DomReadStatus* readStruct, NBRE_DOM_ChunkInfo* info, void** chunk);
static PAL_Error DefaultChunkReadFunc(DomReadStatus* readStruct, NBRE_DOM_ChunkInfo* info, void** chunk);

static void NBMFDestroy(void* chunk);
static void IMGSDestroy(void* chunk);
static void MTRLDestroy(void* chunk);
static void MESHDestroy(void* chunk);
static void TPTHDestroy(void* chunk);
static void TEXTDestroy(void* chunk);
static void OBJXDestroy(void* chunk);
static void SEENDestroy(void* chunk);
static void LAYSDestroy(void* chunk);
static void GEOMDestroy(void* chunk);
static void BMPADestroy(void* chunk);
static void LPTHDestroy(void* chunk);
static void PLINDestroy(void* chunk);
static void AREADestroy(void* chunk);
static void PNTSDestroy(void* chunk);
static void OBJSDestroy(void* chunk);
static void SPLNDestroy(void* chunk);
static void TPSHDestroy(void* chunk);
static void TPARDestroy(void* chunk);
static void POISDestroy(void* chunk);
static void GPINDestroy(void* chunk);
static void EIDSDestroy(void* chunk);
static void DRODDestroy(void* chunk);
static void LPDRDestroy(void* chunk);
static void DefaultDestroy(void* chunk);

static PAL_Error
ReadFlagAndCount(NBRE_IOStream* io, uint16* flag, uint16* count)
{
    PAL_Error err = ReadUint16(io, flag, 1);
    if (err != PAL_Ok)
    {
        return err;
    }

    err = ReadUint16(io, count, 1);
    if (err != PAL_Ok)
    {
        return err;
    }

    if(count == 0)
    {
        err = PAL_ErrNoData;
    }

    return err;
}

static void
GetChunkConfigFromChunkID(uint32 ID, uint32* chunkIndex, ChunkReadFunc* readFunc, NBRE_DOM_ChunkDestroyFunc* destroyFunc)
{
    switch(ID)
    {
    case DOM_ID_NBMF:
        *chunkIndex = NBMF_CHUNK_INDEX;
        *readFunc = ReadFileHeader;
        *destroyFunc = NBMFDestroy;
        break;

    case DOM_ID_LAYS:
        *chunkIndex = LAYS_CHUNK_INDEX;
        *readFunc = ReadLayers;
        *destroyFunc = LAYSDestroy;
        break;

    case DOM_ID_SEEN:
        *chunkIndex = SEEN_CHUNK_INDEX;
        *readFunc = ReadScenes;
        *destroyFunc = SEENDestroy;
        break;

    case DOM_ID_OBJS:
        *chunkIndex = OBJS_CHUNK_INDEX;
        *readFunc = ReadObjects;
        *destroyFunc = OBJSDestroy;
        break;

    case DOM_ID_OBJX:
        *chunkIndex = OBJX_CHUNK_INDEX;
        *readFunc = ReadExternalObjects;
        *destroyFunc = OBJXDestroy;
        break;

    case DOM_ID_MESH:
        *chunkIndex = MESH_CHUNK_INDEX;
        *readFunc = ReadMeshes;
        *destroyFunc = MESHDestroy;
        break;

    case DOM_ID_GEOM:
        *chunkIndex = GEOM_CHUNK_INDEX;
        *readFunc = ReadGeometries;
        *destroyFunc = GEOMDestroy;
        break;

    case DOM_ID_MTRL:
        *chunkIndex = MTRL_CHUNK_INDEX;
        *readFunc = ReadMaterials;
        *destroyFunc = MTRLDestroy;
        break;

    case DOM_ID_BMAP:
        *chunkIndex = BMAP_CHUNK_INDEX;
        *readFunc = ReadBitmaps;
        *destroyFunc = BMPADestroy;
        break;

    case DOM_ID_IMGS:
        *chunkIndex = IMGS_CHUNK_INDEX;
        *readFunc = ReadImages;
        *destroyFunc = IMGSDestroy;
        break;

    case DOM_ID_AREA:
        *chunkIndex = AREA_CHUNK_INDEX;
        *readFunc = ReadAreas;
        *destroyFunc = AREADestroy;
        break;

    case DOM_ID_PNTS:
        *chunkIndex = PNTS_CHUNK_INDEX;
        *readFunc = ReadPoints;
        *destroyFunc = PNTSDestroy;
        break;

    case DOM_ID_TEXT:
        *chunkIndex = TEXT_CHUNK_INDEX;
        *readFunc = ReadTexts;
        *destroyFunc = TEXTDestroy;
        break;

    case DOM_ID_PLIN:
        *chunkIndex = PLIN_CHUNK_INDEX;
        *readFunc = ReadPolylines;
        *destroyFunc = PLINDestroy;
        break;

    case DOM_ID_SPLN:
        *chunkIndex = SPLN_CHUNK_INDEX;
        *readFunc = ReadSplines;
        *destroyFunc = SPLNDestroy;
        break;

    case DOM_ID_TPSH:
        *chunkIndex = TPSH_CHUNK_INDEX;
        *readFunc = ReadPathShields;
        *destroyFunc = TPSHDestroy;
        break;

    case DOM_ID_TPAR:
        *chunkIndex = THAR_CHUNK_INDEX;
        *readFunc = ReadPathArrows;
        *destroyFunc = TPARDestroy;
        break;

    case DOM_ID_TPTH:
        *chunkIndex = TPTH_CHUNK_INDEX;
        *readFunc = ReadTracks;
        *destroyFunc = TPTHDestroy;
        break;

    case DOM_ID_LPTH:
        *chunkIndex = LTPH_CHUNK_INDEX;
        *readFunc = ReadRoads;
        *destroyFunc = LPTHDestroy;
        break;

    case DOM_ID_POIS:
        *chunkIndex = POIS_CHUNK_INDEX;
        *readFunc = ReadPois;
        *destroyFunc = POISDestroy;
        break;

    case DOM_ID_GPIN:
        *chunkIndex = GPIN_CHUNK_INDEX;
        *readFunc = ReadGPins;
        *destroyFunc = GPINDestroy;
        break;

    case DOM_ID_EIDS:
        *chunkIndex = EIDS_CHUNK_INDEX;
        *readFunc = ReadEids;
        *destroyFunc = EIDSDestroy;
        break;

    case DOM_ID_DROD:
        *chunkIndex = DROD_CHUNK_INDEX;
        *readFunc = ReadDrod;
        *destroyFunc = DRODDestroy;
        break;

    case DOM_ID_LPDR:
        *chunkIndex = LPDR_CHUNK_INDEX;
        *readFunc = ReadRoadWithDrawOrder;
        *destroyFunc = LPDRDestroy;
        break;

    default:
        NBRE_DebugLog(PAL_LogSeverityMajor, "GetChunkConfigFromChunkID, unknown chunk id: %u", ID);
        *chunkIndex = UNSUPPORTED_CHUNK_INDEX;
        *readFunc = DefaultChunkReadFunc;
        *destroyFunc = DefaultDestroy;
        break;
    }
}

//get chunk offset, ID, size
static PAL_Error
ReadChunkInfo(DomReadStatus* domReadStatus, NBRE_DOM_ChunkInfo* chunkInfo)
{
    uint32 errLine = INVALID_INDEX;
    uint32 chunkID = 0;
    uint32 chunkSize = 0;
    uint32 baseSize = 0;

    domReadStatus->isFileEnd = domReadStatus->stream->Eof();
    if(domReadStatus->isFileEnd)
    {
        return PAL_Ok;
    }

    uint32 position = domReadStatus->stream->GetPos();

    PAL_Error err = ReadUint32(domReadStatus->stream, &chunkID, 1);
    if (err != PAL_Ok)
    {
        errLine = __LINE__;
        goto HandleError;
    }
    baseSize += sizeof(chunkID);

    if (!domReadStatus->isNBMFile)
    {
        if (nsl_strncmp((char*)&chunkID, "NBMF", 4) == 0)
        {
            domReadStatus->isNBMFile = TRUE;
        }
        else
        {
            errLine = __LINE__;
            err = PAL_ErrWrongFormat;
            goto HandleError;
        }
    }

    err = ReadUint32(domReadStatus->stream, &chunkSize, 1);
    if (err != PAL_Ok)
    {
        errLine = __LINE__;
        goto HandleError;
    }
    baseSize += sizeof(chunkSize);

    chunkInfo->fileOffset = position - domReadStatus->absNBMFileOffset;
    chunkInfo->ID = chunkID;
    chunkInfo->chunkSize = chunkSize;
    chunkInfo->baseSize = baseSize;

    return PAL_Ok;

HandleError:
    NBRE_DebugLog(PAL_LogSeverityMajor, "ReadChunkInfo, err = 0x%08x, errLine = %u", err, errLine);
    return err;
}

static PAL_Error
DOMCreate(const NBRE_String& domName, NBRE_DOM** dom)
{
    uint16 i = 0;
    NBRE_DOM* result = (NBRE_DOM*)nsl_malloc(sizeof(NBRE_DOM));

    if (result == NULL)
    {
        return PAL_ErrNoMem;
    }
    nsl_memset(result, 0, sizeof(NBRE_DOM));

    result->domName = nsl_strdup(domName.c_str());
    result->arrayNBMChunk = NBRE_ArrayCreate(UNSUPPORTED_CHUNK_INDEX);
    if (result->arrayNBMChunk == NULL)
    {
        goto HandleError;
    }

    for(i=0; i<UNSUPPORTED_CHUNK_INDEX; i++)
    {
        NBRE_Array* array = NBRE_ArrayCreate(1);
        if (array == NULL)
        {
            goto HandleError;
        }
        NBRE_ArrayAppend(result->arrayNBMChunk, array);
    }

    *dom = result;

    return PAL_Ok;

HandleError:
    NBRE_DebugLog(PAL_LogSeverityMajor, "NBRE_DOMCreate Error!");
    NBRE_DOMDestroy(result);
    *dom = NULL;
    return PAL_ErrNoMem;
}

void
NBRE_DOMDestroy(NBRE_DOM* dom)
{
    if(dom != NULL)
    {
        uint32 i = 0;
        for(i=0; i<UNSUPPORTED_CHUNK_INDEX; i++)
        {
            NBRE_Array* chunkArray = (NBRE_Array*)NBRE_ArrayGetAt(dom->arrayNBMChunk, i);
            uint32 count = NBRE_ArrayGetSize(chunkArray);
            uint32 j = 0;

            for(j=0; j<count; j++)
            {
                DOM_Chunk* chunk = (DOM_Chunk*)NBRE_ArrayGetAt(chunkArray, j);
                if (chunk)
                {
                    chunk->chunkInfo.destroyFunc(chunk);
                }
            }
            NBRE_ArrayDestroy(chunkArray);
        }

        NBRE_ArrayDestroy(dom->arrayNBMChunk);

        if(dom->domName != NULL)
        {
            nsl_free(dom->domName);
        }
        nsl_free(dom);
    }
}

PAL_Error
NBRE_DOMGetChunkArray(NBRE_DOM* dom, uint32 chunkID, NBRE_Array** chunkArray)
{
    uint32 index = 0;
    NBRE_DOM_ChunkDestroyFunc destroyFunc;
    ChunkReadFunc  readFunc;

    if (dom == NULL)
    {
        return PAL_ErrBadParam;
    }

    GetChunkConfigFromChunkID(chunkID, &index, &readFunc, &destroyFunc);
    if(index == UNSUPPORTED_CHUNK_INDEX)
    {
        return PAL_ErrNotFound;
    }

    *chunkArray = (NBRE_Array*)NBRE_ArrayGetAt(dom->arrayNBMChunk, index);

    return PAL_Ok;
}

PAL_Error
NBRE_DOMGetChunkById(NBRE_DOM* dom, uint32 chunkID, NBRE_DOM_Chunk** chunk)
{
    NBRE_Array* array = NULL;
    NBRE_DOMGetChunkArray(dom, chunkID, &array);
    if(array != NULL)
    {
        if(NBRE_ArrayGetSize(array) == 1)
        {
            *chunk = (NBRE_DOM_Chunk *)NBRE_ArrayGetAt(array, 0);
            return PAL_Ok;
        }
        else if(NBRE_ArrayGetSize(array) > 1)
        {
            *chunk = (NBRE_DOM_Chunk *)NBRE_ArrayGetAt(array, 0);
            NBRE_DebugLog(PAL_LogSeverityMajor, "NBRE_DOMGetChunkById, static chunk is not singleton, chunkID=%x", chunkID);
            return PAL_Ok;
        }
    }
    NBRE_DebugLog(PAL_LogSeverityMajor, "NBRE_DOMGetChunkById, err=0x%08x, chunkID=%x", PAL_ErrNotFound, chunkID);
    return PAL_ErrNotFound;
}

PAL_Error
NBRE_DOMGetChunk(NBRE_DOM* dom, uint32 chunkOffset, NBRE_DOM_Chunk** chunk)
{
    if(chunk == NULL)
    {
        return PAL_ErrBadParam;
    }

    for(uint32 i=0; i<UNSUPPORTED_CHUNK_INDEX; i++)
    {
        NBRE_Array* chunkArray = (NBRE_Array *)NBRE_ArrayGetAt(dom->arrayNBMChunk, i);
        uint32 count = NBRE_ArrayGetSize(chunkArray);

        for(uint32 j=0; j<count; j++)
        {
            DOM_Chunk* domChunk = (DOM_Chunk*)NBRE_ArrayGetAt(chunkArray, j);
            if (domChunk && domChunk->chunkInfo.fileOffset == chunkOffset)
            {
                *chunk = (NBRE_DOM_Chunk*)domChunk;
                return PAL_Ok;
            }
        }
    }

    NBRE_DebugLog(PAL_LogSeverityMajor, "NBRE_DOMGetChunk, err=0x%08x, chunkOffset=%u", PAL_ErrNotFound, chunkOffset);
    *chunk = NULL;
    return PAL_ErrNotFound;
}

static PAL_Error
JumpToNextChunk(NBRE_DOM_ChunkInfo& info, DomReadStatus& status, uint32 nbmOffset)
{
    uint32 currFilePos = status.stream->GetPos();
    uint32 readChunkSize = currFilePos - (info.fileOffset + info.baseSize + nbmOffset);
//    nbre_assert(info.chunkSize >= readChunkSize);

    PAL_Error err = PAL_Ok;
    uint32 remained = currFilePos % 4;
    if (remained != 0)
    {
        err = status.stream->Seek(PFSO_Current, 4-remained);
        if (err != PAL_Ok)
        {
            NBRE_DebugLog(PAL_LogSeverityMajor, "JumpToNextChunk Error!, err = 0x%x! line = %d", err, __LINE__);
            return err;
        }
        readChunkSize += (4-remained);
    }

    if(info.chunkSize > readChunkSize)
    {
        uint8 chunkID[4] = {0};
        uint32 readBytes = 0;
        err = status.stream->Read(chunkID, sizeof(chunkID), &readBytes);
        if (err != PAL_Ok)
        {
            NBRE_DebugLog(PAL_LogSeverityMajor, "JumpToNextChunk Error!, err = 0x%x! line = %d", err, __LINE__);
            return err;
        }

        err = status.stream->Seek(PFSO_Current, -(int32)(readBytes));
        if (err != PAL_Ok)
        {
            NBRE_DebugLog(PAL_LogSeverityMajor, "JumpToNextChunk Error!, err = 0x%x! line = %d", err, __LINE__);
            return err;
        }

        uint32 isTHSH = nsl_strncmp((char*)&chunkID, "TPSH", 4);
        uint32 isTHAR = nsl_strncmp((char*)&chunkID, "TPAR", 4);

        if (isTHSH != 0 && isTHAR != 0)
        {
            remained = (info.chunkSize - readChunkSize) % 4;
            if(remained == 0)
            {
                err = status.stream->Seek(PFSO_Current, info.chunkSize - readChunkSize);
            }
            else
            {
                err = status.stream->Seek(PFSO_Current, info.chunkSize - readChunkSize + 4 - remained);
            }
        }

        if (err != PAL_Ok)
        {
            NBRE_DebugLog(PAL_LogSeverityMajor, "JumpToNextChunk Error!, remained = %d, err = 0x%x! line = %d", remained, err, __LINE__);
            return err;
        }
    }

    return err;
}

PAL_Error
NBRE_DOMCreateFromFile(NBRE_IOStream* stream, const NBRE_String& domName, uint32 nbmOffset, NBRE_DOM** dom)
{
    PAL_Error err = PAL_Ok;
    uint32 errLine = INVALID_INDEX;

    NBRE_DOM* result = NULL;
    NBRE_DOM_ChunkInfo chunkInfo = {0};
    DomReadStatus readStatus = {0};

    if(stream == NULL || dom == NULL)
    {
        err = PAL_ErrBadParam;
        errLine = __LINE__;
        goto HandleError;
    }
    *dom = NULL;

    err = stream->Seek(PFSO_Current, nbmOffset);
    if (err != PAL_Ok)
    {
        errLine = __LINE__;
        goto HandleError;
    }

    err = DOMCreate(domName, &result);
    if (err != PAL_Ok)
    {
        errLine = __LINE__;
        goto HandleError;
    }

    readStatus.dom = result;
    readStatus.isFileEnd = FALSE;
    readStatus.stream = stream;
    readStatus.absNBMFileOffset = nbmOffset;
    readStatus.isNBMFile = FALSE;

    while(err == PAL_Ok)
    {
        void* chunk = NULL;
        uint32 chunkIndex = (uint32)UNSUPPORTED_CHUNK_INDEX;
        ChunkReadFunc readFunc = NULL;

        //1. read chunk info
        err = ReadChunkInfo(&readStatus, &chunkInfo);
        if(err != PAL_Ok)
        {
            NBRE_DebugLog(PAL_LogSeverityMajor, "NBRE_DOMCreateFromFile, ReadChunkInfo Error!");
            errLine = __LINE__;
            goto HandleError;
        }
        if (readStatus.isFileEnd)
        {
            break;
        }

        //2. convert chunk info
        GetChunkConfigFromChunkID(chunkInfo.ID, &chunkIndex, &readFunc, &chunkInfo.destroyFunc);

        //3. read chunk content
        err = readFunc(&readStatus, &chunkInfo, &chunk);
        if(err != PAL_Ok)
        {
            NBRE_DebugLog(PAL_LogSeverityMajor, "NBRE_DOMCreateFromFile, readFunc Error!");
            errLine = __LINE__;
            goto HandleError;
        }

        //4. add chunk into dom chunk array
        if (chunkIndex != UNSUPPORTED_CHUNK_INDEX)
        {
            err = NBRE_ArrayAppend((NBRE_Array *)NBRE_ArrayGetAt(result->arrayNBMChunk, chunkIndex), chunk);
            if (err != PAL_Ok)
            {
                NBRE_DebugLog(PAL_LogSeverityMajor, "NBRE_DOMCreateFromFile, NBRE_ArrayAppend Error!");
                errLine = __LINE__;
                goto HandleError;
            }
        }

        // 5.move file pointer align 4, file position start with 0.
        //   If there is not sub-chunk, then jump to next chunk basing on chunk size.
        err = JumpToNextChunk(chunkInfo, readStatus, nbmOffset);
        if (err != PAL_Ok)
        {
            NBRE_DebugLog(PAL_LogSeverityMajor, "NBRE_DOMCreateFromFile, JumpToNextChunk Error!");
            errLine = __LINE__;
            goto HandleError;
        }
    }

    *dom = result;

    NBRE_DebugLog(PAL_LogSeverityInfo, "NBRE_DOMCreateFromFile, err=0");
    return err;

HandleError:
    NBRE_DOMDestroy(result);
    NBRE_DebugLog(PAL_LogSeverityMajor, "NBRE_DOMCreateFromFile, err = 0x%08x, errLine = %u", err, errLine);
    return err;
}

static PAL_Error
ReadFileHeader(DomReadStatus* readStruct, NBRE_DOM_ChunkInfo* info, void** chunk)
{
    PAL_Error err = PAL_Ok;
    uint32 errLine = 0;
    uint16 version = 0;

    NBRE_DOM_NBMF* result = (NBRE_DOM_NBMF*)nsl_malloc(sizeof(NBRE_DOM_NBMF));
    if(result == NULL)
    {
        err = PAL_ErrNoMem;
        errLine = __LINE__;
        goto HandleError;
    }
    nsl_memset(result, 0, sizeof(NBRE_DOM_NBMF));

    result->chunkInfo = *info;
    err = ReadUint16(readStruct->stream, &result->flag, 1);
    err = (err != PAL_Ok) ? err : ReadUint16(readStruct->stream, &version, 1);

    if(err != PAL_Ok)
    {
        errLine = __LINE__;
        goto HandleError;
    }

    result->majorVersion = (uint8)((version & 0xff00) >> 8);
    result->minorVersion = (uint8)(version & 0x00ff);

    if(result->majorVersion != CURRENT_SUPPORTED_NBM_REVISOION)
    {
        //err = PAL_ErrWrongFormat;
        //errLine = __LINE__;
        //goto HandleError;
    }

    if (result->majorVersion == 24)
    {
        err = (err != PAL_Ok) ? err : ReadDouble(readStruct->stream, &result->tileSizeX, 1);
        err = (err != PAL_Ok) ? err : ReadDouble(readStruct->stream, &result->tileSizeY, 1);
        err = (err != PAL_Ok) ? err : ReadDouble(readStruct->stream, &result->refCenterX, 1);
        err = (err != PAL_Ok) ? err : ReadDouble(readStruct->stream, &result->refCenterY, 1);

        if(err != PAL_Ok)
        {
            errLine = __LINE__;
            goto HandleError;
        }

        if (result->flag & 0x8000)
        {
            result->flag &= 0x00FF;
            err = (err != PAL_Ok) ? err : ReadFloat(readStruct->stream, &result->aabbLeft, 1);
            err = (err != PAL_Ok) ? err : ReadFloat(readStruct->stream, &result->aabbBottom, 1);
            err = (err != PAL_Ok) ? err : ReadFloat(readStruct->stream, &result->aabbRight, 1);
            err = (err != PAL_Ok) ? err : ReadFloat(readStruct->stream, &result->aabbTop, 1);

            if(err != PAL_Ok)
            {
                errLine = __LINE__;
                goto HandleError;
            }
        }
    }
    else
    {
        float refX = 0;
        float refY = 0;
        err = (err != PAL_Ok) ? err : ReadFloat(readStruct->stream, &refX, 1);
        err = (err != PAL_Ok) ? err : ReadFloat(readStruct->stream, &refY, 1);

        if(err != PAL_Ok)
        {
            errLine = __LINE__;
            goto HandleError;
        }

        result->refCenterX = refX;
        result->refCenterY = refY;
    }

    *chunk = result;

    return err;

HandleError:
    if(result != NULL)
    {
        nsl_free(result);
    }
    NBRE_DebugLog(PAL_LogSeverityMajor, "ReadFileHeader: err=0x%08x, errLine=%d", err, errLine);
    *chunk = NULL;
    return err;
}

static PAL_Error
ReadPolylines(DomReadStatus* readStruct, NBRE_DOM_ChunkInfo* info, void** chunk)
{
    PAL_Error err = PAL_Ok;
    NBRE_DOM_NBMF* nbmf = NULL;
    NBRE_DOM_PLIN* result = (NBRE_DOM_PLIN*)nsl_malloc(sizeof(NBRE_DOM_PLIN));

    if(NULL == result)
    {
        return PAL_ErrNoMem;
    }
    nsl_memset(result, 0, sizeof(NBRE_DOM_PLIN));

    result->chunkInfo = *info;
    err = ReadUint16(readStruct->stream, &result->flag, 1);
    err = (err != PAL_Ok) ? err : ReadUint16(readStruct->stream, &result->count, 1);

    if (result->count == 0)
    {
        err = PAL_ErrNoData;
        goto HandleError;
    }

    if(PAL_Ok == err)
    {
        uint16 i=0;
        uint32 bufferCount = sizeof(NBRE_DOM_Polyline) * result->count;

        result->polylines = (NBRE_DOM_Polyline*)nsl_malloc(bufferCount);
        if(NULL == result->polylines)
        {
            err = PAL_ErrNoMem;
            goto HandleError;
        }
        nsl_memset(result->polylines, 0, bufferCount);

        err = NBRE_DOMGetChunkById(readStruct->dom, DOM_ID_NBMF, (NBRE_DOM_Chunk**)&nbmf);
        if(err != PAL_Ok)
        {
            goto HandleError;
        }

        for(i=0; i<result->count && err==PAL_Ok; ++i)
        {
            NBRE_DOM_Polyline* plins = &result->polylines[i];

            err = ReadUint16(readStruct->stream, &plins->count, 1);
            bufferCount = sizeof(double) * plins->count * FLOAT_TYPE2;
            plins->locations = (double*)nsl_malloc(bufferCount);
            if(NULL == plins->locations)
            {
                err = PAL_ErrNoMem;
                goto HandleError;
            }
            nsl_memset(plins->locations, 0, bufferCount);

            if (result->flag == 2)
            {
                err = (err != PAL_Ok) ? err : ReadFloat(readStruct->stream, (float*)plins->locations, plins->count * FLOAT_TYPE2);
            }
            else if (result->flag == 1)
            {
                double sx = nbmf->tileSizeX;
                double sy = nbmf->tileSizeY;
                err = (err != PAL_Ok) ? err : ReadFloat16v2(readStruct->stream, plins->locations, plins->count * FLOAT_TYPE2, sx, sy);
            }
            else
            {
                err = (err != PAL_Ok) ? err : ReadFloat16(readStruct->stream, (float*)plins->locations, plins->count * FLOAT_TYPE2);
            }

            if (result->flag != 1)
            {
                uint32 n = plins->count * FLOAT_TYPE2;
                float* p = (float*)plins->locations;
                for (uint32 i = 0; i < n; ++i)
                {
                    uint32 idx = n - 1 - i;
                    plins->locations[idx] = p[idx];
                }
            }
        }
    }

    if(PAL_Ok != err)
    {
        goto HandleError;
    }

    *chunk = result;

    return PAL_Ok;

HandleError:
    NBRE_DebugLog(PAL_LogSeverityMajor, "ReadPolylines: err=0x%08x", err);
    PLINDestroy(result);
    *chunk = NULL;
    return err;
}

static PAL_Error
ReadRoads(DomReadStatus* readStruct, NBRE_DOM_ChunkInfo* info, void** chunk)
{
    PAL_Error err = PAL_Ok;
    NBRE_DOM_LPTH* result = (NBRE_DOM_LPTH*)nsl_malloc(sizeof(NBRE_DOM_LPTH));

    if(NULL == result)
    {
        return PAL_ErrNoMem;
    }
    nsl_memset(result, 0, sizeof(NBRE_DOM_LPTH));

    result->chunkInfo = *info;
    err = ReadUint16(readStruct->stream, &result->flag, 1);
    err = (err != PAL_Ok) ? err : ReadUint16(readStruct->stream, &result->count, 1);

    if (result->count == 0)
    {
        err = PAL_ErrNoData;
        goto HandleError;
    }

    if(PAL_Ok == err)
    {
        uint16 i=0;
        uint32 bufferCount = sizeof(NBRE_DOM_Road) * result->count;

        result->roads = (NBRE_DOM_Road*)nsl_malloc(bufferCount);
        if(NULL == result->roads)
        {
            err = PAL_ErrNoMem;
            goto HandleError;
        }
        nsl_memset(result->roads, 0, bufferCount);

        for(i=0; i<result->count && err==PAL_Ok; ++i)
        {
            NBRE_DOM_Road* rds = &result->roads[i];

            err = ReadUint16(readStruct->stream, &rds->material, 1);
            err = (err != PAL_Ok) ? err : ReadFloat16(readStruct->stream, &rds->lineWidth, 1);
            err = (err != PAL_Ok) ? err : ReadUint16(readStruct->stream, &rds->lineIndex, 1);
        }
    }

    if(PAL_Ok != err)
    {
        goto HandleError;
    }

    *chunk = result;

    return PAL_Ok;

HandleError:
    NBRE_DebugLog(PAL_LogSeverityMajor, "ReadRoads: err=0x%08x", err);
    LPTHDestroy(result);
    *chunk = NULL;
    return err;
}

static PAL_Error
ReadRoadWithDrawOrder(DomReadStatus* readStruct, NBRE_DOM_ChunkInfo* info, void** chunk)
{
    PAL_Error err = PAL_Ok;
    NBRE_DOM_LPDR* result = (NBRE_DOM_LPDR*)nsl_malloc(sizeof(NBRE_DOM_LPDR));

    if(NULL == result)
    {
        return PAL_ErrNoMem;
    }
    nsl_memset(result, 0, sizeof(NBRE_DOM_LPDR));

    result->chunkInfo = *info;
    err = ReadUint16(readStruct->stream, &result->flag, 1);
    err = (err != PAL_Ok) ? err : ReadUint16(readStruct->stream, &result->count, 1);

    if (result->count == 0)
    {
        err = PAL_ErrNoData;
        goto HandleError;
    }

    if(PAL_Ok == err)
    {
        uint32 bufferCount = sizeof(NBRE_DOM_RoadWithDrawOrder) * result->count;

        result->roads = (NBRE_DOM_RoadWithDrawOrder*)nsl_malloc(bufferCount);
        if(NULL == result->roads)
        {
            err = PAL_ErrNoMem;
            goto HandleError;
        }
        nsl_memset(result->roads, 0, bufferCount);

        for(uint16 i=0; i<result->count && err==PAL_Ok; ++i)
        {
            NBRE_DOM_RoadWithDrawOrder* rds = &result->roads[i];

            err = (err != PAL_Ok) ? err : ReadUint16(readStruct->stream, &rds->material, 1);

            err = (err != PAL_Ok) ? err : ReadFloat16(readStruct->stream, &rds->lineWidth, 1);
            err = (err != PAL_Ok) ? err : ReadUint16(readStruct->stream, &rds->lineIndex, 1);

            err = (err != PAL_Ok) ? err : ReadUint8(readStruct->stream, &rds->roadFlag, 1);
            err = (err != PAL_Ok) ? err : ReadUint8(readStruct->stream, &rds->drawOrder, 1);

            if((rds->roadFlag & 0x02) != 0)
            {
                err = (err != PAL_Ok) ? err : ReadFloat16(readStruct->stream, &rds->distance, 1);
            }
        }
    }

    if(PAL_Ok != err)
    {
        goto HandleError;
    }

    *chunk = result;

    return PAL_Ok;

HandleError:
    NBRE_DebugLog(PAL_LogSeverityMajor, "ReadRoadWithDrawOrder: err=0x%08x", err);
    LPDRDestroy(result);
    *chunk = NULL;
    return err;
}

static PAL_Error
ReadPois(DomReadStatus* readStruct, NBRE_DOM_ChunkInfo* info, void** chunk)
{
    PAL_Error err = PAL_Ok;
    NBRE_DOM_POIS* result = (NBRE_DOM_POIS*)nsl_malloc(sizeof(NBRE_DOM_POIS));

    if(result == NULL)
    {
        return PAL_ErrNoMem;
    }
    nsl_memset(result, 0, sizeof(NBRE_DOM_POIS));

    result->chunkInfo = *info;
    err = ReadUint16(readStruct->stream, &result->flag, 1);
    err = (err != PAL_Ok) ? err : ReadUint16(readStruct->stream, &result->count, 1);

    if (result->count == 0)
    {
        err = PAL_ErrNoData;
        goto HandleError;
    }

    if(err == PAL_Ok)
    {
        uint16 i=0;
        uint32 bufferSize = sizeof(NBRE_DOM_Poi) * result->count;

        result->pois = (NBRE_DOM_Poi*)nsl_malloc(bufferSize);
        if(result->pois == NULL)
        {
            err = PAL_ErrNoMem;
            goto HandleError;
        }
        nsl_memset(result->pois, 0, bufferSize);

        for(i=0; i<result->count && err==PAL_Ok; ++i)
        {
            NBRE_DOM_Poi* poi = &result->pois[i];

            err = ReadUint16(readStruct->stream, &poi->selectedMaterialId, 1);
            err = (err != PAL_Ok) ? err : ReadUint16(readStruct->stream, &poi->pointId, 1);
            err = (err != PAL_Ok) ? err : ReadUint8(readStruct->stream, &poi->identifierLen, 1);
            if(err == PAL_Ok)
            {
                poi->identifier = (uint8*) nsl_malloc(poi->identifierLen +1);
                if(poi->identifier == NULL)
                {
                    err = PAL_ErrNoMem;
                    goto HandleError;
                }
                nsl_memset(poi->identifier, 0, poi->identifierLen +1);
            }
            err = (err != PAL_Ok) ? err : ReadUint8(readStruct->stream, poi->identifier, poi->identifierLen);
        }
    }

    if(PAL_Ok != err)
    {
        goto HandleError;
    }

    *chunk = result;

    return PAL_Ok;

HandleError:
    NBRE_DebugLog(PAL_LogSeverityMajor, "ReadPois: err=0x%08x", err);
    POISDestroy(result);
    *chunk = NULL;
    return err;
}

static PAL_Error
ReadTracks(DomReadStatus* readStruct, NBRE_DOM_ChunkInfo* info, void** chunk)
{
    PAL_Error err = PAL_Ok;

    NBRE_DOM_TPTH* result = (NBRE_DOM_TPTH*)nsl_malloc(sizeof(NBRE_DOM_TPTH));

    if(NULL == result)
    {
        return PAL_ErrNoMem;
    }
    nsl_memset(result, 0, sizeof(NBRE_DOM_TPTH));

    result->chunkInfo = *info;
    err = ReadUint16(readStruct->stream, &result->flag, 1);
    err = (err != PAL_Ok) ? err : ReadUint16(readStruct->stream, &result->count, 1);

    if(err != PAL_Ok)
    {
        goto HandleError;
    }

    // I'm assuming we should handle "count == 0" as corrupt data and not load it.
    // CreateTPTHData() will fail if count is zero.
    if (result->count != 0)
    {
        result->textPaths = (NBRE_DOM_TextPath*)nsl_malloc(sizeof(NBRE_DOM_TextPath) * result->count);
        if(NULL == result->textPaths)
        {
            err = PAL_ErrNoMem;
            goto HandleError;
        }
        nsl_memset(result->textPaths, 0, sizeof(NBRE_DOM_TextPath) * result->count);

        for(uint16 i = 0; i<result->count; ++i)
        {
            NBRE_DOM_TextPath* tps = &result->textPaths[i];

            err = ReadFloat16(readStruct->stream, &tps->lineWidth, 1);
            err = (err != PAL_Ok) ? err : ReadUint16(readStruct->stream, &tps->label, 1);
            err = (err != PAL_Ok) ? err : ReadUint16(readStruct->stream, &tps->line, 1);

            if(err != PAL_Ok)
            {
                goto HandleError;
            }
        }
    }

    *chunk = result;

    return PAL_Ok;

HandleError:
    NBRE_DebugLog(PAL_LogSeverityMajor, "ReadTracks: err=0x%08x", err);
    TPTHDestroy(result);
    *chunk = NULL;
    return err;
}

static PAL_Error
ReadTexts(DomReadStatus* readStruct, NBRE_DOM_ChunkInfo* info, void** chunk)
{
    PAL_Error err = PAL_Ok;
    uint16 i=0;
    uint32 bufferCount = 0;
    NBRE_DOM_TEXT* result = (NBRE_DOM_TEXT*)nsl_malloc(sizeof(NBRE_DOM_TEXT));

    if(NULL == result)
    {
        return PAL_ErrNoMem;
    }
    nsl_memset(result, 0, sizeof(NBRE_DOM_TEXT));

    result->chunkInfo = *info;
    err = ReadUint16(readStruct->stream, &result->flag, 1);
    err = (err != PAL_Ok) ? err : ReadUint16(readStruct->stream, &result->count, 1);

    if (result->count == 0)
    {
        err = PAL_ErrNoData;
        goto HandleError;
    }

    if(err != PAL_Ok)
    {
        err = PAL_ErrWrongFormat;
        goto HandleError;
    }

    bufferCount = sizeof(NBRE_DOM_Label) * result->count;

    result->labels = (NBRE_DOM_Label*)nsl_malloc(bufferCount);
    if(NULL == result->labels)
    {
        err = PAL_ErrNoMem;
        goto HandleError;
    }
    nsl_memset(result->labels, 0, bufferCount);

    for(i=0; i<result->count; ++i)
    {
        NBRE_DOM_Label* labs = &result->labels[i];
        uint32 stringLen = 0;
        err = ReadUint16(readStruct->stream, &labs->material, 1);
        err = (err != PAL_Ok) ? err : ReadUint8(readStruct->stream, &labs->stringLenHigh, 1);
        err = (err != PAL_Ok) ? err : ReadUint8(readStruct->stream, &labs->stringLenLow, 1);
        if(err != PAL_Ok)
        {
            goto HandleError;
        }
        stringLen = labs->stringLenHigh;
        stringLen = stringLen<<8;
        if (!(result->flag & 0x2)) // old format, ignore stringLenHigh
        {
            stringLen = 0;
        }
        stringLen += labs->stringLenLow;
        if(stringLen == 0)
        {
            err = PAL_ErrWrongFormat;
            NBRE_DebugLog(PAL_LogSeverityMajor, "ReadTexts: empty string found in NBM file.");
        }
        bufferCount = sizeof(uint8) * stringLen + 1;
        labs->string = (uint8*)nsl_malloc(bufferCount);
        if(NULL == labs->string)
        {
            err = PAL_ErrNoMem;
            goto HandleError;
        }
        nsl_memset(labs->string, 0, bufferCount);

        err = ReadUint8(readStruct->stream, labs->string, (uint32)(stringLen));
        if(err != PAL_Ok)
        {
            goto HandleError;
        }
    }

    *chunk = result;

    return PAL_Ok;

HandleError:
    NBRE_DebugLog(PAL_LogSeverityMajor, "ReadTexts: err=0x%08x", err);
    TEXTDestroy(result);
    *chunk = NULL;
    return err;
}

static PAL_Error
ReadAreas(DomReadStatus* readStruct, NBRE_DOM_ChunkInfo* info, void** chunk)
{
    PAL_Error err = PAL_Ok;
    NBRE_DOM_AREA* result = (NBRE_DOM_AREA*)nsl_malloc(sizeof(NBRE_DOM_AREA));

    if(NULL == result)
    {
        return PAL_ErrNoMem;
    }
    nsl_memset(result, 0, sizeof(NBRE_DOM_AREA));

    result->chunkInfo = *info;
    err = ReadUint16(readStruct->stream, &result->flag, 1);
    err = (err != PAL_Ok) ? err : ReadUint16(readStruct->stream, &result->count, 1);

    if (result->count == 0)
    {
        err = PAL_ErrNoData;
        goto HandleError;
    }

    if(PAL_Ok == err)
    {
        uint16 i=0;
        uint32 bufferCount = sizeof(NBRE_DOM_Area) * result->count;

        result->areas = (NBRE_DOM_Area*)nsl_malloc(bufferCount);
        if(NULL == result->areas)
        {
            err = PAL_ErrNoMem;
            goto HandleError;
        }
        nsl_memset(result->areas, 0, bufferCount);

        for(i=0; i<result->count && err==PAL_Ok; ++i)
        {
            NBRE_DOM_Area* ars = &result->areas[i];

            err = ReadUint16(readStruct->stream, &ars->material, 1);
            err = (err != PAL_Ok) ? err : ReadUint16(readStruct->stream, &ars->label, 1);
            err = (err != PAL_Ok) ? err : ReadUint16(readStruct->stream, &ars->shape, 1);
        }
    }

    if(PAL_Ok != err)
    {
        goto HandleError;
    }

    *chunk = result;

    return PAL_Ok;

HandleError:
    NBRE_DebugLog(PAL_LogSeverityMajor, "ReadAreas: err=0x%08x", err);
    AREADestroy(result);
    *chunk = NULL;
    return err;
}

static PAL_Error
ReadPathShields(DomReadStatus* readStruct, NBRE_DOM_ChunkInfo* info, void** chunk)
{
    PAL_Error err = PAL_Ok;
    NBRE_DOM_TPSH* result = (NBRE_DOM_TPSH*)nsl_malloc(sizeof(NBRE_DOM_TPSH));

    if(NULL == result)
    {
        return PAL_ErrNoMem;
    }
    nsl_memset(result, 0, sizeof(NBRE_DOM_TPSH));

    result->chunkInfo = *info;
    err = ReadUint16(readStruct->stream, &result->flag, 1);
    err = (err != PAL_Ok) ? err : ReadUint16(readStruct->stream, &result->count, 1);

    if (result->count == 0)
    {
        err = PAL_ErrNoData;
        goto HandleError;
    }

    if(PAL_Ok == err)
    {
        uint16 i=0;
        uint32 bufferCount = sizeof(NBRE_DOM_PathShield) * result->count;

        result->pathShields = (NBRE_DOM_PathShield*)nsl_malloc(bufferCount);
        if(NULL == result->pathShields)
        {
            err = PAL_ErrNoMem;
            goto HandleError;
        }
        nsl_memset(result->pathShields, 0, bufferCount);

        for(i=0; i<result->count && err==PAL_Ok; ++i)
        {
            NBRE_DOM_PathShield* pss = &result->pathShields[i];

            err = ReadUint16(readStruct->stream, &pss->shieldMaterialIndex, 1);
            err = (err != PAL_Ok) ? err : ReadUint16(readStruct->stream, &pss->labelIndex, 1);
            err = (err != PAL_Ok) ? err : ReadUint16(readStruct->stream, &pss->lineIndex, 1);
        }
    }

    if(PAL_Ok != err)
    {
        goto HandleError;
    }

    *chunk = result;

    return PAL_Ok;

HandleError:
    NBRE_DebugLog(PAL_LogSeverityMajor, "ReadPathShields: err=0x%08x", err);
    TPSHDestroy(result);
    *chunk = NULL;
    return err;
}

static PAL_Error
ReadLayers(DomReadStatus* readStruct, NBRE_DOM_ChunkInfo* info, void** chunk)
{
    PAL_Error err = PAL_Ok;
    NBRE_DOM_LAYS* result = (NBRE_DOM_LAYS*)nsl_malloc(sizeof(NBRE_DOM_LAYS));

    if(result == NULL)
    {
        return PAL_ErrNoMem;
    }
    nsl_memset(result, 0, sizeof(NBRE_DOM_LAYS));

    result->chunkInfo = *info;
    err = ReadUint16(readStruct->stream, &result->flag, 1);
    err = (err != PAL_Ok) ? err : ReadUint16(readStruct->stream, &result->count, 1);

    if (result->count == 0)
    {
        err = PAL_ErrNoData;
        goto HandleError;
    }

    if(err == PAL_Ok )
    {
        uint16 i=0;
        uint32 bufferCount = sizeof(NBRE_DOM_Layer) * result->count;

        result->layers = (NBRE_DOM_Layer*)nsl_malloc(bufferCount);
        if(NULL == result->layers)
        {
            err = PAL_ErrNoMem;
            goto HandleError;
        }
        nsl_memset(result->layers, 0, bufferCount);

        for(i=0; i<result->count && err==PAL_Ok; ++i)
        {
            NBRE_DOM_Layer* lays = &result->layers[i];

            err = ReadUint16(readStruct->stream, (uint16*)&lays->type, 1);
            err = (err != PAL_Ok) ? err : ReadFloat(readStruct->stream, &lays->nearVisibility, 1);
            err = (err != PAL_Ok) ? err : ReadFloat(readStruct->stream, &lays->farVisibility, 1);

            err = (err != PAL_Ok) ? err : ReadUint32(readStruct->stream,&lays->chunkOffset, 1);
            err = (err != PAL_Ok) ? err : ReadUint8(readStruct->stream,&lays->associateLayerIndex, 1);
        }
    }

    if(PAL_Ok != err)
    {
        goto HandleError;
    }

    *chunk = result;

    return err;

HandleError:
    NBRE_DebugLog(PAL_LogSeverityMajor, "ReadLayers: err=0x%08x", err);
    LAYSDestroy(result);
    *chunk = NULL;
    return err;
}

static PAL_Error
ReadImages(DomReadStatus* readStruct, NBRE_DOM_ChunkInfo* info, void** chunk)
{
    PAL_Error err = PAL_Ok;
    NBRE_DOM_IMGS* result = (NBRE_DOM_IMGS*)nsl_malloc(sizeof(NBRE_DOM_IMGS));

    if(result == NULL)
    {
        return PAL_ErrNoMem;
    }
    nsl_memset(result, 0, sizeof(NBRE_DOM_IMGS));

    result->chunkInfo = *info;
    err = ReadUint16(readStruct->stream, &result->flag, 1);
    err = (err != PAL_Ok) ? err : ReadUint16(readStruct->stream, &result->count, 1);

    if (result->count == 0)
    {
        err = PAL_ErrNoData;
        goto HandleError;
    }

    if(PAL_Ok == err)
    {
        uint16 i=0;
        uint32 bufferCount = sizeof(NBRE_DOM_Image) * result->count;

        result->images = (NBRE_DOM_Image*)nsl_malloc(bufferCount);
        if(NULL == result->images)
        {
            err = PAL_ErrNoMem;
            goto HandleError;
        }
        nsl_memset(result->images, 0, bufferCount);

        for(i=0; i<result->count && err==PAL_Ok; ++i)
        {
            NBRE_DOM_Image* imgs = &result->images[i];

            err = ReadUint8(readStruct->stream, &imgs->nameLen, 1);

            bufferCount = sizeof(uint8) * imgs->nameLen + 1;
            imgs->textureName = (uint8*)nsl_malloc(bufferCount);
            if(NULL == imgs->textureName)
            {
                err = PAL_ErrNoMem;
                goto HandleError;
            }
            nsl_memset(imgs->textureName, 0, bufferCount);
            err = (err != PAL_Ok) ? err : ReadUint8(readStruct->stream, imgs->textureName, imgs->nameLen);
        }
    }

    if(PAL_Ok != err)
    {
        goto HandleError;
    }

    *chunk = result;

    return PAL_Ok;

HandleError:
    NBRE_DebugLog(PAL_LogSeverityMajor, "ReadImages: err=0x%08x", err);
    IMGSDestroy(result);
    *chunk = NULL;
    return err;
}

static PAL_Error
ReadBitmaps(DomReadStatus* readStruct, NBRE_DOM_ChunkInfo* info, void** chunk)
{
    PAL_Error err = PAL_Ok;
    uint32 errLine = 0;
    NBRE_DOM_BMAP* result = (NBRE_DOM_BMAP*)nsl_malloc(sizeof(NBRE_DOM_BMAP));

    if(NULL == result)
    {
        err = PAL_ErrNoMem;
        errLine = __LINE__;
        goto HandleError;
    }
    nsl_memset(result, 0, sizeof(NBRE_DOM_BMAP));

    result->chunkInfo = *info;
    err = ReadUint16(readStruct->stream, &result->flag, 1);
    err = (err != PAL_Ok) ? err : ReadUint16(readStruct->stream, &result->count, 1);

    if (result->count == 0)
    {
        err = PAL_ErrNoData;
        errLine = __LINE__;
        goto HandleError;
    }

    if(PAL_Ok == err)
    {
        uint16 i=0;
        uint32 bufferCount = sizeof(NBRE_DOM_Bitmap) * result->count;

        result->bitmaps = (NBRE_DOM_Bitmap*)nsl_malloc(bufferCount);
        if(NULL == result->bitmaps)
        {
            err = PAL_ErrNoMem;
            errLine = __LINE__;
            goto HandleError;
        }
        nsl_memset(result->bitmaps, 0, bufferCount);

        for(i=0; i<result->count && err==PAL_Ok; ++i)
        {
            NBRE_DOM_Bitmap* bms = &result->bitmaps[i];

            err = ReadUint8(readStruct->stream, &bms->nameLen, 1);
            err = (err != PAL_Ok) ? err : readStruct->stream->Seek(PFSO_Current, bms->nameLen);
            err = (err != PAL_Ok) ? err : ReadUint32(readStruct->stream, &bms->fileSize, 1);

            bufferCount = sizeof(uint8) * bms->fileSize;

            if (bufferCount > 0)
            {
                bms->imageData = (uint8*)nsl_malloc(bufferCount);
                if(bms->imageData == NULL)
                {
                    err = PAL_ErrNoMem;
                    errLine = __LINE__;
                    goto HandleError;
                }
                err = (err != PAL_Ok) ? err : ReadUint8(readStruct->stream, bms->imageData, bufferCount);
            }
            else
            {
                // Sometimes the file size is zero. Should we handle it as error or just continue? For now continue
                NBRE_DebugLog(PAL_LogSeverityMajor, "ReadBitmaps, Empty Bitmap: err=0x%08x errLine = %d", PAL_ErrNoData, __LINE__);
            }
        }
    }

    if(PAL_Ok != err)
    {
        errLine = __LINE__;
        goto HandleError;
    }

    *chunk = result;

    return PAL_Ok;

HandleError:
    NBRE_DebugLog(PAL_LogSeverityMajor, "ReadBitmaps: err=0x%08x errLine = %d", err, errLine);
    BMPADestroy(result);
    *chunk = NULL;
    return err;
}

static PAL_Error
ReadRGB(NBRE_IOStream* stream, uint32* color)
{
    PAL_Error err = PAL_Ok;

    err = ReadUint24(stream, color, 1);
    if(err == PAL_Ok)
    {
        *color <<= 8;
        *color |= 0x000000ff;
    }
    return err;
}

static PAL_Error
ReadMaterials(DomReadStatus* readStruct, NBRE_DOM_ChunkInfo* info, void** chunk)
{
    PAL_Error err = PAL_Ok;
    NBRE_DOM_MTRL* result = (NBRE_DOM_MTRL*)nsl_malloc(sizeof(NBRE_DOM_MTRL));

    if(NULL == result)
    {
        return PAL_ErrNoMem;
    }
    nsl_memset(result, 0, sizeof(NBRE_DOM_MTRL));

    result->chunkInfo = *info;
    err = ReadFlagAndCount(readStruct->stream, &result->flag, &result->count);
    if (err != PAL_Ok)
    {
        goto HandleError;
    }

    if(PAL_Ok == err)
    {
        uint32 bufferCount = sizeof(NBRE_DOM_Material) * result->count;

        result->materials = (NBRE_DOM_Material*)nsl_malloc(bufferCount);
        if(NULL == result->materials)
        {
            err = PAL_ErrNoMem;
            goto HandleError;
        }
        nsl_memset(result->materials, 0, bufferCount);

        for(uint16 i=0; i<result->count && err==PAL_Ok; ++i)
        {
            NBRE_DOM_Material* mats = &result->materials[i];

            err = ReadUint8(readStruct->stream, &mats->type, 1);
            err = (err != PAL_Ok) ? err : ReadUint8(readStruct->stream, &mats->materialSize, 1);
            err = (err != PAL_Ok) ? err : ReadUint16(readStruct->stream, &mats->backupIndex, 1);

            switch(mats->type)
            {
            case NBRE_DMT_StandardTexturedMaterial:
                {
                    mats->stm = (NBRE_DOM_StandardTexturedMaterial*)nsl_malloc(sizeof(NBRE_DOM_StandardTexturedMaterial));
                    if(mats->stm == NULL)
                    {
                        err = PAL_ErrNoMem;
                        break;
                    }
                    nsl_memset(mats->stm, 0, sizeof(NBRE_DOM_StandardTexturedMaterial));
                    err = (err != PAL_Ok) ? err : ReadUint32(readStruct->stream, &mats->stm->color, 1);
                    err = (err != PAL_Ok) ? err : ReadUint16(readStruct->stream, &mats->stm->texId, 1);
                }
                break;
            case NBRE_DMT_StandardColorMaterial:
                {
                    mats->scm = (NBRE_DOM_StandardColorMaterial*)nsl_malloc(sizeof(NBRE_DOM_StandardColorMaterial));
                    if(mats->scm == NULL)
                    {
                        err = PAL_ErrNoMem;
                        break;
                    }
                    nsl_memset(mats->scm, 0, sizeof(NBRE_DOM_StandardColorMaterial));
                    err = (err != PAL_Ok) ? err : ReadUint32(readStruct->stream, &mats->scm->color, 1);
                }
                break;
            case NBRE_DMT_OutlinedColorMaterial:
                {
                    mats->ocm = (NBRE_DOM_OutlinedColorsMaterial*)nsl_malloc(sizeof(NBRE_DOM_OutlinedColorsMaterial));
                    if(mats->ocm == NULL)
                    {
                        err = PAL_ErrNoMem;
                        break;
                    }
                    nsl_memset(mats->ocm, 0, sizeof(NBRE_DOM_OutlinedColorsMaterial));
                    err = (err != PAL_Ok) ? err : ReadUint32(readStruct->stream, &mats->ocm->interiorColor, 1);
                    err = (err != PAL_Ok) ? err : ReadUint32(readStruct->stream, &mats->ocm->outlineColor, 1);
                    err = (err != PAL_Ok) ? err : ReadUint8(readStruct->stream, &mats->ocm->outlineWidth, 1);
                }
                break;
            case NBRE_DMT_LightedMaterial:
                {
                    mats->lm = (NBRE_DOM_LightedMaterial*)nsl_malloc(sizeof(NBRE_DOM_LightedMaterial));
                    if(mats->lm == NULL)
                    {
                        err = PAL_ErrNoMem;
                        break;
                    }
                    nsl_memset(mats->lm, 0, sizeof(NBRE_DOM_LightedMaterial));

                    mats->lm->linghting = (NBDM_DOM_Lighting*)nsl_malloc(sizeof(NBDM_DOM_Lighting));
                    if(mats->lm->linghting == NULL)
                    {
                        err = PAL_ErrNoMem;
                        break;
                    }
                    nsl_memset(mats->lm->linghting, 0, sizeof(NBDM_DOM_Lighting));
                    err = (err != PAL_Ok) ? err : ReadUint32(readStruct->stream, &mats->lm->color, 1);
                    err = (err != PAL_Ok) ? err : ReadUint16(readStruct->stream, &mats->lm->texId, 1);
                    err = (err != PAL_Ok) ? err : ReadFloat16(readStruct->stream, (float*)mats->lm->linghting, sizeof(NBDM_DOM_Lighting)/sizeof(float));
                }
                break;
            case NBRE_DMT_FontMaterial:
                {
                    mats->fm = (NBRE_DOM_FontMaterial*)nsl_malloc(sizeof(NBRE_DOM_FontMaterial));
                    if(mats->fm == NULL)
                    {
                        err = PAL_ErrNoMem;
                        break;
                    }
                    nsl_memset(mats->fm, 0, sizeof(NBRE_DOM_FontMaterial));
                    err = (err != PAL_Ok) ? err : ReadUint32(readStruct->stream, &mats->fm->textColor, 1);
                    err = (err != PAL_Ok) ? err : ReadUint32(readStruct->stream, &mats->fm->outlineColor, 1);
                    err = (err != PAL_Ok) ? err : ReadUint8(readStruct->stream, &mats->fm->family, 1);
                    err = (err != PAL_Ok) ? err : ReadUint8(readStruct->stream, &mats->fm->style, 1);
                    err = (err != PAL_Ok) ? err : ReadUint8(readStruct->stream, &mats->fm->optimal, 1);
                    err = (err != PAL_Ok) ? err : ReadUint8(readStruct->stream, &mats->fm->minimum, 1);
                    err = (err != PAL_Ok) ? err : ReadUint8(readStruct->stream, &mats->fm->maximum, 1);
                    err = (err != PAL_Ok) ? err : ReadUint8(readStruct->stream, &mats->fm->latterSpacing, 1);
                    err = (err != PAL_Ok) ? err : ReadUint8(readStruct->stream, &mats->fm->outlineWidth, 1);
                }
                break;
            case NBRE_DMT_LinePatternMaterial:
                {
                    mats->lpm = (NBRE_DOM_LinePatternMaterial*)nsl_malloc(sizeof(NBRE_DOM_LinePatternMaterial));
                    if(mats->lpm == NULL)
                    {
                        err = PAL_ErrNoMem;
                        break;
                    }
                    nsl_memset(mats->lpm, 0, sizeof(NBRE_DOM_LinePatternMaterial));
                    err = (err != PAL_Ok) ? err : ReadUint32(readStruct->stream, &mats->lpm->bitOnColor, 1);
                    err = (err != PAL_Ok) ? err : ReadUint32(readStruct->stream, &mats->lpm->bitOffColor, 1);
                    err = (err != PAL_Ok) ? err : ReadUint32(readStruct->stream, &mats->lpm->outlineColor, 1);
                    err = (err != PAL_Ok) ? err : ReadUint32(readStruct->stream, &mats->lpm->linePattern, 1);
                    err = (err != PAL_Ok) ? err : ReadUint8(readStruct->stream, &mats->lpm->outlineWidth, 1);
                }
                break;
            case NBRE_DMT_BackgroundMaterial:
                {
                    mats->bgm = (NBRE_DOM_BackgroundMaterial*)nsl_malloc(sizeof(NBRE_DOM_BackgroundMaterial));
                    if(mats->bgm == NULL)
                    {
                        err = PAL_ErrNoMem;
                        break;
                    }
                    nsl_memset(mats->bgm, 0, sizeof(NBRE_DOM_BackgroundMaterial));
                    err = (err != PAL_Ok) ? err : ReadUint32(readStruct->stream, &mats->bgm->color, 1);
                    err = (err != PAL_Ok) ? err : ReadUint16(readStruct->stream, &mats->bgm->textureIndex, 1);
                }
                break;
            case NBRE_DMT_ShieldMaterial:
                {
                    mats->sm = (NBRE_DOM_ShieldMaterial*)nsl_malloc(sizeof(NBRE_DOM_ShieldMaterial));
                    if(mats->sm == NULL)
                    {
                        err = PAL_ErrNoMem;
                        break;
                    }
                    nsl_memset(mats->sm, 0, sizeof(NBRE_DOM_ShieldMaterial));
                    err = (err != PAL_Ok) ? err : ReadUint16(readStruct->stream, &mats->sm->materialIndex, 1);
                    err = (err != PAL_Ok) ? err : ReadUint16(readStruct->stream, &mats->sm->iconIndex, 1);
                    err = (err != PAL_Ok) ? err : ReadInt8(readStruct->stream, &mats->sm->xOffset, 1);
                    err = (err != PAL_Ok) ? err : ReadInt8(readStruct->stream, &mats->sm->yOffset, 1);
                    err = (err != PAL_Ok) ? err : ReadUint8(readStruct->stream, &mats->sm->xPadding, 1);
                }
                break;
            case NBRE_DMT_DashOutlineMaterial:
                {
                    mats->dom = (NBRE_DOM_DashOutlineMaterial*)nsl_malloc(sizeof(NBRE_DOM_DashOutlineMaterial));
                    if(mats->dom == NULL)
                    {
                        err = PAL_ErrNoMem;
                        break;
                    }
                    nsl_memset(mats->dom, 0, sizeof(NBRE_DOM_DashOutlineMaterial));
                    err = (err != PAL_Ok) ? err : ReadUint32(readStruct->stream, &mats->dom->bitOnColor, 1);
                    err = (err != PAL_Ok) ? err : ReadUint32(readStruct->stream, &mats->dom->bitOffColor, 1);
                    err = (err != PAL_Ok) ? err : ReadUint32(readStruct->stream, &mats->dom->pattern, 1);
                    err = (err != PAL_Ok) ? err : ReadUint32(readStruct->stream, &mats->dom->interiorColor, 1);
                    err = (err != PAL_Ok) ? err : ReadUint8(readStruct->stream, &mats->dom->outlineWidth, 1);
                }
                break;
            case NBRE_DMT_PathArrowMaterial:
                {
                    mats->pam = (NBRE_DOM_PathArrowMaterial*)nsl_malloc(sizeof(NBRE_DOM_PathArrowMaterial));
                    if(mats->pam == NULL)
                    {
                        err = PAL_ErrNoMem;
                        break;
                    }
                    nsl_memset(mats->pam, 0, sizeof(NBRE_DOM_PathArrowMaterial));
                    err = (err != PAL_Ok) ? err : ReadUint32(readStruct->stream, &mats->pam->arrowColor, 1);
                    err = (err != PAL_Ok) ? err : ReadUint8(readStruct->stream, &mats->pam->tailWidth, 1);
                    err = (err != PAL_Ok) ? err : ReadUint8(readStruct->stream, &mats->pam->tailLength, 1);
                    err = (err != PAL_Ok) ? err : ReadUint8(readStruct->stream, &mats->pam->headWidth, 1);
                    err = (err != PAL_Ok) ? err : ReadUint8(readStruct->stream, &mats->pam->headLength, 1);
                    err = (err != PAL_Ok) ? err : ReadUint8(readStruct->stream, &mats->pam->repeat, 1);
                }
                break;
            case NBRE_DMT_HorizontalLightSourceMaterial:
                {
                    mats->hlsm = (NBRE_DOM_HorizontalLightSourceMaterial*)nsl_malloc(sizeof(NBRE_DOM_HorizontalLightSourceMaterial));
                    if(mats->hlsm == NULL)
                    {
                        err = PAL_ErrNoMem;
                        break;
                    }
                    nsl_memset(mats->hlsm, 0, sizeof(NBRE_DOM_HorizontalLightSourceMaterial));
                    err = (err != PAL_Ok) ? err : ReadFloat16(readStruct->stream, &mats->hlsm->lightAngle, 1);

                    err = (err != PAL_Ok) ? err : ReadRGB(readStruct->stream, &mats->hlsm->degree0Color);
                    err = (err != PAL_Ok) ? err : ReadRGB(readStruct->stream, &mats->hlsm->degree180Color);
                    err = (err != PAL_Ok) ? err : ReadRGB(readStruct->stream, &mats->hlsm->degree270Color);
                    err = (err != PAL_Ok) ? err : ReadRGB(readStruct->stream, &mats->hlsm->degree90Color);
                    err = (err != PAL_Ok) ? err : ReadRGB(readStruct->stream, &mats->hlsm->topColor);
                    err = (err != PAL_Ok) ? err : ReadRGB(readStruct->stream, &mats->hlsm->bottomColor);

                    err = (err != PAL_Ok) ? err : ReadUint8(readStruct->stream, &mats->hlsm->transparency, 1);
                }
                break;
            case NBRE_DMT_PinMaterial:
                {
                    mats->pinm = (NBRE_DOM_PinMaterial*)nsl_malloc(sizeof(NBRE_DOM_PinMaterial));
                    if(mats->pinm == NULL)
                    {
                        err = PAL_ErrNoMem;
                        break;
                    }
                    nsl_memset(mats->pinm, 0, sizeof(NBRE_DOM_PinMaterial));
                    err = (err != PAL_Ok) ? err : ReadUint16(readStruct->stream, &mats->pinm->iconIndex, 1);
                    err = (err != PAL_Ok) ? err : ReadInt8(readStruct->stream, &mats->pinm->xImageOffset, 1);
                    err = (err != PAL_Ok) ? err : ReadInt8(readStruct->stream, &mats->pinm->yImageOffset, 1);
                    err = (err != PAL_Ok) ? err : ReadInt8(readStruct->stream, &mats->pinm->xBubbleOffset, 1);
                    err = (err != PAL_Ok) ? err : ReadInt8(readStruct->stream, &mats->pinm->yBubbleOffset, 1);
                }
                break;
            case NBRE_DMT_OutlinedComplexColorFillMaterial:
                {
                    mats->ocfm = (NBRE_DOM_OutlinedComplexFillMaterial*)nbre_malloc(sizeof(NBRE_DOM_OutlinedComplexFillMaterial));
                    if(mats->ocfm == NULL)
                    {
                        err = PAL_ErrNoMem;
                        break;
                    }

                    nsl_memset(mats->ocfm, 0, sizeof(NBRE_DOM_OutlinedComplexFillMaterial));
                    err = (err != PAL_Ok) ? err : ReadUint32(readStruct->stream, &mats->ocfm->interiorFillColor, 1);
                    err = (err != PAL_Ok) ? err : ReadUint32(readStruct->stream, &mats->ocfm->outlineColor, 1);
                    err = (err != PAL_Ok) ? err : ReadUint8(readStruct->stream, &mats->ocfm->width, 1);
                }
                break;
            case NBRE_DMT_OutlinedSimpleColorFillMaterial:
                {
                    mats->osfm = (NBRE_DOM_OutlinedSimpleFillMaterial*)nbre_malloc(sizeof(NBRE_DOM_OutlinedSimpleFillMaterial));
                    if(mats->osfm == NULL)
                    {
                        err = PAL_ErrNoMem;
                        break;
                    }

                    nsl_memset(mats->osfm, 0, sizeof(NBRE_DOM_OutlinedSimpleFillMaterial));
                    err = (err != PAL_Ok) ? err : ReadUint32(readStruct->stream, &mats->osfm->interiorFillColor, 1);
                    err = (err != PAL_Ok) ? err : ReadUint32(readStruct->stream, &mats->osfm->outlineColor, 1);
                    err = (err != PAL_Ok) ? err : ReadUint8(readStruct->stream, &mats->osfm->width, 1);
                }
                break;
            case NBRE_DMT_ShieldWithYGapMaterial:
                {
                    mats->sym = (NBRE_DOM_ShieldWithYGapMaterial*)nsl_malloc(sizeof(NBRE_DOM_ShieldWithYGapMaterial));
                    if(mats->sym == NULL)
                    {
                        err = PAL_ErrNoMem;
                        break;
                    }
                    nsl_memset(mats->sym, 0, sizeof(NBRE_DOM_ShieldWithYGapMaterial));
                    err = (err != PAL_Ok) ? err : ReadUint16(readStruct->stream, &mats->sym->materialIndex, 1);
                    err = (err != PAL_Ok) ? err : ReadUint16(readStruct->stream, &mats->sym->iconIndex, 1);
                    err = (err != PAL_Ok) ? err : ReadInt8(readStruct->stream, &mats->sym->xOffset, 1);
                    err = (err != PAL_Ok) ? err : ReadInt8(readStruct->stream, &mats->sym->yOffset, 1);
                    err = (err != PAL_Ok) ? err : ReadUint8(readStruct->stream, &mats->sym->xPadding, 1);
                    err = (err != PAL_Ok) ? err : ReadUint8(readStruct->stream, &mats->sym->yPadding, 1);
                }
                break;
            case NBRE_DMT_RadialPinMaterial:
                {
                    mats->rpm = (NBRE_DOM_RadialPinpMaterial*)nsl_malloc(sizeof(NBRE_DOM_RadialPinpMaterial));
                    if(mats->rpm == NULL)
                    {
                        err = PAL_ErrNoMem;
                        break;
                    }
                    nsl_memset(mats->rpm, 0, sizeof(NBRE_DOM_RadialPinpMaterial));
                    err = (err != PAL_Ok) ? err : ReadUint16(readStruct->stream, &mats->rpm->unSelectedBMP, 1);
                    err = (err != PAL_Ok) ? err : ReadUint16(readStruct->stream, &mats->rpm->selectedBMP, 1);

                    err = (err != PAL_Ok) ? err : ReadInt8(readStruct->stream, &mats->rpm->unSelectedCalloutXCenterOffset, 1);
                    err = (err != PAL_Ok) ? err : ReadInt8(readStruct->stream, &mats->rpm->unSelectedCalloutYCenterOffset, 1);
                    err = (err != PAL_Ok) ? err : ReadInt8(readStruct->stream, &mats->rpm->unSelectedBubbleXCenterOffset, 1);
                    err = (err != PAL_Ok) ? err : ReadInt8(readStruct->stream, &mats->rpm->unSelectedBubbleYCenterOffset, 1);

                    err = (err != PAL_Ok) ? err : ReadInt8(readStruct->stream, &mats->rpm->selectedCalloutXCenterOffset, 1);
                    err = (err != PAL_Ok) ? err : ReadInt8(readStruct->stream, &mats->rpm->selectedCalloutYCenterOffset, 1);
                    err = (err != PAL_Ok) ? err : ReadInt8(readStruct->stream, &mats->rpm->selectedBubbleXCenterOffset, 1);
                    err = (err != PAL_Ok) ? err : ReadInt8(readStruct->stream, &mats->rpm->selectedBubbleYCenterOffset, 1);

                    err = (err != PAL_Ok) ? err : ReadUint32(readStruct->stream, &mats->rpm->selectedCircleInteriorColor, 1);
                    err = (err != PAL_Ok) ? err : ReadUint32(readStruct->stream, &mats->rpm->unSelectedCircleInteriorColor, 1);

                    err = (err != PAL_Ok) ? err : ReadFloat(readStruct->stream, &mats->rpm->displayNormalWidth, 1);
                    err = (err != PAL_Ok) ? err : ReadFloat(readStruct->stream, &mats->rpm->dispalyNormalHeight, 1);
                    err = (err != PAL_Ok) ? err : ReadFloat(readStruct->stream, &mats->rpm->displaySelectedWidth, 1);
                    err = (err != PAL_Ok) ? err : ReadFloat(readStruct->stream, &mats->rpm->dispalySelectedHeight, 1);

                    err = (err != PAL_Ok) ? err : ReadUint8(readStruct->stream, &mats->rpm->circleOutlineWidth, 1);

                    err = (err != PAL_Ok) ? err : ReadUint32(readStruct->stream, &mats->rpm->selectedCircleOutlineBitOnColor, 1);
                    err = (err != PAL_Ok) ? err : ReadUint32(readStruct->stream, &mats->rpm->selectedCircleOutlineBitOffColor, 1);
                    err = (err != PAL_Ok) ? err : ReadUint32(readStruct->stream, &mats->rpm->unSelectedCircleOutlineBitOnColor, 1);
                    err = (err != PAL_Ok) ? err : ReadUint32(readStruct->stream, &mats->rpm->unSelectedCircleOutlineBitOffColor, 1);
                    err = (err != PAL_Ok) ? err : ReadUint32(readStruct->stream, &mats->rpm->linePattern, 1);
                }
                break;
            case NBRE_DMT_StaticPOIMaterial:
                {
                    mats->spm = (NBRE_DOM_StaticPOIMaterial*)nsl_malloc(sizeof(NBRE_DOM_StaticPOIMaterial));
                    if(mats->spm == NULL)
                    {
                        err = PAL_ErrNoMem;
                        break;
                    }
                    nsl_memset(mats->spm, 0, sizeof(NBRE_DOM_StaticPOIMaterial));
                    err = (err != PAL_Ok) ? err : ReadUint16(readStruct->stream, &mats->spm->iconBPMIndex, 1);
                    err = (err != PAL_Ok) ? err : ReadUint16(readStruct->stream, &mats->spm->selectedBMPIndex, 1);
                    err = (err != PAL_Ok) ? err : ReadUint16(readStruct->stream, &mats->spm->unSelectedBMPIndex, 1);
                    err = (err != PAL_Ok) ? err : ReadInt8(readStruct->stream, &mats->spm->calloutXCenterOffset, 1);
                    err = (err != PAL_Ok) ? err : ReadInt8(readStruct->stream, &mats->spm->calloutYCenterOffset, 1);
                    err = (err != PAL_Ok) ? err : ReadInt8(readStruct->stream, &mats->spm->bubbleXCenterOffset, 1);
                    err = (err != PAL_Ok) ? err : ReadInt8(readStruct->stream, &mats->spm->bubbleYCenterOffset, 1);
                    err = (err != PAL_Ok) ? err : ReadInt8(readStruct->stream, &mats->spm->distanceToAnother, 1);
                    err = (err != PAL_Ok) ? err : ReadInt8(readStruct->stream, &mats->spm->distanceToLabel, 1);
                    err = (err != PAL_Ok) ? err : ReadInt8(readStruct->stream, &mats->spm->distanceToPoi, 1);
                    err = (err != PAL_Ok) ? err : ReadUint8(readStruct->stream, &mats->spm->iconWidth, 1);
                    err = (err != PAL_Ok) ? err : ReadUint8(readStruct->stream, &mats->spm->iconHeight, 1);
                }
                break;
            case NBRE_DMT_PolylineCapMaterial:
                {
                    mats->pcm = NBRE_NEW NBRE_DOM_PolylineCapMaterial;
                    if(mats->pcm == NULL)
                    {
                        err = PAL_ErrNoMem;
                        break;
                    }
                    err = (err != PAL_Ok) ? err : ReadUint32(readStruct->stream, &mats->pcm->interiorColor, 1);
                    err = (err != PAL_Ok) ? err : ReadUint32(readStruct->stream, &mats->pcm->outlineColor, 1);
                    err = (err != PAL_Ok) ? err : ReadUint8(readStruct->stream, &mats->pcm->outlineWidth, 1);
                    err = (err != PAL_Ok) ? err : ReadUint8(readStruct->stream, &mats->pcm->startType, 1);
                    err = (err != PAL_Ok) ? err : ReadUint8(readStruct->stream, &mats->pcm->endType, 1);
                    err = (err != PAL_Ok) ? err : ReadUint16(readStruct->stream, &mats->pcm->height, 1);
                    err = (err != PAL_Ok) ? err : ReadUint16(readStruct->stream, &mats->pcm->width, 1);
                    err = (err != PAL_Ok) ? err : ReadUint16(readStruct->stream, &mats->pcm->radius, 1);
                }
                break;
            case NBRE_DMT_BitMapPatternMaterial:
                {
                    mats->bmpm = NBRE_NEW NBRE_DOM_BitMapPatternMaterial;
                    if(mats->bmpm == NULL)
                    {
                        err = PAL_ErrNoMem;
                        break;
                    }
                    err = (err != PAL_Ok) ? err : ReadUint32(readStruct->stream, &mats->bmpm->color, 1);
                    err = (err != PAL_Ok) ? err : ReadUint16(readStruct->stream, &mats->bmpm->textureID, 1);
                    err = (err != PAL_Ok) ? err : ReadUint16(readStruct->stream, &mats->bmpm->distance, 1);
                    err = (err != PAL_Ok) ? err : ReadUint16(readStruct->stream, &mats->bmpm->width, 1);
                    err = (err != PAL_Ok) ? err : ReadUint16(readStruct->stream, &mats->bmpm->height, 1);
                }
                break;
            case NBRE_DMT_OutlinedHoleyMaterial:
                {
                    mats->ohm = (NBRE_DOM_OutlinedHoleyMaterial*)nbre_malloc(sizeof(NBRE_DOM_OutlinedHoleyMaterial));
                    if(mats->ohm == NULL)
                    {
                        err = PAL_ErrNoMem;
                        break;
                    }

                    nsl_memset(mats->ohm, 0, sizeof(NBRE_DOM_OutlinedHoleyMaterial));
                    err = (err != PAL_Ok) ? err : ReadUint32(readStruct->stream, &mats->ohm->interiorFillColor, 1);
                    err = (err != PAL_Ok) ? err : ReadUint32(readStruct->stream, &mats->ohm->outlineColor, 1);
                    err = (err != PAL_Ok) ? err : ReadUint8(readStruct->stream, &mats->ohm->width, 1);
                }
                break;
            default:
                {
                    NBRE_DebugLog(PAL_LogSeverityMajor, "NBRE_DomCreateFromFile, ReadMaterials, Unknown material type:%d, size=%d, backupIndex=%d", mats->type, mats->materialSize, mats->backupIndex);
                    readStruct->stream->Seek(PFSO_Current, mats->materialSize);
                }
                break;
            }
        }
    }

    if(PAL_Ok != err)
    {
        goto HandleError;
    }

    *chunk = result;

    return PAL_Ok;

HandleError:
    NBRE_DebugLog(PAL_LogSeverityMajor, "ReadMaterials: err=0x%08x", err);
    MTRLDestroy(result);
    *chunk = NULL;
    return err;
}

PAL_Error FloatReader(uint16 flag, NBRE_IOStream* stream, float* buffer, uint8 geomFlag, uint32 count, double tileSizeX, double tileSizeY)
{
    uint32 c = FLOAT_TYPE3;
    switch (geomFlag)
    {
    case 0x00:
        c = FLOAT_TYPE3;
        break;
    case 0x01:
        c = FLOAT_TYPE3 + FLOAT_TYPE3;
        break;
    case 0x02:
        c = FLOAT_TYPE3 + FLOAT_TYPE2;
        break;
    case 0x03:
        c = FLOAT_TYPE3 + FLOAT_TYPE3 + FLOAT_TYPE2;
        break;
    default:
        return PAL_ErrWrongFormat;
    }

    if(flag == 0 || flag == 2)
    {
        if (flag == 0)
        {
            return ReadFloat16(stream, buffer, count * c);
        }
        else
        {
            return ReadFloat(stream, buffer, count * c);
        }

    }
    else if(flag == 1)
    {
        PAL_Error err = PAL_Ok;
        switch (geomFlag)
        {
        case 0x00:
            for (uint32 i = 0; i < count; ++i)
            {
                err = ReadFloat16v2(stream, buffer + i * c, FLOAT_TYPE2, tileSizeX, tileSizeY);
                if (err != PAL_Ok)
                {
                    return err;
                }
                err = ReadFloat16v2(stream, buffer + i * c + FLOAT_TYPE2, 1, HALF2_Z_RANGE);
                if (err != PAL_Ok)
                {
                    return err;
                }
                buffer[i * c + FLOAT_TYPE2] = METER_TO_MERCATOR(buffer[i * c + FLOAT_TYPE2]);
            }
            break;
        case 0x01:
            for (uint32 i = 0; i < count; ++i)
            {
                err = ReadFloat16v2(stream, buffer + i * c, FLOAT_TYPE2, tileSizeX, tileSizeY);
                if (err != PAL_Ok)
                {
                    return err;
                }
                err = ReadFloat16v2(stream, buffer + i * c + FLOAT_TYPE2, 1, HALF2_Z_RANGE);
                if (err != PAL_Ok)
                {
                    return err;
                }
                buffer[i * c + FLOAT_TYPE2] = METER_TO_MERCATOR(buffer[i * c + FLOAT_TYPE2]);
                err = ReadFloat16v2(stream, buffer + i * c + FLOAT_TYPE3, FLOAT_TYPE3, HALF2_NORMAL_RANGE);
                if (err != PAL_Ok)
                {
                    return err;
                }
            }
            break;
        case 0x02:
            for (uint32 i = 0; i < count; ++i)
            {
                err = ReadFloat16v2(stream, buffer + i * c, FLOAT_TYPE2, tileSizeX, tileSizeY);
                if (err != PAL_Ok)
                {
                    return err;
                }
                err = ReadFloat16v2(stream, buffer + i * c + FLOAT_TYPE2, 1, HALF2_Z_RANGE);
                if (err != PAL_Ok)
                {
                    return err;
                }
                buffer[i * c + FLOAT_TYPE2] = METER_TO_MERCATOR(buffer[i * c + FLOAT_TYPE2]);
                err = ReadFloat16v2(stream, buffer + i * c + FLOAT_TYPE3, FLOAT_TYPE2, HALF2_TEXCOORD_RANGE);
                if (err != PAL_Ok)
                {
                    return err;
                }
            }
            break;
        case 0x03:
            for (uint32 i = 0; i < count; ++i)
            {
                err = ReadFloat16v2(stream, buffer + i * c, FLOAT_TYPE2, tileSizeX, tileSizeY);
                if (err != PAL_Ok)
                {
                    return err;
                }
                err = ReadFloat16v2(stream, buffer + i * c + FLOAT_TYPE2, 1, HALF2_Z_RANGE);
                if (err != PAL_Ok)
                {
                    return err;
                }
                buffer[i * c + FLOAT_TYPE2] = METER_TO_MERCATOR(buffer[i * c + FLOAT_TYPE2]);
                err = ReadFloat16v2(stream, buffer + i * c + FLOAT_TYPE3, FLOAT_TYPE3, HALF2_NORMAL_RANGE);
                if (err != PAL_Ok)
                {
                    return err;
                }
                err = ReadFloat16v2(stream, buffer + i * c + FLOAT_TYPE3 + FLOAT_TYPE3, FLOAT_TYPE2, HALF2_TEXCOORD_RANGE);
                if (err != PAL_Ok)
                {
                    return err;
                }
            }
            break;
        default:
            return PAL_ErrWrongFormat;
        }
        return err;
    }
    else
    {
        return PAL_ErrWrongFormat;
    }
}

static PAL_Error
ReadGeometries(DomReadStatus* readStruct, NBRE_DOM_ChunkInfo* info, void** chunk)
{
    PAL_Error err = PAL_Ok;
    float* dataBuffer = NULL;
    uint16 geoIndex=0;
    uint32 bufferCount = 0;
    uint32 errLine = 0;
    NBRE_DOM_NBMF* nbmf = NULL;
    NBRE_DOM_GEOM* result = (NBRE_DOM_GEOM*)nsl_malloc(sizeof(NBRE_DOM_GEOM));
    if(NULL == result)
    {
        err = PAL_ErrNoMem;
        errLine = __LINE__;
        goto HandleError;
    }
    nsl_memset(result, 0, sizeof(NBRE_DOM_GEOM));

    result->chunkInfo = *info;
    err = ReadUint16(readStruct->stream, &result->flag, 1);
    err = (err != PAL_Ok) ? err : ReadUint16(readStruct->stream, &result->count, 1);

    if (result->count == 0)
    {
        err = PAL_ErrNoData;
        goto HandleError;
    }

    if(err != PAL_Ok)
    {
        errLine = __LINE__;
        goto HandleError;
    }

    bufferCount = sizeof(NBRE_DOM_Geometry) * result->count;
    result->geometries = (NBRE_DOM_Geometry*)nsl_malloc(bufferCount);
    if(NULL == result->geometries)
    {
        err = PAL_ErrNoMem;
        errLine = __LINE__;
        goto HandleError;
    }
    nsl_memset(result->geometries, 0, bufferCount);

    err = NBRE_DOMGetChunkById(readStruct->dom, DOM_ID_NBMF, (NBRE_DOM_Chunk**)&nbmf);
    if(err != PAL_Ok)
    {
        goto HandleError;
    }

    for(geoIndex=0; geoIndex<result->count && err==PAL_Ok; ++geoIndex)
    {
        NBRE_DOM_Geometry* geos = &result->geometries[geoIndex];
        uint16 j = 0;
        uint32 copyPosCount = sizeof(float) * FLOAT_TYPE3;
        uint32 copyNorCount = sizeof(float) * FLOAT_TYPE3;
        uint32 copyTexCount = sizeof(float) * FLOAT_TYPE2;
        uint32 copyInterval = 0;

        err = ReadUint8(readStruct->stream, &geos->flag, 1);
        err = (err != PAL_Ok) ? err : ReadUint16(readStruct->stream, &geos->size, 1);

        bufferCount = sizeof(float) * FLOAT_TYPE3 * geos->size;
        geos->position = (float*)nsl_malloc(bufferCount);
        if(NULL == geos->position)
        {
            err = PAL_ErrNoMem;
            errLine = __LINE__;
            goto HandleError;
        }
        nsl_memset(geos->position, 0, bufferCount);

        switch(geos->flag)
        {
        case 0x0:
            err = (err != PAL_Ok) ? err : FloatReader(result->flag, readStruct->stream, geos->position, geos->flag, geos->size
                ,nbmf->tileSizeX
                ,nbmf->tileSizeY);
            break;

        case 0x1:
            copyInterval = copyPosCount + copyNorCount;
            bufferCount = sizeof(float) * (FLOAT_TYPE3 + FLOAT_TYPE3) * geos->size;
            dataBuffer = (float*)nsl_malloc(bufferCount);
            if(dataBuffer == NULL)
            {
                err = PAL_ErrNoMem;
                errLine = __LINE__;
                goto HandleError;
            }
            nsl_memset(dataBuffer, 0, bufferCount);

            err = (err != PAL_Ok) ? err : FloatReader(result->flag, readStruct->stream, dataBuffer, geos->flag, geos->size
                ,nbmf->tileSizeX
                ,nbmf->tileSizeY);

            bufferCount = sizeof(float) * FLOAT_TYPE3 * geos->size;;
            geos->normal = (float*)nsl_malloc(bufferCount);
            if(NULL == geos->normal)
            {
                err = PAL_ErrNoMem;
                errLine = __LINE__;
                goto HandleError;
            }
            nsl_memset(geos->normal, 0, bufferCount);

            for(j=0; j<geos->size; j++)
            {
                nsl_memcpy((uint8*)geos->position + j * copyPosCount, (uint8*)dataBuffer + j * copyInterval, copyPosCount);
                nsl_memcpy((uint8*)geos->normal + j * copyNorCount, (uint8*)dataBuffer + j * copyInterval + copyPosCount, copyNorCount);
            }
            break;

        case 0x2:
            copyInterval = copyPosCount + copyTexCount;
            bufferCount = copyInterval * geos->size;
            dataBuffer = (float*)nsl_malloc(bufferCount);
            if(dataBuffer == NULL)
            {
                err = PAL_ErrNoMem;
                errLine = __LINE__;
                goto HandleError;
            }
            nsl_memset(dataBuffer, 0, bufferCount);

            err = (err != PAL_Ok) ? err : FloatReader(result->flag, readStruct->stream, dataBuffer, geos->flag, geos->size
                ,nbmf->tileSizeX
                ,nbmf->tileSizeY);

            bufferCount =  sizeof(float) * FLOAT_TYPE2 * geos->size;;
            geos->texCoord = (float*)nsl_malloc(bufferCount);
            if(NULL == geos->texCoord)
            {
                err = PAL_ErrNoMem;
                errLine = __LINE__;
                goto HandleError;
            }
            nsl_memset(geos->texCoord, 0, bufferCount);

            for(j=0; j<geos->size; j++)
            {
                nsl_memcpy((uint8*)geos->position + j * copyPosCount, (uint8*)dataBuffer + j * copyInterval, copyPosCount);
                nsl_memcpy((uint8*)geos->normal + j * copyTexCount, (uint8*)dataBuffer + j * copyInterval + copyPosCount, copyTexCount);
            }
            break;

        case 0x3:
            copyInterval = copyPosCount + copyNorCount + copyTexCount;
            bufferCount = copyInterval * geos->size;
            dataBuffer = (float*)nsl_malloc(bufferCount);
            if(dataBuffer == NULL)
            {
                err = PAL_ErrNoMem;
                errLine = __LINE__;
                goto HandleError;
            }
            nsl_memset(dataBuffer, 0, bufferCount);

            err = (err != PAL_Ok) ? err : FloatReader(result->flag, readStruct->stream, dataBuffer, geos->flag, geos->size
                ,nbmf->tileSizeX
                ,nbmf->tileSizeY);

            bufferCount =  sizeof(float) * FLOAT_TYPE3 * geos->size;
            geos->normal = (float*)nsl_malloc(bufferCount);
            if(NULL == geos->normal)
            {
                err = PAL_ErrNoMem;
                errLine = __LINE__;
                goto HandleError;
            }
            nsl_memset(geos->normal, 0, bufferCount);

            bufferCount =  sizeof(float) * FLOAT_TYPE2 * geos->size;;
            geos->texCoord = (float*)nsl_malloc(bufferCount);
            if(NULL == geos->texCoord)
            {
                err = PAL_ErrNoMem;
                errLine = __LINE__;
                goto HandleError;
            }
            nsl_memset(geos->texCoord, 0, bufferCount);

            for(j=0; j<geos->size; j++)
            {
                nsl_memcpy((uint8*)geos->position + j * copyPosCount, (uint8*)dataBuffer + j * copyInterval, copyPosCount);
                nsl_memcpy((uint8*)geos->normal + j * copyNorCount, (uint8*)dataBuffer + j * copyInterval + copyPosCount, copyNorCount);
                nsl_memcpy((uint8*)geos->texCoord + j * copyTexCount, (uint8*)dataBuffer + j * copyInterval + copyPosCount + copyNorCount, copyTexCount);
            }
            break;

        default:
            err = PAL_Failed;
            errLine = __LINE__;
            goto HandleError;
        }
        if (dataBuffer != NULL)
        {
            nsl_free(dataBuffer);
            dataBuffer = NULL;
        }
    }

    if(PAL_Ok != err)
    {
        goto HandleError;
    }

    *chunk = result;

    return PAL_Ok;

HandleError:
    NBRE_DebugLog(PAL_LogSeverityMajor, "ReadGeometries: err=0x%08x errLine = %d", err, errLine);
    GEOMDestroy(result);
    if (dataBuffer != NULL)
    {
        nsl_free(dataBuffer);
    }
    *chunk = NULL;
    return err;
}

static PAL_Error
ReadMeshes(DomReadStatus* readStruct, NBRE_DOM_ChunkInfo* info, void** chunk)
{
    PAL_Error err = PAL_Ok;
    uint32 errLine = 0;
    uint16 i=0;
    uint32 bufferCount = 0;
    NBRE_DOM_MESH* result = (NBRE_DOM_MESH*)nsl_malloc(sizeof(NBRE_DOM_MESH));

    if(NULL == result)
    {
        err = PAL_ErrNoMem;
        errLine = __LINE__;
        goto HandleError;
    }
    nsl_memset(result, 0, sizeof(NBRE_DOM_MESH));

    result->chunkInfo = *info;
    err = ReadUint16(readStruct->stream, &result->flag, 1);
    err = (err != PAL_Ok) ? err : ReadUint16(readStruct->stream, &result->count, 1);

    if (result->count == 0)
    {
        err = PAL_ErrNoData;
        goto HandleError;
    }

    if(err != PAL_Ok)
    {
        goto HandleError;
    }

    bufferCount = sizeof(NBRE_DOM_Mesh) * result->count;

    result->meshes = (NBRE_DOM_Mesh*)nsl_malloc(bufferCount);
    if(NULL == result->meshes)
    {
        err = PAL_ErrNoMem;
        errLine = __LINE__;
        goto HandleError;
    }
    nsl_memset(result->meshes, 0, bufferCount);

    for(i = 0; i < result->count; ++i)
    {
        NBRE_DOM_Mesh* mes = &result->meshes[i];

        err = ReadUint8(readStruct->stream, &mes->flag, 1);
        err = (err != PAL_Ok) ? err : ReadUint16(readStruct->stream, &mes->materialIndex, 1);
        err = (err != PAL_Ok) ? err : ReadUint16(readStruct->stream, &mes->labelIndex, 1);
        err = (err != PAL_Ok) ? err : ReadUint16(readStruct->stream, &mes->geometryIndex, 1);
        if(err != PAL_Ok)
        {
            goto HandleError;
        }

        if(mes->flag == 0)
        {
            err = ReadUint16(readStruct->stream, &mes->indicesCount, 1);
            if(err != PAL_Ok)
            {
                goto HandleError;
            }

            bufferCount = sizeof(uint16) * mes->indicesCount;
            mes->indices = (uint16*)nsl_malloc(bufferCount);
            if(NULL == mes->indices)
            {
                err = PAL_ErrNoMem;
                errLine = __LINE__;
                goto HandleError;
            }
            nsl_memset(mes->indices, 0, bufferCount);

            err = ReadUint16(readStruct->stream, mes->indices, mes->indicesCount);
            if(err != PAL_Ok)
            {
                goto HandleError;
            }
        }
    }

    *chunk = result;

    return PAL_Ok;

HandleError:
    NBRE_DebugLog(PAL_LogSeverityMajor, "ReadMeshes: err=0x%08x errLine = %d", err, errLine);
    MESHDestroy(result);
    *chunk = NULL;
    return err;
}

static PAL_Error
ReadObjects(DomReadStatus* readStruct, NBRE_DOM_ChunkInfo* info, void** chunk)
{
    PAL_Error err = PAL_Ok;
    NBRE_DOM_OBJS* result = (NBRE_DOM_OBJS*)nsl_malloc(sizeof(NBRE_DOM_OBJS));

    if(NULL == result)
    {
        return PAL_ErrNoMem;
    }
    nsl_memset(result, 0, sizeof(NBRE_DOM_OBJS));

    result->chunkInfo = *info;
    err = ReadUint16(readStruct->stream, &result->flag, 1);
    err = (err != PAL_Ok) ? err : ReadUint16(readStruct->stream, &result->count, 1);

    if (result->count == 0)
    {
        err = PAL_ErrNoData;
        goto HandleError;
    }

    if(PAL_Ok == err)
    {
        uint16 i=0;
        uint32 bufferCount = sizeof(NBRE_DOM_Object) * result->count;

        result->objects = (NBRE_DOM_Object*)nsl_malloc(bufferCount);
        if(NULL == result->objects)
        {
            err = PAL_ErrNoMem;
            goto HandleError;
        }
        nsl_memset(result->objects, 0, bufferCount);

        for(i=0; i<result->count && err==PAL_Ok; ++i)
        {
            NBRE_DOM_Object* objs = &result->objects[i];

            err = ReadUint32(readStruct->stream, &objs->chunkID, 1);
            err = (err != PAL_Ok) ? err : ReadUint16(readStruct->stream, &objs->objIndex, 1);
        }
    }

    if(PAL_Ok != err)
    {
        goto HandleError;
    }

    *chunk = result;

    return PAL_Ok;

HandleError:
    NBRE_DebugLog(PAL_LogSeverityMajor, "ReadObjects: err=0x%08x", err);
    OBJSDestroy(result);
    *chunk = NULL;
    return err;
}

static PAL_Error
ReadScenes(DomReadStatus* readStruct, NBRE_DOM_ChunkInfo* info, void** chunk)
{
    PAL_Error err = PAL_Ok;
    NBRE_DOM_SEEN* result = (NBRE_DOM_SEEN*)nsl_malloc(sizeof(NBRE_DOM_SEEN));

    if(result==NULL)
    {
        err = PAL_ErrNoData;
        goto HandleError;
    }
    nsl_memset(result, 0, sizeof(NBRE_DOM_SEEN));

    result->chunkInfo = *info;
    err = ReadUint16(readStruct->stream, &result->flag, 1);
    err = (err != PAL_Ok) ? err : ReadUint16(readStruct->stream, &result->count, 1);

    if (result->count == 0)
    {
        err = PAL_ErrNoData;
        goto HandleError;
    }

    if(err == PAL_Ok)
    {
        uint16 i=0;
        uint32 bufferCount = sizeof(NBRE_DOM_Scene) * result->count;

        result->scenes = (NBRE_DOM_Scene*)nsl_malloc(bufferCount);
        if(result->scenes == NULL)
        {
            err = PAL_ErrNoMem;
            goto HandleError;
        }
        nsl_memset(result->scenes, 0, bufferCount);

        for(i=0; i<result->count && err==PAL_Ok; ++i)
        {
            NBRE_DOM_Scene* sces = &result->scenes[i];
            nbre_assert(sces!=NULL);
            err = ReadUint32(readStruct->stream, &sces->chunkID, 1);
            err = (err!=PAL_Ok)?err:ReadUint8(readStruct->stream, &sces->placementFlag, 1);

            if(sces->placementFlag != 0)
            {
                err = (err!=PAL_Ok)?err:ReadFloat16(readStruct->stream, sces->placementMatrix, 16);
            }
            err = (err!=PAL_Ok)?err:ReadUint16(readStruct->stream, &sces->objCount, 1);

            bufferCount = sizeof(uint16) * sces->objCount;
            sces->objIndex = (uint16*)nsl_malloc(bufferCount);
            if(sces->objIndex == NULL || err != PAL_Ok)
            {
                err = PAL_ErrNoMem;
                goto HandleError;
            }
            nsl_memset(sces->objIndex, 0, bufferCount);

            err = (err != PAL_Ok) ? err : ReadUint16(readStruct->stream, sces->objIndex, sces->objCount);
        }
    }

    if(err != PAL_Ok)
    {
        goto HandleError;
    }

    *chunk = result;

    return PAL_Ok;

HandleError:
    NBRE_DebugLog(PAL_LogSeverityMajor, "ReadScenes: err=0x%08x", err);
    SEENDestroy(result);
    *chunk = NULL;
    return err;
}

static PAL_Error
ReadPoints(DomReadStatus* readStruct, NBRE_DOM_ChunkInfo* info, void** chunk)
{
    PAL_Error err = PAL_Ok;
    uint16 i=0;
    uint32 bufferCount = 0;
    NBRE_DOM_NBMF* nbmf = NULL;
    NBRE_DOM_PNTS* result = (NBRE_DOM_PNTS*)nsl_malloc(sizeof(NBRE_DOM_PNTS));

    if(NULL == result)
    {
        return PAL_ErrNoMem;
    }
    nsl_memset(result, 0, sizeof(NBRE_DOM_PNTS));

    result->chunkInfo = *info;
    err = ReadUint16(readStruct->stream, &result->flag, 1);
    err = (err != PAL_Ok) ? err : ReadUint16(readStruct->stream, &result->count, 1);

    if (result->count == 0)
    {
        err = PAL_ErrNoData;
        goto HandleError;
    }

    if(err != PAL_Ok)
    {
        goto HandleError;
    }

    bufferCount = sizeof(NBRE_DOM_Point) * result->count;

    result->points = (NBRE_DOM_Point*)nsl_malloc(bufferCount);
    if(NULL == result->points)
    {
        err = PAL_ErrNoMem;
        goto HandleError;
    }
    nsl_memset(result->points, 0, bufferCount);

    err = NBRE_DOMGetChunkById(readStruct->dom, DOM_ID_NBMF, (NBRE_DOM_Chunk**)&nbmf);
    if(err != PAL_Ok)
    {
        goto HandleError;
    }

    for(i=0; i<result->count; ++i)
    {
        NBRE_DOM_Point* pts = &result->points[i];
        err = ReadUint16(readStruct->stream, &pts->material, 1);

        if (result->flag == 2)
        {
            err = (err != PAL_Ok) ? err : ReadFloat(readStruct->stream, pts->location, 2);
        }
        else if (result->flag == 1)
        {
            err = (err != PAL_Ok) ? err : ReadFloat16v2(readStruct->stream, pts->location, 2
                , nbmf->tileSizeX
                , nbmf->tileSizeY);
        }
        else
        {
            err = (err != PAL_Ok) ? err : ReadFloat16(readStruct->stream, pts->location, 2);
        }

        err = (err != PAL_Ok) ? err : ReadUint16(readStruct->stream, &pts->labelIndex, 1);
        if(err != PAL_Ok)
        {
            goto HandleError;
        }
    }

    *chunk = result;
    return PAL_Ok;

HandleError:
    NBRE_DebugLog(PAL_LogSeverityMajor, "ReadPoints: err=0x%08x", err);
    PNTSDestroy(result);
    *chunk = NULL;
    return err;
}

static PAL_Error
ReadExternalObjects(DomReadStatus* readStruct, NBRE_DOM_ChunkInfo* info, void** chunk)
{
    PAL_Error err = PAL_Ok;
    uint16 i=0;
    uint32 bufferCount = 0;
    NBRE_DOM_OBJX* result = (NBRE_DOM_OBJX*)nsl_malloc(sizeof(NBRE_DOM_OBJX));

    if(NULL == result)
    {
        return PAL_ErrNoMem;
    }
    nsl_memset(result, 0, sizeof(NBRE_DOM_OBJX));

    result->chunkInfo = *info;
    err = ReadUint16(readStruct->stream, &result->flag, 1);
    err = (err != PAL_Ok) ? err : ReadUint16(readStruct->stream, &result->count, 1);

    if (result->count == 0)
    {
        err = PAL_ErrNoData;
        goto HandleError;
    }

    if(err != PAL_Ok)
    {
        goto HandleError;
    }

    bufferCount = sizeof(NBRE_DOM_ExternalObject) * result->count;

    result->externalObjs = (NBRE_DOM_ExternalObject*)nsl_malloc(bufferCount);
    if(NULL == result->externalObjs)
    {
        err = PAL_ErrNoMem;
        goto HandleError;
    }
    nsl_memset(result->externalObjs, 0, bufferCount);

    for(i=0; i<result->count; ++i)
    {
        NBRE_DOM_ExternalObject* objx = &result->externalObjs[i];

        err = ReadUint16(readStruct->stream, &objx->fileNameLen, 1);
        if(err != PAL_Ok)
        {
            goto HandleError;
        }

        objx->fileName = (char*)nsl_malloc(objx->fileNameLen + 1);
        if(NULL == objx->fileName)
        {
            err = PAL_ErrNoMem;
            goto HandleError;
        }
        nsl_memset(objx->fileName, 0, objx->fileNameLen + 1);

        err = ReadChar(readStruct->stream, objx->fileName, objx->fileNameLen);
        err = (err != PAL_Ok) ? err : ReadUint32(readStruct->stream, &objx->checkSum, 1);
        err = (err != PAL_Ok) ? err : ReadUint16(readStruct->stream, &objx->count, 1);
        if(err != PAL_Ok)
        {
            goto HandleError;
        }

        bufferCount = objx->count * sizeof(uint16);
        objx->index = (uint16*)nsl_malloc(bufferCount);
        if(NULL == objx->index)
        {
            err = PAL_ErrNoMem;
            goto HandleError;
        }
        nsl_memset(objx->index, 0, bufferCount);

        err = ReadUint16(readStruct->stream, objx->index, objx->count);
        if(err != PAL_Ok)
        {
            goto HandleError;
        }
    }

    *chunk = result;
    return PAL_Ok;

HandleError:
    NBRE_DebugLog(PAL_LogSeverityMajor, "ReadExternalObjects: err=0x%08x", err);
    OBJXDestroy(result);
    *chunk = NULL;
    return err;
}

static PAL_Error
ReadSplines(DomReadStatus* readStruct, NBRE_DOM_ChunkInfo* info, void** chunk)
{
    PAL_Error err = PAL_Ok;
    uint16 i=0;
    uint32 bufferCount = 0;
    NBRE_DOM_SPLN* result = (NBRE_DOM_SPLN*)nsl_malloc(sizeof(NBRE_DOM_SPLN));

    if(NULL == result)
    {
        return PAL_ErrNoMem;
    }
    nsl_memset(result, 0, sizeof(NBRE_DOM_SPLN));

    result->chunkInfo = *info;
    err = ReadUint16(readStruct->stream, &result->flag, 1);
    err = (err != PAL_Ok) ? err : ReadUint16(readStruct->stream, &result->count, 1);

    if (result->count == 0)
    {
        err = PAL_ErrNoData;
        goto HandleError;
    }

    if(err != PAL_Ok)
    {
        goto HandleError;
    }

    bufferCount = sizeof(NBRE_DOM_Curves) * result->count;

    result->curves = (NBRE_DOM_Curves*)nsl_malloc(bufferCount);
    if(NULL == result->curves)
    {
        err = PAL_ErrNoMem;
        goto HandleError;
    }
    nsl_memset(result->curves, 0, bufferCount);

    for(i=0; i<result->count; ++i)
    {
        NBRE_DOM_Curves* cvs = &result->curves[i];

        err = ReadUint16(readStruct->stream, &cvs->material, 1);
        err = (err != PAL_Ok) ? err : ReadFloat16(readStruct->stream, &cvs->width, 1);
        err = (err != PAL_Ok) ? err : ReadUint16(readStruct->stream, &cvs->label, 1);
        err = (err != PAL_Ok) ? err : ReadUint16(readStruct->stream, &cvs->count, 1);

        if(err != PAL_Ok)
        {
            goto HandleError;
        }

        if (cvs->count > 0)
        {
            bufferCount = sizeof(NBRE_DOM_ShapeList) * cvs->count;
            cvs->shapeList = (NBRE_DOM_ShapeList*)nsl_malloc(bufferCount);
            if(NULL == cvs->shapeList)
            {
                err = PAL_ErrNoMem;
                goto HandleError;
            }
            nsl_memset(cvs->shapeList, 0, bufferCount);
            err = ReadFloat16(readStruct->stream, (float*)cvs->shapeList, 9*cvs->count);    // @todo: Where does the '9' come from? Please fix!

            if(err != PAL_Ok)
            {
                goto HandleError;
            }
        }
    }

    *chunk = result;

    return PAL_Ok;

HandleError:
    NBRE_DebugLog(PAL_LogSeverityMajor, "ReadSplines: err=0x%08x", err);
    SPLNDestroy(result);
    *chunk = NULL;
    return err;
}

static PAL_Error
ReadPathArrows(DomReadStatus* readStruct, NBRE_DOM_ChunkInfo* info, void** chunk)
{
    PAL_Error err = PAL_Ok;
    NBRE_DOM_TPAR* result = (NBRE_DOM_TPAR*)nsl_malloc(sizeof(NBRE_DOM_TPAR));

    if(NULL == result)
    {
        return PAL_ErrNoMem;
    }
    nsl_memset(result, 0, sizeof(NBRE_DOM_TPAR));

    result->chunkInfo = *info;
    err = ReadUint16(readStruct->stream, &result->flag, 1);
    err = (err != PAL_Ok) ? err : ReadUint16(readStruct->stream, &result->count, 1);

    if (result->count == 0)
    {
        err = PAL_ErrNoData;
        goto HandleError;
    }

    if(PAL_Ok == err)
    {
        uint16 i=0;
        uint32 bufferCount = sizeof(NBRE_DOM_PathArrow) * result->count;

        result->pathArrows = (NBRE_DOM_PathArrow*)nsl_malloc(bufferCount);
        if(NULL == result->pathArrows)
        {
            err = PAL_ErrNoMem;
            goto HandleError;
        }
        nsl_memset(result->pathArrows, 0, bufferCount);

        for(i=0; i<result->count && err==PAL_Ok; ++i)
        {
            NBRE_DOM_PathArrow* pas = &result->pathArrows[i];

            err = ReadUint16(readStruct->stream, &pas->materialIndex, 1);
            err = (err != PAL_Ok) ? err : ReadUint16(readStruct->stream, &pas->lineIndex, 1);
        }
    }

    if(err != PAL_Ok)
    {
        goto HandleError;
    }

    *chunk = result;

    return PAL_Ok;

HandleError:
    NBRE_DebugLog(PAL_LogSeverityMajor, "ReadPathArrows: err=0x%08x", err);
    TPARDestroy(result);
    *chunk = NULL;
    return err;
}

static PAL_Error
ReadGPins(DomReadStatus* readStruct, NBRE_DOM_ChunkInfo* info, void** chunk)
{
    PAL_Error err = PAL_Ok;
    uint32 bufferSize = 0;

    NBRE_DOM_GPIN* result = (NBRE_DOM_GPIN*) nsl_malloc(sizeof(NBRE_DOM_GPIN));
    if (result == NULL)
    {
        err = PAL_ErrNoMem;
        goto HandleError;
    }
    nsl_memset(result, 0, sizeof(NBRE_DOM_GPIN));

    result->chunkInfo = *info;
    err = ReadUint16(readStruct->stream, &result->flag, 1);
    err = (err != PAL_Ok) ? err : ReadUint16(readStruct->stream, &result->count, 1);

    if (err != PAL_Ok)
    {
        goto HandleError;
    }

    if (result->count == 0)
    {
        err = PAL_ErrNoData;
        goto HandleError;
    }

    bufferSize = sizeof(NBRE_DOM_Gpin) * result->count;
    result->gpins = (NBRE_DOM_Gpin*) nsl_malloc(bufferSize);
    if (result->gpins == NULL)
    {
        err = PAL_ErrNoMem;
        goto HandleError;
    }
    nsl_memset(result->gpins, 0, bufferSize);

    for (uint16 i = 0; i < result->count && err == PAL_Ok; ++i)
    {
       NBRE_DOM_Gpin* gpin = &result->gpins[i];

       err = ReadUint16(readStruct->stream, &gpin->materialIndex, 1);

        if (result->flag == 2)
        {
            err = (err != PAL_Ok) ? err : ReadFloat(readStruct->stream, gpin->location, 2);
        }
        else
        {
            err = (err != PAL_Ok) ? err : ReadFloat16(readStruct->stream, gpin->location, 2);
        }

       err = (err != PAL_Ok) ? err : ReadUint16(readStruct->stream, &gpin->pinIdDndex, 1);
       err = (err != PAL_Ok) ? err : ReadUint16(readStruct->stream, &gpin->radius, 1);
    }

    if (PAL_Ok != err)
    {
        goto HandleError;
    }

    *chunk = result;

    return PAL_Ok;

HandleError:
    NBRE_DebugLog(PAL_LogSeverityMajor, "ReadGPins: err=0x%08x", err);
    GPINDestroy(result);
    *chunk = NULL;
    return err;
}

static PAL_Error
ReadEids(DomReadStatus* readStruct, NBRE_DOM_ChunkInfo* info, void** chunk)
{
    PAL_Error err = PAL_Ok;
    uint32 bufferSize = 0;

    NBRE_DOM_EIDS* result = (NBRE_DOM_EIDS*) nsl_malloc(sizeof(NBRE_DOM_EIDS));
    if (result == NULL)
    {
        err = PAL_ErrNoMem;
        goto HandleError;
    }
    nsl_memset(result, 0, sizeof(NBRE_DOM_EIDS));

    result->chunkInfo = *info;
    err = ReadFlagAndCount(readStruct->stream, &result->flag, &result->count);
    if (err != PAL_Ok)
    {
        goto HandleError;
    }

    bufferSize = sizeof(NBRE_DOM_Eids) * result->count;
    result->eids = (NBRE_DOM_Eids*) nsl_malloc(bufferSize);
    if (result->eids == NULL)
    {
        err = PAL_ErrNoMem;
        goto HandleError;
    }
    nsl_memset(result->eids, 0, bufferSize);

    for (uint16 i = 0; i < result->count; ++i)
    {
       NBRE_DOM_Eids* eids = &result->eids[i];

       err = ReadUint8(readStruct->stream, &eids->byteCount, 1);
       if (PAL_Ok != err)
       {
           goto HandleError;
       }

       eids->asciiString = (uint8*)nsl_malloc(eids->byteCount + 1);
       if(eids->asciiString == NULL)
       {
           err = PAL_ErrNoMem;
           goto HandleError;
       }
       nsl_memset(eids->asciiString, 0, eids->byteCount + 1);

       err = ReadUint8(readStruct->stream, eids->asciiString, eids->byteCount);
       if (PAL_Ok != err)
       {
           goto HandleError;
       }
    }

    *chunk = result;

    return PAL_Ok;

HandleError:
    NBRE_DebugLog(PAL_LogSeverityMajor, "ReadEids: err=0x%08x", err);
    EIDSDestroy(result);
    *chunk = NULL;
    return err;
}

static PAL_Error
ReadDrod(DomReadStatus* readStruct, NBRE_DOM_ChunkInfo* info, void** chunk)
{
    PAL_Error err = PAL_Ok;
    uint32 bufferSize = 0;

    NBRE_DOM_DROD* result = (NBRE_DOM_DROD*) nsl_malloc(sizeof(NBRE_DOM_DROD));
    if (result == NULL)
    {
        err = PAL_ErrNoMem;
        goto HandleError;
    }
    nsl_memset(result, 0, sizeof(NBRE_DOM_DROD));

    result->chunkInfo = *info;
    err = ReadFlagAndCount(readStruct->stream, &result->flag, &result->count);
    if (err != PAL_Ok)
    {
        goto HandleError;
    }

    bufferSize = sizeof(NBRE_DOM_Drod) * result->count;
    result->drod = (NBRE_DOM_Drod*) nsl_malloc(bufferSize);
    if (result->drod == NULL)
    {
        err = PAL_ErrNoMem;
        goto HandleError;
    }
    nsl_memset(result->drod, 0, bufferSize);

    for (uint16 i = 0; i < result->count; ++i)
    {
        NBRE_DOM_Drod* drod = &result->drod[i];

        err = ReadUint16(readStruct->stream, &drod->materialIndex, 1);
        if (PAL_Ok != err)
        {
            goto HandleError;
        }

        err = ReadUint16(readStruct->stream, &drod->drawOrder, 1);
        if (PAL_Ok != err)
        {
            goto HandleError;
        }
    }

    *chunk = result;

    return PAL_Ok;

HandleError:
    NBRE_DebugLog(PAL_LogSeverityMajor, "ReadEids: err=0x%08x", err);
    EIDSDestroy(result);
    *chunk = NULL;
    return err;
}

static PAL_Error
DefaultChunkReadFunc(DomReadStatus* readStruct, NBRE_DOM_ChunkInfo* info, void** chunk)
{
    *chunk = NULL;
    return readStruct->stream->Seek(PFSO_Current, info->chunkSize);
}

static void
DefaultDestroy(void* /*chunk*/)
{
    return;
}

static void
PolylineDestroy(NBRE_DOM_Polyline* plin)
{
    if(plin != NULL)
    {
        if(plin->locations != NULL)
        {
            nsl_free(plin->locations);
        }
    }
}

static void
PLINDestroy(void* chunk)
{
    NBRE_DOM_PLIN* domPlin = (NBRE_DOM_PLIN*)chunk;

    if(domPlin != NULL)
    {
        uint16 i;
        for(i=0; i<domPlin->count; i++)
        {
            PolylineDestroy(&domPlin->polylines[i]);
        }
        if(domPlin->polylines != NULL)
        {
            nsl_free(domPlin->polylines);
        }
        nsl_free(domPlin);
    }
}

static void
LPTHDestroy(void* chunk)
{
    NBRE_DOM_LPTH* domRoad = (NBRE_DOM_LPTH*)chunk;

    if(domRoad != NULL)
    {
        if(domRoad->roads != NULL)
        {
            nsl_free(domRoad->roads);
        }
        nsl_free(domRoad);
    }
}

static void
POISDestroy(void* chunk)
{
    NBRE_DOM_POIS* domPois = (NBRE_DOM_POIS*)chunk;

    if(domPois != NULL)
    {
        uint16 i;
        for(i=0; i<domPois->count; i++)
        {
            if(domPois->pois[i].identifier != NULL)
            {
                nsl_free(domPois->pois[i].identifier);
            }
        }
        if(domPois->pois != NULL)
        {
            nsl_free(domPois->pois);
        }
        nsl_free(domPois);
    }
}

static void
TPTHDestroy(void* chunk)
{
    NBRE_DOM_TPTH* domTextPath = (NBRE_DOM_TPTH*)chunk;

    if(domTextPath != NULL)
    {
        if(domTextPath->textPaths != NULL)
        {
            nsl_free(domTextPath->textPaths);
        }
        nsl_free(domTextPath);
    }
}

static void
LabelDestroy(NBRE_DOM_Label* labels)
{
    if(labels != NULL)
    {
        if(labels->string != NULL)
        {
            nsl_free(labels->string);
        }
    }
}

static void
TEXTDestroy(void* chunk)
{
    NBRE_DOM_TEXT* domText = (NBRE_DOM_TEXT*)chunk;

    if(domText != NULL)
    {
        uint16 i;
        for(i=0; i<domText->count; i++)
        {
            LabelDestroy(&domText->labels[i]);
        }
        if(domText->labels != NULL)
        {
            nsl_free(domText->labels);
        }
        nsl_free(domText);
    }
}

static void
AREADestroy(void* chunk)
{
    NBRE_DOM_AREA* domArea = (NBRE_DOM_AREA*) chunk;

    if(domArea != NULL)
    {
        if(domArea->areas != NULL)
        {
            nsl_free(domArea->areas);
        }
        nsl_free(domArea);
    }
}

static void
TPSHDestroy(void* chunk)
{
    NBRE_DOM_TPSH* domPathShields = (NBRE_DOM_TPSH*)chunk;

    if(domPathShields != NULL)
    {
        if(domPathShields->pathShields != NULL)
        {
            nsl_free(domPathShields->pathShields);
        }
        nsl_free(domPathShields);
    }
}

static void
NBMFDestroy(void* chunk)
{
    NBRE_DOM_NBMF* nbmf = (NBRE_DOM_NBMF*) chunk;

    if(NULL != nbmf)
    {
        nsl_free(nbmf);
    }
}

static void
LAYSDestroy(void* chunk)
{
    NBRE_DOM_LAYS* domLayers = (NBRE_DOM_LAYS*) chunk;

    if(domLayers != NULL)
    {
        if(domLayers->layers != NULL)
        {
            nsl_free(domLayers->layers);
        }
        nsl_free(domLayers);
    }
}

static void
ImagesDestroy(NBRE_DOM_Image* imags)
{
    if(imags != NULL)
    {
        if(imags->textureName != NULL)
        {
            nsl_free(imags->textureName);
        }
    }
}

static void
IMGSDestroy(void* chunk)
{
    NBRE_DOM_IMGS* domImags = (NBRE_DOM_IMGS*) chunk;

    if(domImags != NULL)
    {
        uint16 i;
        for(i=0; i<domImags->count; i++)
        {
            ImagesDestroy(&domImags->images[i]);
        }
        if(domImags->images != NULL)
        {
            nsl_free(domImags->images);
        }
        nsl_free(domImags);
    }
}

static void
BitmapDestroy(NBRE_DOM_Bitmap* bitmaps)
{
    if(bitmaps != NULL)
    {
        if(bitmaps->name != NULL)
        {
            nsl_free(bitmaps->name);
        }
        if(bitmaps->imageData != NULL)
        {
            nsl_free(bitmaps->imageData);
        }
    }
}

static void
BMPADestroy(void* chunk)
{
    NBRE_DOM_BMAP* domBitmaps = (NBRE_DOM_BMAP*) chunk;

    if(domBitmaps != NULL)
    {
        uint16 i;
        for(i=0; i<domBitmaps->count; i++)
        {
            BitmapDestroy(&domBitmaps->bitmaps[i]);
        }
        if(domBitmaps->bitmaps != NULL)
        {
            nsl_free(domBitmaps->bitmaps);
        }
        nsl_free(domBitmaps);
    }
}

static void
MaterialsDestroy(NBRE_DOM_Material* material)
{
    if(material != NULL)
    {
        if(material->stm != NULL)
        {
            nsl_free(material->stm);
        }
        if(material->scm != NULL)
        {
            nsl_free(material->scm);
        }
        if(material->ocm != NULL)
        {
            nsl_free(material->ocm);
        }
        if(material->lm != NULL)
        {
            if(material->lm->linghting != NULL)
            {
                nsl_free(material->lm->linghting);
            }
            nsl_free(material->lm);
        }
        if(material->fm != NULL)
        {
            nsl_free(material->fm);
        }
        if(material->lpm != NULL)
        {
            nsl_free(material->lpm);
        }
        if(material->bgm != NULL)
        {
            nsl_free(material->bgm);
        }
        if(material->sm != NULL)
        {
            nsl_free(material->sm);
        }
        if(material->dom != NULL)
        {
            nsl_free(material->dom);
        }
        if(material->pam != NULL)
        {
            nsl_free(material->pam);
        }
        if(material->hlsm != NULL)
        {
            nsl_free(material->hlsm);
        }
        if(material->pinm != NULL)
        {
            nsl_free(material->pinm);
        }
        if(material->ocfm != NULL)
        {
            nsl_free(material->ocfm);
        }
        if(material->osfm != NULL)
        {
            nsl_free(material->osfm);
        }
        if (material->sym != NULL)
        {
            nsl_free(material->sym);
        }
        if (material->rpm != NULL)
        {
            nsl_free(material->rpm);
        }
        if (material->spm != NULL)
        {
            nsl_free(material->spm);
        }
        if (material->ohm != NULL)
        {
            nsl_free(material->ohm);
        }
        NBRE_DELETE material->bmpm;
        NBRE_DELETE material->pcm;
    }
}

static void
MTRLDestroy(void* chunk)
{
    NBRE_DOM_MTRL* domMaterials = (NBRE_DOM_MTRL*) chunk;

    if(domMaterials != NULL)
    {
        uint16 i;
        for(i=0; i<domMaterials->count; i++)
        {
            MaterialsDestroy(&domMaterials->materials[i]);
        }
        if(NULL != domMaterials->materials)
        {
            nsl_free(domMaterials->materials);
        }
        nsl_free(domMaterials);
    }
}

static void
GeometryDestroy(NBRE_DOM_Geometry* geomery)
{
    if(geomery != NULL)
    {
        if(geomery->position != NULL)
        {
            nsl_free(geomery->position);
        }

        if(geomery->normal != NULL)
        {
            nsl_free(geomery->normal);
        }

        if(geomery->texCoord != NULL)
        {
            nsl_free(geomery->texCoord);
        }
    }
}

static void
GEOMDestroy(void* chunk)
{
    NBRE_DOM_GEOM* domGeomery = (NBRE_DOM_GEOM*) chunk;

    if(domGeomery != NULL)
    {
        uint16 i;
        for(i=0; i<domGeomery->count; i++)
        {
            GeometryDestroy(&domGeomery->geometries[i]);
        }
        if(domGeomery->geometries != NULL)
        {
            nsl_free(domGeomery->geometries);
        }
        nsl_free(domGeomery);
    }
}

static void
MeshDestroy(NBRE_DOM_Mesh* mesh)
{
    if(mesh != NULL)
    {
        if(mesh->indices != NULL)
        {
            nsl_free(mesh->indices);
        }
    }
}

static void
MESHDestroy(void* chunk)
{
    NBRE_DOM_MESH* domMesh = (NBRE_DOM_MESH*) chunk;

    if(domMesh != NULL)
    {
        uint16 i;
        for(i=0; i<domMesh->count; i++)
        {
            MeshDestroy(&domMesh->meshes[i]);
        }
        if(domMesh->meshes != NULL)
        {
            nsl_free(domMesh->meshes);
        }
        nsl_free(domMesh);
    }
}

static void
CurvesDestroy(NBRE_DOM_Curves* curves)
{
    if(curves != NULL)
    {
        if(curves->shapeList != NULL)
        {
            nsl_free(curves->shapeList);
        }

        // Important: Don't destroy curves, all curves got allocated as a chunck and get freed in SPLNDestroy()!
    }
}

static void
SPLNDestroy(void* chunk)
{
    NBRE_DOM_SPLN* splines = (NBRE_DOM_SPLN*) chunk;

    if(splines != NULL)
    {
        uint16 i;
        for(i=0; i<splines->count; ++i)
        {
            CurvesDestroy(&splines->curves[i]);
        }
        if(splines->curves != NULL)
        {
            nsl_free(splines->curves);
        }
        nsl_free(splines);
    }
}

static void
PNTSDestroy(void* chunk)
{
    NBRE_DOM_PNTS* domPoints = (NBRE_DOM_PNTS*) chunk;

    if(domPoints != NULL)
    {
        if(domPoints->points != NULL)
        {
            nsl_free(domPoints->points);
        }
        nsl_free(domPoints);
    }
}

static void
ExternalObjectDestroy(NBRE_DOM_ExternalObject* externalObjects)
{
    if(externalObjects != NULL)
    {
        if(externalObjects->fileName != NULL)
        {
            nsl_free(externalObjects->fileName);
        }
        if(externalObjects->index != NULL)
        {
            nsl_free(externalObjects->index);
        }
    }
}

static void
OBJXDestroy(void* chunk)
{
    NBRE_DOM_OBJX* domExternalObjects = (NBRE_DOM_OBJX*) chunk;

    if(domExternalObjects != NULL)
    {
        uint16 i;
        for(i=0; i<domExternalObjects->count; ++i)
        {
            NBRE_DOM_ExternalObject* objx = &domExternalObjects->externalObjs[i];
            ExternalObjectDestroy(objx);
        }
        if(domExternalObjects->externalObjs != NULL)
        {
            nsl_free(domExternalObjects->externalObjs);
        }
        nsl_free(domExternalObjects);
    }
}

static void
OBJSDestroy(void* chunk)
{
    NBRE_DOM_OBJS* domObjects = (NBRE_DOM_OBJS*) chunk;

    if(domObjects != NULL)
    {
        if(domObjects->objects != NULL)
        {
            nsl_free(domObjects->objects);
        }
        nsl_free(domObjects);
    }
}

static void
SceneDestroy(NBRE_DOM_Scene* scenes)
{
    if(scenes != NULL)
    {
        if(scenes->objIndex != NULL)
        {
            nsl_free(scenes->objIndex);
        }
    }
}

static void
SEENDestroy(void* chunk)
{
    NBRE_DOM_SEEN* domScenes = (NBRE_DOM_SEEN*) chunk;

    if(domScenes != NULL)
    {
        uint16 i;
        for(i=0; i<domScenes->count; i++)
        {
            SceneDestroy(&domScenes->scenes[i]);
        }
        if(domScenes->scenes != NULL)
        {
            nsl_free(domScenes->scenes);
        }
        nsl_free(domScenes);
    }
}

static void
TPARDestroy(void* chunk)
{
    NBRE_DOM_TPAR* domPathsArrows = (NBRE_DOM_TPAR*) chunk;

    if(domPathsArrows != NULL)
    {
        if(domPathsArrows->pathArrows != NULL)
        {
            nsl_free(domPathsArrows->pathArrows );
        }
        nsl_free(domPathsArrows);
    }
}

static void
GPINDestroy(void* chunk)
{
    NBRE_DOM_GPIN* domGPin = (NBRE_DOM_GPIN*) chunk;

    if(domGPin != NULL)
    {
        if(domGPin->gpins != NULL)
        {
            nsl_free(domGPin->gpins );
        }
        nsl_free(domGPin);
    }
}

static void
EidsDestroy(NBRE_DOM_Eids* eids)
{
    if(eids)
    {
        nsl_free(eids->asciiString);
    }
}

static void
EIDSDestroy(void* chunk)
{
    NBRE_DOM_EIDS* domEids = (NBRE_DOM_EIDS*) chunk;

    if(domEids != NULL)
    {
        if(domEids->eids != NULL)
        {
            for(uint16 i = 0; i < domEids->count; ++i)
            {
                NBRE_DOM_Eids* eids = &domEids->eids[i];
                EidsDestroy(eids);
            }
            nsl_free(domEids->eids);
        }
        nsl_free(domEids);
    }
}

static void
DRODDestroy(void* chunk)
{
    NBRE_DOM_DROD* domDrod = (NBRE_DOM_DROD*) chunk;

    if(domDrod != NULL)
    {
        if(domDrod->drod != NULL)
        {
            nsl_free(domDrod->drod);
        }
        nsl_free(domDrod);
    }
}

static void
LPDRDestroy(void* chunk)
{
    NBRE_DOM_LPDR* domLpdr = (NBRE_DOM_LPDR*) chunk;

    if(domLpdr != NULL)
    {
        if(domLpdr->roads != NULL)
        {
            nsl_free(domLpdr->roads);
        }
        nsl_free(domLpdr);
    }
}

static PAL_Error
ReadUint32(NBRE_IOStream* stream, uint32* buffer, uint32 count)
{
    uint32 readBytes = 0;
    uint32 size = count * sizeof(uint32);

    PAL_Error err = stream->Read((uint8*)buffer, size, &readBytes);

    if(err != PAL_Ok || readBytes != size)
    {
        return PAL_ErrFileFailed;
    }
    return PAL_Ok;
}

static PAL_Error
ReadUint24(NBRE_IOStream* stream, uint32* buffer, uint32 count)
{
    uint32 readBytes = 0;
    uint32 size = sizeof(uint32) * 3 / 4 * count;

    PAL_Error err = stream->Read((uint8*)buffer, size, &readBytes);

    if(err != PAL_Ok || readBytes != size)
    {
        return PAL_ErrFileFailed;
    }
    return PAL_Ok;
}

static PAL_Error
ReadUint16(NBRE_IOStream* stream, uint16* buffer, uint32 count)
{
    uint32 readBytes = 0;
    uint32 size = count * sizeof(uint16);

    PAL_Error err = stream->Read((uint8*)buffer, size, &readBytes);

    if(err != PAL_Ok || readBytes != size)
    {
        return PAL_ErrFileFailed;
    }
    return PAL_Ok;
}

static PAL_Error
ReadUint8(NBRE_IOStream* stream, uint8* buffer, uint32 count)
{
    uint32 readBytes = 0;
    uint32 size = count * sizeof(uint8);

    PAL_Error err = stream->Read((uint8*)buffer, size, &readBytes);

    if(err != PAL_Ok || readBytes != size)
    {
        return PAL_ErrFileFailed;
    }
    return PAL_Ok;
}

static PAL_Error
ReadInt8(NBRE_IOStream* stream, int8* buffer, uint32 count)
{
    uint32 readBytes = 0;
    uint32 size = count * sizeof(int8);

    PAL_Error err = stream->Read((uint8*)buffer, size, &readBytes);

    if(err != PAL_Ok || readBytes != size)
    {
        return PAL_ErrFileFailed;
    }
    return PAL_Ok;
}

static PAL_Error
ReadChar(NBRE_IOStream* stream, char* buffer, uint32 count)
{
    uint32 readBytes = 0;
    uint32 size = count*sizeof(char);

    PAL_Error err = stream->Read((uint8*)buffer, size, &readBytes);

    if(err != PAL_Ok || readBytes != size)
    {
        return PAL_ErrFileFailed;
    }
    return PAL_Ok;
}

static PAL_Error
ReadFloat(NBRE_IOStream* stream, float* buffer, uint32 count)
{
    uint32 readBytes = 0;
    uint32 size = count * sizeof(float);

    PAL_Error err = stream->Read((uint8*)buffer, size, &readBytes);

    if(err != PAL_Ok || readBytes != size)
    {
        return PAL_ErrFileFailed;
    }
    return PAL_Ok;
}

static PAL_Error
ReadDouble(NBRE_IOStream* stream, double* buffer, uint32 count)
{
    uint32 readBytes = 0;
    uint32 size = count * sizeof(double);

    PAL_Error err = stream->Read((uint8*)buffer, size, &readBytes);

    if(err != PAL_Ok || readBytes != size)
    {
        return PAL_ErrFileFailed;
    }
    return PAL_Ok;
}

static const double HALF_PI = 3.14159265358979323846 / 2;
static const int HALF_ABS_SHORT = 32767 / 2;
static const int MAX_ABS_SHORT = HALF_ABS_SHORT * 2;

int double2int(double value)
{
    int intVal = (int)value;
    if (value > intVal)
    {
        if ((value - intVal) > (intVal + 1 - value))
            return intVal + 1;
        else
            return intVal;
    }
    else if (value < intVal)
    {
        if ((intVal - value) > (value - (intVal - 1)))
            return intVal - 1;
        else
            return intVal;
    }
    return intVal;
}

static int short2int(short value)
{
    if (abs(value) > HALF_ABS_SHORT)
    {
        double angle = (double)tan(value * HALF_PI / MAX_ABS_SHORT);
        return (int)double2int(angle * HALF_ABS_SHORT);
    }
    else
    {
        return (int)value;
    }
}

float half22float(unsigned short value, float AbsThreshold)
{
    return short2int(value - HALF_ABS_SHORT) * AbsThreshold / HALF_ABS_SHORT;
}

static void ConvertFloat2(unsigned short offsetx, unsigned short offsety, double& x, double& y, double tileSizeX, double tileSizeY)
{
    static const int PRECISION_FACTOR = 10000;
    int offsetx_int = short2int(offsetx - 32767);
    int offsety_int = short2int(offsety - 32767);

    x = (double)(offsetx_int * tileSizeX / PRECISION_FACTOR);
    y = (double)(offsety_int * tileSizeY / PRECISION_FACTOR);
}

PAL_Error
ReadFloat16v2(NBRE_IOStream* stream, float* buffer, uint32 count, double tileSizeX, double tileSizeY)
{
    uint32 readBytes = 0;
    uint32 size = count * sizeof(uint16);

    PAL_Error err = stream->Read((uint8*)buffer, size, &readBytes);

    if(err != PAL_Ok || readBytes != size)
    {
        return PAL_ErrFileFailed;
    }

    uint16* fb = (uint16*)buffer;
    for(uint32 i = 0; i < count; i+=2)
    {
        double x = 0;
        double y = 0;
        ConvertFloat2(fb[count-i-2], fb[count-i-1], x, y, tileSizeX, tileSizeY);
        buffer[count-i-2] = (float)x;
        buffer[count-i-1] = (float)y;
    }
    return PAL_Ok;
}

PAL_Error
ReadFloat16v2(NBRE_IOStream* stream, float* buffer, uint32 count, float absRange)
{
    uint32 readBytes = 0;
    uint32 size = count * sizeof(uint16);

    PAL_Error err = stream->Read((uint8*)buffer, size, &readBytes);

    if(err != PAL_Ok || readBytes != size)
    {
        return PAL_ErrFileFailed;
    }

    uint16* fb = (uint16*)buffer;
    for(uint32 i = 0; i < count; ++i)
    {
        buffer[count-i-1] = half22float(fb[count - i - 1], absRange);
    }

    return PAL_Ok;
}

PAL_Error
ReadFloat16v2(NBRE_IOStream* stream, double* buffer, uint32 count, double tileSizeX, double tileSizeY)
{
    uint32 readBytes = 0;
    uint32 size = count * sizeof(uint16);

    PAL_Error err = stream->Read((uint8*)buffer, size, &readBytes);

    if(err != PAL_Ok || readBytes != size)
    {
        return PAL_ErrFileFailed;
    }

    uint16* fb = (uint16*)buffer;
    for(uint32 i = 0; i < count; i+=2)
    {
        ConvertFloat2(fb[count-i-2], fb[count-i-1], buffer[count-i-2], buffer[count-i-1], tileSizeX, tileSizeY);
    }
    return PAL_Ok;
}

static PAL_Error
ReadFloat16(NBRE_IOStream* stream, float* buffer, uint32 count)
{
    uint32 readBytes = 0;
    uint32 i = 0;
    uint32 size = count * sizeof(float)/2;

    PAL_Error err = stream->Read((uint8*)buffer, size, &readBytes);

    if(err != PAL_Ok || readBytes != size)
    {
        return PAL_ErrFileFailed;
    }

    for(i=0; i<count; ++i)
    {
        uint16* fb = (uint16*)buffer;
        buffer[count-i-1] = ConvertFloat16ToFloat32(fb[count-i-1]);
    }

    return PAL_Ok;
}

typedef union UnionFloatUint32
{
    float f;
    uint32 u;
}UnionFloatUint32;

static float ConvertFloat16ToFloat32(uint16 src)
{
    uint16 hs, he, hm;
    uint32 xs, xe, xm;
    uint32 xes;
    int e;

    UnionFloatUint32 ress = {0};

    if( (src & 0x7FFFu) == 0 )  // Signed zero
    {
        ress.u = ((uint32) src) << 16;  // Return the signed zero
    }
    else// Not zero
    {
        hs = src & 0x8000u;  // Pick off sign bit
        he = src & 0x7C00u;  // Pick off exponent bits
        hm = src & 0x03FFu;  // Pick off mantissa bits
        if( he == 0 )  // Denormal will convert to normalized
        {
            e = -1; // The following loop figures out how much extra to adjust the exponent
            do
            {
                e++;
                hm <<= 1;
            } while( (hm & 0x0400u) == 0 ); // Shift until leading bit overflows into exponent bit
            xs = ((uint32) hs) << 16; // Sign bit
            xes = ((int) (he >> 10)) - 15 + 127 - e; // Exponent unbias the halfp, then bias the single
            xe = (uint32) (xes << 23); // Exponent
            xm = ((uint32) (hm & 0x03FFu)) << 13; // Mantissa
            ress.u = (xs | xe | xm); // Combine sign bit, exponent bits, and mantissa bits
        }
        else if( he == 0x7C00u )  // Inf or NaN (all the exponent bits are set)
        {
            if( hm == 0 ) // If mantissa is zero ...
            {
                ress.u = (((uint32) hs) << 16) | ((uint32) 0x7F800000u); // Signed Inf
            }
            else
            {
                ress.u = (uint32) 0xFFC00000u; // NaN, only 1st mantissa bit set
            }
        }
        else // Normalized number
        {
            xs = ((uint32) hs) << 16; // Sign bit
            xes = ((int) (he >> 10)) - 15 + 127; // Exponent unbias the halfp, then bias the single
            xe = (uint32) (xes << 23); // Exponent
            xm = ((uint32) hm) << 13; // Mantissa
            ress.u = (xs | xe | xm); // Combine sign bit, exponent bits, and mantissa bits
        }
    }
    return ress.f;
}

const char* NBRE_DOMGetName( NBRE_DOM* dom )
{
    if(dom!=NULL)
    {
        return dom->domName;
    }
    return NULL;
}

