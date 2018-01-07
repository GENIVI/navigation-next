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

    @file HttpsConnection.m
    @date 11-1-28
    @defgroup HttpsConnection.m in NBPal
*/
/*
    See file description in header file.

    (C) Copyright 2010 by TeleCommunication Systems, Inc.

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

#import "HttpsConnection.h"

static NSString* kPostMethod = @"POST";
static NSString* headersSplit = @"\r\n";
static NSString* headerValueSplit = @":";

@implementation HttpsConnection

@synthesize userData;
@synthesize connectionStatusCallback;
@synthesize dataSentCallback;
@synthesize httpDataReceivedCallback;
@synthesize httpResponseStatusCallback;

- (id) initWithUseHttps:(BOOL)use {
    if (self = [super init]) {
        useHttps = use;
    }
    return self;
}

- (void) dealloc {
    [self close];
    [host release];
    [super dealloc];
}

- (PAL_Error) connectToHost:(NSString*)hostname andPort:(NSInteger)portnumber {
    [host release];
    host = [hostname retain];
    port = portnumber;
    return PAL_Ok;
}

- (PAL_Error) sendData:(const byte*)data
            withLength:(NSInteger)length
               andVerb:(NSString*)verb
             andObject:(NSString*)object
         andAcceptType:(NSString*)acceptType
  andAdditionalHeaders:(NSString*)additionalHeaders
{
    PAL_Error result = PAL_ErrNetGeneralFailure;
    if (urlConnection == nil) {
        NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
        BOOL postMethod = NO;
        if (verb == nil || [verb caseInsensitiveCompare:kPostMethod] == NSOrderedSame) {
            postMethod = YES;
        }

        NSString* dataLength = nil;
        postData = NULL;
        postDataLength = 0;
        if (postMethod) {
            postData = data;
            postDataLength = length;
            dataLength = [NSString stringWithFormat:@"%ld", (long)length];
        }

        NSString* url = nil;
        if (useHttps) {
            url = [NSString stringWithFormat:@"https://%@:%ld", host, (long)port];
        }
        else {
            url = [NSString stringWithFormat:@"http://%@:%ld", host, (long)port];
        }
        if (object != nil && [object length] > 0) {
            if ([object characterAtIndex:0] == '/')
            {
                url = [NSString stringWithFormat:@"%@%@", url, object];
            }
            else
            {
                url = [NSString stringWithFormat:@"%@/%@", url, object];
            }
        }

        NSMutableURLRequest* request = [[NSMutableURLRequest alloc] init];
        [request setURL:[NSURL URLWithString:url]];
        if (acceptType != nil && [acceptType length] > 0) {
            [request setValue:acceptType forHTTPHeaderField:@"Accept"];
        }

        if(additionalHeaders != nil && [additionalHeaders length] != 0)
        {
            NSArray* headers = [additionalHeaders componentsSeparatedByString:headersSplit];
            for(NSString* header in headers)
            {
                NSArray* values = [header componentsSeparatedByString:headerValueSplit];
                //Avoid the illegal header value
                if([values count] != 2)
                {
                    continue;
                }
                [request setValue:[values objectAtIndex:1] forHTTPHeaderField:[values objectAtIndex:0]];
            }
        }

        if (postMethod) {
            [request setHTTPMethod:kPostMethod];
            [request setValue:dataLength forHTTPHeaderField:@"Content-Length"];
            NSData* d = [NSData dataWithBytes:data length:length];
            [request setHTTPBody:d];
        }

        urlConnection = [[NSURLConnection alloc] initWithRequest:request delegate:self];
        [request release];
        [urlConnection start];

        [pool release];
        result = PAL_Ok;
    }
    return result;
}

- (void) close {
    [urlConnection cancel];
    [urlConnection release];
    urlConnection = nil;
}

#pragma mark NSURLConnectionDelegate

- (void)connection:(NSURLConnection *)connection didReceiveResponse:(NSURLResponse *)response
{
    if ([response isKindOfClass:[NSHTTPURLResponse class]])
    {
        NSHTTPURLResponse* httpResponse = (NSHTTPURLResponse*)(response);
        if (httpResponseStatusCallback)
        {
            httpResponseStatusCallback(userData, (unsigned int) [httpResponse statusCode]);
        }
    }
}

- (void)connection:(NSURLConnection *)connection didReceiveData:(NSData *)data
{
    if (httpDataReceivedCallback) {
        httpDataReceivedCallback(userData, NULL, PAL_Ok, [data bytes], [data length]);
    }
}

- (void)connectionDidFinishLoading:(NSURLConnection *)connection
{
    [connection release];
    urlConnection = nil;
    if (httpDataReceivedCallback) {
        httpDataReceivedCallback(userData, NULL, PAL_Ok, NULL, 0);
    }
}

- (void)connection:(NSURLConnection *)connection didFailWithError:(NSError *)error
{
    [connection release];
    urlConnection = nil;
    if (connectionStatusCallback) {
        connectionStatusCallback(userData, PNCS_Failed);
    }
}

- (void)connection:(NSURLConnection *)connection
   didSendBodyData:(NSInteger)bytesWritten
 totalBytesWritten:(NSInteger)totalBytesWritten
totalBytesExpectedToWrite:(NSInteger)totalBytesExpectedToWrite
{
    if (dataSentCallback) {
        dataSentCallback(userData, postData, totalBytesWritten);
    }
}

@end

/*! @} */
