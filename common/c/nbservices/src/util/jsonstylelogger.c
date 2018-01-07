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
    @file jsonstylelogger.cpp
    @addtogroup locationtoolkit
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
---------------------------------------------------------------------------*/
#include "jsonstylelogger.h"
#include "palstdlib.h"

static int log_indent = 0;
static char* log_buff = NULL;

static void MakeIndent(char* buff)
{
    int i=0;
    
    if(log_indent == 0)
    {
        return;
    }
    switch(log_indent)
    {
    case 1:
        strcat(buff, "    ");
        break;
    case 2:
        strcat(buff, "        ");
        break;
    case 3:
        strcat(buff, "            ");
        break;
    case 4:
        strcat(buff, "                ");
        break;
    case 5:
        strcat(buff, "                    ");
        break;
    case 6:
        strcat(buff, "                        ");
        break;
    default:
        for(i=0; i<log_indent; ++ i)
        {
            strcat(buff, "    ");
        }
        break;
    }

}

NB_DEF const char*
NB_JsonStyleLoggerBuffer()
{
    return log_buff;
}

NB_DEF void
NB_JsonStyleLoggerDestory()
{
    if(log_buff)
    {
        nsl_free(log_buff);
        log_buff = NULL;
    }
}

#define MAKE_BUFF     char buff[1024]; nsl_memset(buff, 0, sizeof(buff))
#define USE_BUFF     NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelInfo, buff); strcat(log_buff, buff);

NB_DEF void
NB_JsonStyleLoggerBegin()
{
    MAKE_BUFF;
    if(!log_buff)
    {
        int buffSize = 10*1024;
        log_buff = (char*)nsl_malloc(buffSize);
        nsl_memset(log_buff, 0, buffSize);
    }

    log_indent = 0;

    MakeIndent(buff);
    strcat(buff, "{\n");
    log_indent++;

    USE_BUFF;
}

NB_DEF void
NB_JsonStyleLoggerEnd()
{
    MAKE_BUFF;

    log_indent--;
    MakeIndent(buff);
    strcat(buff, "}\n");

    USE_BUFF;
}

NB_DEF void
NB_JsonStyleLoggerStartObject(const char *objectName)
{
    MAKE_BUFF;

    MakeIndent(buff);
    if(objectName)
    {
        strcat(buff, "\"");
        strcat(buff, objectName);
        strcat(buff, "\":");
    }
    strcat(buff, "{\n");
    log_indent++;

    USE_BUFF;
}

NB_DEF void
NB_JsonStyleLoggerEndObject()
{
    MAKE_BUFF;

    log_indent--;

    MakeIndent(buff);
    strcat(buff, "},\n");

    USE_BUFF;
}

NB_DEF void
NB_JsonStyleLoggerStartArray(const char *arrayName)
{
    MAKE_BUFF;

    MakeIndent(buff);
    if(arrayName)
    {
        strcat(buff, "\"");
        strcat(buff, arrayName);
        strcat(buff, "\":");
    }
    strcat(buff, "[\n");
    log_indent++;

    USE_BUFF;
}

NB_DEF void
NB_JsonStyleLoggerEndArray()
{
    MAKE_BUFF;

    log_indent--;
    MakeIndent(buff);
    strcat(buff, "],\n");

    USE_BUFF;
}

NB_DEF void
NB_JsonStyleLoggerLogString(const char *name, const char *value)
{
    if(name && value)
    {
        MAKE_BUFF;

        MakeIndent(buff);

        strcat(buff, "\"");
        strcat(buff, name);
        strcat(buff, "\": \"");
        strcat(buff, value);
        strcat(buff, "\",\n");

        USE_BUFF;
    }
}


NB_DEF void
NB_JsonStyleLoggerLogBoolean(const char *name, nb_boolean value)
{
    MAKE_BUFF;

    MakeIndent(buff);

    strcat(buff, "\"");
    strcat(buff, name);
    strcat(buff, "\": ");
    strcat(buff, value?"true":"false");
    strcat(buff, ",\n");

    USE_BUFF;
}


NB_DEF void
NB_JsonStyleLoggerLogDouble(const char *name, double value)
{
    MAKE_BUFF;

    MakeIndent(buff);

    strcat(buff, "\"");
    strcat(buff, name);
    strcat(buff, "\": ");
    char str[64];
    sprintf(str, "%f", value);
    strcat(buff, str);
    strcat(buff, ",\n");

    USE_BUFF;
}

NB_DEF void
NB_JsonStyleLoggerLogInt(const char *name, int value)
{
    MAKE_BUFF;

    MakeIndent(buff);

    strcat(buff, "\"");
    strcat(buff, name);
    strcat(buff, "\": ");
    char str[64];
    sprintf(str, "%d", value);
    strcat(buff, str);
    strcat(buff, ",\n");

    USE_BUFF;
}

