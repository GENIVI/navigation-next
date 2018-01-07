/*
Copyright (c) 2018, TeleCommunication Systems, Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
   * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the TeleCommunication Systems, Inc., nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, ARE
DISCLAIMED. IN NO EVENT SHALL TELECOMMUNICATION SYSTEMS, INC.BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*!--------------------------------------------------------------------------

    @file     UtfString.h
    @date     02/15/2009
    @defgroup MOBIUS_UTILS  Mobius utility classes

    UTF-8 character string class header file.
*/
/*
    (C) Copyright 2014 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
/*! @{ */

#pragma once
#ifndef UTFSTRING_H_IS_INCLUDED
#define UTFSTRING_H_IS_INCLUDED

#include <limits.h>


#include "NcdbTypes.h"

#include "UBaseString.h"
#include <vector>

namespace Ncdb {

class   UniString;   // Unicode strings

/*!    UTF8 string class.
    @note  Regular string operations may result in invalid UTF-8 code string.
           You can use UnicodeLength() function to check the integrity of UTF-8 string.
 */
class NCDB_API UtfString : public UBaseString<char>
{
public:
    //! Default constructor
    UtfString()
        { }

    /*! Constructor from Unicode characters with allocation parameters.
        @param[in] chars   - value to initialize the string with.
        @param[in] size     - size of buffer to be allocated in bytes.
        @param[in] sizeIncr - size of buffer allocation increment (in Unicode chars, not in bytes).
     */
    UtfString(const char* chars,  int size = 0, int sizeIncr = 0)
        : UBaseString<char>(chars,size,sizeIncr)
        { }

    /*! Constructor attaching the string to a preallocated buffer.
        @param[in] size     - size of buffer to be allocated in bytes.
        @param[in] pBuffer  - character buffer address.
        @param[in] chars   - value to initialize the string with.
     */
    UtfString(int size, char* pBuffer,  const char* chars = 0)
        : UBaseString<char>(size, pBuffer, chars)
        { }

    //! Copy constructor
    UtfString(const UtfString& str)
        : UBaseString<char>(str)
        { }

// ---------------  Copy/Concatenation ----------

    //! Concatenate with one character.
    UtfString& operator += (char  c);

    //! Concatenate with characters.
    UtfString& operator += (const char*  chars);

    //! Concatenate with other string.
    UtfString& operator += (const UtfString& src);

    //!     Copy operator from characters.
    UtfString& operator = (const char*  chars);

    //!     Copy operator.
    UtfString& operator = (const UtfString& str);

// ---------------  Conversion from Unicode ------------------

    //! Convert one Unicode character to UTF-8 and concatenate it with the string.
    UtfString& operator += (wchar_t wc);

    //! Convert characters from Unicode to UTF-8 and copy to current string.
    UtfString& operator = (const wchar_t* src);

// --------------- Length and sub-strings functions ----------------

    /*! Count Unicode characters, contained in string.
        @return  the length of string in Unicode characters or -1 of UTF-8 string is not valid.
    */
    int     Length() const;

    //! Get current string length in bytes (not in Unicode charatcres).
    int     LengthBytes()   const
            { return m_Length; }

    
    
    /*! Make Sub-string of Unicode characters, contained in string.
        The function parses the string and axtracts a range of Unicode characters, represented by UTF-8 code bytes.
        @param [out] result    - the string with resulting characters.
        @param [in]  uniStart  - starting index of sub-string (in Unicode characters from zero) at the initial string.
        @param [in]  uniLength - length of sub-string (in Unicode characters).
                                 If pLength < 0, then all characters until the end of the string are selected.
        @return  the length of resulting string in Unicode characters or -1 if UTF-8 string is invalid.
    */
    int Substr(UtfString& result, int uniStart, int uniLength = -1) const;

	/*! Make Sub-string in a new string.
        @note This variant is handy, but requires additional memory allocations.
        @param [in]  start  - starting index of sub-string  from zero in characters.
        @param [in]  length - length of sub-string in characters.
                                If pLength < 0, then all characters until the end of the string are selected.
        @return  the resulting sub-string.
    */
    UtfString  Substr( int start, int length = -1);

	/*! Make Sub-string of Unicode characters, contained in string.
        This different version of substring caters to the unicode characters at substring boundary.
        @param [out] result    - the string with resulting characters.
        @param [in]  uniStart  - starting index of sub-string (in Unicode characters from zero) at the initial string.
        @param [in]  uniLength - length of sub-string (in Unicode characters).
                                 If pLength < 0, then all characters until the end of the string are selected.
        @return  the length of resulting string in Unicode characters or -1 if UTF-8 string is invalid.
    */
    int SubstrUtfString(UtfString& result, int uniStart, int uniLength = -1) const;

	//! Reverse a UtfString 
    void ReverseUtfString(UtfString &out);

    /*! Split incoming string in AutoArray using array of char delimiters. Do not return empty strings.
	*/
	AutoArray<UtfString> Split( AutoArray<char> delimiters ) const;

// ---------------  Utility functions ----------------
    static UtfString Join(AutoArray<UtfString> &strArray, const char* delimeter);

    //!   Parse UTF8 bytes and detect one Unicode character.
    //!  @return pointer to next byte after parsed Unicode charecter or 0 in case of EOS or invalid UTF8 characters.
    static const char * ParseUnicodeChar(const char * chars, wchar_t&  wchar);

private:

};


// --------------- Iteration by Unicode characters  ----------------
class NCDB_API UtfStringIterator
{
public:

    UtfStringIterator() : m_chars(0)
        { }

    UtfStringIterator(const UtfString& str) : m_chars(str.GetBuffer())
        { }

    //!  Prepare to iterate by Unicode characters through Next() function.
    void     Begin(const UtfString&  str);

    //!  Get next Unicode characters from UtfStrinf.
    //!  @return  0 when the end of string or invalid character is reached.
    wchar_t  Next();

private:
    const char *    m_chars;
};

#ifdef _MSC_VER
template class NCDB_API AutoArray< UtfString >;
#endif

};

#endif // UTFSTRING_H_IS_INCLUDED
/*! @} */
