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
#include "nbgmsky.h"
#include "nbgmskywall.h"
#include "nbretransformation.h"
#include "nbreoverlay.h"
#include "nbretypeconvert.h"
#include "nbredefaultpass.h"
#include "nbreintersection.h"
#include "nbredefaultshadergroup.h"
#include "nbrepngcodec.h"
#include "nbrepngtextureimage.h"
#include "nbgmprotected.h"
#include "nbrerenderengine.h"
#include "nbgmlayoutmanager.h"
#include "nbrelog.h"
#include "nbgmbuildutility.h"

#define SKYBOX_VERTEX_COUNT  8
#define SKYPLANE_VERTEX_COUNT 4
#define SKY_PALNE_COUNT 4

#define SKY_GROUP_DAY           "SKY\\DAY"
#define SKY_GROUP_NIGHT         "SKY\\NIGHT"
#define SKY_SHADER_NAME         "SKY\\BOX"
#define SKY_TEXTURE_EAST        "TEXTURE/SKY"
#define SKY_TEXTURE_WEST        "TEXTURE/SKY"
#define SKY_TEXTURE_NORTH       "TEXTURE/SKY"
#define SKY_TEXTURE_SOUTH       "TEXTURE/SKY"
#define SKY_TEXTURE_UP          "TEXTURE/SKY"
#define SKY_TEXTURE_DOWN        "TEXTURE/SKY"
#define SKY_TEXTURE_DAY_WALL    "TEXTURE/SKY"
#define SKY_TEXTURE_NIGHT_WALL  "TEXTURE/NIGHT_SKY"
#define SKY_TEXTURE_FACTOR      1.f/90.f
#define SKY_DAY                 "DAY"
#define SKY_NIGHT               "NIGHT"
#define SKY_GROUP               "SKY\\"

class SkyImage: public NBRE_ITextureImage
{
public:
    SkyImage(NBRE_String path[6], PAL_Instance& pal)
    {
        for(int32 i = 0; i < 6; ++i)
        {
            mImage[i] = NBRE_NEW NBRE_PngTextureImage(pal, path[i].c_str(), 0, TRUE);
        }
    }
    virtual ~SkyImage()
    {
        for(int32 i = 0; i < 6; ++i)
        {
            NBRE_DELETE mImage[i];
        }
    }

public:
    virtual void Release() { NBRE_DELETE this; }
    virtual uint8 GetMipmapCount() const { return 1; }
    virtual uint8 GetFaceCount() const { return 6; }
    virtual const NBRE_Image* GetImage(uint8 face, uint8 mipmapLevel) const {  return mImage[face]->GetImage(0, 0); }
    virtual void ReadInfo(NBRE_ImageInfo &fm,  NBRE_ImageInfo &originalInfo)const
    {
        mImage[0]->ReadInfo(fm, originalInfo);
    }

    virtual PAL_Error Load()
    {
        for(int32 i = 0; i < 6; ++i)
        {
            if (mImage[i] == NULL)
            {
                mImage[i]->Load();
                if (mImage[i] == NULL)
                {
                    return PAL_Failed;
                }
            }
        }
        return PAL_Ok;
    }

private:
    DISABLE_COPY_AND_ASSIGN(SkyImage);

protected:
    NBRE_PngTextureImage* mImage[6];
};

static NBRE_TexturePtr CreateCubeTexture(NBRE_Context& context)
{
    NBRE_String* fullPath = NBRE_NEW NBRE_String[6];
    int32 i = 0;
    fullPath[i++] = NBGM_MakeFullPath(SKY_TEXTURE_EAST);
    fullPath[i++] = NBGM_MakeFullPath(SKY_TEXTURE_WEST);
    fullPath[i++] = NBGM_MakeFullPath(SKY_TEXTURE_UP);
    fullPath[i++] = NBGM_MakeFullPath(SKY_TEXTURE_DOWN);
    fullPath[i++] = NBGM_MakeFullPath(SKY_TEXTURE_NORTH);
    fullPath[i++] = NBGM_MakeFullPath(SKY_TEXTURE_SOUTH);

    NBRE_TexturePtr texture = NBRE_TexturePtr(context.mRenderPal->CreateTexture(NBRE_NEW SkyImage(fullPath, *context.mPalInstance), NBRE_Texture::TT_CUBE_MAP, FALSE, "NBGM_Sky"));
    NBRE_DELETE_ARRAY fullPath;
    return texture;
}

