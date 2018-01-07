/*****************************************************************************
 * 
 *  fs_port.h  - Port-specific definitions
 *
 *  Description:
 *      This file contains portability constructs that MAY need to change in 
 *      in order to get iType to build and run on your device. There are also
 *      some optional settings.
 *
 *      Please make sure that all the entries in this file make sense 
 *      for your machine, compiler and environment. We assume ANSI standard 
 *      include files.  If your system is not compliant you may need to 
 *      make additional changes.
 *
 *  Copyright (C) 2001-2009 Monotype Imaging Inc. All rights reserved.
 *
 *  Confidential information of Monotype Imaging Inc.
 *
 ****************************************************************************/

#ifndef FS_PORT_H
#define FS_PORT_H

#include "fs_config.h"      /* iType build configuration options */


//#ifndef _XOPEN_SOURCE
//#define _XOPEN_SOURCE
//#endif

//#ifndef _XOPEN_SOURCE_EXTENDED
//#define _XOPEN_SOURCE_EXTENDED
//#endif


/*****************************************************************************
 *
 *  System header files that are needed by iType.
 * 
 *  Some systems change these file names or reorganize the contents.
 *  If necessary, modify the file names below to match the needs of your system.
 *
 ****************************************************************************/
#include <stdio.h>      /* FILE, fopen() ... fclose() */
#include <stddef.h>     /* Common Definitions and Types: NULL, size_t, etc. */
#include <stdlib.h>     /* malloc(), free(), realloc() */
#include <string.h>     /* memcpy(), memset(), and strcmp() */
#include <errno.h>      /* system error numbers */
#include <setjmp.h>     /* set jump */


/*****************************************************************************
 *
 *  Define a set of iType basic types in terms of the C-language basic types.
 *
 *  Modify the typedefs below as necessary to match your system's basic types.
 *
 *  Make sure these types are set correctly!
 *
 *  It is imperative that FS_TINY be defined as a signed 8-bit character.
 *  Some compilers treat the “char” type as signed and others treat it as 
 *  unsigned. Using “signed char” generally clarifies things, but some 
 *  compilers do not allow “signed” before the “char” type.
 *
 ****************************************************************************/
typedef signed char FS_TINY;            /* signed 8 bit integer      */
typedef unsigned char FS_BYTE;          /* unsigned 8 bit integer    */
typedef short FS_SHORT;                 /* signed 16 bit integer     */
typedef unsigned short FS_USHORT;       /* unsigned 16 bit integer   */
typedef long FS_LONG;                   /* signed 32 bit integer     */
typedef unsigned long FS_ULONG;         /* unsigned 32 bit integer   */

#define SIZEOF_LONG 4                   /* in bytes ... sizeof(long) */
#define FS_FILENAME_MAX 256
typedef FILE FS_FILE;                   /* abstraction of stdio FILE equiv */


/*****************************************************************************
 *  Macros for system calls. 
 *
 *  Most implementors will not need to modify these. 
 *  Modify these macros only if the target platform has defined 
 *  other methods for these standard operations.
 *
 ****************************************************************************/
#define SYS_MEMCHR(a,b,c) memchr(a,b,c)
#define SYS_MEMCPY(a,b,c) memcpy(a,b,c)
#define SYS_MEMMOVE(a,b,c) memmove(a,b,c)
#define SYS_MEMSET(a,b,c) memset(a,b,c)
#define SYS_STRCHR(a,b) strchr(a,b)
#define SYS_STRNCPY(a,b,c) strncpy(a,b,c)
#define SYS_STRLEN(a) strlen(a)
#define SYS_STRCMP(a,b) strcmp(a,b)
#define SYS_STRNCMP(a,b,c) strncmp(a,b,c)
#define SYS_STRNCAT(a,b,c) strncat(a,b,c)
#define SYS_STRTOL(a,b,c) strtol(a,b,c)
#define SYS_STRTOUL(a,b,c) strtoul(a,b,c)
#define SYS_MALLOC(a) malloc(a)
#define SYS_REALLOC(a,b) realloc(a,b)
#define SYS_FREE(a) free(a)
#define SYS_STRRCHR(a,b) strrchr(a,b)
#define SYS_FOPEN(a,b) fopen(a,b)
#define SYS_FCLOSE(a) fclose(a)
#define SYS_FREAD(a,b,c,d) fread(a,b,c,d)
#define SYS_FSEEK(a,b,c) fseek(a,b,c)
#define SYS_FTELL(a) ftell(a)
#define SYS_FERROR(a) ferror(a)
#define SYS_QSORT(a,b,c,d) qsort(a,b,c,d)


