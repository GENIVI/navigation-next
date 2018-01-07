/***************************************************************************
 *
 * limits.cc - class numeric_limits static member definitions
 *
 * $Id: limits.cc 91768 2005-08-10 18:58:48Z drodgman $
 *
 ***************************************************************************
 *
 * Copyright (c) 1994-2001 Rogue Wave Software, Inc.  All Rights Reserved.
 *
 * This computer software is owned by Rogue Wave Software, Inc. and is
 * protected by U.S. copyright laws and other laws and by international
 * treaties.  This computer software is furnished by Rogue Wave Software,
 * Inc. pursuant to a written license agreement and may be used, copied,
 * transmitted, and stored only in accordance with the terms of such
 * license and with the inclusion of the above copyright notice.  This
 * computer software or any other copies thereof may not be provided or
 * otherwise made available to any other person.
 *
 * U.S. Government Restricted Rights.  This computer software is provided
 * with Restricted Rights.  Use, duplication, or disclosure by the
 * Government is subject to restrictions as set forth in subparagraph (c)
 * (1) (ii) of The Rights in Technical Data and Computer Software clause
 * at DFARS 252.227-7013 or subparagraphs (c) (1) and (2) of the
 * Commercial Computer Software--Restricted Rights at 48 CFR 52.227-19,
 * as applicable.  Manufacturer is Rogue Wave Software, Inc., 5500
 * Flatiron Parkway, Boulder, Colorado 80301 USA.
 *
 **************************************************************************/

#include <rw/_defs.h>

_RWSTD_NAMESPACE_BEGIN (std)

#if !defined (_RWSTD_NO_STATIC_CONST_MEMBER_INIT) && !defined (_RWSTD_DEFINE_EXPORTS)
    
template <class _TypeT>
const bool numeric_limits<_TypeT>::is_specialized;

template <class _TypeT>
const int numeric_limits<_TypeT>::digits;

template <class _TypeT>
const int numeric_limits<_TypeT>::digits10;

template <class _TypeT>
const bool numeric_limits<_TypeT>::is_signed;

template <class _TypeT>
const bool numeric_limits<_TypeT>::is_integer;

template <class _TypeT>
const bool numeric_limits<_TypeT>::is_exact;

template <class _TypeT>
const int numeric_limits<_TypeT>::radix;

template <class _TypeT>
const int numeric_limits<_TypeT>::min_exponent;

template <class _TypeT>
const int numeric_limits<_TypeT>::min_exponent10;

template <class _TypeT>
const int numeric_limits<_TypeT>::max_exponent;

template <class _TypeT>
const int numeric_limits<_TypeT>::max_exponent10;

template <class _TypeT>
const bool numeric_limits<_TypeT>::has_infinity;

template <class _TypeT>
const bool numeric_limits<_TypeT>::has_quiet_NaN;

template <class _TypeT>
const bool numeric_limits<_TypeT>::has_signaling_NaN;

template <class _TypeT>
const float_denorm_style numeric_limits<_TypeT>::has_denorm;

template <class _TypeT>
const bool numeric_limits<_TypeT>::has_denorm_loss;

template <class _TypeT>
const bool numeric_limits<_TypeT>::is_iec559;

template <class _TypeT>
const bool numeric_limits<_TypeT>::is_bounded;

template <class _TypeT>
const bool numeric_limits<_TypeT>::is_modulo;

template <class _TypeT>
const bool numeric_limits<_TypeT>::traps;

template <class _TypeT>
const bool numeric_limits<_TypeT>::tinyness_before;

template <class _TypeT>
const float_round_style numeric_limits<_TypeT>::round_style;

#endif   // !defined (_RWSTD_NO_STATIC_CONST_MEMBER_INIT) && !defined (_RWSTD_DEFINE_EXPORTS)

_RWSTD_NAMESPACE_END   // std
