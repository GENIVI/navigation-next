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
# ifndef BOOST_PREPROCESSOR_ITERATION_ITERATE_HPP
# define BOOST_PREPROCESSOR_ITERATION_ITERATE_HPP
#
# include <boost/preprocessor/arithmetic/dec.hpp>
# include <boost/preprocessor/arithmetic/inc.hpp>
# include <boost/preprocessor/array/elem.hpp>
# include <boost/preprocessor/array/size.hpp>
# include <boost/preprocessor/cat.hpp>
# include <boost/preprocessor/slot/slot.hpp>
# include <boost/preprocessor/tuple/elem.hpp>
#
# /* BOOST_PP_ITERATION_DEPTH */
#
# define BOOST_PP_ITERATION_DEPTH() 0
#
# /* BOOST_PP_ITERATION */
#
# define BOOST_PP_ITERATION() BOOST_PP_CAT(BOOST_PP_ITERATION_, BOOST_PP_ITERATION_DEPTH())
#
# /* BOOST_PP_ITERATION_START && BOOST_PP_ITERATION_FINISH */
#
# define BOOST_PP_ITERATION_START() BOOST_PP_CAT(BOOST_PP_ITERATION_START_, BOOST_PP_ITERATION_DEPTH())
# define BOOST_PP_ITERATION_FINISH() BOOST_PP_CAT(BOOST_PP_ITERATION_FINISH_, BOOST_PP_ITERATION_DEPTH())
#
# /* BOOST_PP_ITERATION_FLAGS */
#
# define BOOST_PP_ITERATION_FLAGS() (BOOST_PP_CAT(BOOST_PP_ITERATION_FLAGS_, BOOST_PP_ITERATION_DEPTH())())
#
# /* BOOST_PP_FRAME_ITERATION */
#
# define BOOST_PP_FRAME_ITERATION(i) BOOST_PP_CAT(BOOST_PP_ITERATION_, i)
#
# /* BOOST_PP_FRAME_START && BOOST_PP_FRAME_FINISH */
#
# define BOOST_PP_FRAME_START(i) BOOST_PP_CAT(BOOST_PP_ITERATION_START_, i)
# define BOOST_PP_FRAME_FINISH(i) BOOST_PP_CAT(BOOST_PP_ITERATION_FINISH_, i)
#
# /* BOOST_PP_FRAME_FLAGS */
#
# define BOOST_PP_FRAME_FLAGS(i) (BOOST_PP_CAT(BOOST_PP_ITERATION_FLAGS_, i)())
#
# /* BOOST_PP_RELATIVE_ITERATION */
#
# define BOOST_PP_RELATIVE_ITERATION(i) BOOST_PP_CAT(BOOST_PP_RELATIVE_, i)(BOOST_PP_ITERATION_)
#
# define BOOST_PP_RELATIVE_0(m) BOOST_PP_CAT(m, BOOST_PP_ITERATION_DEPTH())
# define BOOST_PP_RELATIVE_1(m) BOOST_PP_CAT(m, BOOST_PP_DEC(BOOST_PP_ITERATION_DEPTH()))
# define BOOST_PP_RELATIVE_2(m) BOOST_PP_CAT(m, BOOST_PP_DEC(BOOST_PP_DEC(BOOST_PP_ITERATION_DEPTH())))
# define BOOST_PP_RELATIVE_3(m) BOOST_PP_CAT(m, BOOST_PP_DEC(BOOST_PP_DEC(BOOST_PP_DEC(BOOST_PP_ITERATION_DEPTH()))))
# define BOOST_PP_RELATIVE_4(m) BOOST_PP_CAT(m, BOOST_PP_DEC(BOOST_PP_DEC(BOOST_PP_DEC(BOOST_PP_DEC(BOOST_PP_ITERATION_DEPTH())))))
#
# /* BOOST_PP_RELATIVE_START && BOOST_PP_RELATIVE_FINISH */
#
# define BOOST_PP_RELATIVE_START(i) BOOST_PP_CAT(BOOST_PP_RELATIVE_, i)(BOOST_PP_ITERATION_START_)
# define BOOST_PP_RELATIVE_FINISH(i) BOOST_PP_CAT(BOOST_PP_RELATIVE_, i)(BOOST_PP_ITERATION_FINISH_)
#
# /* BOOST_PP_RELATIVE_FLAGS */
#
# define BOOST_PP_RELATIVE_FLAGS(i) (BOOST_PP_CAT(BOOST_PP_RELATIVE_, i)(BOOST_PP_ITERATION_FLAGS_)())
#
# /* BOOST_PP_ITERATE */
#
# define BOOST_PP_ITERATE() BOOST_PP_CAT(BOOST_PP_ITERATE_, BOOST_PP_INC(BOOST_PP_ITERATION_DEPTH()))
#
# define BOOST_PP_ITERATE_1 <boost/preprocessor/iteration/detail/iter/forward1.hpp>
# define BOOST_PP_ITERATE_2 <boost/preprocessor/iteration/detail/iter/forward2.hpp>
# define BOOST_PP_ITERATE_3 <boost/preprocessor/iteration/detail/iter/forward3.hpp>
# define BOOST_PP_ITERATE_4 <boost/preprocessor/iteration/detail/iter/forward4.hpp>
# define BOOST_PP_ITERATE_5 <boost/preprocessor/iteration/detail/iter/forward5.hpp>
#
# endif
