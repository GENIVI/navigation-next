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
# ifndef BOOST_PREPROCESSOR_SLOT_DETAIL_DEF_HPP
# define BOOST_PREPROCESSOR_SLOT_DETAIL_DEF_HPP
#
# /* BOOST_PP_SLOT_OFFSET_x */
#
# define BOOST_PP_SLOT_OFFSET_10(x) (x) % 1000000000UL
# define BOOST_PP_SLOT_OFFSET_9(x) BOOST_PP_SLOT_OFFSET_10(x) % 100000000UL
# define BOOST_PP_SLOT_OFFSET_8(x) BOOST_PP_SLOT_OFFSET_9(x) % 10000000UL
# define BOOST_PP_SLOT_OFFSET_7(x) BOOST_PP_SLOT_OFFSET_8(x) % 1000000UL
# define BOOST_PP_SLOT_OFFSET_6(x) BOOST_PP_SLOT_OFFSET_7(x) % 100000UL
# define BOOST_PP_SLOT_OFFSET_5(x) BOOST_PP_SLOT_OFFSET_6(x) % 10000UL
# define BOOST_PP_SLOT_OFFSET_4(x) BOOST_PP_SLOT_OFFSET_5(x) % 1000UL
# define BOOST_PP_SLOT_OFFSET_3(x) BOOST_PP_SLOT_OFFSET_4(x) % 100UL
# define BOOST_PP_SLOT_OFFSET_2(x) BOOST_PP_SLOT_OFFSET_3(x) % 10UL
#
# /* BOOST_PP_SLOT_CC_x */
#
# define BOOST_PP_SLOT_CC_2(a, b) BOOST_PP_SLOT_CC_2_D(a, b)
# define BOOST_PP_SLOT_CC_3(a, b, c) BOOST_PP_SLOT_CC_3_D(a, b, c)
# define BOOST_PP_SLOT_CC_4(a, b, c, d) BOOST_PP_SLOT_CC_4_D(a, b, c, d)
# define BOOST_PP_SLOT_CC_5(a, b, c, d, e) BOOST_PP_SLOT_CC_5_D(a, b, c, d, e)
# define BOOST_PP_SLOT_CC_6(a, b, c, d, e, f) BOOST_PP_SLOT_CC_6_D(a, b, c, d, e, f)
# define BOOST_PP_SLOT_CC_7(a, b, c, d, e, f, g) BOOST_PP_SLOT_CC_7_D(a, b, c, d, e, f, g)
# define BOOST_PP_SLOT_CC_8(a, b, c, d, e, f, g, h) BOOST_PP_SLOT_CC_8_D(a, b, c, d, e, f, g, h)
# define BOOST_PP_SLOT_CC_9(a, b, c, d, e, f, g, h, i) BOOST_PP_SLOT_CC_9_D(a, b, c, d, e, f, g, h, i)
# define BOOST_PP_SLOT_CC_10(a, b, c, d, e, f, g, h, i, j) BOOST_PP_SLOT_CC_10_D(a, b, c, d, e, f, g, h, i, j)
#
# define BOOST_PP_SLOT_CC_2_D(a, b) a ## b
# define BOOST_PP_SLOT_CC_3_D(a, b, c) a ## b ## c
# define BOOST_PP_SLOT_CC_4_D(a, b, c, d) a ## b ## c ## d
# define BOOST_PP_SLOT_CC_5_D(a, b, c, d, e) a ## b ## c ## d ## e
# define BOOST_PP_SLOT_CC_6_D(a, b, c, d, e, f) a ## b ## c ## d ## e ## f
# define BOOST_PP_SLOT_CC_7_D(a, b, c, d, e, f, g) a ## b ## c ## d ## e ## f ## g
# define BOOST_PP_SLOT_CC_8_D(a, b, c, d, e, f, g, h) a ## b ## c ## d ## e ## f ## g ## h
# define BOOST_PP_SLOT_CC_9_D(a, b, c, d, e, f, g, h, i) a ## b ## c ## d ## e ## f ## g ## h ## i
# define BOOST_PP_SLOT_CC_10_D(a, b, c, d, e, f, g, h, i, j) a ## b ## c ## d ## e ## f ## g ## h ## i ## j
#
# endif
