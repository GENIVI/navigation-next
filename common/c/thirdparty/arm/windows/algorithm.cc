/***************************************************************************
 *
 * algorithm.cc - Non-inline definitions for the Standard Library algorithms
 *
 * $Id: algorithm.cc 91768 2005-08-10 18:58:48Z drodgman $
 *
 ***************************************************************************
 *
 * Copyright (c) 1994
 * Hewlett-Packard Company
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Hewlett-Packard Company makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
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

#include <rw/_random.h>
#include <rw/_defs.h>


_RWSTD_NAMESPACE_BEGIN (std)


template <class _FwdIter1, class _FwdIter2, class _Distance>
_FwdIter1 __find_end (_FwdIter1 __first1, _FwdIter1 __last1,
                      _FwdIter2 __first2, _FwdIter2 __last2,
                      _Distance*)
{
    _RWSTD_ASSERT_RANGE (__first1, __last1);
    _RWSTD_ASSERT_RANGE (__first2, __last2);

    _Distance __dist1 = _DISTANCE (__first2, __last2, _Distance);

    if (!__dist1)
        return __last1;

    _Distance __dist2 = _DISTANCE (__first1, __last1, _Distance);

    _FwdIter1 __res = __last1;

    while (__dist2 >= __dist1) {
        if (equal (__first2, __last2, __first1))
            __res  = __first1;

        __dist2 = _DISTANCE (++__first1, __last1, _Distance);
    }
    return __res;
}



template <class _FwdIter1, class _FwdIter2,
    class _BinaryPredicate, class _Distance>
_FwdIter1 __find_end (_FwdIter1 __first1, _FwdIter1 __last1,
                      _FwdIter2 __first2, _FwdIter2 __last2,
                      _BinaryPredicate __pred, _Distance*)
{
    _RWSTD_ASSERT_RANGE (__first1, __last1);
    _RWSTD_ASSERT_RANGE (__first2, __last2);

    _Distance __dist1 = _DISTANCE (__first2, __last2, _Distance);

    if (!__dist1)
        return __last1;

    _Distance __dist2 = _DISTANCE (__first1, __last1, _Distance);

    _FwdIter1 __save = __last1;

    while (__dist2 >= __dist1)
    {
        if (_STD::equal (__first2, __last2, __first1, __pred))
            __save = __first1;
        __dist2 = _DISTANCE (++__first1, __last1, _Distance);
    }
    return __save;
}


template <class _FwdIter1, class _FwdIter2>
_FwdIter1 find_first_of (_FwdIter1 __first1, _FwdIter1 __last1,
                         _FwdIter2 __first2, _FwdIter2 __last2)
{
    _RWSTD_ASSERT_RANGE (__first1, __last1);
    _RWSTD_ASSERT_RANGE (__first2, __last2);

    if (__first2 == __last2)
        return __last1;
    _FwdIter1 __next = __first1;
    while (__next != __last1)
    {
        if (_STD::find (__first2, __last2, *__next) != __last2)
            return __next;
        ++__next;
    }
    return __last1;
}


template <class _FwdIter1, class _FwdIter2,
    class _BinaryPredicate>
_FwdIter1 find_first_of (_FwdIter1 __first1, _FwdIter1 __last1,
                         _FwdIter2 __first2, _FwdIter2 __last2,
                         _BinaryPredicate __pred)
{
    _RWSTD_ASSERT_RANGE (__first1, __last1);
    _RWSTD_ASSERT_RANGE (__first2, __last2);

    if (__first2 == __last2)
        return __last1;

    for (_FwdIter1 __next = __first1; __next != __last1; ++__next)
        for (_FwdIter2 __iter = __first2; __iter != __last2; ++__iter)
            if (__pred (*__next, *__iter))
                return __next;
    return __last1;
}


template <class _FwdIter>
_FwdIter adjacent_find (_FwdIter __first, _FwdIter __last)
{
    _RWSTD_ASSERT_RANGE (__first, __last);

    if (__first == __last) return __last;
    _FwdIter __next = __first;
    while (++__next != __last)
    {
        if (*__first == *__next) return __first;
        __first = __next;
    }
    return __last;
}


template <class _FwdIter, class _BinaryPredicate>
_FwdIter adjacent_find (_FwdIter __first, _FwdIter __last,
                        _BinaryPredicate __pred)
{
    _RWSTD_ASSERT_RANGE (__first, __last);

    if (__first == __last) return __last;
    _FwdIter __next = __first;
    while (++__next != __last) {
        if (__pred (*__first, *__next))
            return __first;
        __first = __next;
    }
    return __last;
}



template <class _FwdIter1, class _FwdIter2,
    class _Distance1, class _Distance2>
_FwdIter1 __search (_FwdIter1 __first1, _FwdIter1 __last1,
                    _FwdIter2 __first2, _FwdIter2 __last2,
                    _Distance1*, _Distance2*)
{
    _RWSTD_ASSERT_RANGE (__first1, __last1);
    _RWSTD_ASSERT_RANGE (__first2, __last2);

    _Distance1 __dist1 = _DISTANCE (__first1, __last1, _Distance1);
    _Distance2 __dist2 = _DISTANCE (__first2, __last2, _Distance2);

    if (__dist1 < __dist2) return __last1;

    _FwdIter1 __cur1 = __first1;
    _FwdIter2 __cur2 = __first2;

    while (__cur2 != __last2) {
        if (!(*__cur1 == *__cur2)) {
            ++__cur1;
            ++__cur2;
            if (__dist1-- == __dist2)
                return __last1;
            else {
                __cur1 = ++__first1;
                __cur2 = __first2;
            }
        }
        else {
            ++__cur1;
            ++__cur2;
        }
    }

    return (__cur2 == __last2) ? __first1 : __last1;
}

template <class _FwdIter1, class _FwdIter2,
    class _BinaryPredicate, class _Distance1, class _Distance2>
_FwdIter1 __search (_FwdIter1 __first1, _FwdIter1 __last1,
                    _FwdIter2 __first2, _FwdIter2 __last2,
                    _BinaryPredicate __pred, _Distance1*, _Distance2*)
{
    _RWSTD_ASSERT_RANGE (__first1, __last1);
    _RWSTD_ASSERT_RANGE (__first2, __last2);

    _Distance1 __dist1 = _DISTANCE (__first1, __last1, _Distance1);
    _Distance2 __dist2 = _DISTANCE (__first2, __last2, _Distance2);

    if (__dist1 < __dist2) return __last1;

    _FwdIter1 __cur1 = __first1;
    _FwdIter2 __cur2 = __first2;

    while (__cur2 != __last2) {
        if (!__pred (*__cur1, *__cur2)) {
            ++__cur1;
            ++__cur2;
            if (__dist1-- == __dist2)
                return __last1;
            else {
                __cur1 = ++__first1;
                __cur2 = __first2;
            }
        }
        else {
            ++__cur1;
            ++__cur2;
        }
    }

    return (__cur2 == __last2) ? __first1 : __last1;
}

template <class _FwdIter, class _Distance, class _Size, class _TypeT>
_FwdIter __search_n (_FwdIter __first, _FwdIter __last,
                     _Distance*, _Size __count, const _TypeT& __val)
{
    _RWSTD_ASSERT_RANGE (__first, __last);

    _Distance __dist = _DISTANCE (__first, __last, _Distance);

    if (__dist < __count || __count <= 0) return __last;

    _Distance        __span    = __dist - __count;
    _Size            __matches = 0;
    _FwdIter         __current = __first;

    while (__current != __last) {
        if (!(*__current == __val)) {
            if (__span < __matches + 1)
                return __last;
            __span   -= __matches + 1;
            __matches = 0;
            __first   = ++__current;
        }
        else {
            if (++__matches == __count)
                return __first;
            ++__current;
        }
    }

    return __last;
}

template <class _FwdIter, class _Distance, class _Size, class _TypeT,
    class _BinaryPredicate>
_FwdIter __search_n (_FwdIter __first, _FwdIter __last,
                     _Distance*, _Size __count, const _TypeT& __val,
                     _BinaryPredicate __pred)
{
    _RWSTD_ASSERT_RANGE (__first, __last);

    _Distance __dist = _DISTANCE (__first, __last, _Distance);

    if (__dist < __count || __count <= 0) return __last;

    _Distance        __span    = __dist - __count;
    _Size            __matches = 0;
    _FwdIter __current = __first;

    while (__current != __last) {
        if (!__pred (*__current, __val)) {
            if (__span < __matches + 1)
                return __last;
            __span   -= __matches + 1;
            __matches = 0;
            __first   = ++__current;
        }
        else {
            if (++__matches == __count)
                return __first;
            ++__current;
        }
    }

    return __last;
}

//
// Modifying sequence operations.
//

template <class _Iter, class _OutputIter, class _Predicate, class _TypeT>
_OutputIter replace_copy_if (_Iter __first, _Iter __last,
                             _OutputIter __res, _Predicate __pred,
                             const _TypeT& __new_value)
{
    _RWSTD_ASSERT_RANGE (__first, __last);

    for (; __first != __last; ++__res, ++__first)
        if (__pred (*__first))
            *__res = __new_value;
        else
            *__res = *__first;
    return __res;
}

template <class _InputIter, class _OutputIter, class _TypeT>
_OutputIter remove_copy (_InputIter __first, _InputIter __last,
                         _OutputIter __res, const _TypeT& __val)
{
    _RWSTD_ASSERT_RANGE (__first, __last);

    for (; __first != __last; ++__first)
        if (!(*__first == __val)) {
            *__res = *__first;
            ++__res;
        }
    return __res;
}

template <class _InputIter, class _OutputIter, class _Predicate>
_OutputIter remove_copy_if (_InputIter __first, _InputIter __last,
                            _OutputIter __res, _Predicate __pred)
{
    _RWSTD_ASSERT_RANGE (__first, __last);

    for (; __first != __last; ++__first)
        if (!__pred (*__first)) {
            *__res = *__first;
            ++__res;
        }
    return __res;
}

template <class _InputIter, class _FwdIter>
_FwdIter __unique_copy (_InputIter __first, _InputIter __last,
                        _FwdIter __res, forward_iterator_tag)
{
    _RWSTD_ASSERT_RANGE (__first, __last);

    *__res = *__first;
    while (++__first != __last)
        if (!(*__res == *__first))
            *++__res = *__first;
    return ++__res;
}

template <class _InputIter, class _OutputIter, class _TypeT>
_OutputIter __unique_copy (_InputIter __first, _InputIter __last,
                           _OutputIter __res, _TypeT*)
{
    _RWSTD_ASSERT_RANGE (__first, __last);

    _TypeT __val = *__first;
    *__res = __val;
    while (++__first != __last) {
        if (!(__val == *__first)) {
            __val = *__first;
            *++__res = __val;
        }
    }
    return ++__res;
}

template <class _InputIter, class _FwdIter, class _BinaryPredicate>
_FwdIter __unique_copy (_InputIter __first, _InputIter __last,
                        _FwdIter __res,
                        _BinaryPredicate __pred,
                        forward_iterator_tag)
{
    _RWSTD_ASSERT_RANGE (__first, __last);

    *__res = *__first;
    while (++__first != __last)
        if (!__pred (*__res, *__first))
            *++__res = *__first;
    return ++__res;
}


template <class _InputIter, class _OutputIter, class _BinaryPredicate,
    class _TypeT>
_OutputIter __unique_copy (_InputIter __first, _InputIter __last,
                           _OutputIter __res,
                           _BinaryPredicate __pred, _TypeT*)
{
    _RWSTD_ASSERT_RANGE (__first, __last);

    _TypeT __val = *__first;
    *__res = __val;
    while (++__first != __last)  {
        if (!__pred (__val, *__first))   {
            __val = *__first;
            *++__res = __val;
        }
    }
    return ++__res;
}


template <class _FwdIter, class _Distance>
void __rotate (_FwdIter __first, _FwdIter __middle,
               _FwdIter __last, _Distance*, forward_iterator_tag)
{
    _RWSTD_ASSERT_IN_RANGE (__middle, __first, __last); 

    _FwdIter __i = __middle;
    for (; ; )
    {
        _STD::iter_swap (__first, __i);
        ++__first; ++__i;
        if (__first == __middle)
        {
            if (__i == __last) return;
            __middle = __i;
        }
        else if (__i == __last)
            __i = __middle;
    }
}

template <class _EuclideanRingElement>
_EuclideanRingElement __gcd (_EuclideanRingElement __m,
                            _EuclideanRingElement __n)
{
    while (__n != 0)
    {
        _EuclideanRingElement __r = __m % __n;
        __m = __n;
        __n = __r;
    }
    return __m;
}

template <class _RandomAccessIter, class _Distance, class _TypeT>
void __rotate_cycle (_RandomAccessIter __first, _RandomAccessIter __last,
                     _RandomAccessIter __initial, _Distance __shift, _TypeT*)
{
    _RWSTD_ASSERT_RANGE (__first, __last);

    _TypeT __val = *__initial;
    _RandomAccessIter __ptr1 = __initial;
    _RandomAccessIter __ptr2 = __ptr1 + __shift;
    while (__ptr2 != __initial)
    {
        *__ptr1 = *__ptr2;
        __ptr1 = __ptr2;
        if (__last - __ptr2 > __shift)
            __ptr2 += __shift;
        else
            __ptr2 = __first + (__shift - (__last - __ptr2));
    }
    *__ptr1 = __val;
}

template <class _RandomAccessIter, class _Distance>
void __rotate (_RandomAccessIter __first, _RandomAccessIter __middle,
               _RandomAccessIter __last, _Distance*,
               random_access_iterator_tag)
{
    _RWSTD_ASSERT_IN_RANGE (__middle, __first, __last);
  
    _Distance __n = __gcd (__last - __first, __middle - __first);
    while (__n--)
        __rotate_cycle (__first, __last, __first + __n, __middle - __first,
                       _RWSTD_VALUE_TYPE (_RandomAccessIter));
}

template <class _RandomAccessIter, class _Distance>
void __random_shuffle (_RandomAccessIter __first, _RandomAccessIter __last,
		       _Distance*)
{
    _RWSTD_ASSERT_RANGE (__first, __last);

    if (__first != __last)
	for (_RandomAccessIter __i = __first + 1; __i != __last; ++__i)
	    _STD::iter_swap (__i,
                             __first + _RW::__rw_random (__i - __first + 1));
}

template <class _RandomAccessIter, class _RandomNumberGenerator>
void random_shuffle (_RandomAccessIter __first, _RandomAccessIter __last,
                     _RandomNumberGenerator& __rand)
{
    _RWSTD_ASSERT_RANGE (__first, __last);

    if (! (__first == __last))
        for (_RandomAccessIter __i = __first + 1; __i != __last; ++__i)
            _STD::iter_swap (__i, __first + __rand ((__i - __first) + 1));
}

template <class _BidirIter, class _Predicate>
_BidirIter partition (_BidirIter __first, _BidirIter __last,
                      _Predicate __pred)
{
    _RWSTD_ASSERT_RANGE (__first, __last);

    while (true)
    {
        while (true)
        {
            if (__first == __last)
                return __first;
            else if (__pred (*__first))
                ++__first;
            else
                break;
        }
        --__last;
        while (true)
        {
            if (__first == __last)
                return __first;
            else if (!__pred (*__last))
                --__last;
            else
                break;
        }
        _STD::iter_swap (__first, __last);
        ++__first;
    }
}

template <class _BidirIter, class _Predicate, class _Distance>
_BidirIter __inplace_stable_partition (_BidirIter __first, _BidirIter __last,
                                       _Predicate __pred, _Distance __dist)
{
    _RWSTD_ASSERT_RANGE (__first, __last);

    if (__dist == 1) return __pred (*__first) ? __last : __first;
    _BidirIter __middle = __first;
    _STD::advance (__middle, __dist / 2);
    _BidirIter
        __first_cut = __inplace_stable_partition (__first, __middle, __pred,
                                                  __dist / 2);
    _BidirIter
        __second_cut = __inplace_stable_partition (__middle, __last, __pred,
                                                 __dist - __dist / 2);
    _STD::rotate (__first_cut, __middle, __second_cut);
    __dist = _DISTANCE (__middle, __second_cut, _Distance);
    _STD::advance (__first_cut, __dist);
    return __first_cut;
}

template <class _BidirIter, class _Pointer, class _Predicate,
    class _Distance, class _TypeT>
_BidirIter __stable_partition_adaptive (_BidirIter __first, _BidirIter __last,
                                        _Predicate __pred, _Distance __dist,
                                        _Pointer __buf,
                                        _Distance __buf_size,
                                        _Distance& __fill_pointer, _TypeT*)
{
    _RWSTD_ASSERT_RANGE (__first, __last);

    if (__dist <= __buf_size)
    {
        __dist = 0;
        _BidirIter __res1 = __first;
        _Pointer __res2 = __buf;
        for (; __first != __last && __dist < __fill_pointer; ++__first)
        {
            if (__pred (*__first)) {
                *__res1 = *__first;
                ++__res1;
            }
            else {
                *__res2 = *__first;
                ++__res2;
                ++__dist;
            }
        }
        if (__first != __last)
        {
            raw_storage_iterator<_Pointer, _TypeT> __res3 (__res2);
            for (; __first != __last; ++__first)
            {
                if (__pred (*__first)) {
                    *__res1 = *__first;
                    ++__res1;
                }
                else {
                    *__res3 = *__first;
                    ++__res3;
                    ++__dist;
                }
            }
            __fill_pointer = __dist;
        }
        _STD::copy (__buf, __buf + __dist, __res1);
        return __res1;
    }
    _BidirIter __middle = __first;
    _STD::advance (__middle, __dist / 2);

    // (__dist / 2)'s type may not be the same as that of __dist,
    // hence the seemingly redundant casts below
    _BidirIter __first_cut =
        __stable_partition_adaptive (__first, __middle, __pred,
                                     _Distance (__dist / 2),
                                     __buf, __buf_size,
                                     __fill_pointer, (_TypeT*)0);
    _BidirIter __second_cut =
        __stable_partition_adaptive (__middle, __last, __pred,
                                     _Distance (__dist - __dist / 2),
                                     __buf, __buf_size,
                                     __fill_pointer, (_TypeT*)0);

    _STD::rotate (__first_cut, __middle, __second_cut);
    __dist = _DISTANCE (__middle, __second_cut, _Distance);
    _STD::advance (__first_cut, __dist);
    return __first_cut;
}


template <class _BidirIter, class _Predicate, class _TypeT, class _Distance>
_BidirIter __stable_partition (_BidirIter __first, _BidirIter __last,
                               _Predicate __pred, _TypeT*, _Distance*)
{
    _RWSTD_ASSERT_RANGE (__first, __last);

    _Distance __dist = _DISTANCE (__first, __last, _Distance);

    if (!__dist)
        return __last;
    
    // call an extension of get_temporary_buffer<>() in case partial class
    // specialization (and iterator_traits<>) isn't supported by compiler
    pair<_TypeT*, _Distance> __pair =
        _STD::get_temporary_buffer (__dist, (_TypeT*)0);

    if (__pair.first == 0)
        return __inplace_stable_partition (__first, __last, __pred, __dist);

    _Distance __fill_pointer = 0;
    _BidirIter __res =
        __stable_partition_adaptive (__first, __last, __pred, __dist,
                                     __pair.first, __pair.second,
                                     __fill_pointer, (_TypeT*)0);

    _RW::__rw_destroy (__pair.first, __pair.first + __fill_pointer);
    _STD::return_temporary_buffer (__pair.first);

    return __res;
}

//
// Sorting and related operations.
//

template <class _RandomAccessIter, class _TypeT, class _Compare>
_RandomAccessIter __unguarded_partition (_RandomAccessIter __first,
                                         _RandomAccessIter __last,
                                         _TypeT __pivot, _Compare __comp)
{
    _RWSTD_ASSERT_RANGE (__first, __last);

    while (true) {
        while (__comp (*__first, __pivot))
            ++__first;

        while (__comp (__pivot, *--__last))
            ;

        if (!(__first < __last))
            return __first;

        _STD::iter_swap (__first, __last);
        ++__first;
    }
}


template <class _TypeT, class _Compare>
inline const _TypeT& __median (const _TypeT& __a, const _TypeT& __b,
                               const _TypeT& __c, _Compare __comp)
{
    return __comp (__a, __b)
        ? __comp (__b, __c) ? __b : __comp (__a, __c) ? __c : __a
        : __comp (__a, __c) ? __a : __comp (__b, __c) ? __c : __b;
}


const int __stl_threshold = 16;

template <class _RandomAccessIter, class _TypeT, class _Compare>
void __quick_sort_loop_aux (_RandomAccessIter __first,
                            _RandomAccessIter __last, _TypeT*,
                            _Compare __comp)
{
    _RWSTD_ASSERT_RANGE (__first, __last);

    while (__last - __first > __stl_threshold)
    {
        _RandomAccessIter __cut =
            __unguarded_partition (__first, __last,
                                   _TypeT (__median (*__first,
                                                     * (__first +
                                                        (__last - __first)/2),
                                                     * (__last - 1),
                                                     __comp)),
             __comp);
        if (__cut - __first >= __last - __cut) {
            __quick_sort_loop_aux (__cut, __last,
                                   _RWSTD_VALUE_TYPE (_RandomAccessIter),
                                   __comp);
            __last = __cut;
        }
        else {
            __quick_sort_loop_aux (__first, __cut,
                                   _RWSTD_VALUE_TYPE (_RandomAccessIter),
                                   __comp);
            __first = __cut;
        }
    }
}


template <class _RandomAccessIter, class _Compare>
void __insertion_sort (_RandomAccessIter __first,
                       _RandomAccessIter __last, _Compare __comp)
{
    _RWSTD_ASSERT_RANGE (__first, __last);

    if (! (__first == __last))
        for (_RandomAccessIter __i = __first + 1; __i != __last; ++__i)
            __linear_insert (__first, __i,
                             _RWSTD_VALUE_TYPE (_RandomAccessIter), __comp);
}

template <class _RandomAccessIter, class _TypeT, class _Compare>
void __unguarded_linear_insert (_RandomAccessIter __last, _TypeT __val,
                                _Compare __comp)
{
    for (_RandomAccessIter __next = __last; __comp (__val, *--__next); ) {
        *__last = *__next;
        __last = __next;
    }
    *__last = __val;
}


template <class _RandomAccessIter, class _Compare>
void __final_insertion_sort (_RandomAccessIter __first,
                             _RandomAccessIter __last, _Compare __comp)
{
    _RWSTD_ASSERT_RANGE (__first, __last);

    if (__last - __first > __stl_threshold)
    {
        __insertion_sort (__first, __first + __stl_threshold, __comp);
        __unguarded_insertion_sort (__first + __stl_threshold, __last,
                                    __comp);
    }
    else
        __insertion_sort (__first, __last, __comp);
}


template <class _RandomAccessIter1, class _RandomAccessIter2,
    class _Distance, class _Compare>
void __merge_sort_loop (_RandomAccessIter1 __first,
                        _RandomAccessIter1 __last,
                        _RandomAccessIter2 __res, _Distance __step,
                        _Compare __comp)
{
    _RWSTD_ASSERT_RANGE (__first, __last);

    _Distance __two_step = 2 * __step;

    while (__last - __first >= __two_step) {
        __res = _STD::merge (__first, __first + __step,
                             __first + __step, __first + __two_step, __res,
                             __comp);
        __first += __two_step;
    }
    __step = _STD::min (_Distance (__last - __first), __step);

    _STD::merge (__first, __first + __step, __first + __step, __last,
                 __res, __comp);
}


template <class _RandomAccessIter, class _Distance, class _Compare>
void __chunk_insertion_sort (_RandomAccessIter __first,
                             _RandomAccessIter __last,
                             _Distance __chunk_size, _Compare __comp)
{
    _RWSTD_ASSERT_RANGE (__first, __last);

    while (__last - __first >= __chunk_size)
    {
        __insertion_sort (__first, __first + __chunk_size, __comp);
        __first += __chunk_size;
    }
    __insertion_sort (__first, __last, __comp);
}


template <class _RandomAccessIter, class _Pointer, class _Distance,
    class _TypeT, class _Compare>
void __merge_sort_with_buffer (_RandomAccessIter __first,
                               _RandomAccessIter __last, _Pointer __buf,
                               _Distance*, _TypeT*, _Compare __comp)
{
    _RWSTD_ASSERT_RANGE (__first, __last);

    _Distance __dist = __last - __first;
    _Pointer __buf_last = __buf + __dist;

    const int __stl_chunk_size = 7;

    _Distance __step = __stl_chunk_size;
    __chunk_insertion_sort (__first, __last, __step, __comp);

    while (__step < __dist)
    {
        __merge_sort_loop (__first, __last, __buf, __step, __comp);
        __step *= 2;
        __merge_sort_loop (__buf, __buf_last, __first, __step,
                           __comp);
        __step *= 2;
    }
}


template <class _RandomAccessIter, class _Pointer, class _Distance,
    class _TypeT, class _Compare>
void __stable_sort_adaptive (_RandomAccessIter __first,
                             _RandomAccessIter __last, _Pointer __buf,
                             _Distance __buf_size, _TypeT*,
                             _Compare __comp)
{
    _RWSTD_ASSERT_RANGE (__first, __last);

    _Distance __dist = (__last - __first + 1) / 2;
    _RandomAccessIter __middle = __first + __dist;
    if (__dist > __buf_size)
    {
        __stable_sort_adaptive (__first, __middle, __buf, __buf_size,
                                (_TypeT*)0, __comp);
        __stable_sort_adaptive (__middle, __last, __buf, __buf_size,
                                (_TypeT*)0, __comp);
    }
    else
    {
        __merge_sort_with_buffer (__first, __middle, __buf,
                                 (_Distance*)0, (_TypeT*)0, __comp);
        __merge_sort_with_buffer (__middle, __last, __buf,
                                 (_Distance*)0, (_TypeT*)0, __comp);
    }
    __merge_adaptive (__first, __middle, __last,
                      _Distance (__middle - __first),
                     _Distance (__last - __middle), __buf, __buf_size,
                      (_TypeT*)0, __comp);
}


template <class _RandomAccessIter, class _TypeT, class _Compare>
void __partial_sort (_RandomAccessIter __first, _RandomAccessIter __middle,
                     _RandomAccessIter __last, _TypeT*, _Compare __comp)
{
    _RWSTD_ASSERT_IN_RANGE (__middle, __first, __last);

    _STD::make_heap (__first, __middle, __comp);
    for (_RandomAccessIter __i = __middle; __i < __last; ++__i)
        if (__comp (*__i, *__first))
            __pop_heap (__first, __middle, __i, *__i, __comp,
                        _RWSTD_DIFFERENCE_TYPE (_RandomAccessIter));
    _STD::sort_heap (__first, __middle, __comp);
}


template <class _InputIter, class _RandomAccessIter, class _Compare,
          class _Distance, class _TypeT>
_RandomAccessIter __partial_sort_copy (_InputIter __first,
                                       _InputIter __last,
                                       _RandomAccessIter __first2,
                                       _RandomAccessIter __last2,
                                       _Compare __comp,
                                       _Distance*, _TypeT*)
{
    _RWSTD_ASSERT_RANGE (__first, __last);
    _RWSTD_ASSERT_RANGE (__first2, __last2);

    if (__first2 == __last2) return __last2;
    _RandomAccessIter __res = __first2;
    for (; __first != __last && __res != __last2;
         ++__first, ++__res)
        *__res = *__first;
    _STD::make_heap (__first2, __res, __comp);
    for (; __first != __last; ++__first)
    {
        if (__comp (*__first, *__first2))
            __adjust_heap (__first2, _Distance (), _Distance (__res - __first2),
                           *__first, __comp);
    }
    _STD::sort_heap (__first2, __res, __comp);
    return __res;
}


// David R. Musser's Introspective Sorting algorithm
// O(N * log (N)) worst case complexity
template <class _RandomAccessIter, class _Distance, class _Compare>
void __introsort_loop (_RandomAccessIter __first, _RandomAccessIter __last,
                       _Distance __max_depth, _Compare __comp)
{
    for (; __last - __first > __stl_threshold; __max_depth /= 2) {
        if (0 == __max_depth) {
            __partial_sort (__first, __last, __last,
                            _RWSTD_VALUE_TYPE (_RandomAccessIter), __comp);
            break;
        }

        _RandomAccessIter __cut =
            __unguarded_partition (__first, __last,
                                   __median (*__first,
                                             *(__first + (__last - __first) /2),
                                             *(__last - 1), __comp), __comp);

        // limit the depth of the recursion tree to log2 (last - first)
        // where first and last are the initial values passed in from sort()
        __introsort_loop (__cut, __last, __max_depth, __comp);
        __last = __cut;
    }
}


template <class _RandomAccessIter, class _TypeT, class _Compare>
void __nth_element (_RandomAccessIter __first, _RandomAccessIter __nth,
                    _RandomAccessIter __last, _TypeT*, _Compare __comp)
{
    _RWSTD_ASSERT_IN_RANGE (__nth, __first, __last);

    while (__last - __first > 3)
    {
        _RandomAccessIter __cut =
            __unguarded_partition (__first, __last,
                                   _TypeT (__median (*__first,
                                                     * (__first +
                                                        (__last - __first)/2),
                                                     * (__last - 1),
                                                     __comp)),
             __comp);
        if (__cut <= __nth)
            __first = __cut;
        else
            __last = __cut;
    }
    __insertion_sort (__first, __last, __comp);
}

//
// Binary search.
//

template <class _FwdIter, class _TypeT, class _Compare, class _Distance>
_FwdIter __lower_bound (_FwdIter __first, _FwdIter __last,
                        const _TypeT& __val, _Compare __comp,
                        _Distance*, forward_iterator_tag)
{
    _RWSTD_ASSERT_RANGE (__first, __last);
    
    _Distance __dist = _DISTANCE (__first, __last, _Distance);
    _Distance __half;
    _FwdIter __middle;

    while (__dist > 0)
    {
        __half = __dist / 2;
        __middle = __first;
        _STD::advance (__middle, __half);
        if (__comp (*__middle, __val))
        {
            __first = __middle;
            ++__first;
            __dist = __dist - __half - 1;
        }
        else
            __dist = __half;
    }
    return __first;
}


template <class _RandomAccessIter, class _TypeT, class _Compare,
    class _Distance>
_RandomAccessIter __lower_bound (_RandomAccessIter __first,
                                 _RandomAccessIter __last,
                                 const _TypeT& __val,
                                 _Compare __comp,
                                 _Distance*,
                                 random_access_iterator_tag)
{
    _RWSTD_ASSERT_RANGE (__first, __last);

    _Distance __dist = __last - __first;
    _Distance __half;
    _RandomAccessIter __middle;

    while (__dist > 0)
    {
        __half = __dist / 2;
        __middle = __first + __half;
        if (__comp (*__middle, __val))
        {
            __first = __middle + 1;
            __dist = __dist - __half - 1;
        }
        else
            __dist = __half;
    }
    return __first;
}


template <class _FwdIter, class _TypeT, class _Compare, class _Distance>
_FwdIter __upper_bound (_FwdIter __first, _FwdIter __last,
                        const _TypeT& __val, _Compare __comp,
                        _Distance*, forward_iterator_tag)
{
    _RWSTD_ASSERT_RANGE (__first, __last);

    _Distance __dist = _DISTANCE (__first, __last, _Distance);
    _Distance __half;
    _FwdIter __middle;

    while (__dist > 0)
    {
        __half = __dist / 2;
        __middle = __first;
        _STD::advance (__middle, __half);
        if (__comp (__val, *__middle))
            __dist = __half;
        else {
            __first = __middle;
            ++__first;
            __dist = __dist - __half - 1;
        }
    }
    return __first;
}

template <class _RandomAccessIter, class _TypeT, class _Compare,
    class _Distance>
_RandomAccessIter __upper_bound (_RandomAccessIter __first,
                                 _RandomAccessIter __last,
                                 const _TypeT& __val,
                                 _Compare __comp, _Distance*,
                                 random_access_iterator_tag)
{
    _RWSTD_ASSERT_RANGE (__first, __last);

    _Distance __dist = __last - __first;
    _Distance __half;
    _RandomAccessIter __middle;

    while (__dist > 0)
    {
        __half = __dist / 2;
        __middle = __first + __half;
        if (__comp (__val, *__middle))
            __dist = __half;
        else {
            __first = __middle + 1;
            __dist = __dist - __half - 1;
        }
    }
    return __first;
}


template <class _FwdIter, class _TypeT, class _Compare, class _Distance>
pair<_FwdIter, _FwdIter>
__equal_range (_FwdIter __first, _FwdIter __last, const _TypeT& __val,
               _Compare __comp, _Distance*, forward_iterator_tag)
{
    _RWSTD_ASSERT_RANGE (__first, __last);

    _Distance __dist = _DISTANCE (__first, __last, _Distance);
    _Distance __half;
    _FwdIter __middle, __left, __right;

    while (__dist > 0)
    {
        __half = __dist / 2;
        __middle = __first;
        _STD::advance (__middle, __half);
        if (__comp (*__middle, __val))
        {
            __first = __middle;
            ++__first;
            __dist = __dist - __half - 1;
        }
        else if (__comp (__val, *__middle))
            __dist = __half;
        else
        {
            __left = _STD::lower_bound (__first, __middle, __val, __comp);
            _STD::advance (__first, __dist);
            __right = _STD::upper_bound (++__middle, __first, __val, __comp);
            pair<_FwdIter, _FwdIter> __tmp (__left, __right);
            return __tmp;
        }
    }
    pair<_FwdIter, _FwdIter> __tmp (__first, __first);
    return __tmp;
}

template <class _RandomAccessIter, class _TypeT, class _Compare,
    class _Distance>
pair<_RandomAccessIter, _RandomAccessIter>
__equal_range (_RandomAccessIter __first, _RandomAccessIter __last,
               const _TypeT& __val, _Compare __comp, _Distance*,
               random_access_iterator_tag)
{
    _RWSTD_ASSERT_RANGE (__first, __last);

    _Distance __dist = __last - __first;
    _Distance __half;
    _RandomAccessIter __middle, __left, __right;

    while (__dist > 0)
    {
        __half = __dist / 2;
        __middle = __first + __half;
        if (__comp (*__middle, __val))
        {
            __first = __middle + 1;
            __dist = __dist - __half - 1;
        }
        else if (__comp (__val, *__middle))
            __dist = __half;
        else
        {
            __left  = _STD::lower_bound (__first, __middle, __val, __comp);
            __right = _STD::upper_bound (++__middle, __first + __dist,
                                         __val, __comp);
            pair<_RandomAccessIter, _RandomAccessIter> __tmp(__left, __right);
            return __tmp;
        }
    }
    pair<_RandomAccessIter, _RandomAccessIter> __tmp (__first, __first);
    return __tmp;
}

//
// Merge
//

template <class _InputIter1, class _InputIter2, class _OutputIter,
    class _Compare>
_OutputIter merge (_InputIter1 __first1, _InputIter1 __last1,
                   _InputIter2 __first2, _InputIter2 __last2,
                   _OutputIter __res, _Compare __comp)
{
    _RWSTD_ASSERT_RANGE (__first1, __last1);
    _RWSTD_ASSERT_RANGE (__first2, __last2);

    for (; __first1 != __last1 && __first2 != __last2; ++__res)
    {
        if (__comp (*__first2, *__first1)) {
            *__res = *__first2;
            ++__first2;
        }
        else {
            *__res = *__first1;
            ++__first1;
        }
    }
    return _STD::copy (__first2, __last2, _STD::copy(__first1, __last1, __res));
}


template <class _BidirIter, class _Distance, class _Compare>
void __merge_without_buffer (_BidirIter __first,
                             _BidirIter __middle,
                             _BidirIter __last,
                             _Distance __dist1, _Distance __dist2,
                             _Compare __comp)
{
    _RWSTD_ASSERT_IN_RANGE (__middle, __first, __last);

    if (__dist1 == 0 || __dist2 == 0) return;
    if (__dist1 + __dist2 == 2)
    {
        if (__comp (*__middle, *__first))
            _STD::iter_swap (__first, __middle);
        return;
    }
    _BidirIter __first_cut = __first;
    _BidirIter __second_cut = __middle;
    _Distance __dist11;
    _Distance __dist22;

    if (__dist1 > __dist2) {
        __dist11 = __dist1 / 2;
        _STD::advance (__first_cut, __dist11);
        __second_cut = _STD::lower_bound (__middle, __last,
                                          *__first_cut, __comp);
        __dist22 = _DISTANCE (__middle, __second_cut, _Distance);
    }
    else {
        __dist22 = __dist2 / 2;
        _STD::advance (__second_cut, __dist22);
        __first_cut = _STD::upper_bound (__first, __middle,
                                            *__second_cut, __comp);
        __dist11 = _DISTANCE (__first, __first_cut, _Distance);
    }
    _STD::rotate (__first_cut, __middle, __second_cut);

    _BidirIter __middle2 = __first_cut;

    _STD::advance (__middle2, __dist22);

    __merge_without_buffer (__first, __first_cut, __middle2,
                            __dist11, __dist22, __comp);
    __merge_without_buffer (__middle2, __second_cut, __last,
                            __dist1 - __dist11, __dist2 - __dist22, __comp);
}

template <class _BidirIter1, class _BidirIter2, class _Distance>
_BidirIter1 __rotate_adaptive (_BidirIter1 __first,
                               _BidirIter1 __middle,
                               _BidirIter1 __last,
                               _Distance __dist1, _Distance __dist2,
                               _BidirIter2 __buf,
                               _Distance __buf_size)
{
    _RWSTD_ASSERT_IN_RANGE (__middle, __first, __last);

    _BidirIter2 __buf_end;
    if (__dist1 > __dist2 && __dist2 <= __buf_size)
    {
        __buf_end = _STD::copy (__middle, __last, __buf);
        _STD::copy_backward (__first, __middle, __last);
        return _STD::copy (__buf, __buf_end, __first);
    }
    else if (__dist1 <= __buf_size)
    {
        __buf_end = _STD::copy (__first, __middle, __buf);
        _STD::copy (__middle, __last, __first);
        return _STD::copy_backward (__buf, __buf_end, __last);
    }
    else
    {
        _STD::rotate (__first, __middle, __last);
        _STD::advance (__first, __dist2);
        return __first;
    }
}

template <class _BidirIter1, class _BidirIter2,
    class _BidirIter3>
_BidirIter3 __merge_backward (_BidirIter1 __first1,
                              _BidirIter1 __last1,
                              _BidirIter2 __first2,
                              _BidirIter2 __last2,
                              _BidirIter3 __res)
{
    _RWSTD_ASSERT_RANGE (__first1, __last1);
    _RWSTD_ASSERT_RANGE (__first2, __last2);

    if (__first1 == __last1)
        return _STD::copy_backward (__first2, __last2, __res);

    if (__first2 == __last2)
        return _STD::copy_backward (__first1, __last1, __res);

    --__last1;
    --__last2;
    while (true)
    {
        if (*__last2 < *__last1)
        {
            *--__res = *__last1;
            if (__first1 == __last1)
                return _STD::copy_backward (__first2, ++__last2, __res);
            --__last1;
        }
        else
        {
            *--__res = *__last2;
            if (__first2 == __last2)
                return _STD::copy_backward (__first1, ++__last1, __res);
            --__last2;
        }
    }
}

template <class _BidirIter1, class _BidirIter2,
    class _BidirIter3, class _Compare>
_BidirIter3 __merge_backward (_BidirIter1 __first1,
                              _BidirIter1 __last1,
                              _BidirIter2 __first2,
                              _BidirIter2 __last2,
                              _BidirIter3 __res,
                              _Compare __comp)
{
    _RWSTD_ASSERT_RANGE (__first1, __last1);
    _RWSTD_ASSERT_RANGE (__first2, __last2);

    if (__first1 == __last1)
        return _STD::copy_backward (__first2, __last2, __res);
    if (__first2 == __last2)
        return _STD::copy_backward (__first1, __last1, __res);
    --__last1;
    --__last2;
    while (true)
    {
        if (__comp (*__last2, *__last1))
        {
            *--__res = *__last1;
            if (__first1 == __last1)
                return _STD::copy_backward (__first2, ++__last2, __res);
            --__last1;
        }
        else
        {
            *--__res = *__last2;
            if (__first2 == __last2)
                return _STD::copy_backward (__first1, ++__last1, __res);
            --__last2;
        }
    }
}


template <class _BidirIter, class _Distance, class _Pointer, class _TypeT,
          class _Compare>
void __merge_adaptive (_BidirIter __first, _BidirIter __middle,
                       _BidirIter __last,
                       _Distance __dist1, _Distance __dist2,
                       _Pointer __buf, _Distance __buf_size, _TypeT*,
                       _Compare __comp)
{
    _RWSTD_ASSERT_IN_RANGE (__middle, __first, __last);
    
    if (__dist1 <= __dist2 && __dist1 <= __buf_size)
    {
        _Pointer __buf_end = _STD::copy (__first, __middle, __buf);
        _STD::merge (__buf, __buf_end, __middle, __last, __first, __comp);
    }
    else if (__dist2 <= __buf_size)
    {
        _Pointer __buf_end = _STD::copy (__middle, __last, __buf);
        __merge_backward (__first, __middle, __buf, __buf_end, __last,
                          __comp);
    }
    else
    {
        _BidirIter __first_cut = __first;
        _BidirIter __second_cut = __middle;
        _Distance __dist11;
        _Distance __dist22;
        if (__dist1 > __dist2)
        {
            __dist11 = __dist1 / 2;
            _STD::advance (__first_cut, __dist11);
            __second_cut = _STD::lower_bound (__middle, __last, *__first_cut,
                                              __comp);
            __dist22 = _DISTANCE (__middle, __second_cut, _Distance);
        }
        else
        {
            __dist22 = __dist2 / 2;
            _STD::advance (__second_cut, __dist22);
            __first_cut = _STD::upper_bound (__first, __middle, *__second_cut,
                                                __comp);
            __dist11 = _DISTANCE (__first, __first_cut, _Distance);
        }
        _BidirIter __middle2 =
            __rotate_adaptive (__first_cut, __middle, __second_cut,
                               __dist1 - __dist11, __dist22,
                               __buf, __buf_size);
        __merge_adaptive (__first, __first_cut, __middle2,
                          __dist11, __dist22,
                          __buf,
                          __buf_size, (_TypeT*)0,
                          __comp);
        __merge_adaptive (__middle2, __second_cut, __last,
                          __dist1 - __dist11, __dist2 - __dist22,
                          __buf, __buf_size, (_TypeT*)0, __comp);
    }
}


template <class _BidirIter, class _Distance, class _TypeT, class _Compare>
void __inplace_merge (_BidirIter __first, _BidirIter __middle,
                      _BidirIter __last, _Distance*, _TypeT*, _Compare __comp)
{
    _Distance __dist1 = _DISTANCE (__first, __middle, _Distance);
    _Distance __dist2 = _DISTANCE (__middle, __last, _Distance);
 
    // call an extension of get_temporary_buffer<>() in case partial class
    // specialization (and iterator_traits<>) isn't supported by compiler
    pair<_TypeT*, _Distance> __pair =
        _STD::get_temporary_buffer (__dist1 + __dist2, (_TypeT*)0);

    if (__pair.first == 0) {
        __merge_without_buffer (__first, __middle, __last, __dist1, __dist2,
                                __comp);
    }
    else {
        _Distance __dist = _STD::min (__pair.second, __dist1 + __dist2);
        
        _STD::fill_n (raw_storage_iterator<_TypeT*, _TypeT> (__pair.first),
                      __dist, *__first);
        __merge_adaptive (__first, __middle, __last, __dist1, __dist2,
                          __pair.first, __pair.second,
                          (_TypeT*)0, __comp);
        _RW::__rw_destroy (__pair.first, __pair.first + __dist);
        _STD::return_temporary_buffer (__pair.first);
    }
}

//
// Set operations. Assume sorted sequences.
//


// 25.3.5.1 - returns true iff every (not necessarily distinct) element
// in [first2, last2) occurs (at least as many times) in [first1, last1)
template <class _InputIter1, class _InputIter2, class _Compare>
bool includes (_InputIter1 __first1, _InputIter1 __last1,
               _InputIter2 __first2, _InputIter2 __last2,
               _Compare __comp)
{
    _RWSTD_ASSERT_RANGE (__first1, __last1);
    _RWSTD_ASSERT_RANGE (__first2, __last2);

    while (__first1 != __last1 && __first2 != __last2) {
        if (__comp (*__first2, *__first1))
            return false;

        if (!__comp (*__first1, *__first2))
            ++__first2;

        ++__first1;
    }
    return __first2 == __last2;
}


template <class _InputIter1, class _InputIter2, class _OutputIter,
    class _Compare>
_OutputIter set_union (_InputIter1 __first1, _InputIter1 __last1,
                       _InputIter2 __first2, _InputIter2 __last2,
                       _OutputIter __res, _Compare __comp)
{
    _RWSTD_ASSERT_RANGE (__first1, __last1);
    _RWSTD_ASSERT_RANGE (__first2, __last2);

    for (; __first1 != __last1 && __first2 != __last2; ++__res) {
        if (__comp (*__first1, *__first2)) {
            *__res = *__first1;
            ++__first1;
        }
        else if (__comp (*__first2, *__first1)) {
            *__res = *__first2;
            ++__first2;
        }
        else {
            *__res = *__first1;
            ++__first1;
            ++__first2;
        }
    }
    return _STD::copy (__first2, __last2,
                       _STD::copy (__first1, __last1, __res));
}


template <class _InputIter1, class _InputIter2, class _OutputIter,
    class _Compare>
_OutputIter set_intersection (_InputIter1 __first1, _InputIter1 __last1,
                              _InputIter2 __first2, _InputIter2 __last2,
                              _OutputIter __res, _Compare __comp)
{
    _RWSTD_ASSERT_RANGE (__first1, __last1);
    _RWSTD_ASSERT_RANGE (__first2, __last2);

    while (__first1 != __last1 && __first2 != __last2)
    {
        if (__comp (*__first1, *__first2))
            ++__first1;
        else if (__comp (*__first2, *__first1))
            ++__first2;
        else {
            *__res = *__first1;
            ++__res;
            ++__first1;
            ++__first2;
        }
    }
    return __res;
}


template <class _InputIter1, class _InputIter2, class _OutputIter,
    class _Compare>
_OutputIter set_difference (_InputIter1 __first1, _InputIter1 __last1,
                            _InputIter2 __first2, _InputIter2 __last2,
                            _OutputIter __res, _Compare __comp)
{
    _RWSTD_ASSERT_RANGE (__first1, __last1);
    _RWSTD_ASSERT_RANGE (__first2, __last2);

    while (__first1 != __last1 && __first2 != __last2)
    {
        if (__comp (*__first1, *__first2)) {
            *__res = *__first1;
            ++__res;
            ++__first1;
        }
        else if (__comp (*__first2, *__first1))
            ++__first2;
        else
        {
            ++__first1;
            ++__first2;
        }
    }
    return _STD::copy (__first1, __last1, __res);
}


template <class _InputIter1, class _InputIter2, class _OutputIter,
    class _Compare>
_OutputIter set_symmetric_difference (_InputIter1 __first1,
                                      _InputIter1 __last1,
                                      _InputIter2 __first2,
                                      _InputIter2 __last2,
                                      _OutputIter __res,
                                      _Compare __comp)
{
    _RWSTD_ASSERT_RANGE (__first1, __last1);
    _RWSTD_ASSERT_RANGE (__first2, __last2);

    while (__first1 != __last1 && __first2 != __last2)
    {
        if (__comp (*__first1, *__first2)) {
            *__res = *__first1;
            ++__res;
            ++__first1;
        }
        else if (__comp (*__first2, *__first1)) {
            *__res = *__first2;
            ++__res;
            ++__first2;
        }
        else      {
            ++__first1;
            ++__first2;
        }
    }
    return _STD::copy (__first2, __last2, _STD::copy(__first1, __last1, __res));
}

//
// Heap operations.
//

template <class _RandomAccessIter, class _Distance, class _TypeT,
          class _Compare>
void __push_heap (_RandomAccessIter __first, _Distance __holeIndex,
                  _Distance __topIndex, _TypeT __val, _Compare __comp)
{
    for (_Distance __parent = (__holeIndex - 1) / 2; 
         __holeIndex > __topIndex && __comp (* (__first + __parent), __val);
         __parent = ((__holeIndex = __parent) - 1) / 2) {
        *(__first + __holeIndex) = *(__first + __parent);
    }
    *(__first + __holeIndex) = __val;
}


template <class _RandomAccessIter, class _Distance, class _TypeT,
    class _Compare>
void __adjust_heap (_RandomAccessIter __first, _Distance __holeIndex,
                    _Distance __dist, _TypeT __val, _Compare __comp)
{
    _Distance __topIndex = __holeIndex;
    _Distance __2ndChild;

    while ((__2ndChild = 2 * __holeIndex + 2) < __dist) {
        if (__comp (*(__first + __2ndChild), *(__first + (__2ndChild - 1))))
            __2ndChild--;
        *(__first + __holeIndex) = *(__first + __2ndChild);
        __holeIndex = __2ndChild;
    }
    if (__2ndChild == __dist) {
        *(__first + __holeIndex) = *(__first + (__2ndChild - 1));
        __holeIndex = __2ndChild - 1;
    }
    __push_heap (__first, __holeIndex, __topIndex, __val, __comp);
}


template <class _RandomAccessIter, class _Compare, class _TypeT,
    class _Distance>
void __make_heap (_RandomAccessIter __first, _RandomAccessIter __last,
                  _Compare __comp, _TypeT*, _Distance*)
{
    _RWSTD_ASSERT_RANGE (__first, __last);

    _Distance __dist = __last - __first;
    _Distance __parent = (__dist - 2)/2;
    while (true)
    {
        __adjust_heap (__first, __parent, __dist, * (__first + __parent),
                       __comp);
        if (__parent == 0)
            return;
        __parent--;
    }
}


//
// Minimum and maximum.
//


template <class _FwdIter, class _Compare>
_FwdIter min_element (_FwdIter __first, _FwdIter __last, _Compare __comp)
{
    _RWSTD_ASSERT_RANGE (__first, __last);

    if (__first == __last)
        return __first;

    _FwdIter __res = __first;
    while (++__first != __last)
        if (__comp (*__first, *__res))
            __res = __first;

    return __res;
}


template <class _FwdIter, class _Compare>
_FwdIter max_element (_FwdIter __first, _FwdIter __last, _Compare __comp)
{
    _RWSTD_ASSERT_RANGE (__first, __last);

    if (__first == __last)
        return __first;

    _FwdIter __res = __first;
    while (++__first != __last)
        if (__comp (*__res, *__first))
            __res = __first;

    return __res;
}

//
// Permutations.
//

template <class _BidirIter, class _Compare>
bool next_permutation (_BidirIter __first, _BidirIter __last, _Compare __comp)
{
    _RWSTD_ASSERT_RANGE (__first, __last);

    if (__first == __last) return false;
    _BidirIter __i = __first;
    ++__i;
    if (__i == __last) return false;
    __i = __last;
    --__i;

    for (; ; )
    {
        _BidirIter __ii = __i--;
        if (__comp (*__i, *__ii))
        {
            _BidirIter __j = __last;
            while (!__comp (*__i, *--__j))
                ;
            _STD::iter_swap (__i, __j);
            _STD::reverse (__ii, __last);
            return true;
        }
        if (__i == __first)
        {
            _STD::reverse (__first, __last);
            return false;
        }
    }
}


template <class _BidirIter, class _Compare>
bool prev_permutation (_BidirIter __first, _BidirIter __last, _Compare __comp)
{
    _RWSTD_ASSERT_RANGE (__first, __last);

    if (__first == __last) return false;
    _BidirIter __i = __first;
    ++__i;
    if (__i == __last) return false;
    __i = __last;
    --__i;

    for (; ; )
    {
        _BidirIter __ii = __i--;
        if (__comp (*__ii, *__i))
        {
            _BidirIter __j = __last;
            while (!__comp (*--__j, *__i))
                ;
            _STD::iter_swap (__i, __j);
            _STD::reverse (__ii, __last);
            return true;
        }
        if (__i == __first)
        {
            _STD::reverse (__first, __last);
            return false;
        }
    }
}


_RWSTD_NAMESPACE_END   // std
