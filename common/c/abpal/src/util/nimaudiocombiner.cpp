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

@file     NimAudioCombiner.c
@date     03/02/2009
@defgroup PAL Audio Combiner API

@brief    Audio Combiner for Windows Mobile

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


/*! @( */

// TODO: Remove all magic numbers from this module

#include "nimaudiocombiner.h"
#include <string.h>
#include "palstdlib.h"
#include "paltestlog.h"


#define INCLUDE_GETTIME_FUNCTION    0   // Set to 1 to include the GetTime function
#define INCLUDE_UNUSED_FUNCTIONS    0   // Set to 1 to include all unused functions

static PAL_Error    CombineAddAAC(AudioCombiner* pCombiner, byte* pdata, uint32 nsize, nb_boolean copyNeeded);
static PAL_Error    CombineAddAmr(AudioCombiner* pCombiner, byte* pdata, uint32 nsize);
static PAL_Error    CombineAddQcp(AudioCombiner* pCombiner, byte* pdata, uint32 nsize);

static PAL_Error    GetDataQcp(AudioCombiner* pCombiner, byte** ppdata, int* pnsize);
static PAL_Error    GetDataAmr(AudioCombiner* pCombiner, byte** ppdata, int* pnsize);
static PAL_Error    GetDataCmx(AudioCombiner* pCombiner, byte** ppdata, int* pnsize);
static PAL_Error    GetDataAac(AudioCombiner* pCombiner, byte** ppdata, int* pnsize);
static size_t       AppendAudioQcp(AudioCombiner* pCombiner, byte* pdata, size_t event, size_t packets, nb_boolean last);

#if INCLUDE_GETTIME_FUNCTION
static PAL_Error    GetTime(AudioCombiner* pCombiner, double* time);
#endif

static PAL_Error    FindRiffChunk(const char* type, byte* pdata,
                                  uint32 size, byte** ppchunkdata, uint32* pchunksize);
static uint32       GetUint32(byte* pdata);

#if INCLUDE_UNUSED_FUNCTIONS
static uint16       GetUint16(byte* pdata);
#endif

static void         SetUint32(byte* pdata, uint32 val);
static void         SetUint16Be(byte* pdata, uint16 val);
static void         SetUint32Be(byte* pdata, uint32 val);

ABPAL_DEF PAL_Error
NIMAudioCombiner_Create(ABPAL_AudioFormat format, AudioCombiner** combiner)
{
    size_t bufSize = 8192;

    AudioCombiner* pCombiner = NULL;

    *combiner = NULL;

    pCombiner = (AudioCombiner *)nsl_malloc(sizeof(AudioCombiner));
    if (!pCombiner)
    {
        return PAL_ErrNoMem;
    }

    nsl_memset(pCombiner, 0, sizeof(AudioCombiner));

    pCombiner->format = format;

    // Create the AudioBuffer
    AudioBufferNew(&pCombiner->data, bufSize);

    *combiner = pCombiner;

    return PAL_Ok;
}

ABPAL_DEF PAL_Error
NIMAudioCombiner_Destroy(AudioCombiner* pCombiner)
{
    //int i = 0;

    if(!pCombiner)
    {
        return PAL_ErrBadParam;
    }

    if (pCombiner->fmtData != NULL)
    {
        nsl_free(pCombiner->fmtData);
    }

    if (pCombiner->vratData != NULL)
    {
        nsl_free(pCombiner->vratData);
    }
    AudioBufferDelete(&pCombiner->data);

    if (pCombiner->aacCombiner)
    {
        delete pCombiner->aacCombiner;
        pCombiner->aacCombiner = NULL;
    }

    nsl_free(pCombiner);

    return PAL_Ok;
}

ABPAL_DEF PAL_Error
NIMAudioCombiner_GetData(AudioCombiner* pCombiner, byte** ppdata, int* pnsize)
{
    if(!pCombiner)
    {
        return PAL_ErrBadParam;
    }

    *ppdata = NULL;
    *pnsize = 0;

    switch(pCombiner->format)
    {
    case ABPAL_AudioFormat_QCP:
        return GetDataQcp(pCombiner, ppdata, pnsize);
    case ABPAL_AudioFormat_CMX:
        return GetDataCmx(pCombiner, ppdata, pnsize);
    case ABPAL_AudioFormat_AMR:
        return GetDataAmr(pCombiner, ppdata, pnsize);
    case ABPAL_AudioFormat_AAC:
        return GetDataAac(pCombiner, ppdata, pnsize);
    default:
        NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelWarning, "Audio format %d is unsupported", pCombiner->format );
        break;
    }
    return PAL_ErrUnsupported;
}

