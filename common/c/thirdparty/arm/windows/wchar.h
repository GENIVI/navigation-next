/* wchar.h: C99 header */
/* Copyright 2002 ARM Limited. All rights reserved.            */
/*
 * RCS $Revision: 100044 $
 * Checkin $Date: 2006-05-12 15:51:52 +0100 (Fri, 12 May 2006) $
 * Revising $Author: cadeniyi $
 */

#ifndef __wchar_h
#define __wchar_h

  #ifndef __WCHAR_DECLS
  #define __WCHAR_DECLS

    #undef __CLIBNS
    #ifdef __cplusplus
        #define __CLIBNS ::std::
        namespace std {
        extern "C" {
    #else /* ndef __cplusplus */
      #define __CLIBNS
    #endif /* ndef __cplusplus */

    #if defined(__cplusplus) || !defined(__STRICT_ANSI__) || !defined(__size_t)
     /* always defined in C++ and non-strict C for consistency of debug info */
      typedef unsigned int size_t;   /* see <stddef.h> */
      #if !defined(__cplusplus) && defined(__STRICT_ANSI__)
        #define __size_t 1
      #endif
    #endif

    #ifndef __cplusplus  /* wchar_t is a builtin type for C++ */
      #if !defined(__STRICT_ANSI__) || !defined(__wchar_t)
       /* always defined in non-strict C for consistency of debug info */
        #if defined(__WCHAR32)
          typedef unsigned int wchar_t; /* see <stddef.h> */
        #else
          typedef unsigned short wchar_t; /* see <stddef.h> */
        #endif
        #ifdef __STRICT_ANSI__
          #define __wchar_t 1
        #endif
      #endif
    #endif

    #if defined(__cplusplus) || !defined(__STRICT_ANSI__) || !defined(__wint_t)
     /* always defined in C++ and non-strict C for consistency of debug info */
      typedef int wint_t;   /* see also <wctype.h> */
      #if !defined(__cplusplus) && defined(__STRICT_ANSI__)
        #define __wint_t 1
      #endif
    #endif

    #undef WEOF
    #define WEOF ((__CLIBNS wint_t)-1)

        /* limits of wchar_t */
        /* NB we have to undef and redef because they're defined in both
         * stdint.h and wchar.h */
    #undef WCHAR_MIN
    #undef WCHAR_MAX

    #if defined(__WCHAR32)
      #define WCHAR_MIN   0
      #define WCHAR_MAX   0xffffffffU
    #else
      #define WCHAR_MIN   0
      #define WCHAR_MAX   65535
    #endif


    /* ANSI forbids va_list to be defined here */
    /* keep in step with <stdarg.h> and <stdio.h> */
    #if defined(__cplusplus) || !defined(__STRICT_ANSI__) || !defined(__va_list_defined)
    /* always defined in C++ and non-strict C for consistency of debug info */
      #if defined(__APCS_ADSABI)
        typedef int *__va_list[1];
      #else
        typedef struct __va_list __va_list;
      #endif
      #if !defined(__cplusplus) && defined(__STRICT_ANSI__)
        #define __va_list_defined 1
      #endif
    #endif

    /*
     * mbstate_t must be able to contain all the data from an
     * incomplete MB character, plus shift states. CJK-specific
     * encodings seem to tend to have at most three bytes per
     * character, and a single-figure number of shift states, so 32
     * bits is sufficient for any of these. UTF-8 encoding a full
     * 32-bit value is the hardest thing to deal with, and in that
     * situation the worst case is having to store 25 bits of
     * character (1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx) plus
     * a few bits saying what sort of state is being stored. This is
     * still easy enough to fit into 32 bits, so I think a single
     * 32-bit integer is enough in here.
     */
    typedef struct {
        unsigned int __state, __state2;
    } mbstate_t;

    /*
     * `struct tm' must be declared in this header as an incomplete
     * type.
     */
    struct tm;

    /*
     * The wide-character I/O functions, wide-character formatted I/O
     * functions, and wcsftime() are not currently supported.
     * However, we do support swprintf() and friends, because they're
     * simple string processing routines with no wide char I/O
     * required.
     */
#pragma __printf_args
    int swprintf(wchar_t * /*restrict*/ /*s*/, size_t /*n*/,
                 const wchar_t * /*restrict*/ /*format*/, ...);
#pragma __printf_args
    int vswprintf(wchar_t * /*restrict*/ /*s*/, size_t /*n*/,
                  const wchar_t * /*restrict*/ /*format*/, __va_list /*arg*/);
#pragma __scanf_args
    int swscanf(const wchar_t * /*restrict*/ /*s*/,
                const wchar_t * /*restrict*/ /*format*/, ...);
#pragma __scanf_args
    int vswscanf(const wchar_t * /*restrict*/ /*s*/,
                 const wchar_t * /*restrict*/ /*format*/, __va_list /*arg*/);

    wint_t btowc(int /*c*/);
    wint_t wctob(int /*c*/);
    int mbsinit(const mbstate_t * /*ps*/);
    size_t mbrlen(const char * /*restrict*/ /*s*/, size_t /*n*/,
                  mbstate_t * /*restrict*/ /*ps*/);
    size_t mbrtowc(wchar_t * /*restrict*/ /*pwc*/,
                   const char * /*restrict*/ /*s*/,
                   size_t /*n*/, mbstate_t * /*restrict*/ /*ps*/);
    size_t wcrtomb(char * /*restrict*/ /*s*/, wchar_t /*wc*/,
                   mbstate_t * /*restrict*/ /*ps*/);
    size_t mbsrtowcs(wchar_t * /*restrict*/ /*dst*/,
                     const char ** /*restrict*/ /*src*/,
                     size_t /*len*/, mbstate_t * /*restrict*/ /*ps*/);
    size_t wcsrtombs(char * /*restrict*/ dst,
                     const wchar_t ** /*restrict*/ /*src*/,
                     size_t /*len*/, mbstate_t * /*restrict*/ /*ps*/);

    /*
     * Wide-character string-to-number conversions. Parallel to strto*.
     */
    double wcstod(const wchar_t * /*nptr*/, wchar_t ** /*endptr*/);
    long int wcstol(const wchar_t * /*nptr*/, wchar_t **/*endptr*/,
                    int /*base*/);
    unsigned long int wcstoul(const wchar_t * /*nptr*/,
                              wchar_t ** /*endptr*/, int /*base*/);
#if !defined(__STRICT_ANSI__) || (defined(__STDC_VERSION__) && 199901L <= __STDC_VERSION__)
    long long int wcstoll(const wchar_t * /*restrict*/ /*nptr*/,
                          wchar_t ** /*restrict*/ /*endptr*/, int /*base*/);
    unsigned long long int wcstoull(const wchar_t * /*restrict*/ /*nptr*/,
                                    wchar_t ** /*restrict*/ /*endptr*/,
                                    int /*base*/);
  #endif

    /*
     * General wide-character string utilities.
     */
    wchar_t *wcscpy(wchar_t * /*restrict*/ /*s1*/,
                    const wchar_t * /*restrict*/ /*s2*/);
    wchar_t *wcsncpy(wchar_t * /*restrict*/ /*s1*/,
                     const wchar_t * /*restrict*/ /*s2*/, size_t /*n*/);
    wchar_t *wmemcpy(wchar_t * /*restrict*/ /*s1*/,
                     const wchar_t * /*restrict*/ /*s2*/, size_t /*n*/);
    wchar_t *wmemmove(wchar_t * /*restrict*/ /*s1*/,
                      const wchar_t * /*restrict*/ /*s2*/, size_t /*n*/);
    wchar_t *wcscat(wchar_t * /*restrict*/ /*s1*/,
                    const wchar_t * /*restrict*/ /*s2*/);
    wchar_t *wcsncat(wchar_t * /*restrict*/ /*s1*/,
                     const wchar_t * /*restrict*/ /*s2*/, size_t /*n*/);
    int wcscmp(const wchar_t * /*restrict*/ /*s1*/,
               const wchar_t * /*restrict*/ /*s2*/);
    int wcsncmp(const wchar_t * /*restrict*/ /*s1*/,
                const wchar_t * /*restrict*/ /*s2*/, size_t /*n*/);
    int wcscoll(const wchar_t * /*restrict*/ /*s1*/,
                const wchar_t * /*restrict*/ /*s2*/);
    size_t wcsxfrm(wchar_t * /*restrict*/ /*s1*/,
                   const wchar_t * /*restrict*/ /*s2*/, size_t /*n*/);
    int wmemcmp(const wchar_t * /*restrict*/ /*s1*/,
                const wchar_t * /*restrict*/ /*s2*/, size_t /*n*/);
    size_t wcscspn(const wchar_t * /*restrict*/ /*s1*/,
                   const wchar_t * /*restrict*/ /*s2*/);
    size_t wcsspn(const wchar_t * /*restrict*/ /*s1*/,
                  const wchar_t * /*restrict*/ /*s2*/);
    wchar_t *wcstok(wchar_t * /*restrict*/ /*s1*/,
                    const wchar_t * /*restrict*/ /*s2*/,
                    wchar_t ** /*restrict*/ /*ptr*/);
    wchar_t *wmemchr(const wchar_t * /*restrict*/ /*s*/, wchar_t /*c*/,
                     size_t /*n*/);
    size_t wcslen(const wchar_t * /*restrict*/ /*s*/);
    wchar_t *wmemset(wchar_t * /*restrict*/ /*s*/, wchar_t /*c*/, size_t /*n*/);

#ifdef __cplusplus
    /*
     * C++'s overloading allows us to provide versions of these
     * functions which search a const string and return a const
     * pointer into it, and separate versions which search a
     * non-const string and return a non-const pointer.
     */
    const wchar_t *wcschr(const wchar_t * /*restrict*/ /*s*/, wchar_t /*c*/);
    const wchar_t *wcspbrk(const wchar_t * /*restrict*/ /*s1*/,
                           const wchar_t * /*restrict*/ /*s2*/);
    const wchar_t *wcsrchr(const wchar_t * /*restrict*/ /*s*/, wchar_t /*c*/);
    const wchar_t *wcsstr(const wchar_t * /*restrict*/ /*s1*/,
                          const wchar_t * /*restrict*/ /*s2*/);
    extern "C++" inline wchar_t *wcschr(wchar_t * /*restrict*/ __s, wchar_t __c)
      { return const_cast<wchar_t *>(wcschr(const_cast<const wchar_t *>(__s), __c)); }
    extern "C++" inline wchar_t *wcspbrk(wchar_t * /*restrict*/ __s1, const wchar_t * /*restrict*/ __s2)
      { return const_cast<wchar_t *>(wcspbrk(const_cast<const wchar_t *>(__s1), __s2)); }
    extern "C++" inline wchar_t *wcsrchr(wchar_t * /*restrict*/ __s, wchar_t __c)
      { return const_cast<wchar_t *>(wcsrchr(const_cast<const wchar_t *>(__s), __c)); }
    extern "C++" inline wchar_t *wcsstr(wchar_t * /*restrict*/ __s1,
                          const wchar_t * /*restrict*/ __s2)
      { return const_cast<wchar_t *>(wcsstr(const_cast<const wchar_t *>(__s1), __s2)); }
#else
    /*
     * C's approximation to the above is to have all of these
     * functions search a const string and return a non-const
     * pointer. This is the only way to prevent compile errors in
     * all sensible uses of the functions, but unfortunately
     * renders them unable to spot a lot of the possible error
     * cases.
     */
    wchar_t *wcschr(const wchar_t * /*restrict*/ /*s*/, wchar_t /*c*/);
    wchar_t *wcspbrk(const wchar_t * /*restrict*/ /*s1*/,
                     const wchar_t * /*restrict*/ /*s2*/);
    wchar_t *wcsrchr(const wchar_t * /*restrict*/ /*s*/, wchar_t /*c*/);
    wchar_t *wcsstr(const wchar_t * /*restrict*/ /*s1*/,
                    const wchar_t * /*restrict*/ /*s2*/);
#endif

    #ifdef __cplusplus
         }  /* extern "C" */
      }  /* namespace std */
    #endif /* __cplusplus */
  #endif /* __WCHAR_DECLS */

  #if _AEABI_PORTABILITY_LEVEL != 0 && !defined _AEABI_PORTABLE
    #define _AEABI_PORTABLE
  #endif

  #ifdef __cplusplus
    #ifndef __WCHAR_NO_EXPORTS
      using ::std::size_t;
      using ::std::wint_t;
      using ::std::mbstate_t;
      using ::std::tm;
      using ::std::swprintf;
      using ::std::vswprintf;
      using ::std::btowc;
      using ::std::wctob;
      using ::std::mbsinit;
      using ::std::mbrlen;
      using ::std::mbrtowc;
      using ::std::wcrtomb;
      using ::std::mbsrtowcs;
      using ::std::wcsrtombs;
      using ::std::wcstod;
      using ::std::wcstol;
      using ::std::wcstoul;
#if !defined(__STRICT_ANSI__) || (defined(__STDC_VERSION__) && 199901L <= __STDC_VERSION__)
      using ::std::wcstoll;
      using ::std::wcstoull;
#endif /* !defined(__STRICT_ANSI__) || (defined(__STDC_VERSION__) && 199901L <= __STDC_VERSION__) */
      using ::std::wcscpy;
      using ::std::wcsncpy;
      using ::std::wmemcpy;
      using ::std::wmemmove;
      using ::std::wcscat;
      using ::std::wcsncat;
      using ::std::wcscmp;
      using ::std::wcsncmp;
      using ::std::wcscoll;
      using ::std::wcsxfrm;
      using ::std::wmemcmp;
      using ::std::wcschr;
      using ::std::wcscspn;
      using ::std::wcspbrk;
      using ::std::wcsrchr;
      using ::std::wcsspn;
      using ::std::wcsstr;
      using ::std::wcstok;
      using ::std::wmemchr;
      using ::std::wcslen;
      using ::std::wmemset;
    #endif /* __WCHAR_NO_EXPORTS */
  #endif /* __cplusplus */
#endif /* ndef __wchar_h */

/* end of wchar.h */
