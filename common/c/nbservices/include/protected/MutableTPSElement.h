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

    @file     MutableTPSElement.h

    Description: MutableTPSElement is a wrapper class of tpselt.
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
#ifndef _MUTABLE_TPS_ELEMENT_H_
#define _MUTABLE_TPS_ELEMENT_H_

extern "C"{
#include "tpselt.h"
#include "vec.h"
#include "nberror.h"
#include "nbexp.h"
}

#include "smartpointer.h"

class MutableTpsElement;
typedef shared_ptr<MutableTpsElement> MutableTpsElementPtr;

class MutableTpsElement
{
public:
    MutableTpsElement(bool isRoot = true);
    MutableTpsElement(shared_ptr<string> name, bool isRoot = true);
    MutableTpsElement(tpselt tp, bool isRoot = true);
    ~MutableTpsElement();

    // Managing
    nb_boolean          IsValid        ();
    shared_ptr<string>  GetElementName ();
    MutableTpsElement  Clone     ();

    NB_Error      Attach           (MutableTpsElement* child);
    void          UnlinkFromParent (void);

    MutableTpsElement* GetParent        (void);
    MutableTpsElement* GetChildByIndex  (int index);

    // Attribute getters
    NB_Error GetAttribute(shared_ptr<string> attrName, char** out, uint32* size);
    NB_Error GetAttributeNameFromIndex(int index, char** out);

    int GetAttrCount ();
    int GetChildCount();

    // Attribute Setters
    NB_Error SetAttribute(shared_ptr<string> attrName, const char* in, uint32 size);

    tpselt GetTPSElement();

    void AttachToRequest(bool attachToRequest) { m_isAttachToRequest = attachToRequest; }
    bool IsAttached() { return m_isAttachToRequest; }
    void SetCanDestory(bool canDestory) { m_canDestory = canDestory; }
    bool CanDestory() { return m_canDestory; }

private:
    bool m_isRoot;
    bool m_isAttachToRequest;
    bool m_canDestory;
    tpselt m_te;
    string m_name;
};

#endif //_MUTABLE_TPS_ELEMENT_H_
/*! @{ */