static NBRE_TexturePtr Create2DTexture(NBRE_Context& context, uint32 index)
{
    NBRE_String texName;
    switch(index)
    {
    case 0:
        {
            texName = SKY_TEXTURE_NORTH;
        }
        break;
    case 1:
        {
            texName = SKY_TEXTURE_WEST;
        }
        break;
    case 2:
        {
            texName = SKY_TEXTURE_SOUTH;
        }
        break;
    case 3:
        {
            texName = SKY_TEXTURE_EAST;
        }
        break;
    case 4:
        {
            texName = SKY_TEXTURE_UP;
        }
        break;
    case 5:
        {
            texName = SKY_TEXTURE_DOWN;
        }
        break;
    default:
        NBRE_DebugLog(PAL_LogSeverityCritical, "Create2DTexture failed, index=%d", index);
        nbre_assert(FALSE);
        break;
    }

    NBRE_String fullPath = NBGM_MakeFullPath(texName);
    NBRE_TexturePtr texture = NBGM_BuildTextureUtility::ConstructImageTexture(*context.mRenderPal, context.mPalInstance, fullPath.c_str(), 0, FALSE, NBRE_Texture::TT_2D, FALSE);
    return texture;
}

NBGM_Sky::NBGM_Sky(NBGM_Context& context, int32 overlayId, nb_boolean tranparent, nb_boolean cubeTextureSupported, NBRE_SurfaceSubView* subview):
    NBGM_LayoutElement(context),
    mNBREContext(context.renderingEngine->Context()),
    mOverlayId(overlayId),
    mVisible(TRUE),
    mTransparent(tranparent),
    mCubeTextureSupported(cubeTextureSupported),
    mIndexData(NULL),
    mDistance(0),
    mSubview(subview)
{
    mPriority = -1,
    Initialize();
    SetTypeId(NBGM_LET_SKY);
    mNBGMContext.layoutManager->AddLayoutProvider(this);
}

NBGM_Sky::~NBGM_Sky()
{
    mNBGMContext.layoutManager->RemoveLayoutProvider(this);
    for(uint32 i = 0; i < mVertexData.size(); i++)
    {
        NBRE_DELETE mVertexData[i];
    }
    NBRE_DELETE mIndexData;
    NBRE_DELETE mWall;
}

static NBRE_ShaderPtr
CreateTranparentWallShader()
{
    NBRE_ShaderPtr shader = NBRE_ShaderPtr(NBRE_NEW NBRE_Shader());
    NBRE_PassPtr pass = NBRE_PassPtr(NBRE_NEW NBRE_Pass);
    pass->SetEnableLighting(FALSE);
    pass->SetEnableDepthTest(TRUE);
    pass->SetEnableDepthWrite(TRUE);
    pass->SetEnableCullFace(FALSE);
    pass->SetEnableBlend(FALSE);
    pass->SetEnableColorAlphaWrite(FALSE);
    pass->SetEnableColorRedWrite(FALSE);
    pass->SetEnableColorBlueWrite(FALSE);
    pass->SetEnableColorGreenWrite(FALSE);
    pass->SetColor(NBRE_Color(1.0f, 1.0f, 1.0f, 1.0f));
    shader->AddPass(pass);
    return shader;
}

void NBGM_Sky::Initialize()
{
    mWall = NBRE_NEW NBGM_Skywall(mNBREContext, mTransparent, mOverlayId);

    if(mTransparent)
    {
        mShader = CreateTranparentWallShader();
        mWall->SetShader(mShader);
    }
    else
    {
        CreateSkyWallShader();
    }
    mSubview->AddVisitable(this);
}

