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
#include "nbgmavatar.h"
#include "nbretransformation.h"
#include "nbreoverlay.h"
#include "nbrerenderpal.h"
#include "nbrehardwarebuffer.h"
#include "nbreintersection.h"
#include "nbremath.h"
#include "nbretypeconvert.h"
#include "nbrescenemanager.h"
#include "nbgmbinloader.h"
#include "nbrepngtextureimage.h"
#include "nbrerenderengine.h"
#include "nbgmprotected.h"
#include "nbgmconst.h"
#include "nbrememorystream.h"
#include "nbretransformutil.h"
#include "nbgmbuildutility.h"

static const char* FOLLOW_ME_ICON = "TEXTURE/FOLLOW_ME_ICON";
static const char* LOCATE_ME_ICON = "TEXTURE/LOCATE_ME_ICON";
static const char* NAV_ARROW_AVATAR_TEX = "TEXTURE/ARROW_AVATAR_TEX";

static NBRE_EntityPtr
CreateAvatarEntity(NBGM_Context& context, int32 overlayId, nb_boolean isNav)
{
    const uint32 VertexCount = 4;
    const uint32 indexCount = 6;

    float vertices[] =
    {
       -0.5f, -0.5f, 0.f,
        0.5f, -0.5f, 0.f,
        0.5f,  0.5f, 0.f,
       -0.5f,  0.5f, 0.f
    };

    float texCoords[] =
    {
        1.f, 0.f,
        1.f, 1.f,
        0.f, 1.f,
        0.f, 0.f
    };

    uint16 indices[] =
    {
        0, 1, 2,
        0, 2, 3
    };

    if(isNav)
    {
        // Adjust nav avartar size to make it look as same as before.
        // Because previous avartar uses bin file to assign the mesh, current avatar uses code to assign mesh.
        for(uint32 i = 0; i < VertexCount * 3; ++i)
        {
            vertices[i] *= 2.0f * context.GetModelScaleFactor();
        }
    }

    NBRE_IRenderPal *renderPal = context.renderingEngine->Context().mRenderPal;
    NBRE_HardwareVertexBuffer* vertextBuff = renderPal->CreateVertexBuffer(sizeof(float)*3, VertexCount, TRUE, NBRE_HardwareBuffer::HBU_STATIC);
    vertextBuff->WriteData(0, sizeof(float)*3*VertexCount, vertices, TRUE);
    NBRE_HardwareVertexBuffer* texCoordBuff = renderPal->CreateVertexBuffer(sizeof(float)*2, VertexCount, TRUE, NBRE_HardwareBuffer::HBU_STATIC);
    texCoordBuff->WriteData(0, sizeof(float)*2*VertexCount, texCoords, TRUE);

    NBRE_VertexDeclaration* decalration = renderPal->CreateVertexDeclaration();
    NBRE_VertexElement* posElem = NBRE_NEW NBRE_VertexElement(0, 0, NBRE_VertexElement::VET_FLOAT3, NBRE_VertexElement::VES_POSITION, 0);
    decalration->GetVertexElementList().push_back(posElem);
    NBRE_VertexElement* texCoordElem = NBRE_NEW NBRE_VertexElement(1, 0, NBRE_VertexElement::VET_FLOAT2, NBRE_VertexElement::VES_TEXTURE_COORDINATES, 0);
    decalration->GetVertexElementList().push_back(texCoordElem);

    NBRE_VertexData* vertexData = NBRE_NEW NBRE_VertexData(0);
    vertexData->AddBuffer(vertextBuff);
    vertexData->AddBuffer(texCoordBuff);
    vertexData->SetVertexDeclaration(decalration);

    NBRE_HardwareIndexBuffer* indexBuff = renderPal->CreateIndexBuffer(NBRE_HardwareIndexBuffer::IT_16BIT, indexCount, TRUE, NBRE_HardwareBuffer::HBU_STATIC);
    indexBuff->WriteData(0, sizeof(uint16)*indexCount, indices, TRUE);
    NBRE_IndexData* indexData = NBRE_NEW NBRE_IndexData(indexBuff, 0, indexBuff->GetNumIndexes());

    NBRE_MeshPtr mesh(NBRE_NEW NBRE_Mesh(vertexData));
    mesh->CreateSubMesh(indexData, NBRE_PMT_TRIANGLE_LIST);
    mesh->CalculateBoundingBox();
    NBRE_ModelPtr model(NBRE_NEW NBRE_Model(mesh));
    NBRE_EntityPtr entity(NBRE_NEW NBRE_Entity(context.renderingEngine->Context(), model, overlayId));

    return entity;
}

