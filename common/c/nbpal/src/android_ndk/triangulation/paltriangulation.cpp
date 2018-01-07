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

/*!--------------------------------------------------------------------------

    @file       paltriangulation.cpp

    See header file for description.
*/
/*
    (C) Copyright 2012 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
#include "paltriangulation.h"

#include <vector>
#include "glues.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifndef CALLBACK
#define CALLBACK APIENTRYP
#endif

#define GluFloat GLfloat

struct GluTriContext
{
public:
    GluTriContext(std::vector<PAL_TriangulationVertex>* outputVertices)
        :outputVertices(outputVertices), outputContours(NULL)
    {
    }

    GluTriContext(std::vector<std::vector<PAL_TriangulationVertex> >* outputContours)
        :outputVertices(NULL), outputContours(outputContours)
    {
    }

    ~GluTriContext()
    {
        for (uint32 i = 0; i < generatedVertices.size(); ++i)
        {
            delete generatedVertices[i];
        }
    }

    void BeginPrimitiveBatch(GLenum primitiveType)
    {
        this->primitiveType = primitiveType;
        vertices.clear();
    }

    void EndPrimitiveBatch()
    {
        switch (primitiveType)
        {
        case GL_TRIANGLE_FAN:
            for (uint32 i = 2; i < vertices.size(); ++i)
            {
                outputVertices->push_back(vertices[0]);
                outputVertices->push_back(vertices[i - 1]);
                outputVertices->push_back(vertices[i]);
            }
            break;
        case GL_TRIANGLE_STRIP:
            for (uint32 i = 2; i < vertices.size(); ++i)
            {
                if (i % 2 == 0)
                {
                    outputVertices->push_back(vertices[i - 2]);
                    outputVertices->push_back(vertices[i - 1]);
                    outputVertices->push_back(vertices[i]);
                }
                else
                {
                    outputVertices->push_back(vertices[i - 1]);
                    outputVertices->push_back(vertices[i - 2]);
                    outputVertices->push_back(vertices[i]);
                }
            }
            break;
        case GL_TRIANGLES:
            for (uint32 i = 0; i < vertices.size(); ++i)
            {
                outputVertices->push_back(vertices[i]);
            }
            break;
        case GL_LINE_LOOP:
            {
                std::vector<PAL_TriangulationVertex> contour;
                for (uint32 i = 0; i < vertices.size(); ++i)
                {
                    contour.push_back(vertices[i]);
                }
                outputContours->push_back(contour);
            }
            break;
        default:
            // Unknown type
            break;
        }
    }

    GluFloat* NewVertex(GluFloat x, GluFloat y, GluFloat z, GluFloat s, GluFloat t)
    {
        GluFloat* v = new GluFloat[5];
        v[0] = x;
        v[1] = y;
        v[2] = z;
        v[3] = s;
        v[4] = t;
        generatedVertices.push_back(v);
        return v;
    }

    void AddBatchVertex(GluFloat* v)
    {
        PAL_TriangulationVertex vt;
        vt.x = (float)v[0];
        vt.y = (float)v[1];
        vt.s = (float)v[3];
        vt.t = (float)v[4];
        vertices.push_back(vt);
    }
public:
    std::vector<GluFloat*> generatedVertices;
    GLenum primitiveType;
    std::vector<PAL_TriangulationVertex>* outputVertices;
    std::vector<std::vector<PAL_TriangulationVertex> >* outputContours;
    std::vector<PAL_TriangulationVertex> vertices;
};

void CALLBACK tessBeginCB(GLenum which, void* polygon)
{
    GluTriContext* context = (GluTriContext*)polygon;
    context->BeginPrimitiveBatch(which);
}

void CALLBACK tessEndCB(void* polygon)
{
    GluTriContext* context = (GluTriContext*)polygon;
    context->EndPrimitiveBatch();
}

void CALLBACK tessVertexCB(void *data, void* polygon)
{
    GluTriContext* context = (GluTriContext*)polygon;
    context->AddBatchVertex((GluFloat*)data);
}

void CALLBACK tessCombineCB(const GluFloat newVertex[3], const GluFloat *neighborVertex[4],
                            const GLfloat neighborWeight[4], GluFloat **outData, void* polygon)
{
    GluTriContext* context = (GluTriContext*)polygon;
    GluFloat s = 0;
    GluFloat t = 0;

    for (uint32 i = 0; i < 4; ++i)
    {
        if (neighborVertex[i])
        {
            s += neighborWeight[i] * neighborVertex[i][3];
            t += neighborWeight[i] * neighborVertex[i][4];
        }
    }

    *outData = context->NewVertex(newVertex[0], newVertex[1], newVertex[2], s, t);
}

void CALLBACK tessErrorCB(GLenum errorCode, void* polygon)
{
}

void GluRegulateContoursWinding(const PAL_TriangulationVertex* polygon, uint32 count, std::vector<std::vector<PAL_TriangulationVertex> >& contours)
{
    GLUtesselator *tessContour = gluNewTess();
    gluTessProperty(tessContour, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_NONZERO);
    gluTessProperty(tessContour, GLU_TESS_BOUNDARY_ONLY, GL_TRUE);
    gluTessCallback(tessContour, GLU_TESS_BEGIN_DATA, (void (CALLBACK)(void))tessBeginCB);
    gluTessCallback(tessContour, GLU_TESS_END_DATA, (void (CALLBACK)(void))tessEndCB);
    gluTessCallback(tessContour, GLU_TESS_ERROR_DATA, (void (CALLBACK)(void))tessErrorCB);
    gluTessCallback(tessContour, GLU_TESS_VERTEX_DATA, (void (CALLBACK)(void))tessVertexCB);
    gluTessCallback(tessContour, GLU_TESS_COMBINE_DATA, (void (CALLBACK)(void))tessCombineCB);

    GluTriContext context(&contours);
    gluTessNormal(tessContour, 0, 0, 1);

    gluTessBeginPolygon(tessContour, &context);
    gluTessBeginContour(tessContour);
    for (uint32 i = 0; i < count; ++i)
    {
        GluFloat* vert = context.NewVertex(polygon[i].x, polygon[i].y, 0, polygon[i].s, polygon[i].t);
        gluTessVertex(tessContour, vert, vert);
    }
    gluTessEndContour(tessContour);
    gluTessEndPolygon(tessContour);

    gluDeleteTess(tessContour);
}

PAL_DEF nb_boolean PAL_TriangulationPerform(const PAL_TriangulationVertexList* outerPolygons, uint32 outerCount,
                                            const PAL_TriangulationVertexList* innerPolygons, uint32 innerCount,
                                            PAL_TriangulationVertex** resultVertices, uint32* resultVertexCount,
                                            uint16** resultIndices, uint32* resultIndexCount)
{
    *resultIndexCount = 0;
    *resultVertexCount = 0;
    *resultIndices = NULL;
    *resultVertices = NULL;

    GLUtesselator *tess = gluNewTess();
    if (!tess)
    {
        return FALSE;
    }

    std::vector<PAL_TriangulationVertex> triVertices;
    std::vector<uint16> triIndices;

    try
    {
        // register callback functions
        gluTessCallback(tess, GLU_TESS_BEGIN_DATA, (void (CALLBACK)(void))tessBeginCB);
        gluTessCallback(tess, GLU_TESS_END_DATA, (void (CALLBACK)(void))tessEndCB);
        gluTessCallback(tess, GLU_TESS_ERROR_DATA, (void (CALLBACK)(void))tessErrorCB);
        gluTessCallback(tess, GLU_TESS_VERTEX_DATA, (void (CALLBACK)(void))tessVertexCB);
        gluTessCallback(tess, GLU_TESS_COMBINE_DATA, (void (CALLBACK)(void))tessCombineCB);
        gluTessProperty(tess, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_POSITIVE);

        std::vector<std::vector<PAL_TriangulationVertex> > outerContours;
        for (uint32 i = 0; i < outerCount; ++i)
        {
            GluRegulateContoursWinding(outerPolygons[i].vertices, outerPolygons[i].vertexCount, outerContours);
        }

        std::vector<std::vector<PAL_TriangulationVertex> > holeContours;
        for (uint32 i = 0; i < innerCount; ++i)
        {
            GluRegulateContoursWinding(innerPolygons[i].vertices, innerPolygons[i].vertexCount, holeContours);
        }

        GluTriContext context(&triVertices);

        gluTessNormal(tess, 0, 0, 1);
        gluTessBeginPolygon(tess, &context);
        for (uint32 i = 0; i < outerContours.size(); ++i)
        {
            gluTessBeginContour(tess);
            std::vector<PAL_TriangulationVertex>& c = outerContours[i];
            for (uint32 j = 0; j < c.size(); ++j)
            {
                GluFloat* v = context.NewVertex(c[j].x, c[j].y, 0, c[j].s, c[j].t);
                gluTessVertex(tess, v, v);
            }
            gluTessEndContour(tess);
        }
        for (uint32 i = 0; i < holeContours.size(); ++i)
        {
            gluTessBeginContour(tess);
            gluNextContour(tess, GLU_INTERIOR);
            std::vector<PAL_TriangulationVertex>& c = holeContours[i];
            uint32 countourVertexCount = c.size();
            for (uint32 j = 0; j < countourVertexCount; ++j)
            {
                PAL_TriangulationVertex& cv = c[countourVertexCount - 1 - j];
                GluFloat* v = context.NewVertex(cv.x, cv.y, 0, cv.s, cv.t);
                gluTessVertex(tess, v, v);
            }
            gluTessEndContour(tess);
        }
        gluTessEndPolygon(tess);

        gluDeleteTess(tess);
    }
    catch (...)
    {
    }

    if (triVertices.size() > 0)
    {
        PAL_TriangulationVertex* rv = (PAL_TriangulationVertex*)malloc(sizeof(PAL_TriangulationVertex) * triVertices.size());
        memcpy(rv, &triVertices[0], triVertices.size() * sizeof(PAL_TriangulationVertex));
        *resultVertexCount = triVertices.size();
        *resultVertices = rv;

        for (uint32 i = 0; i < triVertices.size(); ++i)
        {
            triIndices.push_back((uint16)i);
        }

        if (triIndices.size() > 0)
        {
            uint16* ri = (uint16*)malloc(sizeof(uint16) * triIndices.size());
            memcpy(ri, &triIndices[0], triIndices.size() * sizeof(uint16));
            *resultIndices = ri;
            *resultIndexCount = triVertices.size();
        }
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

PAL_DEF void PAL_TriangulationRelease(PAL_TriangulationVertex* vertices, uint16* indices)
{
    if (vertices)
    {
        free(vertices);
    }

    if (indices)
    {
        free(indices);
    }
}
