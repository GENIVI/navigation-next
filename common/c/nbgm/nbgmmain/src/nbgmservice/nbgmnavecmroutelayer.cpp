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
#include "nbgmnavecmroutelayer.h"
#include "nbgmbuildutility.h"
#include "nbrevertexindexdata.h"
#include "nbrerenderpal.h"
#include "nbrelog.h"
#include "nbrerenderpal.h"
#include "nbremesh.h"
#include "nbrelinearinterpolate.h"
#include "nbgmconst.h"
#include "nbgmmapviewimpl.h"
#include "nbremath.h"
#include "nbrecommon.h"
#include "nbrerenderengine.h"

NBGM_NavEcmRouteLayer::Maneuver::Maneuver( const NBGM_Context& context, const NBRE_Point3f& offset, uint32 maneuverID )
:mID(maneuverID)
{
    mOffset.x = context.WorldToModel(offset.x);
    mOffset.y = context.WorldToModel(offset.y);
    mOffset.z = context.WorldToModel(offset.z);
}

NBGM_NavEcmRouteLayer::Maneuver::~Maneuver()
{
}

NBGM_NavEcmRouteLayer::NBGM_NavEcmRouteLayer(NBGM_Context& nbgmContext, NBRE_Node& parentNode, NBRE_SurfaceSubView* /*subView*/, NBRE_DefaultOverlayManager<DrawOrderStruct>& overlayManager, const DrawOrderStruct& routeDrawOrder, const DrawOrderStruct& arrowDrawOrder)
:mNBGMContext(nbgmContext)
,mBuildModelContext(NULL)
,mParentNode(parentNode)
,mRouteNode(NBRE_NEW NBRE_Node)
,mArrowNode(NBRE_NEW NBRE_Node)
,mRouteWidth(mNBGMContext.WorldToModel(NAV_ECM_ROUTE_WIDTH))
,mCurrentManeuverID(NBRE_INVALID_INDEX)
{
    mRouteShader = CreateShader(mNBGMContext.renderingEngine->Context(), NAV_ECM_ROUTE_COLOR, "nav_ecm_route");
    mTurnArrowShader = CreateShader(mNBGMContext.renderingEngine->Context(), NAV_ECM_ARROW_COLOR, "nav_ecm_arrow");

    mRouteNode->SetPosition(NBRE_Vector3d(0.0, 0.0, 0.0));
    mParentNode.AddChild(mRouteNode);
    mArrowNode->SetPosition(NBRE_Vector3d(0.0, 0.0, 0.0));
    mParentNode.AddChild(mArrowNode);

    mBuildModelContext = NBRE_NEW NBGM_BuildModelContext();

    mRouteLayerID = overlayManager.AssignOverlayId(routeDrawOrder);
    mArrowLayerID = overlayManager.AssignOverlayId(arrowDrawOrder);
}

NBGM_NavEcmRouteLayer::~NBGM_NavEcmRouteLayer()
{
    mParentNode.RemoveChild(mRouteNode.get());
    mParentNode.RemoveChild(mArrowNode.get());

    NBRE_DELETE mBuildModelContext;
}

PAL_Error
NBGM_NavEcmRouteLayer::AddManeuver( uint32 id, const NBRE_Point3f& position )
{
    if (id == NBRE_INVALID_INDEX)
    {
        return PAL_ErrBadParam;
    }

    ManeuverArray::const_iterator pMan = mManeuverArray.begin();
    ManeuverArray::const_iterator pManEnd = mManeuverArray.end();

    for (; pMan != pManEnd; ++pMan)
    {
        if (pMan->mID == id)
        {
            return PAL_ErrAlreadyExist;
        }
    }

    mManeuverArray.push_back(Maneuver(mNBGMContext, position, id));

    return PAL_Ok;
}

PAL_Error
NBGM_NavEcmRouteLayer::SetCurrentManeuver( uint32 id )
{
    if(mSplinesPolyline.empty())
    {
        return PAL_ErrBadParam;
    }

    if(mCurrentManeuverID == id)
    {
        return PAL_ErrAlreadyExist;
    }

    ManeuverArray::const_iterator pMan = mManeuverArray.begin();
    ManeuverArray::const_iterator pManEnd = mManeuverArray.end();
    uint32 index = NBRE_INVALID_INDEX;
    for (uint32 counter = 0; pMan != pManEnd; ++pMan, ++counter)
    {
        if (pMan->mID == id)
        {
            index = counter;
            break;
        }
    }

    if (index == NBRE_INVALID_INDEX)
    {
        return PAL_ErrNotFound;
    }

    NBGM_SnapRouteInfo snapRouteInfo;
    NBGM_Location64 location = {{0}};
    location.position.x = mManeuverArray[index].mOffset.x;
    location.position.y = mManeuverArray[index].mOffset.y;
    location.position.z = mManeuverArray[index].mOffset.z;
    location.heading = static_cast<double>(NBRE_INVALID_HEADING);

    PAL_Error err = FindPolylineBestSeg(location, FALSE, snapRouteInfo);
    if(err != PAL_Ok)
    {
        return err;
    }

    ClipPolylineForManeuverDrawing(snapRouteInfo, mNBGMContext.WorldToModel(NAV_ECM_MANEUVER_TURN_ARROW_HEAD_LENGTH));

    mParentNode.RemoveChild(mArrowNode.get());
    mArrowNode.reset(NBRE_NEW NBRE_Node);
    mArrowNode->SetPosition(NBRE_Vector3d(0.0, 0.0, 0.0));
    mParentNode.AddChild(mArrowNode);

    err = ManeuverModelConstruct(mNBGMContext.WorldToModel(NAV_ECM_MANEUVER_TURN_ARROW_HEAD_LENGTH));
    if(err != PAL_Ok)
    {
        return err;
    }
    mCurrentManeuverID = id;

    return err;
}

