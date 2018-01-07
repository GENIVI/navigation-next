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

 @file gpsconfiguration.m
 @date 9/13/10

 GPS configuration

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
#import "gpsconfiguration.h"


@implementation GpsConfiguration

@synthesize providerType;
@synthesize fileName;

-(id) initWithConfigCount:(uint32)configCount gpsConfigPointer:(const ABPAL_GpsConfig *) config
{
    if (self = [super init])
    {
        providerType = GpsProviderTypeGpsId;

        for (uint32 i=0; i<configCount; ++i)
        {
            //const ABPAL_GpsConfig* config = config;
            [self parseName:config->name parseValue:config->value];
        }

    }

    return self;
}

-(void) parseName:(const char *)name parseValue:(const char *) value
{
    const char* KProviderType = "providertype";
    const char* KGpsFilename = "gpsfilename";

    if (strcmp(name, KGpsFilename) == 0)
    {
        providerType = GpsProviderTypeEmu;
        delete [] fileName;
        fileName = NULL;
        int len = strlen(value);
        fileName = new char[len+1];
        memset(fileName, 0, len+1);
        memcpy(fileName, value, len);
    }
    else if (strcmp(name, KProviderType) == 0)
    {
        [self parseType:value];
    }
}

-(void) parseType:(const char *)type
{
    const char* KGpsTypeEmu = "emu";
    const char* KGpsTypeGps = "gpsid";

    if (strcmp(type, KGpsTypeEmu) == 0)
    {
        providerType = GpsProviderTypeEmu;
    }
    else if (strcmp(type, KGpsTypeGps) == 0)
    {
        providerType = GpsProviderTypeGpsId;
    }
}

-(void) dealloc
{
    delete []fileName;
    [super dealloc];
}

@end
