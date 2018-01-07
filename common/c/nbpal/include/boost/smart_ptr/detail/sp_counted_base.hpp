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

#ifndef BOOST_SMART_PTR_DETAIL_SP_COUNTED_BASE_HPP_INCLUDED
#define BOOST_SMART_PTR_DETAIL_SP_COUNTED_BASE_HPP_INCLUDED

// MS compatible compilers support #pragma once

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

//
//  detail/sp_counted_base.hpp
//
//  Copyright 2005-2013 Peter Dimov
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/config.hpp>
#include <boost/smart_ptr/detail/sp_has_sync.hpp>

#if defined( BOOST_SP_DISABLE_THREADS )
# include <boost/smart_ptr/detail/sp_counted_base_nt.hpp>

#elif defined( BOOST_SP_USE_STD_ATOMIC )
# include <boost/smart_ptr/detail/sp_counted_base_std_atomic.hpp>

#elif defined( BOOST_SP_USE_SPINLOCK )
# include <boost/smart_ptr/detail/sp_counted_base_spin.hpp>

#elif defined( BOOST_SP_USE_PTHREADS )
# include <boost/smart_ptr/detail/sp_counted_base_pt.hpp>

#elif defined( BOOST_DISABLE_THREADS ) && !defined( BOOST_SP_ENABLE_THREADS ) && !defined( BOOST_DISABLE_WIN32 )
# include <boost/smart_ptr/detail/sp_counted_base_nt.hpp>

#elif defined( __SNC__ )
# include <boost/smart_ptr/detail/sp_counted_base_snc_ps3.hpp>

#elif defined( __GNUC__ ) && ( defined( __i386__ ) || defined( __x86_64__ ) ) && !defined(__PATHSCALE__)
# include <boost/smart_ptr/detail/sp_counted_base_gcc_x86.hpp>

#elif defined(__HP_aCC) && defined(__ia64)
# include <boost/smart_ptr/detail/sp_counted_base_acc_ia64.hpp>

#elif defined( __GNUC__ ) && defined( __ia64__ ) && !defined( __INTEL_COMPILER ) && !defined(__PATHSCALE__)
# include <boost/smart_ptr/detail/sp_counted_base_gcc_ia64.hpp>

#elif defined( __IBMCPP__ ) && defined( __powerpc )
# include <boost/smart_ptr/detail/sp_counted_base_vacpp_ppc.hpp>

#elif defined( __MWERKS__ ) && defined( __POWERPC__ )
# include <boost/smart_ptr/detail/sp_counted_base_cw_ppc.hpp>

#elif defined( __GNUC__ ) && ( defined( __powerpc__ ) || defined( __ppc__ ) || defined( __ppc ) ) && !defined(__PATHSCALE__) && !defined( _AIX )
# include <boost/smart_ptr/detail/sp_counted_base_gcc_ppc.hpp>

#elif defined( __GNUC__ ) && ( defined( __mips__ ) || defined( _mips ) ) && !defined(__PATHSCALE__)
# include <boost/smart_ptr/detail/sp_counted_base_gcc_mips.hpp>

#elif defined( BOOST_SP_HAS_SYNC )
# include <boost/smart_ptr/detail/sp_counted_base_sync.hpp>

#elif defined(__GNUC__) && ( defined( __sparcv9 ) || ( defined( __sparcv8 ) && ( __GNUC__ * 100 + __GNUC_MINOR__ >= 402 ) ) )
# include <boost/smart_ptr/detail/sp_counted_base_gcc_sparc.hpp>

#elif defined( WIN32 ) || defined( _WIN32 ) || defined( __WIN32__ ) || defined(__CYGWIN__)
# include <boost/smart_ptr/detail/sp_counted_base_w32.hpp>

#elif defined( _AIX )
# include <boost/smart_ptr/detail/sp_counted_base_aix.hpp>

#elif !defined( BOOST_HAS_THREADS )
# include <boost/smart_ptr/detail/sp_counted_base_nt.hpp>

#else
# include <boost/smart_ptr/detail/sp_counted_base_spin.hpp>

#endif

#endif  // #ifndef BOOST_SMART_PTR_DETAIL_SP_COUNTED_BASE_HPP_INCLUDED
