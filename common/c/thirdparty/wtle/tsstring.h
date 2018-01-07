/*****************************************************************************
 *
 *  tsstring.h  - Defines interface to TsString class.
 *
 *  Copyright (C) 2005 Monotype Imaging Inc. All rights reserved.
 *
 *  Confidential Information of Monotype Imaging Inc.
 *
 ****************************************************************************/

#ifndef TSSTRING_H
#define TSSTRING_H

#include "tstypes.h"
#include "tsmem.h"
#include "tsunienc.h"
#include "tsdlp.h"

TS_BEGIN_HEADER


/*****************************************************************************
 *
 *  The string object.
 *
 *  Description:
 *
 *      A TsString is a string of Unicode characters.
 *
 *      The string can represent UTF8, UTF16, UTF32, or ASCII data.
 *      UTF16 and UTF32 must be given in native byte order.
 *
 *      The TsString initialization functions take a pointer to a
 *      contiguous chunk of character data owned by the client.
 *      TsString_initByCopy copies the character data and does not
 *      keep a reference to the original data.
 *      TsString_initByRef does not copy the data, but instead keeps
 *      a reference to it.
 *
 *      When initializing a TsString the length of the character data can
 *      be given as the number of code units (not characters) or the
 *      data can be NULL terminated.
 *
 *  <GROUP tsstring>
 */
typedef struct TsString_ TsString;


/* Private structure: do not access directly ********************************/

struct TsString_
{
    TsUInt32 properties;        /* string properties bit field */
    TsInt32 nChars;             /* string length in characters (before NULL terminator) */
    TsInt32 nUnits;             /* number of code units (including NULL terminator, if present) */
    TsInt32 nNonBMP;            /* number of characters in Unicode planes 1-15 */
    TsInt32 capacity;           /* current capacity of data in code units */
    TsInt32 cacheIndex;         /* stores index of most recently accessed code unit */
    void *cache;                /* pointer to most recent accessed code unit within data */
    void *data;                 /* string data */
    TsEncoding encoding;        /* text encoding type */
    TsMemMgr *memMgr;           /* pointer to memory manager */
    TsUInt32 (*getChar)(TsString *s, TsInt32 index); /* getChar function pointer */
};

/* End of private section ***************************************************/



/*****************************************************************************
 *
 *  Macro for use as the nUnits argument of the TsString_init functions
 *  to indicate that source text is NULL terminated.
 *
 *  <GROUP tsstring>
 */
#define TS_NULL_TERMINATED      (-1)


/******************************************************************************
 *
 *  Creates and initializes a new TsString object using a memory manager.
 *
 *  Parameters:
 *      memMgr      - [in] pointer to a memory manager object or NULL
 *
 *  Return value:
 *      Pointer to new TsString object or NULL if malloc error.
 *
 *  <GROUP tsstring>
 */
TS_EXPORT(TsString *)
TsString_newex(TsMemMgr *memMgr);


/******************************************************************************
 *
 *  Creates and initializes a new TsString object.
 *
 *  Parameters:
 *
 *  Return value:
 *      Pointer to new TsString object or NULL if malloc error.
 *
 *  <GROUP tsstring>
 */
TS_EXPORT(TsString *)
TsString_new(void);


/******************************************************************************
 *
 *  Frees any resource owned by the string object then frees the string object.
 *
 *  Parameters:
 *      s       - [in] pointer to string object
 *
 *  Return value:
 *      none
 *
 *  <GROUP tsstring>
 */
TS_EXPORT(void)
TsString_delete(TsString *s);


/******************************************************************************
 *
 *  Initializes a TsString to an empty string.
 *
 *  Parameters:
 *      object      - [in] pointer to string object
 *
 *  Remarks:
 *      Generally you should use one of the other initializer functions such
 *      as TsString_initByRef() or TsString_initByCopy(). This
 *      function is used internally.
 *
 *  Return value:
 *      TsResult value
 *
 *  <GROUP tsstring>
 */
TS_EXPORT(TsResult)
TsString_init(void *object);


/******************************************************************************
 *
 *  Copies one string object to another.
 *
 *  Description:
 *      This function copies the contents of the string object. If it has
 *      a copy of the character data then that data is copied as well.
 *      If the character data is referenced then only the reference is
 *      copied. Strings with referenced character data should be used
 *      with caution.
 *
 *  Parameters:
 *      objectDest  - [in] void pointer to destination string object
 *      objectSrc   - [in] void pointer to source string object
 *
 *  Return value:
 *      TsResult value.
 *
 *  <GROUP tsstring>
 */
