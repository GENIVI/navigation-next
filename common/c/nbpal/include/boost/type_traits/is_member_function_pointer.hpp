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


#ifndef BOOST_TT_IS_MEMBER_FUNCTION_POINTER_HPP_INCLUDED
#define BOOST_TT_IS_MEMBER_FUNCTION_POINTER_HPP_INCLUDED

#include <boost/type_traits/config.hpp>
#include <boost/detail/workaround.hpp>

#if !BOOST_WORKAROUND(__BORLANDC__, < 0x600) && !defined(BOOST_TT_TEST_MS_FUNC_SIGS)
   //
   // Note: we use the "workaround" version for MSVC because it works for 
   // __stdcall etc function types, where as the partial specialisation
   // version does not do so.
   //
#   include <boost/type_traits/detail/is_mem_fun_pointer_impl.hpp>
#   include <boost/type_traits/remove_cv.hpp>
#else
#   include <boost/type_traits/is_reference.hpp>
#   include <boost/type_traits/is_array.hpp>
#   include <boost/type_traits/detail/yes_no_type.hpp>
#   include <boost/type_traits/detail/false_result.hpp>
#   include <boost/type_traits/detail/ice_or.hpp>
#   include <boost/type_traits/detail/is_mem_fun_pointer_tester.hpp>
#endif

// should be the last #include
#include <boost/type_traits/detail/bool_trait_def.hpp>

namespace boost {

#if defined( __CODEGEARC__ )
BOOST_TT_AUX_BOOL_TRAIT_DEF1(is_member_function_pointer,T,__is_member_function_pointer( T ))
#elif !BOOST_WORKAROUND(__BORLANDC__, < 0x600) && !defined(BOOST_TT_TEST_MS_FUNC_SIGS)

BOOST_TT_AUX_BOOL_TRAIT_DEF1(
      is_member_function_pointer
    , T
    , ::boost::type_traits::is_mem_fun_pointer_impl<typename remove_cv<T>::type>::value
    )

#else

namespace detail {

#ifndef __BORLANDC__

template <bool>
struct is_mem_fun_pointer_select
    : public ::boost::type_traits::false_result
{
};

template <>
struct is_mem_fun_pointer_select<false>
{
    template <typename T> struct result_
    {
#if BOOST_WORKAROUND(BOOST_MSVC_FULL_VER, >= 140050000)
#pragma warning(push)
#pragma warning(disable:6334)
#endif
        static T* make_t;
        typedef result_<T> self_type;

        BOOST_STATIC_CONSTANT(
            bool, value = (
                1 == sizeof(::boost::type_traits::is_mem_fun_pointer_tester(self_type::make_t))
            ));
#if BOOST_WORKAROUND(BOOST_MSVC_FULL_VER, >= 140050000)
#pragma warning(pop)
#endif
    };
};

template <typename T>
struct is_member_function_pointer_impl
    : public is_mem_fun_pointer_select<
          ::boost::type_traits::ice_or<
              ::boost::is_reference<T>::value
            , ::boost::is_array<T>::value
            >::value
        >::template result_<T>
{
};

template <typename T>
struct is_member_function_pointer_impl<T&> : public false_type{};

#else // Borland C++

template <typename T>
struct is_member_function_pointer_impl
{
   static T* m_t;
   BOOST_STATIC_CONSTANT(
              bool, value =
               (1 == sizeof(type_traits::is_mem_fun_pointer_tester(m_t))) );
};

template <typename T>
struct is_member_function_pointer_impl<T&>
{
   BOOST_STATIC_CONSTANT(bool, value = false);
};

#endif

BOOST_TT_AUX_BOOL_TRAIT_IMPL_SPEC1(is_member_function_pointer,void,false)
#ifndef BOOST_NO_CV_VOID_SPECIALIZATIONS
BOOST_TT_AUX_BOOL_TRAIT_IMPL_SPEC1(is_member_function_pointer,void const,false)
BOOST_TT_AUX_BOOL_TRAIT_IMPL_SPEC1(is_member_function_pointer,void volatile,false)
BOOST_TT_AUX_BOOL_TRAIT_IMPL_SPEC1(is_member_function_pointer,void const volatile,false)
#endif

} // namespace detail

BOOST_TT_AUX_BOOL_TRAIT_DEF1(is_member_function_pointer,T,::boost::detail::is_member_function_pointer_impl<T>::value)

#endif

} // namespace boost

#include <boost/type_traits/detail/bool_trait_undef.hpp>

#endif // BOOST_TT_IS_MEMBER_FUNCTION_POINTER_HPP_INCLUDED
