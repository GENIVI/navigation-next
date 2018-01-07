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

#include "test_nbm_archive.h"
#include "nbgmnbmarchive.h"
#include "nbrefilestream.h"
#include "nbrememorystream.h"
#include "nbrepalholder.h"
#include "utility.h"
#include "nbgmnbmchunkconfig.h"
#include "nbgmnbmchunkfactory.h"
#include "nbgmnbmchunk.h"
#include "nbgmnbmheaderchunk.h"

static uint32 gFileSize = 0;
static PAL_Instance* gPal = NULL;
static PAL_File* gNBMFile = NULL;

NBRE_PalHolder* NBRE_Singleton<NBRE_PalHolder>::mSingleton = new NBRE_PalHolder(CreatePal());

/* The suite initialization function.
 * Opens the temporary file used by the tests.
 * Returns zero on success, non-zero otherwise.
 * Case name is mesh_test.
 */
static int init_archive_test(void)
{
    gPal = NBRE_PalHolder::GetSingleton().GetPalInstance();
    PAL_FileGetSize(gPal, "map_tile.nbm", &gFileSize);

    return PAL_FileOpen(gPal, "map_tile.nbm", PFM_Read, &gNBMFile);
}

/* The suite cleanup function.
 * Closes the temporary file used by the tests.
 * Returns zero on success, non-zero otherwise.
 */
static int clean_archive_test(void)
{
    PAL_Destroy(gPal);
    PAL_FileClose(gNBMFile);
    return 0;
}

static NBGM_NBMConfig*
CreateAttrConfig(const NBRE_String& name, NBGM_NBMValueType valueType, const NBRE_String& description)
{
    NBGM_NBMConfig* config = NBRE_NEW NBGM_NBMConfig();
    config->mName = name;
    config->mType = NBGM_NBM_CT_ATTRIBUTE;
    config->mAttributeConfig = NBRE_NEW NBGM_NBMAttributeConfig();
    config->mAttributeConfig->valueType = valueType;
    config->mAttributeConfig->description = NBRE_NEW NBRE_String(description);

    return config;
}

static NBGM_NBMConfig*
CreateFieldConfig(const NBRE_String& name)
{
    NBGM_NBMConfig* config = NBRE_NEW NBGM_NBMConfig();
    config->mName = name;
    config->mType = NBGM_NBM_CT_FIELD;
    config->mFieldConfig = NBRE_NEW NBGM_NBMFieldConfig();

    return config;
}

static NBGM_NBMConfig*
CreateGroupConfig(const NBRE_String& name, NBGM_NBMValueType countValueType)
{
    NBGM_NBMConfig* config = NBRE_NEW NBGM_NBMConfig();
    config->mName = name;
    config->mType = NBGM_NBM_CT_GROUP;
    config->mGroupConfig = NBRE_NEW NBGM_NBMGroupConfig();
    config->mGroupConfig->countValueType = countValueType;

    return config;
}

static void
test_archive_create()
{
    NBRE_FileStream fs(gNBMFile, gFileSize, 1024);
    NBGM_NBMArchive ar(fs, FALSE);

    CU_ASSERT(ar.Storing() == FALSE);
    CU_ASSERT(ar.Eof() == FALSE);
    CU_ASSERT(ar.GetPos() == 0);

    ar.Seek(PFSO_Current, gFileSize);
    CU_ASSERT(ar.Eof() == TRUE);
}

