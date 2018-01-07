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
#include "nbgmbuildutility.h"
#include "nbgmconst.h"
#include "nbrelog.h"
#include "nbretexturemanager.h"
#include "nbremath.h"
#include "nbrelinearinterpolate.h"
#include "nbrerenderpal.h"
#include "nbgmdom.h"
#include "nbrepngtextureimage.h"
#include "nbresubmodel.h"
#include "nbremodel.h"
#include "palmath.h"
#include "paltriangulation.h"

static const float TURN_X_COORD = 0.5f;
static const float TURN_Y_COORD = 0.5f;
static const float TURN_WIDTH = 0.5f;

static const float ROAD_X_COORD = 0.0f;
static const float ROAD_Y_COORD = 0.75f;
static const float ROAD_WIDTH = 1.0f;

static const float SQRT_HALF3 = 0.866f;

static double
Hypot(double x, double y)
{
    return nsl_sqrt(x*x + y*y);
}

static void
GeneratTriangleStripIndex(NBGM_ShortBuffer& indexBuffer, uint16 start, uint16 end)
{
    for(uint16 i = start; i < end-2; i += 2)
    {
        indexBuffer.push_back(i);
        indexBuffer.push_back(i + 1);
        indexBuffer.push_back(i + 2);

        indexBuffer.push_back(i + 1);
        indexBuffer.push_back(i + 3);
        indexBuffer.push_back(i + 2);
    }
}

static void
FillVertexNormalTexCoord2D(NBGM_FloatBuffer& vertexBuffer, NBGM_FloatBuffer& normalBuffer, NBGM_FloatBuffer& texcoordBuffer,
                           NBRE_Point2f* pt, float normalX, float normalY, float texX, float texY)
{
    vertexBuffer.push_back(pt->x);
    vertexBuffer.push_back(pt->y);

    normalBuffer.push_back(normalX);
    normalBuffer.push_back(normalY);

    texcoordBuffer.push_back(texX);
    texcoordBuffer.push_back(texY);
}

static void
FillVertexNormalMultiTexCoord2D(NBGM_FloatBuffer& vertexBuffer, NBGM_FloatBuffer& normalBuffer, NBGM_FloatBuffer& texcoordBuffer1,
                                NBGM_FloatBuffer& texcoordBuffer2, const NBRE_Point2f* pt, float normalX, float normalY, float texX,
                                float texY, float texX2, float texY2)
{
    vertexBuffer.push_back(pt->x);
    vertexBuffer.push_back(pt->y);

    normalBuffer.push_back(normalX);
    normalBuffer.push_back(normalY);

    texcoordBuffer1.push_back(texX);
    texcoordBuffer1.push_back(texY);

    texcoordBuffer2.push_back(texX2);
    texcoordBuffer2.push_back(texY2);
}

PAL_Error
NBGM_BuildModelUtility::PolylinePack2TristripeWithRoundEndpoint(NBRE_Array* polyline, float width, float texX, float texY, NBGM_FloatBuffer& vertexBuffer, NBGM_FloatBuffer& normalBuffer,
                                                                NBGM_FloatBuffer& texcoordBuffer1, NBGM_FloatBuffer& texcoordBuffer2, NBGM_ShortBuffer& indexBuffer)
{
    if(polyline == NULL || width < 0.0f)
    {
        return PAL_ErrBadParam;
    }

    float normalLen = width/2.f;
    uint16 vertexIndex = static_cast<int16>(vertexBuffer.size()/2);
    uint16 start = vertexIndex;
    uint32 numSeg = NBRE_ArrayGetSize(polyline);
    uint32 i = 0;
    NBRE_Point2f* pt1 = static_cast<NBRE_Point2f*>(NBRE_ArrayGetAt(polyline, i++));
    NBRE_Point2f* pt2 = NULL;

    float normalX = 0;
    float normalY = 0;
    while (normalX == 0 && normalY == 0 && i < numSeg)
    {
        pt2 = static_cast<NBRE_Point2f*>(NBRE_ArrayGetAt(polyline, i++));

        normalX = pt1->y - pt2->y;
        normalY = pt2->x - pt1->x;
    }
    if (normalX == 0 && normalY == 0)
    {
        return PAL_ErrBadParam;
    }

    double magnitude = Hypot(normalX, normalY);
    normalX *= static_cast<float>(normalLen/magnitude);
    normalY *= static_cast<float>(normalLen/magnitude);

    FillVertexNormalMultiTexCoord2D(vertexBuffer, normalBuffer, texcoordBuffer1, texcoordBuffer2,
                                    pt1,
                                    -normalY - normalX, normalX - normalY,
                                    TURN_X_COORD - TURN_WIDTH, TURN_Y_COORD - TURN_WIDTH,
                                    texX, texY);

    FillVertexNormalMultiTexCoord2D(vertexBuffer, normalBuffer, texcoordBuffer1, texcoordBuffer2,
                                    pt1,
                                    -normalY + normalX, normalX + normalY,
                                    TURN_X_COORD + TURN_WIDTH, TURN_Y_COORD - TURN_WIDTH,
                                    texX, texY);

    FillVertexNormalMultiTexCoord2D(vertexBuffer, normalBuffer, texcoordBuffer1, texcoordBuffer2,
                                    pt1,
                                    -normalX, -normalY,
                                    TURN_X_COORD - TURN_WIDTH, TURN_Y_COORD,
                                    texX, texY);

    FillVertexNormalMultiTexCoord2D(vertexBuffer, normalBuffer, texcoordBuffer1, texcoordBuffer2,
                                    pt1,
                                    normalX, normalY,
                                    TURN_X_COORD + TURN_WIDTH, TURN_Y_COORD,
                                    texX, texY);
    vertexIndex += 4;

    GeneratTriangleStripIndex(indexBuffer, start, vertexIndex);
    start = vertexIndex;

    FillVertexNormalMultiTexCoord2D(vertexBuffer, normalBuffer, texcoordBuffer1, texcoordBuffer2,
                                    pt1,
                                    -normalX, -normalY,
                                    ROAD_X_COORD, ROAD_Y_COORD,
                                    texX, texY);

    FillVertexNormalMultiTexCoord2D(vertexBuffer, normalBuffer, texcoordBuffer1, texcoordBuffer2,
                                    pt1,
                                    normalX, normalY,
                                    ROAD_X_COORD + ROAD_WIDTH, ROAD_Y_COORD,
                                    texX, texY);
    vertexIndex += 2;

    float prevNormalX = normalX;
    float prevNormalY = normalY;

    pt1 = pt2;
    for (; i < numSeg; ++i)
    {
        pt2 = static_cast<NBRE_Point2f*>(NBRE_ArrayGetAt(polyline, i));
        normalX = pt1->y - pt2->y;
        normalY = pt2->x - pt1->x;
        magnitude = Hypot(normalX, normalY);

        if (magnitude == 0)
        {
            continue;
        }

        normalX *= static_cast<float>(normalLen/magnitude);
        normalY *= static_cast<float>(normalLen/magnitude);

        double cos2Theta = (normalX*prevNormalX + normalY*prevNormalY)/(normalLen*normalLen);
        float tempNormalX = 0.0f;
        float tempNormalY = 0.0f;
        double tempMagnitude = 0.0;

        if(cos2Theta < SQRT_HALF3)
        {
            FillVertexNormalMultiTexCoord2D(vertexBuffer, normalBuffer, texcoordBuffer1, texcoordBuffer2,
                                            pt1,
                                            -prevNormalX, -prevNormalY,
                                            TURN_X_COORD - TURN_WIDTH, TURN_Y_COORD,
                                            texX, texY);

            FillVertexNormalMultiTexCoord2D(vertexBuffer, normalBuffer, texcoordBuffer1, texcoordBuffer2,
                                            pt1,
                                            prevNormalX, prevNormalY,
                                            TURN_X_COORD + TURN_WIDTH, TURN_Y_COORD,
                                            texX, texY);

            FillVertexNormalMultiTexCoord2D(vertexBuffer, normalBuffer, texcoordBuffer1, texcoordBuffer2,
                                            pt1,
                                            prevNormalY - prevNormalX, -prevNormalX - prevNormalY,
                                            TURN_X_COORD - TURN_WIDTH, TURN_Y_COORD - TURN_WIDTH,
                                            texX, texY);

            FillVertexNormalMultiTexCoord2D(vertexBuffer, normalBuffer, texcoordBuffer1, texcoordBuffer2,
                                            pt1,
                                            prevNormalY + prevNormalX, -prevNormalX + prevNormalY,
                                            TURN_X_COORD + TURN_WIDTH, TURN_Y_COORD - TURN_WIDTH,
                                            texX, texY);
            vertexIndex += 4;

            GeneratTriangleStripIndex(indexBuffer, start, vertexIndex);
            start = vertexIndex;

            tempNormalX = normalX ;
            tempNormalY = normalY ;
        }
        else
        {
            double cosTheta = nsl_sqrt((cos2Theta+1)/2);
            tempNormalX = (normalX + prevNormalX) ;
            tempNormalY = (normalY + prevNormalY) ;
            tempMagnitude = Hypot(tempNormalX, tempNormalY);
            tempNormalX *= static_cast<float>(normalLen/cosTheta/tempMagnitude);
            tempNormalY *= static_cast<float>(normalLen/cosTheta/tempMagnitude);
        }
        FillVertexNormalMultiTexCoord2D(vertexBuffer, normalBuffer, texcoordBuffer1, texcoordBuffer2,
                                        pt1,
                                        -tempNormalX, -tempNormalY,
                                        ROAD_X_COORD, ROAD_Y_COORD,
                                        texX, texY);

        FillVertexNormalMultiTexCoord2D(vertexBuffer, normalBuffer, texcoordBuffer1, texcoordBuffer2,
                                        pt1,
                                        tempNormalX, tempNormalY,
                                        ROAD_X_COORD + ROAD_WIDTH, ROAD_Y_COORD,
                                        texX, texY);
        vertexIndex += 2;

        prevNormalX = normalX;
        prevNormalY = normalY;

        pt1 = pt2;
    }

    if (magnitude == 0)
    {
        normalX = prevNormalX ;
        normalY = prevNormalY ;
    }

    FillVertexNormalMultiTexCoord2D(vertexBuffer, normalBuffer, texcoordBuffer1, texcoordBuffer2,
                                    pt2,
                                    -normalX, -normalY,
                                    TURN_X_COORD - TURN_WIDTH, TURN_Y_COORD,
                                    texX, texY);

    FillVertexNormalMultiTexCoord2D(vertexBuffer, normalBuffer, texcoordBuffer1, texcoordBuffer2,
                                    pt2,
                                    normalX, normalY,
                                    TURN_X_COORD + TURN_WIDTH, TURN_Y_COORD,
                                    texX, texY);

    FillVertexNormalMultiTexCoord2D(vertexBuffer, normalBuffer, texcoordBuffer1, texcoordBuffer2,
                                    pt2,
                                    normalY - normalX, -normalX - normalY,
                                    TURN_X_COORD - TURN_WIDTH, TURN_Y_COORD - TURN_WIDTH,
                                    texX, texY);

    FillVertexNormalMultiTexCoord2D(vertexBuffer, normalBuffer, texcoordBuffer1, texcoordBuffer2,
                                    pt2,
                                    normalY + normalX, -normalX + normalY,
                                    TURN_X_COORD + TURN_WIDTH, TURN_Y_COORD - TURN_WIDTH,
                                    texX, texY);
    vertexIndex += 4;

    GeneratTriangleStripIndex(indexBuffer, start, vertexIndex);

    return PAL_Ok;
}

PAL_Error
NBGM_BuildModelUtility::PatternPolylinePack2Tristripe(NBGM_ResourceContext& /*resourceContext*/, NBRE_Array* polyline, float width, float texRatio, NBGM_FloatBuffer& vertexBuffer,
                                                      NBGM_FloatBuffer& normalBuffer, NBGM_FloatBuffer& texcoordBuffer, NBGM_ShortBuffer& indexBuffer, double& roadSegTotalLen)
{
    uint32 numSeg = NBRE_ArrayGetSize(polyline);

    if(numSeg < 2 || width < 0.0f || roadSegTotalLen < 0.0f)
    {
        return PAL_ErrBadParam;
    }

    float normalLen = width * 0.5f;
    uint16 vertexIndex = static_cast<int16>(vertexBuffer.size()/2);
    uint16 startIndex = vertexIndex;
    uint32 i = 0;
    NBRE_Point2f* pt1 = static_cast<NBRE_Point2f*>(NBRE_ArrayGetAt(polyline, i++));
    NBRE_Point2f* pt2 = NULL;
    NBRE_Point2f normal;

    while ((normal.Length() == 0) && i < numSeg)
    {
        pt2 = static_cast<NBRE_Point2f*>(NBRE_ArrayGetAt(polyline, i++));

        normal.x = pt1->y - pt2->y;
        normal.y = pt2->x - pt1->x;
    }
    if (normal.Length() == 0)
    {
        return PAL_ErrBadParam;
    }

    normal.Normalise();
    NBRE_Point2f extendVec = normal * normalLen;

    float texX = static_cast<float>(roadSegTotalLen / width * texRatio);
    FillVertexNormalTexCoord2D(vertexBuffer, normalBuffer, texcoordBuffer, pt1, -extendVec.x, -extendVec.y, texX, 0.0f);
    FillVertexNormalTexCoord2D(vertexBuffer, normalBuffer, texcoordBuffer, pt1, extendVec.x, extendVec.y, texX, 1.0f);

    vertexIndex += 2;

    float lenX = pt2->x - pt1->x;
    float lenY = pt2->y - pt1->y;

    roadSegTotalLen += Hypot(lenX, lenY);

    pt1 = pt2;
    for (; i < numSeg; ++i)
    {
        pt2 = static_cast<NBRE_Point2f*>(NBRE_ArrayGetAt(polyline, i));
        NBRE_Point2f nextNormal(pt1->y - pt2->y, pt2->x - pt1->x);

        if (nextNormal.Length() == 0)
        {
            NBRE_DebugLog(PAL_LogSeverityCritical, "PatternPolylinePack2Tristripe magnitude == 0");
            continue;
        }
        nextNormal.Normalise();


        double cos2Theta = normal.DotProduct(nextNormal);

        texX = static_cast<float>(roadSegTotalLen / width * texRatio);

        if(cos2Theta < SQRT_HALF3)
        {
            extendVec = normal * normalLen;
            FillVertexNormalTexCoord2D(vertexBuffer, normalBuffer, texcoordBuffer, pt1, -extendVec.x, -extendVec.y, texX, 0.0f);
            FillVertexNormalTexCoord2D(vertexBuffer, normalBuffer, texcoordBuffer, pt1, extendVec.x, extendVec.y, texX, 1.0f);
            vertexIndex += 2;
            extendVec = nextNormal * normalLen;
        }
        else
        {
            NBRE_Point2f addNormal = nextNormal + normal;
            addNormal.Normalise();
            extendVec = addNormal * normalLen;
        }

        FillVertexNormalTexCoord2D(vertexBuffer, normalBuffer, texcoordBuffer, pt1, -extendVec.x, -extendVec.y, texX, 0.0f);
        FillVertexNormalTexCoord2D(vertexBuffer, normalBuffer, texcoordBuffer, pt1, extendVec.x, extendVec.y, texX, 1.0f);

        vertexIndex += 2;

        lenX = pt2->x - pt1->x;
        lenY = pt2->y - pt1->y;
        roadSegTotalLen += Hypot(lenX, lenY);

        normal = nextNormal;

        pt1 = pt2;
    }

    normal.Normalise();
    extendVec = normal * normalLen;

    texX = static_cast<float>(roadSegTotalLen / width * texRatio);
    FillVertexNormalTexCoord2D(vertexBuffer, normalBuffer, texcoordBuffer, pt1, -extendVec.x, -extendVec.y, texX, 0.0f);
    FillVertexNormalTexCoord2D(vertexBuffer, normalBuffer, texcoordBuffer, pt1, extendVec.x, extendVec.y, texX, 1.0f);

    vertexIndex += 2;
    GeneratTriangleStripIndex(indexBuffer, startIndex, vertexIndex);

    return PAL_Ok;
}

