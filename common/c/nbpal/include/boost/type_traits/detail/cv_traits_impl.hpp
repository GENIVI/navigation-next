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


//  (C) Copyright Dave Abrahams, Steve Cleary, Beman Dawes, Howard
//  Hinnant & John Maddock 2000.  
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).
//
//  See http://www.boost.org/libs/type_traits for most recent version including documentation.


#ifndef BOOST_TT_DETAIL_CV_TRAITS_IMPL_HPP_INCLUDED
#define BOOST_TT_DETAIL_CV_TRAITS_IMPL_HPP_INCLUDED

#include <cstddef>
#include <boost/config.hpp>
#include <boost/detail/workaround.hpp>


// implementation helper:


namespace boost {
namespace detail {

#if BOOST_WORKAROUND(BOOST_MSVC, == 1700)
#define BOOST_TT_AUX_CV_TRAITS_IMPL_PARAM(X) X
   template <typename T>
   struct cv_traits_imp
   {
      BOOST_STATIC_CONSTANT(bool, is_const = false);
      BOOST_STATIC_CONSTANT(bool, is_volatile = false);
      typedef T unqualified_type;
   };

   template <typename T>
   struct cv_traits_imp<T[]>
   {
      BOOST_STATIC_CONSTANT(bool, is_const = false);
      BOOST_STATIC_CONSTANT(bool, is_volatile = false);
      typedef T unqualified_type[];
   };

   template <typename T>
   struct cv_traits_imp<const T[]>
   {
      BOOST_STATIC_CONSTANT(bool, is_const = true);
      BOOST_STATIC_CONSTANT(bool, is_volatile = false);
      typedef T unqualified_type[];
   };

   template <typename T>
   struct cv_traits_imp<volatile T[]>
   {
      BOOST_STATIC_CONSTANT(bool, is_const = false);
      BOOST_STATIC_CONSTANT(bool, is_volatile = true);
      typedef T unqualified_type[];
   };

   template <typename T>
   struct cv_traits_imp<const volatile T[]>
   {
      BOOST_STATIC_CONSTANT(bool, is_const = true);
      BOOST_STATIC_CONSTANT(bool, is_volatile = true);
      typedef T unqualified_type[];
   };

   template <typename T, std::size_t N>
   struct cv_traits_imp<T[N]>
   {
      BOOST_STATIC_CONSTANT(bool, is_const = false);
      BOOST_STATIC_CONSTANT(bool, is_volatile = false);
      typedef T unqualified_type[N];
   };

   template <typename T, std::size_t N>
   struct cv_traits_imp<const T[N]>
   {
      BOOST_STATIC_CONSTANT(bool, is_const = true);
      BOOST_STATIC_CONSTANT(bool, is_volatile = false);
      typedef T unqualified_type[N];
   };

   template <typename T, std::size_t N>
   struct cv_traits_imp<volatile T[N]>
   {
      BOOST_STATIC_CONSTANT(bool, is_const = false);
      BOOST_STATIC_CONSTANT(bool, is_volatile = true);
      typedef T unqualified_type[N];
   };

   template <typename T, std::size_t N>
   struct cv_traits_imp<const volatile T[N]>
   {
      BOOST_STATIC_CONSTANT(bool, is_const = true);
      BOOST_STATIC_CONSTANT(bool, is_volatile = true);
      typedef T unqualified_type[N];
   };

#else
#define BOOST_TT_AUX_CV_TRAITS_IMPL_PARAM(X) X *
template <typename T> struct cv_traits_imp {};

template <typename T>
struct cv_traits_imp<T*>
{
    BOOST_STATIC_CONSTANT(bool, is_const = false);
    BOOST_STATIC_CONSTANT(bool, is_volatile = false);
    typedef T unqualified_type;
};
#endif

template <typename T>
struct cv_traits_imp<BOOST_TT_AUX_CV_TRAITS_IMPL_PARAM(const T)>
{
    BOOST_STATIC_CONSTANT(bool, is_const = true);
    BOOST_STATIC_CONSTANT(bool, is_volatile = false);
    typedef T unqualified_type;
};

template <typename T>
struct cv_traits_imp<BOOST_TT_AUX_CV_TRAITS_IMPL_PARAM(volatile T)>
{
    BOOST_STATIC_CONSTANT(bool, is_const = false);
    BOOST_STATIC_CONSTANT(bool, is_volatile = true);
    typedef T unqualified_type;
};

template <typename T>
struct cv_traits_imp<BOOST_TT_AUX_CV_TRAITS_IMPL_PARAM(const volatile T)>
{
    BOOST_STATIC_CONSTANT(bool, is_const = true);
    BOOST_STATIC_CONSTANT(bool, is_volatile = true);
    typedef T unqualified_type;
};

} // namespace detail
} // namespace boost 


#endif // BOOST_TT_DETAIL_CV_TRAITS_IMPL_HPP_INCLUDED
