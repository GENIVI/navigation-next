/*!--------------------------------------------------------------------------
 
 @file     SpeexEncoder.cpp
 @date     02/23/2012
 @defgroup Speex Encoder Implementation
 
 @brief    Speex Encoder Implementation
 
 */
/*
 (C) Copyright 2012 by TeleCommunication Systems, Inc.           

 The information contained herein is confidential, proprietary 
 to TeleCommunication Systems, Inc., and considered a trade secret as 
 defined in section 499C of the penal code of the State of     
 California. Use of this information by anyone other than      
 authorized employees of TeleCommunication Systems is granted only    
 under a written non-disclosure agreement, expressly           
 prescribing the scope and manner of such use.                 
 
 ---------------------------------------------------------------------------*/

/*! @{ */

#include "SpeexEncoderInterface.h"
#include "speex.h"
#include "speex_header.h"
#include "ogg.h"
#include "palstdlib.h"
#include "palmath.h" // For MIN & MAX

namespace tpspeexencoder
{

// Default speex encoder quality
static const uint32 DEFAULT_SPEEX_ENCODER_QUALITY =   8;   // 15000 bps. Need good headphones to tell the difference.
static const uint32 MAX_SPEEX_FRAME_BYTES = 2000;          // From speex enc sample


/*! Frame reader interface to read incomming buffer by frames one by one
*/
class FrameReaderInterface : public Base
{
public:
    FrameReaderInterface(uint32 bitsPerSample);
    virtual ~FrameReaderInterface(void);

    /*! Frame Reader Factory
    */
    static FrameReaderInterface* Factory(uint32 bitsPerSample, bool bigEndian, uint32 frameSize);

    /*! Setups buffer to read by frames
    */
    void SetBuffer(byte* inBuffer, uint32 inSize, bool lastBuffer);

    /*! Read one frame. Returns TRUE if there are more frames for read and FALSE otherwise.
    */
    virtual bool ReadFrame(int16* frame);

    /*! Resets frame reader internal state to initial
    */
    void Reset(void);

