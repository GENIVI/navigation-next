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

/*****************************************************************/
/*                                                               */
/* (C) Copyright 2005 by Networks In Motion, Inc.                */
/*                                                               */
/* The information contained herein is confidential, proprietary */
/* to Networks In Motion, Inc., and considered a trade secret as */
/* defined in section 499C of the penal code of the State of     */
/* California. Use of this information by anyone other than      */
/* authorized employees of Networks In Motion is granted only    */
/* under a written non-disclosure agreement, expressly           */
/* prescribing the scope and manner of such use.                 */
/*                                                               */
/*****************************************************************/

/*-
 *  tpsdebug.c: created 2005/01/18 by Mark Goddard.
 */

#include "tpsdebug.h"

#ifndef NDEBUG

#include "vec.h"
#include "cslutil.h"
#include "paldebuglog.h"


void catattr(char* buf, tpselt te, const char* name)
{
    const char* s = te_getattrc(te, name);
    int         l = 0;
    int         n = 0;
    char*       data = 0;
    size_t      size = 0;
    size_t      p = 0;
    char        f[64] = { 0 };
    double      d = 0.0;

    if (s != NULL)
    {
        l = nsl_strlen(s);

        for (n = 0; n < l; n++)
        {
            if (s[n] < 32 || s[n] > 126)
            {
                s = NULL;
                break;
            }
        }
    }

    if (s != NULL)
    {
        nsl_strcat(buf, s);
        nsl_strcat(buf, " ");
    }

    l = nsl_strlen(buf);

    if (te_getattr(te, name, &data, &size))
    {
        if (size <= 48)
        {
            for (p=0; p < size; p++)
            {
                nsl_sprintf(buf+l, "%2.2X", (byte) data[p]);
                l = nsl_strlen(buf);
            }
        }
        else
        {
            nsl_sprintf(buf+l, "...%d bytes...", (int)size);
        }
    }

    l = nsl_strlen(buf);

    if (te_getattrf(te, name, &d) || te_getattrd(te, name, &d))
    {
        nsl_doubletostr(d, f, sizeof(f));
    }
    else
    {
        f[0] = 0;
    }

    nsl_sprintf(buf + l, "(%u,%s)", te_getattru(te, name), f);
}

CSL_DEF void 
dumpelt(tpselt te, int indent)
{
    char*  buf = NULL;
    const char* name;
    int n,l;
    struct CSL_Vector* attrs;
    tpselt ce;

    if (te == NULL)
    {
        return;
    }

    buf = nsl_malloc(1024 * 10);

    if (buf == NULL)
    {
        DEBUGLOG(LOG_SS_MEMORY, LOG_SEV_CRITICAL, ("Memory Allocation failed in dumpelt."));
        return;
    }

    name = te_getname(te);

    buf[0] = 0;
    for (n=0; n < indent; n++)
    {
        nsl_strcat(buf,"  ");
    }

    nsl_strcat(buf, name);
    nsl_strcat(buf, ": ");

    attrs = te_getattrs(te);

    if (attrs != NULL)
    {
        l = CSL_VectorGetLength(attrs);

        for (n=0; n < l; n++)
        {
            nsl_strcat(buf, *((const char**) CSL_VectorGetPointer(attrs, n)));
            nsl_strcat(buf, "=\"");

            catattr(buf, te, *((const char**) CSL_VectorGetPointer(attrs, n)));
            nsl_strcat(buf, "\" ");
        }

        CSL_VectorDealloc(attrs);
    }
    else
    {
        nsl_assert("Memory Allocation Failed");
    }

    DEBUGLOG(LOG_SS_TPS, LOG_SEV_INFO, ("%s", buf));

    n = 0;
    while ((ce = te_nextchild(te, &n)) != NULL)
    {
        dumpelt(ce, indent+1);
    }

    if (buf != NULL)
    {
        nsl_free(buf);
    }
}
#else

CSL_DEF void 
dumpelt(tpselt te, int indent)
{
}

#endif
