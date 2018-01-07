// -*- C++ -*-
/***************************************************************************
 *
 * fstream.cc - Definition for the Standard Library file streams
 *
 * $Id: fstream.cc 91768 2005-08-10 18:58:48Z drodgman $
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

template<class _CharT, class _Traits>
basic_filebuf<_CharT, _Traits>*
basic_filebuf<_CharT, _Traits>::
open (const char *__name, ios_base::openmode __mode, long __prot)
{
    _RWSTD_ASSERT (0 != __name);
    _RWSTD_ASSERT (this->_C_is_valid ());

    if (is_open ())
        return 0;

    this->_C_iomode = __mode;

    _C_file._C_open (__name, __mode & ~ios_base::ate, __prot);
    
    if (!is_open ())
        return 0;

    this->setg (0, 0, 0);
    this->setp (0, 0);
    
    if (__mode & ios_base::ate) {
        if ((_C_cur_pos = _C_beg_pos = _C_file_seek (0, ios_base::end))
            == _RWSTD_INVALID_FPOS) {
            close ();
            return 0;
        }
    }
    else
        _C_cur_pos = _C_beg_pos = pos_type(); 

    return this;
}


template<class _CharT, class _Traits>
basic_filebuf<_CharT, _Traits>*
basic_filebuf<_CharT, _Traits>::close ()
{
    _RWSTD_ASSERT (this->_C_is_valid ());

    if (!is_open ())
        return 0;   // failure

    // avoid expensive call to overflow() unless necessary
    if (this->pptr () != this->pbase () && this->_C_is_eof (overflow ()))
        return 0;   // failure

    // write out any unshift sequence if necessary
    // (applies to multibyte, state dependent encodings only)
    if (this->_C_out_last () && !_C_unshift ())
        return 0;   // failure
            
    _C_cur_pos = _C_beg_pos = _RWSTD_INVALID_FPOS;

    this->setg (0, 0, 0);
    this->setp (0, 0);
    
    if (!_C_file._C_is_std () && !_C_file._C_close ())
        return 0;   // failure

    _C_file = _RWSTD_INVALID_FILE; 
    
    return this;
}


template<class _CharT, class _Traits>
streamsize
basic_filebuf<_CharT, _Traits>::showmanyc ()
{
    _RWSTD_ASSERT (this->_C_is_valid ());

    if (!this->_C_is_in () || !is_open () || _C_cur_pos == _RWSTD_INVALID_FPOS)
        return -1;

    // start with the number of chars in get area
    streamsize __retval = this->egptr () - this->gptr ();

    // no prob if this fails for non-seekable devices
    pos_type __end_pos = _C_file_seek (0, ios_base::end);

    if (__end_pos != _RWSTD_INVALID_FPOS) {

        // restore position within file only if seek succeeded
        _C_file_seek (_C_cur_pos, ios_base::beg);
        
        const _C_codecvt_t &__cvt =
            _USE_FACET (_C_codecvt_t, this->getloc ());

        if (__cvt.always_noconv ()) 
            __retval += __end_pos - _C_cur_pos;
        else
            // make most pessimistic conversion estimate
            __retval += (__end_pos - _C_cur_pos) / __cvt.max_length ();
    }
    
    return __retval > 0 ? __retval : 0;
}


template<class _CharT, class _Traits>
_TYPENAME basic_filebuf<_CharT, _Traits>::int_type
basic_filebuf<_CharT, _Traits>::uflow ()
{
    int_type __c = underflow ();
    
    if (!this->_C_is_eof(__c)) 
        this->gbump (1);
    
    return __c;
}


template<class _CharT, class _Traits>
_TYPENAME basic_filebuf<_CharT, _Traits>::int_type
basic_filebuf<_CharT, _Traits>::underflow ()
{
    _RWSTD_ASSERT (this->_C_is_valid ());

    this->setp (0, 0);          // invalidate put area

    this->_C_out_last (false);  // needed by close ()
    
    if (!this->_C_is_in () || !is_open()) 
        return traits_type::eof ();
    
    char_type* const __to_end = this->_C_buffer + this->_C_bufsize;
        
    // fill the buffer if it's empty

    if (this->gptr () == this->egptr()) {  // N.B.: gptr() could be null here
        
        // determine the maximum possible size of putback area (if any)
        // make sure putback area isn't too big - try to honor
        // _RWSTD_PBACK_SIZE if possible, otherwise shrink

        _C_pbacksize = min (this->_C_putback_avail (),
                            _RWSTD_PBACK_SIZE);

        _RWSTD_ASSERT (0 != this->_C_bufsize);

        if (_C_pbacksize == this->_C_bufsize)
            _C_pbacksize = this->_C_bufsize - 1;

        traits_type::move (this->eback(), this->gptr () - _C_pbacksize,
                           _C_pbacksize);
        
        //  fill the get area from the file, performing code conversion if
        //  necessary

        streamsize __nread = 0; // number of bytes read from file
        
        _C_beg_pos = _C_cur_pos; 
        
        const _C_codecvt_t &__cvt = _USE_FACET (_C_codecvt_t, this->getloc ());

        if (__cvt.always_noconv ()) { 
            // no conversion required
            __nread = (__to_end - this->_C_buffer) - _C_pbacksize;
            __nread = _C_file._C_read (this->_C_buffer + _C_pbacksize,
                                       sizeof (char_type), __nread);
            if (__nread < 0)
                return traits_type::eof ();       // error while reading

            this->setg (this->_C_buffer,
                        this->_C_buffer + _C_pbacksize,
                        this->_C_buffer + _C_pbacksize + __nread);

            // adjust the current position in the file,
            // taking into account CR/LF conversion on windows
            __nread += _C_crlf_intern_count (this->gptr (),
                                             this->gptr () + __nread);
        }
        else { // conversion required
            
            char          __xbuf[_RWSTD_DEFAULT_BUFSIZE];
            char*         __from_base = __xbuf;
            const char*   __from_next = 0;
            char_type*    __to_base   = this->_C_buffer + _C_pbacksize;
            char_type*    __to_next   = __to_base;

            state_type    __state     = _C_cur_pos.state ();

            codecvt_base::result __res = codecvt_base::ok;

            while (__to_next != __to_end) {

                // read only as many characters as we have positions left in 
                //   internal buffer - guarantees we won't read more characters
                //   than we can put into the internal buffer after conversion 
                //   and ending file position isn't in the middle of a shift
                //   sequence
                // N.B.: area between __xbuf and __from_base contains partially
                //   converted sequences left from previous read
                streamsize __n =
                    min (ptrdiff_t (__to_end - __to_next),
                         ptrdiff_t (sizeof __xbuf - (__from_base - __xbuf)));

                __n = _C_file._C_read (__from_base, sizeof (char), __n);

                if (0 > __n)
                    return traits_type::eof ();   // error while reading

                if (0 == __n)
                    break;   // reached eof

                // take into account CR/LF conversion on Win32
                __nread += __n +
                    _C_crlf_extern_count (__from_base, __from_base + __n);

                // adjust `n' to hold the number of external chars in buffer
                __n += __from_base - __xbuf;

                // convert any partially converted sequence from the previous
                // iteration (possibly empty) plus what we just read in
                __res = __cvt.in (__state, __xbuf, __xbuf + __n, __from_next,
                                  __to_base, __to_end, __to_next);

                if (codecvt_base::partial == __res) {
                    // compute the length of partially converted sequence
                    __n -= __from_next - __xbuf;

                    // copy the sequence to beginning of xbuf
                    memmove (__xbuf, __from_next, __n);

                    // will append external chars to end of the sequence
                    __from_base = __xbuf + __n;
                }
                else if (codecvt_base::ok == __res) 
                    __from_base = __xbuf;
                else    // codecvt_base::error
                    return traits_type::eof ();

                __to_base = __to_next;  // continue at end of converted seq
            }

            _RWSTD_ASSERT (codecvt_base::ok == __res);

            _C_cur_pos.state (__state);

            this->setg (this->_C_buffer, this->_C_buffer + _C_pbacksize,
                        __to_next);
        } // end conversion block
        
        if (__nread == 0)
            return traits_type::eof ();

        _C_cur_pos += __nread;
    }

    return traits_type::to_int_type (*this->gptr ());
}


template<class _CharT, class _Traits>
_TYPENAME basic_filebuf<_CharT, _Traits>::int_type
basic_filebuf<_CharT, _Traits>::overflow (int_type __c /* = eof () */)
{
    _RWSTD_ASSERT (this->_C_is_valid ());

    this->setg (0, 0, 0);           // invalidate the get area

    if (!this->_C_is_out () || !is_open()) 
        return traits_type::eof ();
    
    char_type*  __from_beg;     // pointer to start of sequence to write
    char_type*  __from_end;     // pointer to end of sequence to write
    char_type   __c_to_char;    // input arg converted to char_type
                                // (used only in unbuffered mode)

    if (this->_C_is_unbuffered ()) {
        __c_to_char = traits_type::to_char_type(__c);
        __from_beg = &__c_to_char;
        __from_end = __from_beg + 1;
    }
    else {
        __from_beg = this->_C_buffer;
        __from_end = this->pptr ();
    }
        
    if (this->pptr () == 0 && !this->_C_is_unbuffered ()) {
        // put area not valid yet - just need to initialize it
        this->setp (this->_C_buffer, this->_C_buf_end ());
    }     
    else if (   this->pptr () == this->epptr ()
             || this->_C_is_eof (__c)
             || this->_C_is_unbuffered ()) {

        //  flush put area to file,
        //  performing code conversion if necessary
        
        streamsize __nwrote = 0;                    // num chars to write
        state_type __state  = _C_cur_pos.state ();  // state of stream
        
        // N.B. the sentry constructed by the ostream currently
        //   seeks to the end of the file when in append mode
        //   but we're going to seek anyway because the user
        //   can manipulate the buffer directly        
        if (this->_C_iomode & ios_base::app)         
            _C_cur_pos = _C_file_seek (0, ios_base::end);
        
        _C_beg_pos = _C_cur_pos;

        const _C_codecvt_t &__cvt = _USE_FACET (_C_codecvt_t, this->getloc ());

        if (__cvt.always_noconv ()) {   

            // no conversion
            
            __nwrote = __from_end - __from_beg;;
            
            if (!_C_file._C_write (__from_beg, sizeof (char_type), __nwrote))
                return traits_type::eof ();  // error while writing            
        }
        
        else {
            
            // conversion required: we do this a chunk at a time
            // to avoid dynamic allocation of memory 
            
            char             __xbuf[_RWSTD_DEFAULT_BUFSIZE];
            char*            __xbuf_end  = __xbuf + sizeof __xbuf;
            char*            __to_next   = 0;
            const char_type* __from_next = 0;
            const char_type* __base      = __from_beg;
            
            while (__from_next != __from_end) {
                codecvt_base::result __res =
                    __cvt.out (__state, __base, __from_end, __from_next,
                               __xbuf, __xbuf_end, __to_next);
                if (   __res == codecvt_base::error
                    || ! _C_file._C_write (__xbuf, sizeof (char),
                                           __to_next - __xbuf))
                    return traits_type::eof ();
                // N.B. a partial conversion will result if there's
                // not enough space in the conversion buffer
                // to hold the converted sequence, but we're o.k.
                // since we'll be passing any remaining unconverted
                // characters (starting at __from_next) in the next iteration
                
                // __res is ok or partial
                __nwrote += __to_next - __xbuf +
                    _C_crlf_extern_count (__xbuf, __to_next);
                
                __base = __from_next;  // do the next chunk
            }
        } // end conversion block
        
        // adjust the current position in the file
        _C_cur_pos += __nwrote;
        _C_cur_pos.state (__state);
        
        // reset the put area
        if (!this->_C_is_unbuffered ())
            this->setp (this->_C_buffer, this->_C_buf_end ());
    }
    
    // now that there's room in the buffer, call sputc() recursively
    //  to actually place the character in the buffer (unless we're
    //   in unbuffered mode because we just wrote it out)
    if (!this->_C_is_eof (__c) && !this->_C_is_unbuffered ())
        this->sputc (__c);

    this->_C_out_last (true);   // needed by close ()
    
    return traits_type::not_eof (__c);
}


