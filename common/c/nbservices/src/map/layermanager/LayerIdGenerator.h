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
   @file        LayerIdGenerator.h
   @defgroup    nbmap

   Description: LayerIdGenerator is shared by all kinds of LayerProviders to
   generate layer id.
*/
/*
   (C) Copyright 2012 by TeleCommunications Systems, Inc.

   The information contained herein is confidential, proprietary to
   TeleCommunication Systems, Inc., and considered a trade secret as defined
   in section 499C of the penal code of the State of California. Use of this
   information by anyone other than authorized employees of TeleCommunication
   Systems is granted only under a written non-disclosure agreement, expressly
   prescribing the scope and manner of such use.

 --------------------------------------------------------------------------*/

/*! @{ */
#ifndef _LAYERIDGENERATOR_H_
#define _LAYERIDGENERATOR_H_

#include "nbcontext.h"
#include "smartpointer.h"
#include "base.h"

namespace nbmap
{
class LayerIdGenerator : public Base
{
public:
    LayerIdGenerator(NB_Context* context);
    virtual ~LayerIdGenerator();

    /*! Generates a new id.

        @return id generated.
    */
    uint32 GenerateId();

    /*! Resets this generator to regenerate id from m_base.

        @return None.
    */
    void Reset();

    NB_Context* GetContext();

    static const uint32 InvalidId = (uint32)0xFFFFFFFF;

private:
    // LayerIdGenerator(const LayerIdGenerator& instance);
    // LayerIdGenerator& operator=(const LayerIdGenerator& instance);

    uint32 m_currentId;      /*!< Current ID which can be used as LayerId. */
    NB_Context* m_pContext;
};

typedef shared_ptr<LayerIdGenerator> LayerIdGeneratorPtr;

}

#endif /* _LAYERIDGENERATOR_H_ */

/*! @} */