static void
GeneratNavRouteTriangleStripIndex(NBGM_ShortBuffer& indexBuffer, uint16 start, uint16 end)
{
    for(uint16 i = start; i < end-2; i += 2)
    {
        indexBuffer.push_back(i);
        indexBuffer.push_back(i + 3);
        indexBuffer.push_back(i + 1);

        indexBuffer.push_back(i);
        indexBuffer.push_back(i + 2);
        indexBuffer.push_back(i + 3);
    }
}

PAL_Error
NBGM_BuildModelUtility::NavVectorPolyline2Tristripe(NBRE_Array* polyline, float width, float texX, float texY, NBGM_FloatBuffer& vertexBuffer,
        NBGM_FloatBuffer& texcoord1Buffer, NBGM_FloatBuffer& texcoord2Buffer, NBGM_ShortBuffer& indexBuffer)
{
    double tempMagnitude = 0;
    float normalLen = width/2.f;
    uint16 vertexIndex = static_cast<uint16>(vertexBuffer.size()/2);

    if(polyline == NULL)
    {
        return PAL_ErrBadParam;
    }

    uint32 numSeg = NBRE_ArrayGetSize(polyline);
    uint32 i = 0;
    NBRE_Point2f* pt1 = static_cast<NBRE_Point2f*>(NBRE_ArrayGetAt(polyline, i++));
    NBRE_Point2f* pt2 = NULL;
    float normalX = 0.0f;
    float normalY = 0.0f;

    while (normalX == 0 && normalY == 0 && i < numSeg)
    {
        pt2 = static_cast<NBRE_Point2f*>(NBRE_ArrayGetAt(polyline, i++));

        normalX = pt1->y - pt2->y;
        normalY = pt2->x - pt1->x;
    }
    if (normalX == 0 && normalY == 0)
    {
        return PAL_ErrBadParam;
    }

    double magnitude = Hypot(normalX, normalY);
    normalX *= static_cast<float>(normalLen/magnitude);
    normalY *= static_cast<float>(normalLen/magnitude);

    vertexBuffer.push_back(pt1->x - normalY - normalX);
    vertexBuffer.push_back(pt1->y + normalX - normalY);
    texcoord1Buffer.push_back(TURN_X_COORD - TURN_WIDTH);
    texcoord1Buffer.push_back(TURN_Y_COORD - TURN_WIDTH);
    texcoord2Buffer.push_back(texX);
    texcoord2Buffer.push_back(texY);

    vertexBuffer.push_back(pt1->x - normalY + normalX);
    vertexBuffer.push_back(pt1->y + normalX + normalY);
    texcoord1Buffer.push_back(TURN_X_COORD + TURN_WIDTH);
    texcoord1Buffer.push_back(TURN_Y_COORD - TURN_WIDTH);
    texcoord2Buffer.push_back(texX);
    texcoord2Buffer.push_back(texY);

    vertexBuffer.push_back(pt1->x - normalX);
    vertexBuffer.push_back(pt1->y - normalY);
    texcoord1Buffer.push_back(TURN_X_COORD - TURN_WIDTH);
    texcoord1Buffer.push_back(TURN_Y_COORD);
    texcoord2Buffer.push_back(texX);
    texcoord2Buffer.push_back(texY);

    vertexBuffer.push_back(pt1->x + normalX);
    vertexBuffer.push_back(pt1->y + normalY);
    texcoord1Buffer.push_back(TURN_X_COORD + TURN_WIDTH);
    texcoord1Buffer.push_back(TURN_Y_COORD);
    texcoord2Buffer.push_back(texX);
    texcoord2Buffer.push_back(texY);

    GeneratNavRouteTriangleStripIndex(indexBuffer, vertexIndex, static_cast<uint16>(vertexBuffer.size()/2));
    vertexIndex = static_cast<uint16>(vertexBuffer.size()/2);

    vertexBuffer.push_back(pt1->x - normalX);
    vertexBuffer.push_back(pt1->y - normalY);
    texcoord1Buffer.push_back(ROAD_X_COORD);
    texcoord1Buffer.push_back(ROAD_Y_COORD);
    texcoord2Buffer.push_back(texX);
    texcoord2Buffer.push_back(texY);

    vertexBuffer.push_back(pt1->x + normalX);
    vertexBuffer.push_back(pt1->y + normalY);
    texcoord1Buffer.push_back(ROAD_X_COORD + ROAD_WIDTH);
    texcoord1Buffer.push_back(ROAD_Y_COORD);
    texcoord2Buffer.push_back(texX);
    texcoord2Buffer.push_back(texY);

    float prevNormalX = normalX;
    float prevNormalY = normalY;

    pt1 = pt2;
    for (; i < numSeg; ++i)
    {
        pt2 = static_cast<NBRE_Point2f*>(NBRE_ArrayGetAt(polyline, i));
        normalX = pt1->y - pt2->y;
        normalY = pt2->x - pt1->x;
        magnitude = Hypot(normalX, normalY);

        if (magnitude == 0)
        {
            continue;
        }

        normalX *= static_cast<float>(normalLen/magnitude);
        normalY *= static_cast<float>(normalLen/magnitude);

        float tempNormalX = 0.0f;
        float tempNormalY = 0.0f;

        double cos2Theta = (normalX*prevNormalX + normalY*prevNormalY)/(normalLen*normalLen);
        if(cos2Theta < SQRT_HALF3)
        {
            vertexBuffer.push_back(pt1->x - prevNormalX);
            vertexBuffer.push_back(pt1->y - prevNormalY);
            texcoord1Buffer.push_back(TURN_X_COORD - TURN_WIDTH);
            texcoord1Buffer.push_back(TURN_Y_COORD);
            texcoord2Buffer.push_back(texX);
            texcoord2Buffer.push_back(texY);

            vertexBuffer.push_back(pt1->x + prevNormalX);
            vertexBuffer.push_back(pt1->y + prevNormalY);
            texcoord1Buffer.push_back(TURN_X_COORD + TURN_WIDTH);
            texcoord1Buffer.push_back(TURN_Y_COORD);
            texcoord2Buffer.push_back(texX);
            texcoord2Buffer.push_back(texY);

            vertexBuffer.push_back(pt1->x + prevNormalY - prevNormalX);
            vertexBuffer.push_back(pt1->y - prevNormalX - prevNormalY);
            texcoord1Buffer.push_back(TURN_X_COORD - TURN_WIDTH);
            texcoord1Buffer.push_back(TURN_Y_COORD - TURN_WIDTH);
            texcoord2Buffer.push_back(texX);
            texcoord2Buffer.push_back(texY);

            vertexBuffer.push_back(pt1->x + prevNormalY + prevNormalX);
            vertexBuffer.push_back(pt1->y - prevNormalX + prevNormalY);
            texcoord1Buffer.push_back(TURN_X_COORD + TURN_WIDTH);
            texcoord1Buffer.push_back(TURN_Y_COORD - TURN_WIDTH);
            texcoord2Buffer.push_back(texX);
            texcoord2Buffer.push_back(texY);

            GeneratNavRouteTriangleStripIndex(indexBuffer, vertexIndex, static_cast<uint16>(vertexBuffer.size()/2));
            vertexIndex = static_cast<uint16>(vertexBuffer.size()/2);

            tempNormalX = normalX ;
            tempNormalY = normalY ;
        }
        else
        {
            double cosTheta = nsl_sqrt((cos2Theta+1)/2);
            tempNormalX = (normalX + prevNormalX) ;
            tempNormalY = (normalY + prevNormalY) ;
            tempMagnitude = Hypot(tempNormalX, tempNormalY);
            tempNormalX *= static_cast<float>(normalLen/cosTheta/tempMagnitude);
            tempNormalY *= static_cast<float>(normalLen/cosTheta/tempMagnitude);
        }
        vertexBuffer.push_back(pt1->x - tempNormalX);
        vertexBuffer.push_back(pt1->y - tempNormalY);
        texcoord1Buffer.push_back(ROAD_X_COORD);
        texcoord1Buffer.push_back(ROAD_Y_COORD);
        texcoord2Buffer.push_back(texX);
        texcoord2Buffer.push_back(texY);

        vertexBuffer.push_back(pt1->x + tempNormalX);
        vertexBuffer.push_back(pt1->y + tempNormalY);
        texcoord1Buffer.push_back(ROAD_X_COORD + ROAD_WIDTH);
        texcoord1Buffer.push_back(ROAD_Y_COORD);
        texcoord2Buffer.push_back(texX);
        texcoord2Buffer.push_back(texY);

        prevNormalX = normalX;
        prevNormalY = normalY;

        pt1 = pt2;
    }

    if (magnitude == 0)
    {
        normalX = prevNormalX ;
        normalY = prevNormalY ;
    }

    vertexBuffer.push_back(pt2->x - normalX);
    vertexBuffer.push_back(pt2->y - normalY);
    texcoord1Buffer.push_back(TURN_X_COORD - TURN_WIDTH);
    texcoord1Buffer.push_back(TURN_Y_COORD);
    texcoord2Buffer.push_back(texX);
    texcoord2Buffer.push_back(texY);

    vertexBuffer.push_back(pt2->x + normalX);
    vertexBuffer.push_back(pt2->y + normalY);
    texcoord1Buffer.push_back(TURN_X_COORD + TURN_WIDTH);
    texcoord1Buffer.push_back(TURN_Y_COORD);
    texcoord2Buffer.push_back(texX);
    texcoord2Buffer.push_back(texY);

    vertexBuffer.push_back(pt2->x + normalY - normalX);
    vertexBuffer.push_back(pt2->y - normalX - normalY);
    texcoord1Buffer.push_back(TURN_X_COORD - TURN_WIDTH);
    texcoord1Buffer.push_back(TURN_Y_COORD - TURN_WIDTH);
    texcoord2Buffer.push_back(texX);
    texcoord2Buffer.push_back(texY);

    vertexBuffer.push_back(pt2->x + normalY + normalX);
    vertexBuffer.push_back(pt2->y - normalX + normalY);
    texcoord1Buffer.push_back(TURN_X_COORD + TURN_WIDTH);
    texcoord1Buffer.push_back(TURN_Y_COORD - TURN_WIDTH);
    texcoord2Buffer.push_back(texX);
    texcoord2Buffer.push_back(texY);

    GeneratNavRouteTriangleStripIndex(indexBuffer, vertexIndex, static_cast<uint16>(vertexBuffer.size()/2));

    return PAL_Ok;
}

static uint8*
CircleImage2D( int32 width, int32 height, int32 radius, uint8 r, uint8 g, uint8 b )
{
    uint32 count = width*height*4;
    uint8* pixels = NBRE_NEW uint8[count];
    if(pixels != NULL)
    {
        int32 radius2 = radius*radius;
        uint32 index = 0;
        int32 x = 0;
        int32 y = 0;
        for(index = 0; index < count; index += 4)
        {
            pixels[index] = r;
            pixels[index+1] = g;
            pixels[index+2] = b;
            pixels[index+3] = 0;
        }
        index = 0;
        for(y = -height/2; y < height/2; ++y)
        {
            int32 y2 = y*y;
            for(x = -width/2; x < width/2; ++x, index += 4)
            {
                pixels[index] = r;
                pixels[index+1] = g;
                pixels[index+2] = b;
                if(x*x + y2 < radius2)
                {
                    pixels[index+3] = 0xff;
                }
            }
        }
    }
    return pixels;
}

static uint8*
HalfImage2D(uint8* originPx, uint32 length, int32 originWidth, uint8 r, uint8 g, uint8 b )
{
    length /= 4;
    if(originWidth > 1)
    {
        uint8* pixels = NBRE_NEW uint8[length];
        int32 x = 0;
        int32 y = 0;
        uint32 index = 0;
        uint32 acc = 0;
        for(index = 0; index < length; index += 4, x += 2)
        {
            pixels[index] = r;
            pixels[index+1] = g;
            pixels[index+2] = b;
            if(x >= originWidth)
            {
                x = 0;
                y += 2;
            }
            acc = (0xff & originPx[(x+originWidth*y)*4+3])+(0xff & originPx[(x+1+originWidth*y)*4+3])+
                (0xff & originPx[(x+originWidth*(y+1))*4+3])+(0xff & originPx[(x+1+originWidth*(y+1))*4+3]);
            pixels[index+3] = (uint8) (acc>>2);
        }
        return pixels;
    }
    return NULL;
}

