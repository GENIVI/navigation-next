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

#include "stdafx.h"
#include "resource.h"
#include "TestMeshView.h"
#include "nbrecommon.h"
#include "nbretransformation.h"
#include "NBGMTestUtilities.h"
#include "nbrematrix4x4.h"
#include "nbretransformation.h"

CTestMeshView::CTestMeshView(): m_Mesh(NULL)
{
}

CTestMeshView::~CTestMeshView()
{
    NBRE_DELETE m_Mesh;
}

int CTestMeshView::OnCreate()
{
    m_Mesh = CreateSphere(*m_RenderPal, 10.0f);
    return 0;
}

void CTestMeshView::InitRenderState()
{
    ASSERT(m_RenderPal);

    m_RenderPal->EnableCullFace(TRUE);
}

void CTestMeshView::OnSize(UINT /*nType*/, int cx, int cy)
{
    if(cy == 0)
    {
        cy = 1;
    }

    NBRE_Matrix4x4d rotateMatirx = NBRE_Transformationd::BuildRotateDegreesMatrix(20.0f, 1.0f, 0.0f, 0.0f);
    NBRE_Matrix4x4d translateMatirx = NBRE_Transformationd::BuildTranslateMatrix(0.0f, 0.0f, -50.0f);

    m_RenderPal->SetViewPort(0, 0, cx, cy);
    m_RenderPal->SetProjectionTransform(NBRE_Transformationf::BuildPerspectiveMatrix(45.0f, (float)cx/(float)cy,0.1f,100.0f));

    NBRE_Matrix4x4d identity;
    m_RenderPal->SetViewTransform(identity);

    translateMatirx = translateMatirx * rotateMatirx;
    m_RenderPal->SetWorldTransform(translateMatirx);
}

void CTestMeshView::OnRender()
{
    GetWindow()->OnRenderBegine();
    ASSERT(m_RenderPal);
    m_RenderPal->SetClearColor(0, 0, 0, 1);
    uint32 flags = 0;
    flags |= NBRE_CLF_CLEAR_COLOR;
    flags |= NBRE_CLF_CLEAR_DEPTH;

    m_RenderPal->Clear(flags);
    m_RenderPal->BeginScene();
    m_RenderPal->SetWireframeMode(TRUE);
    m_RenderPal->SetColor(0.5f, 0.0f, 0.0f, 1.0f);
    m_RenderPal->SetVertexBuffer(m_Mesh->GetSubMesh(0)->GetVertexData());
    m_RenderPal->DrawIndexedPrimitive(NBRE_PMT_TRIANGLE_LIST, m_Mesh->GetSubMesh(0)->GetIndexData());
    m_RenderPal->EndScene();
    GetWindow()->OnRenderEnd();
}
