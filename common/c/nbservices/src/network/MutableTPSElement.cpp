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

    @file     MutableTPSElement.cpp
*/
/*
    (C) Copyright 2013 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to Networks In Motion, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
/*! @{ */

#include "MutableTPSElement.h"

#define GET_CHAR_PTR(n) n->c_str()

/* See description in header file. */
MutableTpsElement::MutableTpsElement(bool isRoot)
    : m_isRoot(isRoot), m_isAttachToRequest(false), m_canDestory(false)
{
    m_te = te_new("");
}

/* See description in header file. */
MutableTpsElement::MutableTpsElement(shared_ptr<string> name, bool isRoot)
    : m_isRoot(isRoot), m_isAttachToRequest(false), m_canDestory(false)
{
    m_te = te_new(GET_CHAR_PTR(name));
    m_name.assign(name ? name->c_str():"Unnamed....");
}

/* See description in header file. */
MutableTpsElement::MutableTpsElement(tpselt tp, bool isRoot)
    : m_isRoot(isRoot), m_isAttachToRequest(false), m_canDestory(false)
{
    if (m_isRoot)
    {
        m_te = te_clone(tp);
    }
    else
    {
        m_te = tp;
    }
}

/* See description in header file. */
MutableTpsElement::~MutableTpsElement()
{
    if (m_isRoot && IsValid())
    {
        te_dealloc(m_te);
        m_te = NULL;
        m_isRoot = false;
    }
}

/* See description in header file. */
nb_boolean MutableTpsElement::IsValid()
{
    return (m_te != NULL);
}

/* See description in header file. */
shared_ptr<string> MutableTpsElement::GetElementName()
{
    if (IsValid())
        return CHAR_PTR_TO_UTF_STRING_PTR(te_getname(m_te));
    else
        return(CHAR_PTR_TO_UTF_STRING_PTR(""));
}

/* See description in header file. */
MutableTpsElement MutableTpsElement::Clone()
{
    return MutableTpsElement(te_clone(m_te));
}

/* See description in header file. */
NB_Error MutableTpsElement::Attach(MutableTpsElement* child)
{
    if (child)
    {
        child->m_isRoot = false;
        return (te_attach(m_te, child->m_te)) ? NE_OK : NE_INVAL;
    }

    return NE_OK;
}

/* See description in header file. */
void MutableTpsElement::UnlinkFromParent()
{
    te_unlink(m_te);
    m_isRoot = true;
}

/* See description in header file. */
MutableTpsElement* MutableTpsElement::GetParent()
{
    tpselt te = te_getparent(m_te);
    if (te)
    {
        return new MutableTpsElement(te, false);
    }
    else
    {
        return NULL;
    }
}

/* See description in header file. */
MutableTpsElement* MutableTpsElement::GetChildByIndex(int index)
{
    tpselt te = te_nthchild(m_te, index);
    if (te)
    {
        return new MutableTpsElement(te, false);
    }
    else
    {
        return NULL;
    }
}

/* See description in header file. */
NB_Error MutableTpsElement::GetAttribute(shared_ptr<string> attrName, char** out, uint32* size)
{
    size_t tSize = size ? (size_t)*size : 0;
    NB_Error error = (te_getattr(m_te, GET_CHAR_PTR(attrName), out, &tSize)) ? NE_OK : NE_INVAL;
    if (size)
    {
        *size = (uint32)tSize;
    }
    return error;
}

/* See description in header file. */
NB_Error MutableTpsElement::GetAttributeNameFromIndex(int index, char** out)
{
    NB_Error error = NE_OK;
    CSL_Vector* attributes = te_getattrs(m_te);
    if (index < 0 || index > CSL_VectorGetLength(attributes))
    {
        return NE_INVAL;
    }

    char** attributeName = (char**) CSL_VectorGetPointer(attributes, index);
    if ((!attributeName) ||
       (!(*attributeName)) ||
       (nsl_strlen(*attributeName) == 0))
    {
        error = NE_UNEXPECTED;
    }
    *out = *attributeName;

    return error;
}

NB_Error MutableTpsElement::SetAttribute(shared_ptr<string> attrName, const char* in, uint32 size)
{
    return (te_setattr(m_te, GET_CHAR_PTR(attrName), in, (size_t)size)) ? NE_OK : NE_INVAL;
}

/* See description in header file. */
tpselt MutableTpsElement::GetTPSElement()
{
    size_t size = m_name.size();
    if (size > 100)
    {
        string cc;
        cc.assign(m_name, 0, 100);
        return NULL;
    }

    return te_clone(m_te);
}

/* See description in header file. */
int MutableTpsElement::GetAttrCount()
{
    return CSL_VectorGetLength(te_getattrs(m_te));
}

/* See description in header file. */
int MutableTpsElement::GetChildCount()
{
    int count = 0;
    int iter = 0;

    tpselt te = NULL;
    while ((te = te_nextchild(m_te, &iter)))
    {
        count++;
    }

    return count;
}

/*! @} */
