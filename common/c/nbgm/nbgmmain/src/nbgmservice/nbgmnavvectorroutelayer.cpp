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
#include "nbgmnavvectorroutelayer.h"
#include "nbgmbuildutility.h"
#include "nbrevertexindexdata.h"
#include "nbrerenderpal.h"
#include "nbrelog.h"
#include "nbreentity.h"
#include "nbrerenderengine.h"
#include "nbrerenderpal.h"
#include "nbretypes.h"
#include "nbremesh.h"
#include "nbrelinearinterpolate.h"
#include "nbgmconst.h"
#include "nbgmmapviewimpl.h"

NBGM_NavVectorRouteLayer::Maneuver::Maneuver( NBGM_Context& context, NBGM_BuildModelContext* buildModelContext, const NBRE_Point2d& offset, const float* points, uint32 count, uint32 maneuverID )
:mNBGMContext(context)
,mBuildModelContext(buildModelContext)
,mNode(NBRE_NEW NBRE_Node)
,mOffset(offset)
,mManeuverID(maneuverID) 
,mArrowPolyline(NULL)
,mArrowLength(0.0f) 
,mWorldPolyline(count / 2)
,mManeuverPolyline(NULL)
{
    mOffset.x = mNBGMContext.WorldToModel(offset.x);
    mOffset.y = mNBGMContext.WorldToModel(offset.y);

    mRoutePolyline = NBRE_ArrayCreate(count >> 1);
    for (uint32 i = 0; i < count; i += 2)
    {
        NBRE_Point2f* point = NBRE_NEW NBRE_Point2f(mNBGMContext.WorldToModel(points[i]),
                                                    mNBGMContext.WorldToModel(points[i+1]));
        NBRE_ArrayAppend(mRoutePolyline, point);
        mWorldPolyline.SetVertex(i / 2, NBRE_Vector3d(mOffset.x + point->x, mOffset.y + point->y, 0));
    }
    mWorldPolyline.UpdateLength();

    mNode->SetPosition(NBRE_Vector3d(mOffset.x, mOffset.y, 0.0));
}

NBGM_NavVectorRouteLayer::Maneuver::~Maneuver()
{
    if (mRoutePolyline != NULL)
    {
        uint32 count = NBRE_ArrayGetSize(mRoutePolyline);
        for (uint32 i = 0; i < count; ++i)
        {
            NBRE_DELETE static_cast<NBRE_Point2f*>(NBRE_ArrayGetAt(mRoutePolyline, i));
        }
        NBRE_ArrayDestroy(mRoutePolyline);
    }

    if (mArrowPolyline != NULL)
    {
        uint32 count = NBRE_ArrayGetSize(mArrowPolyline);
        for (uint32 i = 0; i < count; ++i)
        {
            NBRE_DELETE static_cast<NBRE_Point2f*>(NBRE_ArrayGetAt(mArrowPolyline, i));
        }
        NBRE_ArrayDestroy(mArrowPolyline);
    }
    NBRE_DELETE mManeuverPolyline;
}

PAL_Error
NBGM_NavVectorRouteLayer::Maneuver::BuildRouteModel( float routeWidth, const NBRE_ShaderPtr& shader, int32* layerID, const NBRE_Point2f& foregroundTexcoord, const NBRE_Point2f& backgroundTexcoord)
{
    NBGM_FloatBuffer& vertexBuffer = mBuildModelContext->mVetexBuffer;
    NBGM_FloatBuffer& texcoordBuffer1 = mBuildModelContext->mTexcoordBuffer1;
    NBGM_FloatBuffer& texcoordBuffer2 = mBuildModelContext->mTexcoordBuffer2;
    NBGM_ShortBuffer& indexBuffer = mBuildModelContext->mIndexBuffer;

    vertexBuffer.clear();
    texcoordBuffer1.clear();
    texcoordBuffer2.clear();
    indexBuffer.clear();

    PAL_Error err = NBGM_BuildModelUtility::NavVectorPolyline2Tristripe(mRoutePolyline, routeWidth, backgroundTexcoord.x,
                                                                        backgroundTexcoord.y, vertexBuffer, texcoordBuffer1, texcoordBuffer2, indexBuffer);
    if (err != PAL_Ok)
    {
        return err;
    }

    NBRE_Entity* entityBackground = BuildEntity();
    entityBackground->SetName("nav_vector_bg_route");
    entityBackground->SetOverlayId(layerID[0]);
    entityBackground->SetShader(shader);
    mNode->AttachObject(NBRE_EntityPtr(entityBackground));

    vertexBuffer.clear();
    texcoordBuffer1.clear();
    texcoordBuffer2.clear();
    indexBuffer.clear();

    err = NBGM_BuildModelUtility::NavVectorPolyline2Tristripe(mRoutePolyline, routeWidth * (1 - NAV_VECTOR_MANEUVER_OUTLINE_PERCENTAGE),
                                                              foregroundTexcoord.x, foregroundTexcoord.y, vertexBuffer, texcoordBuffer1, texcoordBuffer2, indexBuffer);
    if (err != PAL_Ok)
    {
        return err;
    }

    NBRE_Entity* entityForeground = BuildEntity();
    entityForeground->SetName("nav_vector_fg_route");
    entityForeground->SetOverlayId(layerID[1]);
    entityForeground->SetShader(shader);
    mNode->AttachObject(NBRE_EntityPtr(entityForeground));

    return err;
}