PAL_Error
NBGM_NavEcmRouteLayer::AppendSpline( const NBGM_SplineData* splineData )
{
    if (splineData == NULL)
    {
        return PAL_ErrBadParam;
    }

    PAL_Error err = PAL_Ok;

    NBGM_Spline* spline = NBRE_NEW NBGM_Spline(mNBGMContext, splineData->data, splineData->size);

    uint32 oldCount = mSplinesPolyline.size();
    err = ConvertSplineToPolyline(spline, mSplinesPolyline);
    if(err != PAL_Ok)
    {
        NBRE_DELETE spline;
        return err;
    }

    uint32 newCount = mSplinesPolyline.size();
    uint32 assistIndex = 0;
    uint32 startIndex = 0;

    if (oldCount == 0)
    {
        assistIndex = 0;
        startIndex = 0;
    }
    else
    {
        assistIndex = oldCount - 1;
        startIndex = oldCount;
    }

    err = SplineModelConstruct(assistIndex, startIndex, newCount - oldCount);
    if(err != PAL_Ok)
    {
        NBRE_DELETE spline;
        return err;
    }

    NBRE_DELETE spline;

    return PAL_Ok;
}

NBRE_ShaderPtr
NBGM_NavEcmRouteLayer::CreateShader( NBRE_Context& nbreContext, const NBRE_Color& color, const NBRE_String& name)
{
    NBRE_TextureUnit texUnit0;
    texUnit0.SetTexture(NBGM_BuildTextureUtility::ConstructNavEcmRouteTexture(*nbreContext.mRenderPal, color, name));
    texUnit0.SetState(NBRE_TextureState(NBRE_TFT_MIPMAP_TRILINEAR, NBRE_TFT_LINEAR, NBRE_TAM_CLAMP, NBRE_TAM_CLAMP, FALSE));
    texUnit0.SetTextureCoordSet(0);

    NBRE_Pass* pass = NBRE_NEW NBRE_Pass();
    pass->GetTextureUnits().push_back(texUnit0);
    pass->SetEnableLighting(FALSE);
    pass->SetEnableDepthTest(FALSE);
    pass->SetEnableDepthWrite(FALSE);
    pass->SetEnableCullFace(FALSE);
    pass->SetEnableBlend(TRUE);
    pass->SetSrcBlend(NBRE_BF_SRC_ALPHA);
    pass->SetDstBlend(NBRE_BF_ONE_MINUS_SRC_ALPHA);

    NBRE_Shader* shader = NBRE_NEW NBRE_Shader();
    shader->AddPass(NBRE_PassPtr(pass));

    return NBRE_ShaderPtr(shader);
}

PAL_Error
NBGM_NavEcmRouteLayer::SplineModelConstruct( uint32 assistIndex, uint32 startIndex, uint32 count )
{
    uint32 const MAX_HERMITE_POLYLINE_POINT_COUNT = 65535;

    if(assistIndex != startIndex)
    {
        NBRE_Point3f p1 = mSplinesPolyline.at(assistIndex);
        NBRE_Point3f p2 = mSplinesPolyline.at(assistIndex + 1);
        float distance = p1.DistanceTo(p2);
        if (distance > mNBGMContext.WorldToModel(NAV_ECM_ROUTE_POLYLINE_HALF_WIDTH * 2))
        {
            ++count;
        }
    }

    uint32 vertexCount = count << 2;

    //NOTE: OpenGL ES just support short type index
    if(vertexCount > MAX_HERMITE_POLYLINE_POINT_COUNT)
    {
        return PAL_ErrUnsupported;
    }

    NBGM_FloatBuffer& vertexBuffer = mBuildModelContext->mVetexBuffer;
    NBGM_FloatBuffer& texcoordBuffer1 = mBuildModelContext->mTexcoordBuffer1;
    NBGM_ShortBuffer& indexBuffer = mBuildModelContext->mIndexBuffer;

    vertexBuffer.clear();
    texcoordBuffer1.clear();
    indexBuffer.clear();

    PAL_Error err = NBGM_BuildModelUtility::NavEcmPolyline2Tristripe(mSplinesPolyline, assistIndex, startIndex, count, mNBGMContext.WorldToModel(NAV_ECM_ROUTE_POLYLINE_HEIGHT_OFFSET),
                                                                          mNBGMContext.WorldToModel(NAV_ECM_ROUTE_POLYLINE_HALF_WIDTH),vertexBuffer, texcoordBuffer1, indexBuffer);
    if (err != PAL_Ok)
    {
        return err;
    }

    NBRE_Entity* entity = CreateEntity(vertexBuffer, texcoordBuffer1, indexBuffer);
    entity->SetName("nav_ecm_route");
    entity->SetOverlayId(mRouteLayerID);
    entity->SetShader(mRouteShader);
    mRouteNode->AttachObject(NBRE_EntityPtr(entity));

    return PAL_Ok;
}

void
NBGM_NavEcmRouteLayer::RemoveAllSplines()
{
    mManeuverArray.clear();
    mSplinesPolyline.clear();
    mArrowPolyline.clear();

    mParentNode.RemoveChild(mRouteNode.get());
    mParentNode.RemoveChild(mArrowNode.get());

    mRouteNode.reset(NBRE_NEW NBRE_Node);
    mArrowNode.reset(NBRE_NEW NBRE_Node);

    mRouteNode->SetPosition(NBRE_Vector3d(0.0, 0.0, 0.0));
    mParentNode.AddChild(mRouteNode);
    mArrowNode->SetPosition(NBRE_Vector3d(0.0, 0.0, 0.0));
    mParentNode.AddChild(mArrowNode);

    mCurrentManeuverID = NBRE_INVALID_INDEX;
}

