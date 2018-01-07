/***************************************************************************
 *
 * bitset - definitions of out-of-line bitset members
 *
 * $Id: bitset.cc 91768 2005-08-10 18:58:48Z drodgman $
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


#if !defined (_RWSTD_NO_MEMBER_TEMPLATES)

template <size_t _Size>
template <class _CharT, class _Traits, class _Allocator>
bitset<_Size>::
bitset (const basic_string<_CharT,_Traits,_Allocator>                &__str,
        _TYPENAME basic_string<_CharT,_Traits,_Allocator>::size_type  __pos,
        _TYPENAME basic_string<_CharT,_Traits,_Allocator>::size_type  __n)

#else

template <size_t _Size>
bitset<_Size>::bitset (const string& __str, size_t __pos, size_t __n) 

#endif // _RWSTD_NO_MEMBER_TEMPLATES

{
    const size_t __slen = __str.size();

    _RWSTD_REQUIRES (__pos <= __slen,
                     (_RWSTD_ERROR_OUT_OF_RANGE,
                      _RWSTD_FUNC ("bitset::bitset (const basic_string&, "
                                   "size_t, size_t)"), __pos, __slen));

    const size_t __rlen = __n < (__slen - __pos) ? __n : __slen - __pos;
    const size_t __M = _Size >= __rlen ? __rlen : _Size;

    reset ();

    for (size_t __j = __pos; __j < __M + __pos; __j++) {
        const char __c = __str [__slen - __j - 1];

        _RWSTD_REQUIRES (__c == '0' || __c == '1',
                         (_RWSTD_ERROR_INVALID_ARGUMENT,
                          _RWSTD_FUNC ("bitset::bitset (const basic_string&, "
                                       "size_t, size_t)")));

        if (__c == '1') 
            set (__j - __pos);
    }
}


_RWSTD_NAMESPACE_END   // std


_RWSTD_NAMESPACE_BEGIN (__rw)

template <class _CharT, class _Traits, size_t _Size> 
_STD::basic_istream<_CharT, _Traits>&  
__rw_extract_bitset (_STD::basic_istream<_CharT, _Traits> &__strm,
                     _STD::bitset<_Size>                  &__x)
{
    _STD::string __str;
    __str.reserve (_Size);

    typedef _STD::basic_istream<_CharT, _Traits> _Istream;

    _TYPENAME _Istream::iostate __err (__strm.goodbit);

    const _TYPENAME _Istream::sentry __ipfx (__strm);

    if (__ipfx) {

        _TRY {
            // 23.3.5.3, p5 - extract at most _Size chars

            // get next char without extracting
            _TYPENAME _Traits::int_type __c = __strm.rdbuf ()->sgetc ();

            for (_RWSTD_C::size_t __i = 0; __i != _Size; ++__i) {
                if (_Traits::eq_int_type (_Traits::eof (), __c)) {
                    __err = __strm.eofbit;
                    break;
                }

                // convert and narrow
                char __ch = __strm.narrow (_Traits::to_char_type (__c),
                                           _TYPENAME _Traits::char_type ());
                if ('0' != __ch && '1' != __ch)
                    break;

                // append and advance
                __str += __ch;
                __c = __strm.rdbuf ()->snextc ();
            }
        }
        _CATCH (...) {
            __err = __strm.badbit;
        }
    }

    if (_Size && 0 == __str.size ())
        __err |= __strm.failbit;
    else if (!(__strm.badbit & __err))
        __x = _STD::bitset<_Size>(__str);

    if (__err)
        __strm.setstate (__err);

    return __strm;
}

_RWSTD_NAMESPACE_END   // __rw
