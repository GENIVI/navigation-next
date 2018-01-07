/***************************************************************************
 *
 * deque.cc - Non-iniline definitions for the Standard Library deque class
 *
 * $Id: deque.cc 91768 2005-08-10 18:58:48Z drodgman $
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

_RWSTD_NAMESPACE_BEGIN (std)

template <class _TypeT, class _Allocator>
void deque<_TypeT, _Allocator>::_C_alloc_at_begin ()
{
    pointer __ptr =
        _RWSTD_VALUE_ALLOC (_C_value_alloc_type,
                            allocate (_C_bufsize (),
                                      _C_begin._C_current));
    if (!empty ()) {
        if (_C_begin._C_node == _C_map) {
            _C_map_alloc_type __alloc (*this);

            difference_type __dist = _C_end._C_node
                - _C_begin._C_node;

            size_type new_map_size = (__dist + 1) * 2;
            _C_map_pointer __tmp;
            _TRY {
                __tmp = __alloc.allocate (new_map_size, _C_map);
            }
            _CATCH (...) {
                _RWSTD_VALUE_ALLOC (_C_value_alloc_type,
                                    deallocate (__ptr, _C_bufsize ()));
                _RETHROW;
            }      
            copy (_C_begin._C_node,
                  _C_end._C_node + 1,
                  __tmp + new_map_size / 4 + 1);
            __alloc.deallocate (_C_map, _C_map_size);
            _C_map = __tmp;
            _C_map[new_map_size / 4] = __ptr;

            _C_begin = _C_deque_iter (__ptr + _C_bufsize (), 
                                     _C_map + new_map_size / 4);

            _C_end = _C_deque_iter (_C_end._C_current, 
                                   _C_map + new_map_size / 4 + __dist + 1);

            _C_map_size = new_map_size;
        }
        else {
            --_C_begin._C_node;
            *_C_begin._C_node = __ptr;
            _C_begin = _C_deque_iter (__ptr + _C_bufsize (),
                                     _C_begin._C_node);
        }
    }
    else {
        _TRY {
            _C_map = _C_map_alloc_type (*this).allocate (_C_bufsize (), 
                                                               _C_map);
        }
        _CATCH (...) {
            _RWSTD_VALUE_ALLOC (_C_value_alloc_type,
                                deallocate (__ptr, _C_bufsize ()));
            _RETHROW;
        }      
        _C_map_size = _C_bufsize ();
        _C_map[_C_map_size / 2] = __ptr;
        _C_begin = _C_deque_iter (__ptr + _C_bufsize () / 2 + 1, 
                                 _C_map + _C_map_size / 2);
        _C_end = _C_begin;
    }
}


template <class _TypeT, class _Allocator>
void deque<_TypeT, _Allocator>::_C_alloc_at_end ()
{
    pointer __ptr =
        _RWSTD_VALUE_ALLOC (_C_value_alloc_type,
                            allocate (_C_bufsize (),
                                      _C_begin._C_current));

    if (!empty ()) {
        if (_C_end._C_node == _C_map + _C_map_size - 1) {
            _C_map_alloc_type __alloc (*this);
            difference_type __dist = _C_end._C_node
                - _C_begin._C_node;
            size_type new_map_size = (__dist + 1) * 2;
            _C_map_pointer __tmp;
            _TRY {
                __tmp = __alloc.allocate (new_map_size, _C_map);
            }
            _CATCH (...) {
                _RWSTD_VALUE_ALLOC (_C_value_alloc_type,
                                    deallocate (__ptr, _C_bufsize ()));
                _RETHROW;
            }      
            copy (_C_begin._C_node,
                  _C_end._C_node + 1,
                  __tmp + new_map_size / 4);
            __alloc.deallocate (_C_map, _C_map_size);
            _C_map = __tmp;
            _C_map[new_map_size / 4 + __dist + 1] = __ptr;
            _C_begin = _C_deque_iter (_C_begin._C_current,
                                     _C_map + new_map_size / 4);
            _C_end = _C_deque_iter (__ptr, _C_map + new_map_size / 4 + __dist + 1);
            _C_map_size = new_map_size;
        }
        else {
            ++_C_end._C_node;
            *_C_end._C_node = __ptr;
            _C_end = _C_deque_iter (__ptr, _C_end._C_node);
        }
    }
    else {
        _C_map_size = _C_bufsize ();
        _TRY {
            _C_map =
                _C_map_alloc_type (*this).allocate (_C_map_size, _C_map);
        }
        _CATCH (...) {
            _RWSTD_VALUE_ALLOC (_C_value_alloc_type,
                                deallocate (__ptr, _C_bufsize ()));
            _RETHROW;
        }      
        _C_map[_C_map_size / 2] = __ptr;
        _C_begin = _C_deque_iter (__ptr + _C_bufsize () / 2, 
                                 _C_map + _C_map_size / 2);
        _C_end = _C_begin;
    }
}


template <class _TypeT, class _Allocator>
void deque<_TypeT, _Allocator>::_C_free_at_begin ()
{
    _RWSTD_VALUE_ALLOC (_C_value_alloc_type,
                        deallocate (*_C_begin._C_node++,
                                    _C_bufsize ()));
    if (empty ()) {
        _C_begin = _C_end = _C_deque_iter (0, 0);
        _C_map_alloc_type (*this).deallocate (_C_map, _C_map_size);
    }
    else
        _C_begin = _C_deque_iter (*_C_begin._C_node,
                                 _C_begin._C_node);
}


template <class _TypeT, class _Allocator>
void deque<_TypeT, _Allocator>::_C_free_at_end ()
{
    _RWSTD_VALUE_ALLOC (_C_value_alloc_type,
                        deallocate (*_C_end._C_node--,
                                    _C_bufsize ()));
    if (empty ()) {
        _C_begin = _C_end = _C_deque_iter (0, 0);
        _C_map_alloc_type (*this).deallocate (_C_map, _C_map_size);
    }
    else
        _C_end = _C_deque_iter (*_C_end._C_node
                               + _C_bufsize (),
                               _C_end._C_node);
}


template <class _TypeT, class _Allocator>
_TYPENAME deque<_TypeT, _Allocator>::iterator 
deque<_TypeT, _Allocator>::insert (iterator __pos, const_reference __val)
{
    _RWSTD_ASSERT_RANGE (begin (), __pos);

    if (__pos == begin ()) {
        push_front (__val);
        return begin ();
    }

    if (__pos == end ()) {
        push_back (__val);
        return end () - 1;
    }

    difference_type __inx = __pos - begin ();
    if ((size_type)__inx < size ()/2) {
        push_front (*begin ());
        copy (begin () + 2, begin () + __inx + 1, begin () + 1);
    }
    else {
        push_back (*(end () - 1));
        copy_backward (begin () + __inx, end () - 2, end () - 1);
    }
    *(begin () + __inx) = __val;
    return begin () + __inx;
}


template <class _TypeT, class _Allocator>
void deque<_TypeT, _Allocator>::
__insert_aux (iterator __pos, size_type __n, const_reference __val)
{
    _RWSTD_ASSERT_RANGE (begin (), __pos);

    difference_type __inx = __pos - begin ();
    difference_type __rem = size () - __inx;

    if (__rem > __inx) {
        if (__n > (size_type)__inx) {
            
            for (difference_type __i = __n - __inx; __i > 0; --__i)
                push_front (__val);

            for (difference_type __j = __inx; __j; --__j)
                push_front (*(begin () + (__n - 1)));

            fill (begin () + __n, begin () + (__n + __inx), __val);
        }
        else {
            for (difference_type __i = __n; __i; --__i)
                push_front (*(begin () + __n - 1));

            copy (begin () + __n + __n, begin () + __n + __inx,
                  begin () + __n);
            fill (begin () + __inx, begin () + __n + __inx, __val);
        }
    }
    else {
        difference_type orig_len = __inx + __rem;
        if (__n > (size_type)__rem) {
            for (difference_type __i = __n - __rem; __i > 0; --__i)
                push_back (__val);

            for (difference_type __j = 0; __j < __rem; ++__j)
                push_back (*(begin () + __inx + __j));

            fill (begin () + __inx, begin () + orig_len, __val);
        }
        else {
            difference_type __i = 0;
            while ((size_type)__i < __n)
                push_back (*(begin () + orig_len - __n + __i++));
            copy_backward (begin () + __inx, begin () + orig_len - __n, 
                           begin () + orig_len);
            fill (begin () + __inx, begin () + __inx + __n, __val);
        }
    }
}


#ifndef _RWSTD_NO_MEMBER_TEMPLATES

template <class _TypeT, class _Allocator>
template <class _InputIter>
void deque<_TypeT, _Allocator>::
__insert_aux2 (iterator __pos, _InputIter __first, _InputIter __last)
{
    _RWSTD_ASSERT_RANGE (begin (), __pos);
    _RWSTD_ASSERT_RANGE (__first, __last);

    difference_type __inx = __pos - begin ();
    difference_type __rem = size () - __inx;
    size_type         __n = _DISTANCE (__first, __last, size_type);

    if (__rem > __inx)
    {
      if (__n > (size_type)__inx)
      {
        // FIXME: operator-() being applied to InputIterator
        //        assumes RandomAccessIterator
        _InputIter __m = __last - __inx;
        while (__m != __first)
            push_front (*--__m);
        difference_type __i = __inx;
        while (__i--)
            push_front (*(begin () + __n - 1));
        copy (__last - __inx, __last, begin () + __n);
      }
      else
      {
        difference_type __i = __n;
        while (__i--)
            push_front (*(begin () + __n - 1));
        copy (begin () + __n + __n, begin () + __n + __inx, begin () + __n);
        copy (__first, __last, begin () + __inx);
      }
    }
    else
    {
      difference_type orig_len = __inx + __rem;
      if (__n > (size_type)__rem)
      {
        _InputIter __m = __first + __rem;
        for ( ; __m != __last; ++__m )
            push_back (*__m);
        difference_type __i = 0;
        while (__i < __rem)
            push_back (*(begin () + __inx + __i++));
        copy (__first, __first + __rem, begin () + __inx);
      }
      else
      {
        difference_type __i = 0;
        while ((size_type)__i < __n)
            push_back (*(begin () + orig_len - __n + __i++));
        copy_backward (begin () + __inx, begin () + orig_len - __n, 
                      begin () + orig_len);
        copy (__first, __last, begin () + __inx);
      }
    }
  }

#endif
#ifdef _RWSTD_NO_MEMBER_TEMPLATES

template<class _TypeT, class _Allocator>
void deque<_TypeT, _Allocator>::
insert (iterator __pos, const_iterator __first, const_iterator __last)
{
    _RWSTD_ASSERT_RANGE (begin (), __pos);
    _RWSTD_ASSERT_RANGE (__first, __last);

    difference_type __inx = __pos - begin ();
    difference_type __rem = size () - __inx;
    size_type         __n = _DISTANCE (__first, __last, size_type);

    if (__rem > __inx)
    {
      if (__n > (size_type)__inx)
      {
        const_iterator __m = __last - __inx;
        while (__m != __first)
            push_front (*--__m);
        difference_type __i = __inx;
        while (__i--)
            push_front (*(begin () + __n - 1));
        copy (__last - __inx, __last, begin () + __n);
      }
      else
      {
        difference_type __i = __n;
        while (__i--)
            push_front (*(begin () + __n - 1));
        copy (begin () + __n + __n, begin () + __n + __inx, begin () + __n);
        copy (__first, __last, begin () + __inx);
      }
    }
    else
    {
      difference_type orig_len = __inx + __rem;
      if (__n > (size_type)__rem)
      {
        const_iterator __m = __first + __rem;
        for ( ; __m != __last; ++__m )
            push_back (*__m);
        difference_type __i = 0;
        while (__i < __rem)
            push_back (*(begin () + __inx + __i++));
        copy (__first, __first + __rem, begin () + __inx);
      }
      else
      {
        difference_type __i = 0;
        while ((size_type)__i < __n)
            push_back (*(begin () + orig_len - __n + __i++));
        copy_backward (begin () + __inx, begin () + orig_len - __n, 
                      begin () + orig_len);
        copy (__first, __last, begin () + __inx);
      }
    }
  }

template<class _TypeT, class _Allocator>
void deque<_TypeT, _Allocator>::
insert (iterator __pos, const_pointer __first, const_pointer __last)
{
    _RWSTD_ASSERT_RANGE (begin (), __pos);
    _RWSTD_ASSERT_RANGE (__first, __last);

    difference_type __inx = __pos - begin ();
    difference_type __rem = size () - __inx;
    size_type         __n = _DISTANCE (__first, __last, size_type);

    if (__rem > __inx)
    {
      if (__n > (size_type)__inx)
      {
        const_pointer __m = __last - __inx;
        while (__m != __first)
            push_front (*--__m);
        difference_type __i = __inx;
        while (__i--)
            push_front (*(begin () + __n - 1));
        copy (__last - __inx, __last, begin () + __n);
      }
      else
      {
        difference_type __i = __n;
        while (__i--) push_front (*(begin () + __n - 1));
        copy (begin () + __n + __n, begin () + __n + __inx, begin () + __n);
        copy (__first, __last, begin () + __inx);
      }
    }
    else
    {
      difference_type orig_len = __inx + __rem;
      if (__n > (size_type)__rem)
      {
        const_pointer __m = __first + __rem;
        for ( ; __m != __last; ++__m )
            push_back (*__m);
        difference_type __i = 0;
        while (__i < __rem)
            push_back (*(begin () + __inx + __i++));
        copy (__first, __first + __rem, begin () + __inx);
      }
      else
      {
        difference_type __i = 0;
        while ((size_type)__i < __n)
            push_back (*(begin () + orig_len - __n + __i++));
        copy_backward (begin () + __inx, begin () + orig_len - __n, 
                      begin () + orig_len);
        copy (__first, __last, begin () + __inx);
      }
    }
}

#endif /*_RWSTD_NO_MEMBER_TEMPLATES*/


template <class _TypeT, class _Allocator>
_TYPENAME deque<_TypeT, _Allocator>::iterator 
deque<_TypeT, _Allocator>::erase (iterator __pos)
{
    _RWSTD_ASSERT_RANGE (begin (), __pos);

    if (end () - __pos > __pos - begin ()) {
        copy_backward (begin (), __pos, __pos + 1); 
        pop_front ();

        return begin () == end () ? end () : __pos + 1;
    }

    copy (__pos + 1, end (), __pos); 
    pop_back ();
    return __pos;
}
    

template <class _TypeT, class _Allocator>
_TYPENAME deque<_TypeT, _Allocator>::iterator 
deque<_TypeT, _Allocator>::erase (iterator __first, iterator __last)
{
    _RWSTD_ASSERT_RANGE (__first, __last);
    _RWSTD_ASSERT_RANGE (begin (), __first);

    difference_type __n = __last - __first;

    if (end () - __last > __first - begin ()) {
        copy_backward (begin (), __first, __last);
        while (__n-- > 0)
            pop_front ();
        return __last;
    }

    copy (__last, end (), __first);
    while (__n-- > 0)
        pop_back ();
    return __first;
}


_RWSTD_NAMESPACE_END   // std