template<class _CharT, class _Traits>
_TYPENAME basic_filebuf<_CharT, _Traits>::int_type
basic_filebuf<_CharT, _Traits>::pbackfail (int_type __c)
{
    _RWSTD_ASSERT (this->_C_is_valid ());

    if (!is_open ())
        return traits_type::eof ();

    // we could get here if gptr = eback or if __c != *(gptr-1)
    if (!this->_C_putback_avail ()) {
        // try to make a putback area available
        
        if (this->seekoff (-1, ios_base::cur) == _RWSTD_INVALID_FPOS)
            return traits_type::eof ();
    
        if (this->_C_is_eof (underflow ()))
            return traits_type::eof ();
        
        this->gbump (1); 
    }
    
    if (traits_type::eq (traits_type::to_char_type (__c), *(this->gptr () - 1))
        || this->_C_is_eof (__c)) {
        // "put back" original value
        this->gbump (-1);
        return traits_type::not_eof (__c);
    }
    else if (this->_C_is_out ()) {
        // overwrite existing value with new value
        this->gbump (-1);
        *this->gptr () = traits_type::to_char_type (__c);
        return __c;
    }
    else
        return traits_type::eof ();
}
  

template<class _CharT, class _Traits>
basic_streambuf<_CharT, _Traits>*
basic_filebuf<_CharT, _Traits>::setbuf (char_type *__buf, streamsize __len)
{
    _RWSTD_ASSERT (this->_C_is_valid ());

    // sync the buffer to the external file so it can be deallocated
    if (sync () != 0)
        return 0;

    bool __own_old_buf = this->_C_own_buf ();
        
    if (__len > 0) {
        // replace the buffer...
        if (__buf == 0) {
            // ...newly allocated buffer...
            __buf = new char_type[__len]; // try to allocate before deleting 
            this->_C_own_buf (true);
        }
        else
            // ...or with supplied buffer
            this->_C_own_buf (false);
        
        if (__own_old_buf)
            delete [] this->_C_buffer;
        
        this->_C_buffer  = __buf;
        this->_C_bufsize = __len;
        this->_C_set_unbuffered (false); 
    }
    
    else {
        // unbuffer this stream
        if (__buf != 0)         // invalid input (__len == 0 => __buf == 0)
            return 0;

        // character buffer is preserved (used as get area only),
        // streambuf object unbuffered for writing

        // to put a streambuf object into an unbuffered mode (see 27.8.1.4,
        // p10) and affect the size of the get area, setbuf() should first
        // be called with the desired (non-zero) size and then again with
        // both arguments being 0
        this->_C_set_unbuffered (true);
    }
    
    this->setg (0, 0, 0);
    this->setp (0, 0);

    // a character buffer of nonzero size must exist
    // even in unbuffered mode
    _RWSTD_ASSERT (0 != this->_C_buffer);
    _RWSTD_ASSERT (0 != this->_C_bufsize);

    return this; 
}
  

