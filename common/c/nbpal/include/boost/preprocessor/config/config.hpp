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

# /* **************************************************************************
#  *                                                                          *
#  *     (C) Copyright Paul Mensonides 2002-2011.                             *
#  *     (C) Copyright Edward Diener 2011.                                    *
#  *     Distributed under the Boost Software License, Version 1.0. (See      *
#  *     accompanying file LICENSE_1_0.txt or copy at                         *
#  *     http://www.boost.org/LICENSE_1_0.txt)                                *
#  *                                                                          *
#  ************************************************************************** */
#
# /* See http://www.boost.org for most recent version. */
#
# ifndef BOOST_PREPROCESSOR_CONFIG_CONFIG_HPP
# define BOOST_PREPROCESSOR_CONFIG_CONFIG_HPP
#
# /* BOOST_PP_CONFIG_FLAGS */
#
# define BOOST_PP_CONFIG_STRICT() 0x0001
# define BOOST_PP_CONFIG_IDEAL() 0x0002
#
# define BOOST_PP_CONFIG_MSVC() 0x0004
# define BOOST_PP_CONFIG_MWCC() 0x0008
# define BOOST_PP_CONFIG_BCC() 0x0010
# define BOOST_PP_CONFIG_EDG() 0x0020
# define BOOST_PP_CONFIG_DMC() 0x0040
#
# ifndef BOOST_PP_CONFIG_FLAGS
#    if defined(__GCCXML__)
#        define BOOST_PP_CONFIG_FLAGS() (BOOST_PP_CONFIG_STRICT())
#    elif defined(__WAVE__)
#        define BOOST_PP_CONFIG_FLAGS() (BOOST_PP_CONFIG_STRICT())
#    elif defined(__MWERKS__) && __MWERKS__ >= 0x3200
#        define BOOST_PP_CONFIG_FLAGS() (BOOST_PP_CONFIG_STRICT())
#    elif defined(__EDG__) || defined(__EDG_VERSION__)
#        if defined(_MSC_VER) && __EDG_VERSION__ >= 308
#            define BOOST_PP_CONFIG_FLAGS() (BOOST_PP_CONFIG_MSVC())
#        else
#            define BOOST_PP_CONFIG_FLAGS() (BOOST_PP_CONFIG_EDG() | BOOST_PP_CONFIG_STRICT())
#        endif
#    elif defined(__MWERKS__)
#        define BOOST_PP_CONFIG_FLAGS() (BOOST_PP_CONFIG_MWCC())
#    elif defined(__DMC__)
#        define BOOST_PP_CONFIG_FLAGS() (BOOST_PP_CONFIG_DMC())
#    elif defined(__BORLANDC__) && __BORLANDC__ >= 0x581
#        define BOOST_PP_CONFIG_FLAGS() (BOOST_PP_CONFIG_STRICT())
#    elif defined(__BORLANDC__) || defined(__IBMC__) || defined(__IBMCPP__) || defined(__SUNPRO_CC)
#        define BOOST_PP_CONFIG_FLAGS() (BOOST_PP_CONFIG_BCC())
#    elif defined(_MSC_VER) && !defined(__clang__)
#        define BOOST_PP_CONFIG_FLAGS() (BOOST_PP_CONFIG_MSVC())
#    else
#        define BOOST_PP_CONFIG_FLAGS() (BOOST_PP_CONFIG_STRICT())
#    endif
# endif
#
# /* BOOST_PP_CONFIG_EXTENDED_LINE_INFO */
#
# ifndef BOOST_PP_CONFIG_EXTENDED_LINE_INFO
#    define BOOST_PP_CONFIG_EXTENDED_LINE_INFO 0
# endif
#
# /* BOOST_PP_CONFIG_ERRORS */
#
# ifndef BOOST_PP_CONFIG_ERRORS
#    ifdef NDEBUG
#        define BOOST_PP_CONFIG_ERRORS 0
#    else
#        define BOOST_PP_CONFIG_ERRORS 1
#    endif
# endif
#
# /* BOOST_PP_VARIADICS */
#
# if !defined BOOST_PP_VARIADICS
#    /* variadic support explicitly disabled for all untested compilers */
#    if defined __GCCXML__ || defined __CUDACC__ || defined __PATHSCALE__ || defined __DMC__ || defined __CODEGEARC__ || defined __BORLANDC__ || defined __MWERKS__ || defined __SUNPRO_CC || defined __HP_aCC && !defined __EDG__ || defined __MRC__ || defined __SC__ || defined __IBMCPP__ || defined __PGI
#        define BOOST_PP_VARIADICS 0
#    /* VC++ (C/C++) */
#    elif defined _MSC_VER && _MSC_VER >= 1400 && !defined __EDG__ && !defined __clang__
#        define BOOST_PP_VARIADICS 1
#        define BOOST_PP_VARIADICS_MSVC 1
#    /* Wave (C/C++), GCC (C++) */
#    elif defined __WAVE__ && __WAVE_HAS_VARIADICS__ || defined __GNUC__ && __GXX_EXPERIMENTAL_CXX0X__
#        define BOOST_PP_VARIADICS 1
#    /* EDG-based (C/C++), GCC (C), and unknown (C/C++) */
#    elif !defined __cplusplus && __STDC_VERSION__ >= 199901L || __cplusplus >= 201103L
#        define BOOST_PP_VARIADICS 1
#    else
#        define BOOST_PP_VARIADICS 0
#    endif
# elif !BOOST_PP_VARIADICS + 1 < 2
#    undef BOOST_PP_VARIADICS
#    define BOOST_PP_VARIADICS 1
#    if defined _MSC_VER && _MSC_VER >= 1400 && !(defined __EDG__ || defined __GCCXML__ || defined __CUDACC__ || defined __PATHSCALE__ || defined __clang__ || defined __DMC__ || defined __CODEGEARC__ || defined __BORLANDC__ || defined __MWERKS__ || defined __SUNPRO_CC || defined __HP_aCC || defined __MRC__ || defined __SC__ || defined __IBMCPP__ || defined __PGI)
#        define BOOST_PP_VARIADICS_MSVC 1
#    endif
# else
#    undef BOOST_PP_VARIADICS
#    define BOOST_PP_VARIADICS 0
# endif
#
# endif