void NBGM_Sky::CreateSkyWallShader(const NBRE_String& groupName, const NBRE_String& texName)
{
    NBRE_String groupID(groupName);
    NBRE_IShaderGroupPtr shaderGroup(NBRE_NEW NBRE_DefaultShaderGroup());
    mNBREContext.mShaderManager->AddGroup(groupID, shaderGroup);
    mNBREContext.mShaderManager->SetCurrentGroup(groupID);

    NBRE_ShaderPtr shader = NBRE_ShaderPtr(NBRE_NEW NBRE_Shader());
    mNBREContext.mShaderManager->SetShader(SKY_SHADER_NAME, shader);
    SetShaderName(SKY_SHADER_NAME);

    NBRE_PassPtr pass = NBRE_PassPtr(NBRE_NEW NBRE_Pass);
    pass->SetEnableLighting(FALSE);
    pass->SetEnableDepthTest(TRUE);
    pass->SetEnableDepthWrite(TRUE);
    pass->SetEnableCullFace(TRUE);
    pass->SetEnableBlend(FALSE);
    pass->SetColor(NBRE_Color(1.0f, 1.0f, 1.0f, 1.0f));

    NBRE_String fullPath = NBGM_MakeFullPath(texName);
    NBRE_TexturePtr texture = NBGM_BuildTextureUtility::ConstructImageTexture(*mNBGMContext.renderingEngine->Context().mRenderPal, mNBGMContext.renderingEngine->Context().mPalInstance, fullPath, 0, FALSE, NBRE_Texture::TT_2D, FALSE);

    NBRE_TextureState st;
    st.SetWrapS(NBRE_TAM_REPEAT);
    st.SetWrapT(NBRE_TAM_CLAMP);
    NBRE_TextureUnit unit0;
    unit0.SetState(st);
    unit0.SetTexture(texture);
    pass->GetTextureUnits().push_back(unit0);
    shader->AddPass(pass);
}

void NBGM_Sky::CreateSkyWallShader()
{
    CreateSkyWallShader(SKY_GROUP_DAY, SKY_TEXTURE_DAY_WALL);
    CreateSkyWallShader(SKY_GROUP_NIGHT, SKY_TEXTURE_NIGHT_WALL);
}

void NBGM_Sky::SetDayNight(nb_boolean isDay)
{
    if(isDay)
    {
        mNBREContext.mShaderManager->SetCurrentGroup(SKY_GROUP_DAY);
    }
    else
    {
        mNBREContext.mShaderManager->SetCurrentGroup(SKY_GROUP_NIGHT);
    }
}

void NBGM_Sky::SetWallSize(float width, float height, float distance)
{
    mWall->SetSize(width, height, distance);
    mDistance = distance;
}

float NBGM_Sky::GetWallDistance()
{
    return mDistance;
}

void NBGM_Sky::SetShader(const NBRE_ShaderPtr& shader)
{
    mShader = shader;
}

void NBGM_Sky::SetShaderName(const NBRE_String& name)
{
     mShaderName = name;
}

void NBGM_Sky::UpdateRenderQueue(NBRE_Overlay& overlay)
{
    if(mDistance == 0)
    {
        return;
    }
    if(!mTransparent)
    {
        NBRE_ShaderPtr shader = mNBREContext.mShaderManager->GetShader(mShaderName);
        shader->GetPass(0)->GetTextureUnits().at(0).SetTransform(mTransform);
        mWall->SetShader(shader);
    }

    mWall->UpdateRenderQueue(overlay);
}

int32 NBGM_Sky::OverlayId() const
{
    return mOverlayId;
}

void NBGM_Sky::NotifyCamera(const NBRE_Camera& camera)
{
    if(mDistance == 0)
    {
        return;
    }
    if(!mTransparent)
    {
        NBRE_Vector2f dir(camera.WorldRight().x, camera.WorldRight().y);
        mTransform = NBRE_Transformationf::BuildTranslateMatrix(-dir.GetDirection()*SKY_TEXTURE_FACTOR,  0, 0);
    }

    mWall->NotifyCamera(camera);
}

nb_boolean NBGM_Sky::Visible() const
{
    return mVisible;
}

void NBGM_Sky::Accept(NBRE_Visitor& visitor)
{
    visitor.Visit(static_cast<NBRE_IRenderOperationProvider*>(this), mNBREContext.mSurfaceManager);
}


