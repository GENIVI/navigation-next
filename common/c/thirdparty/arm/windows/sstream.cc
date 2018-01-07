/***************************************************************************
 *
 * sstream.cc - Declarations for the Standard Library basic strings
 *
 * $Id: sstream.cc 91768 2005-08-10 18:58:48Z drodgman $
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


template<class _CharT, class _Traits, class _Allocator>
basic_stringbuf<_CharT, _Traits, _Allocator>::
basic_stringbuf (const _C_string_type& __str, ios_base::openmode __mode)
    : basic_streambuf<_CharT, _Traits>(__mode)
{
    this->_C_own_buf (true);
    streamsize __slen = __str.length ();

    if (__slen != 0) {

        typedef _RWSTD_ALLOC_TYPE (allocator_type, char_type) _ValueAlloc;

        this->_C_bufsize = __slen;
        this->_C_buffer  = _ValueAlloc ().allocate (this->_C_bufsize);
        traits_type::copy (this->_C_buffer, __str.data (), __slen);
        
        if (this->_C_is_in ())
            this->setg (this->_C_buffer, this->_C_buffer, this->_C_buf_end ());
        
        if (this->_C_is_out ())
            this->setp (this->_C_buffer, this->_C_buf_end ());
        
        if (__mode & (ios_base::app | ios_base::ate))
            this->pbump (__slen);        // "seek" to end
    }
}

template<class _CharT, class _Traits, class _Allocator>
void
basic_stringbuf<_CharT, _Traits, _Allocator>::str (const _C_string_type& __str)
{
    _RWSTD_ASSERT (this->_C_is_valid ());

    typedef _RWSTD_ALLOC_TYPE (allocator_type, char_type) _ValueAlloc;

    streamsize __slen = __str.length ();

    if (__slen == 0) {

        if (this->_C_own_buf ())
            _ValueAlloc ().deallocate (this->_C_buffer, this->_C_bufsize);

        this->setg(0, 0, 0);
        this->setp(0, 0);
        this->_C_buffer = 0;
        this->_C_bufsize = 0;
    }
    else {
        if (__slen > this->_C_bufsize)  {
            // buffer too small - need to reallocate
            if (this->_C_own_buf ())
                _ValueAlloc ().deallocate (this->_C_buffer, this->_C_bufsize);
            this->_C_bufsize =
                max (__slen,
                     streamsize (_RW::__rw_new_capacity (this->_C_bufsize,
                                                         this)));
            this->_C_buffer = _ValueAlloc ().allocate (this->_C_bufsize);
            this->_C_own_buf (true);
        }
        traits_type::copy (this->_C_buffer, __str.data (), this->_C_bufsize);
        
        if (this->_C_is_in ())
            this->setg (this->_C_buffer, this->_C_buffer,
                        this->_C_buffer + __slen);
        
        if (this->_C_is_out ()) {
            this->setp (this->_C_buffer, this->_C_buffer + __slen);
            
            if ((this->_C_iomode & ios_base::app)
                || (this->_C_iomode & ios_base::ate))
                this->pbump (__slen); // "seek" to end 
        } 
    }
}


template<class _CharT, class _Traits, class _Allocator>
_TYPENAME basic_stringbuf<_CharT, _Traits, _Allocator>::int_type
basic_stringbuf<_CharT, _Traits, _Allocator>::overflow (int_type __c)
{
    _RWSTD_ASSERT (this->_C_is_valid ());

    if (!this->_C_is_out ()) 
        return traits_type::eof ();
    
    if (this->_C_is_eof (__c))
        return  traits_type::not_eof (__c);
    
    streamsize __slen = _C_strlen ();
    
    // reallocate space if necessary

    if (!(this->epptr () < this->_C_buf_end ())) {

        typedef _RWSTD_ALLOC_TYPE (allocator_type, char_type) _ValueAlloc;

        // calculate size of buffer to allocate
        streamsize __new_size =
            max (streamsize (this->_C_bufsize + 1),
                 streamsize (_RW::__rw_new_capacity (this->_C_bufsize, this)));
        
        char_type* __new_buf = _ValueAlloc ().allocate (__new_size);
           
        if (this->_C_buffer) {  // need to copy the old buffer to new buffer
            traits_type::copy (__new_buf, this->_C_buffer, this->_C_bufsize);   
            if (this->_C_own_buf ()) 
                _ValueAlloc ().deallocate (this->_C_buffer, this->_C_bufsize);
        }
        this->_C_own_buf (true);
        this->_C_bufsize = __new_size;
        this->_C_buffer  = __new_buf;
    }

    // increment the end put pointer by one position
    this->setp (this->_C_buffer, this->_C_buffer + __slen + 1);
    this->pbump (__slen);

    // set get area if in in|out mode 
    if (this->_C_is_inout ()) {
        // use the relative offset of gptr () from eback() to set the new gptr
        // although they are invalid, the offset is still valid
        char_type* __gptr_new =   this->_C_buffer
                                + (this->gptr () - this->eback ());  
        // N.B. pptr() has already been incremented  
        this->setg (this->_C_buffer, __gptr_new, this->epptr());
    }

    return this->sputc (__c);
}

template<class _CharT, class _Traits, class _Allocator>
_TYPENAME basic_stringbuf<_CharT, _Traits, _Allocator>::int_type
basic_stringbuf<_CharT, _Traits, _Allocator>::pbackfail (int_type __c)
{
    _RWSTD_ASSERT (this->_C_is_valid ());

    int_type __retval;

    if (!this->_C_putback_avail ())
        return traits_type::eof ();
    
    if (traits_type::eq (traits_type::to_char_type (__c), this->gptr ()[-1])
        || this->_C_is_eof (__c)) {
        // "put back" original value
        this->gbump (-1);
        __retval = traits_type::not_eof (__c);
    }
    else if (this->_C_is_out ()) {
        // overwrite existing value with new value
        this->gbump (-1);
        *this->gptr () = traits_type::to_char_type (__c);
        __retval = __c;
    }
    else
        __retval = traits_type::eof ();
    
    return __retval;
}

template<class _CharT, class _Traits, class _Allocator>
_TYPENAME basic_stringbuf<_CharT, _Traits, _Allocator>::int_type
basic_stringbuf<_CharT, _Traits, _Allocator>::underflow () {
  return this->_C_read_avail () ? traits_type::to_int_type (*this->gptr ()) : traits_type::eof ();
}

template<class _CharT, class _Traits, class _Allocator>
basic_streambuf<_CharT, _Traits>*
basic_stringbuf<_CharT, _Traits, _Allocator>::setbuf (char_type* __buf,
                                                      streamsize __n)
{
    _RWSTD_ASSERT (this->_C_is_valid ());

    if (!__buf && !__n)   // 27.7.1.3, p16
        return this;

    if (__n < _C_strlen() || !this->_C_is_out())  
        return 0;   // failure

    bool __own_old_buf = this->_C_own_buf ();

    streamsize __slen = _C_strlen ();
    
    typedef _RWSTD_ALLOC_TYPE (allocator_type, char_type) _ValueAlloc;

    if (0 == __buf) {
        __buf = _ValueAlloc ().allocate (__n);
        this->_C_own_buf (true);
    }
    else
        this->_C_own_buf (false);
        
    traits_type::copy (__buf, this->_C_buffer, __slen);   

    if (__own_old_buf)
        _ValueAlloc ().deallocate (this->_C_buffer, this->_C_bufsize);
    
    this->_C_buffer  = __buf;
    this->_C_bufsize = __n;
    streamsize __pptr_off = this->pptr () - this->pbase ();  // save pptr pos
    this->setp (this->_C_buffer, this->_C_buffer + __slen);
    this->pbump (__pptr_off);   // ... and restore it
 
    // set get area if in in|out mode
    if (this->_C_is_inout()) {
        // use the relative offset of gptr () from eback() to set the new gptr
        // (although the pointers are invalid, the offset is still valid)
        char_type* __gptr_new =   this->_C_buffer
                                + (this->gptr () - this->eback ());  
        this->setg (this->_C_buffer, __gptr_new, this->epptr());
    }
    
    return this;
}


template<class _CharT, class _Traits, class _Allocator>
_TYPENAME basic_stringbuf<_CharT, _Traits, _Allocator>::pos_type
basic_stringbuf<_CharT, _Traits, _Allocator>::
seekoff (off_type __off, ios_base::seekdir __way, ios_base::openmode __which)
{
    _RWSTD_ASSERT (this->_C_is_valid ());

    // should implicitly hold as long as ios::seekdir is an enum
    _RWSTD_ASSERT (   ios_base::beg == __way
                   || ios_base::cur == __way
                   || ios_base::end == __way);
        
    streamsize __newoff = 0;

    if (__which & ios_base::in) {
        if (!this->_C_is_in () || !this->gptr ())
            return pos_type (off_type (-1));
        // do the checks for in|out mode here
        if (__which & ios_base::out) {
            if ((__way & ios_base::cur) || !this->_C_is_out ())
                return pos_type (off_type (-1));
        }     
        switch (__way) {

        case ios_base::beg:
            __newoff = 0;
            break;
            
        case ios_base::cur:
            __newoff = this->gptr () - this->eback ();
            break;
        
        case ios_base::end:
            __newoff = this->egptr () - this->eback ();
        }

        if ((__newoff + __off) < 0
            || (this->egptr () - this->eback ()) < (__newoff + __off))
            return pos_type (off_type (-1));

        this->setg (this->eback (),
                    this->eback () + __newoff + __off,
                    this->egptr ());

    }
    
    if (__which & ios_base::out) {
        if (!this->_C_is_out () || !this->pptr ())
            return pos_type (off_type (-1));

        switch (__way) {

        case ios_base::beg:
            __newoff = 0;
            break;
            
        case ios_base::cur:
            __newoff = this->pptr () - this->pbase ();
            break;
            
        case ios_base::end:
            __newoff = this->epptr () - this->pbase ();
            break;
        }
        
        if ((__newoff + __off) < 0
            || (this->epptr () - this->pbase ()) < (__newoff + __off))
            return pos_type (off_type (-1));

        this->setp (this->pbase (), this->epptr ());
        this->pbump (__newoff + __off);
    }

    return pos_type (__newoff + __off);
}


template<class _CharT, class _Traits, class _Allocator>
_TYPENAME basic_stringbuf<_CharT, _Traits, _Allocator>::pos_type
basic_stringbuf<_CharT, _Traits, _Allocator>::
seekpos (pos_type __sp, ios_base::openmode __which)
{
    _RWSTD_ASSERT (this->_C_is_valid ());

    streamsize __newoff = __sp;
    
    // return invalid pos if no positioning operation succeeds
    pos_type      __retval = pos_type (off_type (-1));
    
    if (__newoff < 0)
        return pos_type (off_type (-1));

    // see 27.7.1.3 p.11 for required conditions 
    if ((__which & ios_base::in) && this->_C_is_in () && this->gptr()) {
        
        if ((this->eback () + __newoff) > this->egptr ())
            return pos_type (off_type (-1));
        
        this->setg (this->eback (),
                    this->eback () + __newoff,
                    this->egptr ());

        __retval = __sp;
    }
        
    if ((__which & ios_base::out) && this->_C_is_out () && this->pptr()) {

        if ((this->pbase () + __newoff) > this->epptr ()) 
            return pos_type (off_type (-1));
        
        this->setp (this->pbase (), this->epptr ());
        this->pbump (__newoff);

        __retval = __sp;
    }
    
    return __retval;
}

template<class _CharT, class _Traits, class _Allocator>
streamsize basic_stringbuf<_CharT, _Traits, _Allocator>::
xsputn (const char_type *__s, streamsize __n)
{
    _RWSTD_ASSERT (this->_C_is_valid ());

    if (!__s || (__n == 0) || !this->_C_is_out ())
        return 0;

    if ((this->pptr () + __n) >= this->_C_buf_end ()) {
        // need to (re)allocate space in the buffer
        streamsize __new_size =
            max (streamsize (this->_C_bufsize + __n),
                 streamsize (_RW::__rw_new_capacity (this->_C_bufsize, this)));
        // catch memory allocation failure
        _TRY {
            setbuf (0, __new_size);
        }
        _CATCH (...) {
            return 0;
        }
    }

    traits_type::copy (this->pptr (), __s, __n);

    // if put extended the put area, set the new end pointer
    if ((this->pptr () + __n) >= this->epptr ()) {
        // save the original offset of pptr() ...
        streamsize __pptr_off = (this->pptr() - this->pbase ());
        this->setp (this->pbase (), this->pptr () + __n);
        this->pbump(__pptr_off);  // ... and restore it
        
        if (this->_C_is_in ())   // need to extend the get area, too
            this->setg (this->eback (), this->gptr (), this->epptr ());
    }

    this->pbump (__n);

    return __n;
}


_RWSTD_NAMESPACE_END   // std
