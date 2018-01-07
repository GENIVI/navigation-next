/***************************************************************************
 *
 * vector.cc - Non-inline definitions for the Standard Library vector class
 *
 * $Id: vector.cc 91768 2005-08-10 18:58:48Z drodgman $
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
vector<_TypeT,_Allocator>&
vector<_TypeT,_Allocator>::operator= (const vector<_TypeT,_Allocator>& __x)
{
    if (&__x == this)
        return *this;

    if (__x.size() > capacity())  {
        size_t __new_capacity = 
            max (__x.size(), (size_t)_RW::__rw_new_capacity(size(),this));
        pointer __start = _RWSTD_VALUE_ALLOC(_C_value_alloc_type,
                                             allocate(__new_capacity,0));
        
        _TRY {
            uninitialized_copy(__x.begin(), __x.end(), __start,
                               _RWSTD_VALUE_ALLOC_CAST(*this));
        }
        _CATCH (...) {
            _C_destroy (_C_make_iter (__start),
                        _C_make_iter (__start + __x.size()));
            _RWSTD_VALUE_ALLOC(_C_value_alloc_type,
                               deallocate (__start,__x.size()));
            _RETHROW;
        }
        
        _C_destroy(begin (), end ());
        _RWSTD_VALUE_ALLOC(_C_value_alloc_type,
                           deallocate(_C_start, capacity()));
        _C_start = __start;
        _C_finish = _C_start + __x.size();
        _C_end_of_storage = _C_start + __new_capacity;
    }
    else if (size() >= __x.size()) {
        iterator __it = copy (__x.begin (), __x.end (), begin ());
        _C_destroy (__it, end ());
        _C_end_of_storage = _C_finish = _C_start + __x.size();
    }
    else {   //  size() < x.size() < capacity()
        size_type __size = size ();

        // overwrite (assign) existing elements
        copy (__x.begin (), __x.begin () + __size, begin ());

        // extend the size of *this prior to copying
        _C_finish = _C_start + __x.size ();

        // copy (initialize) elelements past the end
        uninitialized_copy (__x.begin () + __size, __x.end (),
                            begin () + __size,
                           _RWSTD_VALUE_ALLOC_CAST (*this));
    }
    return *this;
}

template <class _TypeT, class _Allocator>
void vector<_TypeT,_Allocator>::_C_insert_aux ( iterator __it,
                                                const_reference __x)
{
    if (capacity () > size ()) {
        // avoid dereferencing end () by incrementing _C_finish first
        pointer __old_end = _C_finish;
        ++_C_finish;
        _TRY {
            _RWSTD_VALUE_ALLOC(_C_value_alloc_type,
                               construct (__old_end,
                                          *(__old_end - difference_type (1))));
        }
        _CATCH (...) {
            _RWSTD_VALUE_ALLOC(_C_value_alloc_type, destroy(__old_end));
            --_C_finish;
            _RETHROW;
        }
        copy_backward (__it, _C_make_iter (__old_end - difference_type (1)) ,
                       _C_make_iter (__old_end));
        *__it = __x;
    }
    else {
        // more memory needed
        size_t __new_capacity = _RW::__rw_new_capacity(size(),this);
        pointer __start =
            _RWSTD_VALUE_ALLOC(_C_value_alloc_type,
                               allocate(__new_capacity, (void*)_C_start));
        _TRY {
            uninitialized_copy(begin(), __it, __start,
                               _RWSTD_VALUE_ALLOC_CAST (*this));
            _RWSTD_VALUE_ALLOC(_C_value_alloc_type, 
                construct((__start + (__it - begin())), __x));
            uninitialized_copy(__it, end(),
                               __start + (__it - begin()) + difference_type (1),
                               _RWSTD_VALUE_ALLOC_CAST (*this)); 
        }
        _CATCH (...) {
            _C_destroy (_C_make_iter (__start),
                        _C_make_iter (__start + __new_capacity));
            _RWSTD_VALUE_ALLOC(_C_value_alloc_type,
                               deallocate (__start,__new_capacity));
            _RETHROW;
        }
        
        // compute size before deallocating
        size_type __size = size ();

        _C_destroy (begin(), end());

        // invalidates all iterators into `*this'
        _RWSTD_VALUE_ALLOC(_C_value_alloc_type,
                           deallocate(_C_start, capacity ()));

        _C_start          = __start;
        _C_finish         = __start + __size + difference_type (1);
        _C_end_of_storage = __start + __new_capacity;
    }
}

template <class _TypeT, class _Allocator>
void vector<_TypeT,_Allocator>::_C_insert_aux (iterator __it,
                                               size_type __n,
                                               const_reference __x)
{
    if (__n == 0)
        return;
    if ((size () + __n) <= capacity ())  {
        iterator __old_end = end();
        // avoid dereferencing end () by incrementing _C_finish first
        _C_finish += __n;
        if (__it + __n < __old_end) { //insertion entirely before old end
            _TRY {
                uninitialized_copy (__old_end - __n, __old_end, __old_end,
                                    _RWSTD_VALUE_ALLOC_CAST (*this));
            }
            _CATCH (...) {
                _C_destroy (__old_end, end());
                _C_finish -= __n;
                _RETHROW;
            }
            copy_backward(__it, __old_end - __n, __old_end);
            fill(__it, __it + __n, __x);
        }
        else {
            size_type __first_part = __n - (__old_end - __it);
            _TRY {
                uninitialized_fill_n (__old_end, __first_part, __x,
                                      _RWSTD_VALUE_ALLOC_CAST (*this));
                uninitialized_copy(__it, __old_end, __it + __n,
                                   _RWSTD_VALUE_ALLOC_CAST (*this));
            }
            _CATCH (...) {
                _C_destroy (__old_end, __old_end + __n);
                _C_finish -= __n;
                _RETHROW;
            }
            fill(__it, __old_end, __x);
        }
    }

    else  { // not enough capacity

        size_t __new_capacity = 
            max (size_t (size () + __n),
                 size_t (_RW::__rw_new_capacity (size (), this)));

        pointer __start =
            _RWSTD_VALUE_ALLOC (_C_value_alloc_type,
                                allocate (__new_capacity, (void*)_C_start));
        
        _TRY {
            uninitialized_copy(begin(), __it, _C_make_iter (__start),
                               _RWSTD_VALUE_ALLOC_CAST (*this));
            uninitialized_fill_n(__start + (__it - begin()), __n, __x,
                                 _RWSTD_VALUE_ALLOC_CAST (*this));
            uninitialized_copy(__it, end(),
                               __start + (__it - begin() + __n),
                               _RWSTD_VALUE_ALLOC_CAST (*this));
        }
        _CATCH (...) {
            _C_destroy (_C_make_iter (__start),
                        _C_make_iter (__start + __new_capacity));
            _RWSTD_VALUE_ALLOC(_C_value_alloc_type,
                               deallocate(__start,__new_capacity));
            _RETHROW;
        }

        // compute size before deallocating
        size_type __size = size ();

        _C_destroy (begin(), end());

        // invalidates all iterators into *this
        _RWSTD_VALUE_ALLOC (_C_value_alloc_type,
                            deallocate(_C_start, _C_end_of_storage - _C_start));

        _C_start          = __start;
        _C_finish         = __start + __size + __n;
        _C_end_of_storage = __start + __new_capacity;
    }
}


#ifndef _RWSTD_NO_MEMBER_TEMPLATES

template<class _TypeT, class _Allocator>
template<class _InputIter>
void vector<_TypeT,_Allocator>::_C_insert_aux2 (iterator   __it, 
                                                _InputIter __first, 
                                                _InputIter __last)
#else

template<class _TypeT, class _Allocator>
void vector<_TypeT,_Allocator>::_C_insert_aux2 (iterator       __it,
                                                const_iterator __first,
                                                const_iterator __last)
#endif   // _RWSTD_NO_MEMBER_TEMPLATES
{
    if (__first == __last) return;
    size_type __n = _DISTANCE(__first, __last, size_type);

    if ((size () + __n) <= capacity ()) {
        iterator __old_end= end();
        // avoid dereferencing end () by incrementing _C_finish first
        _C_finish += __n;
        if (__it + __n < __old_end) { //insertion entirely before old end
            _TRY {
                uninitialized_copy (__old_end - __n, __old_end, __old_end,
                                    _RWSTD_VALUE_ALLOC_CAST (*this));
            }
            _CATCH (...) {
                _C_destroy (__old_end, end ());
                _C_finish -= __n;
                _RETHROW;
            }
            copy_backward(__it, __old_end - __n, __old_end);
            copy(__first, __last, __it);
        }
        else {
            size_type __first_part = __old_end - __it;
#ifndef _RWSTD_NO_MEMBER_TEMPLATES
            _InputIter __iter(__first);
#else
            const_iterator __iter(__first);
#endif
            advance(__iter, __first_part);
            _TRY {
                uninitialized_copy(__iter, __last, __old_end,
                                   _RWSTD_VALUE_ALLOC_CAST (*this));
                uninitialized_copy(__it, __old_end, __it + __n,
                                   _RWSTD_VALUE_ALLOC_CAST (*this));
            }
            _CATCH (...) {
                _C_destroy (__old_end, end());
                _C_finish -= __n;
                _RETHROW;
            }
            copy(__first, __iter, __it);
        }
    }
    else {
        size_t __new_capacity = 
            max ((size_t)(size() + __n), (size_t)_RW::__rw_new_capacity(size(),this));
        pointer __start =
            _RWSTD_VALUE_ALLOC(_C_value_alloc_type,
                               allocate(__new_capacity,_C_start));
        
        _TRY {
            uninitialized_copy(begin(), __it, __start,
                               _RWSTD_VALUE_ALLOC_CAST (*this));
            uninitialized_copy(__first, __last,
                               __start + (__it - begin()),
                               _RWSTD_VALUE_ALLOC_CAST (*this));
            uninitialized_copy(__it, end(),
                               __start + (__it - begin() + __n),
                               _RWSTD_VALUE_ALLOC_CAST (*this));
        }
        _CATCH (...) {
            _C_destroy (_C_make_iter (__start),
                        _C_make_iter (__start + __new_capacity));
            _RWSTD_VALUE_ALLOC(_C_value_alloc_type,
                               deallocate(__start,__new_capacity));
            _RETHROW;
        }
        
        // compute size before deallocating
        size_type __size = size ();

        _C_destroy (begin (), end ());

        // invalidates all iterators into *this
        _RWSTD_VALUE_ALLOC(_C_value_alloc_type,
                           deallocate(_C_start, capacity()));

        _C_start          = __start;
        _C_finish         = __start + __size + __n;
        _C_end_of_storage = __start + __new_capacity;
    }
}

#ifndef _RWSTD_NO_VECTOR_BOOL

#ifndef _RWSTD_NO_MEMBER_TEMPLATES
// The body of this function is duplicated in src/vecbool.cpp and
// further down in this file as well.
#if !defined (_RWSTD_NO_CLASS_PARTIAL_SPEC) 
  template <class _Allocator>
  template<class _InputIter>
  void vector<bool, _Allocator >::insert 
#else
  template<class _InputIter>
  void vector<bool, allocator<bool> >::insert 
#endif // _RWSTD_NO_CLASS_PARTIAL_SPEC
  (iterator __it, _InputIter __first, _InputIter __last)

  {
    if (__first == __last) return;
    size_type __n = _DISTANCE (__first, __last, size_type);
    if (capacity() - size() >= __n)
    {
      copy_backward(__it, end(), _C_finish + __n);
      copy(__first, __last, __it);
      _C_finish += __n;
    }
    else
    {
      size_type __len = size() + max(size(), __n);
      unsigned int* __q = _C_bit_alloc(__len);
      iterator __i = copy(begin(), __it, iterator(__q, 0));
      __i = copy(__first, __last, __i);
      _C_finish = copy(__it, end(), __i);
      _C_value_alloc_type (*this).
          deallocate((pointer)_C_start._C_p,_C_end_of_storage - _C_start._C_p);
      _C_end_of_storage = __q + (__len + _RWSTD_WORD_BIT - 1)/_RWSTD_WORD_BIT;
      _C_start = iterator(__q, 0);
    }
  }
#endif // _RWSTD_NO_MEMBER_TEMPLATES

#ifndef _RWSTD_NO_CLASS_PARTIAL_SPEC

// Duplicates of the followign functions exist in src/stl/vecbool.cpp.
// Which set is used depends on the availability of partial specialization.

  template <class _Allocator>
  void vector<bool,_Allocator >::flip ()
  {
    for (iterator __i = begin(); !(__i == end()); __i++)
      *__i = !(*__i);
  }

  template <class _Allocator>
  void vector<bool,_Allocator >::swap (reference __x, reference __y)
  {
    bool __tmp = __x; __x = __y; __y = __tmp;
  }

  template <class _Allocator>
  void vector<bool,_Allocator >::_C_insert_aux (iterator __it, bool __x)
  {
    if (_C_finish._C_p != _C_end_of_storage)
    {
      _C_copy_backward(__it, _C_finish - difference_type (1), _C_finish);
      *__it = __x;
      ++_C_finish;
    }
    else
    {
      size_type __len = size() ? 2 * size() : _RWSTD_WORD_BIT;
      unsigned int* __q = _C_bit_alloc(__len);
      iterator __i = _C_copy(begin(), __it, iterator(__q, 0));
      *__i++ = __x;
      _C_finish = _C_copy(__it, end(), __i);
      _C_value_alloc_type (*this).
          deallocate((pointer)_C_start._C_p,_C_end_of_storage - _C_start._C_p);
      _C_end_of_storage = __q + (__len + _RWSTD_WORD_BIT - 1)/_RWSTD_WORD_BIT;
      _C_start = iterator(__q, 0);
    }
  }

  template <class _Allocator>
  void vector<bool,_Allocator >::insert (iterator __it, size_type __n,
                                         const bool& __x)
  {
    if (__n == 0) return;
    if (capacity() - size() >= __n)
    {
      _C_copy_backward(__it, end(), _C_finish + __n);
      _C_fill(__it, __it + __n, __x);
      _C_finish += __n;
    }
    else
    {
      size_type __len = size() + max(size(), __n);
      unsigned int* __q = _C_bit_alloc(__len);
      iterator __i = _C_copy(begin(), __it, iterator(__q, 0));
      _C_fill_n(__i, __n, __x);
      _C_finish = _C_copy(__it, end(), __i + __n);
      _C_value_alloc_type (*this).
          deallocate((pointer)_C_start._C_p,_C_end_of_storage - _C_start._C_p);
      _C_end_of_storage = __q + (__len + _RWSTD_WORD_BIT - 1)/_RWSTD_WORD_BIT;
      _C_start = iterator(__q, 0);
    }
  }


#ifdef _RWSTD_NO_MEMBER_TEMPLATES
  template <class _Allocator>
  void vector<bool,_Allocator >::insert (iterator       __it,
                                         const_iterator __first,
                                         const_iterator __last)
  {
    if (__first == __last) return;
    size_type __n = _DISTANCE(__first, __last, size_type);
    if (capacity() - size() >= __n)
    {
      _C_copy_backward(__it, end(), _C_finish + __n);
      _C_copy(__first, __last, __it);
      _C_finish += __n;
    }
    else
    {
      size_type __len = size() + max(size(), __n);
      unsigned int* __q = _C_bit_alloc(__len);
      iterator __i = _C_copy(begin(), __it, iterator(__q, 0));
      __i = _C_copy(__first, __last, __i);
      _C_finish = _C_copy(__it, end(), __i);
      _C_value_alloc_type (*this).
          deallocate((pointer)_C_start._C_p,_C_end_of_storage - _C_start._C_p);
      _C_end_of_storage = __q + (__len + _RWSTD_WORD_BIT - 1)/_RWSTD_WORD_BIT;
      _C_start = iterator(__q, 0);
    }
  }
#endif // _RWSTD_NO_MEMBER_TEMPLATES

  template <class _Allocator>
  void vector<bool,_Allocator >::resize (size_type __new_size, bool __c)
  {
    if (__new_size > size())
      insert(end(), __new_size - size(), __c);             
    else if (__new_size < size())
      erase(begin() + __new_size, end());
  }
#endif // _RWSTD_NO_CLASS_PARTIAL_SPEC

#endif // _RWSTD_NO_VECTOR_BOOL

_RWSTD_NAMESPACE_END   // std