PAL_Error
NBGM_NavVectorRouteLayer::Maneuver::CreateTurnArrowPolyline(Maneuver* nextManeuver, float checkLength, float halfRouteWidth )
{
    if(NBRE_ArrayGetSize(this->mRoutePolyline) == 0 || NBRE_ArrayGetSize(nextManeuver->mRoutePolyline) == 0)
    {
        return PAL_ErrNoInit;
    }

    NBRE_Array* tailPolyline = NBRE_ArrayCreate(2);
    NBRE_Array* headerPolyline = NBRE_ArrayCreate(2);

    //get tail polyline
    NBRE_Point2f* newPt = NBRE_NEW NBRE_Point2f();
    uint32 segNum = NBRE_ArrayGetSize(mRoutePolyline);
    uint32 ptIndex = 0;
    NBRE_Point2f* currPos = static_cast<NBRE_Point2f*>(NBRE_ArrayGetAt(mRoutePolyline, segNum - 1 - ptIndex));
    NBRE_Point2f* nextPos = NULL;
    *newPt = *currPos;
    NBRE_ArrayAppend(tailPolyline, newPt);

    float length = mNBGMContext.WorldToModel(NAV_VECTOR_MANEUVER_TURN_ARROW_TAIL_FIRST_SEGMENT_LENGTH);
    ++ptIndex;
    while(length>0 && ptIndex<segNum)
    {
        float newSegLength = 0.0f;
        newPt = NBRE_NEW NBRE_Point2f();
        nextPos = static_cast<NBRE_Point2f*>(NBRE_ArrayGetAt(mRoutePolyline, segNum - 1 - ptIndex));
        newSegLength = currPos->DistanceTo(*nextPos);
        if(length > newSegLength)
        {
            *newPt = *nextPos;
        }
        else
        {
            float t = length/newSegLength;
            *newPt = NBRE_LinearInterpolatef::Lerp(*currPos, *nextPos, t);
        }
        NBRE_ArrayAppend(tailPolyline, newPt);
        length -= newSegLength;

        ++ptIndex;
        currPos = nextPos;
    }

    //get header polyline
    newPt = NBRE_NEW NBRE_Point2f();
    segNum = NBRE_ArrayGetSize(nextManeuver->mRoutePolyline);
    ptIndex = 0;

    currPos = static_cast<NBRE_Point2f*>(NBRE_ArrayGetAt(nextManeuver->mRoutePolyline, ptIndex));
    *newPt = *currPos;
    length = mNBGMContext.WorldToModel(NAV_VECTOR_MANEUVER_TURN_ARROW_TAIL_SECOND_SEGMENT_LENGTH);
    NBRE_ArrayAppend(headerPolyline, newPt);
    ++ptIndex;
    while(length>0 && ptIndex<segNum)
    {
        float newSegLength = 0;
        newPt = NBRE_NEW NBRE_Point2f();
        nextPos = static_cast<NBRE_Point2f*>(NBRE_ArrayGetAt(nextManeuver->mRoutePolyline, ptIndex));
        newSegLength = currPos->DistanceTo(*nextPos);
        if((length > newSegLength) && ((ptIndex + 1) < segNum))
        {
            *newPt = *nextPos;
        }
        else
        {
            if(((ptIndex + 1) < segNum) || (newSegLength > (checkLength + length)))
            {
                float t = length/newSegLength;
                *newPt = NBRE_LinearInterpolatef::Lerp(*currPos, *nextPos, t);
            }
            else
            {
                if(newSegLength > (checkLength + halfRouteWidth))
                {
                    float t = (newSegLength - checkLength)/newSegLength;
                    *newPt = NBRE_LinearInterpolatef::Lerp(*currPos, *nextPos, t);
                }
                else
                {
                    float t = (halfRouteWidth + mNBGMContext.WorldToModel(NAV_VECTOR_MANEUVER_TURN_ARROW_TAIL_SECOND_SEGMENT_MIN_LENGTH))/newSegLength;
                    *newPt = NBRE_LinearInterpolatef::Lerp(*currPos, *nextPos, t);
                }
            }
        }
        NBRE_ArrayAppend(headerPolyline, newPt);
        length -= newSegLength;
        ++ptIndex;
        currPos = nextPos;
    }

    //create output polyline
    mArrowPolyline = NBRE_ArrayCreate(NBRE_ArrayGetSize(tailPolyline) + NBRE_ArrayGetSize(headerPolyline));

    //tail polyline
    segNum = NBRE_ArrayGetSize(tailPolyline) ;
    for(ptIndex = 0; ptIndex < segNum; ++ptIndex)
    {
        NBRE_Point2f* pt = static_cast<NBRE_Point2f*>(NBRE_ArrayGetAt(tailPolyline, segNum -1 - ptIndex));
        NBRE_ArrayAppend(mArrowPolyline, pt);
    }

    //header polyline
    segNum = NBRE_ArrayGetSize(headerPolyline) ;
    for(ptIndex = 0; ptIndex < segNum; ++ptIndex)
    {
        NBRE_Point2f* pt = static_cast<NBRE_Point2f*>(NBRE_ArrayGetAt(headerPolyline, ptIndex));
        pt->x = (float)(pt->x + nextManeuver->mOffset.x - mOffset.x);
        pt->y = (float)(pt->y + nextManeuver->mOffset.y - mOffset.y);
        NBRE_ArrayAppend(mArrowPolyline, pt);
    }

    //Create maneuver polyline for overlapping checking
    uint32 arrowVertexCount = NBRE_ArrayGetSize(mArrowPolyline);
    if (arrowVertexCount > 0 && mManeuverPolyline == NULL)
    {
        mManeuverPolyline = NBRE_NEW NBRE_Polyline3d(arrowVertexCount);
        for (uint32 i = 0; i < arrowVertexCount; ++i)
        {
           NBRE_Point2f* pt = static_cast<NBRE_Point2f*>(NBRE_ArrayGetAt(mArrowPolyline, i));
            mManeuverPolyline->SetVertex(i, NBRE_Vector3d(mOffset.x + pt->x, mOffset.y + pt->y, 0));
        }
        mManeuverPolyline->UpdateLength();
    }

    NBRE_ArrayDestroy(tailPolyline);
    NBRE_ArrayDestroy(headerPolyline);

    return PAL_Ok;
}