TS_EXPORT(TsResult)
TsString_copy(void *objectDest, void *objectSrc);


/******************************************************************************
 *
 *  Compares one string object to another.
 *
 *  Description:
 *      The strings are equal if the character data and encodings
 *      are the identical.
 *
 *  Parameters:
 *      objectOne   - [in] void pointer to first string object
 *      objectTwo   - [in] void pointer to second string object
 *
 *  Return value:
 *      Comparison value
 *
 *  <GROUP tsstring>
 */
TS_EXPORT(TsInt32)
TsString_comp(void *objectOne, void *objectTwo);


/******************************************************************************
 *
 *  Frees resources owned by string object then reinitializes that object.
 *
 *  Parameters:
 *      object  - [in] void pointer string object
 *
 *  Return value:
 *      TsResult value
 *
 *  <GROUP tsstring>
 */
TS_EXPORT(void)
TsString_done(void *object);


/******************************************************************************
 *
 *  Initializes a string so that it references a chunk of external character data.
 *
 *  Description:
 *      A chunk is a contiguous block of character data in a specific encoding.
 *      This function should be used with caution because it references, rather
 *      than copying the data. This can be efficient for large strings, but
 *      the data must persist as long as the string or any copies of the string
 *      exist.
 *
 *      The size the chunk is specified in code units. A code unit is the
 *      basic unit of the encoding, e.g. 1 byte for UTF8, 2 bytes for UTF16,
 *      and 4 bytes for UTF32. Code units are not character values.
 *      It takes one or more code units to specify a character value.
 *
 *      If the chunk is known to be null terminated then you may specify
 *      TS_NULL_TERMINATED or any negative value as the nUnits parameter.
 *
 *      The encoding can be any of the encodings specified in the TsEncoding
 *      enumeration.
 *
 *  Parameters:
 *      s           - [in] pointer to string object
 *      data        - [in] pointer a chunk of character data
 *      nUnits      - [in] length of chunk in code units or TS_NULL_TERMINATED
 *      encoding    - [in] how the chunk of character data is encoded
 *
 *  Return value:
 *      none
 *
 *  <GROUP tsstring>
 */
TS_EXPORT(void)
TsString_initByRef(TsString *s, void *data, TsInt32 nUnits, TsEncoding encoding);


/******************************************************************************
 *
 *  Initializes a string so that it copies a chunk of external character data.
 *
 *  Description:
 *      A chunk is a contiguous block of character data in a specific encoding.
 *      This function safely copies the chunk.
 *
 *      The size the chunk is specified in code units. A code unit is the
 *      basic unit of the encoding, e.g. 1 byte for UTF8, 2 bytes for UTF16,
 *      and 4 bytes for UTF32. Code units are not character values.
 *      It takes one or more code units to specify a character value.
 *
 *      If the chunk is known to be null terminated then you may specify
 *      TS_NULL_TERMINATED or any negative value as the nUnits parameter.
 *
 *      The encoding can be any of the encodings specified in the TsEncoding
 *      enumeration.
 *
 *  Parameters:
 *      s           - [in] pointer to string object
 *      memMgr      - [in] pointer to a memory manager object or NULL
 *      data        - [in] pointer a chunk of character data
 *      nUnits      - [in] length of chunk in code units or TS_NULL_TERMINATED
 *      encoding    - [in] how the chunk of character data is encoded
 *
 *  Return value:
 *      TS_OK or error code.
 *
 *  <GROUP tsstring>
 */
TS_EXPORT(TsResult)
TsString_initByCopy(TsString *s, TsMemMgr *memMgr, void *data, TsInt32 nUnits, TsEncoding encoding);


/******************************************************************************
 *
 *  Set the contents of string by copy
 *
 *  Description:
 *      Similar to initByCopy except it doesn't touch capacity, data or memMgr.
 *      The string must have been previously initialized by calling TsStrin_new
 *      or TsString_initByCopy.
 *
 *  Parameters:
 *      s           - [in] pointer to string object
 *      data        - [in] pointer to a chunk of character data
 *      nUnits      - [in] length of chunk in code units or TS_NULL_TERMINATED
 *      encoding    - [in] how the chunk of character data is encoded
 *
 *  Return value:
 *      TS_OK or error code.
 *
 *  <GROUP tsstring>
 */
TsResult
TsString_set(TsString *s, void *data, TsInt32 nUnits, TsEncoding encoding);