static NBRE_ShaderPtr
CreateHaloShader(NBRE_Context& context, const NBRE_Color& clr, uint8 /*edgeWidth*/)
{
    NBRE_PassPtr pass(NBRE_NEW NBRE_Pass());
    pass->SetEnableDepthTest(TRUE);
    pass->SetEnableDepthWrite(FALSE);
    pass->SetEnableCullFace(FALSE);
    pass->SetEnableBlend(TRUE);
    NBRE_ShaderPtr shader(NBRE_NEW NBRE_Shader());

    NBRE_Image *image = NBRE_NEW NBRE_Image(1, 1, NBRE_PF_R8G8B8A8);
    uint8 *buffer = image->GetImageData();
    buffer[0] = static_cast<uint8>(clr.r*255);
    buffer[1] = static_cast<uint8>(clr.g*255);
    buffer[2] = static_cast<uint8>(clr.b*255);
    buffer[3] = static_cast<uint8>(clr.a*255);

    NBRE_Image **images = NBRE_NEW NBRE_Image*[1];
    images[0] = image;
    NBRE_TexturePtr texture(context.mRenderPal->CreateTexture(images, 1, 1, FALSE, NBRE_Texture::TT_2D, ""));

    NBRE_TextureUnit unit;
    unit.SetState(NBRE_TextureState(NBRE_TFT_POINT, NBRE_TFT_POINT, NBRE_TAM_CLAMP, NBRE_TAM_CLAMP, FALSE));
    unit.SetTexture(texture);
    pass->GetTextureUnits().push_back(unit);

    shader->AddPass(pass);
    return shader;
}

static NBRE_EntityPtr
CreateHaloEntity(NBGM_Context& context, int32 overlayId)
{
    const uint32 N = 90;
    const float Det = NBRE_Math::PiTwo/N;
    const uint32 VertexCount = N+2;
    const uint32 indexCount0 = N*3;
    const uint32 indexCount1 = N+1;

    float* vertices = NBRE_NEW float[VertexCount*3];
    float* texCoords = NBRE_NEW float[VertexCount*2];
    uint16* indices0 = NBRE_NEW uint16[indexCount0];
    uint16* indices1 = NBRE_NEW uint16[indexCount1];
    nsl_memset(vertices, 0, VertexCount*3*sizeof(float));
    nsl_memset(texCoords, 0, VertexCount*2*sizeof(float));
    nsl_memset(indices0, 0, indexCount0*sizeof(uint16));
    nsl_memset(indices1, 0, indexCount1*sizeof(uint16));

    // build vertex buffer
    for(uint32 i = 1; i < VertexCount; i++)
    {
        const uint32 index  = i*3;
        const float angle = Det*(i-1);
        vertices[index]   = nsl_cos(angle);
        vertices[index+1] = nsl_sin(angle);
        vertices[index+2] = 0;
    }

    // build index buffer for the inner of the halo
    for(uint16 i = 0, j = 1; i < N; i++, j++)
    {
       const uint16 index = i*3;
       indices0[index]   = 0;
       indices0[index+1] = j;
       indices0[index+2] = j+1;
    }

     // build index buffer for the outline of the halo
    for(uint16 i = 0; i < indexCount1; i++)
    {
        indices1[i] = i+1;
    }

    NBRE_IRenderPal *renderPal = context.renderingEngine->Context().mRenderPal;
    NBRE_HardwareVertexBuffer* vertextBuff = renderPal->CreateVertexBuffer(sizeof(float)*3, VertexCount, TRUE, NBRE_HardwareBuffer::HBU_STATIC);
    vertextBuff->WriteData(0, sizeof(float)*3*VertexCount, vertices, TRUE);
    NBRE_HardwareVertexBuffer* texCoordBuff = renderPal->CreateVertexBuffer(sizeof(float)*2, VertexCount, TRUE, NBRE_HardwareBuffer::HBU_STATIC);
    texCoordBuff->WriteData(0, sizeof(float)*2*VertexCount, texCoords, TRUE);

    NBRE_VertexDeclaration* decalration = renderPal->CreateVertexDeclaration();
    NBRE_VertexElement* posElem = NBRE_NEW NBRE_VertexElement(0, 0, NBRE_VertexElement::VET_FLOAT3, NBRE_VertexElement::VES_POSITION, 0);
    decalration->GetVertexElementList().push_back(posElem);
    NBRE_VertexElement* texCoordElem = NBRE_NEW NBRE_VertexElement(1, 0, NBRE_VertexElement::VET_FLOAT2, NBRE_VertexElement::VES_TEXTURE_COORDINATES, 0);
    decalration->GetVertexElementList().push_back(texCoordElem);

    NBRE_VertexData* vertexData = NBRE_NEW NBRE_VertexData(0);
    vertexData->AddBuffer(vertextBuff);
    vertexData->AddBuffer(texCoordBuff);
    vertexData->SetVertexDeclaration(decalration);

    NBRE_HardwareIndexBuffer* indexBuff0 = renderPal->CreateIndexBuffer(NBRE_HardwareIndexBuffer::IT_16BIT, indexCount0, TRUE, NBRE_HardwareBuffer::HBU_STATIC);
    indexBuff0->WriteData(0, sizeof(uint16)*indexCount0, indices0, TRUE);
    NBRE_IndexData* indexData0 = NBRE_NEW NBRE_IndexData(indexBuff0, 0, indexBuff0->GetNumIndexes());

    NBRE_HardwareIndexBuffer* indexBuff1 = renderPal->CreateIndexBuffer(NBRE_HardwareIndexBuffer::IT_16BIT, indexCount1, TRUE, NBRE_HardwareBuffer::HBU_STATIC);
    indexBuff1->WriteData(0, sizeof(uint16)*indexCount1, indices1, TRUE);
    NBRE_IndexData* indexData1 = NBRE_NEW NBRE_IndexData(indexBuff1, 0, indexBuff1->GetNumIndexes());

    NBRE_MeshPtr mesh(NBRE_NEW NBRE_Mesh(vertexData));
    mesh->CreateSubMesh(indexData0, NBRE_PMT_TRIANGLE_LIST);
    mesh->CreateSubMesh(indexData1, NBRE_PMT_LINE_STRIP);
    mesh->CalculateBoundingBox();
    NBRE_ModelPtr model(NBRE_NEW NBRE_Model(mesh));
    NBRE_EntityPtr entity(NBRE_NEW NBRE_Entity(context.renderingEngine->Context(), model, overlayId));

    NBRE_DELETE_ARRAY vertices;
    NBRE_DELETE_ARRAY texCoords;
    NBRE_DELETE_ARRAY indices0;
    NBRE_DELETE_ARRAY indices1;
    return entity;
}

