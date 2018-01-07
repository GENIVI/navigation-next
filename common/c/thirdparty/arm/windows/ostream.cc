/***************************************************************************
 *
 * ostream.cc - Definitions for the Standard Library ostream classes
 *
 * $Id: ostream.cc 91768 2005-08-10 18:58:48Z drodgman $
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
_STD::basic_ostream<_CharT, _Traits>&
__rw_insert (_STD::basic_ostream<_CharT, _Traits> &__strm,
             _NativeType                           __val)
{
    _STD::ios_base::iostate __err = _STD::ios_base::goodbit;

    _TYPENAME _STD::basic_ostream<_CharT, _Traits>::sentry __opfx (__strm);

    _TRY {

        if (__opfx) {

            typedef _STD::ostreambuf_iterator<_CharT, _Traits> _Iter;
            typedef _STD::num_put<_CharT, _Iter>               _NumPut;

            if (_USE_FACET (_NumPut, __strm.getloc ())
                .put (_Iter(__strm), __strm,
                      __strm.fill (), __val).failed ())
                __err = _STD::ios_base::badbit;

            __strm.width (0);
        } 
    }
    _CATCH (...) {
        if (__strm.setstate (_STD::ios_base::badbit, 0))
            _RETHROW;
    }

    if (__err)
        __strm.setstate (__err);

    return __strm;
}


// generic case (e.g., basic_ostream<wchar_t>, wchar_t*)
template <class _CharT, class _Traits>
inline _STD::streamsize
__rw_sputn (_STD::basic_ostream<_CharT, _Traits> &__strm,
            const _CharT*__s, _STD::streamsize __len)
{
    return __strm.rdbuf ()->sputn (__s, __len);

}


#ifndef _RWSTD_NO_FUNC_PARTIAL_SPEC

// specialized for char* (e.g., basic_ostream<wchar_t>, char*)
template <class _CharT, class _Traits>
inline _STD::streamsize
__rw_sputn (_STD::basic_ostream<_CharT, _Traits> &__strm,
            const char *__s, _STD::streamsize __len)
{
    _RWSTD_ASSERT (!__len || __len > 0 && 0 != __s);

    const _STD::ctype<_CharT>& __ctp =
        _USE_FACET (_STD::ctype<_CharT>, __strm.getloc ());

    for (_STD::streamsize __i = 0; __i < __len; ++__i) {
        _TYPENAME _STD::basic_ostream<_CharT, _Traits>::int_type __n =
            __strm.rdbuf ()->sputc (__ctp.widen (__s [__i]));
        if (_Traits::eq_int_type (__n, _Traits::eof ()))
            return __i;
    }

    return __len;
}


// specialized for basic_ostream<char> and char*
template <class _Traits>
inline _STD::streamsize
__rw_sputn (_STD::basic_ostream<char, _Traits> &__strm,
            const char*__s, _STD::streamsize __len)
{
    return __strm.rdbuf ()->sputn (__s, __len);
}

#else   // if defined (_RWSTD_NO_FUNC_PARTIAL_SPEC)

// specialized for basic_ostream<[w]char[_t], char_traits<[w]char[_t]> >
// and char*; any other specialization (e.g., user-defined traits) will be
// generated from the primary template

_RWSTD_SPECIALIZED_FUNCTION
inline _STD::streamsize
__rw_sputn (_STD::basic_ostream<char, _STD::char_traits<char> > &__strm,
            const char*__s, _STD::streamsize __len)
{
    return __strm.rdbuf ()->sputn (__s, __len);
}


inline _STD::streamsize
__rw_sputn (_STD::basic_ostream<wchar_t, _STD::char_traits<wchar_t> > &__strm,
            const char*__s, _STD::streamsize __len)
{
    _RWSTD_ASSERT (!__len || __len > 0 && 0 != __s);

    const _STD::ctype<wchar_t>& __ctp =
        _USE_FACET (_STD::ctype<wchar_t>, __strm.getloc ());

    for (_STD::streamsize __i = 0; __i < __len; ++__i) {
        _STD::basic_ostream<wchar_t, _STD::char_traits<wchar_t> >::int_type __n
            = __strm.rdbuf ()->sputc (__ctp.widen (__s [__i]));
        if (_STD::char_traits<wchar_t>::eq_int_type (__n,
                                            _STD::char_traits<wchar_t>::eof ()))
            return __i;
    }

    return __len;
}

#endif   // _RWSTD_NO_FUNC_PARTIAL_SPEC


template<class _CharT, class _Traits, class _StringT>
_STD::basic_ostream<_CharT, _Traits>&
__rw_insert (_STD::basic_ostream<_CharT, _Traits> &__strm,
             _StringT                             *__s,
             _STD::streamsize                     __len,
             _STD::streamsize                     __width)
{
    _RWSTD_ASSERT (0 != __strm.rdbuf ());
    _RWSTD_ASSERT (0 != __s);
      
    _STD::ios_base::iostate __err = _STD::ios_base::goodbit;

    _TRY {
  
        _TYPENAME _STD::basic_ostream<_CharT, _Traits>::sentry __opfx (__strm);

        if (__opfx) {

            // compute the number of fill chars to pad with
            // according to the rules described in 22.2.2.2.2, p19
            const _STD::streamsize __pad = __width - __len;
            const int __padbits = _STD::ios_base::adjustfield & __strm.flags ();

            // output left padding (output is right-aligned by default)
            if (   _STD::ios_base::left != __padbits
                && __pad != __strm._C_pad (__pad))
                __err = _STD::ios_base::badbit;

            // write out (not necessarily null-terminated) string
            if (   _STD::ios_base::goodbit == __err
                && __len && __len != __rw_sputn (__strm, __s, __len))
                __err = _STD::ios_base::badbit;

            // output right padding (output is left-aligned)
            if (   _STD::ios_base::goodbit == __err
                && _STD::ios_base::left == __padbits
                && __pad != __strm._C_pad (__pad))
                __err = _STD::ios_base::badbit;
        }
    }
    _CATCH (...) {
        if (__strm.setstate (_STD::ios_base::badbit, 0 /* nothrow */))
            _RETHROW;
    }

    if (__err)
        __strm.setstate (__err);  

    return __strm;
}


