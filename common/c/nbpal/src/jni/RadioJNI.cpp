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

    @file     RadioJNI.cpp
    @date     3/16/2012
*/
/*
    See file description in header file.

    (C) Copyright 2012 by Networks In Motion, Inc.

    The information contained herein is confidential, proprietary
    to Networks In Motion, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#include "RadioJNI.h"

extern "C" {
#include "paldebuglog.h"
}

struct fields_t {
    jclass clazz;
    jmethodID get_gsm_network_info;
    jmethodID get_cdma_network_info;
};
static fields_t fields;

RadioJNI::RadioJNI(JNI_Config* conf) :
    BaseJNI(conf->env, conf->jniRadio)
{

}

RadioJNI::~RadioJNI() {

}

PAL_RadioGSMInfo* RadioJNI::getGSMNetworkInfo() {
    jint p = this->callIntMethod(fields.clazz, fields.get_gsm_network_info);

    if (p != 0) {
        return (PAL_RadioGSMInfo*) p;
    } else {
        return NULL;
    }
}

int register_RadioProxy(JNIEnv *env) {
    jclass clazz = env->FindClass("com/navbuilder/pal/ndk/radio/Radio");
    if (clazz != NULL) {
        fields.clazz = (jclass) env->NewGlobalRef(clazz);
        if (fields.clazz == NULL) {
            DEBUGLOG(LOG_SS_NB_PAL, LOG_SEV_CRITICAL, ("Can't create global ref for com/navbuilder/pal/ndk/radio/Radio"));
            return JNI_ERR;
        }

        /* The local reference is no longer useful */
        env->DeleteLocalRef(clazz);
    } else {
        DEBUGLOG(LOG_SS_NB_PAL, LOG_SEV_CRITICAL, ("Can't find com/navbuilder/pal/ndk/radio/Radio"));
        return JNI_ERR;
    }

    fields.get_gsm_network_info = env->GetStaticMethodID(fields.clazz,
            "getGSMNetworkInfo", "(Ljava/lang/Object;)I");
    if (fields.get_gsm_network_info == NULL) {
        DEBUGLOG(LOG_SS_NB_PAL, LOG_SEV_CRITICAL, ("Can't find Radio.getGSMNetworkInfo()"));
        return JNI_ERR;
    }

/*    fields.get_cdma_network_info = env->GetStaticMethodID(fields.clazz,
            "getCDMANetworkInfo", "(Ljava/lang/Object;)Ljava/lang/Object;");
    if (fields.get_gsm_network_info == NULL) {
        DEBUGLOG(LOG_SS_NB_PAL, LOG_SEV_CRITICAL, ("Can't find Radio.getCDMANetworkInfo()"));
        return JNI_ERR;
    }*/

    return JNI_OK;
}