/*****************************************************************************
 *
 *  Macro that indicates assembly code is available for doing fixed-point math
 *
 *  Description:
 *      Defining this macro causes fs_fixed.c to call assembly routines 
 *      instead of its built-in c-code, in several places, in order to 
 *      improve performance. The function calls are as follows: 
 *
 *      <TABLE>
 *      C-function      Corresponding assembly routine
 *      -----------     ------------------------------
 *      muldiv          muldiv_asm
 *      varmul          varmul_asm
 *      vardiv          vardiv_asm
 *      </TABLE>
 *
 *      The iType SDK includes assembly for:
 *          - ARMv4 running on a ARM9 core such as the ARM920
 *          - Microsoft Visual C compiler on x86 processors
 *
 *      For other systems, it is necessary to write your own assembly
 *      code in order to use this option.
 *
 *  Notes:
 *      This is an advanced optimization step. 
 *      It should only be undertaken after iType is otherwise
 *      fully integrated and test procedures are in place.
 *
 *  <GROUP fs-port>
 */
#undef HAS_ASM


/*****************************************************************************
 *
 *  Macro that indicates longjmp/setjmp error handling is NOT supported.
 *
 *  Description:
 *      By default, iType's core TrueType code uses the longjmp/setjmp 
 *      error-handling mechanism. If your compiler does not support 
 *      longjmp/setjmp then you should define KILL_SETJMP_LONGJMP below. 
 *      Defining this macro implements a different error-handling mechanism
 *      at the cost of a small drop in performance.
 *
 *  Notes:
 *      Symbian systems have known issues with longjmp/setjmp, so define
 *      this macro when using Symbian. Otherwise, this macro is 
 *      commonly left undefined.
 *
 *  <GROUP fs-port>
 */
#undef KILL_SETJMP_LONGJMP


/*****************************************************************************
 *
 *  Macros that define byte order options.
 *  DON'T CHANGE THESE!
 *  
 ****************************************************************************/
#define FS_BIG_ENDIAN      0    /* Big-endian ("Motorola") byte order */
#define FS_LITTLE_ENDIAN   1    /* Little-endian ("Intel") byte order */


/*****************************************************************************
 *
 *  Macro that defines the byte order of target platform.
 *
 *  Description:
 *      Set this macro to either FS_LITTLE_ENDIAN or FS_BIT_ENDIAN to 
 *      match the byte order of your platform.
 *
 *  <GROUP fs-port>
 */
#define FS_BYTE_ORDER   FS_LITTLE_ENDIAN


/*****************************************************************************
 *
 *  Macro that abstracts the 'const' keyword.
 *  
 *  Description:
 *      If your system does not support the 'const' keyword then comment
 *      out the lowercase 'const' 
 *
 *  <GROUP fs-port>
 */
#define FS_CONST const
    

/*****************************************************************************
 *
 *  Macro that abstracts the 'inline' keyword.
 *  
 *  Description:
 *      Inlining is used in several places within the iType code to improve
 *      performance.
 *
 *      Define the FS_INLINE macro to match your compiler's 'inline' keyword
 *      For example, when using Windows-MSVC, define FS_INLINE as:
 *      #define FS_INLINE __inline
 *
 *      If your compiler does not support 'inline', define FS_INLINE as:
 *      #define FS_INLINE
 *
 *  <GROUP fs-port>
 */
#define FS_INLINE inline


/*****************************************************************************
 *
 *  Macro that abstracts the 'void' keyword.
 *  
 *  Description:
 *      FS_VOID is used in place of the 'void' keyword. It is used for:
 *          - functions without return values: FS_VOID foo(int)
 *          - functions which return generic pointers: FS_VOID *foo(int)
 *          - functions which take a generic pointer as argument: foo(FS_VOID *)
 *      
 *      If your system does not support 'void', define FS_VOID as:
 *      #define FS_VOID int
 *
 *  <GROUP fs-port>
 */
