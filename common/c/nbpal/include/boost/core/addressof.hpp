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

// Copyright (C) 2002 Brad King (brad.king@kitware.com)
//                    Douglas Gregor (gregod@cs.rpi.edu)
//
// Copyright (C) 2002, 2008, 2013 Peter Dimov
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// For more information, see http://www.boost.org

#ifndef BOOST_CORE_ADDRESSOF_HPP
#define BOOST_CORE_ADDRESSOF_HPP

# include <boost/config.hpp>
# include <boost/detail/workaround.hpp>
# include <cstddef>

namespace boost
{

namespace detail
{

template<class T> struct addr_impl_ref
{
    T & v_;

    BOOST_FORCEINLINE addr_impl_ref( T & v ): v_( v ) {}
    BOOST_FORCEINLINE operator T& () const { return v_; }

private:
    addr_impl_ref & operator=(const addr_impl_ref &);
};

template<class T> struct addressof_impl
{
    static BOOST_FORCEINLINE T * f( T & v, long )
    {
        return reinterpret_cast<T*>(
            &const_cast<char&>(reinterpret_cast<const volatile char &>(v)));
    }

    static BOOST_FORCEINLINE T * f( T * v, int )
    {
        return v;
    }
};

#if !defined( BOOST_NO_CXX11_NULLPTR )

#if !defined( BOOST_NO_CXX11_DECLTYPE ) && ( ( defined( __clang__ ) && !defined( _LIBCPP_VERSION ) ) || defined( __INTEL_COMPILER ) )

    typedef decltype(nullptr) addr_nullptr_t;

#else

    typedef std::nullptr_t addr_nullptr_t;

#endif

template<> struct addressof_impl< addr_nullptr_t >
{
    typedef addr_nullptr_t T;

    static BOOST_FORCEINLINE T * f( T & v, int )
    {
        return &v;
    }
};

template<> struct addressof_impl< addr_nullptr_t const >
{
    typedef addr_nullptr_t const T;

    static BOOST_FORCEINLINE T * f( T & v, int )
    {
        return &v;
    }
};

template<> struct addressof_impl< addr_nullptr_t volatile >
{
    typedef addr_nullptr_t volatile T;

    static BOOST_FORCEINLINE T * f( T & v, int )
    {
        return &v;
    }
};

template<> struct addressof_impl< addr_nullptr_t const volatile >
{
    typedef addr_nullptr_t const volatile T;

    static BOOST_FORCEINLINE T * f( T & v, int )
    {
        return &v;
    }
};

#endif

} // namespace detail

template<class T>
BOOST_FORCEINLINE
T * addressof( T & v )
{
#if (defined( __BORLANDC__ ) && BOOST_WORKAROUND( __BORLANDC__, BOOST_TESTED_AT( 0x610 ) ) ) || defined( __SUNPRO_CC )

    return boost::detail::addressof_impl<T>::f( v, 0 );

#else

    return boost::detail::addressof_impl<T>::f( boost::detail::addr_impl_ref<T>( v ), 0 );

#endif
}

#if defined( __SUNPRO_CC ) && BOOST_WORKAROUND( __SUNPRO_CC, BOOST_TESTED_AT( 0x590 ) )

namespace detail
{

template<class T> struct addressof_addp
{
    typedef T * type;
};

} // namespace detail

template< class T, std::size_t N >
BOOST_FORCEINLINE
typename detail::addressof_addp< T[N] >::type addressof( T (&t)[N] )
{
    return &t;
}

#endif

// Borland doesn't like casting an array reference to a char reference
// but these overloads work around the problem.
#if defined( __BORLANDC__ ) && BOOST_WORKAROUND(__BORLANDC__, BOOST_TESTED_AT(0x564))
template<typename T,std::size_t N>
BOOST_FORCEINLINE
T (*addressof(T (&t)[N]))[N]
{
   return reinterpret_cast<T(*)[N]>(&t);
}

template<typename T,std::size_t N>
BOOST_FORCEINLINE
const T (*addressof(const T (&t)[N]))[N]
{
   return reinterpret_cast<const T(*)[N]>(&t);
}
#endif

} // namespace boost

#endif // BOOST_CORE_ADDRESSOF_HPP
