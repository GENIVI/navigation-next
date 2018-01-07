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

#ifndef BOOST_CORE_TYPEINFO_HPP_INCLUDED
#define BOOST_CORE_TYPEINFO_HPP_INCLUDED

// MS compatible compilers support #pragma once

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

//  core::typeinfo, BOOST_CORE_TYPEID
//
//  Copyright 2007, 2014 Peter Dimov
//
//  Distributed under the Boost Software License, Version 1.0.
//  See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include <boost/config.hpp>

#if defined( BOOST_NO_TYPEID )

#include <boost/current_function.hpp>
#include <functional>

namespace boost
{

namespace core
{

class typeinfo
{
private:

    typeinfo( typeinfo const& );
    typeinfo& operator=( typeinfo const& );

    char const * name_;

public:

    explicit typeinfo( char const * name ): name_( name )
    {
    }

    bool operator==( typeinfo const& rhs ) const
    {
        return this == &rhs;
    }

    bool operator!=( typeinfo const& rhs ) const
    {
        return this != &rhs;
    }

    bool before( typeinfo const& rhs ) const
    {
        return std::less< typeinfo const* >()( this, &rhs );
    }

    char const* name() const
    {
        return name_;
    }
};

inline char const * demangled_name( core::typeinfo const & ti )
{
    return ti.name();
}

} // namespace core

namespace detail
{

template<class T> struct core_typeid_
{
    static boost::core::typeinfo ti_;

    static char const * name()
    {
        return BOOST_CURRENT_FUNCTION;
    }
};

#if defined(__SUNPRO_CC)
// see #4199, the Sun Studio compiler gets confused about static initialization 
// constructor arguments. But an assignment works just fine. 
template<class T> boost::core::typeinfo core_typeid_< T >::ti_ = core_typeid_< T >::name();
#else
template<class T> boost::core::typeinfo core_typeid_< T >::ti_(core_typeid_< T >::name());
#endif

template<class T> struct core_typeid_< T & >: core_typeid_< T >
{
};

template<class T> struct core_typeid_< T const >: core_typeid_< T >
{
};

template<class T> struct core_typeid_< T volatile >: core_typeid_< T >
{
};

template<class T> struct core_typeid_< T const volatile >: core_typeid_< T >
{
};

} // namespace detail

} // namespace boost

#define BOOST_CORE_TYPEID(T) (boost::detail::core_typeid_<T>::ti_)

#else

#include <boost/core/demangle.hpp>
#include <typeinfo>

namespace boost
{

namespace core
{

#if defined( BOOST_NO_STD_TYPEINFO )

typedef ::type_info typeinfo;

#else

typedef std::type_info typeinfo;

#endif

inline std::string demangled_name( core::typeinfo const & ti )
{
    return core::demangle( ti.name() );
}

} // namespace core

} // namespace boost

#define BOOST_CORE_TYPEID(T) typeid(T)

#endif

#endif  // #ifndef BOOST_CORE_TYPEINFO_HPP_INCLUDED