static void
HalfRectImage2D( uint8* pixels, uint32 length, int32 width, int32 height, int32 radius, uint8 r, uint8 g, uint8 b )
{
    uint32 index = length/2;
    int32 x = 0;
    int32 y = 0;
    for(y = 0; y < height/2; ++y){
        for(x = -width/2; x < width/2; ++x, index += 4)
        {
            pixels[index] = r;
            pixels[index+1] = g;
            pixels[index+2] = b;
            if(x < radius && x >= -radius)
            {
                pixels[index+3] = 0xff;
            }else
            {
                pixels[index+3] = 0;
            }
        }
    }
}

static uint8*
CreateImage2D( int32 width, int32 height, uint8 alpha, uint8 r, uint8 g, uint8 b )
{
    uint32 index = 0;
    uint32 count = width*height*4;
    uint8* pixels = NBRE_NEW uint8[count];
    for(index = 0; index < count; index += 4)
    {
        pixels[index] = r;
        pixels[index+1] = g;
        pixels[index+2] = b;
        pixels[index+3] = alpha;
    }
    return pixels;
}

static void
FillImage2D(uint8* pixels, uint32 /*length*/, int32 width, int32 startX, int32 startY, int32 endX, int32 endY, uint8 alpha, uint8 r, uint8 g, uint8 b )
{
    int32 x = 0;
    int32 y = 0;
    uint32 index = 0;
    for(y = startY; y < endY; ++y)
    {
        index = (startX + width*y)*4;
        for(x = startX; x < endX; ++x, index += 4)
        {
            pixels[index] = r;
            pixels[index+1] = g;
            pixels[index+2] = b;
            pixels[index+3] = alpha;
        }
    }
}

static uint8*
ResizeImage2D(uint8* originPx, uint32 originWidth, uint32 originHeight)
{
    uint8* newPixels = NULL;

    if (originPx == NULL)
    {
        return NULL;
    }

    if((originWidth >> 2) > 1)
    {
        uint32 index = 0;
        newPixels = NBRE_NEW uint8[originWidth * originHeight];
        nsl_memset(newPixels, 0, originWidth * originHeight);

        uint32 lengthX = originWidth << 2;

        for(uint32 i = 0; i < originHeight; i += 2)
        {
            for (uint32 j = 0; j < lengthX; j += 8)
            {
                uint32 acc = originPx[i * lengthX + j] + originPx[i * lengthX + j + 4] + originPx[(i + 1) * lengthX + j] + originPx[(i + 1) * lengthX + j + 4];
                newPixels[index++] = static_cast<uint8>(acc >> 2);

                acc = originPx[i * lengthX + j + 1] + originPx[i * lengthX + j + 4 + 1] + originPx[(i + 1) * lengthX + j + 1] + originPx[(i + 1) * lengthX + j + 4 + 1];
                newPixels[index++] = static_cast<uint8>(acc >> 2);

                acc = originPx[i * lengthX + j + 2] + originPx[i * lengthX + j + 4 + 2] + originPx[(i + 1) * lengthX + j + 2] + originPx[(i + 1) * lengthX + j + 4 + 2];
                newPixels[index++] = static_cast<uint8>(acc >> 2);

                acc = originPx[i * lengthX + j + 3] + originPx[i * lengthX + j + 4 + 3] + originPx[(i + 1) * lengthX + j + 3] + originPx[(i + 1) * lengthX + j + 4 + 3];
                newPixels[index++] = static_cast<uint8>(acc >> 2);
            }
        }

        return newPixels;
    }

    return newPixels;
}

NBRE_TexturePtr
NBGM_BuildTextureUtility::ConstructRoadTexture(NBRE_IRenderPal& renderPal, const NBRE_Color& color, const NBRE_String& name)
{
    NBRE_TexturePtr texture;

    PAL_Error err = PAL_Ok;
    NBRE_Texture* tex = NULL;
    const int32 mipmapcount = 6;
    NBRE_Image** images = NBRE_NEW NBRE_Image*[mipmapcount];
    uint8* pixels = NULL;
    uint8* pixels2 = NULL;
    NBRE_Image* image = NULL;
    uint32 i = 0;
    uint8 r = static_cast<uint8>(color.r*255);
    uint8 g = static_cast<uint8>(color.g*255);
    uint8 b = static_cast<uint8>(color.b*255);

    pixels = CircleImage2D(64, 64, 30, r, g, b);
    pixels2 = HalfImage2D(pixels, 64*64*4, 64, r, g, b);
    if(pixels2 == NULL)
    {
        err = PAL_ErrNoMem;
        goto Error;
    }
    NBRE_DELETE_ARRAY pixels;
    pixels = pixels2;
    pixels2 = NULL;
    HalfRectImage2D(pixels, 32*32*4, 32, 32, 15, r, g, b);
    image = NBRE_NEW NBRE_Image(32, 32, NBRE_PF_R8G8B8A8, pixels);
    images[i++] = image->ConvertToFormat(NBRE_PF_R4G4B4A4);
    NBRE_DELETE image;

    pixels = CircleImage2D(32, 32, 14, r, g, b);
    if(pixels == NULL)
    {
        err = PAL_ErrNoMem;
        goto Error;
    }
    pixels2 = HalfImage2D(pixels, 32*32*4, 32, r, g, b);
    if(pixels2 == NULL)
    {
        err = PAL_ErrNoMem;
        goto Error;
    }
    NBRE_DELETE_ARRAY pixels;
    pixels = pixels2;
    pixels2 = NULL;
    HalfRectImage2D(pixels, 16*16*4, 16, 16, 7, r, g, b);
    image = NBRE_NEW NBRE_Image(16, 16, NBRE_PF_R8G8B8A8, pixels);
    images[i++] = image->ConvertToFormat(NBRE_PF_R4G4B4A4);
    NBRE_DELETE image;

    pixels = CircleImage2D(16, 16, 6, r, g, b);
    if(pixels == NULL)
    {
        err = PAL_ErrNoMem;
        goto Error;
    }
    pixels2 = HalfImage2D(pixels, 16*16*4, 16, r, g, b);
    if(pixels2 == NULL)
    {
        err = PAL_ErrNoMem;
        goto Error;
    }
    NBRE_DELETE_ARRAY pixels;
    pixels = pixels2;
    pixels2 = NULL;
    HalfRectImage2D(pixels, 8*8*4, 8, 8, 3, r, g, b);
    image = NBRE_NEW NBRE_Image(8, 8, NBRE_PF_R8G8B8A8, pixels);
    images[i++] = image->ConvertToFormat(NBRE_PF_R4G4B4A4);
    NBRE_DELETE image;

    pixels = CreateImage2D(4, 4, 0x0, r, g, b);
    if(pixels == NULL)
    {
        err = PAL_ErrNoMem;
        goto Error;
    }
    FillImage2D(pixels, 4*4*4, 4, 1, 1, 3, 4, 0xff, r, g, b);
    image = NBRE_NEW NBRE_Image(4, 4, NBRE_PF_R8G8B8A8, pixels);
    images[i++] = image->ConvertToFormat(NBRE_PF_R4G4B4A4);
    NBRE_DELETE image;

    pixels = CreateImage2D(2, 2, 0x40, r, g, b);
    if(pixels == NULL)
    {
        err = PAL_ErrNoMem;
        goto Error;
    }
    image = NBRE_NEW NBRE_Image(2, 2, NBRE_PF_R8G8B8A8, pixels);
    images[i++] = image->ConvertToFormat(NBRE_PF_R4G4B4A4);
    NBRE_DELETE image;

    pixels = CreateImage2D(1, 1, 0x20, r, g, b);
    if(pixels == NULL)
    {
        err = PAL_ErrNoMem;
        goto Error;
    }
    image = NBRE_NEW NBRE_Image(1, 1, NBRE_PF_R8G8B8A8, pixels);
    images[i++] = image->ConvertToFormat(NBRE_PF_R4G4B4A4);
    NBRE_DELETE image;

    tex = renderPal.CreateTexture(images, 1, mipmapcount, TRUE, NBRE_Texture::TT_2D, name);
    if(tex)
    {
        texture.reset(tex);
    }
    else
    {
        err = PAL_ErrNoInit;
        goto Error;
    }
    return texture;

Error:
    NBRE_DELETE_ARRAY pixels;
    NBRE_DELETE_ARRAY pixels2;

    for (i = 0; i < mipmapcount; ++i)
    {
        NBRE_DELETE images[i];
    }

    NBRE_DELETE_ARRAY images;
    NBRE_DebugLog(PAL_LogSeverityMajor, "NBRE_ConstructRoadTexture, err = 0x%x", err);
    return texture;
}

static void
ComputeNormalVector(const NBRE_Point3f& v2, const NBRE_Point3f& v1, NBRE_Point3f& normal, NBRE_Point3f& dir)
{
    NBRE_Point3f tempDir = v2 - v1;

    normal.x = tempDir.y;
    normal.y = -tempDir.x;
    normal.z = 0;

    dir = tempDir;
}

static void
ConstructRouteVertex(NBGM_FloatBuffer& vertexBuffer, NBGM_FloatBuffer& texcoordBuffer, const NBRE_Point3f& v, float heightOffset, const NBRE_Point3f& normal)
{
    NBRE_Point3f v1left = v - normal;
    NBRE_Point3f v1right = v + normal;

    if(!NBRE_Math::IsZero(heightOffset, 1e-5f))
    {
        vertexBuffer.push_back(v1left.x);
        vertexBuffer.push_back(v1left.y);
        vertexBuffer.push_back(v1left.z);
        vertexBuffer.push_back(v1right.x);
        vertexBuffer.push_back(v1right.y);
        vertexBuffer.push_back(v1right.z);

        texcoordBuffer.push_back(0.0f);
        texcoordBuffer.push_back(0.0f);
        texcoordBuffer.push_back(1.0f);
        texcoordBuffer.push_back(0.0f);
    }

    v1left.z += heightOffset;
    v1right.z += heightOffset;

    vertexBuffer.push_back(v1left.x);
    vertexBuffer.push_back(v1left.y);
    vertexBuffer.push_back(v1left.z);
    vertexBuffer.push_back(v1right.x);
    vertexBuffer.push_back(v1right.y);
    vertexBuffer.push_back(v1right.z);

    texcoordBuffer.push_back(0.0f);
    texcoordBuffer.push_back(0.0f);
    texcoordBuffer.push_back(1.0f);
    texcoordBuffer.push_back(0.0f);
}

static void
ConstructRouteVertexIndex(NBGM_ShortBuffer& indexBuffer, uint16 indexSegment, float heightOffset)
{
    if(!NBRE_Math::IsZero(heightOffset, 1e-5f))
    {
        uint16 step = (indexSegment-1) << 2;

        indexBuffer.push_back(2 + step);
        indexBuffer.push_back(6 + step);
        indexBuffer.push_back(0 + step);
        indexBuffer.push_back(0 + step);
        indexBuffer.push_back(6 + step);
        indexBuffer.push_back(4 + step);

        indexBuffer.push_back(2 + step);
        indexBuffer.push_back(7 + step);
        indexBuffer.push_back(6 + step);
        indexBuffer.push_back(2 + step);
        indexBuffer.push_back(3 + step);
        indexBuffer.push_back(7 + step);

        indexBuffer.push_back(3 + step);
        indexBuffer.push_back(1 + step);
        indexBuffer.push_back(7 + step);
        indexBuffer.push_back(7 + step);
        indexBuffer.push_back(1 + step);
        indexBuffer.push_back(5 + step);
    }
    else
    {
        uint16 step = (indexSegment-1) << 1;

        indexBuffer.push_back(0 + step);
        indexBuffer.push_back(1 + step);
        indexBuffer.push_back(3 + step);
        indexBuffer.push_back(0 + step);
        indexBuffer.push_back(3 + step);
        indexBuffer.push_back(2 + step);
    }
}

PAL_Error
NBGM_BuildModelUtility::NavEcmPolyline2Tristripe(const NBRE_Vector<NBRE_Point3f>& polyline, uint32 assistIndex, uint32 startIndex, uint32 count, float heightOffset,
                                                 float halfRouteWidth, NBGM_FloatBuffer& vertexBuffer, NBGM_FloatBuffer& texcoordBuffer, NBGM_ShortBuffer& indexBuffer)
{
    NBRE_Point3f v1;
    NBRE_Point3f v2;
    NBRE_Point3f normal;
    NBRE_Point3f dir;
    NBRE_Point3f newNormal;
    NBRE_Point3f width;
    uint32 index = 0;
    uint16 indexSegment = 0;

    if(assistIndex == startIndex)
    {
        v1 = polyline.at(0);
        v2 = polyline.at(1);

        ComputeNormalVector(v2, v1, normal, dir);
        normal.Normalise();
        normal *= halfRouteWidth;

        ConstructRouteVertex(vertexBuffer, texcoordBuffer, v1, heightOffset, normal);
        index = 2;
    }
    else
    {
        v1 = polyline.at(assistIndex - 1);
        v2 = polyline.at(assistIndex);

        ComputeNormalVector(v2, v1, normal, dir);
        normal.Normalise();
        normal *= halfRouteWidth;

        ConstructRouteVertex(vertexBuffer, texcoordBuffer, v2, heightOffset, normal);

        v1 = polyline.at(assistIndex + 1);
        float distance = v2.DistanceTo(v1);
        if (distance > halfRouteWidth * 2)
        {
            ComputeNormalVector(v1, v2, normal, dir);
            v2 = v1;
            index = assistIndex + 2;
        }
        else
        {
            v2 = polyline.at(assistIndex + 2);
            ComputeNormalVector(v2, v1, normal, dir);
            index = assistIndex + 3;
        }

        normal.Normalise();
        normal *= halfRouteWidth;
    }

    ++indexSegment;
    --count;
    v1 = v2;

    if (index < polyline.size())
    {
        while(count > 1)
        {
            v2 = polyline.at(index);
            ComputeNormalVector(v2, v1, newNormal, dir);

            if (NBRE_Math::IsZero(newNormal.Length(), 1e-4f))
            {
                --count;
                ++index;
                continue;
            }

            newNormal.Normalise();
            newNormal *= halfRouteWidth;
            width = newNormal + normal;
            width.Normalise();
            width *= halfRouteWidth;
            normal = newNormal;

            ConstructRouteVertex(vertexBuffer, texcoordBuffer, v1, heightOffset, width);
            ConstructRouteVertexIndex(indexBuffer, indexSegment, heightOffset);
            ++indexSegment;

            v1 = v2;
            ++index;
            --count;
        }
    }

    ConstructRouteVertex(vertexBuffer, texcoordBuffer, v1, heightOffset, normal);
    ConstructRouteVertexIndex(indexBuffer, indexSegment, heightOffset);

    return PAL_Ok;
}