void
NBGM_NavVectorRouteLayer::Maneuver::BuildTurnArrowModel(float routeWidth, const NBRE_ShaderPtr& shader, int32* layerID,
                                                        const NBRE_Point2f& foregroundTexcoord, const NBRE_Point2f& backgroundTexcoord)
{
    if (mArrowPolyline == NULL)
    {
        return;
    }

    float arrowWidth = routeWidth * NAV_VECTOR_MANEUVER_TURN_ARROW_TAIL_WIDTH_SCALE;

    NBRE_Array* temp = CopyMercatorArray(mArrowPolyline);
    if (temp == NULL)
    {
        return;
    }

    NBGM_FloatBuffer& vertexBuffer = mBuildModelContext->mVetexBuffer;
    NBGM_FloatBuffer& texcoordBuffer1 = mBuildModelContext->mTexcoordBuffer1;
    NBGM_FloatBuffer& texcoordBuffer2 = mBuildModelContext->mTexcoordBuffer2;
    NBGM_ShortBuffer& indexBuffer = mBuildModelContext->mIndexBuffer;

    vertexBuffer.clear();
    texcoordBuffer1.clear();
    texcoordBuffer2.clear();
    indexBuffer.clear();

    // create arrow body vertices 
    PAL_Error err = NBGM_BuildModelUtility::NavVectorPolyline2Tristripe(temp, routeWidth, backgroundTexcoord.x,
                                                                        backgroundTexcoord.y, vertexBuffer, texcoordBuffer1, texcoordBuffer2, indexBuffer);
    if (err != PAL_Ok)
    {
        DestroyMercatorArray(temp);
        return;
    }

    // create arrow head vertices
    ManeuverBuildTurnArrowHeaderVertices(temp, routeWidth, arrowWidth, backgroundTexcoord.x, backgroundTexcoord.y);

    NBRE_Entity* entityBackground = BuildEntity();
    entityBackground->SetName("nav_vector_bg_arrow");
    entityBackground->SetOverlayId(layerID[2]);
    entityBackground->SetShader(shader);
    entityBackground->SetVisible(FALSE);
    mNode->AttachObject(NBRE_EntityPtr(entityBackground));

    // Extend maneuver turn arrow polyline
    uint32 segNum = NBRE_ArrayGetSize(temp);

    NBRE_Point2f* lastPos = static_cast<NBRE_Point2f*>(NBRE_ArrayGetAt(temp, segNum - 1));
    NBRE_Point2f* prevPos = static_cast<NBRE_Point2f*>(NBRE_ArrayGetAt(temp, segNum - 2));

    NBRE_Point2f dir = *lastPos - *prevPos;
    dir.Normalise();
    dir *= static_cast<float>(NAV_VECTOR_MANEUVER_OUTLINE_PERCENTAGE * routeWidth * 0.5f);
    *lastPos += dir;

    double hypot = nsl_sqrt(mArrowLength * mArrowLength + (arrowWidth * 0.5) * (arrowWidth * 0.5));
    double forwardLengthScale = NAV_VECTOR_MANEUVER_OUTLINE_PERCENTAGE * hypot / (NAV_VECTOR_MANEUVER_TURN_ARROW_TAIL_WIDTH_SCALE);

    float preArrowLength = mArrowLength;
    mArrowLength -= static_cast<float>(forwardLengthScale + NAV_VECTOR_MANEUVER_OUTLINE_PERCENTAGE * routeWidth * 0.5f);
    arrowWidth *= mArrowLength / preArrowLength;
    routeWidth *= (1 - NAV_VECTOR_MANEUVER_OUTLINE_PERCENTAGE);

    vertexBuffer.clear();
    texcoordBuffer1.clear();
    texcoordBuffer2.clear();
    indexBuffer.clear();

    // create arrow body vertices 
    err = NBGM_BuildModelUtility::NavVectorPolyline2Tristripe(temp, routeWidth, foregroundTexcoord.x,
                                                              foregroundTexcoord.y, vertexBuffer, texcoordBuffer1, texcoordBuffer2, indexBuffer);
    if (err != PAL_Ok)
    {
        DestroyMercatorArray(temp);
        return;
    }

    // create arrow head vertices
    ManeuverBuildTurnArrowHeaderVertices(temp, routeWidth, arrowWidth, foregroundTexcoord.x, foregroundTexcoord.y);

    NBRE_Entity* foregroudEntity = BuildEntity();
    foregroudEntity->SetName("nav_vector_fg_arrow");
    foregroudEntity->SetOverlayId(layerID[3]);
    foregroudEntity->SetShader(shader);
    foregroudEntity->SetVisible(FALSE);
    mNode->AttachObject(NBRE_EntityPtr(foregroudEntity));

    DestroyMercatorArray(temp);
}

