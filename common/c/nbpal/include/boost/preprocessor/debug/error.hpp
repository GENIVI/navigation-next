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
# ifndef BOOST_PREPROCESSOR_DEBUG_ERROR_HPP
# define BOOST_PREPROCESSOR_DEBUG_ERROR_HPP
#
# include <boost/preprocessor/cat.hpp>
# include <boost/preprocessor/config/config.hpp>
#
# /* BOOST_PP_ERROR */
#
# if BOOST_PP_CONFIG_ERRORS
#    define BOOST_PP_ERROR(code) BOOST_PP_CAT(BOOST_PP_ERROR_, code)
# endif
#
# define BOOST_PP_ERROR_0x0000 BOOST_PP_ERROR(0x0000, BOOST_PP_INDEX_OUT_OF_BOUNDS)
# define BOOST_PP_ERROR_0x0001 BOOST_PP_ERROR(0x0001, BOOST_PP_WHILE_OVERFLOW)
# define BOOST_PP_ERROR_0x0002 BOOST_PP_ERROR(0x0002, BOOST_PP_FOR_OVERFLOW)
# define BOOST_PP_ERROR_0x0003 BOOST_PP_ERROR(0x0003, BOOST_PP_REPEAT_OVERFLOW)
# define BOOST_PP_ERROR_0x0004 BOOST_PP_ERROR(0x0004, BOOST_PP_LIST_FOLD_OVERFLOW)
# define BOOST_PP_ERROR_0x0005 BOOST_PP_ERROR(0x0005, BOOST_PP_SEQ_FOLD_OVERFLOW)
# define BOOST_PP_ERROR_0x0006 BOOST_PP_ERROR(0x0006, BOOST_PP_ARITHMETIC_OVERFLOW)
# define BOOST_PP_ERROR_0x0007 BOOST_PP_ERROR(0x0007, BOOST_PP_DIVISION_BY_ZERO)
#
# endif
