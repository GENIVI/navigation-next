/***************************************************************************
 *
 * valaray.cc - Declarations for the Standard Library valarray
 *
 * $Id: valarray.cc 91768 2005-08-10 18:58:48Z drodgman $
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


_RWSTD_NAMESPACE_BEGIN (std)


template <class _TypeT>
inline valarray<_TypeT> valarray<_TypeT>::shift (int __n) const
{
    if (0 == __n)
        return *this;

    if (size () <= size_t (__n < 0 ? -__n : __n))
        return valarray (_TypeT (), size ());
        
    _RW::__rw_array <_TypeT> __tmp =
        _RW::__rw_array <_TypeT> (_TypeT (), size ());

    // 26.3.2.7, p5 - negative n shifts right, positive left
    if (__n < 0)
        copy (_C_array.begin (), _C_array.end () + __n, __tmp.begin () - __n);
    else
        copy (_C_array.begin () + __n, _C_array.end (), __tmp.begin ());

    return valarray (__tmp);
} 


template <class _TypeT>
inline valarray<_TypeT> valarray<_TypeT>::cshift (int __n) const
{
    // compute non-negative modulus - the sign of (a % b) is
    // implementation-defined if either argument is negative (5.6, p4)
    ptrdiff_t __mod = __n % ptrdiff_t (size ());
    size_t    __rem = __mod < 0 ? -__mod : __mod;

    if (0 == __rem)
        return *this;

    _RW::__rw_array <_TypeT> __tmp (size ());

    // 26.3.2.7, p7 - negative n rotates right, positive left
    rotate_copy (_C_array.begin (),
                 __n < 0 ? _C_array.end () - __rem : _C_array.begin () + __rem,
                 _C_array.end (), __tmp.begin ());

    return valarray (__tmp);
} 


_RWSTD_NAMESPACE_END   // std