PAL_Error
NBGM_NavEcmRouteLayer::FindPolylineBestSeg( const NBGM_Location64& location, nb_boolean reverse, NBGM_SnapRouteInfo& snapRouteInfo ) const
{
    uint32 lastPosIndex = snapRouteInfo.GetLastPositionIndex();
    PAL_Error err = PAL_ErrNotFound;

    if(mSplinesPolyline.size() < 2)
    {
        return PAL_ErrWrongFormat;
    }

    if(lastPosIndex == NBRE_INVALID_INDEX)
    {
        if(reverse)
        {
            lastPosIndex = mSplinesPolyline.size() - 1;
        }
        else
        {
            lastPosIndex = 0;
        }
    }

    NBRE_Point2f point(static_cast<float>(location.position.x), static_cast<float>(location.position.y));
    NBRE_AxisAlignedBox2f box(NBRE_Point2f(point.x - mNBGMContext.WorldToModel(NAV_ECM_ROUTE_MATCH_RANGE), point.y - mNBGMContext.WorldToModel(NAV_ECM_ROUTE_MATCH_RANGE)),
                              NBRE_Point2f(point.x + mNBGMContext.WorldToModel(NAV_ECM_ROUTE_MATCH_RANGE), point.y + mNBGMContext.WorldToModel(NAV_ECM_ROUTE_MATCH_RANGE)));

    NBRE_Point3f v1(mSplinesPolyline.at(lastPosIndex));

    if(reverse)
    {
        --lastPosIndex;
    }
    else
    {
        ++lastPosIndex;
    }

    uint8 matchCount = 0;
    while(lastPosIndex != mSplinesPolyline.size() && matchCount < NAV_ECM_SNAP_ROUTE_MAX_MATCH_NUMBER)
    {
        NBRE_Point2f p1(v1.x, v1.y);
        NBRE_Point3f v2(mSplinesPolyline.at(lastPosIndex));

        if(NBRE_Intersectionf::HitTest(box, p1))
        {
            NBRE_Point2f p2(v2.x, v2.y);
            float projectPosition = 0.f;
            float closestDistance = 0.f;

            Line2dProject(p1, p2, point, closestDistance, projectPosition);

            NBRE_Point2f currDir = p2 - p1;
            float currHeading = currDir.GetDirection();

            nb_boolean replace = FALSE;
            if((NBRE_Math::IsZero(static_cast<float>(location.heading - static_cast<double>(NBRE_INVALID_HEADING)), 1e-5f)) || CheckDirect(static_cast<float>(location.heading), currHeading))
            {
                if(matchCount == 0)
                {
                    replace = TRUE;
                }
                else if(snapRouteInfo.GetClosetDistance() > closestDistance)
                {
                    replace = TRUE;
                }
                ++matchCount;
            }

            if(replace)
            {
                snapRouteInfo.SetClosetDistance(closestDistance);
                snapRouteInfo.SetProjectPosition(projectPosition);

                if(!reverse)
                {
                    snapRouteInfo.SetLastPositionIndex(--lastPosIndex);
                }
                else
                {
                    snapRouteInfo.SetLastPositionIndex(++lastPosIndex);
                }
                err = PAL_Ok;
            }
        }

        v1 = v2;
        if(reverse)
        {
            --lastPosIndex;
        }
        else
        {
            ++lastPosIndex;
        }
    }

    return err;
}

void
NBGM_NavEcmRouteLayer::Line2dProject( const NBRE_Point2f& p1, const NBRE_Point2f& p2, const NBRE_Point2f& pt, float& distance, float& pos ) const
{
    NBRE_Point2f v1 = pt - p1;
    NBRE_Point2f v2 = p2 - p1;

    if(NBRE_Math::IsZero(v2.Length(), 1e-5f))
    {
        pos = 1.0f;
    }
    else
    {
        pos = v1.DotProduct(v2) / v2.DotProduct(v2);
        if(pos > 1.0f)
        {
            pos = 1.0f;
            v1 = v2 - v1;
        }
        else if(pos<0)
        {
            pos = 0.0f;
        }
        else
        {
            v2 *= pos;
            v1 = v2 - v1;
        }
    }

    distance = v1.DotProduct(v1);
}

nb_boolean
NBGM_NavEcmRouteLayer::CheckDirect( float degree1, float degree2 ) const
{
    float theta = degree1 - degree2;

    while(theta < 0)
    {
        theta += 360.0f;
    }
    while(theta >= 360.0f)
    {
        theta -= 360.0f;
    }

    if((theta < NAV_ECM_SNAP_ROUTE_HEADING_OFFSET) || (theta > (360.0f - NAV_ECM_SNAP_ROUTE_HEADING_OFFSET)))
    {
        return TRUE;
    }

    return FALSE;
}

