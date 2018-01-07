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

#ifndef BOOST_TT_ADD_REFERENCE_HPP_INCLUDED
#define BOOST_TT_ADD_REFERENCE_HPP_INCLUDED

#include <boost/type_traits/is_reference.hpp>
#include <boost/detail/workaround.hpp>
#include <boost/config.hpp>

// should be the last #include
#include <boost/type_traits/detail/type_trait_def.hpp>

namespace boost {

namespace detail {

//
// We can't filter out rvalue_references at the same level as
// references or we get ambiguities from msvc:
//

template <typename T>
struct add_reference_rvalue_layer
{
    typedef T& type;
};

#ifndef BOOST_NO_CXX11_RVALUE_REFERENCES
template <typename T>
struct add_reference_rvalue_layer<T&&>
{
    typedef T&& type;
};
#endif

template <typename T>
struct add_reference_impl
{
    typedef typename add_reference_rvalue_layer<T>::type type;
};

BOOST_TT_AUX_TYPE_TRAIT_IMPL_PARTIAL_SPEC1_1(typename T,add_reference,T&,T&)

// these full specialisations are always required:
BOOST_TT_AUX_TYPE_TRAIT_IMPL_SPEC1(add_reference,void,void)
#ifndef BOOST_NO_CV_VOID_SPECIALIZATIONS
BOOST_TT_AUX_TYPE_TRAIT_IMPL_SPEC1(add_reference,void const,void const)
BOOST_TT_AUX_TYPE_TRAIT_IMPL_SPEC1(add_reference,void volatile,void volatile)
BOOST_TT_AUX_TYPE_TRAIT_IMPL_SPEC1(add_reference,void const volatile,void const volatile)
#endif

} // namespace detail

BOOST_TT_AUX_TYPE_TRAIT_DEF1(add_reference,T,typename boost::detail::add_reference_impl<T>::type)

// agurt, 07/mar/03: workaround Borland's ill-formed sensitivity to an additional
// level of indirection, here
#if BOOST_WORKAROUND(__BORLANDC__, < 0x600)
BOOST_TT_AUX_TYPE_TRAIT_PARTIAL_SPEC1_1(typename T,add_reference,T&,T&)
#endif

} // namespace boost

#include <boost/type_traits/detail/type_trait_undef.hpp>

#endif // BOOST_TT_ADD_REFERENCE_HPP_INCLUDED
