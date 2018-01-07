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
Copyright Rene Rivera 2008-2013
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BOOST_PREDEF_ARCHITECTURE_M68K_H
#define BOOST_PREDEF_ARCHITECTURE_M68K_H

#include <boost/predef/version_number.h>
#include <boost/predef/make.h>

/*`
[heading `BOOST_ARCH_M68K`]

[@http://en.wikipedia.org/wiki/M68k Motorola 68k] architecture.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__m68k__`] [__predef_detection__]]
    [[`M68000`] [__predef_detection__]]

    [[`__mc68060__`] [6.0.0]]
    [[`mc68060`] [6.0.0]]
    [[`__mc68060`] [6.0.0]]
    [[`__mc68040__`] [4.0.0]]
    [[`mc68040`] [4.0.0]]
    [[`__mc68040`] [4.0.0]]
    [[`__mc68030__`] [3.0.0]]
    [[`mc68030`] [3.0.0]]
    [[`__mc68030`] [3.0.0]]
    [[`__mc68020__`] [2.0.0]]
    [[`mc68020`] [2.0.0]]
    [[`__mc68020`] [2.0.0]]
    [[`__mc68010__`] [1.0.0]]
    [[`mc68010`] [1.0.0]]
    [[`__mc68010`] [1.0.0]]
    [[`__mc68000__`] [0.0.1]]
    [[`mc68000`] [0.0.1]]
    [[`__mc68000`] [0.0.1]]
    ]
 */

#define BOOST_ARCH_M68K BOOST_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__m68k__) || defined(M68000)
#   undef BOOST_ARCH_M68K
#   if !defined(BOOST_ARCH_M68K) && (defined(__mc68060__) || defined(mc68060) || defined(__mc68060))
#       define BOOST_ARCH_M68K BOOST_VERSION_NUMBER(6,0,0)
#   endif
#   if !defined(BOOST_ARCH_M68K) && (defined(__mc68040__) || defined(mc68040) || defined(__mc68040))
#       define BOOST_ARCH_M68K BOOST_VERSION_NUMBER(4,0,0)
#   endif
#   if !defined(BOOST_ARCH_M68K) && (defined(__mc68030__) || defined(mc68030) || defined(__mc68030))
#       define BOOST_ARCH_M68K BOOST_VERSION_NUMBER(3,0,0)
#   endif
#   if !defined(BOOST_ARCH_M68K) && (defined(__mc68020__) || defined(mc68020) || defined(__mc68020))
#       define BOOST_ARCH_M68K BOOST_VERSION_NUMBER(2,0,0)
#   endif
#   if !defined(BOOST_ARCH_M68K) && (defined(__mc68010__) || defined(mc68010) || defined(__mc68010))
#       define BOOST_ARCH_M68K BOOST_VERSION_NUMBER(1,0,0)
#   endif
#   if !defined(BOOST_ARCH_M68K) && (defined(__mc68000__) || defined(mc68000) || defined(__mc68000))
#       define BOOST_ARCH_M68K BOOST_VERSION_NUMBER_AVAILABLE
#   endif
#   if !defined(BOOST_ARCH_M68K)
#       define BOOST_ARCH_M68K BOOST_VERSION_NUMBER_AVAILABLE
#   endif
#endif

#if BOOST_ARCH_M68K
#   define BOOST_ARCH_M68K_AVAILABLE
#endif

#define BOOST_ARCH_M68K_NAME "Motorola 68k"

#include <boost/predef/detail/test.h>
BOOST_PREDEF_DECLARE_TEST(BOOST_ARCH_M68K,BOOST_ARCH_M68K_NAME)


#endif
