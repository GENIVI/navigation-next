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
#  *
#  * See http://www.boost.org for most recent version.
#  */
#
# /* Revised by Paul Mensonides (2002) */
#
# ifndef BOOST_PREPROCESSOR_LIST_ADT_HPP
# define BOOST_PREPROCESSOR_LIST_ADT_HPP
#
# include <boost/preprocessor/config/config.hpp>
# include <boost/preprocessor/detail/is_binary.hpp>
# include <boost/preprocessor/logical/compl.hpp>
# include <boost/preprocessor/tuple/eat.hpp>
#
# /* BOOST_PP_LIST_CONS */
#
# define BOOST_PP_LIST_CONS(head, tail) (head, tail)
#
# /* BOOST_PP_LIST_NIL */
#
# define BOOST_PP_LIST_NIL BOOST_PP_NIL
#
# /* BOOST_PP_LIST_FIRST */
#
# define BOOST_PP_LIST_FIRST(list) BOOST_PP_LIST_FIRST_D(list)
#
# if ~BOOST_PP_CONFIG_FLAGS() & BOOST_PP_CONFIG_MWCC()
#    define BOOST_PP_LIST_FIRST_D(list) BOOST_PP_LIST_FIRST_I list
# else
#    define BOOST_PP_LIST_FIRST_D(list) BOOST_PP_LIST_FIRST_I ## list
# endif
#
# define BOOST_PP_LIST_FIRST_I(head, tail) head
#
# /* BOOST_PP_LIST_REST */
#
# define BOOST_PP_LIST_REST(list) BOOST_PP_LIST_REST_D(list)
#
# if ~BOOST_PP_CONFIG_FLAGS() & BOOST_PP_CONFIG_MWCC()
#    define BOOST_PP_LIST_REST_D(list) BOOST_PP_LIST_REST_I list
# else
#    define BOOST_PP_LIST_REST_D(list) BOOST_PP_LIST_REST_I ## list
# endif
#
# define BOOST_PP_LIST_REST_I(head, tail) tail
#
# /* BOOST_PP_LIST_IS_CONS */
#
# if BOOST_PP_CONFIG_FLAGS() & BOOST_PP_CONFIG_BCC()
#    define BOOST_PP_LIST_IS_CONS(list) BOOST_PP_LIST_IS_CONS_D(list)
#    define BOOST_PP_LIST_IS_CONS_D(list) BOOST_PP_LIST_IS_CONS_ ## list
#    define BOOST_PP_LIST_IS_CONS_(head, tail) 1
#    define BOOST_PP_LIST_IS_CONS_BOOST_PP_NIL 0
# else
#    define BOOST_PP_LIST_IS_CONS(list) BOOST_PP_IS_BINARY(list)
# endif
#
# /* BOOST_PP_LIST_IS_NIL */
#
# if ~BOOST_PP_CONFIG_FLAGS() & BOOST_PP_CONFIG_BCC()
#    define BOOST_PP_LIST_IS_NIL(list) BOOST_PP_COMPL(BOOST_PP_IS_BINARY(list))
# else
#    define BOOST_PP_LIST_IS_NIL(list) BOOST_PP_COMPL(BOOST_PP_LIST_IS_CONS(list))
# endif
#
# endif
