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

    @file nbreshadermanager.h
*/
/*
    (C) Copyright 2012 by Networks In Motion, Inc.

    The information contained herein is confidential, proprietary
    to Networks In Motion, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
#ifndef _NBRE_SHADER_MANAGER_H_
#define _NBRE_SHADER_MANAGER_H_

#include "nbreishadergroup.h"
#include "nbrecontext.h"

/** \addtogroup NBRE_Resource
*  @{
*/
/** Shader manager is a shader resource look up table.
    It may contain several group sets. Each set may contain several groups which are parallel.
    Group name is defined to SetName\GroupName.For example, if there are two groups which are day/night 
    common material,then the group names may be defined to COMMON_MATERIAL\DAY, COMMON_MATRRIAL\NIGHT. 
    Shader group can be used to implement different themes.
*/

typedef uint32 NBRE_ShaderTimeStamp;

class NBRE_ShaderManager
{
public:
    NBRE_ShaderManager();
    ~NBRE_ShaderManager();

public:
    /*! Add a shader group to shader manager.
    @param groupID follows SetName\GroupName format.
    @param group is the group want to add.
    @return PAL error code.
    */
    PAL_Error AddGroup(const NBRE_String& groupID, const NBRE_IShaderGroupPtr& group);
    /*! Select current shader group in their own set, all shader operation of this set will only affect current group.
    @param groupID follows SetName\GroupName format.
    @return PAL error code.
    */
    PAL_Error SetCurrentGroup(const NBRE_String& groupID);
    /*! Remove a shader group from shader manager.
    @param groupID follows SetName\GroupName format.
    @return PAL error code.
    */
    PAL_Error RemoveGroup(const NBRE_String& groupID);

    /*! Get shader from current group.
    @param shaderID follows SetName\ShaderName format
    @return the selected shader.
    */
    const NBRE_ShaderPtr& GetShader(const NBRE_String& shaderID) const;
    /*! Replace or add a new shader in current group.
    @param shaderID follows SetName\ShaderName format.
    @param shader the shader will be added or replaced.
    */
    void SetShader(const NBRE_String& shaderID, NBRE_ShaderPtr shader);
    /*! Remove a shader in current group.
    @param shaderID follows SetName\ShaderName format.
    */
    void RemoveShader(const NBRE_String& shaderID);

    const NBRE_ShaderPtr& GetDefaultShader() const;
    
    ///return latest time stamp
    NBRE_ShaderTimeStamp LatestUpdated() const;

private:
    DISABLE_COPY_AND_ASSIGN(NBRE_ShaderManager);

private:

    class ShaderGroupSet
    {
    friend class NBRE_ShaderManager;

    public:
        ShaderGroupSet(const NBRE_String& name):mName(name){}
        ~ShaderGroupSet(){}

    private:
        NBRE_Map<NBRE_String, NBRE_IShaderGroupPtr> mGroupMap;
        NBRE_String mName;
        NBRE_IShaderGroupPtr mCurrentGroup;
    };
    typedef shared_ptr<ShaderGroupSet> ShaderGroupSetPtr;

    NBRE_Map<NBRE_String, ShaderGroupSetPtr> mGroupSetMap;

    NBRE_ShaderPtr mDefaultShader;

    ///It's a timestamp, mark update
    ///TODO: may be we need add timestamp in every group set.
    NBRE_ShaderTimeStamp mLatestUpdated;
};

/** @} */
#endif
