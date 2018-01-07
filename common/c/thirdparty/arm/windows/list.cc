/***************************************************************************
 *
 * list.cc - Non-nline list definitions for the Standard Library
 *
 * $Id: list.cc 91768 2005-08-10 18:58:48Z drodgman $
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

#define _ITER_NODE(it)   (_ITER_BASE (it)._C_node)

_RWSTD_NAMESPACE_BEGIN (std)


template <class _TypeT, class _Allocator>
void list<_TypeT, _Allocator>::_C_add_buffer (bool __is_list_empty)
{
    // empty list gets one node
    size_t __next_buffer_size = 1;
    if(!__is_list_empty) {
        if ((void*)_C_buflist) {
            __next_buffer_size = 
                _RW::__rw_new_capacity(_C_buflist->_C_bufsize,this);
        }
        else {
            __next_buffer_size = _RW::__rw_new_capacity(0,this);
        }          
     }
    _C_buf_pointer __tmp = 
        _C_buf_alloc_type (*this).allocate (1, (void*)_C_buflist);

    _TRY {
        __tmp->_C_buffer = 
           _C_node_alloc_type (*this).allocate (__next_buffer_size,
                                                (void*)_C_last);
    }
    _CATCH (...) {
        _C_buf_alloc_type (*this).deallocate (__tmp, 1);
    }

    __tmp->_C_next_buf = _C_buflist;
    __tmp->_C_bufsize  = __next_buffer_size;

    _C_buflist    = __tmp;
    _C_next_avail = _C_buflist->_C_buffer; 
    _C_last       = _C_next_avail + __next_buffer_size;
}


template <class _TypeT, class _Allocator>
void list<_TypeT, _Allocator>::_C_free_buffers ()
{
    while ((void*)_C_buflist) {
        _C_buf_pointer __tmp = _C_buflist;
        _C_buflist = _C_buflist->_C_next_buf;
        _C_node_alloc_type (*this).deallocate (__tmp->_C_buffer, 
                                               __tmp->_C_bufsize);
        _C_buf_alloc_type (*this).deallocate (__tmp, 1);
    }
    _C_free_list  = 0;
    _C_next_avail = 0;
    _C_last       = 0;
}


template <class _TypeT, class _Allocator>
void list<_TypeT, _Allocator>::_C_transfer (iterator __it, iterator __first, 
                                           iterator __last, list& __x)
{
    _RWSTD_ASSERT_RANGE (begin (), __it);
    _RWSTD_ASSERT_RANGE (__first, __last);

    if (this == &__x) {
        (*(_C_link_type ((*_ITER_NODE (__last))._C_prev)))._C_next =
            _ITER_NODE (__it);
        (*(_C_link_type ((*_ITER_NODE (__first))._C_prev)))._C_next =
            _ITER_NODE (__last);
        (*(_C_link_type ((*_ITER_NODE (__it))._C_prev)))._C_next =
            _ITER_NODE (__first);
        
        _C_link_type __tmp = _C_link_type ((*_ITER_NODE (__it))._C_prev);
        
        (*_ITER_NODE (__it))._C_prev    = (*_ITER_NODE (__last))._C_prev;
        (*_ITER_NODE (__last))._C_prev  = (*_ITER_NODE (__first))._C_prev; 
        (*_ITER_NODE (__first))._C_prev = __tmp;
    }
    else {
        insert (__it, __first, __last);
        __x.erase (__first, __last);
    }
}


template <class _TypeT, class _Allocator>
void list<_TypeT, _Allocator>::resize (size_type __size, _TypeT __val)
{
    if (__size > size ())
        insert (end (), __size - size (), __val);
    else if (__size < size ()) {
        iterator __tmp = begin ();
        advance (__tmp, __size);
        erase (__tmp, end ());
    }
}


template <class _TypeT, class _Allocator>
list<_TypeT, _Allocator>&
list<_TypeT, _Allocator>::operator= (const list<_TypeT, _Allocator>& __rhs)
{
    if (this != &__rhs) {
        iterator       __first1 = begin ();
        iterator       __last1  = end ();
        const_iterator __first2 = __rhs.begin ();
        const_iterator __last2  = __rhs.end ();

        for ( ; __first1 != __last1 && __first2 != __last2;
              ++__first1, ++__first2)
            *__first1 = *__first2;

        if (__first2 == __last2)
            erase (__first1, __last1);
        else
            insert (__last1, __first2, __last2);
    }
    return *this;
}

template <class _TypeT, class _Allocator>
void list<_TypeT, _Allocator>::remove (const_reference __val)
{
    for (iterator __first = begin (), __last = end (); __first != __last; ) {
        iterator __next = __first;
        ++__next;
        if (*__first == __val)
            erase (__first);
        __first = __next;
    }
}


template <class _TypeT, class _Allocator>
void list<_TypeT, _Allocator>::unique ()
{
    iterator __first = begin ();
    iterator __last  = end ();

    if (__first == __last)
        return;

    iterator __next = __first;

    while (++__next != __last) {
        if (*__first == *__next)
            erase (__next);
        else
            __first = __next;
        __next = __first;
    }
}


template <class _TypeT, class _Allocator>
void list<_TypeT, _Allocator>::merge (list<_TypeT, _Allocator>& __x)
{
    iterator __first1 = begin ();
    iterator __last1  = end ();
    iterator __first2 = __x.begin ();
    iterator __last2  = __x.end ();

    while (__first1 != __last1 && __first2 != __last2) {
      if (*__first2 < *__first1) {
          iterator __next = __first2;
          _C_transfer (__first1, __first2, ++__next, __x);
          __first2 = __next;
      }
      else
        ++__first1;
    }

    if (__first2 != __last2) 
        _C_transfer (__last1, __first2, __last2, __x);
}


template <class _TypeT, class _Allocator>
void list<_TypeT, _Allocator>::reverse ()
{
    if (size () < 2)
        return;

    for (iterator __first = ++begin (); __first != end ();) {
        iterator __tmp = __first;
        _C_transfer (begin (), __tmp, ++__first, *this);
    }
}

// sorts list by moving nodes within list; preserves iterators pointing to
// elements of the list.
template <class _TypeT, class _Allocator>
void list<_TypeT, _Allocator>::sort ()
{
    for (size_type __n = 1; __n < size (); __n *= 2) {
        iterator __i1 = begin (),
                 __i2 = begin (),
                 __i3 = begin ();

        _C_advance (__i2, (difference_type)__n, end ());
        _C_advance (__i3, (difference_type)(2 * __n), end ());

        for (size_type __m = 0;
             __m < (size () + (2 * __n)) / (__n * 2); __m++) {

            if (__i1 != end () && __i2 != end ()) {
                _C_adjacent_merge (__i1, __i2, __i3);
                __i1 = __i2 = __i3;
                _C_advance (__i2, (difference_type) __n, end ());
                _C_advance (__i3, (difference_type) 2 * __n, end ());
            }
        }
    }
}


#ifndef _RWSTD_NO_MEMBER_TEMPLATES

template<class _TypeT, class _Allocator>
template<class _Predicate>
void list<_TypeT, _Allocator>::remove_if (_Predicate __pred)

#else   // if defined (_RWSTD_NO_MEMBER_TEMPLATES)

template<class _TypeT, class _Allocator>
void list<_TypeT, _Allocator>::remove_if (bool (*__pred)(const_reference))

#endif   // _RWSTD_NO_MEMBER_TEMPLATES

{
    iterator __first = begin ();
    iterator __last = end ();

    while (__first != __last) {
        iterator __next = __first;
        ++__next;
        if (__pred (*__first))
            erase (__first);
        __first = __next;
    }
}

#ifndef _RWSTD_NO_MEMBER_TEMPLATES

template<class _TypeT, class _Allocator>
template<class _BinaryPredicate>
void list<_TypeT, _Allocator>::unique (_BinaryPredicate __pred)

#else   // if defined (_RWSTD_NO_MEMBER_TEMPLATES)

template<class _TypeT, class _Allocator>
void list<_TypeT, _Allocator>::
unique (bool (*__pred)(const_reference, const_reference))

#endif   // _RWSTD_NO_MEMBER_TEMPLATES
{
    iterator __first = begin ();
    iterator __last = end ();

    if (__first == __last)
        return;

    iterator __next = __first;
    while (++__next != __last) {
        if (__pred (*__first, *__next))
            erase (__next);
        else
            __first = __next;
        __next = __first;
    }
}


#ifndef _RWSTD_NO_MEMBER_TEMPLATES

template<class _TypeT, class _Allocator>
template<class _Compare>
void list<_TypeT, _Allocator>::
merge (list<_TypeT, _Allocator>& __x, _Compare __cmp)

#else   // if defined (_RWSTD_NO_MEMBER_TEMPLATES)

template<class _TypeT, class _Allocator>
void list<_TypeT, _Allocator>::
merge (list<_TypeT, _Allocator>& __x,
       bool (*__cmp)(const_reference, const_reference))

#endif   // _RWSTD_NO_MEMBER_TEMPLATES
{
    iterator __first1 = begin ();
    iterator __last1  = end ();
    iterator __first2 = __x.begin ();
    iterator __last2  = __x.end ();

    while (__first1 != __last1 && __first2 != __last2) {
      if (__cmp (*__first2, *__first1)) {
          iterator __next = __first2;
          _C_transfer (__first1, __first2, ++__next, __x);
          __first2 = __next;
      }
      else
          ++__first1;
    }

    if (__first2 != __last2) 
        _C_transfer (__last1, __first2, __last2, __x);
}


#ifndef _RWSTD_NO_MEMBER_TEMPLATES

template <class _TypeT, class _Allocator>
template<class _Compare>
void list<_TypeT, _Allocator>::sort (_Compare __cmp)

#else   // if defined (_RWSTD_NO_MEMBER_TEMPLATES)

template <class _TypeT, class _Allocator>
void list<_TypeT, _Allocator>::
sort (bool (*__cmp)(const_reference, const_reference))

#endif   // _RWSTD_NO_MEMBER_TEMPLATES
{
    for (size_type __n = 1; __n < size (); __n *= 2) {

        iterator __it1 = begin (), 
                 __it2 = begin (), 
                 __it3 = begin ();

        _C_advance (__it2, __n, end ());
        _C_advance (__it3, 2 * __n, end ());

        for (size_type __m = 0;
             __m != (size () + (2 * __n)) / (__n * 2); ++__m) {
            
            if (__it1 != end () && __it2 != end ()) {
                _C_adjacent_merge (__it1, __it2, __it3, __cmp);
                __it1 = __it3;
                __it2 = __it3;
                _C_advance (__it2, __n, end ());
                _C_advance (__it3, 2 * __n, end ());
            }
        }
    }
}


_RWSTD_NAMESPACE_END   // std


// clean up
#undef _ITER_NODE
