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
Copyright Rene Rivera 2008-2014
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BOOST_PREDEF_COMPILER_WATCOM_H
#define BOOST_PREDEF_COMPILER_WATCOM_H

#include <boost/predef/version_number.h>
#include <boost/predef/make.h>

/*`
[heading `BOOST_COMP_WATCOM`]

[@http://en.wikipedia.org/wiki/Watcom Watcom C++] compiler.
Version number available as major, and minor.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__WATCOMC__`] [__predef_detection__]]

    [[`__WATCOMC__`] [V.R.P]]
    ]
 */

#define BOOST_COMP_WATCOM BOOST_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__WATCOMC__)
#   define BOOST_COMP_WATCOM_DETECTION BOOST_PREDEF_MAKE_10_VVRR(__WATCOMC__)
#endif

#ifdef BOOST_COMP_WATCOM_DETECTION
#   if defined(BOOST_PREDEF_DETAIL_COMP_DETECTED)
#       define BOOST_COMP_WATCOM_EMULATED BOOST_COMP_WATCOM_DETECTION
#   else
#       undef BOOST_COMP_WATCOM
#       define BOOST_COMP_WATCOM BOOST_COMP_WATCOM_DETECTION
#   endif
#   define BOOST_COMP_WATCOM_AVAILABLE
#   include <boost/predef/detail/comp_detected.h>
#endif

#define BOOST_COMP_WATCOM_NAME "Watcom C++"

#include <boost/predef/detail/test.h>
BOOST_PREDEF_DECLARE_TEST(BOOST_COMP_WATCOM,BOOST_COMP_WATCOM_NAME)

#ifdef BOOST_COMP_WATCOM_EMULATED
#include <boost/predef/detail/test.h>
BOOST_PREDEF_DECLARE_TEST(BOOST_COMP_WATCOM_EMULATED,BOOST_COMP_WATCOM_NAME)
#endif


#endif
