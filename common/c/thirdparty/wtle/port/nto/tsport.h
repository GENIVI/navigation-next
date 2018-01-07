/*****************************************************************************
 *
 *  tsport.h  - Port-specific definitions
 *
 *  Copyright (C) 2002-2007 Monotype Imaging Inc. All rights reserved.
 *
 *  Confidential Information of Monotype Imaging Inc.
 *
 ****************************************************************************/

#ifndef TSPORT_H
#define TSPORT_H


/*****************************************************************************
 *
 *  Include the system header files that are needed by the base library.
 * 
 *  There are environments where the contents of these files are put in
 *  other files.  If necessary, MODIFY the include statements so that the
 *  appropriate files are included.
 *
 ****************************************************************************/

#include <stdarg.h>     /* Variable argument macro definitions */
#include <stddef.h>     /* Common Definitions and Types: NULL, size_t, etc. */
#include <stdlib.h>     /* General Utilities: memory management */
#include <malloc.h>     /* Memory management */
#include <stdio.h>      /* Input/output: FILE, fopen() ... fclose() */
#include <string.h>     /* String handling and memcpy(), memset() */
#include <limits.h>     /* Sizes of integer types */
#include <assert.h>     /* Diagnostics: assert macro */
#include <slog2.h>      /* Slogger for QNX */

// Default log code used in slog2 calls 
#define LOG_CODE_DEFAULT     0x0000

// Performance related log code used in slog2 calls from performance related modules.
#define LOG_CODE_PERFORMANCE 0x00FF

#define LOGE(pLogTag, pFmt, ...) slog2f(NULL, LOG_CODE_DEFAULT, SLOG2_ERROR, pLogTag"::"pFmt, ##__VA_ARGS__)
#define LOGA(pLogTag, pFmt, ...) slog2f(NULL, LOG_CODE_DEFAULT, SLOG2_INFO, pLogTag"::"pFmt, ##__VA_ARGS__)

#ifndef NDEBUG
#define LOGW(pLogTag, pFmt, ...) slog2f(NULL, LOG_CODE_DEFAULT, SLOG2_WARNING, pLogTag"::"pFmt, ##__VA_ARGS__)
#define LOGD(pLogTag, pFmt, ...) slog2f(NULL, LOG_CODE_DEFAULT, SLOG2_DEBUG1, pLogTag"::"pFmt, ##__VA_ARGS__)
#endif // NDEBUG

/*****************************************************************************
 *
 *  Macro to distinguish DEBUG and NDEBUG builds.
 *
 *  This macro must be defined correcly, otherwise the code size 
 *  and performance of release builds will be degraded.
 *
 *  MODIFY these macros as necessary to match these macros.
 */
#ifdef NDEBUG
#define TS_NDEBUG
#endif


/*****************************************************************************
 *
 *  Macros for system calls. Most implementors will not need to modify these.
 *  Modify these macros only if the target platform has defined 
 *  other methods for these standard operations
 */
#define TS_MEMCPY(a,b,c)    memcpy(a,b,c)
#define TS_MEMSET(a,b,c)    memset(a,b,c)
#define TS_MEMCMP(a,b,n)    memcmp(a,b,n)
#define TS_MEMMOVE(a,b,n)   memmove(a,b,n)
#define TS_STRCPY(a,b)      strcpy(a,b)
#define TS_STRNCPY(a,b,c)   strncpy(a,b,c)
#define TS_STRLEN(a)        strlen(a)
#define TS_STRCMP(a,b)      strcmp(a,b)
#define TS_STRCAT(a,b)      strcat(a,b)
#define TS_STRNCAT(a,b,c)   strncat(a,b,c)