    /*! Returns TRUE if there are more frames for read. Applicable only in last buffer passed case.
    */
    bool HaveMoreFramesInLastBufferMode(void);

protected:
    void FillFrame(int16* frame, uint32 frameSize, uint32* frameFilledSize, int8* buffer, uint32 bufferSize, uint32* bufferDrainSize);
    virtual void DoFillFrame(int16* frame, uint32 frameSize) = 0;   /*! Custom frame filling method */

private:
    PAL_Error Init(uint32 frameSize);

protected:
    byte*                m_bufferPointer;          /*! Pointer to begin of buffer to be read by frames. */
    byte*                m_bufferEndPointer;       /*! Pointer to end of buffer. */
    uint32               m_bufferSize;             /*! Buffer size. */
    uint32               m_frameSize;              /*! Frame size. */
    bool                 m_lastBuffer;             /*! TRUE for last buffer in sequence and FALSE otherwise. */
    byte*                m_restFrame;              /*! Storage for rest unread buffer. */
    uint32               m_restFrameSize;          /*! Size of rest unread buffer. */
    uint32               m_bitsPerSample;          /*! Bits per sample. */
};

FrameReaderInterface::FrameReaderInterface(uint32 bitsPerSample) :
    m_bufferPointer(NULL),
    m_bufferEndPointer(NULL),
    m_bufferSize(0),
    m_frameSize(0),
    m_lastBuffer(FALSE),
    m_restFrame(NULL),
    m_restFrameSize(0),
    m_bitsPerSample(bitsPerSample)
{
}

FrameReaderInterface::~FrameReaderInterface(void)
{
    if (m_restFrame)
    {
        nsl_free(m_restFrame);
        m_restFrame = NULL;
    }
}

PAL_Error
FrameReaderInterface::Init(uint32 frameSize)
{
    if (!frameSize)
    {
        return PAL_ErrBadParam;
    }

    m_frameSize = frameSize;

    // Let's use triple frameSize for rest frame to avoid overlapping in worst case
    m_restFrame = static_cast<byte*>(nsl_malloc(3 * m_frameSize * sizeof(byte)));
    if (!m_restFrame)
    {
        return PAL_ErrNoMem;
    }

    return PAL_Ok;
}

void
FrameReaderInterface::SetBuffer(byte* inBuffer, uint32 inSize, bool lastBuffer)
{
    m_bufferPointer = inBuffer;
    m_bufferEndPointer = m_bufferPointer + inSize;
    m_bufferSize = inSize;
    m_lastBuffer = lastBuffer;
}

bool
FrameReaderInterface::ReadFrame(int16* frame)
{
    if (m_bufferPointer >= m_bufferEndPointer)
    {
        return FALSE;
    }

    // Allow to read left part of buffer only if it's marked as last buffer
    if (!m_lastBuffer)
    {
        uint32 availableSize = (uint32)((8.0 / (double)m_bitsPerSample) * (double)((m_bufferEndPointer - m_bufferPointer) + m_restFrameSize));

        // Left size of buffer and rest buffer is not enough to fill frame fully
        if (availableSize < m_frameSize)
        {
            // Store rest
            nsl_memcpy(m_restFrame + m_restFrameSize, m_bufferPointer, (m_bufferEndPointer - m_bufferPointer) * sizeof(byte));
            m_restFrameSize += (m_bufferEndPointer - m_bufferPointer);
            m_bufferPointer = m_bufferEndPointer;
            return FALSE;
        }
    }

    int16* framePointer = static_cast<int16*>(frame);
    int16* frameEndPointer = frame + m_frameSize;

    // Copy rest buffer to frame
    if (m_restFrameSize)
    {
        uint32 filled = 0;
        uint32 drain = 0;

        FillFrame(framePointer, m_frameSize, &filled, reinterpret_cast<int8*>(m_restFrame), m_restFrameSize, &drain);

        framePointer += filled;
        if (drain >= m_restFrameSize)
        {
            m_restFrameSize = 0;
        }
        else
        {
            // Shift left unread rest to begin of storage
            m_restFrameSize -= drain;
            nsl_memcpy(m_restFrame, m_restFrame + drain, m_restFrameSize * sizeof(byte));
        }

        if (framePointer >= frameEndPointer)
        {
            return TRUE;
        }
    }

    // Copy buffer to frame
    {
        uint32 filled = 0;
        uint32 drain = 0;

        FillFrame(framePointer, (frameEndPointer - framePointer), &filled, reinterpret_cast<int8*>(m_bufferPointer), (m_bufferEndPointer - m_bufferPointer), &drain);

        framePointer += filled;
        m_bufferPointer += drain;

        if (framePointer >= frameEndPointer)
        {
            return TRUE;
        }

        // Fill left part of frame with 0
        nsl_memset(framePointer, 0, (frameEndPointer - framePointer) * sizeof(int16));
    }

    return TRUE;
}

void
FrameReaderInterface::Reset(void)
{
    m_bufferPointer = NULL;
    m_bufferEndPointer = NULL;
    m_bufferSize = 0;
    m_lastBuffer = FALSE;
    m_restFrameSize = 0;
}

bool
FrameReaderInterface::HaveMoreFramesInLastBufferMode(void)
{
    return (m_bufferPointer < m_bufferEndPointer);
}

void
FrameReaderInterface::FillFrame(int16* frame, uint32 frameSize, uint32* frameFilledSize, int8* buffer, uint32 bufferSize, uint32* bufferDrainSize)
{
    uint32 filled = MIN(frameSize, (uint32)(8.0 * (double)bufferSize / (double)m_bitsPerSample));
    uint32 drain = (m_bitsPerSample / 8) * filled;

    *frameFilledSize = filled;
    *bufferDrainSize = drain;

    nsl_memcpy(frame, buffer, drain * sizeof(byte));

    DoFillFrame(frame, filled);
}

class FrameReader8Bit : public FrameReaderInterface
{
public:
    FrameReader8Bit(void) : FrameReaderInterface(8) {}

protected:
    virtual void DoFillFrame(int16* frame, uint32 frameSize);
};

void
FrameReader8Bit::DoFillFrame(int16* frame, uint32 frameSize)
{
    uint8* buffer = reinterpret_cast<uint8*>(frame);

    for (int32 index = frameSize - 1; index >= 0; index--)
    {
        // Convert 8 to 16 bits speex special format
        frame[index] = (int16)((buffer[index] << 8) ^ 0x8000);
    }
}

class FrameReader16BitLittleEndian : public FrameReaderInterface
{
public:
    FrameReader16BitLittleEndian(void) : FrameReaderInterface(16) {}

protected:
    virtual void DoFillFrame(int16* frame, uint32 frameSize) { /* Do nothing */ }
};

class FrameReader16BitBigEndian : public FrameReaderInterface
{
public:
    FrameReader16BitBigEndian(void) : FrameReaderInterface(16) {}

protected:
    virtual void DoFillFrame(int16* frame, uint32 frameSize);
};

void
FrameReader16BitBigEndian::DoFillFrame(int16* frame, uint32 frameSize)
{
    for (uint32 index = 0; index < frameSize; index++)
    {
        // Convert to big endian
        frame[index] = ((int16)((uint16)(frame[index]) << 8) | ((uint16)(frame[index]) >> 8));
    }
}

FrameReaderInterface*
FrameReaderInterface::Factory(uint32 bitsPerSample, bool bigEndian, uint32 frameSize)
{
    FrameReaderInterface* frameReader = NULL;

    // Only 8 and 16 bits format are supported
    if (bitsPerSample == 8)
    {
        frameReader = new FrameReader8Bit;
    }
    else if (bitsPerSample == 16)
    {
        if (bigEndian)
        {
            frameReader = new FrameReader16BitBigEndian;
        }
        else
        {
            frameReader = new FrameReader16BitLittleEndian;
        }
    }

    if (frameReader)
    {
        if (frameReader->Init(frameSize) != PAL_Ok)
        {
            delete frameReader;
            frameReader = NULL;
        }
    }

    return frameReader;
}

class SpeexEncoder : public SpeexEncoderInterface
{
public:
    SpeexEncoder(void);
    virtual ~SpeexEncoder(void);

