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
# include <boost/preprocessor/slot/detail/shared.hpp>
#
# undef BOOST_PP_ITERATION_START_3
#
# undef BOOST_PP_ITERATION_START_3_DIGIT_1
# undef BOOST_PP_ITERATION_START_3_DIGIT_2
# undef BOOST_PP_ITERATION_START_3_DIGIT_3
# undef BOOST_PP_ITERATION_START_3_DIGIT_4
# undef BOOST_PP_ITERATION_START_3_DIGIT_5
# undef BOOST_PP_ITERATION_START_3_DIGIT_6
# undef BOOST_PP_ITERATION_START_3_DIGIT_7
# undef BOOST_PP_ITERATION_START_3_DIGIT_8
# undef BOOST_PP_ITERATION_START_3_DIGIT_9
# undef BOOST_PP_ITERATION_START_3_DIGIT_10
#
# if BOOST_PP_SLOT_TEMP_3 == 0
#    define BOOST_PP_ITERATION_START_3_DIGIT_3 0
# elif BOOST_PP_SLOT_TEMP_3 == 1
#    define BOOST_PP_ITERATION_START_3_DIGIT_3 1
# elif BOOST_PP_SLOT_TEMP_3 == 2
#    define BOOST_PP_ITERATION_START_3_DIGIT_3 2
# elif BOOST_PP_SLOT_TEMP_3 == 3
#    define BOOST_PP_ITERATION_START_3_DIGIT_3 3
# elif BOOST_PP_SLOT_TEMP_3 == 4
#    define BOOST_PP_ITERATION_START_3_DIGIT_3 4
# elif BOOST_PP_SLOT_TEMP_3 == 5
#    define BOOST_PP_ITERATION_START_3_DIGIT_3 5
# elif BOOST_PP_SLOT_TEMP_3 == 6
#    define BOOST_PP_ITERATION_START_3_DIGIT_3 6
# elif BOOST_PP_SLOT_TEMP_3 == 7
#    define BOOST_PP_ITERATION_START_3_DIGIT_3 7
# elif BOOST_PP_SLOT_TEMP_3 == 8
#    define BOOST_PP_ITERATION_START_3_DIGIT_3 8
# elif BOOST_PP_SLOT_TEMP_3 == 9
#    define BOOST_PP_ITERATION_START_3_DIGIT_3 9
# endif
#
# if BOOST_PP_SLOT_TEMP_2 == 0
#    define BOOST_PP_ITERATION_START_3_DIGIT_2 0
# elif BOOST_PP_SLOT_TEMP_2 == 1
#    define BOOST_PP_ITERATION_START_3_DIGIT_2 1
# elif BOOST_PP_SLOT_TEMP_2 == 2
#    define BOOST_PP_ITERATION_START_3_DIGIT_2 2
# elif BOOST_PP_SLOT_TEMP_2 == 3
#    define BOOST_PP_ITERATION_START_3_DIGIT_2 3
# elif BOOST_PP_SLOT_TEMP_2 == 4
#    define BOOST_PP_ITERATION_START_3_DIGIT_2 4
# elif BOOST_PP_SLOT_TEMP_2 == 5
#    define BOOST_PP_ITERATION_START_3_DIGIT_2 5
# elif BOOST_PP_SLOT_TEMP_2 == 6
#    define BOOST_PP_ITERATION_START_3_DIGIT_2 6
# elif BOOST_PP_SLOT_TEMP_2 == 7
#    define BOOST_PP_ITERATION_START_3_DIGIT_2 7
# elif BOOST_PP_SLOT_TEMP_2 == 8
#    define BOOST_PP_ITERATION_START_3_DIGIT_2 8
# elif BOOST_PP_SLOT_TEMP_2 == 9
#    define BOOST_PP_ITERATION_START_3_DIGIT_2 9
# endif
#
# if BOOST_PP_SLOT_TEMP_1 == 0
#    define BOOST_PP_ITERATION_START_3_DIGIT_1 0
# elif BOOST_PP_SLOT_TEMP_1 == 1
#    define BOOST_PP_ITERATION_START_3_DIGIT_1 1
# elif BOOST_PP_SLOT_TEMP_1 == 2
#    define BOOST_PP_ITERATION_START_3_DIGIT_1 2
# elif BOOST_PP_SLOT_TEMP_1 == 3
#    define BOOST_PP_ITERATION_START_3_DIGIT_1 3
# elif BOOST_PP_SLOT_TEMP_1 == 4
#    define BOOST_PP_ITERATION_START_3_DIGIT_1 4
# elif BOOST_PP_SLOT_TEMP_1 == 5
#    define BOOST_PP_ITERATION_START_3_DIGIT_1 5
# elif BOOST_PP_SLOT_TEMP_1 == 6
#    define BOOST_PP_ITERATION_START_3_DIGIT_1 6
# elif BOOST_PP_SLOT_TEMP_1 == 7
#    define BOOST_PP_ITERATION_START_3_DIGIT_1 7
# elif BOOST_PP_SLOT_TEMP_1 == 8
#    define BOOST_PP_ITERATION_START_3_DIGIT_1 8
# elif BOOST_PP_SLOT_TEMP_1 == 9
#    define BOOST_PP_ITERATION_START_3_DIGIT_1 9
# endif
#
# if BOOST_PP_ITERATION_START_3_DIGIT_3
#    define BOOST_PP_ITERATION_START_3 BOOST_PP_SLOT_CC_3(BOOST_PP_ITERATION_START_3_DIGIT_3, BOOST_PP_ITERATION_START_3_DIGIT_2, BOOST_PP_ITERATION_START_3_DIGIT_1)
# elif BOOST_PP_ITERATION_START_3_DIGIT_2
#    define BOOST_PP_ITERATION_START_3 BOOST_PP_SLOT_CC_2(BOOST_PP_ITERATION_START_3_DIGIT_2, BOOST_PP_ITERATION_START_3_DIGIT_1)
# else
#    define BOOST_PP_ITERATION_START_3 BOOST_PP_ITERATION_START_3_DIGIT_1
# endif
