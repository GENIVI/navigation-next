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
 * (C) Copyright 2014 by TeleCommunication Systems, Inc.
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

/*
 * tilenameparser.cpp
 *
 *  Created on: May 13, 2013
 *      Author: btian
 */
#include "tilenameparser.h"
#include <string>
#include <vector>
#include <sstream>
#include "palmath.h"

using namespace std;

static PAL_Error ParseMobiusName(const std::string& fileName, NBMTileInfo& tileInfo);
static PAL_Error ParseCCCName(const std::string& fileName, NBMTileInfo& tileInfo);

void SplitString(std::string str, char split, std::vector<std::string>& result)
{
    int iPos = 0;
    int iNums = 0;
    std::string strTemp = str;
    std::string strRight;
    while (iPos != -1)
    {
        iPos = strTemp.find(split);
        if (iPos == -1)
        {
            break;
        }
        strRight = strTemp.substr(iPos + 1, str.length());
        strTemp = strRight;
        iNums++;
    }
    if (iNums == 0)
    {
        result.push_back(str);
        return;
    }
    strTemp = str;
    for (int i = 0; i < iNums; i++)
    {
        iPos = strTemp.find(split);
        std::string strLeft = strTemp.substr(0, iPos);
        strRight = strTemp.substr(iPos + 1, strTemp.length());
        strTemp = strRight;
        result.push_back(strLeft);
    }
    result.push_back(strTemp);
    return ;
}