void
NBGM_NavEcmRouteLayer::ClipPolylineForManeuverDrawing( const NBGM_SnapRouteInfo& snapRouteInfo, float arrowLength )
{
    NBRE_Point3f pos;
    NBRE_Point3f currPos = mSplinesPolyline.at(snapRouteInfo.GetLastPositionIndex());
    NBRE_Point3f nextPos = mSplinesPolyline.at(snapRouteInfo.GetLastPositionIndex() + 1);
    NBRE_Point3f turningPos = NBRE_LinearInterpolatef::Lerp(currPos, nextPos, snapRouteInfo.GetProjectPosition());

    mArrowPolyline.clear();

    // Backward
    mArrowPolyline.push_back(turningPos);
    NBRE_Vector<NBRE_Point3f>::iterator pPos = mArrowPolyline.begin();

    double totalLength = mNBGMContext.WorldToModel(NAV_ECM_MANEUVER_TURN_ARROW_TAIL_BACKWARD_SEGMENT_LENGTH);
    uint32 currentIndex = snapRouteInfo.GetLastPositionIndex();

    currPos = turningPos;

    while(totalLength > 0 && currentIndex != NBRE_INVALID_INDEX)
    {
        NBRE_Point3f prevPos(mSplinesPolyline.at(currentIndex));
        float distance = currPos.DistanceTo(prevPos);

        if(NBRE_Math::IsZero(distance, 1e-4f))
        {
            --currentIndex;
            continue;
        }

        if (totalLength > distance)
        {
            pPos = mArrowPolyline.insert(pPos, prevPos);
        }
        else
        {
            float t = static_cast<float>(totalLength / distance);
            pos = NBRE_LinearInterpolatef::Lerp(currPos, nextPos, t);
            pPos = mArrowPolyline.insert(pPos, prevPos);
        }

        totalLength -= distance;
        currPos = prevPos;
        --currentIndex;
    }

    // Forward
    totalLength = mNBGMContext.WorldToModel(NAV_ECM_MANEUVER_TURN_ARROW_TAIL_FORWARD_SEGMENT_LENGTH) + arrowLength;
    currentIndex = snapRouteInfo.GetLastPositionIndex() + 1;
    currPos = turningPos;
    float distanceSum = 0.0f;
    float length = 0.0f;

    while (totalLength > 0.0f && currentIndex != mSplinesPolyline.size())
    {
        nextPos = mSplinesPolyline.at(currentIndex);
        float distance = currPos.DistanceTo(nextPos);

        distanceSum += distance;
        totalLength -= distance;

        currPos = nextPos;
        ++currentIndex;
    }

    totalLength = mNBGMContext.WorldToModel(NAV_ECM_MANEUVER_TURN_ARROW_TAIL_FORWARD_SEGMENT_LENGTH) + arrowLength;
    currentIndex = snapRouteInfo.GetLastPositionIndex() + 1;
    currPos = turningPos;

    // construct arrow tail polyline
    if (distanceSum >= totalLength)
    {
        length = mNBGMContext.WorldToModel(NAV_ECM_MANEUVER_TURN_ARROW_TAIL_FORWARD_SEGMENT_LENGTH);
    }
    else if((distanceSum < totalLength) && (distanceSum > arrowLength))
    {
        length = distanceSum - arrowLength;
    }
    else
    {
        length = mNBGMContext.WorldToModel(NAV_ECM_MANEUVER_TURN_ARROW_TAIL_FORWARD_SEGMENT_MIN_LENGTH + NAV_ECM_ROUTE_POLYLINE_HALF_WIDTH);
    }

    while(length > 0 && currentIndex != mSplinesPolyline.size())
    {
        nextPos = mSplinesPolyline.at(currentIndex);
        float distance = currPos.DistanceTo(nextPos);

        if (NBRE_Math::IsZero(distance, 1e-4f))
        {
            currentIndex++;
            continue;
        }

        if (length > distance)
        {
            pos = nextPos;
            currPos = nextPos;
            currentIndex++;
        }
        else
        {
            float t = static_cast<float>(length / distance);
            pos = NBRE_LinearInterpolatef::Lerp(currPos, nextPos, t);
        }
        mArrowPolyline.push_back(pos);
        length -= distance;
    }
}

PAL_Error
NBGM_NavEcmRouteLayer::ManeuverModelConstruct( float turnArrowHeadHeight )
{
    uint32 count = mArrowPolyline.size();
    if (count < 2)
    {
        return PAL_ErrBadParam;
    }

    NBGM_FloatBuffer& vertexBuffer = mBuildModelContext->mVetexBuffer;
    NBGM_FloatBuffer& texcoordBuffer1 = mBuildModelContext->mTexcoordBuffer1;
    NBGM_ShortBuffer& indexBuffer = mBuildModelContext->mIndexBuffer;

    vertexBuffer.clear();
    texcoordBuffer1.clear();
    indexBuffer.clear();

    // create arrow body vertices
    PAL_Error err = NBGM_BuildModelUtility::NavEcmPolyline2Tristripe(mArrowPolyline, 0, 0, count, 0.0f, mNBGMContext.WorldToModel(NAV_ECM_ROUTE_POLYLINE_HALF_WIDTH), vertexBuffer, texcoordBuffer1, indexBuffer);
    if (err != PAL_Ok)
    {
        return err;
    }

    // create arrow head vertices
    NBRE_Point3f v1 = mArrowPolyline.at(count - 2);
    NBRE_Point3f v2 = mArrowPolyline.back();
    NBRE_Point3f headPos = v2 - v1;
    headPos.Normalise();
    NBRE_Point3f width(-headPos.y, headPos.x, 0.0f);

    headPos *= turnArrowHeadHeight;
    width *= mNBGMContext.WorldToModel(NAV_ECM_MANEUVER_TURN_ARROW_HEAD_HALF_WIDTH / NAV_ECM_ROUTE_POLYLINE_HALF_WIDTH);

    uint16 startIndex = static_cast<uint16>(vertexBuffer.size() / 3);

    v1 = v2 - width;
    vertexBuffer.push_back(v1.x);
    vertexBuffer.push_back(v1.y);
    vertexBuffer.push_back(v1.z);

    texcoordBuffer1.push_back(0.5f);
    texcoordBuffer1.push_back(0.0f);

    v1 = v2 + width;
    vertexBuffer.push_back(v1.x);
    vertexBuffer.push_back(v1.y);
    vertexBuffer.push_back(v1.z);

    texcoordBuffer1.push_back(0.5f);
    texcoordBuffer1.push_back(0.0f);

    v1 = v2 + headPos;
    vertexBuffer.push_back(v1.x);
    vertexBuffer.push_back(v1.y);
    vertexBuffer.push_back(v1.z);

    texcoordBuffer1.push_back(0.5f);
    texcoordBuffer1.push_back(1.0f);

    indexBuffer.push_back(startIndex);
    indexBuffer.push_back(startIndex + 1);
    indexBuffer.push_back(startIndex + 2);

    NBRE_Entity* entity = CreateEntity(vertexBuffer, texcoordBuffer1, indexBuffer);
    entity->SetName("nav_ecm_arrow");
    entity->SetOverlayId(mArrowLayerID);
    entity->SetShader(mTurnArrowShader);
    mArrowNode->AttachObject(NBRE_EntityPtr(entity));

    return PAL_Ok;
}