static NBRE_ShaderPtr
CreateMapShader(NBGM_Context& nbgmContext, const NBRE_String& path, NBRE_Vector2d &entityPixelSize)
{
    NBRE_TexturePtr texture = NBGM_BuildTextureUtility::ConstructImageTexture(*nbgmContext.renderingEngine->Context().mRenderPal, nbgmContext.renderingEngine->Context().mPalInstance, path, 0, FALSE, NBRE_Texture::TT_2D, TRUE);

    entityPixelSize.x = texture->GetOriginalWidth()/UX_IMAGE_DPI;
    entityPixelSize.y = texture->GetOriginalHeight()/UX_IMAGE_DPI;

    NBRE_PassPtr pass(NBRE_NEW NBRE_Pass());
    pass->SetEnableDepthTest(TRUE);
    pass->SetEnableDepthWrite(FALSE);
    pass->SetEnableCullFace(FALSE);
    pass->SetEnableBlend(TRUE);
    NBRE_TextureUnit unit;
    unit.SetState(NBRE_TextureState(NBRE_TFT_MIPMAP_TRILINEAR, NBRE_TFT_LINEAR, NBRE_TAM_CLAMP, NBRE_TAM_CLAMP, FALSE));
    unit.SetTexture(texture);
    pass->GetTextureUnits().push_back(unit);
    NBRE_ShaderPtr shader(NBRE_NEW NBRE_Shader());
    shader->AddPass(pass);
    return shader;
}

static NBRE_ShaderPtr
CreateMapShader(NBGM_Context& nbgmContext, NBRE_IOStream* stream, NBRE_Vector2d &entityPixelSize)
{
    NBRE_ITextureImage* textureImage = NBRE_NEW NBRE_PngTextureImage(*(nbgmContext.renderingEngine->Context().mPalInstance), stream, 0, FALSE);
    NBRE_TexturePtr texture(nbgmContext.renderingEngine->Context().mRenderPal->CreateTexture(textureImage, NBRE_Texture::TT_2D, TRUE, "") );

    entityPixelSize.x = texture->GetOriginalWidth()/UX_IMAGE_DPI;
    entityPixelSize.y = texture->GetOriginalHeight()/UX_IMAGE_DPI;

    NBRE_PassPtr pass(NBRE_NEW NBRE_Pass());
    pass->SetEnableDepthTest(TRUE);
    pass->SetEnableDepthWrite(FALSE);
    pass->SetEnableCullFace(FALSE);
    pass->SetEnableBlend(TRUE);
    NBRE_TextureUnit unit;
    unit.SetState(NBRE_TextureState(NBRE_TFT_MIPMAP_TRILINEAR, NBRE_TFT_LINEAR, NBRE_TAM_CLAMP, NBRE_TAM_CLAMP, FALSE));
    unit.SetTexture(texture);
    pass->GetTextureUnits().push_back(unit);
    NBRE_ShaderPtr shader(NBRE_NEW NBRE_Shader());
    shader->AddPass(pass);
    return shader;
}

static void
CalculateBaseSize(NBRE_Context& /*context*/, NBRE_EntityPtr entity, double& baseSize)
{
    NBRE_AxisAlignedBox3d aabb = NBRE_TypeConvertd::Convert(entity->GetModel()->GetMesh()->AxisAlignedBox());
    NBRE_Vector3d size = aabb.GetSize();
    if (size.x > size.y)
    {
        baseSize = size.x;
    }
    else
    {
        baseSize = size.y;
    }
}