void
NBGM_NavVectorRouteLayer::Maneuver::UpdateRegion()
{
    NBGM_LayoutManager& layoutManager = *mNBGMContext.layoutManager;
    layoutManager.UpdateRegion(mWorldPolyline, 1, NAV_ROUTE_UPDATE_MASK);
    if (mManeuverPolyline)
    {
        layoutManager.UpdateRegion(*mManeuverPolyline, 7, NAV_MANEUVER_UPDATE_MASK);
    }
}

NBRE_Array*
NBGM_NavVectorRouteLayer::Maneuver::CopyMercatorArray( NBRE_Array *polyline ) const
{
    if (polyline == NULL)
    {
        return NULL;
    }

    uint32 count = NBRE_ArrayGetSize(polyline);
    if (count < 2)
    {
        return NULL;
    }

    NBRE_Array* result = NBRE_ArrayCreate(count);
    if (result == NULL)
    {
        return NULL;
    }

    for (uint32 i = 0; i < count; ++i)
    {
        NBRE_Point2f* old = static_cast<NBRE_Point2f*>(NBRE_ArrayGetAt(polyline, i));
        if (old != NULL)
        {
            NBRE_Point2f* point = NBRE_NEW NBRE_Point2f(old->x, old->y);
            if (point != NULL)
            {
                NBRE_ArrayAppend(result, point);
            }
        }
    }
    return result;
}

void
NBGM_NavVectorRouteLayer::Maneuver::DestroyMercatorArray( NBRE_Array *polyline ) const
{
    if(polyline != NULL)
    {
        uint32 count = NBRE_ArrayGetSize(polyline);
        for(uint32 i = 0; i < count; ++i)
        {
            NBRE_Point2f* pt = static_cast<NBRE_Point2f*>(NBRE_ArrayGetAt(polyline, i));
            NBRE_DELETE pt;
        }
        NBRE_ArrayDestroy(polyline);
    }
}

