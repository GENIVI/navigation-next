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
Copyright Rene Rivera 2012-2013
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BOOST_PREDEF_OS_BSD_NET_H
#define BOOST_PREDEF_OS_BSD_NET_H

#include <boost/predef/os/bsd.h>

/*`
[heading `BOOST_OS_BSD_NET`]

[@http://en.wikipedia.org/wiki/Netbsd NetBSD] operating system.

[table
    [[__predef_symbol__] [__predef_version__]]

    [[`__NETBSD__`] [__predef_detection__]]
    [[`__NetBSD__`] [__predef_detection__]]

    [[`__NETBSD_version`] [V.R.P]]
    [[`NetBSD0_8`] [0.8.0]]
    [[`NetBSD0_9`] [0.9.0]]
    [[`NetBSD1_0`] [1.0.0]]
    [[`__NetBSD_Version`] [V.R.P]]
    ]
 */

#define BOOST_OS_BSD_NET BOOST_VERSION_NUMBER_NOT_AVAILABLE

#if !defined(BOOST_PREDEF_DETAIL_OS_DETECTED) && ( \
    defined(__NETBSD__) || defined(__NetBSD__) \
    )
#   ifndef BOOST_OS_BSD_AVAILABLE
#       define BOOST_OS_BSD BOOST_VERSION_NUMBER_AVAILABLE
#       define BOOST_OS_BSD_AVAILABLE
#   endif
#   undef BOOST_OS_BSD_NET
#   if defined(__NETBSD__)
#       if defined(__NETBSD_version)
#           if __NETBSD_version < 500000
#               define BOOST_OS_BSD_NET \
                    BOOST_PREDEF_MAKE_10_VRP000(__NETBSD_version)
#           else
#               define BOOST_OS_BSD_NET \
                    BOOST_PREDEF_MAKE_10_VRR000(__NETBSD_version)
#           endif
#       else
#           define BOOST_OS_BSD_NET BOOST_VERSION_NUMBER_AVAILABLE
#       endif
#   elif defined(__NetBSD__)
#       if !defined(BOOST_OS_BSD_NET) && defined(NetBSD0_8)
#           define BOOST_OS_BSD_NET BOOST_VERSION_NUMBER(0,8,0)
#       endif
#       if !defined(BOOST_OS_BSD_NET) && defined(NetBSD0_9)
#           define BOOST_OS_BSD_NET BOOST_VERSION_NUMBER(0,9,0)
#       endif
#       if !defined(BOOST_OS_BSD_NET) && defined(NetBSD1_0)
#           define BOOST_OS_BSD_NET BOOST_VERSION_NUMBER(1,0,0)
#       endif
#       if !defined(BOOST_OS_BSD_NET) && defined(__NetBSD_Version)
#           define BOOST_OS_BSD_NET \
                BOOST_PREDEF_MAKE_10_VVRR00PP00(__NetBSD_Version)
#       endif
#       if !defined(BOOST_OS_BSD_NET)
#           define BOOST_OS_BSD_NET BOOST_VERSION_NUMBER_AVAILABLE
#       endif
#   endif
#endif

#if BOOST_OS_BSD_NET
#   define BOOST_OS_BSD_NET_AVAILABLE
#   include <boost/predef/detail/os_detected.h>
#endif

#define BOOST_OS_BSD_NET_NAME "DragonFly BSD"

#include <boost/predef/detail/test.h>
BOOST_PREDEF_DECLARE_TEST(BOOST_OS_BSD_NET,BOOST_OS_BSD_NET_NAME)

#endif
