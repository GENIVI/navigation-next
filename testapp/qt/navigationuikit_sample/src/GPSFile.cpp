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
 @file         GPSFile.m
 @defgroup     NavkitExample
 */
/*
 (C) Copyright 2014 by TeleCommunications Systems, Inc.

 The information contained herein is confidential, proprietary to
 TeleCommunication Systems, Inc., and considered a trade secret as defined
 in section 499C of the penal code of the State of California. Use of this
 information by anyone other than authorized employees of TeleCommunication
 Systems is granted only under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.
 --------------------------------------------------------------------------*/

/*! @{ */

#include "stdlib.h"
#include "stdio.h"
#include "GPSFile.h"

using namespace locationtoolkit;

/*! Convert the GPSFileRecord data into GpsLocation
 This method is used to convert the GPSFileRecord in to GPSFile Location,
 which is then used for Navigation as a new fix
 @return none
 */
void GPSInfo2GPSFix(GPSFileRecord* pos, Place* place)
{
    MapLocation location;
    location.center.latitude = pos->dwLat * 0.00000536441803;
    location.center.longitude = pos->dwLon * 0.00000536441803;
    place->SetLocation(location);
}

void GetDefaultDestinationAndOrigin(const std::string& gpsFile, Place* destination, Place* origin)
{
    unsigned char* fileBuffer = NULL;
    size_t fileSize = 0;
    FILE* pfile = fopen(gpsFile.c_str(), "rb");

    if (pfile == NULL)
    {
        return;
    }
    // obtain file size:
    fseek (pfile , 0 , SEEK_END);
    fileSize = ftell (pfile);
    rewind (pfile);

    // allocate memory to contain the whole file:
    fileBuffer = (unsigned char*) malloc (sizeof(unsigned char)*fileSize);
    if (fileBuffer == NULL)
    {
        return;
    }

    // copy the file into the buffer:
    if (fread (fileBuffer,1,fileSize,pfile) != fileSize)
    {
        return;
    }
    if (fileSize > sizeof(GPSFileRecord))
    {
        GPSFileRecord* allRecord = (GPSFileRecord*)fileBuffer;
        GPSInfo2GPSFix(allRecord, origin);
        size_t recordNum = fileSize / sizeof(GPSFileRecord);
        allRecord += recordNum - 1;
        GPSInfo2GPSFix(allRecord, destination);
    }
    //close the file
    fclose (pfile);
    if (fileBuffer)
    {
        free(fileBuffer);
    }
}


/*! @} */
