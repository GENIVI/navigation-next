/***************************************************************************
 *
 * ios.cc - Definition for the Standard Library iostreams
 *
 * $Id: ios.cc 91768 2005-08-10 18:58:48Z drodgman $
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
basic_ios<_CharT, _Traits>&
basic_ios<_CharT, _Traits>::copyfmt (const basic_ios &__rhs)
{
    if (this == &__rhs)
        return *this;

    // may lock, fires erase events (27.4.4.2, p17), never unlocks
    _C_copyfmt (__rhs);

    // may still be locked, just establish a guard
    _RW::__rw_guard __guard (0);
    if (!(flags () & _RWSTD_IOS_NOLOCK))
        __guard._C_set (&_C_mutex);

    _C_tiestrm = __rhs._C_tiestrm;
    _C_fillch  = __rhs._C_fillch;

    _TRY {
        // fire copy events (27.4.4.2, p17), may temporarily unlock or throw
        _C_fire_event (copyfmt_event, true /* reentrant */);

        // assign exceptions last (27.4.4.2, p15, bullet 2)
        _C_except = __rhs._C_except;
    }
    _CATCH (...) {
        // must still assign exceptions
        _C_except = __rhs._C_except;

        // just rethrow caught exception (don't call clear())
        _RETHROW;
    }

    // leave state alone but throw an exception if necessary
    _C_unsafe_clear (rdstate ());

    return *this;
}


template<class _CharT, class _Traits>
void basic_ios<_CharT, _Traits>::init (streambuf_type *__sb)
{
    _RWSTD_MT_GUARD (flags () & _RWSTD_IOS_NOLOCK ? 0 : &_C_mutex);

    // NOTE: `sb' may point to a yet uninitialized object
    //       it is unsafe to reference any of its members

    _C_state   = 0 != (_C_strmbuf = __sb) ? goodbit : badbit;
    _C_tiestrm = 0;
    _C_except  = goodbit;
    _C_fmtfl   = skipws | dec;
    _C_wide    = 0;
    _C_prec    = 6;
    _C_fillch  = widen (' ');

#if     defined (_RWSTD_MULTI_THREAD)          \
    && !defined (_RWSTD_NO_EXT_REENTRANT_IO)   \
    && !defined (_RWSTD_NO_REENTRANT_IO_DEFAULT)

    // disable locking of iostream objects and their associated buffers
    // standard iostream objects will override in ios_base::Init::Init()
    _C_fmtfl  |= _RWSTD_IOS_NOLOCK | _RWSTD_IOS_NOLOCKBUF;

#endif   // _RWSTD_MULTI_THREAD && !_RWSTD_NO_REENTRANT_IO && ...
}


_RWSTD_NAMESPACE_END   // std