void
NBGM_NavVectorRouteLayer::Maneuver::ManeuverBuildTurnArrowHeaderVertices( NBRE_Array* newArrowPolyline, float routeWidth, float arrowWidth, float texX, float texY)
{
    const float TURN_ARROW_TEXTURE_COORDINATE_X = 0.5f;
    const float TURN_ARROW_TEXTURE_COORDINATE_Y = 0.5f;

    uint32 polylineCount = NBRE_ArrayGetSize(newArrowPolyline);
    NBRE_Point2f lastPt = *static_cast<NBRE_Point2f*>(NBRE_ArrayGetAt(newArrowPolyline, polylineCount - 1));
    NBRE_Point2f prevLastPt = *static_cast<NBRE_Point2f*>(NBRE_ArrayGetAt(newArrowPolyline, polylineCount - 2));

    NBRE_Vector2f dir = lastPt - prevLastPt;
    dir.Normalise();
    NBRE_Vector2f normal = dir.Perpendicular();

    NBRE_Vector2f magnitude = normal * arrowWidth * 0.5f;
    NBRE_Point2f leftPt = lastPt + magnitude;
    NBRE_Point2f rightPt = lastPt - magnitude;

    magnitude = dir * mArrowLength;
    NBRE_Point2f headPt = lastPt + magnitude;

    NBGM_FloatBuffer& vertexBuffer = mBuildModelContext->mVetexBuffer;
    NBGM_FloatBuffer& texcoordBuffer1 = mBuildModelContext->mTexcoordBuffer1;
    NBGM_FloatBuffer& texcoordBuffer2 = mBuildModelContext->mTexcoordBuffer2;
    NBGM_ShortBuffer& indexBuffer = mBuildModelContext->mIndexBuffer;

    uint16 vertexIndex = static_cast<uint16>(vertexBuffer.size() / 2);

    vertexBuffer.push_back(rightPt.x);
    vertexBuffer.push_back(rightPt.y);
    texcoordBuffer1.push_back(TURN_ARROW_TEXTURE_COORDINATE_X);
    texcoordBuffer1.push_back(TURN_ARROW_TEXTURE_COORDINATE_Y);
    texcoordBuffer2.push_back(texX);
    texcoordBuffer2.push_back(texY);

    vertexBuffer.push_back(headPt.x);
    vertexBuffer.push_back(headPt.y);
    texcoordBuffer1.push_back(TURN_ARROW_TEXTURE_COORDINATE_X);
    texcoordBuffer1.push_back(TURN_ARROW_TEXTURE_COORDINATE_Y);
    texcoordBuffer2.push_back(texX);
    texcoordBuffer2.push_back(texY);

    vertexBuffer.push_back(leftPt.x);
    vertexBuffer.push_back(leftPt.y);
    texcoordBuffer1.push_back(TURN_ARROW_TEXTURE_COORDINATE_X);
    texcoordBuffer1.push_back(TURN_ARROW_TEXTURE_COORDINATE_Y);
    texcoordBuffer2.push_back(texX);
    texcoordBuffer2.push_back(texY);

    indexBuffer.push_back(vertexIndex);
    indexBuffer.push_back(vertexIndex + 1);
    indexBuffer.push_back(vertexIndex + 2);
}

NBRE_Entity*
NBGM_NavVectorRouteLayer::Maneuver::BuildEntity()
{
    NBGM_FloatBuffer& vertexBuffer = mBuildModelContext->mVetexBuffer;
    NBGM_FloatBuffer& texcoordBuffer1 = mBuildModelContext->mTexcoordBuffer1;
    NBGM_FloatBuffer& texcoordBuffer2 = mBuildModelContext->mTexcoordBuffer2;
    NBGM_ShortBuffer& indexBuffer = mBuildModelContext->mIndexBuffer;

    NBRE_IRenderPal* rp = mNBGMContext.renderingEngine->Context().mRenderPal;
    NBRE_VertexDeclaration* decalration = rp->CreateVertexDeclaration();

    NBRE_VertexElement* posElem = NBRE_NEW NBRE_VertexElement(0, 0, NBRE_VertexElement::VET_FLOAT2, NBRE_VertexElement::VES_POSITION, 0);
    decalration->GetVertexElementList().push_back(posElem);

    NBRE_VertexElement* texElem1 = NBRE_NEW NBRE_VertexElement(1, 0, NBRE_VertexElement::VET_FLOAT2, NBRE_VertexElement::VES_TEXTURE_COORDINATES, 0);
    decalration->GetVertexElementList().push_back(texElem1);

    NBRE_VertexElement* texElem2 = NBRE_NEW NBRE_VertexElement(2, 0, NBRE_VertexElement::VET_FLOAT2, NBRE_VertexElement::VES_TEXTURE_COORDINATES, 1);
    decalration->GetVertexElementList().push_back(texElem2);

    NBRE_VertexData* vertexData = NBRE_NEW NBRE_VertexData(vertexBuffer.size()/2);
    vertexData->SetVertexDeclaration(decalration);

    NBRE_HardwareVertexBuffer* vertextBuf = rp->CreateVertexBuffer(sizeof(float) * 2, vertexBuffer.size()/2, TRUE, NBRE_HardwareBuffer::HBU_STATIC);

    vertexData->AddBuffer(vertextBuf);
    vertextBuf->WriteData(0, vertexBuffer.size()*sizeof(float), &vertexBuffer.front(), FALSE);
    NBRE_HardwareVertexBuffer* texBuf2 = rp->CreateVertexBuffer(sizeof(float) * 2, texcoordBuffer2.size()/2, TRUE, NBRE_HardwareBuffer::HBU_STATIC);
    vertexData->AddBuffer(texBuf2);
    NBRE_HardwareVertexBuffer* texBuf1 = rp->CreateVertexBuffer(sizeof(float) * 2, texcoordBuffer1.size()/2, TRUE, NBRE_HardwareBuffer::HBU_STATIC);
    vertexData->AddBuffer(texBuf1);
    texBuf1->WriteData(0, texcoordBuffer1.size()*sizeof(float), &texcoordBuffer1.front(), FALSE);
    texBuf2->WriteData(0, texcoordBuffer2.size()*sizeof(float), &texcoordBuffer2.front(), FALSE);

    NBRE_HardwareIndexBuffer* indexBuf = rp->CreateIndexBuffer(NBRE_HardwareIndexBuffer::IT_16BIT, indexBuffer.size(), TRUE, NBRE_HardwareBuffer::HBU_STATIC);
    indexBuf->WriteData(0, indexBuffer.size()*sizeof(uint16), &indexBuffer.front(), FALSE);
    NBRE_IndexData* indexData = NBRE_NEW NBRE_IndexData(indexBuf, 0, indexBuf->GetNumIndexes());

    NBRE_Mesh* mesh = NBRE_NEW NBRE_Mesh(vertexData);
    mesh->CreateSubMesh(indexData, NBRE_PMT_TRIANGLE_LIST);
    mesh->CalculateBoundingBox();

    NBRE_ModelPtr model(NBRE_NEW NBRE_Model(NBRE_MeshPtr(mesh)));
    return NBRE_NEW NBRE_Entity(mNBGMContext.renderingEngine->Context(), model, 0);
}

