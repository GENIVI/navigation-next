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
Copyright Rene Rivera 2011-2013
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BOOST_PREDEF_ARCHITECTURE_CONVEX_H
#define BOOST_PREDEF_ARCHITECTURE_CONVEX_H

#include <boost/predef/version_number.h>
#include <boost/predef/make.h>

/*`
[heading `BOOST_ARCH_CONVEX`]

[@http://en.wikipedia.org/wiki/Convex_Computer Convex Computer] architecture.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__convex__`] [__predef_detection__]]

    [[`__convex_c1__`] [1.0.0]]
    [[`__convex_c2__`] [2.0.0]]
    [[`__convex_c32__`] [3.2.0]]
    [[`__convex_c34__`] [3.4.0]]
    [[`__convex_c38__`] [3.8.0]]
    ]
 */

#define BOOST_ARCH_CONVEX BOOST_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__convex__)
#   undef BOOST_ARCH_CONVEX
#   if !defined(BOOST_ARCH_CONVEX) && defined(__convex_c1__)
#       define BOOST_ARCH_CONVEX BOOST_VERSION_NUMBER(1,0,0)
#   endif
#   if !defined(BOOST_ARCH_CONVEX) && defined(__convex_c2__)
#       define BOOST_ARCH_CONVEX BOOST_VERSION_NUMBER(2,0,0)
#   endif
#   if !defined(BOOST_ARCH_CONVEX) && defined(__convex_c32__)
#       define BOOST_ARCH_CONVEX BOOST_VERSION_NUMBER(3,2,0)
#   endif
#   if !defined(BOOST_ARCH_CONVEX) && defined(__convex_c34__)
#       define BOOST_ARCH_CONVEX BOOST_VERSION_NUMBER(3,4,0)
#   endif
#   if !defined(BOOST_ARCH_CONVEX) && defined(__convex_c38__)
#       define BOOST_ARCH_CONVEX BOOST_VERSION_NUMBER(3,8,0)
#   endif
#   if !defined(BOOST_ARCH_CONVEX)
#       define BOOST_ARCH_CONVEX BOOST_VERSION_NUMBER_AVAILABLE
#   endif
#endif

#if BOOST_ARCH_CONVEX
#   define BOOST_ARCH_CONVEX_AVAILABLE
#endif

#define BOOST_ARCH_CONVEX_NAME "Convex Computer"

#include <boost/predef/detail/test.h>
BOOST_PREDEF_DECLARE_TEST(BOOST_ARCH_CONVEX,BOOST_ARCH_CONVEX_NAME)



#endif
