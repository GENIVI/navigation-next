/* alloca.h: Stack based allocation */
/* Copyright 1999 ARM Limited. All rights reserved */

/*
 * RCS $Revision: 91721 $
 * Checkin $Date: 2005-08-08 19:30:07 +0100 (Mon, 08 Aug 2005) $
 * Revising $Author: drodgman $
 */

#ifndef __alloca_h
#define __alloca_h

/* alloca.h has nothing to export since alloca is a macro */
/*  #ifndef __ALLOCA_DECLS */
/*  #define __ALLOCA_DECLS */

    #undef __CLIBNS

    #ifdef __cplusplus
      namespace std {
          #define __CLIBNS ::std::      
          extern "C" {
    #else
      #define __CLIBNS
    #endif  /* __cplusplus */

#if defined(__cplusplus) || !defined(__STRICT_ANSI__)
 /* unconditional in C++ and non-strict C for consistency of debug info */
  typedef unsigned int size_t;
#elif !defined(__size_t)
  #define __size_t 1
  typedef unsigned int size_t;   /* see <stddef.h> */
#endif

#undef NULL
#define NULL 0                  /* see <stddef.h> */

typedef struct {
    void *__chunk;
    int __pos;
} __alloca_state;

#define alloca(n) (__CLIBNS __alloca(n))

void *__alloca(size_t /* size */);

/* Allocate a block of at least size bytes. Returns a pointer to the first
 * word, or NULL if the allocation isn't successful.
 * Deallocation of all blocks allocated in a function is done on return.
 * Alloca can not be used as a function pointer.
 */

    #ifdef __cplusplus
          }  /* extern "C" */
      }  /* namespace std */
      typedef std::__alloca_state __alloca_state;
    #endif /* __cplusplus */
/*  #endif */ /* __ALLOCA_DECLS */

/*  #ifdef __cplusplus */
/*    #ifndef __ALLOCA_NO_EXPORTS */
/*      nothing to export */
/*    #endif */
/*  #endif */ /*__cplusplus*/
#endif

/* end of alloca.h */