#define TS_FOPEN(filename, mode, fileMgr)               fopen(filename, mode)
#define TS_FCLOSE(stream, fileMgr)                      fclose(stream)
#define TS_FREAD(buffer, size, count, stream, fileMgr)  fread(buffer, size, count, stream)
#define TS_WRITE(buffer, size, count, stream, fileMgr)  fwrite(buffer, size, count, stream)
#define TS_FSEEK(stream, offset, origin, fileMgr)       fseek(stream, offset, origin)
#define TS_FTELL(stream, fileMgr)                       ftell(stream)
#define TS_REWIND(stream, fileMgr)                      rewind(stream)
#define TS_FGETC(stream, fileMgr)                       fgetc(stream)
#define TS_FPUTC(value, stream, fileMgr)                fputc(value, stream)

#define TS_ASSERT(expr)  assert(expr)


/*****************************************************************************
 *
 *  Macros used to emit print-formatted messages to a console (or file).
 *  The behavior defined here is to wrap the system functions defined
 *  in the standard include file stdio.h.
 *
 *  Define these as appropriate given the available resources.
 *  Actual print output is not necessary for proper functionality
 *  of the core WTLE library. The output aids in debugging and
 *  is used to help port test applications such as text2png.
 *
 *  TS_PRINTF is used when the message should be printed for both 
 *  release and debug builds. For example, the test applications use
 *  this macro to print instructions and output.
 *
 *  TS_PRINTF_DEBUG is used when the message is intended only for
 *  debug builds. It is recommended that this macro be defined
 *  to NoOp 'do nothing' macros for release builds to avoid including
 *  the message strings in these builds.
 *
 *  Parameters:
 *      message - [in] printf-like, parenthesis-surrounded message format
 *
 *  Notes:
 *      Parameter must include parens, ie. ("Hello %s\n", "world")
 *  
 *  <GROUP port>
 */
#define TS_PRINTF(message)     LOGA("WTLE", message)
#define TS_FPRINTF(message)    { fprintf message; }

#ifndef TS_NDEBUG
#define TS_PRINTF_DEBUG(message)  TS_PRINTF(message) 
#else
#define TS_PRINTF_DEBUG(message)  {}
#endif


/*****************************************************************************
 *
 * Macros for memory allocation routines. Most implementors will not need to 
 * modify these. Modify these macros only if the target platform has defined 
 * other methods for these standard operations. 
 *
 */
#define TS_MALLOC(a)        malloc(a)
#define TS_FREE(a)          free(a)
#define TS_CALLOC(a,b)      calloc(a,b)
#define TS_REALLOC(a,b)     realloc(a,b)


/*****************************************************************************
 *
 * Macros for memory allocation routines for systems that have versions
 * of these routines that take as parameters the file and line number where 
 * the allocation or free took place. 
 * 
 * These are intended for use with debug builds only. These are only active
 * when the TS_MEM_FILE_AND_LINE_INFO macro is defined in tsconfig.h
 *
 * Below is an example of how these macros could be used if your system
 * does not have memory functions that take the file and line as parameters.
 *
 * Another way to get information about memory usage in WTLE is to define the
 * TS_MEM_TRACING in tsconfig.h. Doing so will cause information 
 * about memory calls to be printed to stdout.
 *
 * #define TS_MALLOC_DEBUG(a,f,l)    malloc(a); {TS_PRINTF(("malloc %ld bytes at %s %ld\n", a, f, l));}
 * #define TS_FREE_DEBUG(a,f,l)      free(a); {TS_PRINTF(("free %p at %s %ld\n", a, f, l));}
 * #define TS_CALLOC_DEBUG(a,b,f,l)  calloc(a,b); {TS_PRINTF(("calloc %ld bytes at %s %ld\n", a*b, f, l));}
 * #define TS_REALLOC_DEBUG(a,b,f,l) realloc(a,b); {TS_PRINTF(("realloc %p to %ld bytes at %s %ld\n", a, b, f, l));}
 *
 */