NBRE_Entity*
NBGM_NavEcmRouteLayer::CreateEntity( NBGM_FloatBuffer& vertexBuffer, NBGM_FloatBuffer& texcoordBuffer, NBGM_ShortBuffer& indexBuffer )
{
    NBRE_IRenderPal& rp = *(mNBGMContext.renderingEngine->Context().mRenderPal);
    NBRE_VertexDeclaration* decalration = rp.CreateVertexDeclaration();

    NBRE_VertexElement* posElem = NBRE_NEW NBRE_VertexElement(0, 0, NBRE_VertexElement::VET_FLOAT3, NBRE_VertexElement::VES_POSITION, 0);
    decalration->GetVertexElementList().push_back(posElem);

    NBRE_VertexElement* texElem1 = NBRE_NEW NBRE_VertexElement(1, 0, NBRE_VertexElement::VET_FLOAT2, NBRE_VertexElement::VES_TEXTURE_COORDINATES, 0);
    decalration->GetVertexElementList().push_back(texElem1);

    NBRE_VertexData* vertexData = NBRE_NEW NBRE_VertexData(vertexBuffer.size()/3);
    vertexData->SetVertexDeclaration(decalration);

    NBRE_HardwareVertexBuffer* vertextBuf = rp.CreateVertexBuffer(sizeof(float) * 3, vertexBuffer.size()/3, FALSE, NBRE_HardwareBuffer::HBU_STATIC);
    NBRE_HardwareVertexBuffer* texcoordBuf = rp.CreateVertexBuffer(sizeof(float) * 2, texcoordBuffer.size()/2, FALSE, NBRE_HardwareBuffer::HBU_STATIC);

    vertexData->AddBuffer(vertextBuf);
    vertexData->AddBuffer(texcoordBuf);

    vertextBuf->WriteData(0, vertexBuffer.size()*sizeof(float), &vertexBuffer.front(), FALSE);
    texcoordBuf->WriteData(0, texcoordBuffer.size()*sizeof(float), &texcoordBuffer.front(), FALSE);

    NBRE_HardwareIndexBuffer* indexBuf = rp.CreateIndexBuffer(NBRE_HardwareIndexBuffer::IT_16BIT, indexBuffer.size(), FALSE, NBRE_HardwareBuffer::HBU_STATIC);
    indexBuf->WriteData(0, indexBuffer.size()*sizeof(uint16), &indexBuffer.front(), FALSE);
    NBRE_IndexData* indexData = NBRE_NEW NBRE_IndexData(indexBuf, 0, indexBuf->GetNumIndexes());

    NBRE_Mesh* mesh = NBRE_NEW NBRE_Mesh(vertexData);
    mesh->CreateSubMesh(indexData, NBRE_PMT_TRIANGLE_LIST);
    mesh->CalculateBoundingBox();

    NBRE_ModelPtr model(NBRE_NEW NBRE_Model(NBRE_MeshPtr(mesh)));
    NBRE_Entity* entity = NBRE_NEW NBRE_Entity(mNBGMContext.renderingEngine->Context(), model, 0);

    return entity;
}

