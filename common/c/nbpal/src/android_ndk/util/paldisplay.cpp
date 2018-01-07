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
 * (C) Copyright 2013 by TeleCommunication Systems, Inc.
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

#include "paldisplay.h"
#include "palconfig.h"
#include <jni.h>

#define DEFAULT_DPI           (160.0)
static float deviceDpi = 160.0f;

extern JNIEnv* getJNIEnv();

struct fields_display
{
    jclass clazz;
    jfieldID  displayDPI;
    jfieldID  displayWidth;
    jfieldID  displayHeight;
};
static fields_display fields;

static void release_fields(JNIEnv* env)
{
    if (fields.clazz)
    {
        env->DeleteGlobalRef(fields.clazz);
        fields.clazz = NULL;
    }
}

PAL_DEC PAL_Error PAL_DisplayGetInfo(
    PAL_DisplayIndex displayIndex,  /*<! Display index */
    uint32* width,                  /*<! Output width in pixels */
    uint32* height,                 /*<! Output height in pixels */
    uint32* dpi                     /*<! Output pixels per inch */
                                     )
{
    PAL_Error ret = PAL_Failed;
    *width = *height = *dpi = -1;
    JNIEnv* env = getJNIEnv();
    if(env == NULL)
    {
        return ret;
    }

    switch (displayIndex)
    {
        case PD_Main:
            {
                *width = (uint32)env->GetStaticIntField(fields.clazz, fields.displayWidth);
                *height = (uint32)env->GetStaticIntField(fields.clazz, fields.displayHeight);
                *dpi = (uint32)deviceDpi;
                ret = PAL_Ok;
            }
            break;
        case PD_External:
        default:
            {
                ret = PAL_ErrUnsupported;
            }
            break;
    }
    return ret;
}

PAL_DEF double PAL_DisplayGetDensity()
{
    uint32 width = 0;
    uint32 height = 0;
    uint32 dpi = 0;
    PAL_DisplayGetInfo(PD_Main, &width, &height, &dpi);
    return (double)dpi / DEFAULT_DPI;
}

PAL_DEF PAL_Error PAL_DisplayInitialize(PAL_Instance *pal)
{
    PAL_Config config = {0};
    PAL_ConfigGet(pal, &config);
    float dpi = config.dpi;
    if (dpi > 0)
    {
        deviceDpi = dpi;
    }
    return PAL_Ok;
}

/* Just stubs for success compilation */
PAL_DEF uint32 PAL_DisplayGetWidth()
{
    return 0;
}

PAL_DEF uint32 PAL_DisplayGetHeight()
{
    return 0;
}

int register_dispalyandroid(JNIEnv* env)
{
    uint32 errorLine = 0;

    jclass clazz = env->FindClass("com/navbuilder/pal/android/ndk/PalDisplay");
    if (clazz != NULL)
    {
        fields.clazz = (jclass) env->NewGlobalRef(clazz);
        if (fields.clazz == NULL)
        {
            errorLine = __LINE__;
            goto HandleError;
        }

        /* The local reference is no longer useful */
        env->DeleteLocalRef(clazz);
    }
    else
    {
        errorLine = __LINE__;
        goto HandleError;
    }

    fields.displayDPI = env->GetStaticFieldID(fields.clazz, "mDPI", "I");
    if (fields.displayDPI == NULL)
    {
        errorLine = __LINE__;
        goto HandleError;
    }

    fields.displayWidth = env->GetStaticFieldID(fields.clazz, "mWidth", "I");
    if (fields.displayWidth == NULL)
    {
        errorLine = __LINE__;
        goto HandleError;
    }

    fields.displayHeight = env->GetStaticFieldID(fields.clazz, "mHeight", "I");
    if (fields.displayHeight == NULL)
    {
        errorLine = __LINE__;
        goto HandleError;
    }

    return JNI_OK;

HandleError:
    release_fields(env);
    return JNI_ERR;
}

int unregister_displayandroid(JNIEnv* env)
{
    release_fields(env);
    return JNI_OK;
}

