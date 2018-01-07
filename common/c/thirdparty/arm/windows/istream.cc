/***************************************************************************
 *
 * istream.cc - istream definitions
 *
 * $Id: istream.cc 91768 2005-08-10 18:58:48Z drodgman $
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

_RWSTD_NAMESPACE_BEGIN (__rw)

template <class _CharT, class _Traits, class _NativeType>
_STD::basic_istream<_CharT, _Traits>&
__rw_extract (_STD::basic_istream<_CharT, _Traits> &__strm,
              _NativeType                          &__val)
{
    _RWSTD_ASSERT (0 != __strm.rdbuf ());

    typedef _STD::istreambuf_iterator<_CharT, _Traits> _Iter;
    typedef _STD::num_get<_CharT, _Iter>               _NumGet;

    _STD::ios_base::iostate __err = _STD::ios_base::goodbit;

    _TYPENAME _STD::basic_istream<_CharT, _Traits>::sentry __ipfx (__strm);

    if (__ipfx) {
        _TRY {
            _USE_FACET (_NumGet, __strm.getloc ())
                .get (_Iter (__strm), _Iter (), __strm, __err, __val);
        }
        _CATCH (...) {
            if (__strm.setstate (_STD::ios_base::badbit, 0 /* no throw */))
                _RETHROW;
        }
    }

    if (_STD::ios_base::goodbit != __err)
        __strm.setstate (__err);

    return __strm;
}


_RWSTD_NAMESPACE_END   // __rw


_RWSTD_NAMESPACE_BEGIN (std)


template<class _CharT, class _Traits>
basic_istream<_CharT, _Traits>&
basic_istream<_CharT, _Traits>::_C_ipfx (int __flags)
{
    _RWSTD_ASSERT (0 != this->rdbuf ());

    ios_base::iostate __err = this->rdstate ();

    // extension - zero out irrespective of state so that gcount()
    // called after any failed extraction (even formatted) reports 0
    _C_chcount = 0;

    if (ios_base::goodbit == __err) {

        if (this->tie ())
            this->tie ()->flush ();

        if (_C_skipws & __flags) {

            int_type __c = this->rdbuf ()->sgetc ();

            // avoid reference counting in MT environments
            locale &__loc = this->getloc ();

            for ( ; ; ) {

                if (traits_type::eq_int_type (__c, traits_type::eof ())) {
                    __err =  ios_base::eofbit;
                    break;
                }

                if (!(isspace)(traits_type::to_char_type (__c), __loc))
                    break;

                __c = this->rdbuf ()->snextc ();

                // extension - always count _all_ extracted characters
                ++_C_chcount;
            }
        }
    }

    // call setstate only after preparation is done in case it throws
    if (ios_base::goodbit != __err)
        this->setstate (__err | (_C_faileof & __flags ? ios_base::failbit : 0));

    return *this;
}


template<class _CharT, class _Traits>
_TYPENAME basic_istream<_CharT, _Traits>::int_type
basic_istream<_CharT, _Traits>::
_C_unsafe_get (streamsize *__cnt   /* = 0     */,               
               int_type    __delim /* = eof() */,
               int         __flags /* = 0     */)
{
    _RWSTD_ASSERT (0 != this->rdbuf ());

    ios_base::iostate __err = ios_base::goodbit;

    // initialize in case sgetc() below throws
    int_type __c = traits_type::eof ();

    _TRY {

        // get next char without extracting
        __c = this->rdbuf ()->sgetc ();

        // extract delimiter or eos only if asked to
        // extract whitespace only if it doesn't terminate input
        if (traits_type::eq_int_type (__c, traits_type::eof ()))
            __err =   ios_base::eofbit
                    | (__flags & _C_faileof ? ios_base::failbit : 0);
        else if (   (   _C_eatdelim & __flags
                     || !traits_type::eq_int_type (__c, __delim))
                 && (   _C_eatnull & __flags
                     || !traits_type::eq (__c, char_type ()))
                 && (   !(_C_wsterm & __flags)
                     || !(isspace)(traits_type::to_char_type (__c),
                                   this->getloc()))) {
            if (traits_type::eq_int_type (this->rdbuf ()->sbumpc (),
                                          traits_type::eof ()))
                __err = ios_base::failbit;
            else if (__cnt)
                ++*__cnt;   // increment number of extracted chars
        }
    }
    _CATCH (...) {
        if (this->setstate (ios_base::badbit, 0 /* no throw */))
            _RETHROW;
    }

    if (__err)
        this->setstate (__err);

    return __c;    
}