    PAL_Error Init(PAL_Instance* pal, uint32 bitsPerSample, uint32 sampleRate, bool bigEndian, int32 quality);

    // Overrides SpeexEncoderInterface
    virtual void Release(void);
    virtual PAL_Error EncodeBuffer(byte* inBuffer, uint32 inSize, nb_boolean lastBuffer, byte** outBuffer, uint32* outSize);
    virtual PAL_Error DecodeBuffer(byte* inBuffer, uint32 inSize, byte** outBuffer, uint32* outSize);
    virtual void ResetHistory(void);

protected:
    const SpeexMode* GetSpeexMode(void) const;
    uint32 WriteOggPageToBuffer(ogg_page* page, byte* buffer, uint32 maxBufferSize);

private:
    PAL_Instance*                m_pal;                  /*! Pal instance. */
    uint32                       m_bitsPerSample;        /*! Bits per Sample passe by client. */
    uint32                       m_sampleRate;           /*! Sample rate passed by client. */
    bool                         m_bigEndian;            /*! TRUE for big endian, and FALSE for little endian. Passed by client. */

    // Speex environment
    void*                        m_encState;             /*! Speex encoder state. */
    SpeexBits                    m_bits;                 /*! Speex bit-packing struct. */
    int32                        m_frameSize;            /*! Frame size in samples to be processed by speex encoder. */
    int32                        m_quality;              /*! Speex encoder output quality. Passed by client. */
    FrameReaderInterface*        m_frameReader;          /*! Frame reader to process incomming from client buffer by frames. */
    int16*                       m_frame;                /*! Frame to be filled by frame reader and encoded with speex. */

