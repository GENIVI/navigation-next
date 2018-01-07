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
 
    @file HttpsConnection.h
    @date 11-1-28
    @defgroup HttpsConnection.h in NBPal

    See header file for description
 
*/
/*
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

#import <Foundation/Foundation.h>
#import "palnet.h"

/*! Callback to response status of HTTPS */
typedef void
(*HttpsResponseStatusCallback) (void* userData,         /*!< User data */
                                unsigned int status     /*!< Status code to response */
                                );

@interface HttpsConnection : NSObject
{
    NSString*                      host;
    NSInteger                      port;
    BOOL                           useHttps;
    BOOL                           usePost;
    NSURLConnection*               urlConnection;
    const byte*                    postData;
    NSInteger                      postDataLength;

    void*                               userData;
    PAL_NetConnectionStatusCallback     connectionStatusCallback;
    PAL_NetDataSentCallback             dataSentCallback;
    PAL_NetHttpDataReceivedCallback     httpDataReceivedCallback;
    HttpsResponseStatusCallback         httpResponseStatusCallback;

    // @todo: Support request and response headers. PAL_NetHttpResponseHeadersCallback
}

@property (nonatomic, assign) void* userData;
@property (nonatomic, assign) PAL_NetConnectionStatusCallback connectionStatusCallback;
@property (nonatomic, assign) PAL_NetDataSentCallback dataSentCallback;
@property (nonatomic, assign) PAL_NetHttpDataReceivedCallback httpDataReceivedCallback;
@property (nonatomic, assign) HttpsResponseStatusCallback httpResponseStatusCallback;

- (id) initWithUseHttps:(BOOL)use;
- (PAL_Error) connectToHost:(NSString*)hostname andPort:(NSInteger)portnumber;
- (PAL_Error) sendData:(const byte*)data
            withLength:(NSInteger)length
               andVerb:(NSString*)verb
             andObject:(NSString*)object
         andAcceptType:(NSString*)acceptType
  andAdditionalHeaders:(NSString*)additionalHeaders;
- (void) close;

@end

/*! @} */
