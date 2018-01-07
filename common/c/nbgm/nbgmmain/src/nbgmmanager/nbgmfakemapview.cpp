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

/*--------------------------------------------------------------------------
 
 (C) Copyright 2012 by Networks In Motion, Inc.
 
 The information contained herein is confidential, proprietary
 to Networks In Motion, Inc., and considered a trade secret as
 defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of Networks In Motion is granted only
 under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.
 
 ---------------------------------------------------------------------------*/
#if GL_CAPABILITY_TEST
#include "nbgmfakemapview.h"
#include "nbrerenderengine.h"
#include "nbredefaultspatialdividemanager.h"
#include "nbrespatialdividemanagerdebug.h"
#include "nbreentity.h"
#include "nbgmmapmaterialmanager.h"
#include "nbrelog.h"
#include "nbrebillboardnode.h"
#include "nbgmlayoutmanager.h"
#include "nbgmlabellayer.h"
#include "nbgmpoilayer.h"
#include "nbgmtilemaplayer.h"
#include "nbrespatialdividemanagerdebug.h"
#include "nbretypeconvert.h"
#include "nbretransformation.h"
#include "nbgmavatar.h"
#include "palclock.h"
#include "nbrelinearspatialdividemanager.h"
#include "nbgmbuildmodelcontext.h"
#include "nbgmmapmaterialmanager.h"
#include "nbgmmapviewdatacache.h"
#include "nbgmcommon.h"
#include "nbreruntimeinfo.h"
#include "nbgmsky.h"
#include "nbgm.h"
#include "nbgmnaviconlayer.h"
#include "nbgmflag.h"
#include "nbgmgridentity.h"
#include "nbgmnavvectorroutelayer.h"
#include "nbgmtransparentoverlay.h"
#include "nbgmnavecmroutelayer.h"
#include "nbgmconfig.h"
#include "nbgmanimation.h"
#include "palgl.h"

// These two value controls test data, the total triangles number for each frame: 
// Total = TRIANGLE_COUNT_EACH_SUMBIT * SUBMIT_TIMES.
// The default value is 150k

//Triangles count for each submit, note that is MUST be less than 65535
static const uint32 TRIANGLE_COUNT_EACH_SUMBIT = 30000;
//Submit times for each frame
static const uint32 SUBMIT_TIMES = 5;

//change to 1 if you want to test triangle fan
#define USE_TRIANGLES 0
#define USE_VBO 0

static const float R = 10.f;

NBGM_FakeMapView::PackedTriangles::PackedTriangles()
:mVertex(NULL)
,mVertexVBOId(0)
,mNormal(NULL)
,mNormalVBOId(0)
,mTexcoord(NULL)
,mTexcoordVBOId(0)
,mIndex(NULL)
,mTrianglesNum(0)
,mTexid(0)
,mVertexNum(0)
{
}

NBGM_FakeMapView::PackedTriangles::~PackedTriangles()
{
    NBRE_DELETE_ARRAY mVertex;
    NBRE_DELETE_ARRAY mNormal;
    NBRE_DELETE_ARRAY mTexcoord;
    NBRE_DELETE_ARRAY mIndex;
    
    if (mTexid != 0)
    {
        glDeleteTextures(1, &mTexid);
    }
    if(mVertexVBOId != 0)
    {
        glDeleteBuffers(1, &mVertexVBOId);
    }
    if(mNormalVBOId != 0)
    {
        glDeleteBuffers(1, &mNormalVBOId);
    }
    if(mTexcoordVBOId != 0)
    {
        glDeleteBuffers(1, &mTexcoordVBOId);
    }
}

NBGM_FakeMapView::NBGM_FakeMapView(const NBGM_MapViewImplConfig& config, NBRE_RenderEngine* renderEngine)
:mInitialized(FALSE),
 mProfiler(3, 50)
{
}

NBGM_FakeMapView::~NBGM_FakeMapView()
{
    NBRE_Vector<PackedTriangles*>::iterator pTriangles = mTrianglesArray.begin();
    NBRE_Vector<PackedTriangles*>::iterator pTrianglesEnd = mTrianglesArray.end();
    for (; pTriangles != pTrianglesEnd; ++pTriangles)
    {
        NBRE_DELETE (*pTriangles);
    }
}

