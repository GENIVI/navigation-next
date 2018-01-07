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

 @file     tcpconnection.m
 @date     6/23/2014
 @defgroup PAL_NET PAL Network I/O Functions

 Platform-independent network I/O API.
 */
/*
 (C) Copyright 2014 by TeleCommunication Systems, Inc.

 The information contained herein is confidential, proprietary
 to TeleCommunication Systems, Inc., and considered a trade secret
 as defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of TeleCommunication Systems is granted only
 under a written non-disclosure agreement, expressly prescribing
 the scope and manner of such use.

 ---------------------------------------------------------------------------*/

/*! @{ */

#include "palstdlib.h"
#import "tcpconnection.h"

#include "palfile.h"

@interface TCPConnection () <NSStreamDelegate>
{
    // Status Vars.
    BOOL _inputStreamOpened;
    BOOL _outputStreamOpened;
}

@property (nonatomic, retain) NSString* hostname;
@property (nonatomic, assign) NSInteger port;
@property (nonatomic, retain) NSInputStream* inputStream;
@property (nonatomic, retain) NSOutputStream* outputStream;
@property (nonatomic, retain) NSMutableArray* sendingQueue;
@property (nonatomic, retain) NSMutableArray* recievingQueue;

- (void)stream:(NSStream*)aStream handleEvent:(NSStreamEvent)eventCode;

- (void)streamEventOpenCompleted:(NSStream*)aStream;
- (void)streamEventHasBytesAvailable:(NSStream*)aStream;
- (void)streamEventHasSpaceAvailable:(NSStream*)aStream;
- (void)streamEventErrorOccurred:(NSStream*)aStream;
- (void)streamEventEndEncountered:(NSStream*)aStream;

- (void)dealloc;

@end

@implementation TCPConnection

@synthesize userData;
@synthesize connectionStatusCallback;
@synthesize dataSentCallback;
@synthesize dataReceivedCallback;
@synthesize dnsResultCallback;

@synthesize hostname;
@synthesize port;
@synthesize inputStream;
@synthesize outputStream;
@synthesize sendingQueue;
@synthesize recievingQueue;

- (instancetype)init
{
    self = [super init];
    if (self)
    {
        _inputStreamOpened = NO;
        _outputStreamOpened = NO;
    }

    NSLog(@"TCP Connection is initialized.");

    return self;
}

