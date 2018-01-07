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

#include "TpsElement.h"

using namespace protocol;


#define GET_CHAR_PTR(n) n->c_str()

TpsElement::TpsElement()
{
    m_te = te_new("");
}

TpsElement::TpsElement(shared_ptr<string> name)
{
    m_te = te_new(GET_CHAR_PTR(name));
}

TpsElement::TpsElement(tpselt tp)
{
    m_te = te_clone(tp);
}

TpsElement::~TpsElement()
{
    te_dealloc(m_te);
}

nb_boolean TpsElement::IsValid()
{
    return (m_te != NULL);
}

shared_ptr<string> TpsElement::GetElementName()
{
    if (IsValid())
        return CHAR_PTR_TO_UTF_STRING_PTR(te_getname(m_te));
    else
        return(CHAR_PTR_TO_UTF_STRING_PTR(""));
}

TpsElement TpsElement::Clone()
{
    return TpsElement(te_clone(m_te));
}

NB_Error TpsElement::Attach(TpsElementPtr child)
{
    if (child)
    {
        return (te_attach(m_te, te_clone(child->m_te))) ? NE_OK : NE_INVAL;
    }

    /* @todo: I will discuss with others about this returned error. Because other generated code
       may pass an empty pointer to this function.
    */
    return NE_OK;
}

void TpsElement::UnlinkFromParent()
{
    te_unlink(m_te);
}

TpsElementPtr TpsElement::GetParent()
{
    tpselt te = te_getparent(m_te);
    if (te)
    {
        return TpsElementPtr( new TpsElement(te));
    }
    else
    {
        return TpsElementPtr();
    }
}

TpsElementPtr TpsElement::GetChild(shared_ptr<string> childName)
{
    tpselt te = te_getchild(m_te, GET_CHAR_PTR(childName));
    if (te)
    {
        return TpsElementPtr( new TpsElement(te));
    }
    else
    {
        return TpsElementPtr();
    }
}

TpsElementPtr TpsElement::GetNextChild(int *iterator)
{
    tpselt te = te_nextchild(m_te, iterator);
    if (te)
    {
        return TpsElementPtr( new TpsElement(te));
    }
    else
    {
        return TpsElementPtr();
    }
}

TpsElementPtr TpsElement::GetChildByIndex(int index)
{
    tpselt te = te_nthchild(m_te, index);
    if (te)
    {
        return TpsElementPtr( new TpsElement(te));
    }
    else
    {
        return TpsElementPtr();
    }
}

void TpsElement::DeleteAttribute(shared_ptr<string> attrName)
{
    te_delattr(m_te, GET_CHAR_PTR(attrName));
}

NB_Error TpsElement::GetAttribute (shared_ptr<string> attrName, char** out, uint32* size)
{
    size_t tSize = size ? (size_t)*size : 0;
    NB_Error error = (te_getattr(m_te, GET_CHAR_PTR(attrName), out, &tSize)) ? NE_OK : NE_INVAL;
    if (size)
    {
        *size = (uint32)tSize;
    }
    return error;
}

shared_ptr<string> TpsElement::GetAttributeAsString (shared_ptr<string> attrName)
{
    //shared_ptr<string> temp(new std::string(0))
    const char* attribute = te_getattrc(m_te, GET_CHAR_PTR(attrName));
    if(attribute)
    {
        return CHAR_PTR_TO_UTF_STRING_PTR(attribute);
    }
    else
    {
        return(CHAR_PTR_TO_UTF_STRING_PTR(""));
    }
}

uint32 TpsElement::GetAttributeAsUint32 (shared_ptr<string> attrName)
{
    return te_getattru(m_te, GET_CHAR_PTR(attrName));
}

uint64 TpsElement::GetAttributeAsUint64 (shared_ptr<string> attrName)
{
    uint64 retval = 0;
    te_getattru64(m_te, GET_CHAR_PTR(attrName), &retval);
    return retval;
}

float TpsElement::GetAttributeAsFloat (shared_ptr<string> attrName)
{
    double retval = 0.0;
    te_getattrf(m_te, GET_CHAR_PTR(attrName), &retval);
    return (float)retval;
}

double TpsElement::GetAttributeAsDouble (shared_ptr<string> attrName)
{
    double retval = 0.0;
    te_getattrd(m_te, GET_CHAR_PTR(attrName), &retval);
    return retval;
}

int32 TpsElement::GetAttributeAsInt32(shared_ptr<string> attrName)
{
    return (int32)GetAttributeAsUint32(attrName);
}

byte TpsElement::GetAttributeAsByte(shared_ptr<string> attrName)
{
    return (byte)GetAttributeAsUint32(attrName);
}

CSL_Vector* TpsElement::GetAllAttributesVector (void)
{
    return te_getattrs(m_te);
}

tpselt TpsElement::GetTPSElement()
{
    return te_clone(m_te);
}

NB_Error TpsElement::SetAttribute (shared_ptr<string> attrName, const char* in, uint32 size)
{
    return (te_setattr(m_te, GET_CHAR_PTR(attrName), in, (size_t)size)) ? NE_OK : NE_INVAL;
}

NB_Error TpsElement::SetAttributeString (shared_ptr<string> attrName, shared_ptr<string> attrValue)
{
    return (te_setattrc(m_te, GET_CHAR_PTR(attrName), attrValue?GET_CHAR_PTR(attrValue):"")) ? NE_OK : NE_INVAL;
}

NB_Error TpsElement::SetAttributeUint32 (shared_ptr<string> attrName, uint32 value)
{
    return (te_setattru(m_te, GET_CHAR_PTR(attrName), value)) ? NE_OK : NE_INVAL;
}

NB_Error TpsElement::SetAttributeUint64 (shared_ptr<string> attrName, uint64 value)
{
    return (te_setattru64(m_te, GET_CHAR_PTR(attrName), value)) ? NE_OK : NE_INVAL;
}

NB_Error TpsElement::SetAttributeFloat (shared_ptr<string> attrName, float value)
{
    return (te_setattrf(m_te, GET_CHAR_PTR(attrName), value)) ? NE_OK : NE_INVAL;
}

NB_Error TpsElement::SetAttributeDouble (shared_ptr<string> attrName, double value)
{
    return (te_setattrd(m_te, GET_CHAR_PTR(attrName), value)) ? NE_OK : NE_INVAL;
}

NB_Error TpsElement::SetAttributeInt32(shared_ptr<string> attrName, int32 value)
{
    return SetAttributeUint32(attrName, (int32)value);
}

NB_Error TpsElement::SetAttributeByte(shared_ptr<string> attrName, byte value)
{
    return SetAttributeUint32(attrName, (byte)value);
}