NBGM_Avatar::NBGM_Avatar(NBGM_Context& nbgmContext,
                                 NBRE_SceneManager* sceneManager,
                                 NBRE_SurfaceSubView* subView,
                                 NBRE_DefaultOverlayManager<DrawOrderStruct>& overlayManager,
                                 DrawOrderStruct drawOrder,
                                 int32 glowOverlayId,
                                 NBGM_AvatarMode mode,
                                 float modelSize,
                                 float referenceHeight,
                                 const NBRE_Point3f& posOffset,
                                 const NBRE_String& id)
    :NBGM_LayoutElement(nbgmContext)
    ,mSceneManager(sceneManager)
    ,mSubView(subView)
    ,mOverlayManager(overlayManager)
    ,mModelScale(1.0f)
    ,mModelSize(modelSize)
    ,mRefHeight(referenceHeight)
    ,mPosOffset(posOffset)
    ,mMode(mode)
    ,mState(NBGM_AS_DISABLED)
    ,mNeedUpdate(TRUE)
    ,mAvailableScreenPt(TRUE)
    ,mShowHalo(TRUE)
    ,mHaloOutlineClr(NAV_VECTOR_AVATAR_HALO_OUTLINE_COLOR)
    ,mHaloInnerClr(NAV_VECTOR_AVATAR_HALO_INNER_COLOR)
    ,mMapEntityBaseSize(0.0)
    ,mMapEntityFollowMePixelSize(0.0, 0.0)
    ,mMapEntityLocateMePixelSize(0.0, 0.0)
    ,mExtendType(NBGM_TMBET_NONE)
    ,mFollowMeOffset(0.5f, 0.5f)
    ,mLocateMeOffset(0.5f, 0.5f)
    ,mID(id)
    ,mHaloEdgeWidth(1)
{
    nsl_memset(&mLocation, 0, sizeof(NBGM_Location64));
    mOverlayHaloID = mOverlayManager.AssignOverlayId(drawOrder);
    drawOrder.layer += 1;
    mOverlayID = mOverlayManager.AssignOverlayId(drawOrder);
    mGlowOverlayId = glowOverlayId;
}

NBGM_Avatar::~NBGM_Avatar()
{
    if(mExtendType != NBGM_TMBET_NONE)
    {
        mExtendAvatarNode->RemoveChild(mExtendSubAvatarNode.get());
        mParentNode->RemoveChild(mExtendAvatarNode.get());
        mParentNode->RemoveChild(mExtendHaloNode.get());
    }
    mAvatarNode->RemoveChild(mSubAvatarNode.get());
    mParentNode->RemoveChild(mAvatarNode.get());
    mParentNode->RemoveChild(mHaloNode.get());
    mSceneManager->RootSceneNode()->RemoveChild(mParentNode.get());
}

void
NBGM_Avatar::AddToSence()
{
    NBGM_LayoutElement::AddToSence();

    mParentNode = NBRE_NodePtr(NBRE_NEW NBRE_Node());
    mSceneManager->RootSceneNode()->AddChild(mParentNode);

    mAvatarNode = NBRE_NodePtr(NBRE_NEW NBRE_Node());
    mExtendAvatarNode = NBRE_NodePtr(NBRE_NEW NBRE_Node());
    mParentNode->AddChild(mAvatarNode);

    mHaloNode = NBRE_NodePtr(NBRE_NEW NBRE_Node());
    mExtendHaloNode = NBRE_NodePtr(NBRE_NEW NBRE_Node());
    mParentNode->AddChild(mHaloNode);

    mSubAvatarNode = NBRE_NodePtr(NBRE_NEW NBRE_Node());
    mExtendSubAvatarNode = NBRE_NodePtr(NBRE_NEW NBRE_Node());
    mAvatarNode->AddChild(mSubAvatarNode);
    mExtendAvatarNode->AddChild(mExtendSubAvatarNode);

    mMapFollowMeShader = CreateMapShader(mNBGMContext, NBGM_MakeFullPath(FOLLOW_ME_ICON), mMapEntityFollowMePixelSize);
    mMapLocateMeShader = CreateMapShader(mNBGMContext, NBGM_MakeFullPath(LOCATE_ME_ICON), mMapEntityLocateMePixelSize);
    mHaloOutlineShader = CreateHaloShader(mNBGMContext.renderingEngine->Context(), mHaloOutlineClr, mHaloEdgeWidth);
    mHaloInnerShader = CreateHaloShader(mNBGMContext.renderingEngine->Context(), mHaloInnerClr, mHaloEdgeWidth);
    NBRE_Vector<NBRE_ModelPtr> models;
    NBRE_String path = NBGM_MakeFullPath("CAR_AVATAR_MODEL");
    NBGM_LoadBinFile(*mNBGMContext.renderingEngine->Context().mRenderPal, mNBGMContext.renderingEngine->Context().mPalInstance,
        mNBGMContext.GetModelScaleFactor(), path.c_str(), models);
    mCarEntity = NBRE_EntityPtr(NBRE_NEW NBRE_Entity(mNBGMContext.renderingEngine->Context(), models.back(), mOverlayID));

    mArrowEntity = CreateAvatarEntity(mNBGMContext, mOverlayID, true);
    NBRE_Vector2d pixelSize;
    NBRE_ShaderPtr navArrowAvatarShader = CreateMapShader(mNBGMContext, NBGM_MakeFullPath(NAV_ARROW_AVATAR_TEX), pixelSize);
    mArrowEntity->GetSubEntity(0).SetShader(navArrowAvatarShader);

    mMapEntity = CreateAvatarEntity(mNBGMContext, mOverlayID, false);

    mAvatarNode->AttachObject(mCarEntity);
    mAvatarNode->AttachObject(mArrowEntity);
    mSubAvatarNode->AttachObject(mMapEntity);

    mExtendCarEntity = NBRE_EntityPtr(mCarEntity->Duplicate());
    mExtendArrowEntity = NBRE_EntityPtr(mArrowEntity->Duplicate());
    mExtendMapEntity = NBRE_EntityPtr(mMapEntity->Duplicate());
    mExtendAvatarNode->AttachObject(mExtendCarEntity);
    mExtendAvatarNode->AttachObject(mExtendArrowEntity);
    mExtendSubAvatarNode->AttachObject(mExtendMapEntity);

    mHaloEntity = CreateHaloEntity(mNBGMContext, mOverlayHaloID);
    mExtendHaloEntity = NBRE_EntityPtr(mHaloEntity->Duplicate());
    mHaloNode->AttachObject(mHaloEntity);
    mExtendHaloNode->AttachObject(mExtendHaloEntity);

    nsl_memset(&mLocation, 0, sizeof(mLocation));

    NBRE_Context& cont = mNBGMContext.renderingEngine->Context();
    CalculateBaseSize(cont, mMapEntity, mMapEntityBaseSize);


    NBRE_EntityPtr glowEntity = CreateAvatarEntity(mNBGMContext, mGlowOverlayId, false);
    mSubAvatarNode->AttachObject(glowEntity);
    glowEntity->GetSubEntity(0).SetShader(mMapFollowMeShader);
    NBRE_EntityPtr extendGlowEntity = NBRE_EntityPtr(glowEntity->Duplicate());
    mExtendSubAvatarNode->AttachObject(extendGlowEntity);
    extendGlowEntity->GetSubEntity(0).SetShader(mMapFollowMeShader);
}