// 27.8.1.4, p 11
template<class _CharT, class _Traits>
_TYPENAME basic_filebuf<_CharT, _Traits>::pos_type
basic_filebuf<_CharT, _Traits>::
seekoff (off_type __off, ios_base::seekdir __way, ios_base::openmode)
{
    _RWSTD_ASSERT (this->_C_is_valid ());

    if (!is_open ())
        return _RWSTD_INVALID_FPOS;

    int __width = _USE_FACET (_C_codecvt_t, this->getloc ()).encoding (); 

    // offset must be zero with multibyte encoding
    if (0 != __off && __width <= 0) 
        return _RWSTD_INVALID_FPOS;

    // sync the buffer... (this also invalidates the get/put area)
    if (sync () != 0)
        return _RWSTD_INVALID_FPOS;
    
    // ...and, if last op was output, append an unshift sequence
    if (this->_C_out_last ())
        _C_unshift ();
    
    // perform the seek
    pos_type __new_pos;
    if (__width > 0)
        __new_pos = _C_file_seek (__width * __off, __way);
    else
        __new_pos = _C_file_seek (0, __way);

    if (__new_pos != _RWSTD_INVALID_FPOS)
        _C_cur_pos = _C_beg_pos = __new_pos;
            
    this->_C_out_last (false);  // needed by close()

    return __new_pos;
}


