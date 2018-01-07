/* setjmp.h: ANSI 'C' (X3J11 Oct 88) library header, section 4.6 */

/* Copyright (C) ARM Ltd., 1999
 * All rights reserved
 * RCS $Revision: 91721 $
 * Checkin $Date: 2005-08-08 19:30:07 +0100 (Mon, 08 Aug 2005) $
 * Revising $Author: drodgman $
 */

/* Copyright (C) Codemist Ltd., 1988                            */
/* Copyright 1991 ARM Limited. All rights reserved.             */


/*
 * setjmp.h declares two functions and one type, for bypassing the normal
 * function call and return discipline (useful for dealing with unusual
 * conditions encountered in a low-level function of a program).
 */

#ifndef __setjmp_h
#define __setjmp_h

#define _ARMABI __declspec(__nothrow)
#define _ARMABI_NORETURN __declspec(__nothrow) __declspec(__noreturn)

  #ifndef __SETJMP_DECLS
  #define __SETJMP_DECLS

    #undef __CLIBNS

    #ifdef __cplusplus
      namespace std {
          #define __CLIBNS std::
          extern "C" {
    #else
      #define __CLIBNS
    #endif  /* __cplusplus */

#if _AEABI_PORTABILITY_LEVEL != 0 || (!defined _AEABI_PORTABILITY_LEVEL && __DEFAULT_AEABI_PORTABILITY_LEVEL != 0)
typedef __int64 jmp_buf[];      /* do not specify size at all */
#else
typedef __int64 jmp_buf[48];    /* size as defined in the ATEPCS */
                            /* an array type suitable for holding the data */
                            /* needed to restore a calling environment.    */
#endif
extern const int __aeabi_JMP_BUF_SIZE;
              
/* setjmp is a macro so that it cannot be used other than directly called. */
/* NB that ANSI declare that anyone who undefines the setjmp macro or uses */
/* (or defines) the name setjmp without including this header will get     */
/* what they deserve.  In addition, C++ requires setjmp to be a macro.     */

extern _ARMABI int setjmp(jmp_buf /*env*/);
   /* Saves its calling environment in its jmp_buf argument, for later use
    * by the longjmp function.
    * Returns: If the return is from a direct invocation, the setjmp function
    *          returns the value zero. If the return from a call to the longjmp
    *          function, the setjmp function returns a non zero value.
    */
#define setjmp(jmp_buf) (__CLIBNS setjmp(jmp_buf))

extern _ARMABI_NORETURN void longjmp(jmp_buf /*env*/, int /*val*/);
   /* Restores the environment saved by the most recent call to setjmp in the
    * same invocation of the program, with the corresponding jmp_buf argument.
    * If there has been no such call, or if the function containing the call
    * to setjmp has terminated execution (eg. with a return statement) in the
    * interim, the behaviour is undefined.
    * All accessible objects have values as of the time longjmp was called,
    * except that the values of objects of automatic storage duration that do
    * not have volatile type and have been changed between the setjmp and
    * longjmp calls are indeterminate.
    * As it bypasses the usual function call and return mechanism, the longjmp
    * function shall execute correctly in contexts of interrupts, signals and
    * any of their associated functions. However, if the longjmp function is
    * invoked from a nested signal handler (that is, from a function invoked as
    * a result of a signal raised during the handling of another signal), the
    * behaviour is undefined.
    * Returns: After longjmp is completed, program execution continues as if
    *          the corresponding call to setjmp had just returned the value
    *          specified by val. The longjmp function cannot cause setjmp to
    *          return the value 0; if val is 0, setjmp returns the value 1.
    */

    #ifdef __cplusplus
         }  /* extern "C" */    
      }  /* namespace std */
    #endif /* __cplusplus */
  #endif /* __SETJMP_DECLS */

  #if _AEABI_PORTABILITY_LEVEL != 0 && !defined _AEABI_PORTABLE
    #define _AEABI_PORTABLE
  #endif

  #ifdef __cplusplus
    #ifndef __SETJMP_NO_EXPORTS
      using std::jmp_buf;
      using std::setjmp;
      using std::longjmp;
    #endif 
  #endif /* __cplusplus */

#endif

/* end of setjmp.h */
