/* stddef.h: ANSI 'C' (X3J11 Oct 88) library header, section 4.1.4 */

/* Copyright (C) ARM Ltd., 1999
 * All rights reserved
 * RCS $Revision: 100044 $
 * Checkin $Date: 2006-05-12 15:51:52 +0100 (Fri, 12 May 2006) $
 * Revising $Author: cadeniyi $
 */

/* Copyright (C) Codemist Ltd., 1988                            */
/* Copyright 1991 ARM Limited. All rights reserved.             */
/* version 0.05 */

/*
 * The following types and macros are defined in several headers referred to in
 * the descriptions of the functions declared in that header. They are also
 * defined in this header file.
 */

#ifndef __stddef_h
#define __stddef_h

  #ifndef __STDDEF_DECLS
  #define __STDDEF_DECLS
    #undef __CLIBNS
    #ifdef __cplusplus
        namespace std {
        #define __CLIBNS ::std::
        extern "C" {
    #else
      #define __CLIBNS
    #endif  /* __cplusplus */

typedef signed int ptrdiff_t;

#if defined(__cplusplus) || !defined(__STRICT_ANSI__)
 /* unconditional in C++ and non-strict C for consistency of debug info */
  typedef unsigned int size_t;
#elif !defined(__size_t)
  #define __size_t 1
  typedef unsigned int size_t;   /* others (e.g. <stdio.h>) also define */
   /* the unsigned integral type of the result of the sizeof operator. */
#endif

#ifndef __cplusplus  /* wchar_t is a builtin type for C++ */
  #if !defined(__STRICT_ANSI__)
  /* unconditional in non-strict C for consistency of debug info */
    #if defined(__WCHAR32)
      typedef unsigned int wchar_t; /* also in <stdlib.h> and <inttypes.h> */
    #else
      typedef unsigned short wchar_t; /* also in <stdlib.h> and <inttypes.h> */
    #endif
  #elif !defined(__wchar_t)
    #define __wchar_t 1
    #if defined(__WCHAR32)
      typedef unsigned int wchar_t; /* also in <stdlib.h> and <inttypes.h> */
    #else
      typedef unsigned short wchar_t; /* also in <stdlib.h> and <inttypes.h> */
    #endif
   /*
    * An integral type whose range of values can represent distinct codes for
    * all members of the largest extended character set specified among the
    * supported locales; the null character shall have the code value zero and
    * each member of the basic character set shall have a code value when used
    * as the lone character in an integer character constant.
    */
  #endif
#endif

#undef NULL  /* others (e.g. <stdio.h>) also define */
#define NULL 0
   /* null pointer constant. */

  /* EDG uses __INTADDR__ to avoid errors when strict */
  #define offsetof(t, memb) ((__CLIBNS size_t)__INTADDR__(&(((t *)0)->memb)))

    #ifdef __cplusplus
         }  /* extern "C" */
      }  /* namespace std */
    #endif /* __cplusplus */
  #endif /* __STDDEF_DECLS */


  #ifdef __cplusplus
    #ifndef __STDDEF_NO_EXPORTS
      using ::std::size_t;
      using ::std::ptrdiff_t;
    #endif 
  #endif /* __cplusplus */

#endif

/* end of stddef.h */