template<class _CharT, class _Traits>
basic_istream<_CharT, _Traits>&
basic_istream<_CharT, _Traits>::
get (basic_streambuf<char_type, traits_type> *__sb, int_type __delim)
{
    _RWSTD_ASSERT (0 != __sb);
    _RWSTD_ASSERT (0 != this->rdbuf ());

    ios_base::iostate __err = ios_base::goodbit;   

    sentry __ipfx (*this, true);

    if (__ipfx) { 

        _TRY {
            for ( ; ; ) {
                int_type __c = this->rdbuf ()->sgetc ();

                if (traits_type::eq_int_type (__c, traits_type::eof ())) {
                    __err = ios_base::eofbit;
                    break;
                }

                if (traits_type::eq (traits_type::to_char_type (__c), __delim))
                    break;

                _TRY {
                    // do not rethrow on failure - 27.6.1.3, p12, bullet 4
                    __c = __sb->sputc (__c);
                }
                _CATCH (...) {
                    __c = traits_type::eof ();
                }

                if (traits_type::eq_int_type (__c, traits_type::eof ())) {
                    __err = ios_base::failbit;  
                    break;
                }

                ++_C_chcount;
                this->rdbuf ()->sbumpc ();  
            }
        }
        _CATCH (...) {
            if (this->setstate (ios_base::badbit, 0 /* no throw */))
                _RETHROW;
        }

        if (__err)
            this->setstate (__err);
    }

    // 27.6.1.3, p13
    if (!gcount ())
        this->setstate (ios_base::failbit);

    return *this;
}


template<class _CharT, class _Traits>
basic_istream<_CharT, _Traits>&
basic_istream<_CharT, _Traits>::read (char_type  *__s,
                                      streamsize  __n,
                                      int_type    __delim,
                                      int         __flags)
{
    // if __s is 0 function doesn't store characters (used by ignore)

    _RWSTD_ASSERT (0 <= __n);
    _RWSTD_ASSERT (0 != this->rdbuf ());

    // read and readline are unformatted (noskipws), others are formatted
    const sentry __ipfx (*this, !(_C_skipws & __flags));

    // 27.6.1.2.1, p1 and 27.6.1.3, p1: proceed iff sentry is okay
    if (__ipfx) {

        const char_type __eos = char_type ();

        // "clear" buffer if reading a null-terminated string
        if (_C_nullterm & __flags)
            traits_type::assign (*__s, __eos);

        const bool __getline =     (_C_failend | _C_nullterm)
                               == ((_C_failend | _C_nullterm) & __flags);

        // read at most n - 1 characters when null-terminating
        while (__n) {

            // handle 27.6.1.3, p17 and p18
            if (0 == --__n && _C_nullterm & __flags) {
                int_type __c;

                if (   !__getline
                    || !traits_type::eq_int_type (__c = this->rdbuf ()->sgetc(),
                                                  traits_type::eof ())
                    && !traits_type::eq_int_type (__c, __delim))
                    break;

                __flags &= ~_C_failend;   // prevent failbit from being set

                // will break out in the check below...
            }

            // get (possibly extract) next char
            const int_type __c = _C_unsafe_get (&_C_chcount, __delim, __flags);

            // eof or delimiter may terminate input
            if (   traits_type::eq_int_type (__c, traits_type::eof())
                || traits_type::eq_int_type (__c, __delim))
                break;

            const char_type __ch = traits_type::to_char_type (__c);

            // space or eos char may terminate input
            if (   !(_C_eatnull & __flags) && traits_type::eq (__ch, __eos)
                || _C_wsterm & __flags && (isspace)(__ch, this->getloc ()))
                break;

            // ignore input if __s is null
            if (__s)
                traits_type::assign (*__s++, __ch);
        }

        if (_C_nullterm & __flags && __s)
            traits_type::assign (*__s, __eos);
    }

    // fail if no chars extracted or if end of buffer has been reached
    if (   _C_failnoi & __flags && !gcount ()
        || _C_failend & __flags && !__n)
        this->setstate (ios_base::failbit);

    return *this;
}