NBRE_TexturePtr
NBGM_BuildTextureUtility::ConstructNavEcmRouteTexture(NBRE_IRenderPal& renderPal, const NBRE_Color& color, const NBRE_String& name )
{
    NBRE_TexturePtr texture;

    const int32 mipmapcount = 6;
    NBRE_Image** images = NBRE_NEW NBRE_Image*[mipmapcount];
    NBRE_Image* image = NULL;

    uint32 i = 0;
    uint8 r = static_cast<uint8>(color.r*255);
    uint8 g = static_cast<uint8>(color.g*255);
    uint8 b = static_cast<uint8>(color.b*255);
    uint8 a = static_cast<uint8>(color.a*255);

    uint8* pixels = CreateImage2D(64, 64, 0, r, g, b);
    FillImage2D(pixels, 64, 64, 4, 0, 60, 64, a, r, g, b);
    uint8* pixels2 = ResizeImage2D(pixels, 64, 64);
    NBRE_DELETE_ARRAY pixels;
    image = NBRE_NEW NBRE_Image(32, 32, NBRE_PF_R8G8B8A8, pixels2);
    images[i++] = image->ConvertToFormat(NBRE_PF_R4G4B4A4);
    NBRE_DELETE image;

    pixels = CreateImage2D(32, 32, 0, r, g, b);
    FillImage2D(pixels, 32, 32, 2, 0, 30, 32, a, r, g, b);
    pixels2 = ResizeImage2D(pixels, 32, 32);
    NBRE_DELETE_ARRAY pixels;
    image = NBRE_NEW NBRE_Image(16, 16, NBRE_PF_R8G8B8A8, pixels2);
    images[i++] = image->ConvertToFormat(NBRE_PF_R4G4B4A4);
    NBRE_DELETE image;

    pixels = CreateImage2D(16, 16, 0, r, g, b);
    FillImage2D(pixels, 16, 16, 1, 0, 15, 16, a, r, g, b);
    pixels2 = ResizeImage2D(pixels, 16, 16);
    NBRE_DELETE_ARRAY pixels;
    image = NBRE_NEW NBRE_Image(8, 8, NBRE_PF_R8G8B8A8, pixels2);
    images[i++] = image->ConvertToFormat(NBRE_PF_R4G4B4A4);
    NBRE_DELETE image;

    pixels = CreateImage2D(8, 8, 0, r, g, b);
    FillImage2D(pixels, 8, 8, 1, 0, 7, 8, a, r, g, b);
    image = NBRE_NEW NBRE_Image(4, 4, NBRE_PF_R8G8B8A8, pixels);
    images[i++] = image->ConvertToFormat(NBRE_PF_R4G4B4A4);
    NBRE_DELETE image;

    pixels =CreateImage2D(2, 2, 0x40, r, g, b);
    image = NBRE_NEW NBRE_Image(2, 2, NBRE_PF_R8G8B8A8, pixels);
    images[i++] = image->ConvertToFormat(NBRE_PF_R4G4B4A4);
    NBRE_DELETE image;

    pixels = CreateImage2D(1, 1, 0x20, r, g, b);
    image = NBRE_NEW NBRE_Image(1, 1, NBRE_PF_R8G8B8A8, pixels);
    images[i++] = image->ConvertToFormat(NBRE_PF_R4G4B4A4);
    NBRE_DELETE image;

    NBRE_Texture *tex = renderPal.CreateTexture(images, 1, mipmapcount, TRUE, NBRE_Texture::TT_2D, name);
    if(tex)
    {
        texture.reset(tex);
    }
    return texture;
}

NBRE_TexturePtr
NBGM_BuildTextureUtility::ConstructPathArrowTexture(NBRE_IRenderPal& renderPal, const NBRE_Color& color, float tailWidth, float /*headWidth*/, float /*tailLength*/, float /*headLength*/, const NBRE_String& name )
{
    NBRE_TexturePtr texture;

    uint32 width = 64;
    uint32 height = 64;
    uint32 halfHeight = height >> 1;
    uint32 xPixelsCount = width << 2;

    uint32 pixelCount = width * height * 4;
    uint8* pixels2D = NBRE_NEW uint8[pixelCount];
    if (pixels2D == NULL)
    {
        return NBRE_TexturePtr();
    }
    nsl_memset(pixels2D, 0x00, pixelCount);

    float sqrt2 = 1.414f;

    float headWidthP = tailWidth * 2.0f / 3.0f;
    float tailWidthP = tailWidth;
    float tailLengthP = headWidthP * 15.0f;
    float headLengthP = headWidthP * 5.0f;

    float totalLen = tailLengthP + tailWidthP / 2.0f;
    float tempHeadLen = headLengthP / sqrt2 + headWidthP / 2.0f;

    float headLenPropotion = tempHeadLen / totalLen;
    float tailWidthPropotion = tailWidthP / 2.0f / tempHeadLen;
    float textureHalfHeight = halfHeight * (tempHeadLen * 2.0f / totalLen);

    uint32 textureHeadLen = static_cast<uint32>(width * headLenPropotion);
    uint32 textureTailLen = width - textureHeadLen;
    uint32 halfTextureTailWidth = static_cast<uint32>(textureHalfHeight * tailWidthPropotion);
    uint32 halfTextureHeadWidth = textureHeadLen;

    uint8 red = static_cast<uint8>(color.r * 255);
    uint8 green = static_cast<uint8>(color.g * 255);
    uint8 blue = static_cast<uint8>(color.b * 255);
    uint8 alpha = static_cast<uint8>(color.a * 255);

    uint32 startLine = halfHeight - 1;

    // build tail
    for (uint32 i = startLine; i > startLine - halfTextureTailWidth; --i)
    {
        uint32 xIndex = 0;
        for (uint32 j = 0; j < textureTailLen; ++j)
        {
            uint32 yIndex = xPixelsCount * i;
            if (j != 0)
            {
                pixels2D[yIndex + xIndex++] = red;
                pixels2D[yIndex + xIndex++] = green;
                pixels2D[yIndex + xIndex++] = blue;
                pixels2D[yIndex + xIndex++] = alpha;
            }
            else
            {
                xIndex += 4;
            }
        }
    }

    // build head
    uint32 step = 0;
    for(uint32 i = startLine; i > startLine - halfTextureHeadWidth; --i)
    {
        uint32 xIndex = 0;
        for (uint32 j = 0; j < textureHeadLen - step; ++j)
        {
            uint32 yIndex = xPixelsCount * i + textureTailLen * 4;
            pixels2D[yIndex + xIndex++] = red;
            pixels2D[yIndex + xIndex++] = green;
            pixels2D[yIndex + xIndex++] = blue;
            pixels2D[yIndex + xIndex++] = alpha;
        }
        ++step;
    }

    // remove useless pixels
    step = 0;
    float removeLen = headLengthP / sqrt2 - tailWidthP / 2.0f - 1;
    uint32 removeTextureLen = static_cast<uint32>(halfTextureHeadWidth * (removeLen / tempHeadLen));

    startLine = halfHeight - halfTextureTailWidth - 1;
    for(uint32 i = startLine; i > startLine - removeTextureLen; --i)
    {
        uint32 xIndex = 0;
        for (uint32 j = 0; j < removeTextureLen - step; ++j)
        {
            uint32 yIndex = xPixelsCount * i + textureTailLen * 4;
            pixels2D[yIndex + xIndex++] = 0;
            pixels2D[yIndex + xIndex++] = 0;
            pixels2D[yIndex + xIndex++] = 0;
            pixels2D[yIndex + xIndex++] = 0;
        }
        ++step;
    }

    // build the other half
    for (uint32 i = 0; i < halfHeight; ++i)
    {
        uint8* srcAddr = pixels2D + xPixelsCount * i;
        uint8* desAddr = pixels2D + xPixelsCount * (height - 1 - i);

        nsl_memcpy(desAddr, srcAddr, xPixelsCount);
    }

    uint8* newPixels = HalfImage2D(pixels2D, pixelCount, width, red, green, blue );
    NBRE_DELETE_ARRAY pixels2D;

    NBRE_Image* image = NBRE_NEW NBRE_Image(width >> 1, height >> 1, NBRE_PF_R8G8B8A8, newPixels);

    NBRE_Image** images = NBRE_NEW NBRE_Image*[1];
    images[0] = image->ConvertToFormat(NBRE_PF_R4G4B4A4);

    NBRE_DELETE image;

    NBRE_Texture *tex = renderPal.CreateTexture(images, 1, 1, FALSE, NBRE_Texture::TT_2D, name);
    if(tex)
    {
        texture.reset(tex);
    }
    return texture;
}

NBRE_TexturePtr
NBGM_BuildTextureUtility::ConstructColorTexture(NBRE_IRenderPal& renderPal, const NBRE_Color& c, const NBRE_String& name)
{
    NBRE_Image** images = NBRE_NEW NBRE_Image*[1];

    if (c.a == 1.0f)
    {
        uint8* pixels = NBRE_NEW uint8[3];

        pixels[0] = static_cast<uint8>(c.r*255);
        pixels[1] = static_cast<uint8>(c.g*255);
        pixels[2] = static_cast<uint8>(c.b*255);

        NBRE_Image* img = NBRE_NEW NBRE_Image(1, 1, NBRE_PF_R8G8B8, pixels);
        images[0] = img->ConvertToFormat(NBRE_PF_R5G6B5);
        NBRE_DELETE img;
    }
    else
    {
        uint8* pixels = NBRE_NEW uint8[4];

        pixels[0] = static_cast<uint8>(c.r*255);
        pixels[1] = static_cast<uint8>(c.g*255);
        pixels[2] = static_cast<uint8>(c.b*255);
        pixels[3] = static_cast<uint8>(c.a*255);

        images[0] = NBRE_NEW NBRE_Image(1, 1, NBRE_PF_R8G8B8A8, pixels);
    }

    return NBRE_TexturePtr(renderPal.CreateTexture(images, 1, 1, FALSE, NBRE_Texture::TT_2D, name));
}

NBRE_TexturePtr
NBGM_BuildTextureUtility::ConstructImageTexture(NBRE_IRenderPal& renderPal, PAL_Instance* palInstance, const NBRE_String& name, uint32 /*offset*/, nb_boolean useLowQuality, NBRE_Texture::TextureType texType, nb_boolean isUsingMipmap)
{
    NBRE_ITextureImage* textureImage = NBRE_NEW NBRE_PngTextureImage(*(palInstance), name.c_str(), 0, useLowQuality);
    NBRE_TexturePtr texture;
    NBRE_Texture* pTexture = renderPal.CreateTexture(textureImage, texType, isUsingMipmap, name);
    if(pTexture)
    {
        texture.reset(pTexture);
    }
    else
    {
        textureImage->Release();
    }
    return texture;
}

NBRE_TexturePtr
NBGM_BuildTextureUtility::ConstructCommonTexture(NBRE_IRenderPal& renderPal, const NBRE_String& name, const NBRE_ImageInfo& info, NBRE_Texture::TextureType texType, nb_boolean isUsingMipmap)
{
    NBRE_TexturePtr texture;
    if (info.mHeight == 0 || info.mWidth == 0 || info.mFormat == NBRE_PF_NOTSUPPORT)
    {
        return texture;
    }

    NBRE_Texture *tex = renderPal.CreateTexture(info, texType, isUsingMipmap, name);
    if (tex)
    {
        texture.reset(tex);
    }

    return texture;
}

NBRE_TexturePtr
NBGM_BuildTextureUtility::ConstructCommonTexture(NBRE_IRenderPal& renderPal, const NBRE_String& name, NBRE_ITextureImage* textureImage, NBRE_Texture::TextureType texType, nb_boolean isUsingMipmap)
{
    NBRE_TexturePtr texture;
    if(textureImage == NULL)
    {
        return texture;
    }

    NBRE_Texture* pTexture = renderPal.CreateTexture(textureImage, texType, isUsingMipmap, name);
    if(pTexture)
    {
        texture.reset(pTexture);
    }
    else
    {
        textureImage->Release();
    }
    return texture;
}

NBRE_TexturePtr
NBGM_BuildTextureUtility::ConstructCommonTexture(NBRE_IRenderPal& renderPal, const NBRE_String& name, NBRE_Image** image, NBRE_Texture::TextureType texType, uint8 faceCount, uint8 numMipmaps, nb_boolean isUsingMipmap)
{
    NBRE_TexturePtr texture;
    if (image == NULL || numMipmaps == 0)
    {
        return texture;
    }

    NBRE_Texture* pTexture = renderPal.CreateTexture(image, 1, numMipmaps, isUsingMipmap, texType, name);
    if(pTexture)
    {
        texture.reset(pTexture);
    }
    else
    {
        for(uint32 i = 0; i< static_cast<uint32>(faceCount * numMipmaps); ++i)
        {
            NBRE_DELETE image[i];
        }
    }
    return texture;
}

struct SegInfo
{
    SegInfo(const NBRE_Point2f& p1, const NBRE_Point2f& p2, float d)
        :pt1(p1), pt2(p2), distance(d)
    {
    }

    NBRE_Point2f    pt1;
    NBRE_Point2f    pt2;
    float           distance;
};

typedef NBRE_Vector<SegInfo> SegInfoVector;

