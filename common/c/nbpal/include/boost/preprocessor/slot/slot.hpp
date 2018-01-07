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
# ifndef BOOST_PREPROCESSOR_SLOT_SLOT_HPP
# define BOOST_PREPROCESSOR_SLOT_SLOT_HPP
#
# include <boost/preprocessor/cat.hpp>
# include <boost/preprocessor/slot/detail/def.hpp>
#
# /* BOOST_PP_ASSIGN_SLOT */
#
# define BOOST_PP_ASSIGN_SLOT(i) BOOST_PP_CAT(BOOST_PP_ASSIGN_SLOT_, i)
#
# define BOOST_PP_ASSIGN_SLOT_1 <boost/preprocessor/slot/detail/slot1.hpp>
# define BOOST_PP_ASSIGN_SLOT_2 <boost/preprocessor/slot/detail/slot2.hpp>
# define BOOST_PP_ASSIGN_SLOT_3 <boost/preprocessor/slot/detail/slot3.hpp>
# define BOOST_PP_ASSIGN_SLOT_4 <boost/preprocessor/slot/detail/slot4.hpp>
# define BOOST_PP_ASSIGN_SLOT_5 <boost/preprocessor/slot/detail/slot5.hpp>
#
# /* BOOST_PP_SLOT */
#
# define BOOST_PP_SLOT(i) BOOST_PP_CAT(BOOST_PP_SLOT_, i)()
#
# endif
