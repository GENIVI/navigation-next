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

 @file     SpeechProcessor.cpp
 @date     02/18/2012
 @defgroup Speech Processor API

 @brief    Speech Processor Implementation

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

#include "SpeechProcessorInterface.h"
#include "palmath.h"


namespace abspeechprocessor
{

// Speech processor math
#define speechprocessor_ceil(a)  ((((double)(a) - (double)((int32)(a))) > 0.0) ? ((int32)(a) + 1) : ((int32)(a)))
#define speechprocessor_amplitudeToDb(ampl)  (20.0 * nsl_log10(ampl))

// Speech processor default configs
static const uint32 DEFAULT_BASE_CHUNK_SIZE =                       64;  // Size of chunk in samples. Spectrum algorithm supports only 64 chunk size.
static const uint32 DEFAULT_NOISE_DETECTION_INTERVAL =              10;  // Noise detection interval in chunks count
static const double DEFAULT_SN_AMPLITUDE_THRESHOLD =              0.75;  // SN amp threshold for amplitude based algorithm. 0.0 - 1.0. Optimazed for 16 bit audio.
static const double DEFAULT_SN_AMPLITUDE_CONTINUE_THRESHOLD =     0.90;  // SN amp continue speech threshold for amplitude based algorithm. DEFAULT_SN_AMPLITUDE_THRESHOLD - 1.0. Optimazed for 16 bit audio.
static const double DEFAULT_SPEECH_DECLARATION_THRESHOLD =         0.1;  // Speech declaration threshold. 0.0 - 1.0
static const double DEFAULT_MAX_NOISE_LOUDNESS =                 -40.0;  // Max noise loudness for amplitude based algorithm. Optimazed for 16 bit audio.
static const double DEFAULT_NOISE_LOUDNESS_CORRECTION =          -45.0;  // Noise loudness to be used in max loudness case for amplitude based algorithm. Optimazed for 16 bit audio.
static const uint32 DEFAULT_SN_SPECTRUM_THRESHOLD =                400;  // SN spetrum algorithm threshold
static const double DEFAULT_SPECTRUM_NOISE_CORRECTION_RATIO =    0.999;  // Noise correction ratio for spectrum algorithm


/*! Sample convertor from pcm representation to another format ex. amplitude or decibels
*/
class SampleConvertorInterface : public Base
{
public:
    SampleConvertorInterface(void);
    virtual ~SampleConvertorInterface(void) {};

    /*! Decibels convertor factory
    */
    static SampleConvertorInterface* DecibelsFactory(uint32 bitsPerSample);

    /*! Amplitude convertor factory
    */
    static SampleConvertorInterface* AmplitudeFactory(uint32 bitsPerSample);

    /*! Converts sample from pcm byte format to target(amplitude, decibels) double format
    */
    double ConvertSample(int32 sample);

protected:
    virtual NB_Error DoInit(void);                      /*! Custom initializer. */
    virtual double DoConvertSample(int32 sample) = 0;   /*! Custom convertor. */

private:
    NB_Error Init(uint32 bitsPerSample);

protected:
    uint32   m_maxPossibleSampleValue;                  /*! Max possible sample value for given bits per sample. */
};

SampleConvertorInterface::SampleConvertorInterface(void) :
    m_maxPossibleSampleValue(0)
{
}

double
SampleConvertorInterface::ConvertSample(int32 sample)
{
    return DoConvertSample((int32)nsl_fabs(static_cast<double>(sample)));
}

NB_Error
SampleConvertorInterface::Init(uint32 bitsPerSample)
{
    m_maxPossibleSampleValue = (uint32)nsl_pow(2, bitsPerSample - 1 /* Remove sign bit */);
    return DoInit();
}

NB_Error
SampleConvertorInterface::DoInit(void)
{
    return NE_OK;
}

class SampleToDecibelsConvertor : public SampleConvertorInterface
{
protected:
    virtual double DoConvertSample(int32 sample);
};

double
SampleToDecibelsConvertor::DoConvertSample(int32 sample)
{
    double safeSample = ((sample == 0) ? 1.0 : (double)sample);
    return speechprocessor_amplitudeToDb(safeSample / (double)m_maxPossibleSampleValue);
}

class SampleToAmplitudeConvertor : public SampleConvertorInterface
{
protected:
    virtual double DoConvertSample(int32 sample);
};

double
SampleToAmplitudeConvertor::DoConvertSample(int32 sample)
{
    double safeSample = ((sample == 0) ? 1.0 : (double)sample);
    return safeSample / (double)m_maxPossibleSampleValue;
}

class SampleToDecibelsConvertorWithCache : public SampleConvertorInterface
{
public:
    SampleToDecibelsConvertorWithCache(void);
    virtual ~SampleToDecibelsConvertorWithCache(void);

protected:
    virtual NB_Error DoInit(void);
    virtual double DoConvertSample(int32 sample);
    virtual double CalculateCacheElement(int32 sample);

protected:
    double*      m_cacheTable;     /*! Cache table to hold converted values. */
};

SampleToDecibelsConvertorWithCache::SampleToDecibelsConvertorWithCache(void) :
    m_cacheTable(NULL)
{
}

SampleToDecibelsConvertorWithCache::~SampleToDecibelsConvertorWithCache(void)
{
    if (m_cacheTable)
    {
        nsl_free(m_cacheTable);
        m_cacheTable = NULL;
    }
}

NB_Error
SampleToDecibelsConvertorWithCache::DoInit(void)
{
    m_cacheTable = (double*)nsl_malloc(sizeof(double) * (m_maxPossibleSampleValue + 1));
    if (!m_cacheTable)
    {
        return NE_NOMEM;
    }

    // Calculate decibel values from min to max sample
    for (uint32 sample = 1; sample <= m_maxPossibleSampleValue; sample++)
    {
        m_cacheTable[sample] = CalculateCacheElement(sample);
    }

    m_cacheTable[0] = m_cacheTable[1];

    return NE_OK;
}

double
SampleToDecibelsConvertorWithCache::DoConvertSample(int32 sample)
{
    return m_cacheTable[sample];
}

double
SampleToDecibelsConvertorWithCache::CalculateCacheElement(int32 sample)
{
    return speechprocessor_amplitudeToDb((double)sample / (double)m_maxPossibleSampleValue);
}

class SampleToAmplitudeConvertorWithCache : public SampleToDecibelsConvertorWithCache
{
protected:
    virtual double CalculateCacheElement(int32 sample);
};

double
SampleToAmplitudeConvertorWithCache::CalculateCacheElement(int32 sample)
{
    return (double)sample / (double)m_maxPossibleSampleValue;
}

SampleConvertorInterface*
SampleConvertorInterface::DecibelsFactory(uint32 bitsPerSample)
{
    // Only 8 and 16 bits format are supported
    if ((bitsPerSample != 8) && (bitsPerSample != 16))
    {
        return NULL;
    }

    SampleConvertorInterface* convertor = new SampleToDecibelsConvertorWithCache;
    if (convertor->Init(bitsPerSample) != NE_OK)
    {
        // Convertor with cache table is failed probably due to low memory condition
        delete convertor;
        convertor = NULL;

        convertor = new SampleToDecibelsConvertor;
        if (convertor->Init(bitsPerSample) != NE_OK)
        {
            delete convertor;
            convertor = NULL;
        }
    }

    return convertor;
}

SampleConvertorInterface*
SampleConvertorInterface::AmplitudeFactory(uint32 bitsPerSample)
{
    // Only 8 and 16 bits format are supported
    if ((bitsPerSample != 8) && (bitsPerSample != 16))
    {
        return NULL;
    }

    SampleConvertorInterface* convertor = new SampleToAmplitudeConvertorWithCache;
    if (convertor->Init(bitsPerSample) != NE_OK)
    {
        // Convertor with cache table is failed probably due to low memory condition
        delete convertor;
        convertor = NULL;

        convertor = new SampleToAmplitudeConvertor;
        if (convertor->Init(bitsPerSample) != NE_OK)
        {
            delete convertor;
            convertor = NULL;
        }
    }

    return convertor;
}

/*! pcm reader interface to read incomming buffer by chunks one by one
*/
class PCMReaderInterface : public Base
{
public:
    virtual ~PCMReaderInterface(void);
    static PCMReaderInterface* Factory(uint32 bitsPerSample, nb_boolean bigEndian, SampleConvertorInterface* sampleConvertor);