// 27.6.1.3, p28
template<class _CharT, class _Traits>
basic_istream<_CharT, _Traits>&
basic_istream<_CharT, _Traits>::read (char_type *__s, streamsize __n)
{
    _RWSTD_ASSERT (0 <= __n);
    _RWSTD_ASSERT (0 != this->rdbuf ());

    // 27.6.1.3, p28: sets ios_base::failbit if !this->good()
    const sentry __ipfx (*this, true);

    if (__ipfx) {
        streamsize __nread = this->rdbuf ()->sgetn (__s, __n);

        if (__nread >= 0)
            _C_chcount = __nread;

        // 27.6.1.2.1, p1 and 27.6.1.3, p1: proceed iff sentry is okay
        if (__n != __nread)
            this->setstate (ios_base::eofbit | ios_base::failbit);
    }

    return *this;
}


template<class _CharT, class _Traits>
streamsize basic_istream<_CharT, _Traits>::
readsome (char_type *__s, streamsize __n)
{
    _RWSTD_ASSERT (0 != __s);
    _RWSTD_ASSERT (0 <= __n);
    _RWSTD_ASSERT (0 != this->rdbuf());

    _C_chcount = 0;

    if (!this->good ()) {
        this->setstate (ios_base::failbit);
        return 0;
    }
    
    streamsize __navail = this->rdbuf ()->in_avail ();
   
    if(-1 == __navail) {   
        this->setstate (ios_base::eofbit);
        return 0;
    }

    if (0 == __navail)
        return 0;

    if (__n > __navail)
        __n = __navail;

    read (__s, __n);
    return __n;
}


template<class _CharT, class _Traits>
_TYPENAME basic_istream<_CharT, _Traits>::pos_type
basic_istream<_CharT, _Traits>::tellg ()
{
    pos_type __p = off_type (-1);

    if (!this->fail ()) {

        _TRY {
            __p = this->rdbuf ()->pubseekoff (0, ios_base::cur, ios_base::in);
          }
        _CATCH (...) {
            if (this->setstate (ios_base::badbit, 0 /* no throw */))
                _RETHROW;
        }
    }

    return __p;
}


template<class _CharT, class _Traits>
basic_istream<_CharT, _Traits>&
basic_istream<_CharT, _Traits>::putback (char_type __c)
{
    ios_base::iostate __err = ios_base::goodbit;

    if (this->rdbuf ()) {
        
        sentry __ipfx (*this, true);

        if (__ipfx) {
      
            _TRY {
                if (traits_type::eq_int_type (this->rdbuf ()->sputbackc (__c), 
                                              traits_type::eof ()))
                    __err = ios_base::badbit;
            }
            _CATCH (...) {
                if (this->setstate (ios_base::badbit, 0 /* no throw */))
                    _RETHROW;
            }
        }
    }
    else
        __err = ios_base::badbit;

    if (__err)
        this->setstate (__err);

    return *this;
}


template<class _CharT, class _Traits>
basic_istream<_CharT, _Traits>&
basic_istream<_CharT, _Traits>::unget ()
{
    ios_base::iostate __err = ios_base::goodbit;

    if (this->rdbuf ()) {

        sentry __ipfx (*this, true);

        if (__ipfx) {

            _TRY {

                if (traits_type::eq_int_type (this->rdbuf ()->sungetc (), 
                                              traits_type::eof ()))
                    __err = ios_base::badbit;  
            }
            _CATCH (...) {
                if (this->setstate (ios_base::badbit, 0 /* no throw */))
                    _RETHROW;
            }
        }
    }
    else
        __err = ios_base::badbit;

    if (__err)
        this->setstate (__err);
    
    return *this;
}


