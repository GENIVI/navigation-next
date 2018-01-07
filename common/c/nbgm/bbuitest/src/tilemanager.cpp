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
 * tilemanager.cpp
 *
 *  Created on: Jun 17, 2013
 *      Author: btian
 */
#include "tilemanager.h"
#include "palstdlib.h"
#include "tilenameparser.h"
#include <qdir.h>
#include <qdiriterator.h>

#define TILE_COUNT 2

TileManager::TileManager()
{

}

TileManager::~TileManager()
{

}

void TileManager::Reset()
{
	mAllTiles.clear();
}

void TileManager::AddFolder(QString folder)
{
	mTileFolder = folder.toStdString();
	ScanDir(folder);
}

void TileManager::AddTile(QString file)
{
    NBMTileInfo tileInfo;
    nsl_memset(&tileInfo, 0, sizeof(NBMTileInfo));
    if(GetNBMTileInfo(file.toStdString(), tileInfo) == PAL_Ok)
    {
    	mAllTiles.push_back(tileInfo);
    }
}

bool TileManager::ScanDir(QString folder)
{
	QDir dir(folder);
	if (!dir.exists())
		return false;

   QDirIterator iterator(dir.absolutePath(),
		   QDir::Files | QDir::NoSymLinks | QDir::AllDirs | QDir::NoDotAndDotDot,
		   QDirIterator::Subdirectories);
   while (iterator.hasNext())
   {
	  iterator.next();
	  if (iterator.fileInfo().isFile())
	  {
		 AddTile(iterator.filePath());
	  }
   }
   fflush(stdout);
   return(true);
}

bool TileManager::GetTile(unsigned int index, NBMTileInfo& tileInfo) const
{
	if(index<mAllTiles.size())
	{
		tileInfo = mAllTiles[index];
		return true;
	}
	return false;
}

#define URI_NAME_LEN 256
std::string
FormatString(const char* fmt, ...)
{
    va_list argptr;
    char buf[URI_NAME_LEN];
    nsl_memset(buf, 0, URI_NAME_LEN);

    va_start(argptr, fmt);
    nsl_vsnprintf(buf, URI_NAME_LEN-1, fmt, argptr);
    va_end(argptr);

    std::string s(buf);
    return s;
}
void TileManager::GetTile(int x, int y, int z, std::vector<NBMTileInfo>& tileList) const
{
//	printf("mAllTiles.size()=%d\n", mAllTiles.size());
//	for(unsigned int i=0; i<mAllTiles.size(); ++i)
//	{
//		NBMTileInfo info = mAllTiles[i];
//		if(info.x < x+TILE_COUNT && info.x > x-TILE_COUNT &&
//				info.y < y+TILE_COUNT && info.y > y-TILE_COUNT &&
//				info.z == z)
//		{
//			tileList.push_back(info);
//		}
//	}
	for(int mx = x-TILE_COUNT; mx<=x+TILE_COUNT; ++mx)
	{
		for(int my = y-TILE_COUNT; my<=y+TILE_COUNT; ++my)
		{
			//ADD DVR
			{
				NBMTileInfo info;
				info.x = mx;
				info.y = my;
				info.z = z;
				info.layerType = DVR;
				info.labelType = DVR_LABEL;
				info.name = FormatString("T_DVR_1_NBM_23_%d_%d", mx, my);
				info.path = mTileFolder + "/NBM/DVR/" + info.name;
				tileList.push_back(info);
			}
			//ADD DVA
			{
				NBMTileInfo info;
				info.x = mx;
				info.y = my;
				info.z = z;
				info.layerType = DVA;
				info.labelType = DVA_LABEL;
				info.name = FormatString("T_DVA_1_NBM_23_%d_%d", mx, my);
				info.path = mTileFolder + "/NBM/DVA/" + info.name;
				tileList.push_back(info);
			}
			//ADD B2D
			{
				NBMTileInfo info;
				info.x = mx;
				info.y = my;
				info.z = z;
				info.layerType = DVA;
				info.labelType = DVA_LABEL;
				info.name = FormatString("T_B2D_1_NBM_23_%d_%d", mx, my);
				info.path = mTileFolder + "/NBM/B2D/" + info.name;
				tileList.push_back(info);
			}
		}
	}

}
