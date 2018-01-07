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

#ifndef BOOST_DETAIL_SP_COUNTED_BASE_GCC_MIPS_HPP_INCLUDED
#define BOOST_DETAIL_SP_COUNTED_BASE_GCC_MIPS_HPP_INCLUDED

// MS compatible compilers support #pragma once

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

//
//  detail/sp_counted_base_gcc_mips.hpp - g++ on MIPS
//
//  Copyright (c) 2009, Spirent Communications, Inc.
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
//
//  Lock-free algorithm by Alexander Terekhov
//

#include <boost/detail/sp_typeinfo.hpp>

namespace boost
{

namespace detail
{

inline void atomic_increment( int * pw )
{
    // ++*pw;

    int tmp;

    __asm__ __volatile__
    (
        "0:\n\t"
        ".set push\n\t"
        ".set mips2\n\t"
        "ll %0, %1\n\t"
        "addiu %0, 1\n\t"
        "sc %0, %1\n\t"
        ".set pop\n\t"
        "beqz %0, 0b":
        "=&r"( tmp ), "=m"( *pw ):
        "m"( *pw )
    );
}

inline int atomic_decrement( int * pw )
{
    // return --*pw;

    int rv, tmp;

    __asm__ __volatile__
    (
        "0:\n\t"
        ".set push\n\t"
        ".set mips2\n\t"
        "ll %1, %2\n\t"
        "addiu %0, %1, -1\n\t"
        "sc %0, %2\n\t"
        ".set pop\n\t"
        "beqz %0, 0b\n\t"
        "addiu %0, %1, -1":
        "=&r"( rv ), "=&r"( tmp ), "=m"( *pw ):
        "m"( *pw ):
        "memory"
    );

    return rv;
}

inline int atomic_conditional_increment( int * pw )
{
    // if( *pw != 0 ) ++*pw;
    // return *pw;

    int rv, tmp;

    __asm__ __volatile__
    (
        "0:\n\t"
        ".set push\n\t"
        ".set mips2\n\t"
        "ll %0, %2\n\t"
        "beqz %0, 1f\n\t"
        "addiu %1, %0, 1\n\t"
        "sc %1, %2\n\t"
        ".set pop\n\t"
        "beqz %1, 0b\n\t"
        "addiu %0, %0, 1\n\t"
        "1:":
        "=&r"( rv ), "=&r"( tmp ), "=m"( *pw ):
        "m"( *pw ):
        "memory"
    );

    return rv;
}

class sp_counted_base
{
private:

    sp_counted_base( sp_counted_base const & );
    sp_counted_base & operator= ( sp_counted_base const & );

    int use_count_;        // #shared
    int weak_count_;       // #weak + (#shared != 0)

public:

    sp_counted_base(): use_count_( 1 ), weak_count_( 1 )
    {
    }

    virtual ~sp_counted_base() // nothrow
    {
    }

    // dispose() is called when use_count_ drops to zero, to release
    // the resources managed by *this.

    virtual void dispose() = 0; // nothrow

    // destroy() is called when weak_count_ drops to zero.

    virtual void destroy() // nothrow
    {
        delete this;
    }

    virtual void * get_deleter( sp_typeinfo const & ti ) = 0;
    virtual void * get_untyped_deleter() = 0;

    void add_ref_copy()
    {
        atomic_increment( &use_count_ );
    }

    bool add_ref_lock() // true on success
    {
        return atomic_conditional_increment( &use_count_ ) != 0;
    }

    void release() // nothrow
    {
        if( atomic_decrement( &use_count_ ) == 0 )
        {
            dispose();
            weak_release();
        }
    }

    void weak_add_ref() // nothrow
    {
        atomic_increment( &weak_count_ );
    }

    void weak_release() // nothrow
    {
        if( atomic_decrement( &weak_count_ ) == 0 )
        {
            destroy();
        }
    }

    long use_count() const // nothrow
    {
        return static_cast<int const volatile &>( use_count_ );
    }
};

} // namespace detail

} // namespace boost

#endif  // #ifndef BOOST_DETAIL_SP_COUNTED_BASE_GCC_MIPS_HPP_INCLUDED