GLuint CreateTexture(uint8 r, uint8 g, uint8 b)
{
    uint32 count = 128*128*4;
    uint8* pixels  = new uint8[count];

    for (uint32 i = 0; i<count; i+=4)
    {
        pixels[i] =     r;
        pixels[i+1] =   g;
        pixels[i+2] =   b;
        pixels[i+3] =   0xff;
    }

    GLuint res;
    glEnable(GL_TEXTURE_2D);
    glGenTextures(1, &res);
    glBindTexture(GL_TEXTURE_2D, res);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 128, 128, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );

    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
    delete[] pixels;
    return res;
}

static void
CreateVertex(float* vertex, float* normal, float* texcoord
          , float r, float theta, float z
          , uint32& vIndex)
{
    float x = (float)cos(theta);
    float y = (float)sin(theta);
    
    vertex[vIndex*3 + 0] = r*x;  vertex[vIndex*3 + 1] = r*y;  vertex[vIndex*3 + 2] = z;
    normal[vIndex*3 + 0] = 0;    normal[vIndex*3 + 1] = 0;    normal[vIndex*3 + 2] = -1;
    if(r == 0)
    {
        texcoord[vIndex*2 + 0] = 0.5f;               texcoord[vIndex*2 + 1] = 0.5f;
    }
    else
    {
        texcoord[vIndex*2 + 0] = x*0.5f + 0.5f;     texcoord[vIndex*2 + 1] = y*0.5f + 0.5f;
    }
    vIndex++;
}

void
NBGM_FakeMapView::CreateTriangleFan(uint16 trianglesCount)
{
    if(trianglesCount < 3)
    {
        return;
    }
    static int triIndex = 0;

    PackedTriangles* element = NBRE_NEW PackedTriangles();
    uint16 vertexCount = trianglesCount+1;
    
    element->mVertexNum = vertexCount;
    element->mTrianglesNum = trianglesCount;
    element->mVertex = NBRE_NEW float[vertexCount * 3];
    element->mTexcoord = NBRE_NEW float[vertexCount * 2];
    element->mNormal =  NBRE_NEW float[vertexCount * 3];
    element->mIndex = NBRE_NEW uint16[element->mTrianglesNum+2];

    float detla = 3.1415926f*2/trianglesCount;
    float theta = 0;

    uint32 vIndex = 0;
    uint16 iIndex = 0;

    NBRE_Point3f Cv;
    NBRE_Point3f Cn;
    NBRE_Point2f Ct;
    
    CreateVertex(element->mVertex, element->mNormal, element->mTexcoord, 0, 0, (float)triIndex, vIndex);
    element->mIndex[iIndex] = iIndex;
    iIndex++;

    for(uint16 i=0; i< trianglesCount; ++i)
    {
        CreateVertex(element->mVertex, element->mNormal, element->mTexcoord, R, theta, (float)triIndex, vIndex);
        element->mIndex[iIndex] = iIndex;
        iIndex++;
        theta+=detla;
    }
    element->mIndex[iIndex] = 0;

    element->mTexid = CreateTexture(25, 59, 255);
    mTrianglesArray.push_back(element);
    
    triIndex++;
}

static void
AddTrianglesIndex(uint16* index, uint16 p0, uint16 p1, uint16 p2, uint32& iIndex)
{
    index[iIndex++] = p0;
    index[iIndex++] = p1;
    index[iIndex++] = p2;
}

void
NBGM_FakeMapView::CreateTriangles(uint16 trianglesCount)
{
    static int triIndex = 0;

    PackedTriangles* element = NBRE_NEW PackedTriangles();

    uint32 vertexCount = trianglesCount+1;

    element->mVertexNum = vertexCount;
    element->mTrianglesNum = trianglesCount;
    element->mVertex = NBRE_NEW float[vertexCount * 3];
    element->mNormal =  NBRE_NEW float[vertexCount * 3];
    element->mTexcoord = NBRE_NEW float[vertexCount * 2];
    element->mIndex = NBRE_NEW uint16[trianglesCount*3];
    

    NBRE_Point3f Cv;
    NBRE_Point3f Cn;
    NBRE_Point2f Ct;

    float detla = 3.1415926f*2/trianglesCount;
    float theta = 0;
    uint32 vIndex = 0;
    uint32 iIndex = 0;

    CreateVertex(element->mVertex, element->mNormal, element->mTexcoord, 0, 0, (float)triIndex, vIndex);
    CreateVertex(element->mVertex, element->mNormal, element->mTexcoord, R, theta, (float)triIndex, vIndex);
    for (uint16 j = 1; j < trianglesCount; ++j)
    {
        theta += detla;
        CreateVertex(element->mVertex, element->mNormal, element->mTexcoord, R, theta, (float)triIndex, vIndex);
        AddTrianglesIndex(element->mIndex, 0, j-1, j, iIndex);
    }

    AddTrianglesIndex(element->mIndex, 0, element->mTrianglesNum-1, 1, iIndex);

    element->mTexid = CreateTexture(215, 125, 21);
    mTrianglesArray.push_back(element);

    triIndex++;
}

