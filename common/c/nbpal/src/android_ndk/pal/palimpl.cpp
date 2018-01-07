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
 * (C) Copyright 2014 by TeleCommunication Systems, Inc.
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
#include <jni.h>
#include <string>
#include "palimpl.h"
#include "paltestlog.h"

extern JNIEnv* getJNIEnv();

typedef struct fields_t {
    jclass uuidGeneratorClazz;
    jmethodID midGenerateUUID;
} fields;
static fields generateUUIDFields;

void release_palcommon_fields()
{
    JNIEnv* env = getJNIEnv();
    if (generateUUIDFields.uuidGeneratorClazz && env)
    {
        env->DeleteGlobalRef(generateUUIDFields.uuidGeneratorClazz);
        generateUUIDFields.uuidGeneratorClazz = NULL;
    }
}

int unregister_palcommon(JNIEnv* env)
{
    release_palcommon_fields();
    return JNI_OK;
}

int register_palcommon(JNIEnv* env)
{
    if(!env)
    {
        return JNI_ERR;
    }

    nsl_memset(&generateUUIDFields, 0, sizeof(generateUUIDFields));
    jclass clazz = env->FindClass("com/navbuilder/pal/android/ndk/UUIDGenerator");
    if (clazz != NULL)
    {
        generateUUIDFields.uuidGeneratorClazz = (jclass) env->NewGlobalRef(clazz);
        if (generateUUIDFields.uuidGeneratorClazz == NULL)
        {
            return JNI_ERR;
        }

        env->DeleteLocalRef(clazz);
    }
    else
    {
        return JNI_ERR;
    }


    generateUUIDFields.midGenerateUUID = env->GetStaticMethodID(generateUUIDFields.uuidGeneratorClazz, "generateUUIDBytes", "()[B");
    if (generateUUIDFields.midGenerateUUID == NULL)
    {
        goto HandleError;
    }
    return JNI_OK;

HandleError:
    release_palcommon_fields();
    return JNI_ERR;
}

PAL_DEC PAL_Error PAL_GenerateUUID(PAL_Instance* pal, uint8* buffer, uint32 bufferSize, uint32* bytesRead)
{
    JNIEnv* env = getJNIEnv();
    if(!env)
    {
        return PAL_Failed;
    }

    jbyteArray array = (jbyteArray)env->CallStaticObjectMethod(generateUUIDFields.uuidGeneratorClazz, generateUUIDFields.midGenerateUUID);

    if(array)
    {
        jbyte* byteData = env->GetByteArrayElements(array, NULL);
        jsize count = env->GetArrayLength(array);
        PAL_TestLog(PAL_LogBuffer_0, PAL_LogLevelInfo, "PAL_GenerateUUID successfully");
        if(count > bufferSize)
        {
            nsl_memcpy(buffer, byteData, bufferSize);
        }
        else
        {
            nsl_memcpy(buffer, byteData, count);
        }
        *bytesRead = count;
        return PAL_Ok;
    }
    PAL_TestLog(PAL_LogBuffer_0, PAL_LogLevelInfo, "PAL_GenerateUUID = NULL.");
    return PAL_Failed;
}