    // Ogg environment
    ogg_stream_state             m_os;                   /*! Ogg stream. */
    int8*                        m_cbits;                /*! Buffer to temporary hold encoded data. */
    uint32                       m_totalPacket;          /*! Total count of stored ogg packets. */
    int32                        m_lookahead;            /*! Lookahead bytes count. */
};

SpeexEncoder::SpeexEncoder(void) :
    m_pal(NULL),
    m_bitsPerSample(0),
    m_sampleRate(0),
    m_bigEndian(FALSE),
    m_encState(NULL),
    m_frameSize(0),
    m_quality(0),
    m_frameReader(NULL),
    m_frame(NULL),
    m_cbits(NULL),
    m_totalPacket(0),
    m_lookahead(0)
{
    nsl_memset(&m_bits, 0, sizeof(SpeexBits));
    nsl_memset(&m_os, 0, sizeof(ogg_stream_state));
}

PAL_Error
SpeexEncoder::Init(PAL_Instance* pal, uint32 bitsPerSample, uint32 sampleRate, bool bigEndian, int32 quality)
{
    m_pal = pal;

    // Only 8 and 16 bits incomming audio format are supported
    m_bitsPerSample = bitsPerSample;
    if ((m_bitsPerSample != 8) && (m_bitsPerSample != 16))
    {
        return PAL_ErrUnsupported;
    }

    // Only 8 and 16 kHz incomming audio format are supported
    m_sampleRate = sampleRate;
    if ((m_sampleRate != 8000) && (m_sampleRate != 16000))
    {
        return PAL_ErrUnsupported;
    }

    m_bigEndian = bigEndian;
    m_quality = quality;

    // Create speex encoder
    {
        speex_bits_init(&m_bits);
        m_encState = speex_encoder_init(GetSpeexMode());
        if (!m_encState)
        {
            return PAL_Failed;
        }
    }

    // Setup speex encoder
    speex_encoder_ctl(m_encState, SPEEX_SET_SAMPLING_RATE, &m_sampleRate);
    speex_encoder_ctl(m_encState, SPEEX_SET_QUALITY, &m_quality);

    // Get frame size
    speex_encoder_ctl(m_encState, SPEEX_GET_FRAME_SIZE, &m_frameSize);
    if (!m_frameSize)
    {
        return PAL_Failed;
    }

    speex_encoder_ctl(m_encState, SPEEX_GET_LOOKAHEAD, &m_lookahead);

    // Create frame reader
    m_frameReader = FrameReaderInterface::Factory(m_bitsPerSample, m_bigEndian, m_frameSize);
    if (!m_frameReader)
    {
        return PAL_Failed;
    }

    // Allocate space to hold frame
    m_frame = static_cast<int16*>(nsl_malloc(m_frameSize * sizeof(int16)));
    if (!m_frame)
    {
        return PAL_ErrNoMem;
    }

    m_cbits = static_cast<int8*>(nsl_malloc(MAX_SPEEX_FRAME_BYTES));
    if (!m_cbits)
    {
        return PAL_ErrNoMem;
    }

    return PAL_Ok;
}

SpeexEncoder::~SpeexEncoder(void)
{
    if (m_encState)
    {
        speex_encoder_destroy(m_encState);
        m_encState = NULL;
    }

    speex_bits_destroy(&m_bits);
    ogg_stream_clear(&m_os);

    if (m_frameReader)
    {
        delete m_frameReader;
        m_frameReader = NULL;
    }

    if (m_frame)
    {
        nsl_free(m_frame);
        m_frame = NULL;
    }

    if (m_cbits)
    {
        nsl_free(m_cbits);
        m_cbits = NULL;
    }
}

void
SpeexEncoder::Release(void)
{
    delete this;
}

PAL_Error
SpeexEncoder::EncodeBuffer(byte* inBuffer, uint32 inSize, nb_boolean lastBuffer, byte** outBuffer, uint32* outSize)
{
    // Guard against invalid params
    if (!inBuffer || !inSize || !outBuffer || !outSize)
    {
        return PAL_ErrBadParam;
    }

    PAL_Error err = PAL_Failed;

    // Ogg packet
    ogg_packet oggPacket;
    nsl_memset(&oggPacket, 0, sizeof(ogg_packet));

    // Ogg page
    ogg_page oggPage;
    nsl_memset(&oggPage, 0, sizeof(ogg_page));

    int bytesEncoded = 0;

    // We don't know exact encoded buffer size for now. In worst case it will not be great than double size of input buffer plus frame size.
    uint32 encodedBufferMaxSize = (inSize + m_frameSize) * sizeof(int16);
    uint32 encodedBufferActualSize = 0;

    // Allocate space to hold encoded buffer
    byte* encodedBuffer = static_cast<byte*>(nsl_malloc(encodedBufferMaxSize));
    if (!encodedBuffer)
    {
        err = PAL_ErrNoMem;
        goto errexit;
    }

    // Store header as first page
    if (m_totalPacket == 0)
    {
        // Init ogg stream
        ogg_stream_clear(&m_os);
        if (ogg_stream_init(&m_os, 1 /* Logical stream id */) == -1)
        {
            err = PAL_ErrNoMem;
            goto errexit;
        }

        SpeexHeader speexHeader;
        nsl_memset(&speexHeader, 0, sizeof(SpeexHeader));
        speex_init_header(&speexHeader, m_sampleRate, 1 /* Number of channels */, GetSpeexMode());
        speexHeader.frames_per_packet = 1;

        int packetSize = 0;
        nsl_memset(&oggPacket, 0, sizeof(ogg_packet));
        oggPacket.packet = reinterpret_cast<unsigned char*>(speex_header_to_packet(&speexHeader, &packetSize));
        if (!oggPacket.packet)
        {
            err = PAL_ErrNoMem;
            goto errexit;
        }

        oggPacket.bytes = packetSize;
        oggPacket.b_o_s = 1;
        oggPacket.e_o_s = 0;
        oggPacket.granulepos = 0;
        oggPacket.packetno = 0;
        ogg_stream_packetin(&m_os, &oggPacket);

        // Flush page with header
        while (ogg_stream_flush(&m_os, &oggPage))
        {
            encodedBufferActualSize += WriteOggPageToBuffer(&oggPage, (encodedBuffer + encodedBufferActualSize), (encodedBufferMaxSize - encodedBufferActualSize));
        }

        // We need free header packet
        nsl_free(oggPacket.packet);
        oggPacket.packet = NULL;

        m_totalPacket = 1;
    }

    // Setup passed buffer to frame reader
    m_frameReader->SetBuffer(inBuffer, inSize, lastBuffer);

    // Read frames one by one
    while (m_frameReader->ReadFrame(m_frame))
    {
        // Pass frame to speex encoder and append results to encoded buffer
        speex_bits_reset(&m_bits);
        speex_encode_int(m_encState, m_frame, &m_bits);
        bytesEncoded = speex_bits_write(&m_bits, reinterpret_cast<char*>(m_cbits), MAX_SPEEX_FRAME_BYTES);

        // Setup ogg packet
        nsl_memset(&oggPacket, 0, sizeof(ogg_packet));
        oggPacket.packet = reinterpret_cast<unsigned char*>(m_cbits);
        oggPacket.bytes = bytesEncoded;
        oggPacket.packetno = m_totalPacket;
        oggPacket.granulepos = m_totalPacket * m_frameSize - m_lookahead;
        oggPacket.b_o_s = 0;
        oggPacket.e_o_s = 0;

        // Mark last packet
        if (lastBuffer && !m_frameReader->HaveMoreFramesInLastBufferMode())
        {
            oggPacket.e_o_s = 1;
        }

        ogg_stream_packetin(&m_os, &oggPacket);

        // Store full ogg pages to encoded buffer
        while (ogg_stream_pageout(&m_os, &oggPage))
        {
            encodedBufferActualSize += WriteOggPageToBuffer(&oggPage, (encodedBuffer + encodedBufferActualSize), (encodedBufferMaxSize - encodedBufferActualSize));
        }

        m_totalPacket++;

        // Avoid overhead
        if (encodedBufferActualSize >= encodedBufferMaxSize)
        {
            break;
        }
    }

    // Flush left ogg pages
    while (ogg_stream_flush(&m_os, &oggPage))
    {
        encodedBufferActualSize += WriteOggPageToBuffer(&oggPage, (encodedBuffer + encodedBufferActualSize), (encodedBufferMaxSize - encodedBufferActualSize));
    }

    if (encodedBufferActualSize) // Success
    {
        // Realloc encodedBuffer down to actual size. Client is responsible to free this data.
        encodedBuffer = static_cast<byte*>(nsl_realloc(encodedBuffer, encodedBufferActualSize));
        if (encodedBuffer)
        {
            *outBuffer = encodedBuffer;
            *outSize = encodedBufferActualSize;
            encodedBuffer = NULL;
            err = PAL_Ok;
        }
    }
    else // Failed
    {
        goto errexit;
    }

    return err;

errexit:

    if (encodedBuffer)
    {
        nsl_free(encodedBuffer);
        encodedBuffer = NULL;
    }

    return err;
}

PAL_Error
SpeexEncoder::DecodeBuffer(byte* inBuffer, uint32 inSize, byte** outBuffer, uint32* outSize)
{
    // TODO: unsupported for now
    return PAL_ErrUnsupported;
}

void
SpeexEncoder::ResetHistory(void)
{
    m_frameReader->Reset();
    m_totalPacket = 0;
    ogg_stream_clear(&m_os);
}

const SpeexMode*
SpeexEncoder::GetSpeexMode(void) const
{
    int32 modeID = -1;

    if (m_sampleRate == 8000)
    {
        modeID = SPEEX_MODEID_NB;
    }
    else if (m_sampleRate == 16000)
    {
        modeID = SPEEX_MODEID_WB;
    }

    return speex_lib_get_mode(modeID);
}

uint32
SpeexEncoder::WriteOggPageToBuffer(ogg_page* page, byte* buffer, uint32 maxBufferSize)
{
    uint32 stored = 0;

    if (page->header_len <= maxBufferSize)
    {
        nsl_memcpy(buffer, page->header, page->header_len);
        stored += page->header_len;
    }

    if (page->body_len <= (maxBufferSize - stored))
    {
        nsl_memcpy(buffer + stored, page->body, page->body_len);
        stored += page->body_len;
    }

    return stored;
}

PAL_Error
CreateSpeexEncoder(PAL_Instance* pal,
                   uint32 bitsPerSample,
                   uint32 sampleRate,
                   nb_boolean bigEndian,
                   SpeexEncoderInterface** speexEncoder
                   )
{
    PAL_Error err = PAL_ErrUnsupported;

    if (!pal || !speexEncoder)
    {
        return PAL_ErrBadParam;
    }

    SpeexEncoder* encoder = new SpeexEncoder;

    if (encoder)
    {
        err = encoder->Init(pal, bitsPerSample, sampleRate, bigEndian, DEFAULT_SPEEX_ENCODER_QUALITY);
    }

    if (err == PAL_Ok)
    {
        *speexEncoder = encoder;
        encoder = NULL;
    }
    else
    {
        delete encoder;
        encoder = NULL;
    }

    return err;
}

} //tpspeexencoder

/*! @} */