    /*! Setups buffer to read by chunks
    */
    void SetBuffer(byte* inBuffer, uint32 bufferSize, uint32 chunkSize);

    /*! Returns overall chunks count for buffer that was setup thru SetBuffer call
    */
    uint32 GetOverallChunksCount(void);

    /*! Read one chunk. outBuffer will be filled with chunk data. Returns TRUE if there are more chunks to read, and FALSE otherwise.
    */
    nb_boolean ReadChunk(double* outBuffer);

protected:
    PCMReaderInterface(uint32 bitsPerSample);

    /*! Reads one sample and increment internal buffers pointer to next sample
    */
    int32 ReadSample(void);

    /*! Custom read sample implementation
    */
    virtual int32 DoReadSample(void) = 0;

private:
    NB_Error Init(SampleConvertorInterface* sampleConvertor);

protected:
    uint32                       m_bitsPerSample;           /*! Bits per sample passed by client. */
    byte*                        m_bufferPointer;           /*! Pointer to begin of buffer to be read by chunks. */
    byte*                        m_bufferEndPointer;        /*! Pointer to end of buffer. */
    uint32                       m_bufferSize;              /*! Buffer size. */
    uint32                       m_chunkSize;               /*! Chunk size. */
    SampleConvertorInterface*    m_sampleConvertor;         /*! Sample convertor. */
};

PCMReaderInterface::PCMReaderInterface(uint32 bitsPerSample) :
    m_bitsPerSample(bitsPerSample),
    m_bufferPointer(NULL),
    m_bufferEndPointer(NULL),
    m_bufferSize(0),
    m_chunkSize(0),
    m_sampleConvertor(NULL)
{
}

PCMReaderInterface::~PCMReaderInterface(void)
{
    if (m_sampleConvertor)
    {
        delete m_sampleConvertor;
        m_sampleConvertor = NULL;
    }
}

NB_Error
PCMReaderInterface::Init(SampleConvertorInterface* sampleConvertor)
{
    if (!sampleConvertor)
    {
        return NE_BADDATA;
    }

    m_sampleConvertor = sampleConvertor;

    return NE_OK;
}

void
PCMReaderInterface::SetBuffer(byte* buffer, uint32 bufferSize, uint32 chunkSize)
{
    m_bufferPointer = buffer;
    m_bufferEndPointer = m_bufferPointer + bufferSize;
    m_bufferSize = bufferSize;
    m_chunkSize = chunkSize;
}

uint32
PCMReaderInterface::GetOverallChunksCount(void)
{
    uint32 chunksCount = 0;

    if (m_bufferPointer && m_bufferSize && m_chunkSize)
    {
       chunksCount = speechprocessor_ceil((double)m_bufferSize / (double)(m_chunkSize * (m_bitsPerSample / 8)));
    }

    return chunksCount;
}

nb_boolean
PCMReaderInterface::ReadChunk(double* outBuffer)
{
    if (m_bufferPointer >= m_bufferEndPointer)
    {
        return FALSE;
    }

    for (uint32 sample = 0; sample < m_chunkSize; sample++)
    {
        outBuffer[sample] = m_sampleConvertor->ConvertSample(ReadSample());
    }

    return TRUE;
}

int32
PCMReaderInterface::ReadSample(void)
{
    int32 sample = 0;

    if (m_bufferPointer < m_bufferEndPointer)
    {
        sample = DoReadSample();
    }

    return sample;
}

class PCMReader8Bit : public PCMReaderInterface
{
public:
    PCMReader8Bit(void) : PCMReaderInterface(8) {};

protected:
    virtual int32 DoReadSample(void);
};

int32
PCMReader8Bit::DoReadSample(void)
{
    int8 sample = (int8)(*m_bufferPointer);
    m_bufferPointer++;

    return (int32)sample;
}

class PCMReader16BitLittleEndian : public PCMReaderInterface
{
public:
    PCMReader16BitLittleEndian(void) : PCMReaderInterface(16) {};

protected:
    virtual int32 DoReadSample(void);
    virtual int32 BitOrderedSample(int32 byte1, int32 byte2);
};

int32
PCMReader16BitLittleEndian::DoReadSample(void)
{
    int8 byte1 = (int8)(*m_bufferPointer);
    m_bufferPointer++;

    int8 byte2 = 0;
    if (m_bufferPointer < m_bufferEndPointer)
    {
        byte2 = (int8)(*m_bufferPointer);
        m_bufferPointer++;
    }

    return BitOrderedSample(byte1, byte2);
}

int32
PCMReader16BitLittleEndian::BitOrderedSample(int32 byte1, int32 byte2)
{
    return (int16)(byte1 | (byte2 << 8));
}

class PCMReader16BitBigEndian : public PCMReader16BitLittleEndian
{
protected:
    virtual int32 BitOrderedSample(int32 byte1, int32 byte2);
};

int32
PCMReader16BitBigEndian::BitOrderedSample(int32 byte1, int32 byte2)
{
    return (int16)((byte1 << 8) | byte2);
}

PCMReaderInterface*
PCMReaderInterface::Factory(uint32 bitsPerSample, nb_boolean bigEndian, SampleConvertorInterface* sampleConvertor)
{
    PCMReaderInterface* pcmReader = NULL;

    // Only 8 and 16 bits format are supported
    if (bitsPerSample == 8)
    {
        pcmReader = new PCMReader8Bit;
    }
    else if (bitsPerSample == 16)
    {
        if (bigEndian)
        {
            pcmReader = new PCMReader16BitBigEndian;
        }
        else
        {
            pcmReader = new PCMReader16BitLittleEndian;
        }
    }

    if (pcmReader)
    {
        if (pcmReader->Init(sampleConvertor) != NE_OK)
        {
            delete pcmReader;
            pcmReader = NULL;
        }
    }

    return pcmReader;
}

class SpeechProcessor : public SpeechProcessorInterface
{
public:
    SpeechProcessor(void);
    virtual ~SpeechProcessor(void);

