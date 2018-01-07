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

#ifndef BOOST_SMART_PTR_DETAIL_SP_COUNTED_BASE_ACC_IA64_HPP_INCLUDED
#define BOOST_SMART_PTR_DETAIL_SP_COUNTED_BASE_ACC_IA64_HPP_INCLUDED

//
//  detail/sp_counted_base_acc_ia64.hpp - aC++ on HP-UX IA64
//
//  Copyright 2007 Baruch Zilber
//  Copyright 2007 Boris Gubenko
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
//
//  Lock-free algorithm by Alexander Terekhov
//

#include <boost/detail/sp_typeinfo.hpp>
#include <machine/sys/inline.h>

namespace boost
{

namespace detail
{

inline void atomic_increment( int * pw )
{
    // ++*pw;

    _Asm_fetchadd(_FASZ_W, _SEM_REL, pw, +1, _LDHINT_NONE);
} 

inline int atomic_decrement( int * pw )
{
    // return --*pw;

    int r = static_cast<int>(_Asm_fetchadd(_FASZ_W, _SEM_REL, pw, -1, _LDHINT_NONE));
    if (1 == r)
    {
        _Asm_mf();
    }
    
    return r - 1;
}

inline int atomic_conditional_increment( int * pw )
{
    // if( *pw != 0 ) ++*pw;
    // return *pw;

    int v = *pw;
    
    for (;;)
    {
        if (0 == v)
        {
            return 0;
        }
        
        _Asm_mov_to_ar(_AREG_CCV,
                       v,
                       (_UP_CALL_FENCE | _UP_SYS_FENCE | _DOWN_CALL_FENCE | _DOWN_SYS_FENCE));
        int r = static_cast<int>(_Asm_cmpxchg(_SZ_W, _SEM_ACQ, pw, v + 1, _LDHINT_NONE));
        if (r == v)
        {
            return r + 1;
        }
        
        v = r;
    }
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
        return static_cast<int const volatile &>( use_count_ ); // TODO use ld.acq here
    }
};

} // namespace detail

} // namespace boost

#endif  // #ifndef BOOST_SMART_PTR_DETAIL_SP_COUNTED_BASE_ACC_IA64_HPP_INCLUDED
