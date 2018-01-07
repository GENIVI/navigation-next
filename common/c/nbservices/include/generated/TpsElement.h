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

#ifndef _PROTOCOL_TPS_ELEMENT_H_
#define _PROTOCOL_TPS_ELEMENT_H_

extern "C"{
    #include "tpselt.h"
    #include "vec.h"
    #include "nberror.h"
    #include "nbexp.h"
}

#include "base.h"
#include "smartpointer.h"


namespace protocol
{

class TpsElement;
typedef shared_ptr<protocol::TpsElement> TpsElementPtr;

class NB_DLL TpsElement : public Base
{
public:
    TpsElement();
    TpsElement(shared_ptr<string> name);
    TpsElement(tpselt tp);
    ~TpsElement();

    // Managing
    nb_boolean          IsValid        ();
    shared_ptr<string>  GetElementName ();
    TpsElement          Clone          ();

    NB_Error      Attach           (TpsElementPtr child);
    void          UnlinkFromParent (void);

    TpsElementPtr GetParent        (void);
    TpsElementPtr GetChild         (shared_ptr<string> childName);
    TpsElementPtr GetNextChild     (int *iterator);
    TpsElementPtr GetChildByIndex  (int index);

    void          DeleteAttribute  (shared_ptr<string> attrName);

    // Attribute getters
    NB_Error           GetAttribute           (shared_ptr<string> attrName, char** out, uint32* size);
    byte               GetAttributeAsByte     (shared_ptr<string> attrName);
    double             GetAttributeAsDouble   (shared_ptr<string> attrName);
    float              GetAttributeAsFloat    (shared_ptr<string> attrName);
    int32              GetAttributeAsInt32    (shared_ptr<string> attrName);
    uint32             GetAttributeAsUint32   (shared_ptr<string> attrName);
    uint64             GetAttributeAsUint64   (shared_ptr<string> attrName);
    shared_ptr<string> GetAttributeAsString   (shared_ptr<string> attrName);
    CSL_Vector*        GetAllAttributesVector (void);

    tpselt             GetTPSElement();

    // Attribute Setters
    NB_Error SetAttribute       (shared_ptr<string> attrName, const char* in, uint32 size);
    NB_Error SetAttributeByte   (shared_ptr<string> attrName, byte value);
    NB_Error SetAttributeDouble (shared_ptr<string> attrName, double value);
    NB_Error SetAttributeFloat  (shared_ptr<string> attrName, float value);
    NB_Error SetAttributeInt32  (shared_ptr<string> attrName, int32 value);
    NB_Error SetAttributeUint32 (shared_ptr<string> attrName, uint32 value);
    NB_Error SetAttributeUint64 (shared_ptr<string> attrName, uint64 value);
    NB_Error SetAttributeString (shared_ptr<string> attrName, shared_ptr<string> attrValue);

private:
    tpselt m_te;
};

}
#endif //_PROTOCOL_TPS_ELEMENT_H_