ABPAL_DEF PAL_Error
NIMAudioCombiner_AddData(AudioCombiner* pCombiner, byte* pdata, uint32 nsize, nb_boolean copyNeeded)
{
    PAL_Error result = PAL_Ok;
    if(pCombiner == NULL)
    {
        result = PAL_ErrBadParam;
    }
    else
    {
        switch(pCombiner->format)
        {
            case ABPAL_AudioFormat_QCP:
            {
                result = CombineAddQcp(pCombiner, pdata, nsize);
                if (!copyNeeded)
                {
                    nsl_free(pdata);
                }
                break;
            }
            case ABPAL_AudioFormat_AMR:
            {
                result = CombineAddAmr(pCombiner, pdata, nsize);
                if (!copyNeeded)
                {
                    nsl_free(pdata);
                }
                break;
            }
            case ABPAL_AudioFormat_AAC:
            {
                result = CombineAddAAC(pCombiner, pdata, nsize, copyNeeded);
                break;
            }
            default:
            {
                result = PAL_ErrBadParam;
                break;
            }
        }
    }
    return result;
}

PAL_Error
CombineAddAAC(AudioCombiner* pCombiner, byte* pdata, uint32 nsize, nb_boolean copyNeeded)
{
    if (pCombiner == NULL)
    {
        return PAL_ErrBadParam;
    }

    if (!pCombiner->aacCombiner)
    {
        pCombiner->aacCombiner = new pal::audio::AACCombainer();
    }
    if (!pCombiner->aacCombiner)
    {
        return PAL_ErrNoMem;
    }



    if (!pCombiner->aacCombiner->Combine(pdata, nsize))
    {
        return PAL_ErrAudioGeneral;
    }
    if (!copyNeeded)
    {
        nsl_free(pdata);
    }

    return PAL_Ok;
}

PAL_Error
CombineAddAmr(AudioCombiner* pCombiner, byte* pdata, uint32 nsize)
{
    PAL_Error err = PAL_Ok;

    byte* pdata_data = NULL;
    uint32    data_size = 0;
    uint32  header_size = 0;

    header_size = nsl_strlen((const char* )AMR_header);
    if (nsize < header_size)
    {
        return PAL_ErrAudioBadData; //data is smaller than the header size
    }

    if (nsl_memcmp(pdata, AMR_header, header_size) != 0) //See if it has amr header
    {
        return PAL_ErrAudioBadData; // Not amr data
    }

    data_size = nsize - header_size;
    pdata_data = pdata + header_size; // use offset to get to the sound data

    AudioBufferCat(&pCombiner->data, pdata_data, data_size);

    if (AudioBufferError(&pCombiner->data) == 0)
    {
        err = PAL_Ok;
    }

    return err;
}

PAL_Error
CombineAddQcp(AudioCombiner* pCombiner, byte* pdata, uint32 nsize)
{
    PAL_Error err = PAL_Ok;

    byte*     fmt_data = NULL;
    uint32    fmt_size = 0;

    byte*     vrat_data = NULL;
    uint32    vrat_size = 0;

    byte*     data_data = NULL;
    uint32    data_size = 0;

    err = FindRiffChunk("fmt ", pdata, nsize, &fmt_data, &fmt_size);

    if (err != PAL_Ok)
    {
        goto errexit;
    }

    err = FindRiffChunk("vrat", pdata, nsize, &vrat_data, &vrat_size);

    if (err != PAL_Ok)
    {
        goto errexit;
    }

    err = FindRiffChunk("data", pdata, nsize, &data_data, &data_size);

    if (err != PAL_Ok)
    {
        goto errexit;
    }

    if (fmt_data != NULL && data_data != NULL)
    {
        if (pCombiner->fmtData != NULL && pCombiner->vratData != NULL)
        {
            if (pCombiner->fmtSize != fmt_size || nsl_memcmp(pCombiner->fmtData, fmt_data, fmt_size) != 0)
            {
                err = PAL_ErrAudioBadData;
                goto errexit;
            }

            SetUint32(pCombiner->vratData+4, GetUint32(pCombiner->vratData+4) + GetUint32(vrat_data+4));
        }
        else
        {
            if (pCombiner->fmtData != NULL || pCombiner->vratData != NULL)
            {
                err = PAL_ErrAudioBadData;
                goto errexit;
            }

            pCombiner->fmtData = fmt_data;
            pCombiner->fmtSize = fmt_size;
            pCombiner->vratData = vrat_data;
            pCombiner->vratSize = vrat_size;

            fmt_data = NULL;
            fmt_size = 0;
            vrat_data = NULL;
            vrat_size = 0;
        }

        AudioBufferCat(&pCombiner->data, data_data, data_size);

        if (AudioBufferError(&pCombiner->data) == 0)
        {
            err = PAL_Ok;
        }
    }
    else
    {
        err = PAL_ErrAudioBadData;
    }

errexit:

    if (data_data != NULL)
    {
        nsl_free(data_data);
    }

    if (fmt_data != NULL)
    {
        nsl_free(fmt_data);
    }

    if (vrat_data != NULL)
    {
        nsl_free(vrat_data);
    }

    return err;
}

