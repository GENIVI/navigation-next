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
   @file         PinMaterialParser.cpp
   @defgroup     nbmap
*/
/*
    (C) Copyright 2013 by TeleCommunications Systems, Inc.

   The information contained herein is confidential, proprietary to
   TeleCommunication Systems, Inc., and considered a trade secret as defined
   in section 499C of the penal code of the State of California. Use of this
   information by anyone other than authorized employees of TeleCommunication
   Systems is granted only under a written non-disclosure agreement, expressly
   prescribing the scope and manner of such use.
--------------------------------------------------------------------------*/

/*! @{ */
#include "PinMaterialParser.h"
#include "NBMChunk.h"
#include "tinyxml.h"

using namespace nbmap;


/*! PinMaterialIndex maps a pin type to material id which can be found in the material
    file. */
struct PinMaterialIndex
{
    PinType     type;
    const char* materialId;
};

static struct PinMaterialIndex materialIndexes[] = {
    {PT_COMMON, "common-pin"},
    {PT_START_FLAG, "start-flag"},
    {PT_END_FLAG, "end-flag"},
    {PT_MINOR_INCIDENT, "incident-yellow"},
    {PT_MAJOR_INCIDENT, "incident-orange"},
    {PT_SEVERE_INCIDENT, "incident-red"},
    {PT_INVALID, NULL}
};

// Implementation of PinMaterialParser.

/* See description in header file. */
PinMaterialParser::PinMaterialParser(const char* path)
{
    LoadPinTypeMapping();
    LoadFromFile(path);
}

/* See description in header file. */
PinMaterialParser::~PinMaterialParser()
{
}

/* See description in header file. */
void PinMaterialParser::LoadFromFile(const char* path)
{
    ResetCachedMaterials();

    if (!path)
    {
        PAL_TestLog(PAL_LogBuffer_1, PAL_LogLevelInfo,
                    "%s: failed to load from file, path is empty!\n",
                    __FUNCTION__);
        return;
    }

    TiXmlDocument xmlDoc;
    if (!xmlDoc.LoadFile(path))
    {
        PAL_TestLog(PAL_LogBuffer_1, PAL_LogLevelInfo,
                    "%s: failed to load from file: %s",
                    __FUNCTION__, path);
        return;
    }

    TiXmlElement* root = xmlDoc.RootElement();
    if (!root)
    {
        PAL_TestLog(PAL_LogBuffer_1, PAL_LogLevelInfo,
                    "%s: failed to load from file: %s, can't find root\n",
                    __FUNCTION__, path);
        return;
    }

    TiXmlElement* libMaterials = root->FirstChildElement("library_materials");
    if (!libMaterials)
    {
        PAL_TestLog(PAL_LogBuffer_1, PAL_LogLevelInfo,
                    "%s: failed to load from file: %s, can't find libMaterials\n",
                    __FUNCTION__, path);
        return;
    }

    TiXmlElement* material = libMaterials->FirstChildElement("material");
    if (!material)
    {
        PAL_TestLog(PAL_LogBuffer_1, PAL_LogLevelInfo,
                    "%s: failed to load from file: %s, can't find any material.\n",
                    __FUNCTION__, path);
        return ;
    }

    while (material) {
        PinMaterial pinMaterial(material);
        m_cachedMaterials.insert(make_pair(pinMaterial.m_id, material));
        material = material->NextSiblingElement("material");
    }

    m_valid = !m_cachedMaterials.empty();
}

/* See description in header file. */
bool PinMaterialParser::IsValid()
{
    return m_valid;
}

/* See description in header file. */
NB_Error
PinMaterialParser::GetPinMaterialIndex(PinType type,
                                       uint16& pinIndex)
{
    // If a valid configuration file has been parsed, return value from parser,
    // or just them to NBM_INVALIDATE_INDEX.
    map<PinType, string>::iterator typeIter = m_typeMapping.find(type);
    if (typeIter == m_typeMapping.end() || typeIter->second.empty())
    {
        return NE_UNEXPECTED;
    }

    map<string, PinMaterial>::iterator materialIter = m_cachedMaterials.find(typeIter->second);
    if (materialIter == m_cachedMaterials.end())
    {
        return NE_NOENT;
    }

    pinIndex = materialIter->second.m_index;
    return NE_OK;
}

/* See description in header file. */
void PinMaterialParser::ResetCachedMaterials()
{
    m_valid = false;
    m_cachedMaterials.clear();
}

/* See description in header file. */
void PinMaterialParser::LoadPinTypeMapping()
{
    // Add each PinType and find indexes for them.

    struct PinMaterialIndex* indexPtr = materialIndexes;
    while (indexPtr->type != PT_INVALID && indexPtr->materialId)
    {
        m_typeMapping.insert(make_pair(indexPtr->type, indexPtr->materialId));
        indexPtr++;
    }
}

/* See description in header file. */
PinMaterial::PinMaterial(const TiXmlElement* element)
        : m_index(NBM_INVALIDATE_INDEX)
{
    if (element)
    {
        const char* value = element->Attribute("index");
        if (value)
        {
            m_index = static_cast<uint16>(atoi(value)) | 0x8000; // Set as external material.
        }

        value = element->Attribute("name");
        if (value)
        {
            m_name.assign(value);
        }

        value = element->Attribute("id");
        if (value)
        {
            m_id.assign(value);
        }
    }
}

/* See description in header file. */
PinMaterial::PinMaterial(const PinMaterial& material)
        : m_name(material.m_name),
          m_id(material.m_id),
          m_index(material.m_index)
{
}


/* See description in header file. */
PinMaterial::~PinMaterial()
{
}

/*! @} */
