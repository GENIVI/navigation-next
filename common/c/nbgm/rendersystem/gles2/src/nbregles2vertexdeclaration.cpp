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
#include "nbregles2vertexdeclaration.h"

static NBRE_VertexInputFormat
ToVertexInputFormat(NBRE_VertexElement::VertexElementSemantic sem, uint32 index)
{
    switch (sem)
    {
    case NBRE_VertexElement::VES_POSITION:
        return NBRE_VSIF_POSITION;

    case NBRE_VertexElement::VES_TEXTURE_COORDINATES:
        {
            switch (index)
            {
            case 0:
                return NBRE_VSIF_TEXCORD0;
            case 1:
                return NBRE_VSIF_TEXCORD1;
            default:
                nbre_assert(0);
                return NBRE_VSIF_TEXCORD0;
            }
        }

    case NBRE_VertexElement::VES_DIFFUSE:
        return NBRE_VSIF_COLOR;

    case NBRE_VertexElement::VES_NORMAL:
        return NBRE_VSIF_NORMAL;

    case NBRE_VertexElement::VES_SPECULAR:
    case NBRE_VertexElement::VES_BLEND_INDICES:
    case NBRE_VertexElement::VES_BLEND_WEIGHTS:
    case NBRE_VertexElement::VES_BINORMAL:
    case NBRE_VertexElement::VES_TANGENT:
    default:
        return NBRE_VSIF_POSITION;
    }
}

NBRE_GLES2VertexDeclaration::NBRE_GLES2VertexDeclaration():mId(0),mVertexInputFormat(0xFFFFFFFF)
{
}

NBRE_GLES2VertexDeclaration::~NBRE_GLES2VertexDeclaration()
{
    if(mId != 0)
    {
        glDeleteVertexArrays(1, &mId);        
    }
}

uint32
NBRE_GLES2VertexDeclaration::GetVertexInputFormat()const
{
    if(mVertexInputFormat == 0xFFFFFFFF)
    {
        mVertexInputFormat = 0;
        VertexElementList::const_iterator end = mElementList.end();
        for(VertexElementList::const_iterator it = mElementList.begin(); it != end; it++)
        {
            mVertexInputFormat |= ToVertexInputFormat((*it)->Semantic(), (*it)->Index());
        }
    }
    return mVertexInputFormat;
}