#define FS_VOID void


/*****************************************************************************
 *
 *  Macro that abstracts 'void' keyword as used with function with no arguments.
 *  
 *  Description:
 *      FS_VOID_ARGS is used in place of the 'void' keyword to indicate an argument
 *      list in a function that takes no arguments: int foo(VOID_ARGS)
 *
 *      If your system does not support 'void' when used as an argument, 
 *      define VOID_ARGS as nothing: #define VOID_ARGS      
 *
 *  <GROUP fs-port>
 */
#define FS_VOID_ARGS void


/*****************************************************************************
 *
 *  Macro that indicates a memory management function exists that
 *  returns the number of bytes allocated for a given pointer.
 *
 *  Description:
 *      Some memory management systems have a function that takes a pointer
 *      as an argument and returns the number of bytes allocated for that
 *      pointer. For example, the win32 stdlib defines _msize(p). *  
 *      
 *      If your system has such a function, then define FS_FREE_SIZE_DEFINED 
 *      and then see also the FS_FREE_SIZE macro.
 *
 *  <GROUP fs-port>
 */
#undef FS_FREE_SIZE_DEFINED


/*****************************************************************************
 *
 *  Macro that defines the name of a system function that returns
 *  the number of bytes allocated for a given pointer.
 *
 *  Description:
 *      Some memory management systems have a function that takes a pointer
 *      as an argument and returns the number of bytes allocated for that
 *      pointer. For example, the win32 stdlib defines _msize(p). *  
 *      
 *      If your system has such a function then define FS_FREE_SIZE in terms
 *      of that function and also define the FS_FREE_SIZE_DEFINED macro.
 *
 *      Otherwise, FS_FREE_SIZE should do nothing and the FS_FREE_SIZE_DEFINED
 *      macro should be undefined.
 *
 *      The FS_FREE_SIZE and FS_FREE_SIZE_DEFINED macros are used only by
 *      iType's external memory model, which is used when FS_INT_MEM 
 *      is NOT defined (see fs_config.h). FS_FREE_SIZE simplifies the
 *      'free' operation, allowing iType to use the system to keep
 *      track of the number of bytes allocated.
 *
 *      If you are using FS_INT_MEM then FS_FREE_SIZE and FS_FREE_SIZE_DEFINED
 *      have no effect.
 *
 *  <GROUP fs-port>
 */
/* #define FS_FREE_SIZE(p) */


/*****************************************************************************
 *
 *  Macro that indicates 64-bit support.
 *
 *  Description:
 *      If 64-bit integers are available on your system, they can be used to
 *      improve the fixed point math performance by doing certain operations
 *      with 64-bit integers. In that case, define the HAS_FS_INT64 macro and
 *      provide a definition for FS_INT64 below.
 *
 *  <GROUP fs-port>
 */
#define HAS_FS_INT64


/*****************************************************************************
 *
 *  Typedef for 64-bit integer.
 *
 *  Description:
 *      If your system has a way of supporting 64-bit integers then specify
 *      the 64-bit integer type.
 *
 *  <GROUP fs-port>
 */
typedef long long FS_INT64;


/*****************************************************************************
 *
 *  Macro that specifies extra adornment for API functions.
 *
 *  Description:
 *      Some operating systems such as Windows require extra function 
 *      adornment depending on whether the function is to be exported 
 *      from or imported to a DLL.
 * 
 *      Since this feature does not apply to this platform, the macro 
 *      is defined to be nothing.
 *
 *  <GROUP fs-port>
 */
#define ITYPE_API


/*****************************************************************************
 *
 *  Macro that specifies a directory that contains fonts.
 *
 *  Description:
 *      This is a convenience for samples and test programs.
 *      It is not used directly the iType core library.
 *
 *      Define this directory relative to 'bin'.
 *
 *  <GROUP fs-port>
 */