uint32
NBGM_NavEcmRouteLayer::CalcHermiteCurveStepCount( NBRE_Point3f v1, NBRE_Point3f v2, NBRE_Point3f t1, NBRE_Point3f t2 )
{
    static const float COS9 = 0.98768834059513772619004024769344f;
    static const float COS18 = 0.95105651629515357211643933337938f;
    static const float COS27 = 0.89100652418836786235970957141363f;
    static const float COS36 = 0.80901699437494742410229341718282f;
    static const float COS45 = 0.70710678118654752440084436210485f;
    static const float COS54 = 0.58778525229247312916870595463907f;
    static const float COS63 = 0.45399049973954679156040836635787f;
    static const float COS72 = 0.30901699437494742410229341718282f;
    static const float COS81 = 0.15643446504023086901010531946717f;
    static const float COS90 = 0.00000000000000000000000000000000f;
    static const float COS99 = -0.15643446504023086901010531946717f;
    static const float COS108 = -0.30901699437494742410229341718282f;
    static const float COS117 = -0.45399049973954679156040836635787f;
    static const float COS126 =  -0.58778525229247312916870595463907f;
    static const float COS135 = -0.70710678118654752440084436210485f;
    static const float COS144 = -0.80901699437494742410229341718282f;
    static const float COS153 = -0.89100652418836786235970957141363f;
    static const float COS162 = -0.95105651629515357211643933337938f;
    static const float COS171 = -0.98768834059513772619004024769344f;

    NBRE_Point3f v = v1 - v2;
    float len = v.Length();

    if(len < mNBGMContext.WorldToModel(ECM_ROUTE_POLYLINE_MIN_LENGTH))
    {
        return 0;
    }

    uint32 step = static_cast<uint32>(len / mNBGMContext.WorldToModel(ECM_ROUTE_POLYLINE_SGEMENT_LENGTH) + 1);

    if (NBRE_Math::IsZero(t1.Length(), 1e-5f) || NBRE_Math::IsZero(t2.Length(), 1e-5f))
    {
        return step;
    }

    t1.Normalise();
    t2.Normalise();

    float cosValue = t1.DotProduct(t2);
    uint32 theta = 1;

    if(cosValue<COS171)
    {
        theta = 20;
    }
    else if(cosValue<COS162)
    {
        theta = 19;
    }
    else if(cosValue<COS153)
    {
        theta = 18;
    }
    else if(cosValue<COS144)
    {
        theta = 17;
    }
    else if(cosValue<COS135)
    {
        theta = 16;
    }
    else if(cosValue<COS126)
    {
        theta = 15;
    }
    else if(cosValue<COS117)
    {
        theta = 14;
    }
    else if(cosValue<COS108)
    {
        theta = 13;
    }
    else if(cosValue<COS99)
    {
        theta = 12;
    }
    else if(cosValue<COS90)
    {
        theta = 11;
    }
    else if(cosValue<COS81)
    {
        theta = 10;
    }
    else if(cosValue<COS72)
    {
        theta = 9;
    }
    else if(cosValue<COS63)
    {
        theta = 8;
    }
    else if(cosValue<COS54)
    {
        theta = 7;
    }
    else if(cosValue<COS45)
    {
        theta = 6;
    }
    else if(cosValue<COS36)
    {
        theta = 5;
    }
    else if(cosValue<COS27)
    {
        theta = 4;
    }
    else if(cosValue<COS18)
    {
        theta = 3;
    }
    else if(cosValue<COS9)
    {
        theta = 2;
    }

    return (step>theta)?step:theta;
}

PAL_Error
NBGM_NavEcmRouteLayer::ConvertSplineToPolyline( const NBGM_Spline* spline, NBRE_Vector<NBRE_Point3f>& polyline)
{
    const float* RoutePosition = spline->GetPositionArray();
    const float* RoutePositionOutTangent = spline->GetIntanArray();
    const float* RoutePositionInTangent = spline->GetOuttanArray();
    uint32 vertexCount = spline->GetVertexCount();

    if ( RoutePosition == NULL ||  RoutePositionOutTangent == NULL || RoutePositionInTangent == NULL || vertexCount < 2)
    {
        return PAL_ErrBadParam;
    }

    uint32 routeindex = 0;
    NBRE_Point3f v1;
    NBRE_Point3f v2;
    NBRE_Point3f t1;
    NBRE_Point3f t2;
    NBRE_Point3f currentPos;
    uint32 counter = 0;

    for(uint32 j = 0; j < vertexCount-1; ++j)
    {
        routeindex = 3 * j;
        v1.x = RoutePosition[routeindex];
        v1.y = RoutePosition[routeindex+1];
        v1.z = RoutePosition[routeindex+2];

        t1.x = RoutePositionOutTangent[routeindex];
        t1.y = RoutePositionOutTangent[routeindex+1];
        t1.z = RoutePositionOutTangent[routeindex+2];

        routeindex += 3;

        v2.x = RoutePosition[routeindex];
        v2.y = RoutePosition[routeindex+1];
        v2.z = RoutePosition[routeindex+2];

        t2.x = -RoutePositionInTangent[routeindex];
        t2.y = -RoutePositionInTangent[routeindex+1];
        t2.z = -RoutePositionInTangent[routeindex+2];

        uint32 step = CalcHermiteCurveStepCount(v1, v2, t1, t2);
        if(step == 0)
        {
            NBRE_DebugLog(PAL_LogSeverityMajor, "NBGM_NavEcmRouteLayer::ConvertSplineToPolyline: The step is zero");
        }

        for(uint32 i = 0; i < step; ++i)
        {
            float s = static_cast<float>(i) / step;
            currentPos = CalcHermiteCurve(v1, v2, t1, t2, s);
            polyline.push_back(currentPos);
            ++counter;
        }
    }

    if(counter == 0)
    {
        return PAL_Failed;
    }

    routeindex = 3 * (vertexCount - 1);
    v1.x = RoutePosition[routeindex];
    v1.y = RoutePosition[routeindex + 1];
    v1.z = RoutePosition[routeindex + 2];

    v2 = v1 - currentPos;
    if (!NBRE_Math::IsZero(v2.Length(), 1e-5f))
    {
        polyline.push_back(v1);
        ++counter;
    }

    // if the number of vertex is less than 2 , we can not create scene.
    if(counter < 2)
    {
        return PAL_ErrWrongFormat;
    }

    return PAL_Ok;
}

NBRE_Point3f
NBGM_NavEcmRouteLayer::CalcHermiteCurve( const NBRE_Point3f& p1, const NBRE_Point3f& p2, const NBRE_Point3f& t1, const NBRE_Point3f& t2, float s )
{
    float h1 = 2 * s * s * s - 3 * s * s + 1;       // calculate basis function 1
    float h2 = -2 * s * s * s + 3 * s * s;          // calculate basis function 2
    float h3 = s * s * s - 2 * s * s + s;           // calculate basis function 3
    float h4 = s * s * s - s * s;                   // calculate basis function 4

    NBRE_Point3f a = p1 * h1;
    NBRE_Point3f b = p2 * h2;
    a += b;

    b = t1 * h3;
    a += b;

    b = t2 * h4;
    a += b;

    return a;
}