_RWSTD_NAMESPACE_END   // __rw


_RWSTD_NAMESPACE_BEGIN (std)


template<class _CharT, class _Traits>
basic_ostream<_CharT, _Traits>&
basic_ostream<_CharT, _Traits>::_C_opfx ()
{
    if (this->tie ())
        this->tie ()->flush ();

    if (this->_C_is_sync ()) {
    
        if (_C_is_cout ())       // [w]cout
            fflush (stdout);
        else if (_C_is_std ())   // [w]cerr or [w]clog
            fflush (stderr);
    }

    // cast to ios_base::seekdir makes MSVC 6.0/sp3 happy
    if (   this->rdbuf () 
        && this->rdbuf ()->_C_mode () & ios_base::app)
        this->rdbuf ()->pubseekoff (0, ios_base::seekdir (ios_base::end),
                                    ios_base::out);

    return *this;
}


template<class _CharT, class _Traits>
basic_ostream<_CharT, _Traits>&
basic_ostream<_CharT, _Traits>::
operator<< (basic_streambuf<_CharT, _Traits>& __sb)
{
    ios_base::iostate __err = ios_base::goodbit;

    bool __extracted = false;    // successfully extracted chars from __sb?
    
    if (__sb._C_mode () & ios_base::in) {

        sentry __opfx (*this);

        _TRY {

            if (__opfx) {
                if (_Traits::eq_int_type (__sb.sgetc (), _Traits::eof ()))
                    __err = ios_base::failbit;
                else {
                    for ( ; ; ) {
                        int_type __c = __sb.sbumpc ();
                        if (_Traits::eq_int_type (__c, _Traits::eof ()))
                            break;
                        if (_Traits::eq_int_type (this->rdbuf ()->sputc (__c),
                                                  _Traits::eof ())) {
                            __err = ios_base::failbit;
                            break;
                        }
                        __extracted = true;
                    } 
                }
            }
        }
        _CATCH (...) {
            if (this->setstate (ios_base::badbit, 0))
                _RETHROW;
        }
    }
    // 27.6.2.5.3.8
    if (!__extracted) 
        __err = ios_base::failbit;
    
    if (__err)
        this->setstate (__err);

    return *this;
}


_RWSTD_NAMESPACE_END   // std