#define TS_MALLOC_DEBUG(a,f,l)        malloc(a)
#define TS_FREE_DEBUG(a,f,l)          free(a)
#define TS_CALLOC_DEBUG(a,b,f,l)      calloc(a,b)
#define TS_REALLOC_DEBUG(a,b,f,l)     realloc(a,b)


/*****************************************************************************
 
 *  Macro that wraps sprintf functionality. It is used in
 *  in some cases for debugging and for information reporting.
 *  If you do not have this functionality then define the macro as: 
 *  #define TS_SNPRINT(args) (0)
 *
 *  Parameters:
 *      message - [in] printf-like, parenthesis-surrounded message format
 *
 *  Returns:
 *      Returns the number of characters printed into string
 *
 *  <GROUP port>
 */
#define TS_SNPRINTF(message)  snprintf message;


/*****************************************************************************
 
 * Porting hint: the TS_QSORT macro is currently used only in the Thai 
 * dictionary code. If you don't have a qsort function then you can write one,
 * or to get the port up and running more quickly, just stub out the TS_QSORT
 * macro and do not add the Thai dictionary. Once up and running you may
 *  properly define the macro and make the dictionary calls.
 */
#define TS_QSORT(base, num, size, compareFunc)  qsort(base, num, size, compareFunc)


/*****************************************************************************
 *
 *  Here we define a set of types in terms of the C-language basic types.
 *  These new types are not used directly, instead, they are intermediate forms
 *  used by the API code to define the actual basic types used throughout the
 *  API and code.  The intermediate forms are defined in order
 *  to insulate the API from the portability code.
 *
 *  MODIFY these types as necessary to match the system's basic types.
 *
 ****************************************************************************/

typedef signed char     TsInt8_t;   /*   signed  8-bit integer */
typedef signed short    TsInt16_t;  /*   signed 16-bit integer */
typedef signed long     TsInt32_t;  /*   signed 32-bit integer */
typedef unsigned char   TsUInt8_t;  /* unsigned  8-bit integer */
typedef unsigned short  TsUInt16_t; /* unsigned 16-bit integer */
typedef unsigned long   TsUInt32_t; /* unsigned 32-bit integer */

typedef ptrdiff_t       TsPtrDiff_t;/* to be used for pointer math */
typedef size_t          TsSize_t;   /* used with sizeof operator */
typedef wchar_t         TsWChar_t;  /* wide character type */

typedef TsUInt16_t      TsRefCount; /* used for storing object reference counts */


/*****************************************************************************
 *
 * Macros for min/max integer values.
 *
 * MODIFY these macros if needed to match the definition on your platform.
 *
 * <GROUP port>
 */
#define TS_UINT32_MIN   0x00000000UL        /* minimum unsigned 32-bit value */
#define TS_UINT32_MAX   0xffffffffUL        /* maximum unsigned 32-bit value */
#define TS_INT32_MIN    (-2147483647L - 1)  /* minimum (signed) 32-bit value */
#define TS_INT32_MAX    2147483647L         /* maximum (signed) 32-bit value */


/*****************************************************************************
 *
 *  Here we define macros to decorate functions, and headers.
 *
 *  MODIFY these as necessary for your platform.
 *
 ****************************************************************************/


#ifdef TS_EXPORT_DLL
#define TS_DECLSPEC /* __declspec(dllexport) */
#elif defined (TS_IMPORT_DLL)
#define TS_DECLSPEC /* __declspec(dllimport) */
#else
#define TS_DECLSPEC 
#endif

#ifdef __cplusplus

#define TS_BEGIN_HEADER extern "C" {
#define TS_END_HEADER   }  
#define TS_CDECL        extern "C"
#define TS_EXPORT(x)  TS_CDECL x      

#else

#define TS_BEGIN_HEADER
#define TS_END_HEADER  
#define TS_CDECL        
#define TS_EXPORT(x)  x

#endif


/*****************************************************************************
 *
 *  Macro that wraps the "inline" keyword.
 *
 *  Uncomment "inline" if the inline keyword is supported and desired.
 *
 *  <GROUP port>
 */  