- (PAL_Error)connectToHost:(NSString*)hostName port:(NSInteger)portNumber
{
    self.hostname = hostName;
    self.port = portNumber;

    CFReadStreamRef readStreamRef;
    CFWriteStreamRef writeStreamRef;
    CFStreamCreatePairWithSocketToHost(NULL, (__bridge CFStringRef)self.hostname, (UInt32)portNumber, &readStreamRef, &writeStreamRef);

    self.inputStream = (__bridge NSInputStream*)readStreamRef;
    self.outputStream = (__bridge NSOutputStream*)writeStreamRef;

    [self.inputStream setDelegate:self];
    [self.outputStream setDelegate:self];

    [self.inputStream scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
    [self.outputStream scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];

    [self.inputStream open];
    [self.outputStream open];

    if (self.connectionStatusCallback)
    {
        self.connectionStatusCallback(self.userData, PNCS_Connecting);
    }

    NSLog(@"TCP Connection is starting to connect.");

    return PAL_Ok;
}

- (PAL_Error)sendData:(NSData*)data
{
    PAL_Error err = PAL_Ok;

    if (self.outputStream == nil)
    {
        err = PAL_ErrNoInit;
        return err;
    }

    NSStreamStatus status = [self.outputStream streamStatus];

    if (status == NSStreamStatusOpen)
    {
        if ([self.outputStream hasSpaceAvailable])
        {
            [self.outputStream write:data.bytes maxLength:data.length];
            NSLog(@"%ld bytes sent directly on TCP connection, no delay.", (unsigned long)data.length);
            if (self.dataSentCallback)
            {
                self.dataSentCallback(self.userData, data.bytes, (uint32)data.length);
            }
        }
        else
        {
            [self.sendingQueue addObject:data];
        }
    }
    else
    {
        err = PAL_ErrNoInit;
    }

    return err;
}

- (void)close
{
    [self.inputStream removeFromRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
    [self.outputStream removeFromRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];

    [self.inputStream close];
    [self.outputStream close];

    [self.inputStream setDelegate:nil];
    [self.outputStream setDelegate:nil];

    self.inputStream = nil;
    self.outputStream = nil;

    self.hostname = nil;
    self.port = 0;
    self.sendingQueue = nil;
    self.recievingQueue = nil;

    _inputStreamOpened = NO;
    _outputStreamOpened = NO;

    if (self.connectionStatusCallback)
    {
        self.connectionStatusCallback(self.userData, PNCS_Closed);
    }

    NSLog(@"TCP Connection is closed.");
}

- (void)stream:(NSStream*)aStream handleEvent:(NSStreamEvent)eventCode
{
    switch (eventCode)
    {
        case NSStreamEventOpenCompleted:
        {
            [self streamEventOpenCompleted:aStream];
            break;
        }

        case NSStreamEventHasBytesAvailable:
        {
            [self streamEventHasBytesAvailable:aStream];
            break;
        }

        case NSStreamEventHasSpaceAvailable:
        {
            [self streamEventHasSpaceAvailable:aStream];
            break;
        }

        case NSStreamEventErrorOccurred:
        {
            [self streamEventErrorOccurred:aStream];
            break;
        }

        case NSStreamEventEndEncountered:
        {
            [self streamEventEndEncountered:aStream];
            break;
        }

        default:
        {
            nsl_assert(0); // Should never be here.
            break;
        }
    }
}

- (void)streamEventOpenCompleted:(NSStream*)aStream
{
    if (aStream == self.outputStream && self.sendingQueue == nil)
    {
        self.sendingQueue = [[NSMutableArray alloc] init];
        _outputStreamOpened = YES;
    }

    if (aStream == self.inputStream && self.recievingQueue == nil)
    {
        self.recievingQueue = [[NSMutableArray alloc] init];
        _inputStreamOpened = YES;
    }

    if (self.connectionStatusCallback && _inputStreamOpened && _outputStreamOpened)
    {
        self.connectionStatusCallback(self.userData, PNCS_Connected);
    }
}

- (void)streamEventHasBytesAvailable:(NSStream*)aStream
{
#define BUFSIZE 32768
    uint8_t buf[BUFSIZE] = {0};
    NSInteger bytesRead = [self.inputStream read:buf maxLength:BUFSIZE];
    if (bytesRead > 0) // A positive number indicates the number of bytes read.
    {
        NSMutableData* data = [[NSMutableData alloc] initWithBytes:buf length:bytesRead];
        NSLog(@"%ld bytes read on TCP connection.", (unsigned long)data.length);
        [self.recievingQueue addObject:data];
        if (self.dataReceivedCallback)
        {
            self.dataReceivedCallback(self.userData, data.bytes, (uint32)data.length);
        }
        [data release];
    }
    else if (bytesRead == -1) // A negative number means that the operation failed.
    {
        if (self.connectionStatusCallback)
        {
            self.connectionStatusCallback(self.userData, PNCS_Error);
        }
    }
    else if (bytesRead == 0) // 0 indicates that the end of the buffer was reached.
    {
        if (self.dataReceivedCallback)
        {
            self.dataReceivedCallback(self.userData, NULL, 0);
        }
    }
    else
    {
        // Never be here.
        nsl_assert(0);
    }
#undef BUFSIZE
}

- (void)streamEventHasSpaceAvailable:(NSStream*)aStream
{
    if (self.sendingQueue.count > 0)
    {
        NSData* data = [self.sendingQueue objectAtIndex:0];
        NSInteger bytesWritten = [self.outputStream write:data.bytes maxLength:data.length];
        NSLog(@"%ld bytes written on TCP connection.", (long)bytesWritten);
        if (bytesWritten == -1)
        {
            if (self.connectionStatusCallback)
            {
                self.connectionStatusCallback(self.userData, PNCS_Error);
            }
        }
        else
        {
            if (self.dataSentCallback)
            {
                self.dataSentCallback(self.userData, (const byte*)data.bytes, (uint32)data.length);
            }
            [self.sendingQueue removeObjectAtIndex:0];
        }
    }
}

- (void)streamEventErrorOccurred:(NSStream*)aStream
{
    NSError* error = [aStream streamError];
    NSLog(@"%@", [error description]);

    if (aStream == self.outputStream && [error.domain compare:(__bridge NSString*)kCFErrorDomainCFNetwork] == NSOrderedSame)
    {
        if (error.code == kCFHostErrorUnknown)
        {
            if (self.connectionStatusCallback)
            {
                self.connectionStatusCallback(self.userData, PNCS_Failed);
            }
        }
        else if (error.code == kCFHostErrorHostNotFound)
        {
            if (self.dnsResultCallback)
            {
                self.dnsResultCallback(self.userData, PAL_ErrNetDnsUnknownHostName, [self.hostname UTF8String], NULL, 0);
            }
        }
        else
        {
            if (self.connectionStatusCallback)
            {
                self.connectionStatusCallback(self.userData, PNCS_Error);
            }
        }
    }
    else if ([error.domain compare:NSPOSIXErrorDomain] == NSOrderedSame)
    {
        if (self.connectionStatusCallback)
        {
            self.connectionStatusCallback(self.userData, PNCS_Error);
        }
    }
    else
    {
        if (self.connectionStatusCallback)
        {
            self.connectionStatusCallback(self.userData, PNCS_Error);
        }
    }
}

- (void)streamEventEndEncountered:(NSStream*)aStream
{
    // TODO: Should we close streams here?
}

- (void)dealloc
{
    [self close];
    
    [super dealloc];
    NSLog(@"TCPConnection is about to be destroyed.");
}

@end

/*! @} */
