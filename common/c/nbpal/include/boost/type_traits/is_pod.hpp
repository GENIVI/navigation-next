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

#ifndef BOOST_TT_IS_POD_HPP_INCLUDED
#define BOOST_TT_IS_POD_HPP_INCLUDED

#include <boost/type_traits/config.hpp>
#include <boost/type_traits/is_void.hpp>
#include <boost/type_traits/is_scalar.hpp>
#include <boost/type_traits/detail/ice_or.hpp>
#include <boost/type_traits/intrinsics.hpp>

#include <cstddef>

// should be the last #include
#include <boost/type_traits/detail/bool_trait_def.hpp>

#ifndef BOOST_IS_POD
#define BOOST_INTERNAL_IS_POD(T) false
#else
#define BOOST_INTERNAL_IS_POD(T) BOOST_IS_POD(T)
#endif

namespace boost {

// forward declaration, needed by 'is_pod_array_helper' template below
template< typename T > struct is_POD;

namespace detail {


template <typename T> struct is_pod_impl
{ 
    BOOST_STATIC_CONSTANT(
        bool, value =
        (::boost::type_traits::ice_or<
            ::boost::is_scalar<T>::value,
            ::boost::is_void<T>::value,
            BOOST_INTERNAL_IS_POD(T)
         >::value));
};

#if !defined(BOOST_NO_ARRAY_TYPE_SPECIALIZATIONS)
template <typename T, std::size_t sz>
struct is_pod_impl<T[sz]>
    : public is_pod_impl<T>
{
};
#endif


// the following help compilers without partial specialization support:
BOOST_TT_AUX_BOOL_TRAIT_IMPL_SPEC1(is_pod,void,true)

#ifndef BOOST_NO_CV_VOID_SPECIALIZATIONS
BOOST_TT_AUX_BOOL_TRAIT_IMPL_SPEC1(is_pod,void const,true)
BOOST_TT_AUX_BOOL_TRAIT_IMPL_SPEC1(is_pod,void volatile,true)
BOOST_TT_AUX_BOOL_TRAIT_IMPL_SPEC1(is_pod,void const volatile,true)
#endif

} // namespace detail

BOOST_TT_AUX_BOOL_TRAIT_DEF1(is_pod,T,::boost::detail::is_pod_impl<T>::value)
// is_POD is the old depricated name for this trait, do not use this as it may
// be removed in future without warning!!
BOOST_TT_AUX_BOOL_TRAIT_DEF1(is_POD,T,::boost::is_pod<T>::value)

} // namespace boost

#include <boost/type_traits/detail/bool_trait_undef.hpp>

#undef BOOST_INTERNAL_IS_POD

#endif // BOOST_TT_IS_POD_HPP_INCLUDED
