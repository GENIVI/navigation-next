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

#ifndef BOOST_SMART_PTR_DETAIL_SPINLOCK_GCC_ARM_HPP_INCLUDED
#define BOOST_SMART_PTR_DETAIL_SPINLOCK_GCC_ARM_HPP_INCLUDED

//
//  Copyright (c) 2008, 2011 Peter Dimov
//
//  Distributed under the Boost Software License, Version 1.0.
//  See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/smart_ptr/detail/yield_k.hpp>

#if defined(__ARM_ARCH_7__) || defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__) || defined(__ARM_ARCH_7S__)

# define BOOST_SP_ARM_BARRIER "dmb"
# define BOOST_SP_ARM_HAS_LDREX

#elif defined(__ARM_ARCH_6__) || defined(__ARM_ARCH_6J__) || defined(__ARM_ARCH_6K__) || defined(__ARM_ARCH_6Z__) || defined(__ARM_ARCH_6ZK__) || defined(__ARM_ARCH_6T2__)

# define BOOST_SP_ARM_BARRIER "mcr p15, 0, r0, c7, c10, 5"
# define BOOST_SP_ARM_HAS_LDREX

#else

# define BOOST_SP_ARM_BARRIER ""

#endif

namespace boost
{

namespace detail
{

class spinlock
{
public:

    int v_;

public:

    bool try_lock()
    {
        int r;

#ifdef BOOST_SP_ARM_HAS_LDREX

        __asm__ __volatile__(
            "ldrex %0, [%2]; \n"
            "cmp %0, %1; \n"
            "strexne %0, %1, [%2]; \n"
            BOOST_SP_ARM_BARRIER :
            "=&r"( r ): // outputs
            "r"( 1 ), "r"( &v_ ): // inputs
            "memory", "cc" );

#else

        __asm__ __volatile__(
            "swp %0, %1, [%2];\n"
            BOOST_SP_ARM_BARRIER :
            "=&r"( r ): // outputs
            "r"( 1 ), "r"( &v_ ): // inputs
            "memory", "cc" );

#endif

        return r == 0;
    }

    void lock()
    {
        for( unsigned k = 0; !try_lock(); ++k )
        {
            boost::detail::yield( k );
        }
    }

    void unlock()
    {
        __asm__ __volatile__( BOOST_SP_ARM_BARRIER ::: "memory" );
        *const_cast< int volatile* >( &v_ ) = 0;
    }

public:

    class scoped_lock
    {
    private:

        spinlock & sp_;

        scoped_lock( scoped_lock const & );
        scoped_lock & operator=( scoped_lock const & );

    public:

        explicit scoped_lock( spinlock & sp ): sp_( sp )
        {
            sp.lock();
        }

        ~scoped_lock()
        {
            sp_.unlock();
        }
    };
};

} // namespace detail
} // namespace boost

#define BOOST_DETAIL_SPINLOCK_INIT {0}

#undef BOOST_SP_ARM_BARRIER
#undef BOOST_SP_ARM_HAS_LDREX

#endif // #ifndef BOOST_SMART_PTR_DETAIL_SPINLOCK_GCC_ARM_HPP_INCLUDED