#define TS_INLINE   /* inline */


/*****************************************************************************
 *
 *  Macro that defines whether writable static data is allowed.  
 *
 *  Description:
 *      Some embedded environments that build code for ROM use and that
 *      do not have an associated data segment.  This means that no variable 
 *      declarations can occur outside of functions.
 *
 *      Undefine this macro if writable static data is NOT allowed.
 *
 *  Note:
 *      constant static values are allowed, even if writable statics are not.
 *
 *  <GROUP port>
 */
#define TS_WRITABLE_STATICS_ALLOWED


/*****************************************************************************
 *
 *  Macro that declares that function pointers be stored and initialized
 *  at run time.
 *
 *  Description:
 *      Some embedded environments that build code for ROM use do not allow
 *      function pointers to be initialized with symbols at build-time.
 *
 *      If this macro is defined then objects have a function pointer table 
 *      and that table is initialized when the object is constructed.
 *
 *      Most systems will not need to define this macro.
 *
 *  <GROUP port>
 */
/* #define TS_USE_DYNAMIC_FUNCTION_POINTER_TABLES */


/*****************************************************************************
 *
 *  Macros that define the particular byte order options supported.
 *  Don't change these.
 *  
 ****************************************************************************/

#define TS_BIG_ENDIAN      0    /* Big-endian byte order */
#define TS_LITTLE_ENDIAN   1    /* Little-endian byte order */


/*****************************************************************************
 *
 *  Macro that define the byte order for this build.  Should be defined
 *  as one of the byte order options defined above.  
 *
 *  MODIFY this macro to match the endianess of your platform.
 *
 *  <GROUP port>
 */
#define TS_BYTE_ORDER TS_LITTLE_ENDIAN


/*****************************************************************************
 *
 *  Macro that enables external user data functionality within all of the
 *  major WTLE objects.
 *
 *  ENABLE this macro if you need to manage external data. 
 *
 *  <GROUP port>
 */
/* #define TS_USE_EXTERNAL_USERDATA */


/*****************************************************************************
 *
 *  Macro that determines the size in bytes of the user data area.
 *  It is only used if TS_USER_EXTERNAL_USERDATA is defined.
 *
 *  The sample implementation just stores refCount and objectID. 
 *  We allow eight bytes for objectID enum. A real implementation
 *  should compute this value robustly.
 *
 *  See the TsExternalUserData_ structure in tseudat.h.
 *  
 *  <GROUP port>
 */
#define TS_EXTERNAL_USERDATA_SIZE (sizeof(TsRefCount) + 8)


/****************************************************************************
 *
 *  A macro that returns a TsColor value initialized with 8-bit
 *  alpha, red, green, blue components.
 *
 *  Description:
 *      This macro is used by color macros that are used in the reference 
 *      rendering implementation (tsrender.c) and to set default colors.
 *      The default implementation is ARGB8888. You may modify this macro
 *      if needed for a different color specification such as indexed color.
 * 
 *  Version:
 *      As of 3.1 this macro is moved to tsport.h.
 *
 *  <GROUP port>
 */
#define TS_COLOR_ARGB(a,r,g,b) (  (((TsColor)((TsByte)(a))) << 24)  | \
                                  (((TsColor)((TsByte)(r))) << 16)  | \
                                  (((TsColor)((TsByte)(g))) <<  8)  | \
                                  (((TsColor)((TsByte)(b)))      ) )

/****************************************************************************
 *
 *  A macro that is used to indicate a parameter is intentionally unused.
 *
 *  Description:
 *      This macro is used to indicate that a parameter is intentionally
 *      unused, so that compilers and lint tools will not complain.
 *
 *  <GROUP port>
 */
#define TS_UNUSED_PARAMETER(x) ((void)(x))

#endif /* TSPORT_H */