void
NBGM_NavVectorRouteLayer::Maneuver::SetOverlayId(int32* overlayId)
{
    int32 entityCount = mNode->AttachedObjectsNumber();
    for (int32 i = 0; i < entityCount; ++i)
    {
        NBRE_Entity* entity = (NBRE_Entity*)mNode->GetAttachedObject(i);
        entity->SetOverlayId(overlayId[i]);
    }
}

NBGM_NavVectorRouteLayer::NBGM_NavVectorRouteLayer(NBGM_Context& nbgmContext, NBRE_Node& parentNode, NBRE_SurfaceSubView* subView, NBRE_DefaultOverlayManager<DrawOrderStruct>& overlayManager, uint32 drawOrder)
:NBGM_LayoutElement(nbgmContext)
,mNBGMContext(nbgmContext)
,mParentNode(parentNode)
,mBuildModelContext(NULL)
,mDrawOrder(drawOrder)
,mRouteWidth(mNBGMContext.WorldToModel(NAV_VECTOR_ROUTE_WIDTH))
,mCurrentManeuverID(NBRE_INVALID_INDEX)
,mClipManeuverID(NBRE_INVALID_INDEX)
,mCurrentManeuver(NULL)
,mRouteForegroundTexcoord(0.0f, 0.0f)
,mRouteBackgroundTexcoord(1.0f, 0.0f)
,mArrowForegroundTexcoord(0.0f, 1.0f)
,mArrowBackgroundTexcoord(1.0f, 1.0f)
,mSubView(subView)
,mOverlayManager(overlayManager)
,mIsDrawOrderInitialized(FALSE)
{
    mShader = CreateShader();
    mBuildModelContext = NBRE_NEW NBGM_BuildModelContext();
    AddToSence();

    for(int32 i = 0; i < 4; ++i)
    {
        //mLayerID[i] = mOverlayManager.AssignOverlayId(drawOrder + i);
    }
    mNBGMContext.layoutManager->AddLayoutProvider(this);
}

NBGM_NavVectorRouteLayer::~NBGM_NavVectorRouteLayer()
{
    mNBGMContext.layoutManager->RemoveLayoutProvider(this);

    ManeuverArray::iterator pManeuver = mManeuverArray.begin();
    ManeuverArray::iterator pManeuverEnd = mManeuverArray.end();

    for (; pManeuver != pManeuverEnd; ++pManeuver)
    {
        NBRE_DELETE (*pManeuver);
    }

    NBRE_DELETE mBuildModelContext;
}

