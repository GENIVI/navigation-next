/***************************************************************************
 *
 * string.cc - Definitions for the Standard Library string classes
 *
 * $Id: string.cc 91768 2005-08-10 18:58:48Z drodgman $
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


#if    defined (_RWSTD_LLP64_ARCHITECTURE)          \
    && defined (_RWSTD_NO_STATIC_CONST_MEMBER_INIT) \
    && defined (_RWBUILD_std)

template <class _CharT, class _Traits, class _Allocator>
const _TYPENAME basic_string<_CharT, _Traits, _Allocator>::size_type
basic_string<_CharT, _Traits, _Allocator>::npos = size_t(-1);

#elif !defined(_RWSTD_NO_STATIC_CONST_MEMBER_INIT) 

template <class _CharT, class _Traits, class _Allocator>
const _TYPENAME basic_string<_CharT, _Traits, _Allocator>::size_type
basic_string<_CharT, _Traits, _Allocator>::npos;

#endif    


#ifndef _RWSTD_NO_COLLAPSE_TEMPLATE_STATICS

template <class _CharT, class _Traits, class _Allocator>
_RW::__null_ref<_CharT, _Traits, _Allocator>
basic_string<_CharT, _Traits, _Allocator>::__nullref;

#endif   // _RWSTD_NO_STRING_REF_COUNT


template <class _CharT, class _Traits, class _Allocator>
_TYPENAME basic_string<_CharT, _Traits, _Allocator>::_C_string_ref_type *
basic_string<_CharT, _Traits, _Allocator>::
_C_getRep (size_type __cap, size_type __len)
{
    _RWSTD_REQUIRES (__cap <= max_size (),
                     (_RWSTD_ERROR_LENGTH_ERROR,
                      _RWSTD_FUNC ("basic_string::_C_getRep(size_type, "
                                   "size_type)"), __cap, max_size ()));

    _RWSTD_REQUIRES (__len <= __cap,
                     (_RWSTD_ERROR_LENGTH_ERROR,
                      _RWSTD_FUNC ("basic_string::_C_getRep(size_type, "
                                   "size_type)"), __len, __cap));

    if (!__cap) {
        _RWSTD_ASSERT (!__len);

#ifndef _RWSTD_NO_COLLAPSE_TEMPLATE_STATICS

        return &__nullref;

#else   // if defined (_RWSTD_NO_COLLAPSE_TEMPLATE_STATICS)

        return _RWSTD_REINTERPRET_CAST (_C_string_ref_type*, &_RW::__nullref);

#endif   // _RWSTD_NO_COLLAPSE_TEMPLATE_STATICS

    }

    // allocate, initialize the __string_ref, and initialize each character
    _C_string_ref_type * __ret =
    _RWSTD_REINTERPRET_CAST (_C_string_ref_type*,
            _RWSTD_VALUE_ALLOC (_C_value_alloc_type,
                                allocate (__cap + sizeof (_C_string_ref_type) /
                                          sizeof (value_type) + 2)));

    // avoid copy construction (mutex isn't copy-constructible)
    // _C_ref_alloc_type (*this).construct (__ret, _C_string_ref_type ());
    new (__ret) _C_string_ref_type ();

#ifndef _RWSTD_NO_STRING_REF_COUNT

    // set initial reference count to 1
    __ret->_C_init (1, __cap, __len);

#else   // if defined (_RWSTD_NO_STRING_REF_COUNT)

    // initial reference count is 0 (ref counting disabled)
    __ret->_C_init (0, __cap, __len);

#endif   // _RWSTD_NO_STRING_REF_COUNT


    _RWSTD_VALUE_ALLOC (_C_value_alloc_type, construct (__ret->data () + __len,
        value_type ()));

    return __ret;
}


template <class _CharT, class _Traits, class _Allocator>
basic_string<_CharT, _Traits, _Allocator>::
basic_string (const basic_string &__s, 
              size_type __pos, size_type __n, const allocator_type& __alloc)
    : allocator_type (__alloc)
{
    _RWSTD_REQUIRES (__pos <= __s.size (),
                     (_RWSTD_ERROR_OUT_OF_RANGE,
                      _RWSTD_FUNC ("basic_string::basic_string(const "
                                   "basic_string&, size_type, size_type)"),
                      __pos, __s.size ()));

    size_type __slen = __s.size() - __pos;
    size_type __rlen = __n < __slen ? __n : __slen; 
    size_type __nlen =  __n == npos ? 0 : __n;
    size_type __maxlen = __nlen > __rlen ? __nlen : __rlen; 
    if (__maxlen)
        _C_data = _C_allocate (0, __maxlen, __rlen);
    else
        _C_data = _C_getRep (__maxlen,__rlen)->data();

    traits_type::copy(_C_data, &__s._C_data[__pos], __rlen);
}


template <class _CharT, class _Traits, class _Allocator>
void basic_string<_CharT, _Traits, _Allocator>::
_C_initn (size_type __n, value_type __c)
{
    _RWSTD_REQUIRES (__n <= max_size (),
                     (_RWSTD_ERROR_LENGTH_ERROR,
                      _RWSTD_FUNC ("basic_string::_C_initn(size_type, "
                                   "value_type)"), __n, max_size ()));
   
    _C_data = __n ? _C_allocate (0, __n, __n) : _C_null ();

    while (__n--)
        traits_type::assign (_C_data [__n], __c);
}


template <class _CharT, class _Traits, class _Allocator>
basic_string<_CharT, _Traits, _Allocator>::
basic_string (const_pointer __s, size_type __n, const allocator_type& __alloc)
    : allocator_type (__alloc)
{
    // extension: if `s' is 0 then `n' unitialized elements are allocated

    _RWSTD_REQUIRES (__n <= max_size (),
                     (_RWSTD_ERROR_LENGTH_ERROR,
                      _RWSTD_FUNC ("basic_string::basic_string(const_pointer,"
                                   "size_type, const allocator_type&)"),
                      __n, max_size ()));
    
    _C_data = __n ? _C_allocate (0, __n, __n) : _C_null ();

    if (__s)
        traits_type::copy (_C_data, __s, __n);
}


template <class _CharT, class _Traits, class _Allocator>
basic_string<_CharT, _Traits, _Allocator>::
basic_string (const_pointer __s, const allocator_type& __alloc)
    : allocator_type(__alloc)
{     
    _RWSTD_ASSERT(__s != 0);

    size_type __n = traits_type::length (__s);

    _C_data = __n ? _C_allocate (0, __n, __n) : _C_null ();

    traits_type::copy (_C_data, __s, __n);
}


#ifndef _RWSTD_NO_MEMBER_TEMPLATES

template <class _CharT, class _Traits, class _Allocator>
template <class _InputIterator>
basic_string<_CharT, _Traits, _Allocator>::
basic_string (_InputIterator __first, _InputIterator __last, 
              const allocator_type &__alloc)
    : allocator_type (__alloc),
      _C_data (_C_null ())
{
    _RWSTD_ASSERT_RANGE (__first, __last);

    replace (_C_make_iter (_C_data), _C_make_iter (_C_data), __first, __last);
}

#endif   // _RWSTD_NO_MEMBER_TEMPLATES


template <class _CharT, class _Traits, class _Allocator>
basic_string<_CharT, _Traits, _Allocator>::
basic_string (const_pointer __first, const_pointer __last, 
              const allocator_type& __alloc)
    : allocator_type (__alloc)
{
    _RWSTD_ASSERT_RANGE (__first, __last);
    _RWSTD_ASSERT (__first <= __last);

    size_type __n = __last - __first;

    _C_data = __n ? _C_allocate (0, __n, __n) : _C_null ();

    traits_type::copy (_C_data, __first, __n);
}


template <class _CharT, class _Traits, class _Allocator>
basic_string<_CharT, _Traits, _Allocator>&
basic_string<_CharT, _Traits, _Allocator>::operator= (const basic_string &__rhs)
{
    if (__rhs._C_pref ()->_C_ref_count () > 0) {
        __rhs._C_pref ()->_C_inc_ref ();
        _C_unlink ();
        _C_data = __rhs._C_data;
    }
    else if (this != &__rhs)
        replace (0, size (), __rhs.data (), __rhs.size ());

    return *this;
}

template <class _CharT, class _Traits, class _Allocator>
basic_string<_CharT, _Traits, _Allocator>&
basic_string<_CharT, _Traits, _Allocator>::operator= (const_pointer __rhs)
{
    _RWSTD_ASSERT (__rhs != 0);

    size_type __len = traits_type::length (__rhs);

    if (0 == __len) {
        if (_C_pref ()->_C_ref_count () == 1) {
            _C_pref ()->_C_size = 0;
            traits_type::assign (_C_data [0], value_type ());
        }
        else {
            _C_unlink();
            _C_data = _C_null ();
        }
        return *this;
    }

    return replace (0, size (), __rhs, __len);
}


template <class _CharT, class _Traits, class _Allocator>
basic_string<_CharT, _Traits, _Allocator>&
basic_string<_CharT, _Traits, _Allocator>::
append (const basic_string &__str, size_type __pos, size_type __n)
{
    _RWSTD_REQUIRES (__pos <= __str.size (),
                     (_RWSTD_ERROR_OUT_OF_RANGE,
                      _RWSTD_FUNC ("basic_string::append(const basic_string&,"
                                   " size_type, size_type)"),
                      __pos, __str.size ()));

    size_type __slen = __str.size() - __pos;
    size_type __rlen = __n < __slen ? __n : __slen; 


    _RWSTD_REQUIRES (size () <= max_size () - __rlen,
                     (_RWSTD_ERROR_LENGTH_ERROR,
                      _RWSTD_FUNC ("basic_string::append(const basic_string&,"
                                   " size_type, size_type)"),
                      size (), max_size () - __rlen));

    replace(size(), 0, __str.c_str(), __str.size(), __pos, __n);

    return *this;
}


template <class _CharT, class _Traits, class _Allocator>
basic_string<_CharT, _Traits, _Allocator>&
basic_string<_CharT, _Traits, _Allocator>::
assign (const basic_string &__str, size_type __pos, size_type __n)
{
    _RWSTD_REQUIRES (__pos <= __str.size (),
                     (_RWSTD_ERROR_OUT_OF_RANGE,
                      _RWSTD_FUNC ("basic_string::assign(basic_string&, "
                                   "size_type, size_type)"),
                      __pos, __str.size ()));

    size_type __slen = __str.size() - __pos;
    size_type __rlen = __n < __slen ? __n : __slen; 
    return replace(0, size(), __str, __pos, __rlen);
}


template <class _CharT, class _Traits, class _Allocator>
basic_string<_CharT, _Traits, _Allocator>&
basic_string<_CharT, _Traits, _Allocator>::
insert (size_type __pos1, const basic_string& __str, 
        size_type  __pos2,  size_type         __n)
{
    _RWSTD_REQUIRES (__pos1 <= size() && __pos2 <= __str.size (),
                     (_RWSTD_ERROR_OUT_OF_RANGE,
                      _RWSTD_FUNC ("basic_string::insert(size_type, const "
                                   "basic_string&, size_type, size_type)"), 
                      __pos1 > size() ? __pos1:__pos2, __str.size ()));
    
    size_type __slen = __str.size() - __pos2;
    size_type __rlen = __n < __slen ? __n : __slen; 

    _RWSTD_REQUIRES (size () <= max_size () - __rlen,
                     (_RWSTD_ERROR_LENGTH_ERROR,
                      _RWSTD_FUNC ("basic_string::insert(size_type, const "
                                   "basic_string&, size_type, size_type)"), 
                      size (), max_size () - __rlen));

    return replace(__pos1, 0, __str, __pos2, __n);
}

template <class _CharT, class _Traits, class _Allocator>
basic_string<_CharT, _Traits, _Allocator>&
basic_string<_CharT, _Traits, _Allocator>::
insert (size_type __pos1, const basic_string &__str)
{
    _RWSTD_REQUIRES (__pos1 <= size (),
                     (_RWSTD_ERROR_OUT_OF_RANGE,
                      _RWSTD_FUNC ("basic_string::insert(size_type, const "
                                   "basic_string&)"), __pos1, size ()));
    
    _RWSTD_REQUIRES (size() <= max_size() - __str.size(),
                     (_RWSTD_ERROR_LENGTH_ERROR,
                      _RWSTD_FUNC ("basic_string::insert(size_type, "
                                   "const basic_string&)"),
                      size (), max_size () - __str.size ()));

    return replace(__pos1, 0, __str);
}


template <class _CharT, class _Traits, class _Allocator>
_TYPENAME basic_string<_CharT, _Traits, _Allocator>::iterator 
basic_string<_CharT, _Traits, _Allocator>::
replace (size_type __pos1, size_type __n1, const_pointer __cs, 
         size_type __cslen, size_type __pos2, size_type __n2)
{
    _RWSTD_REQUIRES (__pos1 <= size () && __pos2 <= __cslen,
                     (_RWSTD_ERROR_OUT_OF_RANGE, 
                      _RWSTD_FUNC ("basic_string::replace(size_type, size_type,"
                                   " const_pointer, size_type, size_type, "
                                   "size_type)"), 
                      __pos1 > size() ? __pos1 : __pos2,
                      size () > __cslen ? size () : __cslen));

    size_type __slen = size() - __pos1;
    size_type __xlen = __n1 < __slen ? __n1 : __slen; 
    size_type __clen = __cslen - __pos2;
    size_type __rlen = __n2 < __clen ? __n2 : __clen; 

    _RWSTD_REQUIRES (size () - __xlen <= max_size () - __rlen,
                     (_RWSTD_ERROR_LENGTH_ERROR,
                      _RWSTD_FUNC ("basic_string::replace(size_type, size_type,"
                                   " const_pointer, size_type, size_type, "
                                   "size_type)"), 
                      size () - __xlen, max_size() - __rlen));

    size_type __len = size() - __xlen + __rlen;  // final string length

    if (!__len) {
        // Special case a substitution that leaves the string empty.
        _C_unlink ();
        _C_data = _C_null ();
    }
    else {
        // Length of bit at end of string
        size_type __rem = size () - __xlen - __pos1;
        // Check for shared representation, insufficient capacity, 
        //  or overlap copy.
        if (   _C_pref()->_C_ref_count () > 1
            || capacity () < __len
            || __cs && __cs >= data () && __cs < data () + size ()) {
            // Need to allocate a new reference.
            size_t __new_capacity = max (_RW::__rw_new_capacity (size(), this),
                                         size_t (__len));
            _C_string_ref_type * __temp = _C_getRep(__new_capacity, __len);
            if (__pos1)
                traits_type::copy (__temp->data (), _C_data, __pos1);

            if (__rlen)
                traits_type::copy (__temp->data() + __pos1,
                                   __cs + __pos2, __rlen);

            if (__rem)
                traits_type::copy (__temp->data() + __pos1 + __rlen,
                                   _C_data + __pos1 + __n1, __rem);
            _C_unlink();
            _C_data = __temp->data();
        }
        else {
            // Current reference has enough room.
            if (__rem)
                traits_type::move (_C_data + __pos1 + __rlen,
                                   _C_data + __pos1 + __n1, __rem);

            if (__rlen)
                traits_type::move (_C_data + __pos1, __cs + __pos2, __rlen);

            traits_type::assign (_C_data [_C_pref()->_C_size = __len],
                                 value_type ());
        }
    }
    return _C_make_iter (_C_data + __pos1);
}


template <class _CharT, class _Traits, class _Allocator>
basic_string<_CharT, _Traits, _Allocator>&
basic_string<_CharT, _Traits, _Allocator>::
replace (size_type __pos, size_type __n, size_type __n2, value_type __c)
{
    _RWSTD_REQUIRES (__pos <= size (),
                     (_RWSTD_ERROR_OUT_OF_RANGE, 
                      _RWSTD_FUNC ("basic_string:::replace(size_type, "
                                   "size_type, size_type, value_type)"),
                      __pos, size ()));
    
    size_type __slen = size() - __pos;
    size_type __xlen = __n < __slen ? __n : __slen; 

    _RWSTD_REQUIRES (size() - __xlen < max_size () - __n2,
                     (_RWSTD_ERROR_LENGTH_ERROR,
                      _RWSTD_FUNC ("basic_string::replace(size_type, "
                                   "size_type, size_type, value_type)"), 
                      size () - __xlen, max_size () - __n2));
    
    size_type __len = size() - __xlen + __n2;  // Final string length.

    if (!__len)
    {
      // Special case a substitution that leaves the string empty.
      _C_unlink();
      _C_data = _C_null ();
    }
    else
    {
      size_type __rem = size () - __xlen - __pos; // length of bit at the end
      // Check for shared representation, insufficient capacity, 
      if ( (_C_pref()->_C_ref_count() > 1) || (capacity() < __len))
      {
        // Need to allocate a new reference.
          size_t __new_capacity = max (_RW::__rw_new_capacity (size(), this),
                                       size_t (__len));
        _C_string_ref_type * __temp = _C_getRep(__new_capacity, __len);
        if (__pos) traits_type::copy(__temp->data(), _C_data, __pos);
        if (__n2) traits_type::assign(__temp->data()+__pos, __n2, __c);
        if (__rem)
            traits_type::copy (__temp->data () + __pos + __n2,
                               _C_data + __pos + __n, __rem);
        _C_unlink();
        _C_data = __temp->data();
      }
      else
      {
        // Current reference has enough room.
        if (__rem) 
            traits_type::move(_C_data+__pos+__n2, _C_data+__pos+__n, __rem);
        if (__n2)
            traits_type::assign(_C_data+__pos, __n2, __c);
        traits_type::assign (_C_data [_C_pref()->_C_size = __len],
                             value_type ());
      }
    }
    return *this;
}


#if     !defined (_RWSTD_NO_MEMBER_TEMPLATES)   \
     && !defined (_RWSTD_NO_CLASS_PARTIAL_SPEC)

template<class _CharT, class _Traits, class _Allocator>
template<class _InputIter>
basic_string<_CharT, _Traits, _Allocator>& 
basic_string<_CharT, _Traits, _Allocator>::
replace (iterator __first1, iterator __last1,
         _InputIter __first2, _InputIter __last2, _RWSTD_DISPATCH_INT (false))
{
    _RWSTD_ASSERT_RANGE (__first1, _C_make_iter (_C_data + size ()));
    _RWSTD_ASSERT_RANGE (__first1, __last1);
    _RWSTD_ASSERT_RANGE (__first2, __last2);

     // use a (probably) faster algorithm if possible
     if (__is_bidirectional_iterator(_RWSTD_ITERATOR_CATEGORY (_InputIter,
                                                               __last2)))
         return __replace_aux (__first1, __last1, __first2, __last2);

     for ( ; __first2 != __last2; ++__first1, ++__first2) {

         size_type __off = __first1 - _C_make_iter (_C_data);

         _RWSTD_REQUIRES (__off <= max_size(),
                          (_RWSTD_ERROR_LENGTH_ERROR,
                           _RWSTD_FUNC ("basic_string::replace(iterator, "
                                        "iterator, InputIterator, "
                                        "InputIterator)"),
                           __first1 - _C_make_iter (_C_data), max_size ()));
         
         // extend the string if necessary
         if (__first1 == __last1) {
             // compute the size of new buffer
             size_t __new_cap = _RW::__rw_new_capacity (size (), this);
             size_t __delta   = __new_cap - size ();

             // allocate a new buffer
             _C_string_ref_type *__tmp = _C_getRep (__new_cap, __new_cap);

             // copy data from old to new, leaving a hole for additions
             traits_type::copy (__tmp->data (), _C_data, __off);
             traits_type::copy (__tmp->data () + __off + __delta,
                                _C_data + __off,
                                _C_make_iter (_C_data + size ()) - __last1);
             _C_unlink ();
             _C_data  = __tmp->data ();
             __first1 = _C_make_iter (_C_data + __off);
             __last1  = __first1 + __delta;
         }

         // copy data over
         traits_type::assign (*__first1, *__first2);
     }

     if (__first1 != __last1)
         replace (__first1 - _C_make_iter (_C_data), __last1 - __first1,
                  0, value_type ());

     return *this;
}

// Special function for random access and bi-directional iterators
// Avoids the possibility of multiple allocations
// We still have to copy characters over one at a time.
template<class _CharT, class _Traits, class _Allocator>
template<class _InputIterator>
basic_string<_CharT, _Traits, _Allocator>& 
basic_string<_CharT, _Traits, _Allocator>::
__replace_aux (iterator __first1, iterator __last1, 
               _InputIterator __first2, _InputIterator __last2)
{
    _RWSTD_ASSERT_RANGE (__first1, _C_make_iter (_C_data + size ()));
    _RWSTD_ASSERT_RANGE (__first1, __last1);
    _RWSTD_ASSERT_RANGE (__first2, __last2);

    difference_type __n2  = _DISTANCE (__first2, __last2, __n2);
    size_type       __n   = __last1 - __first1;
    size_type       __pos = __first1 - _C_make_iter (_C_data);

    _RWSTD_REQUIRES (__pos <= size (),
                     (_RWSTD_ERROR_OUT_OF_RANGE,
                      _RWSTD_FUNC ("basic_string::__replace_aux(iterator, "
                                   "iterator, InputIterator, InputIterator)"),
                      __pos, size ()));

    size_type __slen = size() - __pos;
    size_type __xlen = __n < __slen ? __n : __slen; 

    _RWSTD_REQUIRES (size () - __xlen < max_size() - __n2,
                     (_RWSTD_ERROR_LENGTH_ERROR, 
                      _RWSTD_FUNC ("basic_string::__replace_aux(iterator, "
                                   "iterator, InputIterator, InputIterator)"),
                      size () - __xlen, max_size () - __n2));

    size_type __len = size() - __xlen + __n2;  // Final string length.

    if (!__len) {
        // Special case a substitution that leaves the string empty.
        _C_unlink();
        _C_data = _C_null ();
    }
    else {
      size_type __d = 0;
      size_type __rem = size() - __xlen - __pos; // length of bit at the end
      // Check for shared representation, insufficient capacity, 
      if ( (_C_pref()->_C_ref_count () > 1) || (capacity() < __len))
      {
        // Need to allocate a new reference.
        size_t __new_capacity = max (_RW::__rw_new_capacity (size(), this),
                                     size_t (__len));
        _C_string_ref_type * __temp = _C_getRep(__new_capacity, __len);
        if (__pos) traits_type::copy(__temp->data(), _C_data, __pos);
        for (__d = 0; __d < (size_type)__n2; __d++)
            traits_type::assign (*(__temp->data()+__pos+__d), *__first2++);
        if (__rem)
            traits_type::copy (__temp->data () + __pos + __n2,
                               _C_data + __pos + __n, __rem);
        _C_unlink();
        _C_data = __temp->data();
      }
      else
      {
        // Current reference has enough room.
        if (__rem)  
          traits_type::move(_C_data+__pos+__n2, _C_data+__pos+__n, __rem);
        for (__d = 0; __d < (size_type)__n2; __d++)
            traits_type::assign (*(_C_data+__pos+__d), *__first2++);
        traits_type::assign (_C_data[_C_pref()->_C_size = __len],
                             value_type());
      }
    }
    return *this;
}

#endif // _RWSTD_NO_MEMBER_TEMPLATES


template <class _CharT, class _Traits, class _Allocator>
_TYPENAME basic_string<_CharT, _Traits, _Allocator>::size_type
basic_string<_CharT, _Traits, _Allocator>::
copy (pointer __s, size_type __n, size_type __pos) const
{
    _RWSTD_REQUIRES (__pos <= size(),
                     (_RWSTD_ERROR_OUT_OF_RANGE, 
                      _RWSTD_FUNC ("basic_string::copy(pointer, size_type, "
                                   "size_type)"), __pos, size ()));

    size_type __slen = size() - __pos;
    size_type __rlen = __n < __slen ? __n : __slen;
    traits_type::copy(__s, _C_data+__pos, __rlen);
    return __rlen;
}


template <class _CharT, class _Traits, class _Allocator>
_TYPENAME basic_string<_CharT, _Traits, _Allocator>::size_type
basic_string<_CharT, _Traits, _Allocator>::
find (const_pointer __s, size_type __pos, size_type __n) const
{
    _RWSTD_ASSERT(__s != 0);

    _RWSTD_REQUIRES (__n <= max_size (),
                     (_RWSTD_ERROR_LENGTH_ERROR, 
                      _RWSTD_FUNC ("basic_string::find(const_pointer, "
                                   "size_type, size_type) const"),
                      __n, max_size ()));

    for (size_type xpos = __pos; (xpos + __n) <= size() ; xpos++)
    {
      if (!traits_type::compare(_C_data+xpos, __s, __n))
        return xpos;
    }

    return npos;
}


template <class _CharT, class _Traits, class _Allocator>
_TYPENAME basic_string<_CharT, _Traits, _Allocator>::size_type
basic_string<_CharT, _Traits, _Allocator>::
rfind (const_pointer __s,  size_type __pos, size_type __n) const
{
    _RWSTD_ASSERT(__s != 0);

    _RWSTD_REQUIRES (__n <= max_size (),
                     (_RWSTD_ERROR_LENGTH_ERROR, 
                      _RWSTD_FUNC ("basic_string::rfind(const_pointer, "
                                   "size_type, size_type) const"),
                      __n, max_size ()));

    if (size() < __n)
      return npos;
    
    size_type __slen = size() -__n;
    size_type xpos_start = __slen < __pos ? __slen : __pos; 

    for (size_type xpos = xpos_start+1; xpos != 0 ; xpos--)
    {
      if (!traits_type::compare(_C_data+xpos-1, __s, __n))
        return xpos-1;
    }

    return npos;
}


template <class _CharT, class _Traits, class _Allocator>
_TYPENAME basic_string<_CharT, _Traits, _Allocator>::size_type
basic_string<_CharT, _Traits, _Allocator>::
find_first_of (const_pointer __s, size_type __pos, size_type __n) const
{
    _RWSTD_ASSERT(__s != 0);

    _RWSTD_REQUIRES (__n <= max_size (),
                     (_RWSTD_ERROR_LENGTH_ERROR,
                      _RWSTD_FUNC ("basic_string::find_first_of(const_pointer, "
                                   "size_type, size_type) const"),
                      __n, max_size ()));

    for (size_type xpos = __pos; xpos < size() ; xpos++)
    {
      for (size_type __i = 0; __i < __n ; __i++)
        if (traits_type::eq(_C_data[xpos], __s[__i]))
          return xpos;
    }

    return npos;
}


template <class _CharT, class _Traits, class _Allocator>
_TYPENAME basic_string<_CharT, _Traits, _Allocator>::size_type
basic_string<_CharT, _Traits, _Allocator>::
find_last_of (const_pointer __s, size_type __pos, size_type __n) const
{
    _RWSTD_ASSERT(__s != 0);

    _RWSTD_REQUIRES (__n <= max_size (),
                     (_RWSTD_ERROR_LENGTH_ERROR, 
                      _RWSTD_FUNC ("basic_string::find_last_of(const_pointer, "
                                   "size_type, size_type) const"),
                      __n, max_size ()));

    if (size())
    {
      size_type __slen = size() -1;
      size_type xpos_start = __pos < __slen ? __pos : __slen; 
      for (size_type xpos = xpos_start+1; xpos != 0 ; xpos--)
      {
        for(size_type __i = 0; __i < __n ; __i++)
          if (traits_type::eq(_C_data[xpos-1], __s[__i]))
            return xpos-1;
      } 
    }
    return npos;
}


template <class _CharT, class _Traits, class _Allocator>
_TYPENAME basic_string<_CharT, _Traits, _Allocator>::size_type
basic_string<_CharT, _Traits, _Allocator>::
find_first_not_of (const_pointer __s, size_type __pos, size_type __n) const
{
    _RWSTD_ASSERT(__s != 0);

    _RWSTD_REQUIRES (__n <= max_size (),
                     (_RWSTD_ERROR_LENGTH_ERROR, 
                      _RWSTD_FUNC ("basic_string::find_first_not_of("
                                  "const_pointer, size_type, size_type) const"),
                      __n, max_size ()));

    for (size_type xpos = __pos; xpos < size() ; xpos++)
    {
      bool found = false;
      for (size_type __i = 0; __i < __n ; __i++)
      {
        if (traits_type::eq(_C_data[xpos], __s[__i]))
        {
          found = true;
          break;
        }
      }
      if (!found)
        return xpos;
    }

    return npos;
}


template <class _CharT, class _Traits, class _Allocator>
_TYPENAME basic_string<_CharT, _Traits, _Allocator>::size_type
basic_string<_CharT, _Traits, _Allocator>::
find_last_not_of(const_pointer __s, size_type __pos, size_type __n) const
{
    _RWSTD_ASSERT(__s != 0);

    _RWSTD_REQUIRES (__n <= max_size (),
                     (_RWSTD_ERROR_LENGTH_ERROR, 
                      _RWSTD_FUNC ("basic_string::find_last_not_of("
                                  "const_pointer, size_type, size_type) const"),
                      __n, max_size ()));
    
    if (size())
    {
      size_type __slen = size() -1;
      size_type xpos_start = __pos < __slen ? __pos : __slen; 
      for (size_type xpos = xpos_start+1; xpos != 0 ; xpos--)
      {
        bool found = false;
        for (size_type __i = 0; __i < __n ; __i++)
        {
          if (traits_type::eq(_C_data[xpos-1], __s[__i]))
          {
            found = true;
            break;
          }
        }
        if (!found)
          return xpos-1;
      }
    }

    return npos;
}


template <class _CharT, class _Traits, class _Allocator>
basic_string<_CharT, _Traits, _Allocator>
basic_string<_CharT, _Traits, _Allocator>::
substr (size_type __pos, size_type __n) const
{
    _RWSTD_REQUIRES (__pos <= size (),
                     (_RWSTD_ERROR_OUT_OF_RANGE, 
                      _RWSTD_FUNC ("basic_string::substr(size_type, size_type) "
                                   "const"), __pos, size ()));

    size_type __slen = size() -__pos;
    size_type __rlen = __n < __slen ? __n : __slen;
    return basic_string (_C_data + __pos, __rlen);
}


template <class _CharT, class _Traits, class _Allocator>
int basic_string<_CharT, _Traits, _Allocator>::
compare (size_type __pos1, size_type __n1, 
         const basic_string& __str, 
         size_type __pos2, size_type __n2) const
{
    _RWSTD_REQUIRES (__pos2 <= __str.size (),
                     (_RWSTD_ERROR_OUT_OF_RANGE, 
                      _RWSTD_FUNC ("basic_string::compare(size_type, size_type,"
                                   "const basic_string&, size_type, size_type) "
                                   "const"), __pos2, __str.size ()));

//  "reduce" __n2 if necessary, where
//  "reduce" := Ensure __pos2+__n2 < __str.size() so we can call a function that
//              doesn't check this same inequality.

    if(__str.size() - __pos2 < __n2)
      __n2 = __str.size() - __pos2;

//  compare(size_type, size_type, char*, size_type) will both
//   (i) check pos1 to make sure it's less than size() and
//  (ii) "reduce" __n1 if necessary

    return compare(__pos1, __n1, __str.c_str()+__pos2, __n2);
}


template <class _CharT, class _Traits, class _Allocator>
int basic_string<_CharT, _Traits, _Allocator>::
compare (size_type __pos, size_type __n1,
         const_pointer __s, size_type __n2) const
{
    _RWSTD_REQUIRES (__pos <= size (),
                     (_RWSTD_ERROR_OUT_OF_RANGE, 
                      _RWSTD_FUNC ("basic_string::compare(size_type, size_type,"
                                   " const const_pointer, size_type) const"),
                      __pos, size ()));

    if(size() - __pos < __n1)
      __n1 = size() - __pos;
    size_type __rlen = __n1 < __n2 ? __n1 : __n2; 
    int __res = traits_type::compare(_C_data+__pos,__s, __rlen);

    if (__res == 0)
      __res = (__n1 < __n2) ? -1 : (__n1 != __n2);

    return __res;
}


_RWSTD_NAMESPACE_END   // std
