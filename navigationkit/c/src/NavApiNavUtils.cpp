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
 * (C) Copyright 2013 by TeleCommunication Systems, Inc.
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

#include "NavApiNavUtils.h"
extern "C"
{
#include "nbcontextprotected.h"
#include "nbspatial.h"
#include "nbcontextaccess.h"
#include "abpalaudio.h"
#include "palfile.h"
#include "paldisplay.h"
}

namespace nbnav
{

using namespace std;

static const string AUDIO_FILESET_SUFFIX = "_audio";

Image NavUtils::GetImage(NB_Context* nbContext, string imageName)
{
    if (!nbContext)
    {
        return Image(NULL, 0, PNG);
    }

    string workPath(PAL_FileGetWorkPath());
    string commonPath(workPath + PATH_DELIMITER + "NK_common" + PATH_DELIMITER + "routing_icons" + PATH_DELIMITER);
    string fullName(commonPath + imageName + ".png");
    unsigned char* pBuf = NULL;
    uint32 size = 0;

    if (PAL_FileLoadFile(NB_ContextGetPal(nbContext), fullName.c_str(), &pBuf, &size) == PAL_Ok)
    {
        return Image(pBuf, size, PNG);
    }
    return Image(NULL, 0, PNG);
}

BinaryBuffer NavUtils::GetNavigationVoice(NB_Context* nbContext, string voiceName)
{
    if (!nbContext)
    {
        return BinaryBuffer(NULL, 0);
    }

    string workPath(PAL_FileGetWorkPath());
    string commonPath(workPath + PATH_DELIMITER + "NK_" + PAL_GetLocale(NB_ContextGetPal(nbContext)) + PATH_DELIMITER);
    string fullName(commonPath + voiceName + ".aac");
    unsigned char* pBuf = NULL;
    uint32 size = 0;

    if (PAL_FileLoadFile(NB_ContextGetPal(nbContext), fullName.c_str(), &pBuf, &size) == PAL_Ok)
    {
        return BinaryBuffer(pBuf, size);
    }
    return BinaryBuffer(NULL, 0);
}

Image NavUtils::GetRoutingIconImage(NB_Context* nbContext, string routingIconName)
{
    double density = PAL_DisplayGetDensity();
    string ssize = "medium";
    if (density > 1)
    {
        ssize = "large";
    }
    else if (density < 1)
    {
        ssize = "small";
    }
    string name = ssize + "-" + routingIconName + ".png";

    return NavUtils::GetImage(nbContext, name);
}
    
std::string NavUtils::ConvertFormatStringToPlainString(std::string oriString)
{
    char* tmpBuf = new char[oriString.length()+1];
    int j = 0;
    int markedCount = 0;
    for(size_t i = 0; i< oriString.length(); i++)
    {
        if(oriString[i] == '<')
        {
            markedCount++;
            continue;
        }
        else if (oriString[i] == '>')
        {
            markedCount--;
            continue;
        }
        if(markedCount <= 0)
        {
            tmpBuf[j] = oriString[i];
            j++;
        }
    }
    tmpBuf[j] = '\0';
    std::string ret = std::string(tmpBuf);
    delete[] tmpBuf;
    return ret;
}

NKProununceDateBase NavUtils::GetAvailableAudioStyle(const string& workpath)
{
    NKProununceDateBase proununceDatabase;
    if (workpath.length() == 0)
    {
        return proununceDatabase;
    }
    string path = (workpath[workpath.length()-1] == (byte)PATH_DELIMITER)?
                    workpath.substr(0,workpath.length()-1):workpath;
    PAL_FileEnum* fileEnum = NULL;
    PAL_FileEnumInfo info;
    memset(&info, 0, sizeof(info));
    PAL_Instance* pal = (PAL_Instance*)0x01;   // pal is useless but could not be NULL.
    if (PAL_FileEnumerateCreate(pal, path.c_str(), FALSE, &fileEnum) == PAL_Ok)
    {
        while(PAL_FileEnumerateNext(fileEnum, &info) == PAL_Ok)
        {
            if(PAL_FileIsDirectory(pal, (path + PATH_DELIMITER + (string)info.filename).c_str()))
            {
                string subPath(info.filename);
                if (subPath.find(AUDIO_FILESET_SUFFIX) != string::npos)
                {
                    /*! get language */
                    string language = subPath.substr(subPath.find('_'),subPath.find_last_of('_'));
                    /*! get voicestyle */
                    PAL_FileEnum* subfileEnum = NULL;
                    PAL_FileEnumInfo subInfo = {0};
                    if (PAL_FileEnumerateCreate(pal,(path + PATH_DELIMITER + subPath).c_str(),
                                                FALSE, &subfileEnum) == PAL_Ok)
                    {
                        while(PAL_FileEnumerateNext(subfileEnum, &subInfo) == PAL_Ok)
                        {
                            if(PAL_FileIsDirectory(pal,
                                                   (path + PATH_DELIMITER + subPath + PATH_DELIMITER
                                                    + (string)subInfo.filename).c_str()))
                            {
                                string voiceStyle(subInfo.filename);
                                string value = voiceStyle.substr(0,voiceStyle.find('-'));
                                proununceDatabase.insert( make_pair(language,make_pair(value,voiceStyle)));
                            }
                        }
                        PAL_FileEnumerateDestroy(subfileEnum);
                    }
                }
            }
        }
        PAL_FileEnumerateDestroy(fileEnum);
    }
    return proununceDatabase;
}

void NavUtils::MercatorForward(double latitude, double longitude, double* xMercator, double* yMercator)
{
    NB_SpatialConvertLatLongToMercator(latitude, longitude, xMercator, yMercator);
}

void NavUtils::MercatorReverse(double xMercator, double yMercator,double* latitude, double* longitude)
{
    NB_SpatialConvertMercatorToLatLong(xMercator, yMercator, latitude, longitude);
}

double NavUtils::CalculateDistance(double startLatitude,
                                   double startLongitude,
                                   double endLatitude,
                                   double endLongitude,
                                   double* heading)
{
    //return distance in meters.
    return NB_SpatialGetLineOfSightDistance(startLatitude,
                                            startLongitude,
                                            endLatitude,
                                            endLongitude,
                                            heading);
}

}
