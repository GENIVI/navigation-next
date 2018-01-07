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

#ifndef _NBRE_TEST_UTILITIES_H
#define _NBRE_TEST_UTILITIES_H

#include "nbremesh.h"
#include "nbrerenderengine.h"
#include "pal.h"
#include "nbgm.h"

#define NBGM_COMMON_MATERIAL "common"
#define NBGM_PIN_MATERIAL "pin"

char* GetExePath();
char* GetModelPath(const char* modelName);
char* StringToChar(CString& str);
float TileToMercatorX(int tx, float px, int tz); 
float TileToMercatorY(int ty, float py, int tz);
void SpatialConvertLatLongToMercator(double latitude, double longitude, double& xMercator, double& yMercator);
CString* SplitString(CString str, char split, int& iSubStrs);
 
NBRE_Mesh* CreateRegularPrism(NBRE_IRenderPal& renderPal, uint16 edgeNum, float edgeHeight, float radius);
NBRE_Mesh* CreateSphere(NBRE_IRenderPal& renderPal, float radius);
NBRE_Mesh* Create3DTextureSphere(NBRE_IRenderPal& renderPal, float radius);
NBRE_Mesh* CreateCube(NBRE_IRenderPal& renderPal, float length, float width, float height);

/// vertices must be saved in counter clockwise
NBRE_Mesh* CreatePolygon3D(NBRE_IRenderPal& renderPal, float* vertices, uint16 vertexCount);
NBRE_Mesh* CreateGround(NBRE_IRenderPal& renderPal, float width, int32 count, float r = 0.5f, float g = 0.5f, float b = 0.5f);

PAL_Instance* GetPal();
void DestoryPal();
void InitializeNBGM();
NBRE_RenderEngine* CreateRenderEngine(NBRE_IRenderPal* renderer);
#endif