#ifndef FS_FONTS_DIR
#define FS_FONTS_RELDIR "../iType/product/fontdata/"
#define FS_FONTS_DIR FsUtil_formDir(fsFontsDir, FS_FONTS_RELDIR)
#endif


/*****************************************************************************
 *
 *  Macro used to emit printf-formatted messages to a console (or file).
 *
 *  Description:
 *      The behavior defined here is to wrap the system printf function
 *      define in the standard include file stdio.h.
 *     
 *      The parameter must include parens, ie. FS_PRINTF(("Hello %s\n", world));
 * 
 *      If you wish to redefine FS_PRINTF, always include the braces { }
 *      For example: #define FS_PRINTF(message) { }
 *
 *  Parameters:
 *      message - [in] printf-like, parenthesis-surrounded message format
 *
 *  <GROUP fs-port>
 */
#define FS_PRINTF(message) { printf message ; fflush(stdout); }


/*****************************************************************************
 *
 *  Macro used to emit fprintf-formatted messages to a file.
 *
 *  Description:
 *      The behavior defined here is to wrap the system fprintf function
 *      defined in the standard include file stdio.h.
 *     
 *      The parameter must include parens, eg. FS_FPRINTF((fp,"Hello %s\n", world));
 * 
 *      If you wish to redefine FS_FPRINTF, always include the braces { }
 *      For example: #define FS_FPRINTF(message) { }
 *
 *  Parameters:
 *      fp      - [in] file pointer
 *      message - [in] fprintf-like, parenthesis-surrounded message format
 *                     include the file pointer
 *
 *  <GROUP fs-port>
 */
#define FS_FPRINTF(message) { fprintf message ; }


/*****************************************************************************
 *
 *  Macro used to emit sprintf-formatted messages to a string.
 *
 *  Description:
 *      The behavior defined here is to wrap the system sprintf function
 *      defined in the standard include file stdio.h.
 *     
 *      The parameter must include parens, eg. FS_SPRINTF((str,"Hello %s\n", world));
 * 
 *      If you wish to redefine FS_SPRINTF, always include the braces { }
 *      For example: #define FS_SPRINTF(message) { }
 *
 *  Parameters:
 *      str     - [in] string pointer
 *      message - [in] sprintf-like, parenthesis-surrounded message format
 *                     include the file pointer
 *
 *  <GROUP fs-port>
 */
#define FS_SPRINTF(message) { sprintf message ; }


/*****************************************************************************
 *
 *  Macro used to flush a file in conjunction with FS_FPRINTF.
 *  The behavior defined here is to wrap the system fflush function defined
 *  in the standard include file stdio.h.
 *
 *  Parameters:
 *      fp - [in] file pointer
 *
 *  Notes:
 *      Example: FS_FFLUSH(fp);
 * 
 *      If you wish to redefine FS_FFLUSH, always include the braces { }
 *      For example: 
 *  #define FS_FFLUSH(message) { }
 *  
 */
#define FS_FFLUSH(fp) { fflush(fp); }


/*****************************************************************************
 *
 *  Macro that defines whether writable static data is allowed.  
 *
 *  Description:
 *      Some embedded environments build code for ROM use and do not have
 *      an associated data segment.  This means that no variable 
 *      declarations can occur outside of functions.
 *
 *      Undefine this macro if writable static data is NOT allowed.
 *
 *  Note:
 *      constant static values are allowed, even if writable statics are not.
 *
 *  <GROUP port>
 */
#define GLOBAL_STATIC_FUNCTION_PTRS_OK


/**
 * Force single thread mmap implementation.
 */
#define FS_MAPPED_FONTS


/* The following macros only apply if FS_MULTI_PROCESS is defined */
#ifdef FS_MULTI_PROCESS

/*****************************************************************************
 *
 *  Macro that specifies a shared memory segment name.
 *
 *  Description:
 *      This macro is only used by multi-processing code, that is, if
 *      FS_MULTI_PROCESS is defined in fs_config.h. This is not common.
 *
 *  <GROUP fs-port>
 */
#define FS_SHM_OPEN_NAME    "iTypeSharedHeap"


#endif /* FS_MULTI_PROCESS */

#endif /* FS_PORT_H */
