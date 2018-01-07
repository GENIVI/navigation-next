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
# ifndef BOOST_PREPROCESSOR_LIST_LIST_FOR_EACH_I_HPP
# define BOOST_PREPROCESSOR_LIST_LIST_FOR_EACH_I_HPP
#
# include <boost/preprocessor/arithmetic/inc.hpp>
# include <boost/preprocessor/config/config.hpp>
# include <boost/preprocessor/list/adt.hpp>
# include <boost/preprocessor/repetition/for.hpp>
# include <boost/preprocessor/tuple/elem.hpp>
# include <boost/preprocessor/tuple/rem.hpp>
#
# /* BOOST_PP_LIST_FOR_EACH_I */
#
# if ~BOOST_PP_CONFIG_FLAGS() & BOOST_PP_CONFIG_EDG() && ~BOOST_PP_CONFIG_FLAGS() & BOOST_PP_CONFIG_MSVC()
#    define BOOST_PP_LIST_FOR_EACH_I(macro, data, list) BOOST_PP_FOR((macro, data, list, 0), BOOST_PP_LIST_FOR_EACH_I_P, BOOST_PP_LIST_FOR_EACH_I_O, BOOST_PP_LIST_FOR_EACH_I_M)
# else
#    define BOOST_PP_LIST_FOR_EACH_I(macro, data, list) BOOST_PP_LIST_FOR_EACH_I_I(macro, data, list)
#    define BOOST_PP_LIST_FOR_EACH_I_I(macro, data, list) BOOST_PP_FOR((macro, data, list, 0), BOOST_PP_LIST_FOR_EACH_I_P, BOOST_PP_LIST_FOR_EACH_I_O, BOOST_PP_LIST_FOR_EACH_I_M)
# endif
#
# if BOOST_PP_CONFIG_FLAGS() & BOOST_PP_CONFIG_STRICT()
#    define BOOST_PP_LIST_FOR_EACH_I_P(r, x) BOOST_PP_LIST_FOR_EACH_I_P_D x
#    define BOOST_PP_LIST_FOR_EACH_I_P_D(m, d, l, i) BOOST_PP_LIST_IS_CONS(l)
# else
#    define BOOST_PP_LIST_FOR_EACH_I_P(r, x) BOOST_PP_LIST_IS_CONS(BOOST_PP_TUPLE_ELEM(4, 2, x))
# endif
#
# if ~BOOST_PP_CONFIG_FLAGS() & BOOST_PP_CONFIG_MWCC()
#    define BOOST_PP_LIST_FOR_EACH_I_O(r, x) BOOST_PP_LIST_FOR_EACH_I_O_D x
#    define BOOST_PP_LIST_FOR_EACH_I_O_D(m, d, l, i) (m, d, BOOST_PP_LIST_REST(l), BOOST_PP_INC(i))
# else
#    define BOOST_PP_LIST_FOR_EACH_I_O(r, x) (BOOST_PP_TUPLE_ELEM(4, 0, x), BOOST_PP_TUPLE_ELEM(4, 1, x), BOOST_PP_LIST_REST(BOOST_PP_TUPLE_ELEM(4, 2, x)), BOOST_PP_INC(BOOST_PP_TUPLE_ELEM(4, 3, x)))
# endif
#
# if ~BOOST_PP_CONFIG_FLAGS() & BOOST_PP_CONFIG_EDG()
#    define BOOST_PP_LIST_FOR_EACH_I_M(r, x) BOOST_PP_LIST_FOR_EACH_I_M_D(r, BOOST_PP_TUPLE_ELEM(4, 0, x), BOOST_PP_TUPLE_ELEM(4, 1, x), BOOST_PP_TUPLE_ELEM(4, 2, x), BOOST_PP_TUPLE_ELEM(4, 3, x))
# else
#    define BOOST_PP_LIST_FOR_EACH_I_M(r, x) BOOST_PP_LIST_FOR_EACH_I_M_I(r, BOOST_PP_TUPLE_REM_4 x)
#    define BOOST_PP_LIST_FOR_EACH_I_M_I(r, x_e) BOOST_PP_LIST_FOR_EACH_I_M_D(r, x_e)
# endif
#
# define BOOST_PP_LIST_FOR_EACH_I_M_D(r, m, d, l, i) m(r, d, i, BOOST_PP_LIST_FIRST(l))
#
# /* BOOST_PP_LIST_FOR_EACH_I_R */
#
# if ~BOOST_PP_CONFIG_FLAGS() & BOOST_PP_CONFIG_EDG()
#    define BOOST_PP_LIST_FOR_EACH_I_R(r, macro, data, list) BOOST_PP_FOR_ ## r((macro, data, list, 0), BOOST_PP_LIST_FOR_EACH_I_P, BOOST_PP_LIST_FOR_EACH_I_O, BOOST_PP_LIST_FOR_EACH_I_M)
# else
#    define BOOST_PP_LIST_FOR_EACH_I_R(r, macro, data, list) BOOST_PP_LIST_FOR_EACH_I_R_I(r, macro, data, list)
#    define BOOST_PP_LIST_FOR_EACH_I_R_I(r, macro, data, list) BOOST_PP_FOR_ ## r((macro, data, list, 0), BOOST_PP_LIST_FOR_EACH_I_P, BOOST_PP_LIST_FOR_EACH_I_O, BOOST_PP_LIST_FOR_EACH_I_M)
# endif
#
# endif
