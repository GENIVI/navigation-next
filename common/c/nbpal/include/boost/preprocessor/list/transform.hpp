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
# ifndef BOOST_PREPROCESSOR_LIST_TRANSFORM_HPP
# define BOOST_PREPROCESSOR_LIST_TRANSFORM_HPP
#
# include <boost/preprocessor/config/config.hpp>
# include <boost/preprocessor/list/fold_right.hpp>
# include <boost/preprocessor/tuple/elem.hpp>
# include <boost/preprocessor/tuple/rem.hpp>
#
# /* BOOST_PP_LIST_TRANSFORM */
#
# if ~BOOST_PP_CONFIG_FLAGS() & BOOST_PP_CONFIG_EDG()
#    define BOOST_PP_LIST_TRANSFORM(op, data, list) BOOST_PP_TUPLE_ELEM(3, 2, BOOST_PP_LIST_FOLD_RIGHT(BOOST_PP_LIST_TRANSFORM_O, (op, data, BOOST_PP_NIL), list))
# else
#    define BOOST_PP_LIST_TRANSFORM(op, data, list) BOOST_PP_LIST_TRANSFORM_I(op, data, list)
#    define BOOST_PP_LIST_TRANSFORM_I(op, data, list) BOOST_PP_TUPLE_ELEM(3, 2, BOOST_PP_LIST_FOLD_RIGHT(BOOST_PP_LIST_TRANSFORM_O, (op, data, BOOST_PP_NIL), list))
# endif
#
# if ~BOOST_PP_CONFIG_FLAGS() & BOOST_PP_CONFIG_EDG()
#    define BOOST_PP_LIST_TRANSFORM_O(d, odr, elem) BOOST_PP_LIST_TRANSFORM_O_D(d, BOOST_PP_TUPLE_ELEM(3, 0, odr), BOOST_PP_TUPLE_ELEM(3, 1, odr), BOOST_PP_TUPLE_ELEM(3, 2, odr), elem)
# else
#    define BOOST_PP_LIST_TRANSFORM_O(d, odr, elem) BOOST_PP_LIST_TRANSFORM_O_I(d, BOOST_PP_TUPLE_REM_3 odr, elem)
#    define BOOST_PP_LIST_TRANSFORM_O_I(d, im, elem) BOOST_PP_LIST_TRANSFORM_O_D(d, im, elem)
# endif
#
# define BOOST_PP_LIST_TRANSFORM_O_D(d, op, data, res, elem) (op, data, (op(d, data, elem), res))
#
# /* BOOST_PP_LIST_TRANSFORM_D */
#
# if ~BOOST_PP_CONFIG_FLAGS() & BOOST_PP_CONFIG_EDG()
#    define BOOST_PP_LIST_TRANSFORM_D(d, op, data, list) BOOST_PP_TUPLE_ELEM(3, 2, BOOST_PP_LIST_FOLD_RIGHT_ ## d(BOOST_PP_LIST_TRANSFORM_O, (op, data, BOOST_PP_NIL), list))
# else
#    define BOOST_PP_LIST_TRANSFORM_D(d, op, data, list) BOOST_PP_LIST_TRANSFORM_D_I(d, op, data, list)
#    define BOOST_PP_LIST_TRANSFORM_D_I(d, op, data, list) BOOST_PP_TUPLE_ELEM(3, 2, BOOST_PP_LIST_FOLD_RIGHT_ ## d(BOOST_PP_LIST_TRANSFORM_O, (op, data, BOOST_PP_NIL), list))
# endif
#
# endif