void
NBGM_NavVectorRouteLayer::Reset()
{
    ManeuverArray::iterator pManeuver = mManeuverArray.begin();
    ManeuverArray::iterator pManeuverEnd = mManeuverArray.end();

    for (; pManeuver != pManeuverEnd; ++pManeuver)
    {
        mParentNode.RemoveChild((*pManeuver)->mNode.get());
        NBRE_DELETE (*pManeuver);
    }
    mManeuverArray.clear();

    mClipManeuverID = NBRE_INVALID_INDEX;
    mCurrentManeuverID = NBRE_INVALID_INDEX;
    mCurrentManeuver = NULL;
}

PAL_Error
NBGM_NavVectorRouteLayer::AddManeuver( const NBRE_Point2d& offset, const float* points, uint32 count, uint32 maneuverID)
{
    if (points == NULL || maneuverID == NBRE_INVALID_INDEX)
    {
        return PAL_ErrBadParam;
    }

    Maneuver* maneuver = NBRE_NEW Maneuver(mNBGMContext, mBuildModelContext, offset, points, count, maneuverID);
    if (maneuver == NULL)
    {
        return PAL_ErrNoMem;
    }

    PAL_Error err = maneuver->BuildRouteModel( mRouteWidth, mShader, mLayerID, mRouteForegroundTexcoord, mRouteBackgroundTexcoord);
    if (err != PAL_Ok)
    {
        NBRE_DELETE maneuver;
        return err;
    }
    mManeuverArray.push_back(maneuver);

    if(mManeuverArray.size() > 1)
    {
        Maneuver* prevManeuver = mManeuverArray.at(mManeuverArray.size() - 2);

        float checkLength = mRouteWidth * NAV_VECTOR_MANEUVER_TURN_ARROW_TAIL_WIDTH_SCALE ; 
        err = prevManeuver->CreateTurnArrowPolyline(maneuver, checkLength, mRouteWidth * 0.5f);
        if (err != PAL_Ok)
        {
            NBRE_DELETE maneuver;
            return err;
        }
        prevManeuver->mArrowLength = checkLength;
        prevManeuver->BuildTurnArrowModel(mRouteWidth, mShader, mLayerID, mArrowForegroundTexcoord, mArrowBackgroundTexcoord);
    }

    mParentNode.AddChild(maneuver->mNode);

    return PAL_Ok;
}

void
NBGM_NavVectorRouteLayer::SetCurrentManeuver( uint32 maneuverId )
{
    if (mCurrentManeuver) 
    {
        if (mCurrentManeuver->ManeuverID() == maneuverId)
        {
            return;
        }
    }

    ManeuverArray::iterator pManeuverEnd = mManeuverArray.end();    
    for (ManeuverArray::iterator pManeuver = mManeuverArray.begin(); pManeuver != pManeuverEnd; ++pManeuver)
    {
        if ((*pManeuver)->ManeuverID() == maneuverId) 
        {
            SetArrowVisible(mCurrentManeuver, FALSE);            
            mCurrentManeuver = *pManeuver;
            SetArrowVisible(mCurrentManeuver, TRUE);            
            break;
        }
    }
}

uint32
NBGM_NavVectorRouteLayer::CurrentManeuver()
{
    if (mCurrentManeuver) 
    {
        return mCurrentManeuver->ManeuverID() ;
    }

	return 0;

}