PAL_Error
NBGM_NavEcmRouteLayer::SnapRoute( NBGM_Location64& location, NBGM_SnapRouteInfo& snapRouteInfo ) const
{
    if(mSplinesPolyline.size() < 2)
    {
        return PAL_ErrBadParam;
    }

    NBGM_Location64 loc = location;
    loc.position.x = mNBGMContext.WorldToModel(loc.position.x);
    loc.position.y = mNBGMContext.WorldToModel(loc.position.y);
    loc.position.z = mNBGMContext.WorldToModel(loc.position.z);

    PAL_Error err = FindPolylineBestSeg(loc, FALSE, snapRouteInfo);
    if(err == PAL_Ok)
    {
        uint32 index = snapRouteInfo.GetLastPositionIndex();
        NBRE_Point3f nextPoint = mSplinesPolyline.at(index + 1);
        NBRE_Point3f prevPoint = mSplinesPolyline.at(index);
        NBRE_Point3f dir3d = nextPoint - prevPoint;
        NBRE_Point2f dir2d(dir3d.x, dir3d.y);

        loc.heading = dir2d.GetDirection();
        dir3d = NBRE_LinearInterpolatef::Lerp(prevPoint, nextPoint, snapRouteInfo.GetProjectPosition());
        loc.position.x = dir3d.x;
        loc.position.y = dir3d.y;
        loc.position.z = dir3d.z;
    }

    location.heading = loc.heading;
    location.position.x = mNBGMContext.ModelToWorld(loc.position.x);
    location.position.y = mNBGMContext.ModelToWorld(loc.position.y);
    location.position.z = mNBGMContext.ModelToWorld(loc.position.z);

    return err;
}

PAL_Error
NBGM_NavEcmRouteLayer::SnapToRouteStart( NBGM_Location64& location, NBGM_SnapRouteInfo& pRouteInfo ) const
{
    if(mSplinesPolyline.size() < 2)
    {
        return PAL_ErrBadParam;
    }

    NBRE_Point3f firstPoint = mSplinesPolyline.at(0);
    NBRE_Point3f nextPoint = mSplinesPolyline.at(1);
    NBRE_Point3f dir3d = nextPoint - firstPoint;
    NBRE_Point2f dir2d(dir3d.x, dir3d.y);
    location.heading = dir2d.GetDirection();

    location.position.x = mNBGMContext.ModelToWorld(firstPoint.x);
    location.position.y = mNBGMContext.ModelToWorld(firstPoint.y);
    location.position.z = mNBGMContext.ModelToWorld(firstPoint.z);

    return PAL_Ok;
}

PAL_Error
NBGM_NavEcmRouteLayer::CalculateForecastPoint( const NBGM_SnapRouteInfo& prevSnapInfo, float distance, NBRE_Point3f& resultPoint ) const
{
    if (mSplinesPolyline.size() < 2 || prevSnapInfo.GetLastPositionIndex() == NBRE_INVALID_INDEX || prevSnapInfo.GetLastPositionIndex() > mSplinesPolyline.size())
    {
        return PAL_ErrBadParam;
    }

    uint32 lastIndex = prevSnapInfo.GetLastPositionIndex();
    uint32 endIndex = mSplinesPolyline.size();
    NBRE_Point3f prevPoint = mSplinesPolyline.at(lastIndex);
    NBRE_Point3f nextPoint = mSplinesPolyline.at(lastIndex + 1);
    NBRE_Point3f resultVector = nextPoint - prevPoint;
    NBRE_Point3f projectVector = resultVector * prevSnapInfo.GetProjectPosition();
    float offsetBetween2Points = projectVector.Length();

    if(distance >= 0)
    {
        distance += offsetBetween2Points;

        while(distance > 0 && lastIndex != endIndex - 1)
        {
            prevPoint = mSplinesPolyline.at(lastIndex);
            nextPoint = mSplinesPolyline.at(lastIndex + 1);
            float seglen = prevPoint.DistanceTo(nextPoint);
            distance -= seglen;

            if(distance <= 0)
            {
                float t = distance / seglen;
                NBRE_Point3f dir = prevPoint - nextPoint;
                dir *= t;
                resultPoint = nextPoint + dir;
                return PAL_Ok;
            }
            ++lastIndex;
        }
    }
    else
    {
        distance -= offsetBetween2Points;

        while(distance < 0 && lastIndex != 0)
        {
            prevPoint = mSplinesPolyline.at(lastIndex);
            nextPoint = mSplinesPolyline.at(lastIndex - 1);
            float seglen = prevPoint.DistanceTo(nextPoint);
            distance += seglen;

            if(distance >= 0)
            {
                float t = distance / seglen;
                NBRE_Point3f dir = nextPoint - prevPoint;
                dir *= t;
                resultPoint = prevPoint + dir;
                return PAL_Ok;
            }
            --lastIndex;
        }
    }

    return PAL_ErrNotFound;
}

PAL_Error
NBGM_NavEcmRouteLayer::CalculateDistanceFromRouteStartToSnapPosition(const NBGM_SnapRouteInfo& snapInfo, float& distance) const
{
    if (mSplinesPolyline.size() < 2 || snapInfo.GetLastPositionIndex() == NBRE_INVALID_INDEX || snapInfo.GetLastPositionIndex() >= mSplinesPolyline.size())
    {
        return PAL_ErrBadParam;
    }

    uint32 currentIndex = 0;
    uint32 lastIndex = mSplinesPolyline.size() - 1;
    uint32 snapLastIndex = snapInfo.GetLastPositionIndex();

    while(currentIndex != lastIndex)
    {
        NBRE_Point3f prevPos = mSplinesPolyline.at(currentIndex);
        NBRE_Point3f nextPos = mSplinesPolyline.at(currentIndex + 1);

        float seglen = prevPos.DistanceTo((nextPos));
        if(currentIndex == snapLastIndex)
        {
            distance += seglen * snapInfo.GetProjectPosition();
            break;
        }
        distance += seglen;
        ++currentIndex;
    }

    return PAL_Ok;
}