static void
test_chunk_info_configuration_create()
{
    NBGM_NBMConfig* infoConfig = NBGM_NBMChunkFactory::CreateChunkInfoConfig();
    CU_ASSERT(infoConfig->mType = NBGM_NBM_CT_FIELD);
    CU_ASSERT(infoConfig->mFieldConfig != NULL);
    CU_ASSERT(infoConfig->mFieldConfig->subConfig->size() == 3);    //id, size, flag

    NBGM_NBMConfig* attrConfig = infoConfig->mFieldConfig->subConfig->operator[](0);
    CU_ASSERT(attrConfig->mType == NBGM_NBM_CT_ATTRIBUTE);
    CU_ASSERT(attrConfig->mName == "Chunk Identifier");
    CU_ASSERT(attrConfig->mAttributeConfig->valueType == NBRE_DVT_UINT32);
    CU_ASSERT(*attrConfig->mAttributeConfig->description == "");

    attrConfig = infoConfig->mFieldConfig->subConfig->operator[](1);
    CU_ASSERT(attrConfig->mType == NBGM_NBM_CT_ATTRIBUTE);
    CU_ASSERT(attrConfig->mName == "Chunk Size");
    CU_ASSERT(attrConfig->mAttributeConfig->valueType == NBRE_DVT_UINT32);
    CU_ASSERT(*attrConfig->mAttributeConfig->description == "");

    attrConfig = infoConfig->mFieldConfig->subConfig->operator[](2);
    CU_ASSERT(attrConfig->mType == NBGM_NBM_CT_ATTRIBUTE);
    CU_ASSERT(attrConfig->mName == "Flags");
    CU_ASSERT(attrConfig->mAttributeConfig->valueType == NBRE_DVT_UINT16);
    CU_ASSERT(*attrConfig->mAttributeConfig->description == "");

    delete infoConfig;
}
static void test_nbm_field()
{
    NBRE_FileStream fs(gNBMFile, gFileSize, 1024);
    NBGM_NBMArchive ar(fs, FALSE);
    ar.Seek(PFSO_Start, 0);
 
    NBGM_NBMField chunkInfoField;
    NBGM_NBMField nbmheaderField;

    NBGM_NBMChunkInfo chunkInfo;
    chunkInfo.mIdentifier = NBM_ID_NBMF;

    NBGM_NBMConfig* infoConfig = NBGM_NBMChunkFactory::CreateChunkInfoConfig();
    ar.SetCurrentChunkConfig(*infoConfig);
    ar>>chunkInfoField;
    delete infoConfig;

    NBGM_NBMConfig* nbmFileConfig = NBGM_NBMChunkFactory::CreateChunkConfig(chunkInfo);
    ar.SetCurrentChunkConfig(*nbmFileConfig);
    ar>>nbmheaderField;
    delete nbmFileConfig;

    CU_ASSERT(chunkInfoField.mName == "Chunk Info");
    CU_ASSERT(chunkInfoField.mGroups.size() == 0);
    CU_ASSERT(chunkInfoField.mSubFields.size() == 0);
    CU_ASSERT(chunkInfoField.mAttributes.size() == 3);

    NBGM_NBMAttribute* attr = chunkInfoField.mAttributes["Chunk Identifier"];
    CU_ASSERT(attr != NULL);
    attr = chunkInfoField.mAttributes["Chunk Size"];
    CU_ASSERT(attr != NULL);
    attr = chunkInfoField.mAttributes["Flags"];
    CU_ASSERT(attr != NULL);

    CU_ASSERT(nbmheaderField.mName == "File Header");
    CU_ASSERT(nbmheaderField.mGroups.size() == 0);
    CU_ASSERT(nbmheaderField.mSubFields.size() == 0);
    CU_ASSERT(nbmheaderField.mAttributes.size() == 2);
    attr = nbmheaderField.mAttributes["File Version"];
    CU_ASSERT(attr != NULL);
    attr = nbmheaderField.mAttributes["Reference Center"];
    CU_ASSERT(attr != NULL);
}

static void
test_chunkinfo_read()
{
    NBRE_FileStream fs(gNBMFile, gFileSize, 1024);
    NBGM_NBMArchive ar(fs, FALSE);
    ar.Seek(PFSO_Start, 0);

    NBGM_NBMChunkInfo chunkInfo;
    chunkInfo.Serialize(ar);

    NBGM_NBMChunk* chunk = NBGM_NBMChunkFactory::CreateChunk(chunkInfo);
    CU_ASSERT(chunkInfo.mName == NBM_NAME_NBMF);
    CU_ASSERT(chunkInfo.mIdentifier == NBM_ID_NBMF);
    CU_ASSERT(chunkInfo.mFlag == 0);
    CU_ASSERT(chunkInfo.mOffset == 0);
    CU_ASSERT(chunkInfo.mSize == 12);
    
    delete chunk;
}

