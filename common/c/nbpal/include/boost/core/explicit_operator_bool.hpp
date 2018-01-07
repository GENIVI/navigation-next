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

/*
 *          Copyright Andrey Semashev 2007 - 2013.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */

/*!
 * \file   explicit_operator_bool.hpp
 * \author Andrey Semashev
 * \date   08.03.2009
 *
 * This header defines a compatibility macro that implements an unspecified
 * \c bool operator idiom, which is superseded with explicit conversion operators in
 * C++11.
 */

#ifndef BOOST_CORE_EXPLICIT_OPERATOR_BOOL_HPP
#define BOOST_CORE_EXPLICIT_OPERATOR_BOOL_HPP

#include <boost/config.hpp>

#ifdef BOOST_HAS_PRAGMA_ONCE
#pragma once
#endif

#if !defined(BOOST_NO_CXX11_EXPLICIT_CONVERSION_OPERATORS)

/*!
 * \brief The macro defines an explicit operator of conversion to \c bool
 *
 * The macro should be used inside the definition of a class that has to
 * support the conversion. The class should also implement <tt>operator!</tt>,
 * in terms of which the conversion operator will be implemented.
 */
#define BOOST_EXPLICIT_OPERATOR_BOOL()\
    BOOST_FORCEINLINE explicit operator bool () const\
    {\
        return !this->operator! ();\
    }

/*!
 * \brief The macro defines a noexcept explicit operator of conversion to \c bool
 *
 * The macro should be used inside the definition of a class that has to
 * support the conversion. The class should also implement <tt>operator!</tt>,
 * in terms of which the conversion operator will be implemented.
 */
#define BOOST_EXPLICIT_OPERATOR_BOOL_NOEXCEPT()\
    BOOST_FORCEINLINE explicit operator bool () const BOOST_NOEXCEPT\
    {\
        return !this->operator! ();\
    }

/*!
 * \brief The macro defines a constexpr explicit operator of conversion to \c bool
 *
 * The macro should be used inside the definition of a class that has to
 * support the conversion. The class should also implement <tt>operator!</tt>,
 * in terms of which the conversion operator will be implemented.
 */
#define BOOST_CONSTEXPR_EXPLICIT_OPERATOR_BOOL()\
    BOOST_FORCEINLINE BOOST_CONSTEXPR explicit operator bool () const BOOST_NOEXCEPT\
    {\
        return !this->operator! ();\
    }

#else // !defined(BOOST_NO_CXX11_EXPLICIT_CONVERSION_OPERATORS)

#if (defined(__SUNPRO_CC) && (__SUNPRO_CC <= 0x530)) && !defined(BOOST_NO_COMPILER_CONFIG)
// Sun C++ 5.3 can't handle the safe_bool idiom, so don't use it
#define BOOST_NO_UNSPECIFIED_BOOL
#endif // (defined(__SUNPRO_CC) && (__SUNPRO_CC <= 0x530)) && !defined(BOOST_NO_COMPILER_CONFIG)

#if !defined(BOOST_NO_UNSPECIFIED_BOOL)

namespace boost {

namespace detail {

#if !defined(_MSC_VER) && !defined(__IBMCPP__)

    struct unspecified_bool
    {
        // NOTE TO THE USER: If you see this in error messages then you tried
        // to apply an unsupported operator on the object that supports
        // explicit conversion to bool.
        struct OPERATORS_NOT_ALLOWED;
        static void true_value(OPERATORS_NOT_ALLOWED*) {}
    };
    typedef void (*unspecified_bool_type)(unspecified_bool::OPERATORS_NOT_ALLOWED*);

#else

    // MSVC and VACPP are too eager to convert pointer to function to void* even though they shouldn't
    struct unspecified_bool
    {
        // NOTE TO THE USER: If you see this in error messages then you tried
        // to apply an unsupported operator on the object that supports
        // explicit conversion to bool.
        struct OPERATORS_NOT_ALLOWED;
        void true_value(OPERATORS_NOT_ALLOWED*) {}
    };
    typedef void (unspecified_bool::*unspecified_bool_type)(unspecified_bool::OPERATORS_NOT_ALLOWED*);

#endif

} // namespace detail

} // namespace boost

#define BOOST_EXPLICIT_OPERATOR_BOOL()\
    BOOST_FORCEINLINE operator boost::detail::unspecified_bool_type () const\
    {\
        return (!this->operator! () ? &boost::detail::unspecified_bool::true_value : (boost::detail::unspecified_bool_type)0);\
    }

#define BOOST_EXPLICIT_OPERATOR_BOOL_NOEXCEPT()\
    BOOST_FORCEINLINE operator boost::detail::unspecified_bool_type () const BOOST_NOEXCEPT\
    {\
        return (!this->operator! () ? &boost::detail::unspecified_bool::true_value : (boost::detail::unspecified_bool_type)0);\
    }

#define BOOST_CONSTEXPR_EXPLICIT_OPERATOR_BOOL()\
    BOOST_FORCEINLINE BOOST_CONSTEXPR operator boost::detail::unspecified_bool_type () const BOOST_NOEXCEPT\
    {\
        return (!this->operator! () ? &boost::detail::unspecified_bool::true_value : (boost::detail::unspecified_bool_type)0);\
    }

#else // !defined(BOOST_NO_UNSPECIFIED_BOOL)

#define BOOST_EXPLICIT_OPERATOR_BOOL()\
    BOOST_FORCEINLINE operator bool () const\
    {\
        return !this->operator! ();\
    }

#define BOOST_EXPLICIT_OPERATOR_BOOL_NOEXCEPT()\
    BOOST_FORCEINLINE operator bool () const BOOST_NOEXCEPT\
    {\
        return !this->operator! ();\
    }

#define BOOST_CONSTEXPR_EXPLICIT_OPERATOR_BOOL()\
    BOOST_FORCEINLINE BOOST_CONSTEXPR operator bool () const BOOST_NOEXCEPT\
    {\
        return !this->operator! ();\
    }

#endif // !defined(BOOST_NO_UNSPECIFIED_BOOL)

#endif // !defined(BOOST_NO_CXX11_EXPLICIT_CONVERSION_OPERATORS)

#endif // BOOST_CORE_EXPLICIT_OPERATOR_BOOL_HPP
