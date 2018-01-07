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


//  (C) Copyright Steve Cleary, Beman Dawes, Howard Hinnant & John Maddock 2000.
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).
//
//  See http://www.boost.org/libs/type_traits for most recent version including documentation.

#ifndef BOOST_TT_IS_INTEGRAL_HPP_INCLUDED
#define BOOST_TT_IS_INTEGRAL_HPP_INCLUDED

#include <boost/config.hpp>

// should be the last #include
#include <boost/type_traits/detail/bool_trait_def.hpp>

namespace boost {

//* is a type T an [cv-qualified-] integral type described in the standard (3.9.1p3)
// as an extension we include long long, as this is likely to be added to the
// standard at a later date
#if defined( __CODEGEARC__ )
BOOST_TT_AUX_BOOL_TRAIT_DEF1(is_integral,T,__is_integral(T))
#else
BOOST_TT_AUX_BOOL_TRAIT_DEF1(is_integral,T,false)

BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,unsigned char,true)
BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,unsigned short,true)
BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,unsigned int,true)
BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,unsigned long,true)

BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,signed char,true)
BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,signed short,true)
BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,signed int,true)
BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,signed long,true)

BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,bool,true)
BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,char,true)

#ifndef BOOST_NO_INTRINSIC_WCHAR_T
// If the following line fails to compile and you're using the Intel
// compiler, see http://lists.boost.org/MailArchives/boost-users/msg06567.php,
// and define BOOST_NO_INTRINSIC_WCHAR_T on the command line.
BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,wchar_t,true)
#endif

// Same set of integral types as in boost/type_traits/integral_promotion.hpp.
// Please, keep in sync. -- Alexander Nasonov
#if (defined(BOOST_INTEL_CXX_VERSION) && defined(_MSC_VER) && (BOOST_INTEL_CXX_VERSION <= 600)) \
    || (defined(__BORLANDC__) && (__BORLANDC__ == 0x600) && (_MSC_VER < 1300))
BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,unsigned __int8,true)
BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,__int8,true)
BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,unsigned __int16,true)
BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,__int16,true)
BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,unsigned __int32,true)
BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,__int32,true)
#ifdef __BORLANDC__
BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,unsigned __int64,true)
BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,__int64,true)
#endif
#endif

# if defined(BOOST_HAS_LONG_LONG)
BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral, ::boost::ulong_long_type,true)
BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral, ::boost::long_long_type,true)
#elif defined(BOOST_HAS_MS_INT64)
BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,unsigned __int64,true)
BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,__int64,true)
#endif
        
#ifdef BOOST_HAS_INT128
BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,boost::int128_type,true)
BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,boost::uint128_type,true)
#endif
#ifndef BOOST_NO_CXX11_CHAR16_T
BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,char16_t,true)
#endif
#ifndef BOOST_NO_CXX11_CHAR32_T
BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,char32_t,true)
#endif

#endif  // non-CodeGear implementation

} // namespace boost

#include <boost/type_traits/detail/bool_trait_undef.hpp>

#endif // BOOST_TT_IS_INTEGRAL_HPP_INCLUDED
