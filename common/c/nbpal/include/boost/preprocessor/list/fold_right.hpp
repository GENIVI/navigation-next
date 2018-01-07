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

# /* Copyright (C) 2001
#  * Housemarque Oy
#  * http://www.housemarque.com
#  *
#  * Distributed under the Boost Software License, Version 1.0. (See
#  * accompanying file LICENSE_1_0.txt or copy at
#  * http://www.boost.org/LICENSE_1_0.txt)
#  */
#
# /* Revised by Paul Mensonides (2002) */
#
# /* See http://www.boost.org for most recent version. */
#
# ifndef BOOST_PREPROCESSOR_LIST_FOLD_RIGHT_HPP
# define BOOST_PREPROCESSOR_LIST_FOLD_RIGHT_HPP
#
# include <boost/preprocessor/cat.hpp>
# include <boost/preprocessor/control/while.hpp>
# include <boost/preprocessor/debug/error.hpp>
# include <boost/preprocessor/detail/auto_rec.hpp>
#
# if 0
#    define BOOST_PP_LIST_FOLD_RIGHT(op, state, list)
# endif
#
# define BOOST_PP_LIST_FOLD_RIGHT BOOST_PP_CAT(BOOST_PP_LIST_FOLD_RIGHT_, BOOST_PP_AUTO_REC(BOOST_PP_WHILE_P, 256))
#
# define BOOST_PP_LIST_FOLD_RIGHT_257(o, s, l) BOOST_PP_ERROR(0x0004)
#
# define BOOST_PP_LIST_FOLD_RIGHT_D(d, o, s, l) BOOST_PP_LIST_FOLD_RIGHT_ ## d(o, s, l)
# define BOOST_PP_LIST_FOLD_RIGHT_2ND BOOST_PP_LIST_FOLD_RIGHT
# define BOOST_PP_LIST_FOLD_RIGHT_2ND_D BOOST_PP_LIST_FOLD_RIGHT_D
#
# if BOOST_PP_CONFIG_FLAGS() & BOOST_PP_CONFIG_EDG()
#    include <boost/preprocessor/list/detail/edg/fold_right.hpp>
# else
#    include <boost/preprocessor/list/detail/fold_right.hpp>
# endif
#
# endif
