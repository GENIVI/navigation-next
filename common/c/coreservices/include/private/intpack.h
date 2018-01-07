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
 * intpack.h: created 2004/12/04 by Dima Dorfman.
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

#ifndef INTPACK_H
#define	INTPACK_H

/*
 * Serialization of fixed-width unsigned longs.
 */

/*
 * Pack long into a buffer. Use exactly len bytes, writing zeros to
 * the beginning if necessary.
 */
void bepackl(char *s, int len, unsigned long val);

/*
 * Unpack a long from a buffer, reading exactly len bytes.
 */
unsigned long beunpackl(const char *s, size_t len);


/*
 * Variable-length representation of unsigned integers. The unpacker
 * must be able to tell where the value ends without any external
 * input. For this purpose, the MSB is used to indicate the last byte.
 */

#define	MAXUINTPACKLEN(type)	(sizeof(type) * 8 / 7 + 1)
#define	MAXLONGPACKLEN		MAXUINTPACKLEN(long)

/*
 * Pack value into the buffer, which must be at least MAXLONGPACKLEN
 * bytes long. Return the number of bytes used.
 */
int uintpackl(unsigned long value, char *s);

/*
 * Unpack value from the buffer and write it into val. Return the
 * number of bytes consumed or -1 on overflow.
 */
int uintunpackl(const char *s, size_t len, unsigned long *val);

#endif