void
NBGM_FakeMapView::PackedTriangles::GenerateVBO()
{
    if(mVertexVBOId == 0)
    {
        glGenBuffers(1, &mVertexVBOId);
        glBindBuffer(GL_ARRAY_BUFFER, mVertexVBOId);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float)*3*mVertexNum, mVertex, GL_STATIC_DRAW);
    }

    if(mNormalVBOId == 0)
    {
        glGenBuffers(1, &mNormalVBOId);
        glBindBuffer(GL_ARRAY_BUFFER, mNormalVBOId);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float)*3*mVertexNum, mNormal, GL_STATIC_DRAW);
    }

    if(mTexcoordVBOId == 0)
    {
        glGenBuffers(1, &mTexcoordVBOId);
        glBindBuffer(GL_ARRAY_BUFFER, mTexcoordVBOId);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float)*2*mVertexNum, mTexcoord, GL_STATIC_DRAW);
    }
}

void NBGM_FakeMapView::InitializeEnvironment()
{
    if (mInitialized)
    {
        return;
    }
    for (int i=0; i<SUBMIT_TIMES; ++i) {
#if USE_TRIANGLES
        CreateTriangles(TRIANGLE_COUNT_EACH_SUMBIT);
#else
        CreateTriangleFan(TRIANGLE_COUNT_EACH_SUMBIT);
#endif
    }
    mInitialized = TRUE;
}

void NBGM_FakeMapView::ActiveSurface()
{
}

NBRE_RenderSurface* NBGM_FakeMapView::Surface()
{
    return NULL;
}