template<class _CharT, class _Traits>
_TYPENAME basic_filebuf<_CharT, _Traits>::pos_type
basic_filebuf<_CharT, _Traits>::seekpos (pos_type __pos, ios_base::openmode)
{ 
    _RWSTD_ASSERT (this->_C_is_valid ());

    if (!is_open ())
        return _RWSTD_INVALID_FPOS;

    // flush the output area if it exists
    if (this->pptr () != 0) {
        if (this->_C_is_eof (this->overflow (traits_type::eof ())))
            return _RWSTD_INVALID_FPOS;
    }

    if (_C_file_seek (__pos, ios_base::beg) == _RWSTD_INVALID_FPOS)
        return _RWSTD_INVALID_FPOS;
    
    _C_cur_pos = _C_beg_pos = __pos;
    
    this->setg (0, 0, 0);
    this->setp (0, 0);
    
    this->_C_out_last (false);  // needed by close()

    return __pos;
}


template<class _CharT, class _Traits>
int
basic_filebuf<_CharT, _Traits>::sync ()
{
    _RWSTD_ASSERT (this->_C_is_valid ());

    // put area active
    if (this->pptr () != 0) {
        // flush the buffer to the file
        if (this->_C_is_eof (overflow (traits_type::eof ())))
            return -1;
    }
    
    // get area active
    if (this->gptr () != 0) {
        // get the number of chars consumed in the buffer
        streamoff __consumed = (this->gptr () - this->eback ()) - _C_pbacksize;

        const _C_codecvt_t &__cvt = _USE_FACET (_C_codecvt_t, this->getloc ());
        int __width = __cvt.encoding ();

        if (__width > 0) {
            // constant width conversion:
            // adjust the current position/state in the file,
            // taking into account CR/LF conversion on Win32
            _C_cur_pos = _C_beg_pos;
            _C_cur_pos += (__width * __consumed)
                + _C_crlf_intern_count (this->eback () + _C_pbacksize,
                                        this->gptr());
        }
        else {
            // This gets a bit tricky here because we don't know the external
            // file position corresponding to the position in the
            // internal buffer. To figure this out, we'll use the known
            // file position/state corresponding to the start of the buffer
            // (which we have carefully saved in _C_beg_pos) and 
            // convert the characters up to the current position in the
            // buffer, counting how many external chars result. We can
            // then use the offset from _C_beg_pos and the state
            // returned from codecvt::out() to construct the current file
            // position
            
            streamoff        __ext_chars = 0; // converted external chars
            
            char             __xbuf[_RWSTD_DEFAULT_BUFSIZE];
            char*            __xbuf_end  = __xbuf + sizeof __xbuf;
            char*            __to_next   = 0;
            const char_type* __from_next = 0;
            const char_type* __from_end  = this->gptr ();
            state_type       __state     = _C_beg_pos.state ();
            const char_type* __base      = this->eback () + _C_pbacksize;
        
            while (__from_next != __from_end) {
                codecvt_base::result __res =
                    __cvt.out (__state, __base, __from_end, __from_next,
                               __xbuf, __xbuf_end, __to_next);
                if (__res == codecvt_base::error)
                    return -1;

                // take into account CR/LF conversion on Win32
                __ext_chars += (__to_next - __xbuf) +
                    _C_crlf_extern_count (__xbuf, __to_next);
                
                // repeat with next chunk
                __base = __from_next;  
            }

            _C_cur_pos = _C_beg_pos + __ext_chars;
            _C_cur_pos.state (__state);
        }
        if (_C_file_seek (_C_cur_pos, ios_base::beg) == _RWSTD_INVALID_FPOS)
            return -1;

        traits_type::move (this->eback(), this->gptr () - _C_pbacksize,
                           _C_pbacksize);

        this->setg (this->eback(), this->eback() + _C_pbacksize,
                    this->eback() + _C_pbacksize);
    }

    _C_beg_pos = _C_cur_pos;

    this->setp (0, 0);

    return 0;
}


