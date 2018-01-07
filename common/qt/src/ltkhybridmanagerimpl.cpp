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

 @file ltkhybridmanagerimpl.cpp
 @date 02/15/2014
 @addtogroup common

 */
/*
 * (C) Copyright 2015 by TeleCommunication Systems, Inc.
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

#include "ltkhybridmanagerimpl.h"
#include "ltkcontextimpl.h"

#include "nbhybridmanager.h"
#include "HybridStrategy.h"

using namespace locationtoolkit;

class DefaultHybridStrategy: public nbcommon::HybridStrategy
{
public:
    DefaultHybridStrategy(NB_Context* context)
        :nbcommon::HybridStrategy(context)
    {

    }
    virtual ~DefaultHybridStrategy(){}

public:
    virtual nbcommon::HybridMode getMode() const{
        return enableOnboard ? nbcommon::HBM_ONBOARD_ONLY : nbcommon::HBM_OFFBOARD_ONLY;
    }

    void EnableOnboard(bool enable){
        enableOnboard = enable;
        onHybridModeChanged();
    }

private:
    bool enableOnboard;
};

LTKHybridManager* LTKHybridManager::CreateInstance(LTKContext& ltkContext)
{
    return new LTKHybridManagerImpl(ltkContext);
}

LTKHybridManagerImpl::LTKHybridManagerImpl(LTKContext& ltkContext)
    :mLtkContext(ltkContext)
{
    NB_Context* nbContext = static_cast<LTKContextImpl&>(ltkContext).GetNBContext();
    hybridStrategy = new DefaultHybridStrategy(nbContext);
    NB_HybridManagerSetStrategy(nbContext, hybridStrategy);
}

LTKHybridManagerImpl::~LTKHybridManagerImpl()
{
}

bool LTKHybridManagerImpl::EnableOnboard(bool enable)
{
    if(hybridStrategy){
        static_cast<DefaultHybridStrategy*>(hybridStrategy)->EnableOnboard(enable);
        return TRUE;
    }
    return FALSE;
}

bool LTKHybridManagerImpl::IsOnboardEnabled()
{
    ///TODO: useless
    return FALSE;
}

int LTKHybridManagerImpl::SetLanguage(const QString& language)
{
    LTKContextImpl& contextImpl = static_cast<LTKContextImpl &>( mLtkContext );
    return NB_HybridManagerSetLanguage(contextImpl.GetNBContext(), language.toStdString().c_str());
}

int LTKHybridManagerImpl::AddMapDataPath(const QString& path)
{
    LTKContextImpl& contextImpl = static_cast<LTKContextImpl &>( mLtkContext );
    return NB_HybridManagerAddMapDataPath(contextImpl.GetNBContext(), path.toStdString().c_str());
}

int LTKHybridManagerImpl::SetWorldMapDataPath(const QString& path)
{
    LTKContextImpl& contextImpl = static_cast<LTKContextImpl &>( mLtkContext );
    return NB_HybridManagerAddWorldMapDataPath(contextImpl.GetNBContext(), path.toStdString().c_str());
}

int LTKHybridManagerImpl::BeginLoadMapData()
{
    LTKContextImpl& contextImpl = static_cast<LTKContextImpl &>( mLtkContext );
    return NB_HybridManagerBeginLoadMapData(contextImpl.GetNBContext());
}

int LTKHybridManagerImpl::EndLoadMapData()
{
    LTKContextImpl& contextImpl = static_cast<LTKContextImpl &>( mLtkContext );
    return NB_HybridManagerEndLoadMapData(contextImpl.GetNBContext());
}

const char* LTKHybridManagerImpl::MapDataVersion()
{
    LTKContextImpl& contextImpl = static_cast<LTKContextImpl &>( mLtkContext );
    return NB_HybridManagerGetMapDataVersion(contextImpl.GetNBContext());
}