NBGM_NavEcmRouteLayer::NBGM_Spline::NBGM_Spline( const NBGM_Context& context, const uint8* data, uint32 size)
:mVertexCount(0)
,mFromLinkPVID(0)
,mToLinkPVID(0)
,mFromLinkDirection(0)
,mToLinkDirection(0)
,mPositionArray(NULL)
,mOuttanArray(NULL)
,mIntanArray(NULL)
{
    PAL_Error err = LoadSplineFromBinaryData(context, data, size);
    if (err != PAL_Ok)
    {
        NBRE_DebugLog(PAL_LogSeverityMajor, "NBGM_NavEcmRouteLayer::NBGM_Spline::NBGM_Spline failed, err = %x", err);
    }
}

NBGM_NavEcmRouteLayer::NBGM_Spline::~NBGM_Spline()
{
    NBRE_DELETE_ARRAY mPositionArray;
}

uint32
NBGM_NavEcmRouteLayer::NBGM_Spline::GetVertexCount() const
{
    return mVertexCount;
}

const float*
NBGM_NavEcmRouteLayer::NBGM_Spline::GetPositionArray() const
{
    return mPositionArray;
}

const float*
NBGM_NavEcmRouteLayer::NBGM_Spline::GetOuttanArray() const
{
    return mOuttanArray;
}

const float*
NBGM_NavEcmRouteLayer::NBGM_Spline::GetIntanArray() const
{
    return mIntanArray;
}

PAL_Error
NBGM_NavEcmRouteLayer::NBGM_Spline::LoadSplineFromBinaryData( const NBGM_Context& context, const uint8* data, uint32 size )
{
    const uint8 SUPPORTED_SPLINE_DATA_VERSION = 0;

    if(data == NULL || size == 0)
    {
        return PAL_ErrBadParam;
    }

    uint8 version = data[0];
    uint8 flag = data[1];

    if (flag != 0 || version != SUPPORTED_SPLINE_DATA_VERSION)
    {
        return PAL_ErrWrongFormat;
    }

    uint32 dataReadOffset = 2;

    nsl_memcpy(&mFromLinkPVID, data+dataReadOffset, sizeof(uint32));
    dataReadOffset += sizeof(uint32);

    nsl_memcpy(&mToLinkPVID, data+dataReadOffset, sizeof(uint32));
    dataReadOffset += sizeof(uint32);

    nsl_memcpy(&mFromLinkDirection, data+dataReadOffset, sizeof(uint32));
    dataReadOffset += sizeof(uint32);

    nsl_memcpy(&mToLinkDirection, data+dataReadOffset, sizeof(uint32));
    dataReadOffset += sizeof(uint32);

    nsl_memcpy(&mVertexCount, data+dataReadOffset, sizeof(uint32));
    dataReadOffset+= sizeof(uint32);

    if ((mVertexCount > 0) && ((size - 2) >= mVertexCount * 3 * 3 * sizeof(float)))
    {
        mPositionArray = NBRE_NEW float[mVertexCount * 3 * 3];
        mOuttanArray = mPositionArray + mVertexCount * 3;
        mIntanArray = mOuttanArray + mVertexCount * 3;

        nsl_memcpy(mPositionArray, data+dataReadOffset, mVertexCount*3*sizeof(float));
        dataReadOffset += mVertexCount*3*sizeof(float);

        nsl_memcpy(mOuttanArray, data+dataReadOffset, mVertexCount*3*sizeof(float));
        dataReadOffset += mVertexCount*3*sizeof(float);

        nsl_memcpy(mIntanArray, data+dataReadOffset, mVertexCount*3*sizeof(float));

        for(uint32 i = 0; i < mVertexCount * 3; ++i)
        {
            mPositionArray[i] = context.WorldToModel(mPositionArray[i]);
            mOuttanArray[i] = context.WorldToModel(mOuttanArray[i]);
            mIntanArray[i] = context.WorldToModel(mIntanArray[i]);
        }
    }
    else
    {
        return PAL_ErrWrongFormat;
    }

    return PAL_Ok;
}

NBGM_SnapRouteInfo::NBGM_SnapRouteInfo()
:mLastPosIndex(NBRE_INVALID_INDEX)
,mProjectPosition(0.0f)
,mClosestDistance(0.0f)
{
}

NBGM_SnapRouteInfo::~NBGM_SnapRouteInfo()
{
}

void
NBGM_SnapRouteInfo::SetLastPositionIndex( uint32 index )
{
    mLastPosIndex = index;
}

void
NBGM_SnapRouteInfo::SetProjectPosition( float pos )
{
    mProjectPosition = pos;
}

void
NBGM_SnapRouteInfo::SetClosetDistance( float distance )
{
    mClosestDistance = distance;
}

uint32
NBGM_SnapRouteInfo::GetLastPositionIndex() const
{
    return mLastPosIndex;
}

float
NBGM_SnapRouteInfo::GetProjectPosition() const
{
    return mProjectPosition;
}

float
NBGM_SnapRouteInfo::GetClosetDistance() const
{
    return mClosestDistance;
}

nb_boolean
NBGM_SnapRouteInfo::IsValid() const
{
    if (mLastPosIndex == NBRE_INVALID_INDEX)
    {
        return FALSE;
    }

    return TRUE;
}