template<class _CharT, class _Traits>
streamsize
basic_filebuf<_CharT, _Traits>::xsputn (const char_type* __s, streamsize __len)
{
    _RWSTD_ASSERT (this->_C_is_valid ());

    if (!__s || (__len == 0) || !this->_C_is_out ())
        return 0;

    if (this->pptr () != 0 && (this->pptr () + __len) < this->epptr ()) {
        traits_type::copy (this->pptr (), __s, __len);
        this->pbump (int(__len));
    }
    else {
        for (streamsize __n = 0; __n < __len; ++__n) {
            if (this->_C_is_eof (this->sputc (*__s++)))
                return __n;
        }
    }
    return __len;
}


template<class _CharT, class _Traits>
basic_filebuf<_CharT, _Traits>*
basic_filebuf<_CharT, _Traits>::_C_open (_RW::__rw_file_t __frep,
                                         char_type *__buf,
                                         streamsize __n)  
{   
    if (is_open () || !setbuf (__buf,__n) || !__frep._C_is_open ())
        return 0;

    _C_file = __frep;

    this->_C_iomode = _C_file._C_get_mode ();

    this->setg (0, 0, 0);
    this->setp (0, 0);
    
    _C_cur_pos = _C_beg_pos = pos_type();    // offset == 0, state = state()
   
    return this;
}


