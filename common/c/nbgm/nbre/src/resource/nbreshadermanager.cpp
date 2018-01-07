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
#include "nbreshadermanager.h"
#include "nbredefaultshadergroup.h"
#include "nbretypes.h"

NBRE_ShaderManager::NBRE_ShaderManager()
    :mLatestUpdated(0)
{
    AddGroup("\\\\", NBRE_IShaderGroupPtr(NBRE_NEW NBRE_DefaultShaderGroup()));
    SetCurrentGroup("\\\\");

    mDefaultShader = NBRE_ShaderPtr(NBRE_NEW NBRE_Shader());
    NBRE_PassPtr pass(NBRE_NEW NBRE_Pass());
    mDefaultShader->AddPass(pass);
}

NBRE_ShaderManager::~NBRE_ShaderManager()
{
}

static NBRE_String
ParseGroupType(const NBRE_String& groupID)
{
    uint32 index = groupID.find_first_of('\\');
    if (index == NBRE_INVALID_INDEX)
    {
        return "";
    }

    return NBRE_String(groupID, 0, index);
}

static NBRE_String
ParseGroupName(const NBRE_String& groupID)
{
    uint32 index = groupID.find_first_of('\\');
    if (index == NBRE_INVALID_INDEX)
    {
        return "";
    }

    return NBRE_String(groupID, index + 1, groupID.length());
}

static NBRE_String
ParseShaderName(const NBRE_String& groupID)
{
    uint32 index = groupID.find_last_of('\\');
    if (index == NBRE_INVALID_INDEX)
    {
        return groupID;
    }

    return NBRE_String(groupID, index + 1, groupID.length());
}

const NBRE_ShaderPtr&
NBRE_ShaderManager::GetShader(const NBRE_String& shaderID) const
{
    NBRE_String groupType = ParseGroupType(shaderID);
    NBRE_String shaderName = ParseShaderName(shaderID);

    const NBRE_ShaderPtr* result = NULL;
    NBRE_Map<NBRE_String, ShaderGroupSetPtr>::const_iterator i = mGroupSetMap.find(groupType);
    if (i != mGroupSetMap.end())
    {
        const ShaderGroupSetPtr& groupSetPtr = i->second;
        nbre_assert(groupSetPtr->mCurrentGroup);
        
        result = &groupSetPtr->mCurrentGroup->GetShader(shaderName);
    }
    else
    {
        result = &mDefaultShader;
    }
    NBRE_DebugLog(PAL_LogSeverityCritical, "NBRE_ShaderManager::GetShader end shaderId=%s", shaderID.c_str());
    return *result;
}

void
NBRE_ShaderManager::SetShader(const NBRE_String& shaderID, NBRE_ShaderPtr shader)
{
    NBRE_String groupType = ParseGroupType(shaderID);
    NBRE_String shaderName = ParseShaderName(shaderID);

    NBRE_Map<NBRE_String, ShaderGroupSetPtr>::iterator i = mGroupSetMap.find(groupType);
    if (i != mGroupSetMap.end())
    {
        ShaderGroupSetPtr& groupSetPtr = i->second;
        nbre_assert(groupSetPtr->mCurrentGroup);

        groupSetPtr->mCurrentGroup->SetShader(shaderName, shader);
    }
}

void
NBRE_ShaderManager::RemoveShader(const NBRE_String& shaderID)
{
    NBRE_String groupType = ParseGroupType(shaderID);
    NBRE_String shaderName = ParseShaderName(shaderID);

    NBRE_Map<NBRE_String, ShaderGroupSetPtr>::iterator i = mGroupSetMap.find(groupType);
    if (i != mGroupSetMap.end())
    {
        ShaderGroupSetPtr& groupSetPtr = i->second;
        nbre_assert(groupSetPtr->mCurrentGroup);

        groupSetPtr->mCurrentGroup->RemoveShader(shaderName);
    }
}

PAL_Error
NBRE_ShaderManager::AddGroup(const NBRE_String& groupID, const NBRE_IShaderGroupPtr& group)
{
    if (!group)
    {
        return PAL_ErrBadParam;
    }

    NBRE_String groupType = ParseGroupType(groupID);
    NBRE_String groupName = ParseGroupName(groupID);

    NBRE_Map<NBRE_String, ShaderGroupSetPtr>::iterator i = mGroupSetMap.find(groupType);
    if (i != mGroupSetMap.end())
    {
        ShaderGroupSetPtr& groupSet = i->second;
        groupSet->mGroupMap[groupName] = group;
    }
    else
    {
        ShaderGroupSetPtr groupSetPtr(NBRE_NEW ShaderGroupSet(groupType));
        groupSetPtr->mGroupMap[groupName] = group;
        mGroupSetMap[groupType] = groupSetPtr;
    }
    return PAL_Ok;
}

PAL_Error
NBRE_ShaderManager::SetCurrentGroup(const NBRE_String& groupID)
{
    NBRE_String groupType = ParseGroupType(groupID);
    NBRE_String groupName = ParseGroupName(groupID);

    if(!groupType.empty() && groupName.empty())
    {
        NBRE_Map<NBRE_String, ShaderGroupSetPtr>::iterator i = mGroupSetMap.find(groupType);
        if (i != mGroupSetMap.end())
        {
            i->second->mCurrentGroup.reset();
        }
        ++mLatestUpdated;
        return PAL_Ok;
    }

    NBRE_Map<NBRE_String, ShaderGroupSetPtr>::const_iterator i = mGroupSetMap.find(groupType);
    if (i != mGroupSetMap.end())
    {
        const ShaderGroupSetPtr& groupSetPtr = i->second;
        NBRE_Map<NBRE_String, NBRE_IShaderGroupPtr>::const_iterator j = groupSetPtr->mGroupMap.find(groupName);
        if (j != groupSetPtr->mGroupMap.end())
        {
            groupSetPtr->mCurrentGroup = j->second;
        }
        else
        {
            return PAL_ErrNotFound;
        }
    }
    else
    {
        return PAL_ErrNotFound;
    }
    ++mLatestUpdated;

    return PAL_Ok;
}

PAL_Error
NBRE_ShaderManager::RemoveGroup(const NBRE_String& groupID)
{
    NBRE_String groupType = ParseGroupType(groupID);
    NBRE_String groupName = ParseGroupName(groupID);

    NBRE_Map<NBRE_String, ShaderGroupSetPtr>::iterator i = mGroupSetMap.find(groupType);
    if (i != mGroupSetMap.end())
    {
        ShaderGroupSetPtr& groupSetPtr = i->second;
        NBRE_Map<NBRE_String, NBRE_IShaderGroupPtr>::iterator j = groupSetPtr->mGroupMap.find(groupName);

        if (j != groupSetPtr->mGroupMap.end())
        {
            if (j->second == groupSetPtr->mCurrentGroup)
            {
                return PAL_ErrNotReady;
            }
            else
            {
                groupSetPtr->mGroupMap.erase(j);
            }
        }
        else
        {
            return PAL_ErrNotFound;
        }

        if (i->second->mGroupMap.empty())
        {
            mGroupSetMap.erase(i);
        }
    }
    else
    {
        return PAL_ErrNotFound;
    }

    return PAL_Ok;
}

const NBRE_ShaderPtr&
NBRE_ShaderManager::GetDefaultShader() const
{
    return mDefaultShader;
}

NBRE_ShaderTimeStamp
NBRE_ShaderManager::LatestUpdated() const
{
    return mLatestUpdated;
}
