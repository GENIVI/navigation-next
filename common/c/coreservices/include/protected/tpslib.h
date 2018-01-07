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
 * tpslib.h: created 2004/11/30 by Dima Dorfman.
 *
 * Copyright (C) 2004 Networks In Motion, Inc. All rights reserved.
 *
 * The information contained herein is confidential and proprietary to
 * Networks In Motion, Inc., and is considered a trade secret as
 * defined in section 499C of the California Penal Code. Use of this
 * information by anyone other than authorized employees of Networks
 * In Motion is granted only under a written non-disclosure agreement
 * expressly prescribing the scope and manner of such use.
 */

/*
 * Templated Packet Stream Template Library interface.
 */

#ifndef TPSLIB_H
#define	TPSLIB_H

#include "tpselt.h"
#include "cslexp.h"

#define	TPSLIB_IDLEN	20

typedef uint16 tpstid;

struct tpstpl {
	tpstid id;
	char *name;
	char **attrs;
};

struct tpslib;

/*
 * Get a template that can be used to pack this element. The returned
 * pointer is valid for the life of the library.
 */
CSL_DEC struct tpstpl *tpslib_byte(struct tpslib *tl, tpselt te);

/*
 * Get the template with the specified ID number. The returned pointer
 * is valid for the life of the library.
 */
CSL_DEC struct tpstpl *tpslib_byid(struct tpslib *tl, tpstid id);

/*
 * Deallocate the template library. All tpstpl pointers that may have
 * been returned become invalid.
 */
CSL_DEC void tpslib_dealloc(struct tpslib *tl);

/*
 * Return the capabilites dictionary.
 */
CSL_DEC struct CSL_Dictionary *tpslib_getcaps(struct tpslib *tl);

/*
 * Fill out with the library ID.
 */
CSL_DEC void tpslib_getid(struct tpslib *tl, char *out);

/*
 * Interfaces to implementations.
 *
 * XXX: Exactly one of these must be linked in.
 */
CSL_DEC struct tpslib *tpslib_preload(const char *text, size_t textlen);

#endif