nb_boolean NBGM_FakeMapView::RenderFrame(float secondsPassed)
{
    uint32 begin = PAL_ClockGetTimeMs();
    glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDisable(GL_SMOOTH);
    glDisable(GL_FOG);
    glDisable(GL_LIGHTING);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    NBRE_Vector<PackedTriangles*>::iterator pElement = mTrianglesArray.begin();
    NBRE_Vector<PackedTriangles*>::iterator pElementEnd = mTrianglesArray.end();
    for (; pElement != pElementEnd; ++pElement)
    {
#if USE_VBO
        (*pElement)->GenerateVBO();
#endif
        glBindTexture(GL_TEXTURE_2D, (*pElement)->mTexid);
        if((*pElement)->mVertexVBOId != 0)
        {
            glBindBuffer(GL_ARRAY_BUFFER, (*pElement)->mVertexVBOId);
            glVertexPointer(3, GL_FLOAT, 0, 0);
        }
        else
        {
            glVertexPointer(3, GL_FLOAT, 0, (*pElement)->mVertex);
        }
        if((*pElement)->mNormalVBOId != 0)
        {
            glBindBuffer(GL_ARRAY_BUFFER, (*pElement)->mNormalVBOId);
            glNormalPointer(GL_FLOAT, 0, 0);
        }
        else
        {
            glNormalPointer(GL_FLOAT, 0, (*pElement)->mNormal);
        }
        if((*pElement)->mTexcoordVBOId != 0)
        {
            glBindBuffer(GL_ARRAY_BUFFER, (*pElement)->mTexcoordVBOId);
            glTexCoordPointer(2, GL_FLOAT, 0, 0);
        }
        else
        {
            glTexCoordPointer(2, GL_FLOAT, 0, (*pElement)->mTexcoord);
        }
#if USE_TRIANGLES
        glDrawElements(GL_TRIANGLES, (*pElement)->mTrianglesNum*3, GL_UNSIGNED_SHORT, (*pElement)->mIndex);
#else
        glDrawElements(GL_TRIANGLE_FAN, (*pElement)->mTrianglesNum+2, GL_UNSIGNED_SHORT, (*pElement)->mIndex);
#endif
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    uint32 end = PAL_ClockGetTimeMs();
    printf("Current frame rate: %f\n", 1000.f/(end - begin));
    mProfiler.SetTotalTriangleNum(TRIANGLE_COUNT_EACH_SUMBIT* SUBMIT_TIMES);
    mProfiler.SetTotalRenderTime(end - begin);

    return 1;
}

void NBGM_FakeMapView::SetViewCenter(double mercatorX, double mercatorY)
{
}

void NBGM_FakeMapView::GetViewCenter(double& mercatorX, double& mercatorY)
{
}

void NBGM_FakeMapView::LoadCommonMaterial(const NBRE_String& materialName, NBRE_IOStream& istream)
{
}

void NBGM_FakeMapView::LoadPinMaterial(const NBRE_String& pinMaterialName, NBRE_IOStream& istream)
{
}

void NBGM_FakeMapView::SetCurrentCommonMaterial(const NBRE_String& materialName)
{
}

void NBGM_FakeMapView::OnSizeChanged(int32 x, int32 y, uint32 w, uint32 h)
{
    if (h == 0)
    {
        h = 1;
    }
    
    glViewport(0, 0, w, h);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    gluPerspective(45, w/(float)h, 0.5, 15000);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    gluLookAt(0, 0, -30, 0, 0, 0, 0, 1, 0);
}

void NBGM_FakeMapView::SetPerspective(float fov, float aspect)
{
}

nb_boolean NBGM_FakeMapView::IsNbmDataExist(const NBRE_String& nbmName)
{
    return FALSE;
}

void NBGM_FakeMapView::LoadNbmData(const NBRE_String& nbmName, uint8 baseDrawOrder, uint8 labelDrawOrder, NBRE_IOStream& istream)
{
}

void NBGM_FakeMapView::LoadBinData(const NBRE_String& binName, NBRE_IOStream& istream)
{
}

void NBGM_FakeMapView::UnLoadTile(const NBRE_String& nbmName)
{
}

void NBGM_FakeMapView::SyncMapData()
{
}

void NBGM_FakeMapView::OnTouchEvent(float x, float y)
{
}

void NBGM_FakeMapView::OnPaning(float x, float y)
{
}

void NBGM_FakeMapView::OnLongPress()
{
    NBRE_DebugLog(PAL_LogSeverityCritical, "NBGM_FakeMapView::OnLongPress not support yet");
    nbre_assert(0);
}

void NBGM_FakeMapView::OnSingleTapUp()
{
    NBRE_DebugLog(PAL_LogSeverityCritical, "NBGM_FakeMapView::OnSingleTapUp not support yet");
    nbre_assert(0);
}

void NBGM_FakeMapView::Move(float dx, float dy)
{
}

void NBGM_FakeMapView::ScreenToMapPosition(float screenX, float screenY, double& mercatorX, double& mercatorY)
{
}

nb_boolean NBGM_FakeMapView::GetFrustumPositionInWorld(double frustumHeight, NBRE_Vector<NBGM_Point2d64>& positions)
{
    return 0;
}

void NBGM_FakeMapView::SetHorizonDistance(double horizonDistance)
{
}

void NBGM_FakeMapView::SetRotateCenter(float x, float y)
{
}

void NBGM_FakeMapView::Rotate(float angle)
{
}

void NBGM_FakeMapView::SetRotateAngle(float angle)
{
}

float NBGM_FakeMapView::RotateAngle()
{
    return 0;
}

void NBGM_FakeMapView::Tilt(float angle)
{
}

void NBGM_FakeMapView::SetTiltAngle(float angle)
{
}

float NBGM_FakeMapView::TiltAngle()
{
    return 0;
}

void NBGM_FakeMapView::Zoom(float deltaH)
{
}

float NBGM_FakeMapView::ViewPointDistance()
{
    return 0;
}

void NBGM_FakeMapView::SetRoadWidthFactor(float scale)
{
}

void NBGM_FakeMapView::SetViewPointDistance(float distance)
{
}

void NBGM_FakeMapView::SetAvatarLocation(const NBGM_Location64& location)
{
}

void NBGM_FakeMapView::SetAvatarScale(float scaleValue)
{
}

void NBGM_FakeMapView::SetAvatarState(NBGM_AvatarState state)
{
}

void NBGM_FakeMapView::SetAvatarMode(NBGM_AvatarMode mode)
{
}

void NBGM_FakeMapView::EnableProfiling(nb_boolean enabled)
{
}

void NBGM_FakeMapView::EnableVerboseProfiling(nb_boolean enabled)
{
}

nb_boolean NBGM_FakeMapView::GetPoiPosition(const NBRE_String& poiId, NBRE_Point2f& coordinates)
{
    return 0;
}

nb_boolean NBGM_FakeMapView::SelectAndTrackPoi(const NBRE_String& poiId)
{
    return 0;
}

nb_boolean NBGM_FakeMapView::UnselectAndStopTrackingPoi(const NBRE_String& poiId)
{
    return 0;
}

uint32 NBGM_FakeMapView::GetInteractedPois(NBRE_Vector<NBRE_String>& pois, const NBRE_Point2f& screenPosition)
{
    return 0;
}

float NBGM_FakeMapView::GetCameraHeight() const
{
    return 0;
}

void NBGM_FakeMapView::SetSkySize(float width, float height, float distance)
{
}

void NBGM_FakeMapView::SetEndFlagLocation(const NBGM_Location64& location)
{
}

void NBGM_FakeMapView::SetStartFlagLocation(const NBGM_Location64& location)
{
}

void NBGM_FakeMapView::RefreshNavPois(const NBRE_Vector<NBGM_Poi*>& pois)
{
}

PAL_Error NBGM_FakeMapView::AddNavVectorRoutePolyline(const NBGM_VectorRouteData* routeData )
{
    return PAL_ErrHttpNoContent;
}

void NBGM_FakeMapView::SetNavVectorCurrentManeuver( uint32 currentManuever )
{
}

uint32 NBGM_FakeMapView::NavVectorCurrentManeuver()
{
    return 0;
}

void NBGM_FakeMapView::ResetNavVectorRouteLayer()
{
}

void NBGM_FakeMapView::SetAvatarModelSizeScale(float s)
{
}

void NBGM_FakeMapView::SetOrthoCameraSetting(NBGM_CameraSetting setting)
{
}

void NBGM_FakeMapView::SetPerspectiveCameraSetting(NBGM_CameraSetting setting)
{
}

PAL_Error NBGM_FakeMapView::AddNavEcmRouteSpline( const NBGM_SplineData* data )
{
    return PAL_ErrHttpNoContent;
}

PAL_Error NBGM_FakeMapView::SetNavEcmCurrentManeuver(uint32 currentManueverID)
{
    return PAL_ErrHttpNoContent;
}

void NBGM_FakeMapView::ResetNavEcmRouteLayer()
{
}

PAL_Error NBGM_FakeMapView::AddNavEcmManeuver(uint32 maneuverID, const NBGM_Point3d& position)
{
    return PAL_ErrHttpNoContent;
}

PAL_Error
NBGM_FakeMapView::NavEcmSnapRoute( NBGM_Location64& location, NBGM_SnapRouteInfo& snapRouteInfo ) const
{
    return PAL_ErrBadParam;
}

NBGM_SnapRouteInfo&
NBGM_FakeMapView::NavEcmGetAvatarSnapRouteInfo()
{
    return *new NBGM_SnapRouteInfo;
}

PAL_Error
NBGM_FakeMapView::NavEcmSnapToRouteStart( NBGM_Location64& location, NBGM_SnapRouteInfo& pRouteInfo ) const
{
    return PAL_ErrHttpNoContent;
}

void
NBGM_FakeMapView::SetIsNavMode(nb_boolean value)
{
}

void 
NBGM_FakeMapView::AdjustFlagSize(float size, float minPixelSize, float maxPixelSize)
{
}

void 
NBGM_FakeMapView::DumpProfile(NBGM_MapViewProfile& profile)
{
    mProfiler.Dump(profile);
}

nb_boolean
NBGM_FakeMapView::MapPositionToScreen(float& screenX, float& screenY, double mercatorX, double mercatorY)
{
    return FALSE;
}

nb_boolean
NBGM_FakeMapView::GetAvatarScreenPosition( NBRE_Point2f& pos )
{
    return FALSE;
}
#endif