//1. try to parse 09371125363DLM.nbm
static PAL_Error ParseMobiusName(const std::string& fileTitle, NBMTileInfo& tileInfo)
{
    PAL_Error err = PAL_ErrUnsupported;
    std::string fileType = fileTitle.substr(10, 4);
    if(fileType == "AREA")
    {
        tileInfo.layerType = DVA;
        tileInfo.labelType = DVA_LABEL;
        err = PAL_Ok;
    }
    else if(fileType == "ROAD")
    {
        tileInfo.layerType = DVR;
        tileInfo.labelType = DVR_LABEL;
        err = PAL_Ok;
    }
    else if(fileType == "3DLM" || fileType == "LM3D")
    {
        tileInfo.layerType = LM3D;
        tileInfo.labelType = DVA_LABEL;
        err = PAL_Ok;
    }
    else if(fileType == "3DUT")
    {
        tileInfo.layerType = B3D;
        tileInfo.labelType = DVA_LABEL;
        err = PAL_Ok;
    }
    else if(fileType == "DMAT" || fileType == "NMAT")
    {
        err = PAL_Ok;
    }
    else if(fileType == "LABE")
    {
        //tileInfo.layerType = POINT_LABEL;
        err = PAL_Ok;
    }
    else if(fileType == "BR2")
    {
        tileInfo.layerType = BR2;
        const char* p = fileTitle.c_str();
        std::string strX(p, 5);
        std::string strY(p+5, 5);
        int x, y;
        stringstream(strX)>>x;
        stringstream(strY)>>y;
        tileInfo.x = x;
        tileInfo.y = y;
        tileInfo.z = 2;
        return PAL_Ok;
    }
    else if(fileType == "BR3")
    {
        tileInfo.layerType = BR3;
        const char* p = fileTitle.c_str();
        std::string strX(p, 5);
        std::string strY(p+5, 5);
        int x, y;
        stringstream(strX)>>x;
        stringstream(strY)>>y;
        tileInfo.x = x;
        tileInfo.y = y;
        tileInfo.z = 3;
        return PAL_Ok;
    }
    else if(fileType == "BR4")
    {
        tileInfo.layerType = BR4;
        const char* p = fileTitle.c_str();
        std::string strX(p, 5);
        std::string strY(p+5, 5);
        int x, y;
        stringstream(strX)>>x;
        stringstream(strY)>>y;
        tileInfo.x = x;
        tileInfo.y = y;
        tileInfo.z = 4;
        return PAL_Ok;
    }
    else if(fileType == "BR5")
    {
        tileInfo.layerType = BR5;
        const char* p = fileTitle.c_str();
        std::string strX(p, 5);
        std::string strY(p+5, 5);
        int x, y;
        stringstream(strX)>>x;
        stringstream(strY)>>y;
        tileInfo.x = x;
        tileInfo.y = y;
        tileInfo.z = 5;
        return PAL_Ok;
    }
    else if(fileType == "BR6")
    {
        tileInfo.layerType = BR6;
        const char* p = fileTitle.c_str();
        std::string strX(p, 5);
        std::string strY(p+5, 5);
        int x, y;
        stringstream(strX)>>x;
        stringstream(strY)>>y;
        tileInfo.x = x;
        tileInfo.y = y;
        tileInfo.z = 6;
        return PAL_Ok;
    }
    else if(fileType == "BR7")
    {
        tileInfo.layerType = BR7;
        const char* p = fileTitle.c_str();
        std::string strX(p, 5);
        std::string strY(p+5, 5);
        int x, y;
        stringstream(strX)>>x;
        stringstream(strY)>>y;
        tileInfo.x = x;
        tileInfo.y = y;
        tileInfo.z = 7;
        return PAL_Ok;
    }
    else if(fileType == "BR8")
    {
        tileInfo.layerType = BR8;
        const char* p = fileTitle.c_str();
        std::string strX(p, 5);
        std::string strY(p+5, 5);
        int x, y;
        stringstream(strX)>>x;
        stringstream(strY)>>y;
        tileInfo.x = x;
        tileInfo.y = y;
        tileInfo.z = 8;
        return PAL_Ok;
    }
    else if(fileType == "BR9")
    {
        tileInfo.layerType = BR9;
        const char* p = fileTitle.c_str();
        std::string strX(p, 5);
        std::string strY(p+5, 5);
        int x, y;
        stringstream(strX)>>x;
        stringstream(strY)>>y;
        tileInfo.x = x;
        tileInfo.y = y;
        tileInfo.z = 9;
        return PAL_Ok;
    }
    else if(fileType == "BR10")
    {
        tileInfo.layerType = BR10;
        const char* p = fileTitle.c_str();
        std::string strX(p, 5);
        std::string strY(p+5, 5);
        int x, y;
        stringstream(strX)>>x;
        stringstream(strY)>>y;
        tileInfo.x = x;
        tileInfo.y = y;
        tileInfo.z = 10;
        return PAL_Ok;
    }
    else if(fileType == "BR11")
    {
        tileInfo.layerType = BR11;
        const char* p = fileTitle.c_str();
        std::string strX(p, 5);
        std::string strY(p+5, 5);
        int x, y;
        stringstream(strX)>>x;
        stringstream(strY)>>y;
        tileInfo.x = x;
        tileInfo.y = y;
        tileInfo.z = 11;
        return PAL_Ok;
    }
    else if(fileType == "BR12")
    {
        tileInfo.layerType = BR12;
        const char* p = fileTitle.c_str();
        std::string strX(p, 5);
        std::string strY(p+5, 5);
        int x, y;
        stringstream(strX)>>x;
        stringstream(strY)>>y;
        tileInfo.x = x;
        tileInfo.y = y;
        tileInfo.z = 12;
        return PAL_Ok;
    }
    else if(fileType == "BR13")
    {
        tileInfo.layerType = BR13;
        const char* p = fileTitle.c_str();
        std::string strX(p, 5);
        std::string strY(p+5, 5);
        int x, y;
        stringstream(strX)>>x;
        stringstream(strY)>>y;
        tileInfo.x = x;
        tileInfo.y = y;
        tileInfo.z = 13;
        return PAL_Ok;
    }
    else if(fileType == "BR14")
    {
        tileInfo.layerType = BR14;
        const char* p = fileTitle.c_str();
        std::string strX(p, 5);
        std::string strY(p+5, 5);
        int x, y;
        stringstream(strX)>>x;
        stringstream(strY)>>y;
        tileInfo.x = x;
        tileInfo.y = y;
        tileInfo.z = 14;
        return PAL_Ok;
    }
    else if(fileType == "BR15")
    {
        tileInfo.layerType = BR15;
        const char* p = fileTitle.c_str();
        std::string strX(p, 5);
        std::string strY(p+5, 5);
        int x, y;
        stringstream(strX)>>x;
        stringstream(strY)>>y;
        tileInfo.x = x;
        tileInfo.y = y;
        tileInfo.z = 15;
        return PAL_Ok;
    }
    else if(fileType == "TRAF")
    {
        tileInfo.layerType = TRAFFIC;
        const char* p = fileTitle.c_str();
        std::string strX(p, 5);
        std::string strY(p+5, 5);
        int x, y;
        stringstream(strX)>>x;
        stringstream(strY)>>y;
        tileInfo.x = x;
        tileInfo.y = y;
        tileInfo.z = 11;
        return PAL_Ok;
    }
    else if(fileType == "RLLP")
    {
        tileInfo.layerType = POINT_LABEL;
        const char* p = fileTitle.c_str();
        std::string strX(p, 5);
        std::string strY(p+5, 5);
        int x, y;
        stringstream(strX)>>x;
        stringstream(strY)>>y;
        tileInfo.x = x;
        tileInfo.y = y;
        tileInfo.z = 8;
        return PAL_Ok;
    }
    else if(fileType == "RUTE")
    {
        //tileInfo.layerType = DVRT;
        err = PAL_Ok;
    }
    else if(fileType == "RAST")
    {
        tileInfo.layerType = RASTR;
        const char* p = fileTitle.c_str();
        std::string strX(p, 5);
        std::string strY(p+5, 5);
        int x, y;
        stringstream(strX)>>x;
        stringstream(strY)>>y;
        tileInfo.x = x;
        tileInfo.y = y;
        tileInfo.z = 15;
        return PAL_Ok;
    }
    else if(fileType == "LBLT")
    {
        tileInfo.layerType = RASTR;
        const char* p = fileTitle.c_str();
        std::string strX(p, 5);
        std::string strY(p+5, 5);
        int x, y;
        stringstream(strX)>>x;
        stringstream(strY)>>y;
        tileInfo.x = x;
        tileInfo.y = y;
        tileInfo.z = 15;
        return PAL_Ok;
    }
    else
    {
        err = PAL_ErrUnsupported;
    }
    if(err == PAL_Ok)
    {
        const char* p = fileTitle.c_str();
        std::string strX(p, 5);
        std::string strY(p+5, 5);
        int x, y;
        stringstream(strX)>>x;
        stringstream(strY)>>y;
        tileInfo.x = x;
        tileInfo.y = y;
        tileInfo.z = 15;
    }

    return err;
}
//2. try to parse T_DVA_1_NBM_23_5899_12853
static PAL_Error ParseCCCName(const std::string& fileName, NBMTileInfo& tileInfo)
{
    PAL_Error err = PAL_ErrUnsupported;

    std::vector<std::string> pStr;
    SplitString(fileName, '_', pStr);
    if(pStr.size() != 7)
    {
        err = PAL_ErrUnsupported;
    }
    else if(pStr[0] != "T")
    {
        err = PAL_ErrUnsupported;
    }
    else
    {
        err = PAL_Ok;
        //x
        std::string strX(pStr[5]);
        stringstream(strX)>>tileInfo.x;
        //y
        std::string strY(pStr[6]);
        stringstream(strY)>>tileInfo.y;
        //z
        //type
        if(pStr[1] == "DVR")
        {
            tileInfo.layerType = DVR;
            tileInfo.labelType = DVR_LABEL;
            tileInfo.z = 15;
        }
        else if(pStr[1] == "DVA")
        {
            tileInfo.layerType = DVA;
            tileInfo.labelType = DVA_LABEL;
            tileInfo.z = 15;
        }
        else if(pStr[1] == "GVR")
        {
            tileInfo.layerType = GVR;
            tileInfo.labelType = DVR_LABEL;
            tileInfo.z = 12;
        }
        else if(pStr[1] == "GVA")
        {
            tileInfo.layerType = GVA;
            tileInfo.labelType = DVA_LABEL;
            tileInfo.z = 12;
        }
        else if(pStr[1] == "B3D")
        {
            tileInfo.layerType = B3D;
            tileInfo.labelType = DVA_LABEL;
            tileInfo.z = 15;
            err = PAL_Ok;
        }
        else if(pStr[1] == "B2D")
        {
            tileInfo.layerType = DVA;
            tileInfo.labelType = DVA_LABEL;
            tileInfo.z = 15;
            err = PAL_Ok;
        }
        else if(pStr[1] == "3DLM" || pStr[1] == "LM3D")
        {
            tileInfo.layerType = LM3D;
            tileInfo.labelType = DVA_LABEL;
            err = PAL_Ok;
        }
        else if(pStr[1] == "BR2")
        {
            tileInfo.layerType = BR2;
            tileInfo.z = 2;
            err = PAL_Ok;
        }
        else if(pStr[1] == "BR3")
        {
            tileInfo.layerType = BR3;
            tileInfo.z = 3;
            err = PAL_Ok;
        }
        else if(pStr[1] == "BR4")
        {
            tileInfo.layerType = BR4;
            tileInfo.z = 4;
            err = PAL_Ok;
        }
        else if(pStr[1] == "BR5")
        {
            tileInfo.layerType = BR5;
            tileInfo.z = 5;
            err = PAL_Ok;
        }
        else if(pStr[1] == "BR6")
        {
            tileInfo.layerType = BR6;
            tileInfo.z = 6;
            err = PAL_Ok;
        }
        else if(pStr[1] == "BR7")
        {
            tileInfo.layerType = BR7;
            tileInfo.z = 7;
            err = PAL_Ok;
        }
        else if(pStr[1] == "BR8")
        {
            tileInfo.layerType = BR8;
            tileInfo.z = 8;
            err = PAL_Ok;
        }
        else if(pStr[1] == "BR9")
        {
            tileInfo.layerType = BR9;
            tileInfo.z = 9;
            err = PAL_Ok;
        }
        else if(pStr[1] == "BR10")
        {
            tileInfo.layerType = BR10;
            tileInfo.z = 10;
            err = PAL_Ok;
        }
        else if(pStr[1] == "BR11")
        {
            tileInfo.layerType = BR11;
            tileInfo.z = 11;
            err = PAL_Ok;
        }
        else if(pStr[1] == "BR12")
        {
            tileInfo.layerType = BR12;
            tileInfo.z = 12;
            err = PAL_Ok;
        }
        else if(pStr[1] == "BR13")
        {
            tileInfo.layerType = BR13;
            tileInfo.z = 13;
            err = PAL_Ok;
        }
        else if(pStr[1] == "BR14")
        {
            tileInfo.layerType = BR14;
            tileInfo.z = 14;
            err = PAL_Ok;
        }
        else if(pStr[1] == "BR15")
        {
            tileInfo.layerType = BR15;
            tileInfo.z = 15;
            err = PAL_Ok;
        }
        else if(pStr[1] == "DVRT")
        {
            tileInfo.layerType = DVR;
            tileInfo.z = 15;
            err = PAL_Ok;
        }
        else
        {
            err = PAL_ErrUnsupported;
        }
    }
    return err;
}