PAL_Error
GetDataAac(AudioCombiner* pCombiner, byte** ppdata, int* pnsize)
{
    if (!pCombiner || !ppdata || !pnsize)
    {
        return PAL_ErrBadParam;
    }
    *pnsize = 0;
    *ppdata = NULL;

    if (!pCombiner->aacCombiner)
    {
        return PAL_ErrNoInit;
    }

    uint32 size = pCombiner->aacCombiner->Data().str().length();
    if (size > 0)
    {
        *ppdata = static_cast<byte*>(nsl_malloc(size));
        if (!*ppdata)
        {
            return PAL_ErrNoMem;
        }

        nsl_memcpy(*ppdata, pCombiner->aacCombiner->Data().str().c_str(), size);
        *pnsize = size;
        return PAL_Ok;
    }
    return PAL_ErrAudioGeneral;
}

PAL_Error
GetDataQcp(AudioCombiner* pCombiner, byte** ppdata, int* pnsize)
{
    uint32 nsize;
    byte* ppos;
    *pnsize = 0;
    if (AudioBufferLen(&pCombiner->data) == 0)
    {
        return PAL_ErrNoInit;
    }

    nsize = /* RIFF Header */   12 +
            /* FMT Header */    8 +
            /* FMT Data */      pCombiner->fmtSize +
            /* VRAT Header */   8 +
            /* VRAT Data */     pCombiner->vratSize +
            /* DATA Header */   8 +
            /* DATA */          AudioBufferLen(&pCombiner->data) +
            /* Padding */       AudioBufferLen(&pCombiner->data) % 2;

    *ppdata = (byte* )nsl_malloc(nsize);

    if (*ppdata == NULL)
    {
        return PAL_ErrNoMem;
    }

    ppos = *ppdata;

    /* RIFF Header */
    nsl_memcpy(ppos, "RIFF", 4);                                ppos += 4;
    SetUint32(ppos, nsize - 8);                                 ppos += 4;
    nsl_memcpy(ppos, "QLCM", 4);                                ppos += 4;

    /* FMT Header */
    nsl_memcpy(ppos, "fmt ", 4);                                ppos += 4;
    SetUint32(ppos, pCombiner->fmtSize);                        ppos += 4;

    /* FMT Data */
    nsl_memcpy(ppos, pCombiner->fmtData, pCombiner->fmtSize);   ppos += pCombiner->fmtSize;

    /* VRAT Header */
    nsl_memcpy(ppos, "vrat", 4);                                ppos += 4;
    SetUint32(ppos, pCombiner->vratSize);                       ppos += 4;

    /* VRAT Data */
    nsl_memcpy(ppos, pCombiner->vratData, pCombiner->vratSize); ppos += pCombiner->vratSize;

    /* DATA Header */
    nsl_memcpy(ppos, "data", 4);                                ppos += 4;
    SetUint32(ppos, AudioBufferLen(&pCombiner->data));          ppos += 4;

    /* DATA */
    nsl_memcpy(ppos, AudioBufferGet(&pCombiner->data), AudioBufferLen(&pCombiner->data));
    ppos += AudioBufferLen(&pCombiner->data);

    /* Padding */
    if (AudioBufferLen(&pCombiner->data) % 2)
    {
        *ppos = 0;
    }

    *pnsize = nsize;

    return PAL_Ok;
}