static void
test_nbm_field_serialize()
{
    NBGM_NBMConfig* fileConfig = CreateFieldConfig("File Config");

    NBGM_NBMConfig* attrConfig = CreateAttrConfig("attr00", NBRE_DVT_UINT32, "0");
    fileConfig->mFieldConfig->AppendConfig(attrConfig);
    attrConfig = CreateAttrConfig("attr01", NBRE_DVT_UINT32, "0");
    fileConfig->mFieldConfig->AppendConfig(attrConfig);
    attrConfig = CreateAttrConfig("attr02", NBRE_DVT_UINT32, "0");
    fileConfig->mFieldConfig->AppendConfig(attrConfig);

    NBGM_NBMConfig* subfieldConfig = CreateFieldConfig("Sub Field In Group");
    attrConfig = CreateAttrConfig("attr10", NBRE_DVT_UINT16, "1");
    subfieldConfig->mFieldConfig->AppendConfig(attrConfig);
    attrConfig = CreateAttrConfig("attr11", NBRE_DVT_UINT16, "1");
    subfieldConfig->mFieldConfig->AppendConfig(attrConfig);

    NBGM_NBMConfig* groupConfig = CreateGroupConfig("Sub Group", NBRE_DVT_UINT16);
    groupConfig->mGroupConfig->fieldConfig.AppendConfig(subfieldConfig);

    fileConfig->mFieldConfig->AppendConfig(groupConfig);

    NBGM_NBMConfig* subfield2 = CreateFieldConfig("Sub Field");
    attrConfig = CreateAttrConfig("attr20", NBRE_DVT_UINT16, "2");
    subfield2->mFieldConfig->AppendConfig(attrConfig);
    attrConfig = CreateAttrConfig("attr21", NBRE_DVT_UINT16, "2");
    subfield2->mFieldConfig->AppendConfig(attrConfig);
    attrConfig = CreateAttrConfig("attr22", NBRE_DVT_UINT16, "2");
    subfield2->mFieldConfig->AppendConfig(attrConfig);
    attrConfig = CreateAttrConfig("attr23", NBRE_DVT_UINT16, "2");
    subfield2->mFieldConfig->AppendConfig(attrConfig);
    fileConfig->mFieldConfig->AppendConfig(subfield2);

    uint8* srcData = NBRE_NEW uint8[1024*1024];
    uint32 srcDataSize = 1024*1024;
    nsl_memset(srcData, 0x0f, srcDataSize);
    NBRE_MemoryStream ms(srcData, srcDataSize, FALSE, 1024);

    NBGM_NBMArchive ar(ms, FALSE);
    ar.Seek(PFSO_Start, 0);
    
    NBGM_NBMField field;
    ar.SetCurrentChunkConfig(*fileConfig);
    ar>>field;

    CU_ASSERT(field.mAttributes.size() == 3);
    CU_ASSERT(field.mGroups.size() == 1);
    CU_ASSERT(field.mSubFields.size() == 1);

    CU_ASSERT(field.mAttributes["attr00"] != NULL);
    CU_ASSERT(field.mAttributes["attr01"] != NULL);
    CU_ASSERT(field.mAttributes["attr02"] != NULL);


    NBGM_NBMGroup* subGroup = field.mGroups["Sub Group"];
    CU_ASSERT(subGroup != NULL);
    CU_ASSERT(subGroup->mFieldArray.size() == 0x0f0f);

    NBGM_NBMField* subGroupField = subGroup->mFieldArray[0];
    CU_ASSERT(subGroupField->mName == "Sub Field In Group");
    CU_ASSERT(subGroupField->mAttributes.size() == 2);
    CU_ASSERT(subGroupField->mAttributes["attr10"] != NULL);
    CU_ASSERT(subGroupField->mAttributes["attr11"] != NULL);

    NBGM_NBMField* subField = field.mSubFields["Sub Field"];
    CU_ASSERT(subField->mAttributes.size() == 4);
    CU_ASSERT(subField->mGroups.size() == 0);
    CU_ASSERT(subField->mSubFields.size() == 0);

    CU_ASSERT(subField->mAttributes["attr20"] != NULL);
    CU_ASSERT(subField->mAttributes["attr21"] != NULL);
    CU_ASSERT(subField->mAttributes["attr22"] != NULL);
    CU_ASSERT(subField->mAttributes["attr23"] != NULL);

    NBRE_DELETE[] srcData;
    NBRE_DELETE fileConfig;
}

NBGM_NBMArchiveTest::NBGM_NBMArchiveTest()
    : NBRE_TestSuite("map_data_test", init_archive_test, clean_archive_test)
{
    AddTestCase(new NBRE_TestCase("test_archive_create", test_archive_create));
    AddTestCase(new NBRE_TestCase("test_chunk_info_configuration_create", test_chunk_info_configuration_create));
    AddTestCase(new NBRE_TestCase("test_nbm_field", test_nbm_field));
    AddTestCase(new NBRE_TestCase("test_chunkinfo_read", test_chunkinfo_read));
    AddTestCase(new NBRE_TestCase("test_nbm_field_serialize", test_nbm_field_serialize));
}