static void
CreatePathArrowVertexAndIndex(const SegInfo& info, float t, float width, float length, NBGM_FloatBuffer& vertexBuffer, NBGM_FloatBuffer& texcoordBuffer, NBGM_ShortBuffer& indexBuffer)
{
    float dirX = info.pt2.x - info.pt1.x;
    float dirY = info.pt2.y - info.pt1.y;

    float normalX = -dirY;
    float normalY = dirX;

    float halfWidth = width * 0.5f;
    float halfLength = length * 0.5f;

    float magnitude = static_cast<float>(nsl_sqrt(normalX * normalX + normalY * normalY));
    normalX = normalX / magnitude * halfWidth;
    normalY = normalY / magnitude * halfWidth;

    magnitude = static_cast<float>(nsl_sqrt(dirX * dirX + dirY * dirY));
    dirX = dirX / magnitude * halfLength;
    dirY = dirY / magnitude * halfLength;

    uint16 vertexIndex = static_cast<uint16>(vertexBuffer.size() >> 1);
    NBRE_Point2f interpolatePt = NBRE_LinearInterpolatef::Lerp(info.pt1, info.pt2, t);

    vertexBuffer.push_back(interpolatePt.x - normalX - dirX);
    vertexBuffer.push_back(interpolatePt.y - normalY - dirY);

    texcoordBuffer.push_back(0.0f);
    texcoordBuffer.push_back(0.0f);

    vertexBuffer.push_back(interpolatePt.x + normalX - dirX);
    vertexBuffer.push_back(interpolatePt.y + normalY - dirY);

    texcoordBuffer.push_back(0.0f);
    texcoordBuffer.push_back(1.0f);

    vertexBuffer.push_back(interpolatePt.x - normalX + dirX);
    vertexBuffer.push_back(interpolatePt.y - normalY + dirY);

    texcoordBuffer.push_back(1.0f);
    texcoordBuffer.push_back(0.0f);

    vertexBuffer.push_back(interpolatePt.x + normalX + dirX);
    vertexBuffer.push_back(interpolatePt.y + normalY + dirY);

    texcoordBuffer.push_back(1.0f);
    texcoordBuffer.push_back(1.0f);

    indexBuffer.push_back(vertexIndex);
    indexBuffer.push_back(vertexIndex + 3);
    indexBuffer.push_back(vertexIndex + 1);

    indexBuffer.push_back(vertexIndex);
    indexBuffer.push_back(vertexIndex + 2);
    indexBuffer.push_back(vertexIndex + 3);
}

int32
GetTargetIndex(const SegInfoVector& info, float locateLen, float checkLen, float& distanceToStart)
{
    float distanceSum = 0.0f;
    int32 targetIndex = 0;
    int32 count = static_cast<int32>(info.size());

    for(int32 i = 0; i < count; ++i)
    {
        if (distanceSum + info[i].distance > locateLen)
        {
            targetIndex = i;
            break;
        }
        distanceSum += info[i].distance;
    }

    if (info[targetIndex].distance > checkLen)
    {
        distanceToStart = distanceSum ;
        return targetIndex;
    }

    return static_cast<int32>(INVALID_INDEX);
}

PAL_Error
NBGM_BuildModelUtility::PathArrowPack2Tristripe(const NBRE_Vector<NBRE_Point2f>& polyline, float width, float length, float repeatDistance,
                                                NBGM_FloatBuffer& vertexBuffer, NBGM_FloatBuffer& texcoordBuffer, NBGM_ShortBuffer& indexBuffer)
{
    uint32 count = polyline.size();
    if (count < 2)
    {
        return PAL_ErrBadParam;
    }

    uint32 index = 0;
    SegInfoVector segInfoVector;
    NBRE_Point2f pt1 = polyline.at(index++);
    float totalDistance = 0.0f;

    while(index < count)
    {
        NBRE_Point2f pt2 = polyline.at(index++);
        float distance = pt1.DistanceTo(pt2);
        totalDistance += distance;

        SegInfo segInfo(pt1, pt2, distance);
        segInfoVector.push_back(segInfo);

        pt1 = pt2;
    }

    if (totalDistance < 3.0f * length)
    {
        NBRE_DebugLog(PAL_LogSeverityInfo, "NBGM_BuildModelUtility::PathArrowPack2Tristripe, length too short !");
        return PAL_Ok;
    }

    if (totalDistance < repeatDistance)
    {
        float distanceToStart = 0.0f;
        int32 targetIndex = GetTargetIndex(segInfoVector, totalDistance * 0.5f, length, distanceToStart);
        if (targetIndex != static_cast<int32>(INVALID_INDEX))
        {
            CreatePathArrowVertexAndIndex(segInfoVector[targetIndex], 0.5f, width, length, vertexBuffer, texcoordBuffer, indexBuffer);
        }
    }
    else
    {
        const float PAD_LEN = 2 * length;
        uint32 count = static_cast<uint32>((totalDistance - 2.0f * PAD_LEN) / repeatDistance);
        float remainder = ((totalDistance - 2.0f * PAD_LEN) / repeatDistance - count) * repeatDistance;
        if (remainder > length)
        {
            ++count;
        }

        for(uint32 i = 0; i < count; ++i)
        {
            float location = PAD_LEN + i * repeatDistance;
            float distanceToStart = 0.0f;
            float halfLength = length * 0.5f;

            int32 targetIndex = GetTargetIndex(segInfoVector, location, length, distanceToStart);
            if (targetIndex != static_cast<int32>(INVALID_INDEX))
            {
                float segDistance = segInfoVector[targetIndex].distance;
                float remaiderLen = location - distanceToStart;
                float t = 0.0f;

                if ((remaiderLen > halfLength) && (segDistance - remainder > halfLength))
                {
                    t = remaiderLen / segDistance;
                }
                else if (remaiderLen < halfLength)
                {
                    t = 0.0f;
                }
                else
                {
                    t = (segDistance - remaiderLen) / segDistance;
                }

                CreatePathArrowVertexAndIndex(segInfoVector[targetIndex], t, width, length, vertexBuffer, texcoordBuffer, indexBuffer);
            }
        }
    }

    return PAL_Ok;
}

PAL_Error
NBGM_BuildModelUtility::Track2Tristripe( const NBRE_Vector<NBRE_Point2f>& polyline, float width, NBGM_FloatBuffer& vertexBuffer, NBGM_ShortBuffer& indexBuffer )
{
    uint16 vertexIndex = static_cast<uint16>(vertexBuffer.size()/2);

    if(polyline.empty())
    {
        return PAL_ErrBadParam;
    }

    uint32 numSeg = polyline.size();
    uint32 i = 0;

    NBRE_Point2f pt1 = polyline.at(i++);
    NBRE_Point2f pt2;

    float normalX = 0;
    float normalY = 0;
    while (normalX == 0 && normalY == 0 && i < numSeg)
    {
        pt2 = polyline.at(i++);

        normalX = pt1.y - pt2.y;
        normalY = pt2.x - pt1.x;
    }
    if (normalX == 0 && normalY == 0)
    {
        return PAL_ErrBadParam;
    }

    float normalLen = width/2.f;
    double magnitude = Hypot(normalX, normalY);
    normalX *= static_cast<float>(normalLen/magnitude);
    normalY *= static_cast<float>(normalLen/magnitude);

    vertexBuffer.push_back(pt1.x - normalX);
    vertexBuffer.push_back(pt1.y - normalY);
    vertexBuffer.push_back(pt1.x + normalX);
    vertexBuffer.push_back(pt1.y + normalY);

    float prevNormalX = normalX;
    float prevNormalY = normalY;

    pt1 = pt2;
    for (; i < numSeg; ++i)
    {
        pt2 = polyline.at(i);
        normalX = pt1.y - pt2.y;
        normalY = pt2.x - pt1.x;
        magnitude = Hypot(normalX, normalY);

        if (magnitude == 0)
        {
            continue;
        }

        normalX *= static_cast<float>(normalLen/magnitude);
        normalY *= static_cast<float>(normalLen/magnitude);

        float tempNormalX = (normalX + prevNormalX) ;
        float tempNormalY = (normalY + prevNormalY) ;
        double tempMagnitude = Hypot(tempNormalX, tempNormalY);
        tempNormalX *= static_cast<float>(normalLen/tempMagnitude);
        tempNormalY *= static_cast<float>(normalLen/tempMagnitude);

        vertexBuffer.push_back(pt1.x - tempNormalX);
        vertexBuffer.push_back(pt1.y - tempNormalY);
        vertexBuffer.push_back(pt1.x + tempNormalX);
        vertexBuffer.push_back(pt1.y + tempNormalY);

        prevNormalX = normalX;
        prevNormalY = normalY;

        pt1 = pt2;
    }

    if (magnitude == 0)
    {
        normalX = prevNormalX ;
        normalY = prevNormalY ;
    }

    vertexBuffer.push_back(pt2.x - normalX);
    vertexBuffer.push_back(pt2.y - normalY);
    vertexBuffer.push_back(pt2.x + normalX);
    vertexBuffer.push_back(pt2.y + normalY);

    GeneratNavRouteTriangleStripIndex(indexBuffer, vertexIndex, static_cast<uint16>(vertexBuffer.size()/2));

    return PAL_Ok;
}

typedef NBRE_List<uint16> NBRE_ShortList;

static float
Cross(float x0, float y0, float x1, float y1)
{
    return x0*y1-x1*y0;
}

static float
Clockwise(float x1, float y1, float x2, float y2, float x3, float y3)
{
    float dx1 = x3-x2;
    float dx2 = x1-x2;
    float dy1 = y3-y2;
    float dy2 = y1-y2;
    return dx1*dy2-dx2*dy1 ;
}

static nb_boolean
IsInside(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3)
{
    if((x0<x1) && (x0<x2) && (x0<x3))
    {
        return FALSE;
    }
    if((x0>x1) && (x0>x2) && (x0>x3))
    {
        return FALSE;
    }
    if((y0<y1) && (y0<y2) && (y0<y3))
    {
        return FALSE;
    }
    if((y0>y1) && (y0>y2) && (y0>y3))
    {
        return FALSE;
    }
    float r1 = (x1-x0)*(y2-y0)-(x2-x0)*(y1-y0);
    float r2 = (x2-x0)*(y3-y0)-(x3-x0)*(y2-y0);
    float r3 = (x3-x0)*(y1-y0)-(x1-x0)*(y3-y0);

    if((r1 == 0) || (r2 == 0) || (r3 == 0))
    {
        return FALSE;
    }

    return (nb_boolean)((r1>=0 && r2>=0 && r3>=0)||(r1<=0 && r2<=0 && r3<=0));
}

static nb_boolean
IsEar(NBRE_ShortList& indexs, NBRE_ShortList::iterator iter , nb_boolean isCw, float* v, NBGM_ShortBuffer& indexBuffer )
{
    NBRE_ShortList::iterator i1 = iter;
    if(i1 == indexs.begin())
    {
        i1 = indexs.end();
    }
    --i1;

    NBRE_ShortList::iterator i2 = iter;

    NBRE_ShortList::iterator i3 = ++iter;
    if(i3 == indexs.end())
    {
        i3 = indexs.begin();
    }

    uint16 v1 = *i1;
    uint16 v2 = *i2;
    uint16 v3 = *i3;

    float cw = Clockwise(v[v1*2], v[v1*2+1], v[v2*2], v[v2*2+1], v[v3*2], v[v3*2+1]);
    if(cw == 0)
    {
        return TRUE;
    }

    nb_boolean isClockWise = FALSE;
    if(cw > 0.0f)
    {
        isClockWise = TRUE;
    }

    if(isClockWise == isCw)
    {
        for(iter = indexs.begin(); iter != indexs.end(); ++iter)
        {
            if(iter==i1 || iter==i2 || iter==i3)
            {
                continue;
            }

            uint16 v0 = *iter;
            if(IsInside(v[v0*2], v[v0*2+1], v[v1*2], v[v1*2+1], v[v2*2], v[v2*2+1], v[v3*2], v[v3*2+1]))
            {
                return FALSE;
            }
        }
        indexBuffer.push_back(v1);
        indexBuffer.push_back(v2);
        indexBuffer.push_back(v3);

        return TRUE;
    }
    return FALSE;
}

static PAL_Error
GenerateComplexPolygonIndex(NBGM_ShortBuffer& indexBuffer, float* v, uint16 start, uint16 end)
{
    PAL_Error err = PAL_Ok;
    NBRE_ShortList indexs;

    for(uint16 i = start; i < end; ++i)
    {
        indexs.push_back(i);
    }

    float result = 0.0f;
    for(NBRE_ShortList::iterator iter = indexs.begin(); iter != indexs.end(); ++iter)
    {
        uint16 v1 = *iter;

        NBRE_ShortList::iterator nextIter = iter;
        ++nextIter;
        if(nextIter == indexs.end())
        {
            nextIter = indexs.begin();
        }
        uint16 v2 = *nextIter;

        result += Cross(v[v1 * 2], v[v1 * 2 + 1], v[v2 * 2], v[v2 * 2 + 1]);
    }

    if(result == 0)
    {
        return PAL_ErrWrongFormat;
    }

    nb_boolean cw = FALSE;
    if(result > 0.0f)
    {
        cw = TRUE;
    }

    while(indexs.size() > 2)
    {
        uint32 count = indexs.size();

        for(NBRE_ShortList::iterator iter = indexs.begin(); iter != indexs.end(); ++iter)
        {
            if(IsEar(indexs, iter, cw, v, indexBuffer))
            {
                indexs.erase(iter);
                break;
            }
        }

        //if size isn't change, the left point is same line.
        if(indexs.size() == count)
        {
            break;
        }
    }
    return err;
}

static void
FillVertexAndTextureCoordinate(NBGM_FloatBuffer& vertexBuffer, NBGM_FloatBuffer& texcoordBuffer, NBRE_Point2f* pt, float texX, float texY)
{
    vertexBuffer.push_back(pt->x);
    vertexBuffer.push_back(pt->y);

    texcoordBuffer.push_back(texX);
    texcoordBuffer.push_back(texY);
}

static void
PolygonPack2FloatVertices(NBGM_ResourceContext& /*resourceContext*/, const NBRE_Point3d& tileCenter, NBRE_Array* polygon,
                          NBGM_FloatBuffer& vertexBuffer, NBGM_FloatBuffer& texcoordBuffer, uint32& ptIndex)
{
    uint32 pointCount = NBRE_ArrayGetSize(polygon);

    NBRE_Point2f* pt = static_cast<NBRE_Point2f*>(NBRE_ArrayGetAt(polygon, ptIndex++));
    float x = static_cast<float>(tileCenter.x - static_cast<int32>(tileCenter.x));
    float y = static_cast<float>(tileCenter.y - static_cast<int32>(tileCenter.y));

    FillVertexAndTextureCoordinate(vertexBuffer, texcoordBuffer, pt, pt->x + x, pt->y + y);

    for (; ptIndex < pointCount; ++ptIndex)
    {
        NBRE_Point2f* pt1 = static_cast<NBRE_Point2f*>(NBRE_ArrayGetAt(polygon, ptIndex));

        if(*pt1 == *pt)
        {
            ++ptIndex;
            break;
        }

        FillVertexAndTextureCoordinate(vertexBuffer, texcoordBuffer, pt1, pt1->x + x, pt1->y + y);
    }
}