size_t
AppendAudioQcp(AudioCombiner* pCombiner, byte* pdata, size_t event, size_t packets, nb_boolean last)
{
    byte* ppos = pdata;
    const byte* data = AudioBufferGet(&pCombiner->data);

    *ppos = (event == 0) ? 0 : 50;                   ppos += 1;
    *ppos = 0xFF;                                    ppos += 1;
    *ppos = 0xF1;                                    ppos += 1;
    SetUint16Be(ppos, (uint16) (7 + (packets*35)));  ppos += 2;
    *ppos = 0;                                       ppos += 1;
    *ppos = 0x44;                                    ppos += 1;
    SetUint32Be(ppos, last ? 0 : 887);               ppos += 4;
    *ppos = (event == 0) ? 0 : 1;                    ppos += 1;
    nsl_memcpy(ppos, data + event*875, packets*35);

    return 12 + (packets*35);
}

PAL_Error
GetDataAmr(AudioCombiner* pCombiner, byte** ppdata, int* pnsize)
{
    uint32 nsize, headersize;
    byte* ppos;
    size_t datalen = AudioBufferLen(&pCombiner->data);
    if (datalen <= 0)
    {
        return PAL_ErrNoInit;
    }

    headersize = nsl_strlen((const char* )AMR_header);

    //Final size = fmt_size + vrat_size - sizeof(AMR_header) only need one header
    nsize = headersize + datalen;
    *ppdata = (byte* )nsl_malloc(nsize);

    if (*ppdata == NULL)
    {
        return PAL_ErrNoMem;
    }

    ppos = *ppdata;
    nsl_memcpy(ppos, AMR_header, headersize);
    ppos += headersize;
    nsl_memcpy(ppos, AudioBufferGet(&pCombiner->data), datalen);
    *pnsize = nsize;
    return PAL_Ok;
}

PAL_Error
GetDataCmx(AudioCombiner* pCombiner, byte** ppdata, int* pnsize)
{
    uint32 nsize;
    byte* ppos;

    size_t datalen = AudioBufferLen(&pCombiner->data);
    const byte* data = AudioBufferGet(&pCombiner->data);
    size_t numpackets;
    size_t packet;
    size_t event;
    size_t numfullaudioevents;
    size_t partialeventpackets;
    size_t trackchunklength;
    byte dt;
    if (datalen == 0)
    {
        return PAL_ErrNoInit;
    }

    /* Make sure that the data size is a even number of packets */
    if ((datalen % 35) != 0)
    {
        return PAL_ErrAudioBadData;
    }

    numpackets = datalen / 35;

    for (packet = 0; packet < numpackets; packet++)
    {
        if (data[packet*35] != 0x04)
        {
            return PAL_ErrAudioBadData;
        }
    }

    numfullaudioevents        =    numpackets / 25;
    partialeventpackets       =    numpackets % 25;

    trackchunklength          =
        /* Full Audio Events */      887 * numfullaudioevents +
        /* Partial Audio Event */    (partialeventpackets > 0 ? 12 : 0) + partialeventpackets * 35 +
        /* "End" Message */          4;

    nsize = /* CMF File ID */       4 +
            /* CMF File Length */   4 +
            /* CMF Header */        32 +
            /* Trac Chunk Header */ 8 +
            /* Trac Chunk Data */   trackchunklength;

    *ppdata = (byte* )nsl_malloc(nsize);

    if (*ppdata == NULL)
    {
        return PAL_ErrNoMem;
    }

    ppos = *ppdata;

    /* CMF File ID */
    nsl_memcpy(ppos, "cmid", 4);            ppos += 4;

    /* CMF File Length */
    SetUint32Be(ppos, nsize - 8);           ppos += 4;

    /* CMF Header */
    SetUint16Be(ppos, 30);                  ppos += 2;
    *ppos = 2;                              ppos += 1;
    *ppos = 2;                              ppos += 1;
    *ppos = 1;                              ppos += 1;
    nsl_memcpy(ppos, "vers", 4);            ppos += 4;
    SetUint16Be(ppos, 4);                   ppos += 2;
    nsl_memcpy(ppos, "0201", 4);            ppos += 4;
    nsl_memcpy(ppos, "wave", 4);            ppos += 4;
    SetUint16Be(ppos, 1);                   ppos += 2;
    *ppos = 1;                              ppos += 1;
    nsl_memcpy(ppos, "cnts", 4);            ppos += 4;
    SetUint16Be(ppos, 4);                   ppos += 2;
    nsl_memcpy(ppos, "WAVE", 4);            ppos += 4;

    /* Trac Chunk Header */
    nsl_memcpy(ppos, "trac", 4);            ppos += 4;
    SetUint32Be(ppos, trackchunklength);    ppos += 4;

    /* Partial Audio Events */
    for (event = 0; event < numfullaudioevents; event++)
    {
        ppos += AppendAudioQcp(pCombiner, ppos, event, 25, (partialeventpackets == 0 && event+1==numfullaudioevents) ? TRUE : FALSE);
    }

    /* Partial Audio Events */
    if (partialeventpackets > 0)
    {
        ppos += AppendAudioQcp(pCombiner, ppos, event, partialeventpackets, TRUE);
        dt = (byte) ((partialeventpackets * 50)/35);
    }
    else
    {
        dt = 50;
    }

    /* "End" Message */
    *ppos = dt;                             ppos += 1;
    *ppos = 0xFF;                           ppos += 1;
    *ppos = 0xDF;                           ppos += 1;
    *ppos = 0x00;
    //    ppos += 1;
    //    ASSERT((ppos == (*ppdata) + nsize), ("CMX Data size mismatch."));

    *pnsize = nsize;

    return PAL_Ok;
}

