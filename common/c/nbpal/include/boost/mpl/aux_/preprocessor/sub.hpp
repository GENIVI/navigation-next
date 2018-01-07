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


#ifndef BOOST_MPL_AUX_PREPROCESSOR_SUB_HPP_INCLUDED
#define BOOST_MPL_AUX_PREPROCESSOR_SUB_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2002-2004
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

#if !defined(BOOST_MPL_CFG_NO_OWN_PP_PRIMITIVES)

#   include <boost/mpl/aux_/preprocessor/tuple.hpp>

#if defined(BOOST_MPL_CFG_BROKEN_PP_MACRO_EXPANSION)
#   include <boost/preprocessor/cat.hpp>

#   define BOOST_MPL_PP_SUB(i,j) \
    BOOST_MPL_PP_SUB_DELAY(i,j) \
    /**/

#   define BOOST_MPL_PP_SUB_DELAY(i,j) \
    BOOST_PP_CAT(BOOST_MPL_PP_TUPLE_11_ELEM_##i,BOOST_MPL_PP_SUB_##j) \
    /**/
#else
#   define BOOST_MPL_PP_SUB(i,j) \
    BOOST_MPL_PP_SUB_DELAY(i,j) \
    /**/

#   define BOOST_MPL_PP_SUB_DELAY(i,j) \
    BOOST_MPL_PP_TUPLE_11_ELEM_##i BOOST_MPL_PP_SUB_##j \
    /**/
#endif

#   define BOOST_MPL_PP_SUB_0 (0,1,2,3,4,5,6,7,8,9,10)
#   define BOOST_MPL_PP_SUB_1 (0,0,1,2,3,4,5,6,7,8,9)
#   define BOOST_MPL_PP_SUB_2 (0,0,0,1,2,3,4,5,6,7,8)
#   define BOOST_MPL_PP_SUB_3 (0,0,0,0,1,2,3,4,5,6,7)
#   define BOOST_MPL_PP_SUB_4 (0,0,0,0,0,1,2,3,4,5,6)
#   define BOOST_MPL_PP_SUB_5 (0,0,0,0,0,0,1,2,3,4,5)
#   define BOOST_MPL_PP_SUB_6 (0,0,0,0,0,0,0,1,2,3,4)
#   define BOOST_MPL_PP_SUB_7 (0,0,0,0,0,0,0,0,1,2,3)
#   define BOOST_MPL_PP_SUB_8 (0,0,0,0,0,0,0,0,0,1,2)
#   define BOOST_MPL_PP_SUB_9 (0,0,0,0,0,0,0,0,0,0,1)
#   define BOOST_MPL_PP_SUB_10 (0,0,0,0,0,0,0,0,0,0,0)

#else

#   include <boost/preprocessor/arithmetic/sub.hpp>

#   define BOOST_MPL_PP_SUB(i,j) \
    BOOST_PP_SUB(i,j) \
    /**/
    
#endif

#endif // BOOST_MPL_AUX_PREPROCESSOR_SUB_HPP_INCLUDED