PAL_Error
NBGM_BuildModelUtility::PolygonPack2FloatArray(NBGM_ResourceContext& resourceContext, const NBRE_Point3d& tileCenter, NBRE_Array* polygon,
                                               NBGM_FloatBuffer& vertexBuffer, NBGM_FloatBuffer& texcoordBuffer, NBGM_ShortBuffer& indexBuffer)
{
    uint32 ptIndex = 0;
    while(ptIndex < NBRE_ArrayGetSize(polygon))
    {
        uint16 vertexIndex = (uint16)vertexBuffer.size()/2;
        PolygonPack2FloatVertices(resourceContext, tileCenter, polygon, vertexBuffer, texcoordBuffer, ptIndex);
        GenerateComplexPolygonIndex(indexBuffer, &vertexBuffer.front(), vertexIndex, (uint16)vertexBuffer.size()/2);
    }
    return PAL_Ok;
}

static bool IsCCW(NBRE_Vector<NBRE_Vector4f>& poly)
{
    double area = 0.0;
    int32 size = (int32)poly.size();
    if (size >= 3)
    {
        double a = 0;
        for (int32 i = 0, j = size -1; i < size; ++i)
        {
            a += ((double)poly[j].x + poly[i].x) * ((double)poly[j].y - poly[i].y);
            j = i;
        }
        area = -a * 0.5;
    }

    return area >= 0.0;
}

static void
ExtractPolygon(NBGM_ResourceContext& /*resourceContext*/, const NBRE_Point3d& tileCenter, NBRE_Array* polygon,
    NBRE_Vector<NBRE_Vector<NBRE_Vector4f> >& outer, NBRE_Vector<NBRE_Vector<NBRE_Vector4f> >& inner, uint32& ptIndex)
{
    uint32 pointCount = NBRE_ArrayGetSize(polygon);
    NBRE_Vector<NBRE_Vector4f> pl;
    NBRE_Point2f* pt = static_cast<NBRE_Point2f*>(NBRE_ArrayGetAt(polygon, ptIndex++));
    float x = static_cast<float>(tileCenter.x - static_cast<int32>(tileCenter.x));
    float y = static_cast<float>(tileCenter.y - static_cast<int32>(tileCenter.y));
    pl.push_back(NBRE_Vector4f(pt->x, pt->y, pt->x + x, pt->y + y));

    for (; ptIndex < pointCount; ++ptIndex)
    {
        NBRE_Point2f* pt1 = static_cast<NBRE_Point2f*>(NBRE_ArrayGetAt(polygon, ptIndex));

        if(*pt1 == *pt)
        {
            ++ptIndex;
            break;
        }

        pl.push_back(NBRE_Vector4f(pt1->x, pt1->y, pt1->x + x, pt1->y + y));
    }

    if (pl.size() >= 3)
    {
        if (IsCCW(pl))
        {
            outer.push_back(pl);
        }
        else
        {
            inner.push_back(pl);
        }
    }
}

bool Triangulation(const NBRE_Vector<NBRE_Vector<NBRE_Vector4f> >& outer, const NBRE_Vector<NBRE_Vector<NBRE_Vector4f> >& inner,
    NBRE_Vector<NBRE_Vector4f>& vertices, NBRE_Vector<uint16>& indices)
{
    bool found = false;
    std::vector<PAL_TriangulationVertexList> ol;
    std::vector<PAL_TriangulationVertexList> il;

    for (uint32 i = 0; i < outer.size(); ++i)
    {
        PAL_TriangulationVertexList vl;
        uint32 n = outer[i].size();
        vl.vertices = new PAL_TriangulationVertex[n];
        vl.vertexCount = n;
        for (uint32 j = 0; j < n; ++j)
        {
            const NBRE_Vector4f& ov = outer[i][j];
            vl.vertices[j].x = ov.x;
            vl.vertices[j].y = ov.y;
            vl.vertices[j].s = ov.z;
            vl.vertices[j].t = ov.w;
        }
        ol.push_back(vl);
    }
    for (uint32 i = 0; i < inner.size(); ++i)
    {
        PAL_TriangulationVertexList vl;
        uint32 n = inner[i].size();
        vl.vertices = new PAL_TriangulationVertex[n];
        vl.vertexCount = n;
        for (uint32 j = 0; j < n; ++j)
        {
            const NBRE_Vector4f& ov = inner[i][j];
            vl.vertices[j].x = ov.x;
            vl.vertices[j].y = ov.y;
            vl.vertices[j].s = ov.z;
            vl.vertices[j].t = ov.w;
        }
        il.push_back(vl);
    }

    PAL_TriangulationVertex* vs = NULL;
    uint32 vsCount = 0;
    uint16* is = NULL;
    uint32 isCount = 0;
    if (PAL_TriangulationPerform(ol.empty() ? NULL : &ol[0], ol.size(), il.empty() ? NULL : &il[0], il.size(), &vs, &vsCount, &is, &isCount))
    {
        for (uint32 i = 0; i < vsCount; ++i)
        {
            vertices.push_back(NBRE_Vector4f(vs[i].x, vs[i].y, vs[i].s, vs[i].t));
        }
        for (uint32 i = 0; i < isCount; ++i)
        {
            indices.push_back(is[i]);
        }

        if (vertices.size() > 0 && indices.size() > 0)
        {
            found = true;
        }
    }
    PAL_TriangulationRelease(vs, is);

    for (uint32 i = 0; i < ol.size(); ++i)
    {
        delete ol[i].vertices;
    }
    for (uint32 i = 0; i < il.size(); ++i)
    {
        delete il[i].vertices;
    }
    return found;
}

PAL_Error
NBGM_BuildModelUtility::HoleyPolygonPack2FloatArray(NBGM_ResourceContext& resourceContext, const NBRE_Point3d& tileCenter, NBRE_Array* polygon,
    NBGM_FloatBuffer& vertexBuffer, NBGM_FloatBuffer& texcoordBuffer, NBGM_ShortBuffer& indexBuffer)
{
    uint32 ptIndex = 0;
    NBRE_Vector<NBRE_Vector<NBRE_Vector4f> > outer;
    NBRE_Vector<NBRE_Vector<NBRE_Vector4f> > inner;
    NBRE_Vector<NBRE_Vector4f> vertices;
    NBRE_Vector<uint16> indices;

    while(ptIndex < NBRE_ArrayGetSize(polygon))
    {
        ExtractPolygon(resourceContext, tileCenter, polygon, outer, inner, ptIndex);
    }

    if (!Triangulation(outer, inner, vertices, indices))
    {
        return PAL_ErrNotFound;
    }

    uint16 baseIndex = (uint16)vertexBuffer.size()/2;
    for (uint32 i = 0; i < vertices.size(); ++i)
    {
        vertexBuffer.push_back(vertices[i].x);
        vertexBuffer.push_back(vertices[i].y);
        texcoordBuffer.push_back(vertices[i].z);
        texcoordBuffer.push_back(vertices[i].w);
    }

    for (uint32 i = 0; i < indices.size(); ++i)
    {
        indexBuffer.push_back(baseIndex + indices[i]);
    }

    return PAL_Ok;
}

static void
PolygonOutlinePack2FloatVertices(NBRE_Array* polygon, NBGM_FloatBuffer& vertexBuffer, NBGM_FloatBuffer& texcoordBuffer, uint32& ptIndex)
{
    uint32 pointCount = NBRE_ArrayGetSize(polygon);

    NBRE_Point2f* pt = static_cast<NBRE_Point2f*>(NBRE_ArrayGetAt(polygon, ptIndex++));

    FillVertexAndTextureCoordinate(vertexBuffer, texcoordBuffer, pt, 0.0f, 0.0f);

    for (; ptIndex < pointCount; ++ptIndex)
    {
        NBRE_Point2f* pt1 = static_cast<NBRE_Point2f*>(NBRE_ArrayGetAt(polygon, ptIndex));
        if(*pt1 == *pt)
        {
            ++ptIndex;
            break;
        }

        FillVertexAndTextureCoordinate(vertexBuffer, texcoordBuffer, pt1, 0.0f, 0.0f);
    }

    FillVertexAndTextureCoordinate(vertexBuffer, texcoordBuffer, pt, 0.0f, 0.0f);
}

static PAL_Error
GeneratePolygonOutlineIndex(NBGM_ShortBuffer& indexBuffer, uint16 start, uint16 end)
{
    if (end - start < 4)
    {
        return PAL_ErrBadParam;
    }

    for(uint16 i = start; i < end - 1; ++i)
    {
        indexBuffer.push_back(i);
        indexBuffer.push_back(i + 1);
    }

    return PAL_Ok;
}

PAL_Error
NBGM_BuildModelUtility::PolygonWireFramePack2FloatArray(const NBRE_Point3d& /*tileCenter*/, NBRE_Array* polygon,
                                                        NBGM_FloatBuffer& vertexBuffer, NBGM_FloatBuffer& texcoordBuffer, NBGM_ShortBuffer& indexBuffer)
{
    uint32 pointCount = NBRE_ArrayGetSize(polygon);
    if(pointCount < 3)
    {
        return PAL_ErrWrongFormat;
    }

    uint32 ptIndex = 0;
    while(ptIndex < pointCount)
    {
        uint16 vertexIndex = static_cast<uint16>(vertexBuffer.size()/2);
        PolygonOutlinePack2FloatVertices(polygon, vertexBuffer, texcoordBuffer, ptIndex);
        GeneratePolygonOutlineIndex(indexBuffer, vertexIndex, static_cast<uint16>(vertexBuffer.size()/2));
    }

    return PAL_Ok;
}

static PAL_Error
GenerateSimplePolygonIndex(NBGM_ShortBuffer& indexBuffer, uint16 start, uint16 end)
{
    if (end - start < 3)
    {
        return PAL_ErrBadParam;
    }

    for(uint16 i = start; i < end - 2; ++i)
    {
        indexBuffer.push_back(start);
        indexBuffer.push_back(i + 1);
        indexBuffer.push_back(i + 2);
    }

    return PAL_Ok;
}

PAL_Error
NBGM_BuildModelUtility::ConvexPolygonPack2FloatArray(NBGM_ResourceContext& resourceContext, const NBRE_Point3d& tileCenter, NBRE_Array* polygon, NBGM_FloatBuffer& vertexBuffer,
                                                     NBGM_FloatBuffer& texcoordBuffer, NBGM_ShortBuffer& indexBuffer)
{
    uint32 ptIndex = 0;
    while(ptIndex < NBRE_ArrayGetSize(polygon))
    {
        uint16 vertexIndex = static_cast<uint16>(vertexBuffer.size()/2);
        PolygonPack2FloatVertices(resourceContext, tileCenter, polygon, vertexBuffer, texcoordBuffer, ptIndex);
        GenerateSimplePolygonIndex(indexBuffer, vertexIndex, static_cast<uint16>(vertexBuffer.size()/2));
    }
    return PAL_Ok;
}

PAL_Error
NBGM_BuildModelUtility::BuildPolylineCircleCap(float radius, const NBRE_Point2f& lastPt, const NBRE_Point2f& texCoord,
                                               NBGM_FloatBuffer& vertexBuffer, NBGM_FloatBuffer& normalBuffer,
                                               NBGM_FloatBuffer& texcoordBuffer1, NBGM_FloatBuffer& texcoordBuffer2,
                                               NBGM_ShortBuffer& indexBuffer)
{
    if(radius < 0.0f)
    {
        return PAL_ErrBadParam;
    }

    NBRE_Vector2f dir(0.0f, 0.0f);
    NBRE_Point2f center = lastPt;
    const uint32 STEP = 10;
    uint16 vertexStartIndex = static_cast<uint16>(vertexBuffer.size() / 2);

    FillVertexNormalMultiTexCoord2D(vertexBuffer, normalBuffer, texcoordBuffer1, texcoordBuffer2,
                                    &center, dir.x, dir.y, 0.5f, 0.5f, texCoord.x, texCoord.y);

    for(uint32 angle = 0; angle <= 360; angle += STEP)
    {
        dir.x = static_cast<float>(radius * nsl_cos(NBRE_Math::DegToRad(static_cast<float>(angle))));
        dir.y = static_cast<float>(radius * nsl_sin(NBRE_Math::DegToRad(static_cast<float>(angle))));

        FillVertexNormalMultiTexCoord2D(vertexBuffer, normalBuffer, texcoordBuffer1, texcoordBuffer2,
                                        &center, dir.x, dir.y, 0.5f, 0.5f, texCoord.x, texCoord.y);
    }

    uint16 vertexEndIndex = static_cast<uint16>(vertexBuffer.size() / 2);
    uint16 centerIndex = vertexStartIndex;
    uint16 index = vertexStartIndex + 1;
    for(; index < vertexEndIndex - 1; ++index)
    {
        indexBuffer.push_back(centerIndex);
        indexBuffer.push_back(index);
        indexBuffer.push_back(index + 1);
    }

    return PAL_Ok;
}

PAL_Error
NBGM_BuildModelUtility::BuildPolylineArrowCap(float width, float length, const NBRE_Point2f& lastPt,
                                              const NBRE_Point2f& prevLastPt, const NBRE_Point2f& texCoord,
                                              NBGM_FloatBuffer& vertexBuffer, NBGM_FloatBuffer& normalBuffer,
                                              NBGM_FloatBuffer& texcoordBuffer1, NBGM_FloatBuffer& texcoordBuffer2,
                                              NBGM_ShortBuffer& indexBuffer)
{
    if(width <= 0.0f || length <= 0.0f)
    {
        return PAL_ErrBadParam;
    }

    NBRE_Vector2f dir = lastPt - prevLastPt;
    dir.Normalise();
    NBRE_Vector2f normal = dir.Perpendicular();

    NBRE_Vector2f w = normal * width * 0.5f;
    NBRE_Vector2f h = dir * length;


    uint16 vertexIndex = static_cast<uint16>(vertexBuffer.size() / 2);
    FillVertexNormalMultiTexCoord2D(vertexBuffer, normalBuffer, texcoordBuffer1, texcoordBuffer2,
        &lastPt, -w.x, -w.y, 0.5f, 0.5f, texCoord.x, texCoord.y);

    FillVertexNormalMultiTexCoord2D(vertexBuffer, normalBuffer, texcoordBuffer1, texcoordBuffer2,
        &lastPt, w.x, w.y, 0.5f, 0.5f, texCoord.x, texCoord.y);

    FillVertexNormalMultiTexCoord2D(vertexBuffer, normalBuffer, texcoordBuffer1, texcoordBuffer2,
        &lastPt, h.x, h.y, 0.5f, 0.5f, texCoord.x, texCoord.y);

    indexBuffer.push_back(vertexIndex);
    indexBuffer.push_back(vertexIndex + 1);
    indexBuffer.push_back(vertexIndex + 2);

    return PAL_Ok;
}

