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

    @file AQPlayerItem.m
    @date 11-3-1
    @defgroup playeritem.m in abpal
*/
/*
    See file description in header file.

    (C) Copyright 2011 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret
    as defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly prescribing
    the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

/* all the file’s code goes here */

#import "palstdlib.h"
#import "playeritem.h"
#import "palaudioplayer.h"

@implementation PlayerItem

- (id)init {
    if (self = [super init]) {
    }
    return self;
}

- (void)dealloc {
    if (m_PacketDescs != NULL) {
        nsl_free(m_PacketDescs);
    }
    if (m_AudioFile != NULL) {
        AudioFileClose(m_AudioFile);
    }
    [super dealloc];
}

- (NSInteger)setFile:(NSString*)filename {
    CFURLRef audioFileURL = CFURLCreateFromFileSystemRepresentation(NULL, (const UInt8 *)[filename UTF8String],
                                                                    [filename length], false);
    // Opening an Audio File
    OSStatus result = AudioFileOpenURL(audioFileURL, kAudioFileReadPermission, 0, &m_AudioFile);
    CFRelease(audioFileURL);
    if (result != 0 || m_AudioFile == NULL) {
        return result;
    }

    UInt32 maxPacketSize = 0;
    UInt32 propertySize = sizeof (maxPacketSize);
    result = AudioFileGetProperty(m_AudioFile, kAudioFilePropertyPacketSizeUpperBound,
                                  &propertySize, &maxPacketSize);
    if (result != 0) {
        return result;
    }

    Float64 estimatedDuration = 0.0;
    propertySize = sizeof(estimatedDuration);
    result = AudioFileGetProperty(m_AudioFile, kAudioFilePropertyEstimatedDuration,
                                  &propertySize, &estimatedDuration);

    // Use default duration in half second
    if ((result != 0) || (estimatedDuration == 0.0))
    {
        estimatedDuration = 0.5;
    }
    estimatedDuration = estimatedDuration * 40;

    AudioStreamBasicDescription format;
    result = [self getFileFormat:&format];
    if (result != 0) {
        return result;
    }

    PalAudioPlayer_DeriveBufferSize(&format, maxPacketSize, estimatedDuration, &m_BufferByteSize, &m_NumPacketsToRead);

    // Allocating Memory for a Packet Descriptions Array
    bool formatVBR = (format.mBytesPerPacket == 0 || format.mFramesPerPacket == 0);
    if (formatVBR) {
        m_PacketDescs = (AudioStreamPacketDescription*)nsl_malloc(m_NumPacketsToRead * sizeof (AudioStreamPacketDescription));
        memset(m_PacketDescs, 0, m_NumPacketsToRead * sizeof (AudioStreamPacketDescription));
    }
    else {
        m_PacketDescs = NULL;
    }

    return result;
}

- (NSInteger)getFileFormat:(AudioStreamBasicDescription*)dataFormat {
    UInt32 dataFormatSize = sizeof(AudioStreamBasicDescription);
    OSStatus result = AudioFileGetProperty(m_AudioFile, kAudioFilePropertyDataFormat,
                                           &dataFormatSize, dataFormat);
    return result;
}

- (NSInteger)prepare:(AudioQueueRef)inAQ {
    OSStatus result = 0;
    // Set a Magic Cookie for a Playback Audio Queue
    UInt32 m_CookieSize = sizeof (UInt32);
    char* m_MagicCookie = NULL;
    bool couldNotGetProperty = AudioFileGetPropertyInfo(m_AudioFile, kAudioFilePropertyMagicCookieData,
                                                        &m_CookieSize, NULL);
    if (!couldNotGetProperty && m_CookieSize) {
        m_MagicCookie = (char*) malloc(m_CookieSize);
        result = AudioFileGetProperty(m_AudioFile, kAudioFilePropertyMagicCookieData,
                                      &m_CookieSize, m_MagicCookie);
        if (result == 0)
        {
            result = AudioQueueSetProperty(inAQ, kAudioQueueProperty_MagicCookie,
                                           m_MagicCookie, m_CookieSize);
        }
    }
    free(m_MagicCookie);
    if (result != 0)
    {
        return result;
    }

    result = AudioQueueAllocateBuffer(inAQ, m_BufferByteSize, &m_Buffers);
    if (result != 0)
    {
        return result;
    }

    [self handleOutputBuffer:inAQ andBuffer:m_Buffers];
    return result;
}

- (void)handleOutputBuffer:(AudioQueueRef)inAQ andBuffer:(AudioQueueBufferRef)inBuffer {
    UInt32 numBytesReadFromFile = 0;
    UInt32 numPackets = m_NumPacketsToRead;
    OSStatus result = AudioFileReadPackets(m_AudioFile, false, &numBytesReadFromFile,
                                           m_PacketDescs, 0, &numPackets, inBuffer->mAudioData);

    if ((result == 0) && (numPackets > 0)) {
        inBuffer->mAudioDataByteSize = numBytesReadFromFile;
        AudioQueueEnqueueBuffer(inAQ, inBuffer, (m_PacketDescs ? numPackets : 0), m_PacketDescs);
    }
}

@end

/*! @} */