/******************************************************************************
 *
 *  Returns the contents of the string.
 *
 *  Description:
 *      This function returns the internal string contents.
 *
 *      The string buffer is returned by reference via the buf pointer.
 *
 *      A TS_ERR_ARGUMENT_IS_NULL_PTR error will occur if 's' or 'buf' is NULL.
 *      The other arguments can be NULL if that argument is not needed.
 *
 *      The 'properties' values is the same as obtained
 *      by calling TsString_getProperties.
 *
 *  Notes:
 *      This function provides direct access to internal data.
 *      The 'buf' pointer is returned by reference to avoid a copy
 *      operation. The contents must be treated as read only.
 *      Also, the 'buf' pointer is not reference counted. It must not
 *      be stored or dereferenced after the calling routine has
 *      completed or after any changes have been made to TsText.
 *
 *  Parameters:
 *      s           - [in] pointer to string object
 *      buf         - [out] returns pointer to the string buffer
 *      nUnits      - [out] returns the number of code units in the buffer
 *      encoding    - [out] returns the character encoding
 *      length      - [out] returns the string length
 *      properties  - [out] returns a bit field containing string properties
 *
 *  Return value:
 *      TS_OK or TS_ERR_ARGUMENT_IS_NULL_PTR error code.
 *
 *  <GROUP tsstring>
 */
TS_EXPORT(TsResult)
TsString_getData(TsString *s, void **data, TsInt32 *nUnits, TsEncoding *encoding, TsInt32 *length, TsUInt32 *properties);


/******************************************************************************
 *
 *  Macro that returns a character value at a given index within string.
 *
 *  Description:
 *      This macro returns the character value as a UTF-32 value.
 *
 *      The macro is defined as a function pointer that is set
 *      when the string is initialized. There are optimized functions
 *      for simple strings that need no decoding and a more
 *      general function for strings that need decoding.
 *
 *      The macro arguments are evaluated more than once. To avoid
 *      side effects do not use expressions as arguments.
 *
 *      The index parameter is the index of the character within the
 *      string. The first character is index 0 and the last character
 *      has index equal to strlen - 1. Note that the index is not
 *      the byte number or code unit index.
 *
 *      The macro returns TS_UCHAR_INVALID_VALUE if
 *      s is NULL or index is < 0 or index >= string length.
 *
 *      Version:
 *          Changes in Version 3.1: changed to a macro.
 *
 *  Parameters:
 *      s           - [in] pointer to string object
 *      index       - [in] index into string
 *
 *  Return value:
 *      UTF32 Unicode character value.
 *
 *  <GROUP tsstring>
 */
#define TsString_getChar(s, index) ((s!=NULL) ? (s)->getChar((s), index) : 0)


/******************************************************************************
 *
 *  Returns length of string in characters, excluding any NULL terminator
 *  character.
 *
 *  Parameters:
 *      s           - [in] pointer to string object
 *
 *  Return value:
 *      Length of string in characters.
 *
 *  <GROUP tsstring>
 */
TS_EXPORT(TsInt32)
TsString_length(TsString *s);


/******************************************************************************
 *
 *  Returns the number of code units in string.
 *
 *  Description:
 *      Returns the number of code units corresponding with the first
 *      nChar code units in the string.
 *
 *      nChar equal zero returns the number of code units in the entire
 *      string, not including any NULL terminator.
 *
 *      Function returns 0 if s == NULL or if nChars is greater than
 *      number of characters in string.
 *
 *  Parameters:
 *      s           - [in] pointer to string object
 *      nChars      - [in] number of characters of interest
 *
 *  Return value:
 *      Length of character data in code units.
 *
 *  <GROUP tsstring>
 */
TS_EXPORT(TsInt32)
TsString_getNumUnits(TsString *s, TsInt32 nChars);


/******************************************************************************
 *
 *  Returns the number of bytes in the string. Value does not include
 *  a NULL terminator.
 *
 *  Parameters:
 *      s           - [in] pointer to string object
 *
 *  Return value:
 *      Length of character data in bytes.
 *
 *  <GROUP tsstring>
 */
TS_EXPORT(TsInt32)
TsString_getNumBytes(TsString *s);


/******************************************************************************
 *
 *  Returns the string encoding.
 *
 *  Description:
 *      The encoding of the data used in this string. Strings that are
 *      initialized by copy always use UTF-16 for internal storage
 *      regardless of the encoding of the source data.
 *
 *  Parameters:
 *      s           - [in] pointer to string object
 *
 *  Return value:
 *      Encoding used for storing string.
 *
 *  <GROUP tsstring>
 */
TS_EXPORT(TsEncoding)
TsString_getEncoding(TsString *s);


