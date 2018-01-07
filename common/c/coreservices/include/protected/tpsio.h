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
 * tpsio.h: created 2004/11/30 by Dima Dorfman.
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
 * Templated packet stream packing and unpacking.
 */

#ifndef TPSIO_H
#define	TPSIO_H

#include "cslexp.h"

struct tpslib;
struct tps_parsestate;

enum tpsio_status { TIO_READY, TIO_NOMEM, TIO_NOTPL,
		    TIO_MALTPL, TIO_MALDOC, TIO_MALTREE };

/*
 * Pack elements into a TPS document. The reader function is called as
 * parts of the document become available. The pointer supplied to the
 * reader function is only valid for the duration of that call.
 */
typedef enum tpsio_status tpspackreader(void *arg, char *doc, size_t doclen);
CSL_DEC enum tpsio_status tps_pack(struct tpslib *tl, tpselt te,
    tpspackreader *reader, void *arg);

/*
 * Unpack TPS document into an element tree. start initializes the
 * state. The packetlen parameter specifies the total length of the
 * document (this is not required, but it allows some optimizations).
 * feed should be called for all the data in the document as it comes
 * in. The number of bytes consumed is returned. If feed returns -1,
 * result should be called to retrieve the status. The element tree is
 * supplied if it is ready, and it becomes the caller's responsibility
 * to free it. In either case, the state structure is freed. If an
 * error occured, it is not possible to resynchronize the stream
 * unless the total packet length is known.
 */
CSL_DEC struct tps_unpackstate *tps_unpack_start(struct tpslib *tl, size_t packetlen);
CSL_DEC nb_size tps_unpack_feed(struct tps_unpackstate *us,
    const char *data, size_t datalen);
CSL_DEC enum tpsio_status tps_unpack_result(struct tps_unpackstate *us, tpselt *tep);

/* Internal */
void obsecure(char *, size_t);

/*
 * Get size of data packet itself(packetlen)
 */
size_t tps_packet_len(struct tps_unpackstate *us);

#endif