void NBGM_Sky::CreateSkyBoxShader()
{
    NBRE_String groupID(SKY_GROUP_DAY);
    NBRE_IShaderGroupPtr shaderGroup(NBRE_NEW NBRE_DefaultShaderGroup());
    mNBREContext.mShaderManager->AddGroup(groupID, shaderGroup);
    mNBREContext.mShaderManager->SetCurrentGroup(groupID);

    NBRE_ShaderPtr shader = NBRE_ShaderPtr(NBRE_NEW NBRE_Shader());
    mNBREContext.mShaderManager->SetShader(SKY_SHADER_NAME, shader);
    SetShaderName(SKY_SHADER_NAME);

    NBRE_TextureState st;
    st.SetWrapS(NBRE_TAM_REPEAT);
    st.SetWrapT(NBRE_TAM_REPEAT);
    NBRE_TextureUnit unit0;
    unit0.SetState(st);

    for(uint32 i = 0; i < SKY_PALNE_COUNT; i++)
    {
        NBRE_PassPtr pass = NBRE_PassPtr(NBRE_NEW NBRE_Pass);
        pass->SetEnableLighting(FALSE);
        pass->SetEnableDepthTest(FALSE);
        pass->SetEnableDepthWrite(FALSE);
        pass->SetEnableCullFace(TRUE);
        pass->SetEnableBlend(FALSE);
        pass->SetColor(NBRE_Color(1.0f, 1.0f, 1.0f, 1.0f));
        unit0.SetTexture(Create2DTexture(mNBREContext, i));
        pass->GetTextureUnits().push_back(unit0);
        shader->AddPass(pass);
    }
}

void NBGM_Sky::CreateSkyBox()
{
    NBRE_IRenderPal& rp = *(mNBREContext.mRenderPal);
    float texcord[] =
    {
        // north
        0.00f, 0.f,
        0.25f*2, 0.f,
        0.00f, 2.f,
        0.25f*2, 2.f,
        // west
        0.75f*2, 0.f,
        1.00f*2, 0.f,
        0.75f*2, 2.f,
        1.00f*2, 2.f,
        // south
        0.50f*2, 0.f,
        0.75f*2, 0.f,
        0.50f*2, 2.f,
        0.75f*2, 2.f,
        // east
        0.25f*2, 0.f,
        0.50f*2, 0.f,
        0.25f*2, 2.f,
        0.50f*2, 2.f,
        // up
        0.f, 0.f,
        1.f, 0.f,
        0.f, 1.f,
        1.f, 1.f
    };
    float vertices[] =
    {
        // north
       -2.f, 2.f, 0.f,
        2.f, 2.f, 0.f,
       -2.f, 2.f, 2.f,
        2.f, 2.f, 2.f,
        // west
        -2.f, -2.f, 0.f,
        -2.f,  2.f, 0.f,
        -2.f, -2.f, 2.f,
        -2.f,  2.f, 2.f,
        // south
        2.f, -2.f, 0.f,
       -2.f, -2.f, 0.f,
        2.f, -2.f, 2.f,
       -2.f, -2.f, 2.f,
        // east
        2.f,  2.f, 0.f,
        2.f, -2.f, 0.f,
        2.f,  2.f, 2.f,
        2.f, -2.f, 2.f,
        // up
       -2.f,  2.f, 2.f,
        2.f,  2.f, 2.f,
       -2.f, -2.f, 2.f,
        2.f, -2.f, 2.f
    };
    for(uint32 i = 0; i < SKY_PALNE_COUNT; i++)
    {
        NBRE_VertexDeclaration* decalration = rp.CreateVertexDeclaration();
        NBRE_VertexElement* posElem = NBRE_NEW NBRE_VertexElement(0, 0, NBRE_VertexElement::VET_FLOAT3, NBRE_VertexElement::VES_POSITION, 0);
        NBRE_VertexElement* texElem = NBRE_NEW NBRE_VertexElement(1, 0, NBRE_VertexElement::VET_FLOAT2, NBRE_VertexElement::VES_TEXTURE_COORDINATES, 0);
        decalration->GetVertexElementList().push_back(posElem);
        decalration->GetVertexElementList().push_back(texElem);

        NBRE_HardwareVertexBuffer* texcordBuf = rp.CreateVertexBuffer(sizeof(float)*2, SKYPLANE_VERTEX_COUNT, TRUE, NBRE_HardwareBuffer::HBU_STATIC);
        texcordBuf->WriteData(0, sizeof(float)*2*SKYPLANE_VERTEX_COUNT, texcord + i*2*SKYPLANE_VERTEX_COUNT, TRUE);
        NBRE_HardwareVertexBuffer* vertextBuf = rp.CreateVertexBuffer(sizeof(float)*3, SKYPLANE_VERTEX_COUNT, TRUE, NBRE_HardwareBuffer::HBU_STATIC);
        vertextBuf->WriteData(0, sizeof(float)*3*SKYPLANE_VERTEX_COUNT, vertices + i*3*SKYPLANE_VERTEX_COUNT, TRUE);

        NBRE_VertexData *data = NBRE_NEW NBRE_VertexData(SKYPLANE_VERTEX_COUNT);
        data->SetVertexDeclaration(decalration);
        data->AddBuffer(vertextBuf);
        data->AddBuffer(texcordBuf);
        mVertexData.push_back(data);
    }
    uint16 indices[] =
    {
        0, 1, 2,
        2, 1, 3
    };
    NBRE_HardwareIndexBuffer* indexBuf = rp.CreateIndexBuffer(NBRE_HardwareIndexBuffer::IT_16BIT, sizeof(indices)/sizeof(uint16), TRUE, NBRE_HardwareBuffer::HBU_STATIC);
    indexBuf->WriteData(0, sizeof(indices), indices, TRUE);
    mIndexData = NBRE_NEW NBRE_IndexData(indexBuf, 0, indexBuf->GetNumIndexes());
}