//  write an unshift sequence if not in initial state
//   N.B.: this is a no-op unless the stream is imbued with user-defined
//   locale/facet because the default implementations of unshift()
//   always return noconv
//   returns: true on success

template<class _CharT, class _Traits>
bool
basic_filebuf<_CharT, _Traits>::_C_unshift ()
{
    // unshifting isn't necessary if encoding isn't state dependent or
    // if state is equivalent to initial state (determined by mbsinit())
    state_type __state = _C_cur_pos.state ();
    if (   _USE_FACET (_C_codecvt_t, this->getloc ()).encoding () >= 0
        || _MBSINIT (&__state) != 0)
        return true;
    
    // buf to hold unshift sequence - assumes that the shift
    //  sequence will be less than 64 chars (we can't safely
    //   use dynamic allocation because _C_unshift
    //   could be called as a result of memory alloc exception)

    char     __useq[64];
    char*    __useq_end = 0;

    codecvt_base::result __res =
        _USE_FACET (_C_codecvt_t, this->getloc ()).unshift (
            __state, __useq, __useq + sizeof __useq,
            __useq_end);

    streamoff __nbytes = __useq_end - __useq;
    
    // in the unlikely event that the buffer isn't big enough, assert
    _RWSTD_ASSERT(__res != codecvt_base::partial); 
    
    if (__res == codecvt_base::error)
        return false;
    if (__res == codecvt_base::noconv)
        return true;

    if (_C_file._C_write (__useq, sizeof (char), __nbytes))
        return false;

    _C_cur_pos += __nbytes;     // CR/LF conversion not an issue here
    
    return true;
}

_RWSTD_NAMESPACE_END  // std