#if INCLUDE_GETTIME_FUNCTION
// Time of qcpdata in seconds: size-in-packets*block-size/sampling-rate
static PAL_Error
GetTime(NIMAudioCombiner* pCombiner, double* time)
{
    uint32 size_in_packets;
    uint16 block_size, sampling_rate;
    // FMT chunk-size major minor codec-info
    byte* ppos = pCombiner->fmtData;
    // codec-info
    ppos += (1+1);
    /*
    codec-info      = codec-guid codec-version codec-name
    average-bps packet-size
    block-size sampling-rate
    sample-size variable-rate 5*UINT32
    */
    ppos += (16+2+80+2+2);
    block_size = GetUint16(ppos);
    ppos += 2;
    sampling_rate = GetUint16(ppos);
    // VRAT chunk-size var-rate-flag size-in-packets
    ppos = pCombiner->vratData;
    ppos += 4;
    size_in_packets = GetUint32(ppos);
    *time = (double)(size_in_packets*block_size)/(double)sampling_rate;
    //FDIV((double)(size_in_packets*block_size),(double)sampling_rate);
    return PAL_Ok;
}
#endif  // INCLUDE_GETTIME_FUNCTION

static uint32
GetUint32(byte* pdata)
{
    uint32 retval;

    nsl_memcpy(&retval, pdata, 4);

    return retval;
}

#if INCLUDE_UNUSED_FUNCTIONS
static uint16
GetUint16(byte* pdata)
{
    uint16 retval;

    memcpy(&retval, pdata, 2);

    return retval;
}
#endif

static void
SetUint32(byte* pdata, uint32 val)
{
    nsl_memcpy(pdata, &val, 4);
}

static void
SetUint16Be(byte* pdata, uint16 val)
{
    uint16 val_be = nsl_htons(val);

    nsl_memcpy(pdata, &val_be, 2);
}

static void
SetUint32Be(byte* pdata, uint32 val)
{
    uint32 val_be = nsl_htonl(val);

    nsl_memcpy(pdata, &val_be, 4);
}

static PAL_Error
FindRiffChunk(const char* type, byte* pdata, uint32 size, byte** ppchunkdata, uint32* pchunksize)
{
    byte* ppos;

    if (pdata == NULL)
    {
        return PAL_ErrBadParam;
    }

    for (ppos = pdata; ppos < (pdata + size - 8); ppos += 2)
    {
        if (nsl_memcmp(ppos, type, 4) == 0)
        {
            *pchunksize = GetUint32(ppos+4);

            *ppchunkdata = (byte* )nsl_malloc(*pchunksize);

            if (*ppchunkdata == NULL)
            {
                return PAL_ErrNoMem;
            }

            if (size-((ppos+8)-pdata) >= *pchunksize)
            {
                nsl_memcpy(*ppchunkdata, ppos+8, *pchunksize);
            }
            else
            {
                return PAL_Failed;
            }

            return PAL_Ok;
        }
    }

    *ppchunkdata = NULL;
    *pchunksize = 0;

    return PAL_Ok;
}


/*! @} */