PAL_Error GetNBMTileInfo(const std::string& filePath, NBMTileInfo& tileInfo)
{
    PAL_Error err = PAL_ErrUnsupported;

    uint32 pos = filePath.rfind('\\');
    if(pos == std::string::npos)
    {
    	pos = filePath.rfind('/');
    }
    uint32 npos = filePath.rfind('.');
	if(npos == std::string::npos)
	{
		npos = filePath.length();
	}
	std::string fileTitle = filePath.substr(pos+1, npos-pos-1);
	tileInfo.name = fileTitle;
	tileInfo.path = filePath;
   //1. try to parse 09371125363DLM.nbm
	err = ParseMobiusName(fileTitle, tileInfo);
	if(err == PAL_Ok)
	{
		return err;
	}
	//2. try to parse T_DVA_1_NBM_23_5899_12853
	return ParseCCCName(fileTitle, tileInfo);

    return err;
}

float TileToMercatorX(int tx, float px, int tz)
{
    int tScale = (1 << (tz - 1));
    return static_cast<float>(((tx + px)/tScale - 1.0f) * PI);
}

float TileToMercatorY(int ty, float py, int tz)
{
    int tScale = (1 << (tz - 1));
    return static_cast<float>((1.0f - (ty + py)/tScale) * PI);
}

int MercatorToTileX(double mx, int tz)
{
    double tscale = (double)(1 << (tz - 1));
    return static_cast<int>(floor((mx / PI + 1.0) * tscale));

}

int MercatorToTileY(double my, int tz)
{
    double tscale = (double)(1 << (tz - 1));
    return static_cast<int>(floor((1.0 - my / PI) * tscale));
}