void NBGM_Sky::CreateSkyBoxShaderCube()
{
    NBRE_String groupID(SKY_GROUP_DAY);
    NBRE_IShaderGroupPtr shaderGroup(NBRE_NEW NBRE_DefaultShaderGroup());
    mNBREContext.mShaderManager->AddGroup(groupID, shaderGroup);
    mNBREContext.mShaderManager->SetCurrentGroup(groupID);

    NBRE_ShaderPtr shader = NBRE_ShaderPtr(NBRE_NEW NBRE_Shader());
    mNBREContext.mShaderManager->SetShader(SKY_SHADER_NAME, shader);
    SetShaderName(SKY_SHADER_NAME);

    NBRE_TextureState st;
    st.SetWrapS(NBRE_TAM_CLAMP);
    st.SetWrapT(NBRE_TAM_CLAMP);
    NBRE_TextureUnit unit0;
    unit0.SetState(st);
    unit0.SetTransform(NBRE_Transformationf::BuildRotateDegreesMatrix(90, NBRE_Vector3f(1, 0, 0)));

    NBRE_PassPtr pass = NBRE_PassPtr(NBRE_NEW NBRE_Pass);
    pass->SetEnableLighting(FALSE);
    pass->SetEnableDepthTest(FALSE);
    pass->SetEnableDepthWrite(FALSE);
    pass->SetEnableCullFace(FALSE);
    pass->SetEnableBlend(FALSE);
    pass->SetColor(NBRE_Color(1.0f, 1.0f, 1.0f, 1.0f));
    unit0.SetTexture(CreateCubeTexture(mNBREContext));
    pass->GetTextureUnits().push_back(unit0);
    shader->AddPass(pass);
}

