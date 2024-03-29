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

#ifndef BOOST_PREDEF_LANGUAGE_STDCPP_H
#define BOOST_PREDEF_LANGUAGE_STDCPP_H

#include <boost/predef/version_number.h>
#include <boost/predef/make.h>

/*`
[heading `BOOST_LANG_STDCPP`]

[@http://en.wikipedia.org/wiki/C%2B%2B Standard C++] language.
If available, the year of the standard is detected as YYYY.MM.1 from the Epoc date.
Because of the way the C++ standardization process works the
defined version year will not be the commonly known year of the standard.
Specifically the defined versions are:

[table Detected Version Number vs. C++ Standard Year
  [[Detected Version Number] [Standard Year] [C++ Standard]]
  [[27.11.1] [1998] [ISO/IEC 14882:1998]]
  [[41.12.1] [2011] [ISO/IEC 14882:2011]]
]

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__cplusplus`] [__predef_detection__]]

    [[`__cplusplus`] [YYYY.MM.1]]
    ]
 */

#define BOOST_LANG_STDCPP BOOST_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__cplusplus)
#   undef BOOST_LANG_STDCPP
#   if (__cplusplus > 100)
#       define BOOST_LANG_STDCPP BOOST_PREDEF_MAKE_YYYYMM(__cplusplus)
#   else
#       define BOOST_LANG_STDCPP BOOST_VERSION_NUMBER_AVAILABLE
#   endif
#endif

#if BOOST_LANG_STDCPP
#   define BOOST_LANG_STDCPP_AVAILABLE
#endif

#define BOOST_LANG_STDCPP_NAME "Standard C++"

#include <boost/predef/detail/test.h>
BOOST_PREDEF_DECLARE_TEST(BOOST_LANG_STDCPP,BOOST_LANG_STDCPP_NAME)


/*`
[heading `BOOST_LANG_STDCPPCLI`]

[@http://en.wikipedia.org/wiki/C%2B%2B/CLI Standard C++/CLI] language.
If available, the year of the standard is detected as YYYY.MM.1 from the Epoc date.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__cplusplus_cli`] [__predef_detection__]]

    [[`__cplusplus_cli`] [YYYY.MM.1]]
    ]
 */

#define BOOST_LANG_STDCPPCLI BOOST_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__cplusplus_cli)
#   undef BOOST_LANG_STDCPPCLI
#   if (__cplusplus_cli > 100)
#       define BOOST_LANG_STDCPPCLI BOOST_PREDEF_MAKE_YYYYMM(__cplusplus_cli)
#   else
#       define BOOST_LANG_STDCPPCLI BOOST_VERSION_NUMBER_AVAILABLE
#   endif
#endif

#if BOOST_LANG_STDCPPCLI
#   define BOOST_LANG_STDCPPCLI_AVAILABLE
#endif

#define BOOST_LANG_STDCPPCLI_NAME "Standard C++/CLI"

#include <boost/predef/detail/test.h>
BOOST_PREDEF_DECLARE_TEST(BOOST_LANG_STDCPPCLI,BOOST_LANG_STDCPPCLI_NAME)


/*`
[heading `BOOST_LANG_STDECPP`]

[@http://en.wikipedia.org/wiki/Embedded_C%2B%2B Standard Embedded C++] language.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__embedded_cplusplus`] [__predef_detection__]]
    ]
 */

#define BOOST_LANG_STDECPP BOOST_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__embedded_cplusplus)
#   undef BOOST_LANG_STDECPP
#   define BOOST_LANG_STDECPP BOOST_VERSION_NUMBER_AVAILABLE
#endif

#if BOOST_LANG_STDECPP
#   define BOOST_LANG_STDECPP_AVAILABLE
#endif

#define BOOST_LANG_STDECPP_NAME "Standard Embedded C++"

#include <boost/predef/detail/test.h>
BOOST_PREDEF_DECLARE_TEST(BOOST_LANG_STDECPP,BOOST_LANG_STDECPP_NAME)


#endif