    NB_Error Init(NB_Context* context, uint32 bitsPerSample, uint32 sampleRate, nb_boolean bigEndian, uint32 noiseDetectionChunksCount, double speechDeclarationThreshold);

    // Overrides SpeechProcessorInterface
    virtual void Release(void);
    virtual SpeechProcessorResult ProcessBuffer(byte* buffer, uint32 bufferSize);
    virtual void ResetHistory(void);

protected:
    virtual nb_boolean DoInit(void);                                    /*! Custom initializer. */
    virtual void DoResetHistory(void) = 0;                              /*! Custom reset history. */
    virtual nb_boolean ProcessChunk(double* chunk) = 0;                 /*! Process chunk of m_baseChunkSize size. Return TRUE for audible chunk, and FALSE otherwise. */
    virtual SampleConvertorInterface* GetSampleConvertor(void) = 0;     /*! Returns custom sample convertor for given algorithm implementation. */

protected:
    uint32                  m_bitsPerSample;                            /*! Bits per sample passed by client. */
    uint32                  m_noiseDetectionChunksCount;                /*! Chunks count to be processed for noise detection. */
    int32                   m_baseChunkSize;                            /*! Chunk size. */
    uint32                  m_chunksCounter;                            /*! Chunks processed count. */
    nb_boolean              m_speechIsDetected;                         /*! TRUE when speech is currently detected and FALSE otherwise. */

private:
    NB_Context*             m_context;                                  /*! NB Context. */
    PCMReaderInterface*     m_pcmReader;                                /*! PCM reader. */
    double*                 m_chunk;                                    /*! Storage to hold chunk. */
    uint32                  m_sampleRate;                               /*! Sample rate for given audio. Don't used for now but can be get in to account in further. Passed by client. */
    nb_boolean              m_bigEndian;                                /*! TRUE for big endian audio format and FALSE otherwise. Passed by client. */
    double                  m_speechDeclarationThreshold;               /*! Speech declaration threshold. */
};

SpeechProcessor::SpeechProcessor(void) :
    m_bitsPerSample(0),
    m_noiseDetectionChunksCount(0),
    m_baseChunkSize(DEFAULT_BASE_CHUNK_SIZE),
    m_chunksCounter(0),
    m_speechIsDetected(FALSE),
    m_context(NULL),
    m_pcmReader(NULL),
    m_chunk(NULL),
    m_sampleRate(0),
    m_bigEndian(FALSE),
    m_speechDeclarationThreshold(0.0)
{
}

SpeechProcessor::~SpeechProcessor(void)
{
    if (m_pcmReader)
    {
        delete m_pcmReader;
        m_pcmReader = NULL;
    }

    if (m_chunk)
    {
        nsl_free(m_chunk);
        m_chunk = NULL;
    }
}

void
SpeechProcessor::Release(void)
{
    delete this;
}

NB_Error
SpeechProcessor::Init(NB_Context* context, uint32 bitsPerSample, uint32 sampleRate, nb_boolean bigEndian, uint32 noiseDetectionChunksCount, double speechDeclarationThreshold)
{
    m_context = context;
    m_bitsPerSample = bitsPerSample;
    m_sampleRate = sampleRate;
    m_bigEndian = bigEndian;
    m_noiseDetectionChunksCount = noiseDetectionChunksCount;
    m_speechDeclarationThreshold = speechDeclarationThreshold;

    m_pcmReader = PCMReaderInterface::Factory(bitsPerSample, bigEndian, GetSampleConvertor());
    if (!m_pcmReader)
    {
        return NE_BADDATA;
    }

    m_chunk = (double*)nsl_malloc(m_baseChunkSize * sizeof(double));
    if (!m_chunk)
    {
        return NE_NOMEM;
    }

    if (!DoInit())
    {
        return NE_BADDATA;
    }

    ResetHistory();

    return NE_OK;
}

void
SpeechProcessor::ResetHistory(void)
{
    m_chunksCounter = 0;
    m_speechIsDetected = FALSE;

    DoResetHistory();
}

SpeechProcessorResult
SpeechProcessor::ProcessBuffer(byte* buffer, uint32 bufferSize)
{
    if (!buffer || !bufferSize)
    {
        return SPR_GeneralError;
    }

    // Setup pcm reader
    m_pcmReader->SetBuffer(buffer, bufferSize, m_baseChunkSize);

    uint32 chunksCount = m_pcmReader->GetOverallChunksCount();
    uint32 chunksCountForSpeechDeclaration = speechprocessor_ceil((double)chunksCount * m_speechDeclarationThreshold);
    uint32 chunksCountForNoSpeechDeclaration = chunksCount - chunksCountForSpeechDeclaration;
    uint32 chunksWithSpeechDetectedCount = 0;

    uint32 chunksLooked = 0;
    nb_boolean chunkIsAudible = FALSE;
    SpeechProcessorResult result = SPR_NoSpeechDetected;

    // Process chunks one by one
    while (m_pcmReader->ReadChunk(m_chunk))
    {
        chunkIsAudible = ProcessChunk(m_chunk);

        m_chunksCounter++;
        chunksLooked++;

        if (chunkIsAudible)
        {
            chunksWithSpeechDetectedCount++;

            if (chunksWithSpeechDetectedCount >= chunksCountForSpeechDeclaration)
            {
                result = SPR_SpeechDetected;
                break;
            }
        }
        else
        {
            uint32 chunkWithNoSpeechDetected = chunksLooked - chunksWithSpeechDetectedCount;

            if (chunkWithNoSpeechDetected >= chunksCountForNoSpeechDeclaration)
            {
                result = SPR_NoSpeechDetected;
                break;
            }
        }
    }

    return result;
}

nb_boolean
SpeechProcessor::DoInit(void)
{
    return TRUE;
}

class AmplitudeBasedAlgorithm : public SpeechProcessor
{
public:
    AmplitudeBasedAlgorithm(double speechRedetectionThreshold, double speechContinueThreshold);

protected:
    virtual void DoResetHistory(void);
    virtual nb_boolean ProcessChunk(double* chunk);
    virtual SampleConvertorInterface* GetSampleConvertor(void);

private:
    double       m_initialNoiseDb;
    double       m_chunkLoudnessDb;
    double       m_speechRedetectionThreshold;
    double       m_speechContinueThreshold;
};

AmplitudeBasedAlgorithm::AmplitudeBasedAlgorithm(double speechRedetectionThreshold, double speechContinueThreshold) :
    m_initialNoiseDb(0.0),
    m_chunkLoudnessDb(0.0),
    m_speechRedetectionThreshold(speechRedetectionThreshold),
    m_speechContinueThreshold(speechContinueThreshold)
{
}

void
AmplitudeBasedAlgorithm::DoResetHistory(void)
{
    m_initialNoiseDb = 0.0;
    m_chunkLoudnessDb = 0.0;
}

nb_boolean
AmplitudeBasedAlgorithm::ProcessChunk(double* chunk)
{
    if (m_chunksCounter < m_noiseDetectionChunksCount)
    {
        for (int sample = 0; sample < m_baseChunkSize; sample++)
        {
            m_initialNoiseDb += chunk[sample];
        }

        return FALSE;
    }
    else
    {
        if (m_chunksCounter == m_noiseDetectionChunksCount)
        {
            m_initialNoiseDb /= m_noiseDetectionChunksCount * m_baseChunkSize;
            if (m_initialNoiseDb > DEFAULT_MAX_NOISE_LOUDNESS)
            {
                m_initialNoiseDb = DEFAULT_NOISE_LOUDNESS_CORRECTION;
            }
        }

        m_chunkLoudnessDb = 0.0;

        for (int32 sample = 0; sample < m_baseChunkSize; sample++)
        {
            m_chunkLoudnessDb += chunk[sample];
        }

        m_chunkLoudnessDb /= m_baseChunkSize;

        double SNd = m_chunkLoudnessDb / m_initialNoiseDb;
        m_speechIsDetected = (SNd < (m_speechIsDetected ? m_speechContinueThreshold : m_speechRedetectionThreshold));
    }

    return m_speechIsDetected;
}

SampleConvertorInterface*
AmplitudeBasedAlgorithm::GetSampleConvertor(void)
{
    return SampleConvertorInterface::DecibelsFactory(m_bitsPerSample);
}

class SpectrumBasedAlgorithm : public SpeechProcessor
{
public:
    SpectrumBasedAlgorithm(uint32 speechRedetectionThreshold);
    virtual ~SpectrumBasedAlgorithm(void);

protected:
    virtual nb_boolean DoInit(void);
    virtual void DoResetHistory(void);
    virtual nb_boolean ProcessChunk(double* chunk);
    virtual SampleConvertorInterface* GetSampleConvertor(void);

private:
/*! FFT staff
 Copyright:
 Copyright(C) 1996-2001 Takuya OOURA
 email: ooura@mmm.t.u-tokyo.ac.jp
 download: http://momonga.t.u-tokyo.ac.jp/~ooura/fft.html
 You may use, copy, modify this code for any purpose and
 without fee. You may distribute this ORIGINAL package.
 */
    void ComplexDFT(int32 n, int32 isgn, double *a, int32 *ip, double *w);
    void makewt(int32 nw, int32* ip, double* w);
    void bitrv2(int32 n, int32* ip, double* a);
    void cftfsub(int32 n, double* a, double* w);
    void bitrv2conj(int32 n, int32* ip, double* a);
    void cftbsub(int32 n, double* a, double* w);
    void cft1st(int32 n, double* a, double* w);
    void cftmdl(int32 n, int32 l, double* a, double* w);

private:
    double       m_snr;
    int32*       m_bitReversalArea;       /*! Work area for bit reversal. */
    double*      m_cosSinTable;           /*! Cos/Sin table. */
    double*      m_fftData;               /*! Fast Fourier Transform data. */
    double*      m_noiseLevel;
    double*      m_lambda;
    double       m_lambda0;
    uint32       m_speechRedetectionThreshold;
};

SpectrumBasedAlgorithm::SpectrumBasedAlgorithm(uint32 speechRedetectionThreshold) :
    m_snr(0.0),
    m_bitReversalArea(NULL),
    m_cosSinTable(NULL),
    m_fftData(NULL),
    m_noiseLevel(NULL),
    m_lambda(NULL),
    m_lambda0(0),
    m_speechRedetectionThreshold(speechRedetectionThreshold)
{
}

SpectrumBasedAlgorithm::~SpectrumBasedAlgorithm(void)
{
    if (m_bitReversalArea)
    {
        nsl_free(m_bitReversalArea);
        m_bitReversalArea = NULL;
    }

    if (m_cosSinTable)
    {
        nsl_free(m_cosSinTable);
        m_cosSinTable = NULL;
    }

    if (m_fftData)
    {
        nsl_free(m_fftData);
        m_fftData = NULL;
    }

    if (m_noiseLevel)
    {
        nsl_free(m_noiseLevel);
        m_noiseLevel = NULL;
    }

    if (m_lambda)
    {
        nsl_free(m_lambda);
        m_lambda = NULL;
    }
}

nb_boolean
SpectrumBasedAlgorithm::DoInit(void)
{
    // Should be power of 2
    if (m_baseChunkSize % 2)
    {
        return FALSE;
    }

    int32 halfChunkSize = m_baseChunkSize >> 1;

    m_bitReversalArea = (int32*)nsl_malloc((2 + (uint32)nsl_sqrt((double)halfChunkSize)) * sizeof(uint32));
    if (!m_bitReversalArea)
    {
        return FALSE;
    }

    m_cosSinTable = (double*)nsl_malloc((halfChunkSize - 1) * sizeof(double));
    if (!m_cosSinTable)
    {
        return FALSE;
    }

    m_fftData = (double*)nsl_malloc(halfChunkSize * sizeof(double));
    if (!m_fftData)
    {
        return FALSE;
    }

    m_noiseLevel = (double*)nsl_malloc(halfChunkSize * sizeof(double));
    if (!m_noiseLevel)
    {
        return FALSE;
    }

    m_lambda = (double*)nsl_malloc(halfChunkSize * sizeof(double));
    if (!m_lambda)
    {
        return FALSE;
    }

    m_lambda0 = nsl_pow(100.0 / (double)nsl_pow(2, 32), 2);

    return TRUE;
}

void
SpectrumBasedAlgorithm::DoResetHistory(void)
{
    m_snr = 0.0;
    m_bitReversalArea[0] = 0;
}

nb_boolean
SpectrumBasedAlgorithm::ProcessChunk(double* chunk)
{
    uint32 index = 0;
    uint32 halfChunkSize = m_baseChunkSize >> 1;

    ComplexDFT(m_baseChunkSize, 1, chunk, m_bitReversalArea, m_cosSinTable);

    // Fill FFT data
    {
        int32 doubleIndex = 0;

        for (index = 0; index < halfChunkSize; index++)
        {
            doubleIndex = index << 1;
            m_fftData[index] = nsl_sqrt(chunk[doubleIndex] * chunk[doubleIndex] + chunk[doubleIndex + 1] * chunk[doubleIndex + 1]);
        }
    }

    // Noise estimation
    if (m_chunksCounter < m_noiseDetectionChunksCount)
    {
        if (m_chunksCounter == 0)
        {
            for (index = 0; index < halfChunkSize; index++)
            {
                m_lambda[index] = m_fftData[index] * m_fftData[index];
                m_noiseLevel[index] = m_lambda[index] + m_lambda0;
            }
        }
        else
        {
            for (index = 0; index < halfChunkSize; index++)
            {
                m_lambda[index] = (m_lambda[index] + m_fftData[index] * m_fftData[index]) / 2.0;
                m_noiseLevel[index] = m_lambda[index] + m_lambda0;
            }
        }

        // Correct max noise level
        if (m_chunksCounter == (m_noiseDetectionChunksCount - 1))
        {
            double noise = 0.0;

            for (index = 0; index < halfChunkSize; index++)
            {
                noise += m_noiseLevel[index];
            }

            if ((noise / (double)halfChunkSize) > 0.012)
            {
                 for (index = 0; index < halfChunkSize; index++)
                 {
                     m_lambda[index] = 0.006;
                     m_noiseLevel[index] = m_lambda[index] + m_lambda0;
                 }
            }
        }
    }

    double previousSnr = m_snr;
    m_snr = 0;

    for (index = 0; index < halfChunkSize; index++)
    {
        m_snr += (m_fftData[index] * m_fftData[index]) / m_noiseLevel[index];
    }

    m_snr /= halfChunkSize;

    if (m_chunksCounter < m_noiseDetectionChunksCount)
    {
        return FALSE;
    }
    else
    {
        if (m_speechIsDetected)
        {
            m_speechIsDetected = ((m_snr > m_speechRedetectionThreshold) || ((m_snr > m_speechRedetectionThreshold / 10) && (previousSnr > m_speechRedetectionThreshold)));
        }
        else
        {
            m_speechIsDetected = ((m_snr > m_speechRedetectionThreshold / 10) && (previousSnr > m_speechRedetectionThreshold));
        }
    }

    // Noise correction in speech is not detected case
    if (!m_speechIsDetected)
    {
        for (index = 0; index < halfChunkSize; index++)
        {
            m_lambda[index] = DEFAULT_SPECTRUM_NOISE_CORRECTION_RATIO * m_lambda[index] +  (1.0 - DEFAULT_SPECTRUM_NOISE_CORRECTION_RATIO) * m_fftData[index] * m_fftData[index];
            m_noiseLevel[index] = m_lambda[index] + m_lambda0;
        }
    }

    return m_speechIsDetected;
}

SampleConvertorInterface*
SpectrumBasedAlgorithm::GetSampleConvertor(void)
{
    return SampleConvertorInterface::AmplitudeFactory(m_bitsPerSample);
}

void
SpectrumBasedAlgorithm::ComplexDFT(int32 n, int32 isgn, double* a, int32* ip, double* w)
{
    if (n > (ip[0] << 2))
    {
        makewt(n >> 2, ip, w);
    }

    if (n > 4)
    {
        if (isgn >= 0)
        {
            bitrv2(n, ip + 2, a);
            cftfsub(n, a, w);
        }
        else
        {
            bitrv2conj(n, ip + 2, a);
            cftbsub(n, a, w);
        }
    }
    else if (n == 4)
    {
        cftfsub(n, a, w);
    }
}

void
SpectrumBasedAlgorithm::makewt(int32 nw, int32* ip, double* w)
{
    int32 j, nwh;
    double delta, x, y;

    ip[0] = nw;
    ip[1] = 1;

    if (nw > 2)
    {
        nwh = nw >> 1;
        delta = nsl_atan(1.0) / nwh;
        w[0] = 1;
        w[1] = 0;
        w[nwh] = nsl_cos(delta * nwh);
        w[nwh + 1] = w[nwh];

        if (nwh > 2)
        {
            for (j = 2; j < nwh; j += 2)
            {
                x = nsl_cos(delta * j);
                y = nsl_sin(delta * j);
                w[j] = x;
                w[j + 1] = y;
                w[nw - j] = y;
                w[nw - j + 1] = x;
            }

            bitrv2(nw, ip + 2, w);
        }
    }
}

void
SpectrumBasedAlgorithm::bitrv2(int32 n, int32* ip, double* a)
{
    int32 j, j1, k, k1, l, m, m2;
    double xr, xi, yr, yi;

    ip[0] = 0;
    l = n;
    m = 1;

    while ((m << 3) < l)
    {
        l >>= 1;

        for (j = 0; j < m; j++)
        {
            ip[m + j] = ip[j] + l;
        }

        m <<= 1;
    }

    m2 = 2 * m;

    if ((m << 3) == l)
    {
        for (k = 0; k < m; k++)
        {
            for (j = 0; j < k; j++)
            {
                j1 = 2 * j + ip[k];
                k1 = 2 * k + ip[j];
                xr = a[j1];
                xi = a[j1 + 1];
                yr = a[k1];
                yi = a[k1 + 1];
                a[j1] = yr;
                a[j1 + 1] = yi;
                a[k1] = xr;
                a[k1 + 1] = xi;
                j1 += m2;
                k1 += 2 * m2;
                xr = a[j1];
                xi = a[j1 + 1];
                yr = a[k1];
                yi = a[k1 + 1];
                a[j1] = yr;
                a[j1 + 1] = yi;
                a[k1] = xr;
                a[k1 + 1] = xi;
                j1 += m2;
                k1 -= m2;
                xr = a[j1];
                xi = a[j1 + 1];
                yr = a[k1];
                yi = a[k1 + 1];
                a[j1] = yr;
                a[j1 + 1] = yi;
                a[k1] = xr;
                a[k1 + 1] = xi;
                j1 += m2;
                k1 += 2 * m2;
                xr = a[j1];
                xi = a[j1 + 1];
                yr = a[k1];
                yi = a[k1 + 1];
                a[j1] = yr;
                a[j1 + 1] = yi;
                a[k1] = xr;
                a[k1 + 1] = xi;
            }

            j1 = 2 * k + m2 + ip[k];
            k1 = j1 + m2;
            xr = a[j1];
            xi = a[j1 + 1];
            yr = a[k1];
            yi = a[k1 + 1];
            a[j1] = yr;
            a[j1 + 1] = yi;
            a[k1] = xr;
            a[k1 + 1] = xi;
        }
    }
    else
    {
        for (k = 1; k < m; k++)
        {
            for (j = 0; j < k; j++)
            {
                j1 = 2 * j + ip[k];
                k1 = 2 * k + ip[j];
                xr = a[j1];
                xi = a[j1 + 1];
                yr = a[k1];
                yi = a[k1 + 1];
                a[j1] = yr;
                a[j1 + 1] = yi;
                a[k1] = xr;
                a[k1 + 1] = xi;
                j1 += m2;
                k1 += m2;
                xr = a[j1];
                xi = a[j1 + 1];
                yr = a[k1];
                yi = a[k1 + 1];
                a[j1] = yr;
                a[j1 + 1] = yi;
                a[k1] = xr;
                a[k1 + 1] = xi;
            }
        }
    }
}

void
SpectrumBasedAlgorithm::cftfsub(int32 n, double* a, double* w)
{
    int32 j, j1, j2, j3, l;
    double x0r, x0i, x1r, x1i, x2r, x2i, x3r, x3i;

    l = 2;

    if (n > 8)
    {
        cft1st(n, a, w);
        l = 8;

        while ((l << 2) < n)
        {
            cftmdl(n, l, a, w);
            l <<= 2;
        }
    }

    if ((l << 2) == n)
    {
        for (j = 0; j < l; j += 2)
        {
            j1 = j + l;
            j2 = j1 + l;
            j3 = j2 + l;
            x0r = a[j] + a[j1];
            x0i = a[j + 1] + a[j1 + 1];
            x1r = a[j] - a[j1];
            x1i = a[j + 1] - a[j1 + 1];
            x2r = a[j2] + a[j3];
            x2i = a[j2 + 1] + a[j3 + 1];
            x3r = a[j2] - a[j3];
            x3i = a[j2 + 1] - a[j3 + 1];
            a[j] = x0r + x2r;
            a[j + 1] = x0i + x2i;
            a[j2] = x0r - x2r;
            a[j2 + 1] = x0i - x2i;
            a[j1] = x1r - x3i;
            a[j1 + 1] = x1i + x3r;
            a[j3] = x1r + x3i;
            a[j3 + 1] = x1i - x3r;
        }
    }
    else
    {
        for (j = 0; j < l; j += 2)
        {
            j1 = j + l;
            x0r = a[j] - a[j1];
            x0i = a[j + 1] - a[j1 + 1];
            a[j] += a[j1];
            a[j + 1] += a[j1 + 1];
            a[j1] = x0r;
            a[j1 + 1] = x0i;
        }
    }
}

void
SpectrumBasedAlgorithm::bitrv2conj(int32 n, int32* ip, double* a)
{
    int32 j, j1, k, k1, l, m, m2;
    double xr, xi, yr, yi;

    ip[0] = 0;
    l = n;
    m = 1;

    while ((m << 3) < l)
    {
        l >>= 1;

        for (j = 0; j < m; j++)
        {
            ip[m + j] = ip[j] + l;
        }

        m <<= 1;
    }

    m2 = 2 * m;

    if ((m << 3) == l)
    {
        for (k = 0; k < m; k++)
        {
            for (j = 0; j < k; j++)
            {
                j1 = 2 * j + ip[k];
                k1 = 2 * k + ip[j];
                xr = a[j1];
                xi = -a[j1 + 1];
                yr = a[k1];
                yi = -a[k1 + 1];
                a[j1] = yr;
                a[j1 + 1] = yi;
                a[k1] = xr;
                a[k1 + 1] = xi;
                j1 += m2;
                k1 += 2 * m2;
                xr = a[j1];
                xi = -a[j1 + 1];
                yr = a[k1];
                yi = -a[k1 + 1];
                a[j1] = yr;
                a[j1 + 1] = yi;
                a[k1] = xr;
                a[k1 + 1] = xi;
                j1 += m2;
                k1 -= m2;
                xr = a[j1];
                xi = -a[j1 + 1];
                yr = a[k1];
                yi = -a[k1 + 1];
                a[j1] = yr;
                a[j1 + 1] = yi;
                a[k1] = xr;
                a[k1 + 1] = xi;
                j1 += m2;
                k1 += 2 * m2;
                xr = a[j1];
                xi = -a[j1 + 1];
                yr = a[k1];
                yi = -a[k1 + 1];
                a[j1] = yr;
                a[j1 + 1] = yi;
                a[k1] = xr;
                a[k1 + 1] = xi;
            }

            k1 = 2 * k + ip[k];
            a[k1 + 1] = -a[k1 + 1];
            j1 = k1 + m2;
            k1 = j1 + m2;
            xr = a[j1];
            xi = -a[j1 + 1];
            yr = a[k1];
            yi = -a[k1 + 1];
            a[j1] = yr;
            a[j1 + 1] = yi;
            a[k1] = xr;
            a[k1 + 1] = xi;
            k1 += m2;
            a[k1 + 1] = -a[k1 + 1];
        }
    }
    else
    {
        a[1] = -a[1];
        a[m2 + 1] = -a[m2 + 1];

        for (k = 1; k < m; k++)
        {
            for (j = 0; j < k; j++)
            {
                j1 = 2 * j + ip[k];
                k1 = 2 * k + ip[j];
                xr = a[j1];
                xi = -a[j1 + 1];
                yr = a[k1];
                yi = -a[k1 + 1];
                a[j1] = yr;
                a[j1 + 1] = yi;
                a[k1] = xr;
                a[k1 + 1] = xi;
                j1 += m2;
                k1 += m2;
                xr = a[j1];
                xi = -a[j1 + 1];
                yr = a[k1];
                yi = -a[k1 + 1];
                a[j1] = yr;
                a[j1 + 1] = yi;
                a[k1] = xr;
                a[k1 + 1] = xi;
            }

            k1 = 2 * k + ip[k];
            a[k1 + 1] = -a[k1 + 1];
            a[k1 + m2 + 1] = -a[k1 + m2 + 1];
        }
    }
}

void
SpectrumBasedAlgorithm::cftbsub(int32 n, double* a, double* w)
{
    int32 j, j1, j2, j3, l;
    double x0r, x0i, x1r, x1i, x2r, x2i, x3r, x3i;

    l = 2;

    if (n > 8)
    {
        cft1st(n, a, w);
        l = 8;

        while ((l << 2) < n)
        {
            cftmdl(n, l, a, w);
            l <<= 2;
        }
    }

    if ((l << 2) == n)
    {
        for (j = 0; j < l; j += 2)
        {
            j1 = j + l;
            j2 = j1 + l;
            j3 = j2 + l;
            x0r = a[j] + a[j1];
            x0i = -a[j + 1] - a[j1 + 1];
            x1r = a[j] - a[j1];
            x1i = -a[j + 1] + a[j1 + 1];
            x2r = a[j2] + a[j3];
            x2i = a[j2 + 1] + a[j3 + 1];
            x3r = a[j2] - a[j3];
            x3i = a[j2 + 1] - a[j3 + 1];
            a[j] = x0r + x2r;
            a[j + 1] = x0i - x2i;
            a[j2] = x0r - x2r;
            a[j2 + 1] = x0i + x2i;
            a[j1] = x1r - x3i;
            a[j1 + 1] = x1i - x3r;
            a[j3] = x1r + x3i;
            a[j3 + 1] = x1i + x3r;
        }
    }
    else
    {
        for (j = 0; j < l; j += 2)
        {
            j1 = j + l;
            x0r = a[j] - a[j1];
            x0i = -a[j + 1] + a[j1 + 1];
            a[j] += a[j1];
            a[j + 1] = -a[j + 1] - a[j1 + 1];
            a[j1] = x0r;
            a[j1 + 1] = x0i;
        }
    }
}

void
SpectrumBasedAlgorithm::cft1st(int32 n, double* a, double* w)
{
    int32 j, k1, k2;
    double wk1r, wk1i, wk2r, wk2i, wk3r, wk3i;
    double x0r, x0i, x1r, x1i, x2r, x2i, x3r, x3i;

    x0r = a[0] + a[2];
    x0i = a[1] + a[3];
    x1r = a[0] - a[2];
    x1i = a[1] - a[3];
    x2r = a[4] + a[6];
    x2i = a[5] + a[7];
    x3r = a[4] - a[6];
    x3i = a[5] - a[7];
    a[0] = x0r + x2r;
    a[1] = x0i + x2i;
    a[4] = x0r - x2r;
    a[5] = x0i - x2i;
    a[2] = x1r - x3i;
    a[3] = x1i + x3r;
    a[6] = x1r + x3i;
    a[7] = x1i - x3r;
    wk1r = w[2];
    x0r = a[8] + a[10];
    x0i = a[9] + a[11];
    x1r = a[8] - a[10];
    x1i = a[9] - a[11];
    x2r = a[12] + a[14];
    x2i = a[13] + a[15];
    x3r = a[12] - a[14];
    x3i = a[13] - a[15];
    a[8] = x0r + x2r;
    a[9] = x0i + x2i;
    a[12] = x2i - x0i;
    a[13] = x0r - x2r;
    x0r = x1r - x3i;
    x0i = x1i + x3r;
    a[10] = wk1r * (x0r - x0i);
    a[11] = wk1r * (x0r + x0i);
    x0r = x3i + x1r;
    x0i = x3r - x1i;
    a[14] = wk1r * (x0i - x0r);
    a[15] = wk1r * (x0i + x0r);
    k1 = 0;

    for (j = 16; j < n; j += 16)
    {
        k1 += 2;
        k2 = 2 * k1;
        wk2r = w[k1];
        wk2i = w[k1 + 1];
        wk1r = w[k2];
        wk1i = w[k2 + 1];
        wk3r = wk1r - 2 * wk2i * wk1i;
        wk3i = 2 * wk2i * wk1r - wk1i;
        x0r = a[j] + a[j + 2];
        x0i = a[j + 1] + a[j + 3];
        x1r = a[j] - a[j + 2];
        x1i = a[j + 1] - a[j + 3];
        x2r = a[j + 4] + a[j + 6];
        x2i = a[j + 5] + a[j + 7];
        x3r = a[j + 4] - a[j + 6];
        x3i = a[j + 5] - a[j + 7];
        a[j] = x0r + x2r;
        a[j + 1] = x0i + x2i;
        x0r -= x2r;
        x0i -= x2i;
        a[j + 4] = wk2r * x0r - wk2i * x0i;
        a[j + 5] = wk2r * x0i + wk2i * x0r;
        x0r = x1r - x3i;
        x0i = x1i + x3r;
        a[j + 2] = wk1r * x0r - wk1i * x0i;
        a[j + 3] = wk1r * x0i + wk1i * x0r;
        x0r = x1r + x3i;
        x0i = x1i - x3r;
        a[j + 6] = wk3r * x0r - wk3i * x0i;
        a[j + 7] = wk3r * x0i + wk3i * x0r;
        wk1r = w[k2 + 2];
        wk1i = w[k2 + 3];
        wk3r = wk1r - 2 * wk2r * wk1i;
        wk3i = 2 * wk2r * wk1r - wk1i;
        x0r = a[j + 8] + a[j + 10];
        x0i = a[j + 9] + a[j + 11];
        x1r = a[j + 8] - a[j + 10];
        x1i = a[j + 9] - a[j + 11];
        x2r = a[j + 12] + a[j + 14];
        x2i = a[j + 13] + a[j + 15];
        x3r = a[j + 12] - a[j + 14];
        x3i = a[j + 13] - a[j + 15];
        a[j + 8] = x0r + x2r;
        a[j + 9] = x0i + x2i;
        x0r -= x2r;
        x0i -= x2i;
        a[j + 12] = -wk2i * x0r - wk2r * x0i;
        a[j + 13] = -wk2i * x0i + wk2r * x0r;
        x0r = x1r - x3i;
        x0i = x1i + x3r;
        a[j + 10] = wk1r * x0r - wk1i * x0i;
        a[j + 11] = wk1r * x0i + wk1i * x0r;
        x0r = x1r + x3i;
        x0i = x1i - x3r;
        a[j + 14] = wk3r * x0r - wk3i * x0i;
        a[j + 15] = wk3r * x0i + wk3i * x0r;
    }
}

void
SpectrumBasedAlgorithm::cftmdl(int32 n, int32 l, double* a, double* w)
{
    int32 j, j1, j2, j3, k, k1, k2, m, m2;
    double wk1r, wk1i, wk2r, wk2i, wk3r, wk3i;
    double x0r, x0i, x1r, x1i, x2r, x2i, x3r, x3i;

    m = l << 2;

    for (j = 0; j < l; j += 2)
    {
        j1 = j + l;
        j2 = j1 + l;
        j3 = j2 + l;
        x0r = a[j] + a[j1];
        x0i = a[j + 1] + a[j1 + 1];
        x1r = a[j] - a[j1];
        x1i = a[j + 1] - a[j1 + 1];
        x2r = a[j2] + a[j3];
        x2i = a[j2 + 1] + a[j3 + 1];
        x3r = a[j2] - a[j3];
        x3i = a[j2 + 1] - a[j3 + 1];
        a[j] = x0r + x2r;
        a[j + 1] = x0i + x2i;
        a[j2] = x0r - x2r;
        a[j2 + 1] = x0i - x2i;
        a[j1] = x1r - x3i;
        a[j1 + 1] = x1i + x3r;
        a[j3] = x1r + x3i;
        a[j3 + 1] = x1i - x3r;
    }

    wk1r = w[2];

    for (j = m; j < l + m; j += 2)
    {
        j1 = j + l;
        j2 = j1 + l;
        j3 = j2 + l;
        x0r = a[j] + a[j1];
        x0i = a[j + 1] + a[j1 + 1];
        x1r = a[j] - a[j1];
        x1i = a[j + 1] - a[j1 + 1];
        x2r = a[j2] + a[j3];
        x2i = a[j2 + 1] + a[j3 + 1];
        x3r = a[j2] - a[j3];
        x3i = a[j2 + 1] - a[j3 + 1];
        a[j] = x0r + x2r;
        a[j + 1] = x0i + x2i;
        a[j2] = x2i - x0i;
        a[j2 + 1] = x0r - x2r;
        x0r = x1r - x3i;
        x0i = x1i + x3r;
        a[j1] = wk1r * (x0r - x0i);
        a[j1 + 1] = wk1r * (x0r + x0i);
        x0r = x3i + x1r;
        x0i = x3r - x1i;
        a[j3] = wk1r * (x0i - x0r);
        a[j3 + 1] = wk1r * (x0i + x0r);
    }

    k1 = 0;
    m2 = 2 * m;

    for (k = m2; k < n; k += m2)
    {
        k1 += 2;
        k2 = 2 * k1;
        wk2r = w[k1];
        wk2i = w[k1 + 1];
        wk1r = w[k2];
        wk1i = w[k2 + 1];
        wk3r = wk1r - 2 * wk2i * wk1i;
        wk3i = 2 * wk2i * wk1r - wk1i;

        for (j = k; j < l + k; j += 2)
        {
            j1 = j + l;
            j2 = j1 + l;
            j3 = j2 + l;
            x0r = a[j] + a[j1];
            x0i = a[j + 1] + a[j1 + 1];
            x1r = a[j] - a[j1];
            x1i = a[j + 1] - a[j1 + 1];
            x2r = a[j2] + a[j3];
            x2i = a[j2 + 1] + a[j3 + 1];
            x3r = a[j2] - a[j3];
            x3i = a[j2 + 1] - a[j3 + 1];
            a[j] = x0r + x2r;
            a[j + 1] = x0i + x2i;
            x0r -= x2r;
            x0i -= x2i;
            a[j2] = wk2r * x0r - wk2i * x0i;
            a[j2 + 1] = wk2r * x0i + wk2i * x0r;
            x0r = x1r - x3i;
            x0i = x1i + x3r;
            a[j1] = wk1r * x0r - wk1i * x0i;
            a[j1 + 1] = wk1r * x0i + wk1i * x0r;
            x0r = x1r + x3i;
            x0i = x1i - x3r;
            a[j3] = wk3r * x0r - wk3i * x0i;
            a[j3 + 1] = wk3r * x0i + wk3i * x0r;
        }

        wk1r = w[k2 + 2];
        wk1i = w[k2 + 3];
        wk3r = wk1r - 2 * wk2r * wk1i;
        wk3i = 2 * wk2r * wk1r - wk1i;

        for (j = k + m; j < l + (k + m); j += 2)
        {
            j1 = j + l;
            j2 = j1 + l;
            j3 = j2 + l;
            x0r = a[j] + a[j1];
            x0i = a[j + 1] + a[j1 + 1];
            x1r = a[j] - a[j1];
            x1i = a[j + 1] - a[j1 + 1];
            x2r = a[j2] + a[j3];
            x2i = a[j2 + 1] + a[j3 + 1];
            x3r = a[j2] - a[j3];
            x3i = a[j2 + 1] - a[j3 + 1];
            a[j] = x0r + x2r;
            a[j + 1] = x0i + x2i;
            x0r -= x2r;
            x0i -= x2i;
            a[j2] = -wk2i * x0r - wk2r * x0i;
            a[j2 + 1] = -wk2i * x0i + wk2r * x0r;
            x0r = x1r - x3i;
            x0i = x1i + x3r;
            a[j1] = wk1r * x0r - wk1i * x0i;
            a[j1 + 1] = wk1r * x0i + wk1i * x0r;
            x0r = x1r + x3i;
            x0i = x1i - x3r;
            a[j3] = wk3r * x0r - wk3i * x0i;
            a[j3 + 1] = wk3r * x0i + wk3i * x0r;
        }
    }
}

NB_Error
CreateSpeechProcessor(NB_Context* context,
                      SpeechProcessorAlgorithm algorithm,
                      uint32 bitsPerSample,
                      uint32 sampleRate,
                      nb_boolean bigEndian,
                      SpeechProcessorInterface** speechProcessor
                      )
{
    NB_Error err = NE_NOSUPPORT;

    if (!context || !speechProcessor)
    {
        return NE_BADDATA;
    }

    SpeechProcessor* speechDetector = NULL;

    switch (algorithm)
    {
        case SPA_Amplitude:
        case SPA_Unknown:
        {
            speechDetector = new AmplitudeBasedAlgorithm(DEFAULT_SN_AMPLITUDE_THRESHOLD, DEFAULT_SN_AMPLITUDE_CONTINUE_THRESHOLD);
            break;
        }

        case SPA_Spectrum:
        {
            speechDetector = new SpectrumBasedAlgorithm(DEFAULT_SN_SPECTRUM_THRESHOLD);
            break;
        }

        default:
            break;
    }

    if (speechDetector)
    {
        err = speechDetector->Init(context, bitsPerSample, sampleRate, bigEndian, DEFAULT_NOISE_DETECTION_INTERVAL, DEFAULT_SPEECH_DECLARATION_THRESHOLD);
        if (err == NE_OK)
        {
            *speechProcessor = speechDetector;
            speechDetector = NULL;
        }
        else
        {
            delete speechDetector;
            speechDetector = NULL;
        }
    }

    return err;
}

} //abspeechprocessor

/*! @} */
