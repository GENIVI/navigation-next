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

/*
 * (C) Copyright 2012 by TeleCommunication Systems, Inc.
 *
 * The information contained herein is confidential, proprietary
 * to TeleCommunication Systems, Inc., and considered a trade secret
 * as defined in section 499C of the penal code of the State of
 * California. Use of this information by anyone other than
 * authorized employees of TeleCommunication Systems is granted only
 * under a written non-disclosure agreement, expressly prescribing
 * the scope and manner of such use.
 *
 */


// stdafx.cpp : source file that includes just the standard includes
// nbgm_test.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"
#include "gpsprovider.h"

#define BUFFER_GRANULARITY 256
GPSLocationProvider::GPSLocationProvider()
{
    m_GPS=NULL;
    Reset();
}

GPSLocationProvider::~GPSLocationProvider()
{
    Reset();
}

void GPSLocationProvider::Reset()
{
    if(m_GPS!=NULL)
    {
        delete m_GPS;
    }
    m_GPS = NULL;
    m_Count = 0;
    m_MaxCount = 0;
    m_CurrentIndex = 0;
}

void GPSLocationProvider::CreateFromGPSFile(CString fileName)
{
    Reset();
    CStdioFile file;
    CFileException except; 
    file.Open(fileName, CFile::modeRead, &except);
    CString line;
    int coordIndex = 0;
    GPSLocation loc ={0};
    while(file.ReadString(line))
    {
        float coord = (float)_tstof(line);
        if(coordIndex==0)
        {
            loc.x = coord;
            coordIndex = 1;
        }
        else if(coordIndex == 1)
        {
            loc.y = coord;
            coordIndex = 2;
        }
        else if(coordIndex == 2)
        {
            loc.z = coord;
            coordIndex = 3;
        }
        else if(coordIndex == 3)
        {
            loc.heading = coord;
            coordIndex = 4;
        }
        else if(coordIndex == 4)
        {
            loc.speed = coord;
            AppendNewGPSLocation(&loc);
            coordIndex = 0;
        }
    }
    file.Close();
}

GPSLocation* GPSLocationProvider::GetFirstGPSLocation()
{
    if(m_Count <=0)
    {
        return NULL;
    }
    return &m_GPS[0];
}

GPSLocation* GPSLocationProvider::GetNextGPSLocation()
{
    if(m_Count <=0)
    {
        return NULL;
    }
    if(m_CurrentIndex>=m_Count)
    {
        m_CurrentIndex = 0;
    }
    return &m_GPS[m_CurrentIndex++];
}

GPSLocation* GPSLocationProvider::GetLastGPSLocation()
{
    if(m_Count <=0)
    {
        return NULL;
    }
    return &m_GPS[m_Count-1];
}

void GPSLocationProvider::AppendNewGPSLocation(GPSLocation* loc)
{
    if(m_Count>=m_MaxCount)
    {
        GPSLocation* buffer = new GPSLocation[m_MaxCount+BUFFER_GRANULARITY];
        memset(buffer, 0, sizeof(GPSLocation)*(m_MaxCount+BUFFER_GRANULARITY));
        if(m_GPS!=NULL)
        {
            memcpy(buffer, m_GPS, sizeof(GPSLocation)*m_MaxCount);   
        }
        m_MaxCount+=BUFFER_GRANULARITY;
        delete m_GPS;
        m_GPS = buffer;
    }
    m_GPS[m_Count++] = *loc;
}