PAL_Error
NBGM_BuildModelUtility::PolylinePack2TristripeWithFlatEndpoint(NBRE_Array* polyline, float width, float texX, float texY, NBGM_FloatBuffer& vertexBuffer, NBGM_FloatBuffer& normalBuffer,
                                                               NBGM_FloatBuffer& texcoordBuffer1, NBGM_FloatBuffer& texcoordBuffer2, NBGM_ShortBuffer& indexBuffer)
{
    if(polyline == NULL || width < 0.0f)
    {
        return PAL_ErrBadParam;
    }

    float normalLen = width/2.f;
    uint16 vertexIndex = static_cast<int16>(vertexBuffer.size()/2);
    uint16 start = vertexIndex;
    uint32 numSeg = NBRE_ArrayGetSize(polyline);
    uint32 i = 0;
    NBRE_Point2f* pt1 = static_cast<NBRE_Point2f*>(NBRE_ArrayGetAt(polyline, i++));
    NBRE_Point2f* pt2 = NULL;

    float normalX = 0;
    float normalY = 0;
    while (normalX == 0 && normalY == 0 && i < numSeg)
    {
        pt2 = static_cast<NBRE_Point2f*>(NBRE_ArrayGetAt(polyline, i++));

        normalX = pt1->y - pt2->y;
        normalY = pt2->x - pt1->x;
    }
    if (normalX == 0 && normalY == 0)
    {
        return PAL_ErrBadParam;
    }

    double magnitude = Hypot(normalX, normalY);
    normalX *= static_cast<float>(normalLen/magnitude);
    normalY *= static_cast<float>(normalLen/magnitude);

    FillVertexNormalMultiTexCoord2D(vertexBuffer, normalBuffer, texcoordBuffer1, texcoordBuffer2,
                                    pt1,
                                    -normalX, -normalY,
                                    ROAD_X_COORD, ROAD_Y_COORD,
                                    texX, texY);

    FillVertexNormalMultiTexCoord2D(vertexBuffer, normalBuffer, texcoordBuffer1, texcoordBuffer2,
                                    pt1,
                                    normalX, normalY,
                                    ROAD_X_COORD + ROAD_WIDTH, ROAD_Y_COORD,
                                    texX, texY);
    vertexIndex += 2;

    float prevNormalX = normalX;
    float prevNormalY = normalY;

    pt1 = pt2;
    for (; i < numSeg; ++i)
    {
        pt2 = static_cast<NBRE_Point2f*>(NBRE_ArrayGetAt(polyline, i));
        normalX = pt1->y - pt2->y;
        normalY = pt2->x - pt1->x;
        magnitude = Hypot(normalX, normalY);

        if (magnitude == 0)
        {
            continue;
        }

        normalX *= static_cast<float>(normalLen/magnitude);
        normalY *= static_cast<float>(normalLen/magnitude);

        double cos2Theta = (normalX*prevNormalX + normalY*prevNormalY)/(normalLen*normalLen);
        float tempNormalX = 0.0f;
        float tempNormalY = 0.0f;
        double tempMagnitude = 0.0;

        if(cos2Theta < SQRT_HALF3)
        {
            FillVertexNormalMultiTexCoord2D(vertexBuffer, normalBuffer, texcoordBuffer1, texcoordBuffer2,
                                            pt1,
                                            -prevNormalX, -prevNormalY,
                                            TURN_X_COORD - TURN_WIDTH, TURN_Y_COORD,
                                            texX, texY);

            FillVertexNormalMultiTexCoord2D(vertexBuffer, normalBuffer, texcoordBuffer1, texcoordBuffer2,
                                            pt1,
                                            prevNormalX, prevNormalY,
                                            TURN_X_COORD + TURN_WIDTH, TURN_Y_COORD,
                                            texX, texY);

            FillVertexNormalMultiTexCoord2D(vertexBuffer, normalBuffer, texcoordBuffer1, texcoordBuffer2,
                                            pt1,
                                            prevNormalY - prevNormalX, -prevNormalX - prevNormalY,
                                            TURN_X_COORD - TURN_WIDTH, TURN_Y_COORD - TURN_WIDTH,
                                            texX, texY);

            FillVertexNormalMultiTexCoord2D(vertexBuffer, normalBuffer, texcoordBuffer1, texcoordBuffer2,
                                            pt1,
                                            prevNormalY + prevNormalX, -prevNormalX + prevNormalY,
                                            TURN_X_COORD + TURN_WIDTH, TURN_Y_COORD - TURN_WIDTH,
                                            texX, texY);
            vertexIndex += 4;

            GeneratTriangleStripIndex(indexBuffer, start, vertexIndex);
            start = vertexIndex;

            tempNormalX = normalX ;
            tempNormalY = normalY ;
        }
        else
        {
            double cosTheta = nsl_sqrt((cos2Theta+1)/2);
            tempNormalX = (normalX + prevNormalX) ;
            tempNormalY = (normalY + prevNormalY) ;
            tempMagnitude = Hypot(tempNormalX, tempNormalY);
            tempNormalX *= static_cast<float>(normalLen/cosTheta/tempMagnitude);
            tempNormalY *= static_cast<float>(normalLen/cosTheta/tempMagnitude);
        }
        FillVertexNormalMultiTexCoord2D(vertexBuffer, normalBuffer, texcoordBuffer1, texcoordBuffer2,
                                        pt1,
                                        -tempNormalX, -tempNormalY,
                                        ROAD_X_COORD, ROAD_Y_COORD,
                                        texX, texY);

        FillVertexNormalMultiTexCoord2D(vertexBuffer, normalBuffer, texcoordBuffer1, texcoordBuffer2,
                                        pt1,
                                        tempNormalX, tempNormalY,
                                        ROAD_X_COORD + ROAD_WIDTH, ROAD_Y_COORD,
                                        texX, texY);
        vertexIndex += 2;

        prevNormalX = normalX;
        prevNormalY = normalY;

        pt1 = pt2;
    }

    if (magnitude == 0)
    {
        normalX = prevNormalX ;
        normalY = prevNormalY ;
    }

    FillVertexNormalMultiTexCoord2D(vertexBuffer, normalBuffer, texcoordBuffer1, texcoordBuffer2,
                                    pt1,
                                    -normalX, -normalY,
                                    ROAD_X_COORD, ROAD_Y_COORD,
                                    texX, texY);

    FillVertexNormalMultiTexCoord2D(vertexBuffer, normalBuffer, texcoordBuffer1, texcoordBuffer2,
                                    pt1,
                                    normalX, normalY,
                                    ROAD_X_COORD + ROAD_WIDTH, ROAD_Y_COORD,
                                    texX, texY);

    vertexIndex += 2;

    GeneratTriangleStripIndex(indexBuffer, start, vertexIndex);

    return PAL_Ok;
}