#define TS_STRING_NEEDS_BIDI                (TS_DLP_NEEDS_BIDI)
#define TS_STRING_NEEDS_DPS                 (TS_DLP_NEEDS_DPS)
#define TS_STRING_NEEDS_NFC                 (TS_DLP_NEEDS_NFC)
#define TS_STRING_NEEDS_SHAPING             (TS_DLP_NEEDS_SHAPING)
#define TS_STRING_HAS_TAB                   (TS_DLP_HAS_TAB)
#define TS_STRING_HAS_MANDATORY_LINE_BREAK  (TS_DLP_HAS_MANDATORY_LINE_BREAK)
#define TS_STRING_HAS_PARAGRAPH_SEPARATOR   (TS_DLP_HAS_PARAGRAPH_SEPARATOR)
#define TS_STRING_HAS_THAI                  (TS_DLP_HAS_THAI)
#define TS_STRING_HAS_DIACRITICS            (TS_DLP_HAS_DIACRITICS)
#define TS_STRING_HAS_CONTROL_CODES         (TS_DLP_HAS_CONTROL_CODES)
#define TS_STRING_IS_COPIED                 (0x00020000)
#define TS_STRING_DO_DECODE                 (0x00040000)


/******************************************************************************
 *
 *  Returns string properties as a bit field.
 *
 *  Description:
 *      This function returns a set of string properties
 *      that are derived from the properties of the characters in the string
 *      and the encoding.
 *
 *      These properties can be used to control whether certain layout
 *      algorithms are needed and whether the original string data can
 *      be accessed directly by index.
 *
 *      To use, extract the properties from the bit field using TS_GET_FLAG macro.
 *
 *      <TABLE>
 *      The bits are as follows:    Bit is set if:
 *
 *      TS_STRING_NEEDS_BIDI        string needs bidi algorithm
 *      TS_STRING_NEEDS_DPS         string needs diacritic positioning system
 *      TS_STRING_NEEDS_NFC         string needs normalization form C
 *      TS_STRING_NEEDS_SHAPING     string needs shaping
 *      TS_STRING_HAS_TAB           string has one or more tab characters
 *      TS_STRING_IS_BEYOND_LATIN1  string contains one or more characters beyond Latin1
 *      TS_STRING_IS_COPIED         characters in string are stored by copy (else referenced)
 *      TS_STRING_DO_DECODE         accessing characters in the string requires that the string be decoded
 *                                      (else characters can be accessed directly by index)
 *      </TABLE>
 *
 *  Parameters:
 *      s           - [in] pointer to string object
 *
 *  Return value:
 *      Returns derived properties as a bit field.
 *
 *  <GROUP tsstring>
 */
TS_EXPORT(TsUInt32)
TsString_getProperties(TsString *s);


/*****************************************************************************
 *
 *  Macro to indicate an index that is always less than or equal to the
 *  start of the string. The +10 is present to avoid wrapping around the edge.
 *
 *  <GROUP tsstring>
 */
#define TS_START_OF_STRING    (TS_INT32_MIN + 10)


/*****************************************************************************
 *
 *  Macro to indicate an index that is always greater than or equal to the
 *  end of the string. The -10 is present to avoid wrapping around the edge.
 *
 *  <GROUP tsstring>
 */
#define TS_END_OF_STRING      (TS_INT32_MAX - 10)


/******************************************************************************
 *
 *  Remove characters from a TsString object.
 *
 *  Parameters:
 *      s           - [in] pointer to string object
 *      startIndex  - [in] index of first character to remove
 *      endIndex    - [in] index of last character to remove
 *
 *  Return value:
 *      Number of bytes removed. 0 if error.
 *
 *  Notes:
 *      Currently the 's' string must be initialized by copy and have UTF16
 *      encoding. startIndex must be less than or equal to endIndex; otherwise
 *      the function will return 0.
 *
 *  <GROUP tsstring>
 */
TS_EXPORT(TsInt32)
TsString_removeChars(TsString *s, TsInt32 startIndex, TsInt32 endIndex);


/******************************************************************************
 *
 *  Inserts one TsString object into another.
 *
 *  Parameters:
 *      s           - [in] pointer to string object
 *      insert      - [in] pointer to string that will be inserted
 *      insertIndex - [in] index of s at which to do insertion
 *
 *  Return value:
 *      Number of characters inserted into string 's'. 0 if error.
 *
 *  Notes:
 *      Currently the 's' string must be initialized by copy and have UTF16
 *      encoding.
 *
 *  <GROUP tsstring>
 */
TS_EXPORT(TsInt32)
TsString_insertString(TsString *s, TsString *insert, TsInt32 insertIndex);


TS_END_HEADER

#endif /* TSSTRING_H */