template<class _CharT, class _Traits>
int basic_istream<_CharT, _Traits>::sync ()
{
    if (!this->rdbuf ())
        return -1;

    sentry __ipfx (*this, true);

    if (__ipfx) {

        _TRY {
            if (-1 != this->rdbuf ()->pubsync ())
                return 0;
        }
        _CATCH (...) {
            if (this->setstate (ios_base::badbit, 0 /* no throw */))
                _RETHROW;
        }

        this->setstate (ios_base::badbit);
    }

    return traits_type::eof ();
}


template<class _CharT, class _Traits, class _Allocator>
basic_istream<_CharT, _Traits>&
operator>> (basic_istream<_CharT, _Traits>&            __is, 
            basic_string<_CharT, _Traits, _Allocator>& __str)
{
    _RWSTD_ASSERT (0 != __is.rdbuf ());

    ios_base::iostate __err = ios_base::goodbit;

    _TRY {

        _TYPENAME basic_istream<_CharT, _Traits>::sentry __ipfx (__is);

        if (__ipfx) {

            // FIXME: code commented out to work around an HP aCC 3.14.10
            // bug #JAGac86264

            // typedef _TYPENAME
            //     basic_string<_CharT, _Traits, _Allocator>::size_type

            size_t __maxlen =
                __is.width () ? __is.width () : __str.max_size ();

            size_t __i = 0;

            __str.erase ();
            __str.resize (32);

            while (__maxlen != __i) {

                _TYPENAME _Traits::int_type __c = __is.rdbuf ()->sgetc ();

                if (_Traits::eq_int_type (__c, _Traits::eof ())) {
                    __err = ios_base::eofbit;
                    break;
                }

                // convert to char_type so that isspace works correctly
                _TYPENAME _Traits::char_type
                    __ch = _Traits::to_char_type (__c);

                if ((isspace)(__ch, __is.getloc ()))
                    break;

                __is.rdbuf ()->sbumpc ();

                if (__str.size () == __i)
                    __str.resize (__i * 2);

                _Traits::assign (__str [__i++], __ch);
            }
            __str.resize (__i);

            __is.width (0);

            if (!__i)
                __err |= ios_base::failbit;
        }
    }
    _CATCH (...) {
        if (__is.setstate (ios_base::badbit, 0 /* no throw */))
            _RETHROW;
    }

    if (__err)
        __is.setstate (__err);

    return __is;
}  


template<class _CharT, class _Traits, class _Allocator>
basic_istream<_CharT, _Traits>&
getline (basic_istream<_CharT, _Traits>&            __is, 
         basic_string<_CharT, _Traits, _Allocator>& __str, 
         _CharT                                     __delim)
{
    _RWSTD_ASSERT (0 != __is.rdbuf ());

    _TYPENAME basic_istream<_CharT, _Traits>::sentry __ipfx (__is, true);

    if (__ipfx) { 

        ios_base::iostate __err = ios_base::failbit;

        _TRY {

            // FIXME: code commented out to work around an HP aCC 3.14.10
            // bug #JAGac86264

            // typedef _TYPENAME
            //     basic_string<_CharT, _Traits, _Allocator>::size_type

            size_t __i = 0;

            __str.erase ();
            __str.resize (32);

            while (__str.max_size () != __i) {

                _TYPENAME _Traits::int_type __c = __is.rdbuf ()->sbumpc ();

                if (_Traits::eq_int_type (__c, _Traits::eof ())) {
                    // 21.3.7.9, p7
                    __err =   ios_base::eofbit
                            | (__i ? ios_base::goodbit : ios_base::failbit);
                    break;
                }

                if (_Traits::eq (_Traits::to_char_type (__c), __delim)) {
                    __err = ios_base::goodbit;
                    break;
                }

                if (__str.size () == __i)
                    __str.resize (__i * 2);

                _Traits::assign (__str [__i++], _Traits::to_char_type (__c));
            }
            __str.resize (__i);   // excluding __delim
        }
        _CATCH (...) {
            if (__is.setstate (ios_base::badbit, 0 /* no throw */))
                _RETHROW;
        }

        if (__err)
            __is.setstate (__err);
    }

    return __is;
}  


_RWSTD_NAMESPACE_END   // std