NBRE_ShaderPtr
NBGM_BuildShaderUtility::CreatePolylineShader(NBRE_IRenderPal& renderPal, NBRE_Image* image)
{
    if(image == NULL)
    {
        return NBRE_ShaderPtr(NBRE_NEW NBRE_Shader());
    }

    NBRE_Image** images = NBRE_NEW NBRE_Image*[1];
    if(images == NULL)
    {
        return NBRE_ShaderPtr(NBRE_NEW NBRE_Shader());
    }
    images[0] = image;

    NBRE_TextureUnit texUnit0;
    texUnit0.SetTexture(NBRE_TexturePtr(renderPal.CreateTexture(images, 1, 1, FALSE, NBRE_Texture::TT_2D, VECTOR_POLYLINE_COLOR_TEXTURE_NAME)));
    texUnit0.SetState(NBRE_TextureState(NBRE_TFT_POINT, NBRE_TFT_POINT, NBRE_TAM_CLAMP, NBRE_TAM_CLAMP, FALSE));
    texUnit0.SetTextureCoordSet(0);

    NBRE_TextureUnit texUnit1;
    texUnit1.SetTexture(NBGM_BuildTextureUtility::ConstructRoadTexture(renderPal, NBRE_Color(1, 1, 1, 1), VECTOR_POLYLINE_SHAPE_TEXTURE_NAME));
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

static void
CreateBitMapPatternLineVertexAndIndex(const SegInfo& info,
                                      float t,
                                      float width,
                                      float length,
                                      NBGM_FloatBuffer& vertexBuffer,
                                      NBGM_FloatBuffer& normalBuffer,
                                      NBGM_FloatBuffer& texcoordBuffer,
                                      NBGM_ShortBuffer& indexBuffer)
{
    float dirX = info.pt2.x - info.pt1.x;
    float dirY = info.pt2.y - info.pt1.y;

    float normalX = -dirY;
    float normalY = dirX;

    float halfWidth = width * 0.5f;
    float halfLength = length * 0.5f;

    float magnitude = static_cast<float>(nsl_sqrt(normalX * normalX + normalY * normalY));
    normalX = normalX / magnitude * halfWidth;
    normalY = normalY / magnitude * halfWidth;

    magnitude = static_cast<float>(nsl_sqrt(dirX * dirX + dirY * dirY));
    dirX = dirX / magnitude * halfLength;
    dirY = dirY / magnitude * halfLength;

    uint16 vertexIndex = static_cast<uint16>(vertexBuffer.size() >> 1);
    NBRE_Point2f interpolatePt = NBRE_LinearInterpolatef::Lerp(info.pt1, info.pt2, t);

    FillVertexNormalTexCoord2D(vertexBuffer, normalBuffer, texcoordBuffer,
                               &interpolatePt, -normalX - dirX, -normalY - dirY, 0.0f, 0.0f);

    FillVertexNormalTexCoord2D(vertexBuffer, normalBuffer, texcoordBuffer,
                               &interpolatePt, normalX - dirX, normalY - dirY, 0.0f, 1.0f);

    FillVertexNormalTexCoord2D(vertexBuffer, normalBuffer, texcoordBuffer,
                               &interpolatePt, -normalX + dirX, -normalY + dirY, 1.0f, 0.0f);

    FillVertexNormalTexCoord2D(vertexBuffer, normalBuffer, texcoordBuffer,
                               &interpolatePt, normalX + dirX, normalY + dirY, 1.0f, 1.0f);

    GeneratTriangleStripIndex(indexBuffer, vertexIndex, vertexIndex + 4);
}

PAL_Error
NBGM_BuildModelUtility::BitmapPatternPolyline2Tristripe(NBGM_ResourceContext& rc,
                                                        NBRE_Array* polyline,
                                                        float width,
                                                        float length,
                                                        float repeatDistance,
                                                        float texRatio,
                                                        nb_boolean distanceIsInvalid,
                                                        NBGM_FloatBuffer& vertexBuffer,
                                                        NBGM_FloatBuffer& normalBuffer,
                                                        NBGM_FloatBuffer& texcoordBuffer,
                                                        NBGM_ShortBuffer& indexBuffer,
                                                        double& roadSegTotalLen)
{
    if(distanceIsInvalid)
    {
        return NBGM_BuildModelUtility::PatternPolylinePack2Tristripe(rc, polyline, width, texRatio, vertexBuffer, normalBuffer, texcoordBuffer, indexBuffer, roadSegTotalLen);
    }

    uint32 posCount = NBRE_ArrayGetSize(polyline);
    if (posCount < 2)
    {
        return PAL_ErrBadParam;
    }

    uint32 index = 0;
    SegInfoVector segInfoVector;
    segInfoVector.reserve(posCount - 1);

    NBRE_Point2f pt1 = *static_cast<NBRE_Point2f*>(NBRE_ArrayGetAt(polyline, index++));
    float totalDistance = 0.0f;

    while(index < posCount)
    {
        NBRE_Point2f pt2 = *static_cast<NBRE_Point2f*>(NBRE_ArrayGetAt(polyline, index++));
        float distance = pt1.DistanceTo(pt2);
        totalDistance += distance;

        SegInfo segInfo(pt1, pt2, distance);
        segInfoVector.push_back(segInfo);

        pt1 = pt2;
    }

    roadSegTotalLen += totalDistance;
    float halfLen = length * 0.5f;
    uint32 count = static_cast<uint32>(totalDistance / repeatDistance) + 1;
    for(uint32 i = 0; i < count; ++i)
    {
        float location = i * repeatDistance + halfLen;
        float distanceToStart = 0.0f;

        int32 targetIndex = GetTargetIndex(segInfoVector, location, length, distanceToStart);
        if (targetIndex != static_cast<int32>(INVALID_INDEX))
        {
            float segDistance = segInfoVector[targetIndex].distance;
            float remaiderLen = location - distanceToStart;
            float t = remaiderLen / segDistance;
            CreateBitMapPatternLineVertexAndIndex(segInfoVector[targetIndex], t, width, length, vertexBuffer, normalBuffer, texcoordBuffer, indexBuffer);
        }
    }

    return PAL_Ok;
}

PAL_Error
NBGM_BuildModelUtility::PolylinePack2TristripeWithFlatStartCapRoundEndCap(NBRE_Array* polyline,
                                                                          float width,
                                                                          float texX,
                                                                          float texY,
                                                                          NBGM_FloatBuffer& vertexBuffer,
                                                                          NBGM_FloatBuffer& normalBuffer,
                                                                          NBGM_FloatBuffer& texcoordBuffer1,
                                                                          NBGM_FloatBuffer& texcoordBuffer2,
                                                                          NBGM_ShortBuffer& indexBuffer)
{
    if(polyline == NULL || width < 0.0f)
    {
        return PAL_ErrBadParam;
    }

    float normalLen = width/2.f;
    uint16 vertexIndex = static_cast<int16>(vertexBuffer.size()/2);
    uint16 start = vertexIndex;
    uint32 numSeg = NBRE_ArrayGetSize(polyline);
    uint32 i = 0;
    NBRE_Point2f* pt1 = static_cast<NBRE_Point2f*>(NBRE_ArrayGetAt(polyline, i++));
    NBRE_Point2f* pt2 = NULL;

    float normalX = 0;
    float normalY = 0;
    while (normalX == 0 && normalY == 0 && i < numSeg)
    {
        pt2 = static_cast<NBRE_Point2f*>(NBRE_ArrayGetAt(polyline, i++));

        normalX = pt1->y - pt2->y;
        normalY = pt2->x - pt1->x;
    }
    if (normalX == 0 && normalY == 0)
    {
        return PAL_ErrBadParam;
    }

    double magnitude = Hypot(normalX, normalY);
    normalX *= static_cast<float>(normalLen/magnitude);
    normalY *= static_cast<float>(normalLen/magnitude);

    FillVertexNormalMultiTexCoord2D(vertexBuffer, normalBuffer, texcoordBuffer1, texcoordBuffer2,
                                    pt1,
                                    -normalX, -normalY,
                                    ROAD_X_COORD, ROAD_Y_COORD,
                                    texX, texY);

    FillVertexNormalMultiTexCoord2D(vertexBuffer, normalBuffer, texcoordBuffer1, texcoordBuffer2,
                                    pt1,
                                    normalX, normalY,
                                    ROAD_X_COORD + ROAD_WIDTH, ROAD_Y_COORD,
                                    texX, texY);
    vertexIndex += 2;

    float prevNormalX = normalX;
    float prevNormalY = normalY;

    pt1 = pt2;
    for (; i < numSeg; ++i)
    {
        pt2 = static_cast<NBRE_Point2f*>(NBRE_ArrayGetAt(polyline, i));
        normalX = pt1->y - pt2->y;
        normalY = pt2->x - pt1->x;
        magnitude = Hypot(normalX, normalY);

        if (magnitude == 0)
        {
            continue;
        }

        normalX *= static_cast<float>(normalLen/magnitude);
        normalY *= static_cast<float>(normalLen/magnitude);

        double cos2Theta = (normalX*prevNormalX + normalY*prevNormalY)/(normalLen*normalLen);
        float tempNormalX = 0.0f;
        float tempNormalY = 0.0f;
        double tempMagnitude = 0.0;

        if(cos2Theta < SQRT_HALF3)
        {
            FillVertexNormalMultiTexCoord2D(vertexBuffer, normalBuffer, texcoordBuffer1, texcoordBuffer2,
                                            pt1,
                                            -prevNormalX, -prevNormalY,
                                            TURN_X_COORD - TURN_WIDTH, TURN_Y_COORD,
                                            texX, texY);

            FillVertexNormalMultiTexCoord2D(vertexBuffer, normalBuffer, texcoordBuffer1, texcoordBuffer2,
                                            pt1,
                                            prevNormalX, prevNormalY,
                                            TURN_X_COORD + TURN_WIDTH, TURN_Y_COORD,
                                            texX, texY);

            FillVertexNormalMultiTexCoord2D(vertexBuffer, normalBuffer, texcoordBuffer1, texcoordBuffer2,
                                            pt1,
                                            prevNormalY - prevNormalX, -prevNormalX - prevNormalY,
                                            TURN_X_COORD - TURN_WIDTH, TURN_Y_COORD - TURN_WIDTH,
                                            texX, texY);

            FillVertexNormalMultiTexCoord2D(vertexBuffer, normalBuffer, texcoordBuffer1, texcoordBuffer2,
                                            pt1,
                                            prevNormalY + prevNormalX, -prevNormalX + prevNormalY,
                                            TURN_X_COORD + TURN_WIDTH, TURN_Y_COORD - TURN_WIDTH,
                                            texX, texY);
            vertexIndex += 4;

            GeneratTriangleStripIndex(indexBuffer, start, vertexIndex);
            start = vertexIndex;

            tempNormalX = normalX ;
            tempNormalY = normalY ;
        }
        else
        {
            double cosTheta = nsl_sqrt((cos2Theta+1)/2);
            tempNormalX = (normalX + prevNormalX) ;
            tempNormalY = (normalY + prevNormalY) ;
            tempMagnitude = Hypot(tempNormalX, tempNormalY);
            tempNormalX *= static_cast<float>(normalLen/cosTheta/tempMagnitude);
            tempNormalY *= static_cast<float>(normalLen/cosTheta/tempMagnitude);
        }
        FillVertexNormalMultiTexCoord2D(vertexBuffer, normalBuffer, texcoordBuffer1, texcoordBuffer2,
                                        pt1,
                                        -tempNormalX, -tempNormalY,
                                        ROAD_X_COORD, ROAD_Y_COORD,
                                        texX, texY);

        FillVertexNormalMultiTexCoord2D(vertexBuffer, normalBuffer, texcoordBuffer1, texcoordBuffer2,
                                        pt1,
                                        tempNormalX, tempNormalY,
                                        ROAD_X_COORD + ROAD_WIDTH, ROAD_Y_COORD,
                                        texX, texY);
        vertexIndex += 2;

        prevNormalX = normalX;
        prevNormalY = normalY;

        pt1 = pt2;
    }

    if (magnitude == 0)
    {
        normalX = prevNormalX ;
        normalY = prevNormalY ;
    }

    FillVertexNormalMultiTexCoord2D(vertexBuffer, normalBuffer, texcoordBuffer1, texcoordBuffer2,
                                    pt2,
                                    -normalX, -normalY,
                                    TURN_X_COORD - TURN_WIDTH, TURN_Y_COORD,
                                    texX, texY);

    FillVertexNormalMultiTexCoord2D(vertexBuffer, normalBuffer, texcoordBuffer1, texcoordBuffer2,
                                    pt2,
                                    normalX, normalY,
                                    TURN_X_COORD + TURN_WIDTH, TURN_Y_COORD,
                                    texX, texY);

    FillVertexNormalMultiTexCoord2D(vertexBuffer, normalBuffer, texcoordBuffer1, texcoordBuffer2,
                                    pt2,
                                    normalY - normalX, -normalX - normalY,
                                    TURN_X_COORD - TURN_WIDTH, TURN_Y_COORD - TURN_WIDTH,
                                    texX, texY);

    FillVertexNormalMultiTexCoord2D(vertexBuffer, normalBuffer, texcoordBuffer1, texcoordBuffer2,
                                    pt2,
                                    normalY + normalX, -normalX + normalY,
                                    TURN_X_COORD + TURN_WIDTH, TURN_Y_COORD - TURN_WIDTH,
                                    texX, texY);
    vertexIndex += 4;

    GeneratTriangleStripIndex(indexBuffer, start, vertexIndex);

    return PAL_Ok;
}

PAL_Error
NBGM_BuildModelUtility::PolylinePack2TristripeWithRoundStartCapFlatEndCap(NBRE_Array* polyline,
                                                                          float width,
                                                                          float texX,
                                                                          float texY,
                                                                          NBGM_FloatBuffer& vertexBuffer,
                                                                          NBGM_FloatBuffer& normalBuffer,
                                                                          NBGM_FloatBuffer& texcoordBuffer1,
                                                                          NBGM_FloatBuffer& texcoordBuffer2,
                                                                          NBGM_ShortBuffer& indexBuffer)
{
    if(polyline == NULL || width < 0.0f)
    {
        return PAL_ErrBadParam;
    }

    float normalLen = width/2.f;
    uint16 vertexIndex = static_cast<int16>(vertexBuffer.size()/2);
    uint16 start = vertexIndex;
    uint32 numSeg = NBRE_ArrayGetSize(polyline);
    uint32 i = 0;
    NBRE_Point2f* pt1 = static_cast<NBRE_Point2f*>(NBRE_ArrayGetAt(polyline, i++));
    NBRE_Point2f* pt2 = NULL;

    float normalX = 0;
    float normalY = 0;
    while (normalX == 0 && normalY == 0 && i < numSeg)
    {
        pt2 = static_cast<NBRE_Point2f*>(NBRE_ArrayGetAt(polyline, i++));

        normalX = pt1->y - pt2->y;
        normalY = pt2->x - pt1->x;
    }
    if (normalX == 0 && normalY == 0)
    {
        return PAL_ErrBadParam;
    }

    double magnitude = Hypot(normalX, normalY);
    normalX *= static_cast<float>(normalLen/magnitude);
    normalY *= static_cast<float>(normalLen/magnitude);

    FillVertexNormalMultiTexCoord2D(vertexBuffer, normalBuffer, texcoordBuffer1, texcoordBuffer2,
                                    pt1,
                                    -normalY - normalX, normalX - normalY,
                                    TURN_X_COORD - TURN_WIDTH, TURN_Y_COORD - TURN_WIDTH,
                                    texX, texY);

    FillVertexNormalMultiTexCoord2D(vertexBuffer, normalBuffer, texcoordBuffer1, texcoordBuffer2,
                                    pt1,
                                    -normalY + normalX, normalX + normalY,
                                    TURN_X_COORD + TURN_WIDTH, TURN_Y_COORD - TURN_WIDTH,
                                    texX, texY);

    FillVertexNormalMultiTexCoord2D(vertexBuffer, normalBuffer, texcoordBuffer1, texcoordBuffer2,
                                    pt1,
                                    -normalX, -normalY,
                                    TURN_X_COORD - TURN_WIDTH, TURN_Y_COORD,
                                    texX, texY);

    FillVertexNormalMultiTexCoord2D(vertexBuffer, normalBuffer, texcoordBuffer1, texcoordBuffer2,
                                    pt1,
                                    normalX, normalY,
                                    TURN_X_COORD + TURN_WIDTH, TURN_Y_COORD,
                                    texX, texY);
    vertexIndex += 4;

    GeneratTriangleStripIndex(indexBuffer, start, vertexIndex);
    start = vertexIndex;

    FillVertexNormalMultiTexCoord2D(vertexBuffer, normalBuffer, texcoordBuffer1, texcoordBuffer2,
                                    pt1,
                                    -normalX, -normalY,
                                    ROAD_X_COORD, ROAD_Y_COORD,
                                    texX, texY);

    FillVertexNormalMultiTexCoord2D(vertexBuffer, normalBuffer, texcoordBuffer1, texcoordBuffer2,
                                    pt1,
                                    normalX, normalY,
                                    ROAD_X_COORD + ROAD_WIDTH, ROAD_Y_COORD,
                                    texX, texY);
    vertexIndex += 2;

    float prevNormalX = normalX;
    float prevNormalY = normalY;

    pt1 = pt2;
    for (; i < numSeg; ++i)
    {
        pt2 = static_cast<NBRE_Point2f*>(NBRE_ArrayGetAt(polyline, i));
        normalX = pt1->y - pt2->y;
        normalY = pt2->x - pt1->x;
        magnitude = Hypot(normalX, normalY);

        if (magnitude == 0)
        {
            continue;
        }

        normalX *= static_cast<float>(normalLen/magnitude);
        normalY *= static_cast<float>(normalLen/magnitude);

        double cos2Theta = (normalX*prevNormalX + normalY*prevNormalY)/(normalLen*normalLen);
        float tempNormalX = 0.0f;
        float tempNormalY = 0.0f;
        double tempMagnitude = 0.0;

        if(cos2Theta < SQRT_HALF3)
        {
            FillVertexNormalMultiTexCoord2D(vertexBuffer, normalBuffer, texcoordBuffer1, texcoordBuffer2,
                                            pt1,
                                            -prevNormalX, -prevNormalY,
                                            TURN_X_COORD - TURN_WIDTH, TURN_Y_COORD,
                                            texX, texY);

            FillVertexNormalMultiTexCoord2D(vertexBuffer, normalBuffer, texcoordBuffer1, texcoordBuffer2,
                                            pt1,
                                            prevNormalX, prevNormalY,
                                            TURN_X_COORD + TURN_WIDTH, TURN_Y_COORD,
                                            texX, texY);

            FillVertexNormalMultiTexCoord2D(vertexBuffer, normalBuffer, texcoordBuffer1, texcoordBuffer2,
                                            pt1,
                                            prevNormalY - prevNormalX, -prevNormalX - prevNormalY,
                                            TURN_X_COORD - TURN_WIDTH, TURN_Y_COORD - TURN_WIDTH,
                                            texX, texY);

            FillVertexNormalMultiTexCoord2D(vertexBuffer, normalBuffer, texcoordBuffer1, texcoordBuffer2,
                                            pt1,
                                            prevNormalY + prevNormalX, -prevNormalX + prevNormalY,
                                            TURN_X_COORD + TURN_WIDTH, TURN_Y_COORD - TURN_WIDTH,
                                            texX, texY);
            vertexIndex += 4;

            GeneratTriangleStripIndex(indexBuffer, start, vertexIndex);
            start = vertexIndex;

            tempNormalX = normalX ;
            tempNormalY = normalY ;
        }
        else
        {
            double cosTheta = nsl_sqrt((cos2Theta+1)/2);
            tempNormalX = (normalX + prevNormalX) ;
            tempNormalY = (normalY + prevNormalY) ;
            tempMagnitude = Hypot(tempNormalX, tempNormalY);
            tempNormalX *= static_cast<float>(normalLen/cosTheta/tempMagnitude);
            tempNormalY *= static_cast<float>(normalLen/cosTheta/tempMagnitude);
        }
        FillVertexNormalMultiTexCoord2D(vertexBuffer, normalBuffer, texcoordBuffer1, texcoordBuffer2,
                                        pt1,
                                        -tempNormalX, -tempNormalY,
                                        ROAD_X_COORD, ROAD_Y_COORD,
                                        texX, texY);

        FillVertexNormalMultiTexCoord2D(vertexBuffer, normalBuffer, texcoordBuffer1, texcoordBuffer2,
                                        pt1,
                                        tempNormalX, tempNormalY,
                                        ROAD_X_COORD + ROAD_WIDTH, ROAD_Y_COORD,
                                        texX, texY);
        vertexIndex += 2;

        prevNormalX = normalX;
        prevNormalY = normalY;

        pt1 = pt2;
    }

    if (magnitude == 0)
    {
        normalX = prevNormalX ;
        normalY = prevNormalY ;
    }

    FillVertexNormalMultiTexCoord2D(vertexBuffer, normalBuffer, texcoordBuffer1, texcoordBuffer2,
                                    pt1,
                                    -normalX, -normalY,
                                    ROAD_X_COORD, ROAD_Y_COORD,
                                    texX, texY);

    FillVertexNormalMultiTexCoord2D(vertexBuffer, normalBuffer, texcoordBuffer1, texcoordBuffer2,
                                    pt1,
                                    normalX, normalY,
                                    ROAD_X_COORD + ROAD_WIDTH, ROAD_Y_COORD,
                                    texX, texY);

    vertexIndex += 2;

    GeneratTriangleStripIndex(indexBuffer, start, vertexIndex);

    return PAL_Ok;
    }
