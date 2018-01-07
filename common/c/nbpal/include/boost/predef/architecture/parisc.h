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

#ifndef BOOST_PREDEF_ARCHITECTURE_PARISC_H
#define BOOST_PREDEF_ARCHITECTURE_PARISC_H

#include <boost/predef/version_number.h>
#include <boost/predef/make.h>

/*`
[heading `BOOST_ARCH_PARISK`]

[@http://en.wikipedia.org/wiki/PA-RISC_family HP/PA RISC] architecture.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__hppa__`] [__predef_detection__]]
    [[`__hppa`] [__predef_detection__]]
    [[`__HPPA__`] [__predef_detection__]]

    [[`_PA_RISC1_0`] [1.0.0]]
    [[`_PA_RISC1_1`] [1.1.0]]
    [[`__HPPA11__`] [1.1.0]]
    [[`__PA7100__`] [1.1.0]]
    [[`_PA_RISC2_0`] [2.0.0]]
    [[`__RISC2_0__`] [2.0.0]]
    [[`__HPPA20__`] [2.0.0]]
    [[`__PA8000__`] [2.0.0]]
    ]
 */

#define BOOST_ARCH_PARISC BOOST_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__hppa__) || defined(__hppa) || defined(__HPPA__)
#   undef BOOST_ARCH_PARISC
#   if !defined(BOOST_ARCH_PARISC) && (defined(_PA_RISC1_0))
#       define BOOST_ARCH_PARISC BOOST_VERSION_NUMBER(1,0,0)
#   endif
#   if !defined(BOOST_ARCH_PARISC) && (defined(_PA_RISC1_1) || defined(__HPPA11__) || defined(__PA7100__))
#       define BOOST_ARCH_PARISC BOOST_VERSION_NUMBER(1,1,0)
#   endif
#   if !defined(BOOST_ARCH_PARISC) && (defined(_PA_RISC2_0) || defined(__RISC2_0__) || defined(__HPPA20__) || defined(__PA8000__))
#       define BOOST_ARCH_PARISC BOOST_VERSION_NUMBER(2,0,0)
#   endif
#   if !defined(BOOST_ARCH_PARISC)
#       define BOOST_ARCH_PARISC BOOST_VERSION_NUMBER_AVAILABLE
#   endif
#endif

#if BOOST_ARCH_PARISC
#   define BOOST_ARCH_PARISC_AVAILABLE
#endif

#define BOOST_ARCH_PARISC_NAME "HP/PA RISC"

#include <boost/predef/detail/test.h>
BOOST_PREDEF_DECLARE_TEST(BOOST_ARCH_PARISC,BOOST_ARCH_PARISC_NAME)


#endif