void NBGM_Sky::CreateSkyBoxCube()
{
    NBRE_IRenderPal& rp = *(mNBREContext.mRenderPal);

    NBRE_VertexDeclaration* decalration = rp.CreateVertexDeclaration();
    NBRE_VertexElement* posElem = NBRE_NEW NBRE_VertexElement(0, 0, NBRE_VertexElement::VET_FLOAT3, NBRE_VertexElement::VES_POSITION, 0);
    NBRE_VertexElement* texElem = NBRE_NEW NBRE_VertexElement(1, 0, NBRE_VertexElement::VET_FLOAT3, NBRE_VertexElement::VES_TEXTURE_COORDINATES, 0);
    decalration->GetVertexElementList().push_back(posElem);
    decalration->GetVertexElementList().push_back(texElem);

    NBRE_HardwareVertexBuffer* texcordBuf = rp.CreateVertexBuffer(sizeof(float)*3, SKYBOX_VERTEX_COUNT, TRUE, NBRE_HardwareBuffer::HBU_STATIC);
    float texcord[] =
    {
        -1.f, -1.f,  0.f,
         1.f, -1.f,  0.f,
        -1.f,  1.f,  0.f,
         1.f,  1.f,  0.f,
        -1.f, -1.f,  1.f,
         1.f, -1.f,  1.f,
        -1.f,  1.f,  1.f,
         1.f,  1.f,  1.f
    };
    texcordBuf->WriteData(0, sizeof(texcord), texcord, TRUE);

    NBRE_HardwareVertexBuffer* vertextBuf = rp.CreateVertexBuffer(sizeof(float)*3, SKYBOX_VERTEX_COUNT, TRUE, NBRE_HardwareBuffer::HBU_STATIC);
    float vertices[] =
    {
        -2.f, -2.f,  0.f,
         2.f, -2.f,  0.f,
        -2.f,  2.f,  0.f,
         2.f,  2.f,  0.f,
        -2.f, -2.f,  2.f,
         2.f, -2.f,  2.f,
        -2.f,  2.f,  2.f,
         2.f,  2.f,  2.f
    };
    vertextBuf->WriteData(0, sizeof(vertices), vertices, TRUE);

    NBRE_VertexData *data = NBRE_NEW NBRE_VertexData(SKYBOX_VERTEX_COUNT);
    data->SetVertexDeclaration(decalration);
    data->AddBuffer(vertextBuf);
    data->AddBuffer(texcordBuf);
    mVertexData.push_back(data);

    uint16 indices[] =
    {
        0, 2, 1,
        1, 2, 3,
        4, 5, 6,
        5, 7, 6,
        0, 1, 4,
        1, 5, 4,
        1, 3, 5,
        3, 7, 5,
        2, 6, 3,
        3, 6, 7,
        0, 4, 2,
        2, 4, 6,
    };
    NBRE_HardwareIndexBuffer* indexBuf = rp.CreateIndexBuffer(NBRE_HardwareIndexBuffer::IT_16BIT, sizeof(indices)/sizeof(uint16), TRUE, NBRE_HardwareBuffer::HBU_STATIC);
    indexBuf->WriteData(0, sizeof(indices), indices, TRUE);
    mIndexData = NBRE_NEW NBRE_IndexData(indexBuf, 0, indexBuf->GetNumIndexes());
}

void
NBGM_Sky::UpdateRegion()
{
    if(mDistance <= 0)
    {
        return;
    }
    NBGM_LayoutManager& layoutManager = *mNBGMContext.layoutManager;
    const NBRE_Camera& camera = *layoutManager.GetCamera().get();
    NBRE_Vector3f dir = camera.WorldRight();
    float lq = (float)nsl_sqrt(dir.x * dir.x + dir.y * dir.y);
    NBRE_Matrix4x4d matrix;
    if(NBRE_Math::IsZero(lq, 0.00001f))
    {
        matrix.Identity();
    }
    else
    {
        float x = dir.x/lq;
        float y = dir.y/lq;
        matrix = NBRE_Matrix4x4d(
          x,  -y, 0.f, 0.f,
          y,   x, 0.f, 0.f,
        0.f, 0.f, 1.f, 0.f,
        0.f, 0.f, 0.f, 1.f);

    }
    NBRE_Vector3d pos = camera.WorldPosition();
    matrix = NBRE_Transformationd::BuildTranslateMatrix(pos.x, pos.y, 0)*matrix;
    NBRE_Vector3d point(0, mDistance, 0);
    NBRE_Transformationd::Transform(matrix, point);
    NBRE_Vector2d screenPos;
    if (layoutManager.WorldToScreen(point, screenPos))
    {
        NBRE_AxisAlignedBox2d rc = layoutManager.GetScreenRect();
        double dy = rc.maxExtend.y - screenPos.y;
        if (dy > 0)
        {
            dy *= 1.2;
            rc.minExtend.y = rc.maxExtend.y - dy;
            layoutManager.GetLayoutBuffer().UpdateRegion(rc, SKY_UPDATE_MASK);
        }
    }
}

void
NBGM_Sky::SetOverlayId(int32 overlayId)
{
    mWall->SetOverlayId(overlayId);
    mOverlayId = overlayId;
}

void
NBGM_Sky::UpdateLayoutList(NBGM_LayoutElementList& layoutElements)
{
    layoutElements.push_back(this);
}