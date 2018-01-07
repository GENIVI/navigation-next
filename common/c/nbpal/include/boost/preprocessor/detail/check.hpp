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

# /* **************************************************************************
#  *                                                                          *
#  *     (C) Copyright Paul Mensonides 2002.
#  *     Distributed under the Boost Software License, Version 1.0. (See
#  *     accompanying file LICENSE_1_0.txt or copy at
#  *     http://www.boost.org/LICENSE_1_0.txt)
#  *                                                                          *
#  ************************************************************************** */
#
# /* See http://www.boost.org for most recent version. */
#
# ifndef BOOST_PREPROCESSOR_DETAIL_CHECK_HPP
# define BOOST_PREPROCESSOR_DETAIL_CHECK_HPP
#
# include <boost/preprocessor/cat.hpp>
# include <boost/preprocessor/config/config.hpp>
#
# /* BOOST_PP_CHECK */
#
# if ~BOOST_PP_CONFIG_FLAGS() & BOOST_PP_CONFIG_MWCC()
#    define BOOST_PP_CHECK(x, type) BOOST_PP_CHECK_D(x, type)
# else
#    define BOOST_PP_CHECK(x, type) BOOST_PP_CHECK_OO((x, type))
#    define BOOST_PP_CHECK_OO(par) BOOST_PP_CHECK_D ## par
# endif
#
# if ~BOOST_PP_CONFIG_FLAGS() & BOOST_PP_CONFIG_MSVC() && ~BOOST_PP_CONFIG_FLAGS() & BOOST_PP_CONFIG_DMC()
#    define BOOST_PP_CHECK_D(x, type) BOOST_PP_CHECK_1(BOOST_PP_CAT(BOOST_PP_CHECK_RESULT_, type x))
#    define BOOST_PP_CHECK_1(chk) BOOST_PP_CHECK_2(chk)
#    define BOOST_PP_CHECK_2(res, _) res
# elif BOOST_PP_CONFIG_FLAGS() & BOOST_PP_CONFIG_MSVC()
#    define BOOST_PP_CHECK_D(x, type) BOOST_PP_CHECK_1(type x)
#    define BOOST_PP_CHECK_1(chk) BOOST_PP_CHECK_2(chk)
#    define BOOST_PP_CHECK_2(chk) BOOST_PP_CHECK_3((BOOST_PP_CHECK_RESULT_ ## chk))
#    define BOOST_PP_CHECK_3(im) BOOST_PP_CHECK_5(BOOST_PP_CHECK_4 im)
#    define BOOST_PP_CHECK_4(res, _) res
#    define BOOST_PP_CHECK_5(res) res
# else /* DMC */
#    define BOOST_PP_CHECK_D(x, type) BOOST_PP_CHECK_OO((type x))
#    define BOOST_PP_CHECK_OO(par) BOOST_PP_CHECK_0 ## par
#    define BOOST_PP_CHECK_0(chk) BOOST_PP_CHECK_1(BOOST_PP_CAT(BOOST_PP_CHECK_RESULT_, chk))
#    define BOOST_PP_CHECK_1(chk) BOOST_PP_CHECK_2(chk)
#    define BOOST_PP_CHECK_2(res, _) res
# endif
#
# define BOOST_PP_CHECK_RESULT_1 1, BOOST_PP_NIL
#
# endif