NBRE_ShaderPtr
NBGM_NavVectorRouteLayer::CreateShader()
{
    NBRE_Context& nbreContext = mNBGMContext.renderingEngine->Context();


    NBRE_Image* image = NBRE_NEW NBRE_Image(2, 2, NBRE_PF_R8G8B8A8);
    uint8* pixels = image->GetImageData();
    pixels[0] = static_cast<uint8>(NAV_VECTOR_FOREGROUND_ROUTE_COLOR.r * 255.0f); pixels[1] = static_cast<uint8>(NAV_VECTOR_FOREGROUND_ROUTE_COLOR.g * 255.0f);
    pixels[2] = static_cast<uint8>(NAV_VECTOR_FOREGROUND_ROUTE_COLOR.b * 255.0f); pixels[3] = static_cast<uint8>(NAV_VECTOR_FOREGROUND_ROUTE_COLOR.a * 255.0f);

    pixels[4] = static_cast<uint8>(NAV_VECTOR_BACKGROUND_ROUTE_COLOR.r * 255.0f); pixels[5] = static_cast<uint8>(NAV_VECTOR_BACKGROUND_ROUTE_COLOR.g * 255.0f);
    pixels[6] = static_cast<uint8>(NAV_VECTOR_BACKGROUND_ROUTE_COLOR.b * 255.0f); pixels[7] = static_cast<uint8>(NAV_VECTOR_BACKGROUND_ROUTE_COLOR.a * 255.0f);

    pixels[8] = static_cast<uint8>(NAV_VECTOR_FOREGROUND_ARROW_COLOR.r * 255.0f); pixels[9] = static_cast<uint8>(NAV_VECTOR_FOREGROUND_ARROW_COLOR.g * 255.0f);
    pixels[10] = static_cast<uint8>(NAV_VECTOR_FOREGROUND_ARROW_COLOR.b * 255.0f); pixels[11] = static_cast<uint8>(NAV_VECTOR_FOREGROUND_ARROW_COLOR.a * 255.0f);

    pixels[12] = static_cast<uint8>(NAV_VECTOR_BACKGROUND_ARROW_COLOR.r * 255.0f); pixels[13] = static_cast<uint8>(NAV_VECTOR_BACKGROUND_ARROW_COLOR.g * 255.0f);
    pixels[14] = static_cast<uint8>(NAV_VECTOR_BACKGROUND_ARROW_COLOR.b * 255.0f); pixels[15] = static_cast<uint8>(NAV_VECTOR_BACKGROUND_ARROW_COLOR.a * 255.0f);

    NBRE_Image** images = NBRE_NEW NBRE_Image*[1];

    images[0] = image->ConvertToFormat(NBRE_PF_R4G4B4A4);
    NBRE_DELETE image;

    NBRE_TextureUnit texUnit0;
    texUnit0.SetTexture(NBRE_TexturePtr(nbreContext.mRenderPal->CreateTexture(images, 1, 1, FALSE, NBRE_Texture::TT_2D, "vec_route_color")));
    texUnit0.SetState(NBRE_TextureState(NBRE_TFT_POINT, NBRE_TFT_POINT, NBRE_TAM_CLAMP, NBRE_TAM_CLAMP, FALSE));
    texUnit0.SetTextureCoordSet(0);
    NBRE_TextureUnit texUnit1;
    texUnit1.SetTexture(NBGM_BuildTextureUtility::ConstructRoadTexture(*nbreContext.mRenderPal, NBRE_Color(1, 1, 1, 1), "vec_route_shape"));
    texUnit1.SetState(NBRE_TextureState(NBRE_TFT_MIPMAP_TRILINEAR, NBRE_TFT_LINEAR, NBRE_TAM_CLAMP, NBRE_TAM_CLAMP, FALSE));
    texUnit1.SetTextureCoordSet(1);

    NBRE_Pass* pass = NBRE_NEW NBRE_Pass();
    pass->GetTextureUnits().push_back(texUnit0);
    pass->GetTextureUnits().push_back(texUnit1);
    pass->SetEnableLighting(FALSE);
    pass->SetEnableDepthTest(TRUE);
    pass->SetEnableDepthWrite(FALSE);
    pass->SetEnableCullFace(FALSE);
    pass->SetEnableBlend(TRUE);
    pass->SetSrcBlend(NBRE_BF_SRC_ALPHA);
    pass->SetDstBlend(NBRE_BF_ONE_MINUS_SRC_ALPHA);

    NBRE_Shader* shader = NBRE_NEW NBRE_Shader();
    shader->AddPass(NBRE_PassPtr(pass));

    return NBRE_ShaderPtr(shader);
}

void
NBGM_NavVectorRouteLayer::SetArrowVisible( Maneuver* man, nb_boolean visible )
{
    if (man != NULL)
    {
        const uint32 BACKGROUND_ARROW_INDEX = 2;
        const uint32 FOREGROUND_ARROW_INDEX = 3;

        NBRE_Entity* entity = static_cast<NBRE_Entity*>(man->mNode->GetAttachedObject(BACKGROUND_ARROW_INDEX));
        if (entity != NULL)
        {
            entity->SetVisible(visible);
        }

        entity = static_cast<NBRE_Entity*>(man->mNode->GetAttachedObject(FOREGROUND_ARROW_INDEX));
        if (entity != NULL)
        {
            entity->SetVisible(visible);
        }
    }
}

void
NBGM_NavVectorRouteLayer::UpdateRegion()
{
    for (uint32 i = 0; i < mManeuverArray.size(); ++i)
    {
        Maneuver* m = mManeuverArray[i];
        m->UpdateRegion();
    }
}

void
NBGM_NavVectorRouteLayer::SetOverlayId(int32 drawOrder)
{
    if (mDrawOrder == drawOrder || mIsDrawOrderInitialized)
    {
        return;
    }

    for(int32 i = 0; i < 4; ++i)
    {
        //mLayerID[i] = mOverlayManager.AssignOverlayId(drawOrder + i);
    }

    for (ManeuverArray::iterator it = mManeuverArray.begin(); it != mManeuverArray.end(); ++it)
    {
        (*it)->SetOverlayId(mLayerID);
    }

    mIsDrawOrderInitialized = TRUE;
    mDrawOrder = drawOrder;
}

void
NBGM_NavVectorRouteLayer::UpdateLayoutList(NBGM_LayoutElementList& layoutElements)
{
    layoutElements.push_back(this);
}