void
NBGM_Avatar::SetLocation(const NBGM_Location64& location)
{
    mLocation = location;
    mLocation.position.x = mNBGMContext.WorldToModel(location.position.x);
    mLocation.position.y = mNBGMContext.WorldToModel(location.position.y);
    mLocation.position.z = mNBGMContext.WorldToModel(location.position.z);
    mLocation.accuracy = METER_TO_MERCATOR(mNBGMContext.WorldToModel(location.accuracy));
    mNeedUpdate = TRUE;
}

const NBGM_Location64&
NBGM_Avatar::GetLocation()
{
    return mLocation;
}

void
NBGM_Avatar::SetScale(float scaleValue)
{
    mModelScale = scaleValue;
    mNeedUpdate = TRUE;
}

void
NBGM_Avatar::SetMode(NBGM_AvatarMode mode)
{
    mMode = mode;
    mNeedUpdate = TRUE;
}

void
NBGM_Avatar::SetState(NBGM_AvatarState state)
{
    mState = state;
    mNeedUpdate = TRUE;
}

nb_boolean
NBGM_Avatar::GetScreenPos(NBRE_Point2f& screenPos) const
{
    if(IsExtendNodeInFrustum())
    {
        NBRE_Point2d sp;
        if (mNBGMContext.layoutManager->WorldToScreen(mExtendAvatarNode->WorldPosition(), sp))
        {
            screenPos.x = static_cast<float>(sp.x);
            screenPos.y = static_cast<float>(sp.y);
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    NBRE_Point2d sp;
    if (mNBGMContext.layoutManager->WorldToScreen(mAvatarNode->WorldPosition(), sp))
    {
        screenPos.x = static_cast<float>(sp.x);
        screenPos.y = static_cast<float>(sp.y);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

void
NBGM_Avatar::GetPolygon(Polygon2f polygons[])
{
    NBRE_Vector2f sp[8];
    for (uint32 i = 0; i < 8; ++i)
    {
        sp[i] = NBRE_TypeConvertf::Convert(mScreenPoints[i]);
    }

    uint32 index = 0;

    polygons[index].push_back(sp[0]);
    polygons[index].push_back(sp[1]);
    polygons[index].push_back(sp[5]);
    polygons[index].push_back(sp[4]);
    ++index;

    polygons[index].push_back(sp[1]);
    polygons[index].push_back(sp[2]);
    polygons[index].push_back(sp[6]);
    polygons[index].push_back(sp[5]);
    ++index;

    polygons[index].push_back(sp[3]);
    polygons[index].push_back(sp[2]);
    polygons[index].push_back(sp[6]);
    polygons[index].push_back(sp[7]);
    ++index;

    polygons[index].push_back(sp[0]);
    polygons[index].push_back(sp[3]);
    polygons[index].push_back(sp[7]);
    polygons[index].push_back(sp[4]);
    ++index;

    polygons[index].push_back(sp[4]);
    polygons[index].push_back(sp[5]);
    polygons[index].push_back(sp[6]);
    polygons[index].push_back(sp[7]);
    ++index;
}

void
NBGM_Avatar::SetModelSale(float s)
{
    mModelScale = s;
}

void
NBGM_Avatar::EnableHalo(bool showHalo)
{
    mShowHalo = showHalo?TRUE:FALSE;
    mNeedUpdate = TRUE;
}

NBRE_AxisAlignedBox3d
NBGM_Avatar::AxisAlignedBox() const
{
    if(IsExtendNodeInFrustum())
    {
        mExtendAvatarNode->UpdatePosition();
        switch (mMode)
        {
        case NBGM_AM_NAV_CAR:
            return mExtendCarEntity->AxisAlignedBox();
        case NBGM_AM_NAV_BYCICLY:
            return mExtendArrowEntity->AxisAlignedBox();
        case NBGM_AM_MAP_FOLLOW_ME:
            return mExtendMapEntity->AxisAlignedBox();
        default:
            nbre_assert(FALSE);
            return NBRE_AxisAlignedBox3d();
        }
    }
    mAvatarNode->UpdatePosition();
    switch (mMode)
    {
    case NBGM_AM_NAV_CAR:
        return mCarEntity->AxisAlignedBox();
    case NBGM_AM_NAV_BYCICLY:
        return mArrowEntity->AxisAlignedBox();
    case NBGM_AM_MAP_FOLLOW_ME:
        return mMapEntity->AxisAlignedBox();
    default:
        nbre_assert(FALSE);
        return NBRE_AxisAlignedBox3d();
    }
}

NBRE_NoneAxisAlignedBox3d
NBGM_Avatar::ObjectAlignedBox() const
{
    if(IsExtendNodeInFrustum())
    {
        mExtendAvatarNode->UpdatePosition();
        switch (mMode)
        {
        case NBGM_AM_NAV_CAR:
            return mExtendCarEntity->BoundingBox();
        case NBGM_AM_NAV_BYCICLY:
            return mExtendArrowEntity->BoundingBox();
        case NBGM_AM_MAP_FOLLOW_ME:
            return mExtendMapEntity->BoundingBox();
        default:
            nbre_assert(FALSE);
            return NBRE_NoneAxisAlignedBox3d();
        }
    }
    mAvatarNode->UpdatePosition();
    switch (mMode)
    {
        case NBGM_AM_NAV_CAR:
            return mCarEntity->BoundingBox();
        case NBGM_AM_NAV_BYCICLY:
            return mArrowEntity->BoundingBox();
        case NBGM_AM_MAP_FOLLOW_ME:
            return mMapEntity->BoundingBox();
        default:
            nbre_assert(FALSE);
            return NBRE_NoneAxisAlignedBox3d();
    }
}

void
NBGM_Avatar::Update(float cameraDistance)
{
    NBRE_Point3d avatarPos(mLocation.position.x + mPosOffset.x, mLocation.position.y + mPosOffset.y, mLocation.position.z + mPosOffset.z);
    NBRE_Vector3d offset;

    switch (mMode)
    {
    case NBGM_AM_NAV_CAR:
    case NBGM_AM_NAV_BYCICLY:
        {
            float s = mModelScale * mModelSize * (cameraDistance/mRefHeight);
            mAvatarNode->SetScale(NBRE_Vector3f(s, s, s));
            mExtendAvatarNode->SetScale(NBRE_Vector3f(s, s, s));
        }
        break;
    case NBGM_AM_MAP_FOLLOW_ME:
        {
            double ppu = 0;
            if (mNBGMContext.transUtil->PixelsPerUnit(avatarPos, ppu))
            {
                float sx = 0.f;
                float sy = 0.f;
                if (mState == NBGM_AS_MAP_STAND_BY)
                {
                    sx = (float)(mMapEntityLocateMePixelSize.y * NBGM_GetConfig()->dpi / (mMapEntityBaseSize * ppu));
                    sy = (float)(mMapEntityLocateMePixelSize.x * NBGM_GetConfig()->dpi / (mMapEntityBaseSize * ppu));
                    offset.x = mLocateMeOffset.y - 0.5;
                    offset.y = mLocateMeOffset.x - 0.5;

                }
                else
                {
                    sx = (float)(mMapEntityFollowMePixelSize.y * NBGM_GetConfig()->dpi / (mMapEntityBaseSize * ppu));
                    sy = (float)(mMapEntityFollowMePixelSize.x * NBGM_GetConfig()->dpi / (mMapEntityBaseSize * ppu));
                    offset.x = mFollowMeOffset.y - 0.5;
                    offset.y = mFollowMeOffset.x - 0.5;
                }
                sx *= mModelScale;
                sy *= mModelScale;
                mAvatarNode->SetScale(NBRE_Vector3f(sx, sy, sx));
                mExtendAvatarNode->SetScale(NBRE_Vector3f(sx, sy, sx));
            }
        }
        break;
    default:
        nsl_assert(FALSE);
    }

    mSubAvatarNode->SetPosition(offset);
    mExtendSubAvatarNode->SetPosition(offset);

    if (!mNeedUpdate)
    {
        return;
    }

    mCarEntity->SetVisible(FALSE);
    mArrowEntity->SetVisible(FALSE);
    mMapEntity->SetVisible(FALSE);
    mHaloEntity->SetVisible(FALSE);

    mExtendCarEntity->SetVisible(FALSE);
    mExtendArrowEntity->SetVisible(FALSE);
    mExtendMapEntity->SetVisible(FALSE);
    mExtendHaloEntity->SetVisible(FALSE);

    if(mState == NBGM_AS_DISABLED)
    {
        return;
    }

    if(mShowHalo && mLocation.accuracy != 0)
    {
        mHaloEntity->SetVisible(TRUE);
        mExtendHaloEntity->SetVisible(TRUE);
        mHaloEntity->GetSubEntity(0).SetShader(mHaloInnerShader);
        mHaloEntity->GetSubEntity(1).SetShader(mHaloOutlineShader);
        mExtendHaloEntity->GetSubEntity(0).SetShader(mHaloInnerShader);
        mExtendHaloEntity->GetSubEntity(1).SetShader(mHaloOutlineShader);
    }

    switch (mMode)
    {
        case NBGM_AM_NAV_CAR:
            mCarEntity->SetVisible(TRUE);
            mExtendCarEntity->SetVisible(TRUE);
            break;
        case NBGM_AM_NAV_BYCICLY:
            mArrowEntity->SetVisible(TRUE);
            mExtendArrowEntity->SetVisible(TRUE);
            break;
        case NBGM_AM_MAP_FOLLOW_ME:
            mMapEntity->SetVisible(TRUE);
            mExtendMapEntity->SetVisible(TRUE);
            if (mState == NBGM_AS_MAP_STAND_BY)
            {
                mMapEntity->GetSubEntity(0).SetShader(mMapLocateMeShader);
                mExtendMapEntity->GetSubEntity(0).SetShader(mMapLocateMeShader);
            }
            else
            {
                mMapEntity->GetSubEntity(0).SetShader(mMapFollowMeShader);
                mExtendMapEntity->GetSubEntity(0).SetShader(mMapFollowMeShader);
            }
            break;
        default:
            nsl_assert(FALSE);
    }

    mAvatarNode->SetOrientation(NBRE_Orientation(NBRE_Vector3f(0, 0, 1), static_cast<float>(90-mLocation.heading)));
    mExtendAvatarNode->SetOrientation(NBRE_Orientation(NBRE_Vector3f(0, 0, 1), static_cast<float>(90-mLocation.heading)));
    mParentNode->SetPosition(avatarPos);
    float s = static_cast<float>(mLocation.accuracy);
    mHaloNode->SetScale(NBRE_Vector3f(s, s, s));
    mExtendHaloNode->SetScale(NBRE_Vector3f(s, s, s));

    NBRE_NoneAxisAlignedBox3d aabb = ObjectAlignedBox();
    NBRE_Vector3d aabbCorners[8];
    aabb.GetCorners(aabbCorners);

    mAvailableScreenPt = TRUE;
    for (uint32 i = 0; i < 8; ++i)
    {
        mAvailableScreenPt = mNBGMContext.transUtil->WorldToScreen(aabbCorners[i], mScreenPoints[i]);
        if(!mAvailableScreenPt)
        {
            break;
        }
    }

    mNeedUpdate = FALSE;
    mUpdated = TRUE;
}

nb_boolean
NBGM_Avatar::IsScreenPointAvailable()
{
    return mAvailableScreenPt;
}

nb_boolean
NBGM_Avatar::IsElementVisible()
{
    return TRUE;
}

void
NBGM_Avatar::UpdateRegion()
{
    //note: use use here
}

nb_boolean
NBGM_Avatar::IsElementRegionAvailable()
{
    return TRUE;
}

NBRE_Vector3d
NBGM_Avatar::GetPosition()
{
    return mParentNode->Position();
}

void
NBGM_Avatar::NotifyExtend(const NBGM_TileMapBoundaryExtendType& extendType)
{
    if(mExtendType != extendType)
    {
        switch(extendType)
        {
        case NBGM_TMBET_NONE:
            mParentNode->RemoveChild(mExtendAvatarNode.get());
            mParentNode->RemoveChild(mExtendHaloNode.get());
            break;
        case NBGM_TMBET_LEFT:
            mExtendAvatarNode->SetPosition(NBRE_Point3f(mNBGMContext.WorldToModel(-NBRE_Math::PiTwo), 0.0f, 0.0f));
            mExtendHaloNode->SetPosition(NBRE_Point3f(mNBGMContext.WorldToModel(-NBRE_Math::PiTwo), 0.0f, 0.0f));
            if(mExtendType == NBGM_TMBET_NONE)
            {
                mParentNode->AddChild(mExtendAvatarNode);
                mParentNode->AddChild(mExtendHaloNode);
            }
            break;
        case NBGM_TMBET_RIGHT:
            mExtendAvatarNode->SetPosition(NBRE_Point3f(mNBGMContext.WorldToModel(NBRE_Math::PiTwo), 0.0f, 0.0f));
            mExtendHaloNode->SetPosition(NBRE_Point3f(mNBGMContext.WorldToModel(NBRE_Math::PiTwo), 0.0f, 0.0f));
            if(mExtendType == NBGM_TMBET_NONE)
            {
                mParentNode->AddChild(mExtendAvatarNode);
                mParentNode->AddChild(mExtendHaloNode);
            }
            break;
        default:
            break;
        }
        mExtendType = extendType;
        mNeedUpdate = TRUE;
    }
}

nb_boolean
NBGM_Avatar::IsExtendNodeInFrustum() const
{
    return !mNBGMContext.layoutManager->IsInFrustum(mExtendAvatarNode->WorldPosition())
        || mExtendType == NBGM_TMBET_NONE
        ? FALSE : TRUE;
}

bool NBGM_Avatar::GetScreenPosition( float& screenX, float& screenY ) const
{
    NBRE_Point2f screenPosition;
    if (GetScreenPos(screenPosition))
    {
        screenPosition.y = mSubView->Viewport().GetRect().GetSize().y - screenPosition.y;
        screenX = screenPosition.x;
        screenY = screenPosition.y;
        return true;
    }
    return false;
}

bool NBGM_Avatar::SelectAndTrack( float x, float y )
{
    NBRE_Ray3d ray = mSubView->Viewport().GetRay(x, y);
    NBRE_AxisAlignedBox3d aabb = AxisAlignedBox();
    return NBRE_Intersectiond::HitTest(aabb, ray, NULL)?true:false;
}

const std::string& NBGM_Avatar::ID() const
{
    return mID;
}

nb_boolean
NBGM_Avatar::IsAvatarInScreen(const NBRE_Vector3d& position)
{
    if ( !mUpdated )
    {
        return TRUE;
    }

    NBGM_LayoutManager* layoutManager = mNBGMContext.layoutManager;

    NBRE_Vector3d pos(mNBGMContext.WorldToModel(position.x), mNBGMContext.WorldToModel(position.y), mNBGMContext.WorldToModel(position.z));
    NBRE_Vector3d positions[] = {pos, pos + mExtendAvatarNode->Position()};

    for (uint32 i = 0; i < 2; ++i)
    {
        NBRE_Vector2d screenPt;
        if (layoutManager->WorldToScreen(positions[i], screenPt))
        {
            const NBRE_AxisAlignedBox2d& screenRect = layoutManager->GetScreenRect();
            double sizeX = (mState == NBGM_AS_MAP_STAND_BY)?mMapEntityLocateMePixelSize.y*0.5:mMapEntityFollowMePixelSize.y * NBGM_GetConfig()->dpi*0.5;
            double sizeY = (mState == NBGM_AS_MAP_STAND_BY)?mMapEntityLocateMePixelSize.x*0.5:mMapEntityFollowMePixelSize.x * NBGM_GetConfig()->dpi*0.5;
            NBRE_Vector2d halfSize(sizeX, sizeY);
            NBRE_AxisAlignedBox2d avatarRect(screenPt - halfSize, screenPt + halfSize);

            if (NBRE_Intersectiond::HitTest(screenRect, avatarRect) )
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}

PAL_Error
NBGM_Avatar::SetHaloStyle(const NBGM_Color& edgeColor, const NBGM_Color& fillColor, uint8 edgeWidth, NBGM_HaloEdgeStyle /*style*/)
{
    NBRE_Color innerClr(fillColor.red, fillColor.green, fillColor.blue, fillColor.alpha);
    NBRE_Color outlineClr(edgeColor.red, edgeColor.green, edgeColor.blue, edgeColor.alpha);

    if(mHaloInnerClr != innerClr)
    {
        mHaloInnerShader = CreateHaloShader(mNBGMContext.renderingEngine->Context(), innerClr, edgeWidth);
        mHaloInnerClr = innerClr;
        mNeedUpdate = TRUE;
    }

    if(mHaloOutlineClr != outlineClr || mHaloEdgeWidth != edgeWidth)
    {
        mHaloOutlineShader = CreateHaloShader(mNBGMContext.renderingEngine->Context(), outlineClr, edgeWidth);
        mHaloOutlineClr = outlineClr;
        mHaloEdgeWidth = edgeWidth;
        mNeedUpdate = TRUE;
    }
    return PAL_Ok;
}

PAL_Error
NBGM_Avatar::SetDirectionalTexture(const std::string& textureFilePath, const NBGM_Point2d& offset)
{
    if(textureFilePath.empty())
    {
        return PAL_ErrBadParam;
    }

    mMapFollowMeShader = CreateMapShader(mNBGMContext, textureFilePath, mMapEntityFollowMePixelSize);
    mFollowMeOffset.x = offset.x;
    mFollowMeOffset.y = offset.y;

    mNeedUpdate = TRUE;
    return PAL_Ok;
}

PAL_Error
NBGM_Avatar::SetDirectionalTexture(const NBGM_BinaryBuffer& textureBuffer, const NBGM_Point2d& offset)
{
    if(textureBuffer.empty())
    {
        return PAL_ErrBadParam;
    }

    NBRE_MemoryStream *memoryIO = NBRE_NEW NBRE_MemoryStream(textureBuffer.addr, textureBuffer.size, TRUE);
    mMapFollowMeShader = CreateMapShader(mNBGMContext, memoryIO, mMapEntityFollowMePixelSize);
    mFollowMeOffset.x = offset.x;
    mFollowMeOffset.y = offset.y;
    mNeedUpdate = TRUE;
    return PAL_Ok;
}

PAL_Error
NBGM_Avatar::SetDirectionlessTexture(const std::string& textureFilePath, const NBGM_Point2d& offset)
{
    if(textureFilePath.empty())
    {
        return PAL_ErrBadParam;
    }

    mMapLocateMeShader = CreateMapShader(mNBGMContext, textureFilePath, mMapEntityLocateMePixelSize);
    mLocateMeOffset.x = offset.x;
    mLocateMeOffset.y = offset.y;
    mNeedUpdate = TRUE;
    return PAL_Ok;
}


PAL_Error
NBGM_Avatar::SetDirectionlessTexture(const NBGM_BinaryBuffer& textureBuffer, const NBGM_Point2d& offset)
{
    if(textureBuffer.empty())
    {
        return PAL_ErrBadParam;
    }

    NBRE_MemoryStream *memoryIO = NBRE_NEW NBRE_MemoryStream(textureBuffer.addr, textureBuffer.size, TRUE);
    mMapLocateMeShader = CreateMapShader(mNBGMContext, memoryIO, mMapEntityLocateMePixelSize);
    mLocateMeOffset.x = offset.x;
    mLocateMeOffset.y = offset.y;
    mNeedUpdate = TRUE;
    return PAL_Ok;
}

void 
NBGM_Avatar::UpdateDPI()
{
    mNeedUpdate = TRUE;
}
