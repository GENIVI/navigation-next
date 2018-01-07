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


#ifndef BOOST_MPL_AUX_PREPROCESSOR_ENUM_HPP_INCLUDED
#define BOOST_MPL_AUX_PREPROCESSOR_ENUM_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2000-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id$
// $Date$
// $Revision$

#include <boost/mpl/aux_/config/preprocessor.hpp>

// BOOST_MPL_PP_ENUM(0,int): <nothing>
// BOOST_MPL_PP_ENUM(1,int): int
// BOOST_MPL_PP_ENUM(2,int): int, int
// BOOST_MPL_PP_ENUM(n,int): int, int, .., int

#if !defined(BOOST_MPL_CFG_NO_OWN_PP_PRIMITIVES)

#   include <boost/preprocessor/cat.hpp>

#   define BOOST_MPL_PP_ENUM(n, param) \
    BOOST_PP_CAT(BOOST_MPL_PP_ENUM_,n)(param) \
    /**/
    
#   define BOOST_MPL_PP_ENUM_0(p)
#   define BOOST_MPL_PP_ENUM_1(p) p
#   define BOOST_MPL_PP_ENUM_2(p) p,p
#   define BOOST_MPL_PP_ENUM_3(p) p,p,p
#   define BOOST_MPL_PP_ENUM_4(p) p,p,p,p
#   define BOOST_MPL_PP_ENUM_5(p) p,p,p,p,p
#   define BOOST_MPL_PP_ENUM_6(p) p,p,p,p,p,p
#   define BOOST_MPL_PP_ENUM_7(p) p,p,p,p,p,p,p
#   define BOOST_MPL_PP_ENUM_8(p) p,p,p,p,p,p,p,p
#   define BOOST_MPL_PP_ENUM_9(p) p,p,p,p,p,p,p,p,p

#else

#   include <boost/preprocessor/comma_if.hpp>
#   include <boost/preprocessor/repeat.hpp>

#   define BOOST_MPL_PP_AUX_ENUM_FUNC(unused, i, param) \
    BOOST_PP_COMMA_IF(i) param \
    /**/

#   define BOOST_MPL_PP_ENUM(n, param) \
    BOOST_PP_REPEAT( \
          n \
        , BOOST_MPL_PP_AUX_ENUM_FUNC \
        , param \
        ) \
    /**/

#endif

#endif // BOOST_MPL_AUX_PREPROCESSOR_ENUM_HPP_INCLUDED
