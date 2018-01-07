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

#ifndef BOOST_PREDEF_ARCHITECTURE_PPC_H
#define BOOST_PREDEF_ARCHITECTURE_PPC_H

#include <boost/predef/version_number.h>
#include <boost/predef/make.h>

/*`
[heading `BOOST_ARCH_PPC`]

[@http://en.wikipedia.org/wiki/PowerPC PowerPC] architecture.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__powerpc`] [__predef_detection__]]
    [[`__powerpc__`] [__predef_detection__]]
    [[`__POWERPC__`] [__predef_detection__]]
    [[`__ppc__`] [__predef_detection__]]
    [[`_M_PPC`] [__predef_detection__]]
    [[`_ARCH_PPC`] [__predef_detection__]]
    [[`__PPCGECKO__`] [__predef_detection__]]
    [[`__PPCBROADWAY__`] [__predef_detection__]]
    [[`_XENON`] [__predef_detection__]]

    [[`__ppc601__`] [6.1.0]]
    [[`_ARCH_601`] [6.1.0]]
    [[`__ppc603__`] [6.3.0]]
    [[`_ARCH_603`] [6.3.0]]
    [[`__ppc604__`] [6.4.0]]
    [[`__ppc604__`] [6.4.0]]
    ]
 */

#define BOOST_ARCH_PPC BOOST_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__powerpc) || defined(__powerpc__) || \
    defined(__POWERPC__) || defined(__ppc__) || \
    defined(_M_PPC) || defined(_ARCH_PPC) || \
    defined(__PPCGECKO__) || defined(__PPCBROADWAY__) || \
    defined(_XENON)
#   undef BOOST_ARCH_PPC
#   if !defined (BOOST_ARCH_PPC) && (defined(__ppc601__) || defined(_ARCH_601))
#       define BOOST_ARCH_PPC BOOST_VERSION_NUMBER(6,1,0)
#   endif
#   if !defined (BOOST_ARCH_PPC) && (defined(__ppc603__) || defined(_ARCH_603))
#       define BOOST_ARCH_PPC BOOST_VERSION_NUMBER(6,3,0)
#   endif
#   if !defined (BOOST_ARCH_PPC) && (defined(__ppc604__) || defined(__ppc604__))
#       define BOOST_ARCH_PPC BOOST_VERSION_NUMBER(6,4,0)
#   endif
#   if !defined (BOOST_ARCH_PPC)
#       define BOOST_ARCH_PPC BOOST_VERSION_NUMBER_AVAILABLE
#   endif
#endif

#if BOOST_ARCH_PPC
#   define BOOST_ARCH_PPC_AVAILABLE
#endif

#define BOOST_ARCH_PPC_NAME "PowerPC"

#include <boost/predef/detail/test.h>
BOOST_PREDEF_DECLARE_TEST(BOOST_ARCH_PPC,BOOST_ARCH_PPC_NAME)


#endif
